/* 	$Id$	 */

#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <dd_math.h>
#include <DataInfo.hh>
#include <DataManager.hh>
#include "se_catch_all.hh"
#include "se_for_each.hh"
#include "se_utils.hh"
#include <seds.h>
#include <solo_editor_structs.h>

extern GString *gs_complaints;

/* c------------------------------------------------------------------------ */

int se_for_each_ray(const std::vector< UiCommandToken > &cmd_tokens)
{
  /* #for-each-ray# */

  return 1;
}

/* c------------------------------------------------------------------------ */

#ifdef USE_RADX
#else

int se_header_value(const std::vector< UiCommandToken > &cmd_tokens)
{
  /* #header-value# */

  // Pull the arguments from the command

  std::string name = cmd_tokens[1].getCommandText();
  std::string s_val = cmd_tokens[2].getCommandText();
  float f_val = cmd_tokens[2].getFloatParam();

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();
  if(seds->finish_up)
    return 1;

  seds->modified = YES;

  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(seds->se_frame);

  if (data_info->isNewSweep())
  {
    if (strstr("range-to-first-cell", name.c_str()) != 0)
    {
      data_info->setStartRange(f_val);
    }
    else if (strstr("cell-spacing", name.c_str()) != 0)
    {
      data_info->setGateSpacing(f_val);
    }
    else if (strstr("nyquist-velocity", name.c_str()) != 0)
    {
      data_info->setNyquistVelocity(f_val);
    }
    else if (strstr("fixed-angle", name.c_str()) != 0)
    {
      data_info->setFixedAngle(f_val);
    }
    else if (strstr("latitude", name.c_str()) != 0)
    {
      data_info->setRadarLatitude(f_val);
    }
    else if (strstr("longitude", name.c_str()) != 0)
    {
      data_info->setRadarLongitude(f_val);
    }
    else if (strstr("altitude", name.c_str()) != 0)
    {
      data_info->setRadarAltitude(f_val);
    }
    else if (strstr("scan-mode", name.c_str()) != 0)
    {
      data_info->setScanMode(s_val);
    }
    else if (strstr("ipp1", name.c_str()) != 0)
    {
      data_info->setInterPulsePeriod(0, f_val);
    }
    else if (strstr("ipp2", name.c_str()) != 0)
    {
      data_info->setInterPulsePeriod(1, f_val);
    }
  }

  if (strstr("tilt-angle", name.c_str()) != 0)
  {
    data_info->setTiltAngle(f_val);
  }
  else if (strstr("rotation-angle", name.c_str()) != 0)
  {
    data_info->setRotationAngle(f_val);
  }
  else if (strstr("elevation-angle", name.c_str()) != 0)
  {
    data_info->setElevationAngle(f_val);
  }
  else if (strstr("diddle-elevation", name.c_str()) != 0)
  {
    data_info->setElevationAngle(FMOD360(180.0 - data_info->getElevationAngle()));
  }
  else if (strstr("corr-elevation", name.c_str()) != 0)
  {
    data_info->setElevationAngle(FMOD360(data_info->getElevationAngle() + f_val));
  }
  else if( strstr("corr-azimuth", name.c_str()) != 0)
  {
    data_info->setAzimuthAngle(FMOD360(data_info->getAzimuthAngle() + f_val));
  }
  else if (strstr("corr-rot-angle", name.c_str()) != 0)
  {
    data_info->setRotationAngle(FMOD360(data_info->getRotationAngle() + f_val));
  }
  else if (strstr("latitude", name.c_str()) != 0)
  {
    data_info->setPlatformLatitude(f_val);
  }
  else if (strstr("longitude", name.c_str()) != 0)
  {
    data_info->setPlatformLongitude(f_val);
  }
  else if (strstr("altitude", name.c_str()) != 0)
  {
    data_info->setPlatformAltitudeMSL(f_val);
  }
  else if (strstr("agl-altitude", name.c_str()) != 0)
  {
    data_info->setPlatformAltitudeAGL(f_val);
  }
  else if (strstr("msl-into-agl-corr", name.c_str()) != 0)
  {
    data_info->setPlatformAltitudeAGL(data_info->getPlatformAltitudeMSL() + f_val);
  }

  return 1;
}  

