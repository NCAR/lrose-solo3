#include <math.h>

#ifdef USE_RADX
#else
#include <se_for_each.hh>
#endif

#include <se_utils.hh>
#include <sii_externals.h>

#include "ForcedUnfoldingCmd.hh"


/*********************************************************************
 * Constructors
 */

ForcedUnfoldingCmd::ForcedUnfoldingCmd() :
  ForEachRayCmd("forced-unfolding",
		"forced-unfolding in <field> around <real>")
{
}


/*********************************************************************
 * Destructor
 */

ForcedUnfoldingCmd::~ForcedUnfoldingCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool ForcedUnfoldingCmd::doIt(const int frame_num, RadxRay &ray) const
{
  // Pull the arguments out of the command

  std::string param_name = _cmdTokens[1].getCommandText();
  float ctr = _cmdTokens[2].getFloatParam();

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->modified = YES;

  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(frame_num);
  
  RadxField *field = ray.getField(param_name);
  
  if (field == 0)
  {
    // Field not found

    g_string_sprintfa(gs_complaints, "Field to be unfolded: %s not found\n",
		      param_name.c_str());
    seds->punt = YES;
    return false;
  }

  const std::size_t num_gates = ray.getNGates();
  const Radx::fl32 *orig_data = field->getDataFl32();
  Radx::fl32 *data = new Radx::fl32[num_gates];
  memcpy(data, orig_data, num_gates * sizeof(Radx::fl32));
  
  const double bad = field->getMissingFl32();
  
  float nyqv = seds->nyquist_velocity ?
    seds->nyquist_velocity : ray.getNyquistMps();
  if (nyqv <= 0)
    return -1;

  if (seds->process_ray_count == 1)
  {
    printf("Nyquist vel: %.1f\n", nyqv);
  }

  double nyqi = 2.0 * nyqv;
  double rcp_nyqi = 1.0 / nyqi;

  unsigned short *boundary_mask = seds->boundary_mask;

  // Loop through the data

  for (std::size_t i = 0; i < num_gates; ++i)
  {
    if (boundary_mask[i] == 0 || data[i] == bad)
      continue;

    double diff = ctr - data[i];
    if (fabs(diff) > nyqv)
    {
      double nn = diff * rcp_nyqi + (diff < 0 ? -.5 : .5);
      data[i] += nn * nyqi;
    }
  }
  
  return true;
}

#else

bool ForcedUnfoldingCmd::doIt() const
{
  if (se_funfold(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
