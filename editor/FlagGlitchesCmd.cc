#include <math.h>

#ifdef USE_RADX
#else
#include <se_defrkl.hh>
#endif

#include <se_utils.hh>

#include "FlagGlitchesCmd.hh"


/*********************************************************************
 * Constructors
 */

FlagGlitchesCmd::FlagGlitchesCmd() :
  ForEachRayCmd("flag-glitches", "flag-glitches in <field>"),
  _queSize(0),
  _que(0),
  _qctr(0)
{
}


/*********************************************************************
 * Destructor
 */

FlagGlitchesCmd::~FlagGlitchesCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool FlagGlitchesCmd::doIt(const int frame_num, RadxRay &ray) const
{
  // Pull the arguments from the command.

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

    printf("Field to be deglitched: %s not found\n", param_name.c_str());
    seds->punt = YES;
    return false;
  }

  std::size_t num_gates = ray.getNGates();
  const Radx::fl32 *data = field->getDataFl32();
  double bad = field->getMissingFl32();
  
  if (seds->deglitch_radius < 1)
    seds->deglitch_radius = 3;
  
  int navg = (seds->deglitch_radius * 2) + 1;
  int half = navg / 2;

  unsigned short *boundary_mask = seds->boundary_mask;
  unsigned short *bad_flag_mask = seds->bad_flag_mask_array;

  // Do the work

  int min_bins;
  
  if (seds->deglitch_min_bins > 0)
  {
    if (seds->deglitch_min_bins > navg)
      seds->deglitch_min_bins = navg;
    min_bins = seds->deglitch_min_bins;
  }
  else
  {
    seds->deglitch_min_bins = navg;
    min_bins = navg;
  }

  if (seds->process_ray_count == 1)
  {
    // set up 

    if (navg > _queSize)
    {
      if (_queSize != 0)
	delete [] _que;

      _que = new double[navg];
      _queSize = navg;
    }
  }

  for (int ndx_ss = 0; ndx_ss < num_gates; )
  {
    // Move the gate index to the first gate inside the next boundary

    for ( ; ndx_ss < num_gates && !boundary_mask[ndx_ss]; ndx_ss++);

    // Set up the queue

    int ndx_qend = 0;
    int good_bins = 0;
    double sum = 0;

    // And start looking for the good gates count to equal or exceed the
    // min_bins and the center bin not be bad

    for (int mm = 0; ndx_ss < num_gates && boundary_mask[ndx_ss]; ndx_ss++)
    {
      if (++mm > navg)
      {
	// After the que is full

	double ival = _que[ndx_qend];

	if (ival != bad)
	{
	  good_bins--;
	  sum -= ival;
	}
      }

      // Raw data value

      double ival = data[ndx_ss];
      _que[ndx_qend] = ival;
      if (ival != bad)
      {
	sum += ival;
	good_bins++;
      }

      int ndx_qctr = (ndx_qend - half + navg) % _queSize;
      _qctr = _que + ndx_qctr;

      if (good_bins >= min_bins && *_qctr != bad)
      {
	// Do a test

	double davg = (sum - *_qctr) / (double)(good_bins - 1);
	double diff = fabs(davg - *_qctr);
	
	if (diff > seds->deglitch_threshold)
	{
	  sum -= *_qctr;
	  good_bins--;
	  *_qctr = bad;
	  bad_flag_mask[ndx_ss - half] = 1; /* flag this gate */
	}
      }
      ++ndx_qend;
      ndx_qend %= _queSize;
    }
  }
  
  return true;
}

#else

bool FlagGlitchesCmd::doIt() const
{
  if (se_flag_glitches(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