#endif

/* c------------------------------------------------------------------------ */
/* Trigger a rewrite of this ray even though no other editing is taking      */
/* place                                                                     */

#ifdef USE_RADX
#else

int se_rewrite(const std::vector< UiCommandToken > &cmd_tokens)
{
  /* #rewrite# */

  // Get the editor information

  struct solo_edit_stuff *seds= return_sed_stuff();
  if (seds->finish_up)
    return 1;

  seds->modified = YES;

  return 1;
}

#endif

/* c------------------------------------------------------------------------ */

#ifdef USE_RADX
#else

int se_remove_field(const std::vector< UiCommandToken > &cmd_tokens)
{
  /* #ignore-field# */

  // Pull the arguments from the command

  std::string name = cmd_tokens[1].getCommandText();

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();
  if (seds->finish_up)
    return 1;

  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(seds->se_frame);
  
  if (data_info->removeField(name))
    seds->modified = YES;
  
  return 1;
}

#endif

/* c------------------------------------------------------------------------ */

#ifdef USE_RADX
#else

int se_cpy_field(const std::vector< UiCommandToken > &cmd_tokens)
{
  /*
   * #duplicate#
   * #establish-and-reset#
   * #copy#
   * #flagged-copy#
   * #shift-field#
   */

  // Pull the arguments out of the command

  std::string src_name = cmd_tokens[1].getCommandText();
  std::string dst_name = cmd_tokens[2].getCommandText();

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();
  if (seds->finish_up)
    return 1;

  seds->modified = YES;

  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(seds->se_frame);
  
  int src_field_num;
  int dst_field_num;
  int ii;
  
  if ((ii = data_info->establishField(src_name, dst_name,
				     src_field_num, dst_field_num)) < 0)
  {
    seds->punt = YES;
    return ii;
  }

  int num_cells = data_info->getNumCells();

  short *src_data_ptr = (short *)data_info->getParamData(src_field_num);
  short *end_src_data_ptr = src_data_ptr +num_cells;
  float src_scale = data_info->getParamScale(src_field_num);
  float src_rcp_scale = 1.0 / src_scale;
  float src_bias = data_info->getParamBias(src_field_num);
  
  short *dst_data_ptr = (short *)data_info->getParamData(dst_field_num);
  float dst_scale = data_info->getParamScale(dst_field_num);
  float dst_bias = data_info->getParamBias(dst_field_num);
  int dst_bad = static_cast<int>(data_info->getParamBadDataValue(dst_field_num));

  bool rescale = false;
  
  if (src_scale != dst_scale ||
      src_bias != dst_bias)
  {
    rescale = true;
  }

  unsigned short *bnd = seds->boundary_mask;

  // Here's where we finally do the copying

  if (cmd_tokens[0].getCommandText().compare(0, 3, "duplicate", 0, 3) == 0)
  {
    if (rescale)
    {
      for (; src_data_ptr < end_src_data_ptr; src_data_ptr++, dst_data_ptr++)
      {
	// Don't use boundary

	if (*src_data_ptr == dst_bad)
	{
	  *dst_data_ptr = dst_bad;
	}
	else
	{
	  float x = DD_UNSCALE((float)(*src_data_ptr), src_rcp_scale, src_bias);
	  *dst_data_ptr = (short)DD_SCALE(x, dst_scale, dst_bias);
	}
      }
    }
    else
    {
      for (; src_data_ptr < end_src_data_ptr; src_data_ptr++, dst_data_ptr++)
      {
	// Don't use boundary

	*dst_data_ptr = *src_data_ptr;
      }
    }
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 4, "shift-field", 0, 4) == 0)
  {
    int fshift = seds->gates_of_shift;
	
    int ii = fshift > 0 ? num_cells -1 : 0;
    int inc = fshift > 0 ? -1 : 1;
    int nn = fshift > 0 ? num_cells - fshift : num_cells + fshift;

    if (!rescale)
    {
      for ( ; nn--; ii += inc)
      {
	dst_data_ptr[ii] = src_data_ptr[ii - fshift];
      }
    }
    else
    {
      for ( ; nn--; ii += inc)
      {
	if (src_data_ptr[ii - fshift] == dst_bad)
	{
	  dst_data_ptr[ii] = dst_bad;
	}
	else
	{
	  float x = DD_UNSCALE((float)(src_data_ptr[ii - fshift]),
			       src_rcp_scale, src_bias);
	  dst_data_ptr[ii] = (short)DD_SCALE(x, dst_scale, dst_bias);
	}
      }
    }

    // Fill in at whichever end

    ii = fshift < 0 ? num_cells -1 : 0;
    inc = fshift < 0 ? -1 : 1;
    nn = fshift < 0 ? -fshift : fshift;

    for (; nn--; ii += inc)
    {
      dst_data_ptr[ii] = dst_bad;
    }
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 3, "establish-and-reset", 0, 3) == 0)
  {
    end_src_data_ptr = dst_data_ptr + num_cells;

    // Just fill with bad flags

    for (; dst_data_ptr < end_src_data_ptr; *dst_data_ptr++ = dst_bad);
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 3, "flagged-copy", 0, 3) == 0)
  { 
    // Copy using flags and boundary

    seds->bad_flag_mask = seds->bad_flag_mask_array;
    unsigned short *flag = seds->bad_flag_mask_array;
      
    for (; src_data_ptr < end_src_data_ptr;
	 src_data_ptr++, dst_data_ptr++, bnd++, flag++)
    {
      if (*bnd && *flag)
      {
	// Copies everything including missing data

	if (rescale)
	{
	  float x = DD_UNSCALE((float)(*src_data_ptr), src_rcp_scale, src_bias);
	  *dst_data_ptr = (short)DD_SCALE(x, dst_scale, dst_bias);
	}
	else
	{
	  *dst_data_ptr = *src_data_ptr;
	}
      }
    }
  }
  else
  {
    // Copy using boundary

    for (; src_data_ptr < end_src_data_ptr;
	 src_data_ptr++, dst_data_ptr++, bnd++)
    {
      if (*bnd)
      {
	if (rescale)
	{
	  if (*src_data_ptr == dst_bad)
	  {
	    *dst_data_ptr = dst_bad;
	  }
	  else
	  {
	    float x =
	      DD_UNSCALE((float)(*src_data_ptr), src_rcp_scale, src_bias);
	    *dst_data_ptr = (short)DD_SCALE(x, dst_scale, dst_bias);
	  }
	}
	else
	{
	  *dst_data_ptr = *src_data_ptr;
	}
      }
    }
  }

  return 1;
}  

