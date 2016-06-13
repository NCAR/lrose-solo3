#include <math.h>

#ifdef USE_RADX
#else
#include <se_defrkl.hh>
#endif

#include <se_utils.hh>

#include "FlagFrecklesCmd.hh"


/*********************************************************************
 * Constructors
 */

FlagFrecklesCmd::FlagFrecklesCmd() :
  ForEachRayCmd("flag-freckles", "flag-freckles in <field>"),
  _raq0(0),
  _raq1(0)
{
}


/*********************************************************************
 * Destructor
 */

FlagFrecklesCmd::~FlagFrecklesCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool FlagFrecklesCmd::doIt(const int frame_num, RadxRay &ray) const
{

  // Pull the arguments from the command

  std::string param_name = _cmdTokens[1].getCommandText();

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->modified = YES;
  seds->bad_flag_mask = seds->bad_flag_mask_array;

  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(frame_num);
  
  RadxField *field = ray.getField(param_name);
  
  if (field == 0)
  {
    // Field not found

    printf("Field to be unfolded: %s not found\n", param_name.c_str());
    seds->punt = YES;
    return false;
  }

  std::size_t num_gates = ray.getNGates();
  
  const Radx::fl32 *orig_data = field->getDataFl32();
  double bad = field->getMissingFl32();
  
  Radx::fl32 *data = new Radx::fl32[num_gates];
  memcpy(data, orig_data, num_gates * sizeof(Radx::fl32));
  
  int navg = seds->freckle_avg_count;
  double rcp_ngts = 1.0 / navg;

  unsigned short *boundary_mask = seds->boundary_mask;
  unsigned short *bad_flag_mask = seds->bad_flag_mask_array;

  // Process the data

  if (seds->process_ray_count == 1)
  {
    // Set up for two running average queues

    _raq0 = RAQ_return_queue(navg);
    _raq1 = RAQ_return_queue(navg);
  }

  int nn = navg + 1;

  for (std::size_t ndx_ss = 0; ndx_ss < num_gates;)
  {
    // Move the cell index to the first good gate inside the next boundary

    for (; ndx_ss < num_gates &&
	   (!boundary_mask[ndx_ss] || data[ndx_ss] == bad);
	 ndx_ss++);

    // See if we can set up a leading queue

    int mm;
    int jj;
    
    for (mm = 0, jj = ndx_ss;
	 mm < nn && jj < num_gates && boundary_mask[jj]; jj++)
    {
      if (data[jj] != bad)
	mm++;
    }

    if (mm < nn)
    {
      // Can't set up queue

      ndx_ss = jj;
      continue;
    }

    bool out_of_bounds = false;

    // Initialize the leading average queue

    struct que_val *qv0 = _raq0->at;
    _raq0->sum = _raq1->sum = 0;

    int ndx_q0;
    
    for (ndx_q0 = ndx_ss, mm = 0; ; ndx_q0++)
    {
      double xx = data[ndx_q0];
      if (xx != bad)
      {
	// Don't use the first good gate in the avg

	if (!mm++)
	  continue;

	// Put this val in the first queue

	_raq0->sum += xx;
	qv0->val = xx;
	qv0 = qv0->next;
	if (mm >= navg + 1)
	  break;
      }
    }

    double ref0 = _raq0->sum * rcp_ngts;
    struct que_val *qv1 = _raq1->at;

    // Now loop through the data until we have encountered navg good gates
    // for the trailing average

    for (mm = 0; ndx_q0 < num_gates; ndx_ss++)
    {
      double xx = data[ndx_ss];
      if(xx == bad)
	continue;

      if (fabs(xx - ref0) > seds->freckle_threshold)
      {
	// Flag this gate

	bad_flag_mask[ndx_ss] = 1;
      }
      else
      {
	// Add this point to the trailing average

	_raq1->sum += xx;
	qv1->val = xx;
	qv1 = qv1->next;
	if (++mm >= navg)
	  break;
      }

      // Find the next good point for the leading average

      for (ndx_q0++; ndx_q0 < num_gates; ndx_q0++)
      {
	if (!boundary_mask[ndx_q0])
	{
	  // We've gone beyond the boundary

	  out_of_bounds = true;
	  break;
	}

	double xx = data[ndx_q0];
	if (xx != bad)
	{
	  _raq0->sum -= qv0->val;
	  _raq0->sum += xx;
	  qv0->val = xx;
	  qv0 = qv0->next;
	  ref0 = _raq0->sum * rcp_ngts;
	  break;
	}
      }

      if (out_of_bounds || ndx_q0 >= num_gates)
	break;
    }

    if (out_of_bounds || ndx_q0 >= num_gates)
    {
      ndx_ss = ndx_q0;
      continue;
    }

    double ref1 = _raq1->sum * rcp_ngts;

    // Else shift to a trailing average

    for (ndx_ss++; ndx_ss < num_gates; ndx_ss++)
    {
      // Check to see if we've gone beyond the boundary

      if (!boundary_mask[ndx_ss])
	break;

      double xx = data[ndx_ss];
      if (xx == bad)
	continue;

      if (fabs(xx - ref1) > seds->freckle_threshold)
      {
	// Flag this gate

	bad_flag_mask[ndx_ss] = 1;
      }
      else
      {
	// Add this point to the trailing average

	_raq1->sum -= qv1->val;
	_raq1->sum += xx;
	qv1->val = xx;
	qv1 = qv1->next;
	ref1 = _raq1->sum * rcp_ngts;
      }
    }
  }
  
  // Update the data in the field object

  field->setDataFl32(num_gates, data, false);
  
  return true;
}

#else

bool FlagFrecklesCmd::doIt() const
{
  if (se_flag_freckles(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
