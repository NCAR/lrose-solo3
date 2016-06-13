#include <math.h>

#ifdef USE_RADX
#else
#include <se_BB_unfold.hh>
#endif

#include <running_avg_que.hh>
#include <se_utils.hh>
#include <solo_defs.h>

#include "BBUnfoldingCmd.hh"


/*********************************************************************
 * Constructors
 */

BBUnfoldingCmd::BBUnfoldingCmd() :
  ForEachRayCmd("BB-unfolding", "BB-unfolding of <field>")
{
  setOneTimeOnly(true);
}


/*********************************************************************
 * Destructor
 */

BBUnfoldingCmd::~BBUnfoldingCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool BBUnfoldingCmd::doIt(const int frame_num, RadxRay &ray) const
{
  // Pull the arguments from the command

  std::string param_name = _cmdTokens[1].getCommandText();

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->modified = 1;

  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(frame_num);
  
  RadxField *field = ray.getField(param_name);
  if (field == 0)
  {
    printf("Source parameter %s not found for copy\n", param_name.c_str());
    seds->punt = 1;
    return false;
  }
  
  // Access the data in the array

  std::size_t num_gates = ray.getNGates();
  const Radx::fl32 *orig_data = field->getDataFl32();
  
  // Copy the data to a local buffer that we can update

  Radx::fl32 *data = new Radx::fl32[num_gates];
  memcpy(data, orig_data, num_gates * sizeof(Radx::fl32));
  double bad = field->getMissingFl32();

  double nyqv = seds->nyquist_velocity ? seds->nyquist_velocity
    : ray.getNyquistMps();
//  int scaled_nyqv = (int)DD_SCALE(nyqv, scale, bias);
  double nyqi = 2.0 * nyqv;
  double rcp_nyqi = 1.0 / nyqi;
//  int scaled_nyqi = 2 * scaled_nyqv;
//  float rcp_nyqi = 1.0 / (float)scaled_nyqi;

  if (seds->process_ray_count == 1)
  {
    // Set up for two running average ques

    _raq0 = RAQ_return_queue(seds->BB_avg_count);
    char *init_string =
      (char *)(seds->BB_init_on == BB_USE_FGG ? "the first good gate"
	       : "the wind");
    printf("Nyq. vel: %.1f; Initializing on %s; Averaging %d cells\n",
	   nyqv, init_string, seds->BB_avg_count);
  }

  float rcp_qsize = _raq0->rcp_num_vals;
  double v0;
  
  if (seds->BB_init_on == BB_USE_FGG)
  {
    // Initialize of the first good gate in the sweep

    if (seds->sweep_ray_count == 1)
    {
      v0 = bad;
      _lastGoodV0 = bad;
    }
    else
    {
      v0 = _lastGoodV0;
    }
    
    if (v0 == bad)
    {
      // Find first good gate

      bool good_gate_found = false;
      
      for (size_t i = 0; i < num_gates; ++i)
      {
	if (data[i] != bad)
	{
	  v0 = data[i];
	  good_gate_found = true;
	  break;
	}
      }
      
      if (!good_gate_found)
	return true;
    }

  }
  else
  {
    double u;
    double v;
    double w;
    
    if (seds->BB_init_on == BB_USE_AC_WIND)
    {
      // NOTE: Need to figure out the winds

//      u = data_info->getEWHorizWind();
//      v = data_info->getNSHorizWind();
//      w = data_info->getVertWind() != -999 ? data_info->getVertWind() : 0;
      u = 0.0;
      v = 0.0;
      w = 0.0;
    }
    else
    {
      // Local wind

      u = seds->ew_wind;
      v = seds->ns_wind;
      w = seds->ud_wind;
    }

    double dazm = RADIANS(ray.getAzimuthDeg());
    double dele = RADIANS(ray.getElevationDeg());
    double insitu_wind = cos(dele) *
      (u * sin(dazm) + v * cos(dazm)) + w * sin(dele);
    v0 = insitu_wind;
  }

  _raq0->sum = 0;
  struct que_val *qv0 = _raq0->at;

  // Initialize the running average queue

  for (int ii = 0; ii < _raq0->num_vals; ii++)
  {
    _raq0->sum += v0;
    qv0->d_val = v0;
    qv0 = qv0->next;
  }

  // Loop through the data

  short *bnd = (short *)seds->boundary_mask;
  bool first_cell = true;
  
  for (std::size_t i = 0; i < num_gates; ++i)
  {
    // Only process good gates

    if (bnd[i] == 0 || data[i] == bad)
      continue;
    
    double vx = data[i];
    double v4 = _raq0->sum * rcp_qsize;
    double folds = (v4 - vx) * rcp_nyqi;
    folds = folds < 0 ? folds - 0.5 : folds + 0.5;
    int fold_count = (int)folds;
    
    if (fold_count)
    {
      int nn;
    
      if (fold_count > 0)
      {
	nn = fold_count - seds->BB_max_pos_folds;
	if (nn > 0)
	  fold_count -= nn;
      }
      else
      {
	nn = fold_count + seds->BB_max_neg_folds;
	if (nn < 0)
	  fold_count -= nn;
      }
    }

    vx += fold_count * nyqi;

    _raq0->sum -= qv0->d_val;
    _raq0->sum += vx;
    qv0->d_val = vx;
    qv0 = qv0->next;

    data[i] = vx;

    if (first_cell)
    {
      first_cell = false;
      _lastGoodV0 = vx;
    }
  }

  // Update the ray data

  // NOTE:  I'm not sure about the isLocal flag.  Would it be better to use
  // true and then free the data buffer after this call?

  field->setDataFl32(num_gates, data, false);
  
  return true;
}

#else

bool BBUnfoldingCmd::doIt() const
{
  if (se_BB_ac_unfold(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