#endif

/* c------------------------------------------------------------------------ */

#ifdef USE_RADX
#else

int se_funfold(const std::vector< UiCommandToken > &cmd_tokens)
{
  /* #forced-unfolding# */

  // Pull the arguments out of the command

  std::string name = cmd_tokens[1].getCommandText();
  float ctr = cmd_tokens[2].getFloatParam();

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
    // Field not found

    g_string_sprintfa(gs_complaints, "Field to be unfolded: %s not found\n",
		      name.c_str());
    seds->punt = YES;
    return -1;
  }

  int num_cells = data_info->getClipGate() + 1;

  short *data_ptr = (short *)data_info->getParamData(field_num);
  short *end_data_ptr = data_ptr +num_cells;
  int bad = static_cast<int>(data_info->getParamBadDataValue(field_num));

  int scaled_ctr = data_info->scaleValue(ctr, field_num);
  
  float nyqv = seds->nyquist_velocity ?
    seds->nyquist_velocity : data_info->getNyquistVelocity();
  if (nyqv <= 0)
    return -1;

  if (seds->process_ray_count == 1)
  {
    printf("Nyquist vel: %.1f\n", nyqv);
  }

  int scaled_nyqv = data_info->scaleValue(nyqv, field_num);
  int scaled_nyqi = (int)(2.0 * scaled_nyqv);
  float rcp_nyqi = 1.0 / (float)scaled_nyqi;

  unsigned short *bnd = seds->boundary_mask;

  // Loop through the data

  for (; data_ptr < end_data_ptr; data_ptr++, bnd++)
  {
    if (!(*bnd) || *data_ptr == bad)
      continue;

    int idiff = scaled_ctr - (*data_ptr);
    if (abs(idiff) > scaled_nyqv)
    {
      int nn = (int)(idiff * rcp_nyqi + (idiff < 0 ? -.5 : .5));
      *data_ptr += nn * scaled_nyqi;
    }
  }

  return field_num;
}  

