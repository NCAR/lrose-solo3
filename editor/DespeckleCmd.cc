#ifdef USE_RADX
#else
#include <se_defrkl.hh>
#endif

#include <se_utils.hh>

#include "DespeckleCmd.hh"


/*********************************************************************
 * Constructors
 */

DespeckleCmd::DespeckleCmd() :
  ForEachRayCmd("despeckle", "despeckle <field>")
{
}


/*********************************************************************
 * Destructor
 */

DespeckleCmd::~DespeckleCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool DespeckleCmd::doIt(const int frame_num, RadxRay &ray) const
{
  // Pull the arguments from the command

  std::string dst_name = _cmdTokens[1].getCommandText();

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->modified = 1;

  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(frame_num);
  
  RadxField *field = ray.getField(dst_name);
  
  if (field == 0)
  {
    // Field not found

    printf("Field to be thresholded: %s not found\n", dst_name.c_str());
    seds->punt = 1;
    return false;
  }

  std::size_t num_gates = ray.getNGates();
  const Radx::fl32 *orig_data = field->getDataFl32();
  Radx::fl32 *data = new Radx::fl32[num_gates];
  memcpy(data, orig_data, num_gates * sizeof(Radx::fl32));

  double bad = field->getMissingFl32();
  
  int a_speckle = seds->a_speckle;
  unsigned short *boundary_mask = seds->boundary_mask;

  // Do the work

  for (std::size_t i = 0; i < num_gates; ++i)
  {
    if (boundary_mask[i] == 0)
      continue;
    
    if (data[i] == bad)
      continue;
    
    // At this point, we are at the first valid gate inside the boundary.
    // Now look forward to see how many gates of valid data we have.

    std::size_t num_valid = 1;
    bool crossed_boundary = false;
    
    for (std::size_t j = i+1; j < num_gates; ++j)
    {
      if (boundary_mask[j] == 0)
      {
	crossed_boundary = true;
	break;
      }
      
      if (data[j] == bad)
	break;
      
      ++num_valid;
    }
    
    // If we crossed the boundary edge or have too many valid gates for
    // a speckle, continue on after this run of data

    if (crossed_boundary || num_valid > a_speckle)
    {
      i += num_valid;
      continue;
    }
    
    // If we get here, we have a speckle and need to zap it

    for (std::size_t j = i; j < i + num_valid; ++j)
      data[j] = bad;
    
    i += num_valid;
  }

  // Update the ray data

  field->setDataFl32(num_gates, data, false);
  
  return true;
}

#else

bool DespeckleCmd::doIt() const
{
  if (se_despeckle(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
