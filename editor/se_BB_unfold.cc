/* 	$Id$	 */

#include <stdio.h>
#include <glib.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include <dd_math.h>
#include <DataInfo.hh>
#include <DataManager.hh>
#include <EarthRadiusCalculator.hh>
#include <running_avg_que.hh>
#include "se_BB_unfold.hh"
#include "se_utils.hh"
#include <solo_editor_structs.h>
#include <solo2.hh>

extern GString *gs_complaints;

#define         BB_USE_FGG 0
#define     BB_USE_AC_WIND 1
#define  BB_USE_LOCAL_WIND 2


/* c------------------------------------------------------------------------ */

#ifdef USE_RADX
#else

int se_ac_surface_tweak(const std::vector< UiCommandToken > &cmd_tokens)
{
  /* 
   * #remove-surface#
   * #remove-only-surface#
   * #remove-only-second-trip-surface#
   */

  // Pull the arguments from the command

  std::string name = cmd_tokens[1].getCommandText();

  bool surface_only =
    cmd_tokens[0].getCommandText().find("ly-sur") != std::string::npos; /* only-surface */
  bool only_2_trip =
    cmd_tokens[0].getCommandText().find("ly-sec") != std::string::npos; /* only-second-trip-surface */

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();
  if (seds->finish_up)
    return 1;

  seds->modified = YES;

  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(seds->se_frame);
  
  // We probably need to make a calculation to determine if the ground echo
  // goes above the aircraft i.e. is the distance (arc length) defined by a
  // ray of length max_range rotated through half the beam width greater than
  // or equal to the altitude of the aircraft?

  double bmwidth = RADIANS(seds->optimal_beamwidth ? seds->optimal_beamwidth :
			   data_info->getVertBeamWidth());
  double half_vbw = 0.5 * bmwidth;

  double alt = data_info->getPlatformAltitudeAGL() * 1000.0;
  double max_range = data_info->getGateRangeC(data_info->getClipGate());
  double elev = data_info->getElevationAngle2();

  double min_grad = 0.08;	/* dbz per meter */
  double elev_limit = -0.0001;
  double fudge = 1.0;
  int g1;
  
  char *alt_gecho_string = getenv ("ALTERNATE_GECHO");
  bool alt_gecho_flag = false;

  if (surface_only && alt_gecho_string != 0)
  {
    if (elev > -0.002)	/* -.10 degrees */
      return 1;

    alt_gecho_flag = true;
    double d = atof(alt_gecho_string);
    if (d > 0)
      min_grad = d;
  }

  double d = max_range * fudge * bmwidth;
  if (!surface_only && d >= alt)
  {
    d -= alt;
    elev_limit = atan2(d, (double)max_range);

    if (elev > elev_limit)
      return 1;

    if (d >= 0 && elev > -fudge * bmwidth)
    {
      only_2_trip = true;
      g1 = 0;
    }
  }

  if (elev > elev_limit)
    return 1;

  if (!only_2_trip)
  {
    double earthr =
      EarthRadiusCalculator::getInstance()->getRadius(data_info->getPlatformLatitude());
    elev -= half_vbw;
    double tan_elev = tan(elev);
    double range1 = (-alt / sin(elev)) *
      (1.0 + alt / (2.0 * earthr * 1000.0 * tan_elev * tan_elev));
    if (range1 > max_range || range1 < 0 )
      return 1;
    
    g1 = data_info->getGateNum(range1);
  }

  int gate_shift = seds->surface_gate_shift;

  if (alt_gecho_flag)
  {
    int zmax_cell;
    
    int ii = data_info->alternateGecho(min_grad, zmax_cell);
    if (ii > 0)
    {
      g1 = ii;
      gate_shift = 0;
    }
    else
    {
      return 1;
    }
  }

  int field_num = data_info->getParamIndex(name);
  if (field_num < 0)
  {
    printf("Source parameter %s not found for surface removal\n", name.c_str());
    seds->punt = YES;
    return -1;
  }

  g1 += gate_shift;
  if (g1 < 0)
    g1 = 0;

  short *data_ptr = (short *)data_info->getParamData(field_num);
  short *end_data_ptr = data_ptr + data_info->getClipGate() + 1;
  short bad = static_cast<short>(data_info->getParamBadDataValue(field_num));

  data_ptr += g1;

  for (; data_ptr < end_data_ptr;)
    *data_ptr++ = bad;
  
  return 1;
}