#endif

/* c------------------------------------------------------------------------ */

#ifdef USE_RADX
#else

int se_hard_zap(const std::vector< UiCommandToken > &cmd_tokens)
{
  /* #unconditional-delete# */

  // Pull the arguements from the command

  std::string dst_name = cmd_tokens[1].getCommandText();

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();
  if (!seds->boundary_exists)
    return 1;

  if (seds->finish_up)
    return 1;

  seds->modified = YES;

  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(seds->se_frame);
  
  int field_num = data_info->getParamIndex(dst_name);
  
  if (field_num < 0)
  {
    // Field not found

    g_string_sprintfa(gs_complaints, "Field to be deleted: %s not found\n",
		      dst_name.c_str());
    seds->punt = YES;
    return -1;
  }

  int num_cells = data_info->getClipGate() + 1;
  short *data_ptr = (short *)data_info->getParamData(field_num);
  short *end_data_ptr = data_ptr + num_cells;
  int bad = static_cast<int>(data_info->getParamBadDataValue(field_num));

  unsigned short *bnd = seds->boundary_mask;

  // Loop through the data

  for (; data_ptr < end_data_ptr; data_ptr++, bnd++)
  {
    if(*bnd)
      *data_ptr = bad;
  }

  return field_num;
}  

#endif

/* c------------------------------------------------------------------------ */

#ifdef USE_RADX
#else

