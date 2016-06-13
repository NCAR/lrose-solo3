#include <math.h>

#ifdef USE_RADX
#else
#include <se_add_mult.hh>
#endif

#include <sii_externals.h>
#include <se_utils.hh>

#include "AbsoluteValueCmd.hh"


/*********************************************************************
 * Constructors
 */

AbsoluteValueCmd::AbsoluteValueCmd() :
  ForEachRayCmd("absolute-value", "absolute-value of <field>")
{
}


/*********************************************************************
 * Destructor
 */

AbsoluteValueCmd::~AbsoluteValueCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool AbsoluteValueCmd::doIt(const int frame_num, RadxRay &ray) const
{
  // Pull the arguments from the command

  std::string param_name = _cmdTokens[1].getCommandText();

  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->modified = 1;

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(frame_num);
  
  // Find the field 

  RadxField *field = ray.getField(param_name);
  
  if (field == 0)
  {
    // Field not found

    g_string_sprintfa(gs_complaints, "Field to be abs()'d: %s not found\n",
		      param_name.c_str());
    seds->punt = 1;
    return false;
  }

  // Access the data in the ray

  std::size_t num_gates = ray.getNGates();
  const Radx::fl32 *orig_data = field->getDataFl32();
  
  // Copy the data to a local buffer that we can update

  Radx::fl32 *data = new Radx::fl32[num_gates];
  memcpy(data, orig_data, num_gates * sizeof(Radx::fl32));
  
  // Loop through the data

  double bad = field->getMissingFl32();
  
  unsigned short *boundary_mask = seds->boundary_mask;
  
  for (std::size_t i = 0; i < num_gates; ++i)
  {
    // Don't process data outside of the boundary or if the data value is bad

    if (boundary_mask[i] == 0 || data[i] == bad)
      continue;

    // Replace the data value with its absolute value

    data[i] = fabs(data[i]);
  }

  // Update the ray data

  field->setDataFl32(num_gates, data, false);
  
  return true;
}

#else

bool AbsoluteValueCmd::doIt() const
{
  if (se_absolute_value(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