#endif

/* c------------------------------------------------------------------------ */

#ifdef USE_RADX
#else

int se_BB_ac_unfold (const std::vector< UiCommandToken > &cmd_tokens)
{
  /* #BB-unfolding# */

  // Pull the arguments from the command

  std::string name = cmd_tokens[1].getCommandText();

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();
  if (seds->finish_up)
    return 1;

  seds->modified = YES;

  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(seds->se_frame);
  
  int param_num = data_info->getParamIndex(name);
  if (param_num < 0)
  {
    printf("Source parameter %s not found for copy\n", name.c_str());
    seds->punt = YES;
    return -1;
  }

  int num_cells = data_info->getClipGate() + 1;
  short *end_gate_ptr = (short *)data_info->getParamData(param_num) + num_cells;

  short bad = static_cast<short>(data_info->getParamBadDataValue(param_num));
  float scale = data_info->getParamScale(param_num);
  float bias = data_info->getParamBias(param_num);
  float nyqv = seds->nyquist_velocity ? seds->nyquist_velocity
    : data_info->getNyquistVelocity();
  int scaled_nyqv = (int)DD_SCALE(nyqv, scale, bias);
  int scaled_nyqi = 2 * scaled_nyqv;
  float rcp_nyqi = 1.0 / (float)scaled_nyqi;

  static struct running_avg_que *raq0;
  
  if (seds->process_ray_count == 1)
  {
    // Set up for two running average ques

    raq0 = RAQ_return_queue(seds->BB_avg_count);
    char *aa = (char *)(seds->BB_init_on == BB_USE_FGG ? "the first good gate"
			: "the wind");
    printf("Nyq. vel: %.1f; Initializing on %s; Averaging %d cells\n"
	   , nyqv, aa, seds->BB_avg_count);
  }

  float rcp_qsize = raq0->rcp_num_vals;
  short v0;
  static short last_good_v0;
  
  if (seds->BB_init_on == BB_USE_FGG)
  {
    // Initialize of the first good gate in the sweep

    if(seds->sweep_ray_count == 1)
      v0 = last_good_v0 = bad;
    else
      v0 = last_good_v0;

    if (v0 == bad)
    {
      // Find first good gate

      short *gate_ptr = (short *)data_info->getParamData(param_num);

      for (; *gate_ptr == bad && gate_ptr < end_gate_ptr; gate_ptr++);
      if (gate_ptr == end_gate_ptr)
	return 1;
      v0 = *gate_ptr;
    }
  }
  else
  {
    double u;
    double v;
    double w;
    
    if (seds->BB_init_on == BB_USE_AC_WIND)
    {
      u = data_info->getEWHorizWind();
      v = data_info->getNSHorizWind();
      w = data_info->getVertWind() != -999 ? data_info->getVertWind() : 0;
    }
    else
    {
      // Local wind

      u = seds->ew_wind;
      v = seds->ns_wind;
      w = seds->ud_wind;
    }

    double dazm = RADIANS(data_info->getAzimuthAngleCalc());
    double dele = RADIANS(data_info->getElevationAngleCalc());
    double insitu_wind = cos(dele) *
      (u * sin(dazm) + v * cos(dazm)) + w * sin(dele);
    v0 = (short)DD_SCALE(insitu_wind, scale, bias);
  }

  raq0->sum = 0;
  struct que_val *qv0 = raq0->at;

  // Initialize the running average queue

  for (int ii = 0; ii < raq0->num_vals; ii++)
  {
    raq0->sum += v0;
    qv0->val = v0;
    qv0 = qv0->next;
  }

  // Loop through the data

  short *bnd = (short *)seds->boundary_mask;
  short *gate_ptr = (short *)data_info->getParamData(param_num);

  bool first_cell = true;
  
  int i = 0;
  
  for(; gate_ptr < end_gate_ptr;)
  {
    // Find the next good gate

    for (; gate_ptr < end_gate_ptr && (!(*bnd) || *gate_ptr == bad);
	 gate_ptr++, bnd++, i++);

    if (gate_ptr == end_gate_ptr)
      break;

    short vx = *gate_ptr;
    short v4 = (int)(raq0->sum * rcp_qsize);
    double folds = (v4 - vx) * rcp_nyqi;
    folds = folds < 0 ? folds - 0.5 : folds + 0.5;
    int fold_count = (int)folds;

    if (fold_count)
    {
      int nn;
    
      if (fold_count > 0)
      {
	if ((nn = fold_count - seds->BB_max_pos_folds) > 0)
	  fold_count -= nn;
      }
      else if ((nn = fold_count + seds->BB_max_neg_folds) < 0)
      {
	fold_count -= nn;
      }
    }

    vx += fold_count*scaled_nyqi;

    raq0->sum -= qv0->val;
    raq0->sum += vx;
    qv0->val = vx;
    qv0 = qv0->next;

    *gate_ptr++ = vx;
    bnd++;
    i++;
    
    if (first_cell)
    {
      first_cell = false;
      last_good_v0 = vx;
    }
  }

  return 1;
}