int se_threshold_field(const std::vector< UiCommandToken > &cmd_tokens)
{
  /* #threshold# */

  // Pull the arguments out of the command

  int token_num = 1;
  
  std::string dst_name = cmd_tokens[token_num++].getCommandText();
  std::string thr_name = cmd_tokens[token_num++].getCommandText();
  std::string where = cmd_tokens[token_num++].getCommandText();
  float what = cmd_tokens[token_num++].getFloatParam();
  float what2 = 0.0;
  if (cmd_tokens[token_num].getTokenType() == UiCommandToken::UTT_VALUE)
    what2 = cmd_tokens[token_num].getFloatParam();
  
  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();
  if (seds->finish_up)
    return 1;

  seds->modified = YES;

  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(seds->se_frame);
  
  int thr_field_num = data_info->getParamIndex(thr_name);
  if (thr_field_num < 0)
  {
    // Thr field not found

    g_string_sprintfa(gs_complaints, "Threshold field: %s not found\n",
		      thr_name.c_str());
    seds->punt = YES;
    return -1;
  }

  int dst_field_num = data_info->getParamIndex(dst_name);
  if (dst_field_num < 0)
  {	
    // Field not found

    g_string_sprintfa(gs_complaints, "Field to be thresholded: %s not found\n",
		      dst_name.c_str());
    seds->punt = YES;
    return -1;
  }

  int num_cells = data_info->getClipGate() + 1;

  short *thr_data_ptr = (short *)data_info->getParamData(thr_field_num);
  int thr_bad = static_cast<int>(data_info->getParamBadDataValue(thr_field_num));

  short *dst_data_ptr = (short *)data_info->getParamData(dst_field_num);
  short *end_dst_data_ptr = dst_data_ptr + num_cells;
  int dst_bad = static_cast<int>(data_info->getParamBadDataValue(dst_field_num));

  data_info->setParamThreshold(dst_field_num, thr_name, what);
  
  int scaled_thr1 = data_info->scaleValue(what, thr_field_num);
  int scaled_thr2 = 0;
  if (cmd_tokens[token_num].getTokenType() == UiCommandToken::UTT_VALUE)
    scaled_thr2 = data_info->scaleValue(what2, thr_field_num);

  int fgg = seds->first_good_gate;
  unsigned short *bnd = seds->boundary_mask;

  // Loop through the data

  for (int gg = 0;  gg < fgg && dst_data_ptr < end_dst_data_ptr;
       *dst_data_ptr = dst_bad, gg++, dst_data_ptr++, thr_data_ptr++, bnd++);

  if (where.compare(0, 3, "below", 0, 3) == 0)
  {
    for (; dst_data_ptr < end_dst_data_ptr;
	 dst_data_ptr++, thr_data_ptr++, bnd++)
    {
      if (!(*bnd) || *dst_data_ptr == dst_bad)
	continue;

      if (*thr_data_ptr == thr_bad || *thr_data_ptr < scaled_thr1)
	*dst_data_ptr = dst_bad;
    }
  }
  else if (where.compare(0, 3, "above", 0, 3) == 0)
  {
    for (; dst_data_ptr < end_dst_data_ptr;
	 dst_data_ptr++, thr_data_ptr++, bnd++)
    {
      if (!(*bnd) || *dst_data_ptr == dst_bad)
	continue;

      if (*thr_data_ptr == thr_bad || *thr_data_ptr > scaled_thr1)
	*dst_data_ptr = dst_bad;
    }
  }
  else
  {
    // Between

    if (cmd_tokens[token_num].getTokenType() == UiCommandToken::UTT_VALUE)
    {
      for (; dst_data_ptr < end_dst_data_ptr;
	   dst_data_ptr++, thr_data_ptr++, bnd++)
      {
	if (!(*bnd) || *dst_data_ptr == dst_bad)
	  continue;

	if (*thr_data_ptr == thr_bad ||
	    (*thr_data_ptr >= scaled_thr1 && *thr_data_ptr <= scaled_thr2))
	{
	  *dst_data_ptr = dst_bad;
	}
      }
    }
  }

  return dst_field_num;
}  

#endif

/* c------------------------------------------------------------------------ */

#ifdef USE_RADX
#else

int se_radial_shear(const std::vector< UiCommandToken > &cmd_tokens)
{
  /* #radial-shear# */

  // Pull the arguments from the command

  std::string src_name = cmd_tokens[1].getCommandText();
  std::string dst_name = cmd_tokens[2].getCommandText();

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();
  if (seds->finish_up)
    return 1;

  seds->modified = YES;

  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(seds->se_frame);
  
  int ii;
  int src_field_num;
  int dst_field_num;
  
  if ((ii = data_info->establishField(src_name, dst_name,
				     src_field_num, dst_field_num)) < 0)
  {
    seds->punt = YES;
    return ii;
  }

  short *src_data_ptr = (short *)data_info->getParamData(src_field_num);
  short *dd = src_data_ptr + seds->gate_diff_interval;
  short *end_src_data_ptr = src_data_ptr + data_info->getClipGate() + 1;
  int src_bad = static_cast<int>(data_info->getParamBadDataValue(src_field_num));

  short *dst_data_ptr = (short *)data_info->getParamData(dst_field_num);

  unsigned short *bnd = seds->boundary_mask;

  // Loop through the data

  for (; dd < end_src_data_ptr;)
  {
    // Move inside the next boundary

    for (; dd < end_src_data_ptr && !(*bnd);
	 dd++, src_data_ptr++, dst_data_ptr++, bnd++);

    if (dd == end_src_data_ptr)
      break;
    
    // See if we can calculate a shear

    if (*dd == src_bad || *src_data_ptr == src_bad)
      *dst_data_ptr = src_bad;
    else
      *dst_data_ptr = *dd - *src_data_ptr;

    dd++; src_data_ptr++; dst_data_ptr++; bnd++;
  }

  return dst_field_num;
}  

#endif

/* c------------------------------------------------------------------------ */

