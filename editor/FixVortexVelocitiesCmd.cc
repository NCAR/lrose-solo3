#ifdef USE_RADX
#else
#include <se_defrkl.hh>
#endif

#include <se_utils.hh>

#include "FixVortexVelocitiesCmd.hh"


/*********************************************************************
 * Constructors
 */

FixVortexVelocitiesCmd::FixVortexVelocitiesCmd() :
  ForEachRayCmd("fix-vortex-velocities",
		"fix-vortex-velocities in <field>")
{
}


/*********************************************************************
 * Destructor
 */

FixVortexVelocitiesCmd::~FixVortexVelocitiesCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool FixVortexVelocitiesCmd::doIt(const int frame_num, RadxRay &ray) const
{
  // Pull the arguments from the command

  std::string param_name = _cmdTokens[1].getCommandText();

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

    printf("Vortex velocity field: %s not found\n", param_name.c_str());
    seds->punt = YES;
    return false;
  }

  RadxField *vl_field = ray.getField("VL");
  
  if (vl_field == 0)
  {
    // Field not found

    printf("VL field not found");
    seds->punt = YES;
    return false;
  }

  RadxField *vs_field = ray.getField("VS");
  
  if (vs_field == 0)
  {
    // Field not found

    printf("VS field not found");
    seds->punt = YES;
    return false;
  }

  const std::size_t num_gates = ray.getNGates();
  
  const Radx::fl32 *orig_data = field->getDataFl32();
  Radx::fl32 *data = new Radx::fl32[num_gates];
  memcpy(data, orig_data, num_gates * sizeof(Radx::fl32));
  double bad = field->getMissingFl32();
  
  const Radx::fl32 *vl_data = vl_field->getDataFl32();
  double vl_bad = vl_field->getMissingFl32();
  
  const Radx::fl32 *vs_data = vs_field->getDataFl32();
  double vs_bad = vs_field->getMissingFl32();
  
  const unsigned short *boundary_mask = seds->boundary_mask;

  if (seds->process_ray_count == 1)
  {
    const std::vector< double > frequencies = data_info->getFrequencyHz();
    
    double total_freq = 0.0;
    
    for (std::vector< double >::const_iterator freq = frequencies.begin();
	 freq != frequencies.end(); ++freq)
      total_freq += *freq;
    
    double wvl = SPEED_OF_LIGHT / (total_freq / (double)frequencies.size());
    
    // Get the PRFs

    double prfS = ray.getPrtSec();
    double prfL = ray.getPrtSec() * ray.getPrtRatio();
    
    if (ray.getPrtRatio() < 1.0)
    {
      double temp = prfS;
      prfS = prfL;
      prfL = temp;
    }
    
    float X = prfS / (prfS - prfL);
    float Y = prfL / (prfS - prfL);
    double ratioXY = prfS / prfL;
    int levels = (int)(X + Y + 0.1);
    _levelBias = levels / 2;
    double av_nyqL = 0.25 * wvl * prfL;
    _rcpHalfNyqL = 1.0 / (0.5 * av_nyqL);
    double *ctr = &_vConsts[_levelBias];
    *ctr = 0.0;

    for (int ii = 1; ii <= _levelBias; ii++)
    {
      switch(ii)
      {
	// These cases represent (vL - vS) normalized by half the nyquist
	// velocity

      case 1 :
      {
	double d = av_nyqL * (1.0 + ratioXY);
	*(ctr - ii) = d;
	*(ctr + ii) = -d;
	break;
      }
      
      case 2 :
      {
	double d = 2.0 * av_nyqL * (1.0 + ratioXY);
	*(ctr - ii) = d;
	*(ctr + ii) = -d;
	break;
      }
      
      case 3 :
      {
	double d = av_nyqL * (2.0 + ratioXY);
	*(ctr - ii) = -d;
	*(ctr + ii) = d;
	break;
      }
      
      case 4 :
      {
	double d = av_nyqL;
	*(ctr - ii) = -d;
	*(ctr + ii) = d;
	break;
      }
      }
    }

    // vS corresponds to V1 in some of the documentation and vL corresponds
    // to V2

    double d = -2.0 * av_nyqL;

    for (int jj = 0; jj < levels; jj++)
    {
      printf("dual prt const: %6.2f for (vs-vl) = %8.4f\n",
	     _vConsts[jj], d);
      d += 0.5 * av_nyqL;
    }
  }

  for (std::size_t i = 0; i < num_gates; ++i)
  {
    if (boundary_mask[i] == 0)
      continue;

    // NOTE: Need to figure out these calculations.  The initial calculations
    // were done on scaled data so need to double-check things like kk.

    double vS = vs_data[i];
    double vL = vl_data[i];
    double d = (vS - vL) * _rcpHalfNyqL + _levelBias + 0.5;
    int kk = (int)d;
    data[i] = ((vS + vL) / 2.0) + _vConsts[kk];
  }
  
  // Update the data in the ray

  field->setDataFl32(num_gates, data, false);
  
  return true;
}

#else

bool FixVortexVelocitiesCmd::doIt() const
{
  if (se_fix_vortex_vels(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