#endif

/* c------------------------------------------------------------------------ */
/* This routine will process all commands associated with the setup-input    */
/*  procedure. It also updates the sfic texts.                               */

#ifdef USE_RADX
#else

int se_BB_setup(const std::vector< UiCommandToken > &cmd_tokens)	
{
  /*
   * #BB-gates-averaged#
   * #BB-max-pos-folds#
   * #BB-max-neg-folds#
   * #BB-use-first-good-gate#
   * #BB-use-ac-wind#
   * #BB-use-local-wind#
   * #nyquist-velocity#
   * ##
   * ##
   * ##
   */

  struct solo_edit_stuff *seds = return_sed_stuff();

  if (cmd_tokens[0].getCommandText().compare(0, 11, "BB-gates-averaged",
					   0, 11) == 0)
  {
    seds->BB_avg_count = cmd_tokens[1].getIntParam();
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 11, "BB-max-pos-folds",
						0, 11) == 0)
  {
    seds->BB_max_pos_folds = cmd_tokens[1].getIntParam();
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 11, "BB-max-neg-folds",
						0, 11) == 0)
  {
    seds->BB_max_neg_folds = cmd_tokens[1].getIntParam();
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 9, "BB-use-first-good-gate",
						0, 9) == 0)
  {
    seds->BB_init_on = BB_USE_FGG;
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 9, "BB-use-ac-wind",
						0, 9) == 0)
  {
    seds->BB_init_on = BB_USE_AC_WIND;
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 9, "BB-use-local-wind",
						0, 9) == 0)
  {
    seds->BB_init_on = BB_USE_LOCAL_WIND;
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 3, "nyquist-velocity",
						0, 3) == 0)
  {
    seds->nyquist_velocity = FABS(cmd_tokens[1].getFloatParam());
  }

  return 1;
}

#endif

/* c------------------------------------------------------------------------ */
/* Remove the aircraft motion from velocities.                               */

#ifdef USE_RADX
#else

int se_remove_ac_motion(const std::vector< UiCommandToken > &cmd_tokens)
{
  /* #remove-aircraft-motion# */

  // Pull the arguments from the command

  std::string name = cmd_tokens[1].getCommandText();

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();
  if (seds->finish_up)
    return 1;

  seds->modified = YES;
  
  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(seds->se_frame);
  
  int field_num = data_info->getParamIndex(name);
  if (field_num < 0)
  {
    printf("Source parameter %s not found for copy\n", name.c_str());
    seds->punt = YES;
    return -1;
  }

  float ac_vel = data_info->getAircraftVelocity();
  int num_cells = data_info->getClipGate() + 1;

  short *data_ptr = (short *)data_info->getParamData(field_num);
  short *end_data_ptr = data_ptr +num_cells;
  short bad = static_cast<short>(data_info->getParamBadDataValue(field_num));
  float scale = data_info->getParamScale(field_num);
  float bias = data_info->getParamBias(field_num);

  float nyqv = seds->nyquist_velocity ?
    seds->nyquist_velocity : data_info->getNyquistVelocity();
  int scaled_nyqv = (int)DD_SCALE(nyqv, scale, bias);
  int scaled_nyqi = 2 * scaled_nyqv;
  int scaled_ac_vel = (int)DD_SCALE(ac_vel, scale, bias);
  int adjust = scaled_ac_vel % scaled_nyqi;

  if (abs(adjust) > scaled_nyqv)
    adjust = adjust > 0 ? adjust - scaled_nyqi : adjust + scaled_nyqi;

  short *bnd = (short *) seds->boundary_mask;

  // Process the command

  for (; data_ptr < end_data_ptr; data_ptr++, bnd++)
  {
    if (!(*bnd) || *data_ptr == bad)
      continue;

    short vx = *data_ptr + adjust;
    if (abs(vx) > scaled_nyqv)
      vx = vx > 0 ? vx - scaled_nyqi : vx + scaled_nyqi;

    *data_ptr = vx;
  }

  return 1;
}

#endif

/* c------------------------------------------------------------------------ */
/* Remove the aircraft motion from velocities.                               */

#ifdef USE_RADX
#else

int se_remove_storm_motion(const std::vector< UiCommandToken > &cmd_tokens)
{
  /* #remove-storm-motion# */

  // Pull the arguments from the command

  std::string name = cmd_tokens[1].getCommandText();
  float wind = cmd_tokens[2].getFloatParam(); /* angle */
  wind = FMOD360(wind + 180.0); /* change to wind vector */
  float speed = cmd_tokens[3].getFloatParam();

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();
  if (seds->finish_up)
    return 1;

  seds->modified = YES;

  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(seds->se_frame);
  
  int field_num = data_info->getParamIndex(name);
  if (field_num < 0)
  {
    printf("Source parameter %s not found for copy\n", name.c_str());
    seds->punt = YES;
    return -1;
  }

  int num_cells = data_info->getClipGate() + 1;

  short *data_ptr = (short *)data_info->getParamData(field_num);
  short *end_data_ptr = data_ptr +num_cells;
  float scale = data_info->getParamScale(field_num);
  float bias = data_info->getParamBias(field_num);
  short bad = static_cast<short>(data_info->getParamBadDataValue(field_num));

  double az = data_info->getRotationAngleCalc();
  double cosEl = cos(RADIANS(data_info->getElevationAngleCalc()));
  if (fabs(cosEl) < 0.0001)
    return 1;
  double rcp_cosEl = 1.0 / cosEl;
  double theta = angle_diff(az, wind); /* clockwise from az to wind */
  double adjust = cos(RADIANS (theta)) * speed;

  double scaled_adjust = DD_SCALE(adjust, scale, bias);

  short *bnd = (short *) seds->boundary_mask;

  // Process the command

  for (; data_ptr < end_data_ptr; data_ptr++, bnd++)
  {
    if (!(*bnd) || *data_ptr == bad)
      continue;

    double d = (*data_ptr * cosEl - scaled_adjust) * rcp_cosEl;
    *data_ptr = (short)d;
  }

  return 1;
}

#endif

/* c------------------------------------------------------------------------ */
