/* 	$Id$	 */

#include <glib.h>
#include <string.h>
#include <stdlib.h>

#include <dd_math.h>
#include <DataInfo.hh>
#include <DataManager.hh>
#include <running_avg_que.hh>
#include "se_defrkl.hh"
#include "se_utils.hh"
#include <solo_editor_structs.h>
#include <solo2.hh>

extern GString *gs_complaints;

/* c------------------------------------------------------------------------ */
/* Routine to remove a ring of data; e.g. a test pulse                       */

#ifdef USE_RADX
#else

int se_rescale_field(const std::vector< UiCommandToken > &cmd_tokens)
{
  /* #rescale-field# */

  // Pull the arguments from the command

  std::string dst_name = cmd_tokens[1].getCommandText();
  float scale = cmd_tokens[2].getFloatParam();
  float bias = cmd_tokens[3].getFloatParam();

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();
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

    printf("Field to be rescaled: %s not found\n", dst_name.c_str());
    seds->punt = YES;
    return -1;
  }

  if (data_info->isNewSweep())
  {
    // Nab and keep the old scale and bias

    seds->old_scale[field_num] = data_info->getParamScale(field_num);
    seds->old_bias[field_num] = data_info->getParamBias(field_num);
    data_info->setParamScale(field_num, scale);
    data_info->setParamBias(field_num, bias);
  }

  float rcp_scale = 1.0 / seds->old_scale[field_num];
  float old_bias = seds->old_bias[field_num];

  int num_cells = data_info->getNumCells();

  short *data_ptr = (short *)data_info->getParamData(field_num);
  short *end_data_ptr = data_ptr + num_cells;
  int bad = static_cast<int>(data_info->getParamBadDataValue(field_num));
    
  // Loop through the data

  for (; data_ptr < end_data_ptr; data_ptr++)
  {
    if (*data_ptr == bad)
      continue;	

    float x = DD_UNSCALE((float)(*data_ptr), rcp_scale, old_bias);
    *data_ptr = (short)DD_SCALE(x, scale, bias);
  }

  return field_num;
}

#endif

/* c------------------------------------------------------------------------ */
/* Routine to remove a ring of data; e.g. a test pulse                       */

#ifdef USE_RADX
#else

int se_ring_zap(const std::vector< UiCommandToken > &cmd_tokens)
{
  /* #remove-ring# */

  // Pull the arguments from the command

  std::string dst_name = cmd_tokens[1].getCommandText();
  float r1 = KM_TO_M(cmd_tokens[2].getFloatParam());
  float r2 = 1.0e22;
  if (cmd_tokens[3].getTokenType() == UiCommandToken::UTT_VALUE)
    r2 = KM_TO_M(cmd_tokens[3].getFloatParam());

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();
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

    printf("Field to be de-ringed: %s not found\n", dst_name.c_str());
    seds->punt = YES;
    return -1;
  }

  int g1 = data_info->getGateNum(r1);
  int g2 = data_info->getGateNum(r2) + 1;

  short *data_ptr = (short *)data_info->getParamData(field_num);
  int bad = static_cast<int>(data_info->getParamBadDataValue(field_num));

  data_ptr += g1;
    
  unsigned short *bnd = seds->boundary_mask + g1;

  // Loop through the data

  for (; g1++ < g2; data_ptr++, bnd++)
  {
    if (!(*bnd))
      continue;	

    *data_ptr = bad;
  }

  return field_num;
}

#endif

/* c------------------------------------------------------------------------ */
/* Routine to remove discountinuities (freckles) from the data such as birds */
/* and radio towers by comparing a particular data point to a running        */
/* average that is ahead of the point of interest but switches to a trailing */
/* average once enough good points have been encountered                     */

#ifdef USE_RADX
#else

int se_fix_vortex_vels(const std::vector< UiCommandToken > &cmd_tokens)
{
 /* #fix-vortex-velocities# */

  static int vConsts[16];
  static int *ctr;
  static int level_bias;
  static double rcp_half_nyqL;
  static double d_round = 0.5;

  // Pull the arguments from the command

  std::string name = cmd_tokens[1].getCommandText();

  /*
   * boundary mask is set to 1 if the corresponding cell satisfies
   * conditions of the boundaries
   */

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
    printf("Vortex velocity field: %s not found\n", name.c_str());
    seds->punt = YES;
    return -1;
  }
  
  int vl_field_num = data_info->getParamIndex("VL");
  if (vl_field_num < 0)
  {
    printf("Vortex velocity field: %s not found\n", name.c_str());
    seds->punt = YES;
    return -1;
  }

  int vs_field_num = data_info->getParamIndex("VS");
  if (vs_field_num < 0)
  {
    printf("Vortex velocity field: %s not found\n", name.c_str());
    seds->punt = YES;
    return -1;
  }

  int num_cells = data_info->getClipGate() + 1;

  short *data_ptr = (short *)data_info->getParamData(field_num);
  short *end_data_ptr = data_ptr + num_cells;

  short *vl_data_ptr = (short *)data_info->getParamData(vl_field_num);

  short *vs_data_ptr = (short *)data_info->getParamData(vs_field_num);
  float vs_scale = data_info->getParamScale(vs_field_num);
  float vs_bias = data_info->getParamBias(vs_field_num);

  short *bnd = (short *) seds->boundary_mask;

  if (seds->process_ray_count == 1)
  {
    // set up constants
    // first calculate the average frequency
    
    struct parameter_d *parm = data_info->getParamInfo(vs_field_num);

    // This assumes the scale and the bias are the same for all three fields

    double d;
    int ii;
    int kk;
    
    for (d = 0, ii = 0, kk = 1; ii < 5; ii++, kk <<= 1)
    {
      if (kk & parm->xmitted_freq)
	d += data_info->getFrequency(ii);
    }

    // Frequency is in Ghz 

    double wvl =
      SPEED_OF_LIGHT / ((d / (double)data_info->getNumFreqTrans()) * 1.e9);

    // Get the PRFs

    for (ii = 0, kk = 1; ii < 5; ii++, kk <<= 1)
    {
      if (kk & parm->interpulse_time)
	break;
    }

    // prfS is the greater prf
    // the interpulse period is in milliseconds

    double prfS = 1.0 / (data_info->getInterPulsePeriod(ii) * 0.001);
    parm = data_info->getParamInfo(vl_field_num);

    for (ii = 0, kk = 1; ii < 5; ii++, kk <<= 1)
    {
      if (kk & parm->interpulse_time)
	break;
    }

    double prfL = 1.0 / (data_info->getInterPulsePeriod(ii) * 0.001);

    float X = prfS / (prfS - prfL);
    float Y = prfL / (prfS - prfL);
    double ratioXY = prfS / prfL;
    int levels = (int)(X + Y + 0.1);
    level_bias = levels/2;
    double av_nyqL = 0.25 * wvl * prfL;
    rcp_half_nyqL = 1./DD_SCALE((.5 * av_nyqL), vs_scale, vs_bias);
    ctr = &vConsts[level_bias];
    *ctr = 0;

    for (ii = 1; ii <= level_bias; ii++)
    {
      switch(ii)
      {
	// These cases represent (vL - vS) normalized by half the nyquist
	// velocity

      case 1 :
      {
	double d = DD_SCALE(av_nyqL * (1.0 + ratioXY), vs_scale, vs_bias);
	*(ctr - ii) = (int)d;
	*(ctr + ii) = (int)(-d);
	break;
      }
      
      case 2 :
      {
	double d = DD_SCALE(2. * av_nyqL * (1. + ratioXY), vs_scale, vs_bias);
	*(ctr - ii) = (int)d;
	*(ctr + ii) = (int)(-d);
	break;
      }
      
      case 3 :
      {
	double d = DD_SCALE(av_nyqL * (2. + ratioXY), vs_scale, vs_bias);
	*(ctr - ii) = (int)(-d);
	*(ctr + ii) = (int)d;
	break;
      }
      
      case 4 :
      {
	double d = DD_SCALE(av_nyqL, vs_scale, vs_bias);
	*(ctr - ii) = (int)(-d);
	*(ctr + ii) = (int)d;
	break;
      }
      }
    }

    // vS corresponds to V1 in some of the documentation and vL corresponds
    // to V2

    float vs_rcp_scale = 1.0 / vs_scale;
    d = -2.0 * av_nyqL;

    for (ii = 0; ii < levels; ii++)
    {
      printf("dual prt const: %6.2f for (vs-vl) = %8.4f\n",
	     DD_UNSCALE(vConsts[ii], vs_rcp_scale, vs_bias), d);
      d += 0.5 * av_nyqL;
    }
  }

  for(; data_ptr < end_data_ptr;
      data_ptr++, bnd++, vl_data_ptr++, vs_data_ptr++)
  {
    if (!bnd)
      continue;

    int vS = *vs_data_ptr;
    int vL = *vl_data_ptr;
    double d = ((double)vS - (double)vL) * rcp_half_nyqL + level_bias + d_round;
    int kk = (int)d;
    *data_ptr = ((vS + vL) >> 1) + vConsts[kk];
  }

  return 1;
}

#endif

/* c------------------------------------------------------------------------ */
/* Routine to remove discountinuities (freckles) from the data such as birds */
/* and radio towers by comparing a particular data point to a running        */
/* average that is ahead of the point of interest but switches to a trailing */
/* average once enough good points have been encountered.                    */

#ifdef USE_RADX
#else

int se_flag_freckles(const std::vector< UiCommandToken > &cmd_tokens)
{
  /* #flag-freckles# */

  static struct running_avg_que *raq0;
  static struct running_avg_que *raq1;

  /* boundary mask is set to 1 if the corresponding cell satisfies
   * conditions of the boundaries
   */

//  if (cmd_tokens[0].getCommandText().compare(0, 7, "flag-glitches", 0, 7) == 0)
//    return se_flag_glitches(cmd_tokens);

  // Pull the arguments from the command

  std::string name = cmd_tokens[1].getCommandText();

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();
  if (seds->finish_up)
    return 1;

  seds->modified = YES;
  seds->bad_flag_mask = seds->bad_flag_mask_array;

  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(seds->se_frame);
  
  int field_num = data_info->getParamIndex(name);
  if (field_num  < 0)
  {
    printf("Field to be unfolded: %s not found\n", name.c_str());
    seds->punt = YES;
    return -1;
  }

  short *data_ptr = (short *)data_info->getParamData(field_num);
  int bad = static_cast<int>(data_info->getParamBadDataValue(field_num));

  int scaled_thr = (int)DD_SCALE(seds->freckle_threshold,
				 data_info->getParamScale(field_num),
				 data_info->getParamBias(field_num));
  int navg = seds->freckle_avg_count;
  double rcp_ngts = 1.0 / navg;

  unsigned short *bnd = seds->boundary_mask;
  unsigned short *flag = seds->bad_flag_mask_array;

  // Process the data

  if (seds->process_ray_count == 1)
  {
    // Set up for two running average queues

    raq0 = RAQ_return_queue(navg);
    raq1 = RAQ_return_queue(navg);
  }

  int num_cells = data_info->getClipGate() + 1;
  int nn = navg + 1;

  for (int ndx_ss = 0; ndx_ss < num_cells;)
  {
    // Move the cell index to the first good gate inside the next boundary

    for (; ndx_ss < num_cells && (!bnd[ndx_ss] || data_ptr[ndx_ss] == bad);
	 ndx_ss++);

    // See if we can set up a leading queue

    int mm;
    int jj;
    
    for (mm = 0, jj = ndx_ss; mm < nn && jj < num_cells && bnd[jj]; jj++)
    {
      if (data_ptr[jj] != bad)
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

    struct que_val *qv0 = raq0->at;
    raq0->sum = raq1->sum = 0;

    int ndx_q0;
    
    for (ndx_q0 = ndx_ss, mm = 0; ; ndx_q0++)
    {
      short xx = data_ptr[ndx_q0];
      if (xx != bad)
      {
	// Don't use the first good gate in the avg

	if (!mm++)
	  continue;

	// Put this val in the first queue

	raq0->sum += xx;
	qv0->val = xx;
	qv0 = qv0->next;
	if (mm >= navg + 1)
	  break;
      }
    }

    short ref0 = (short)(raq0->sum * rcp_ngts);
    struct que_val *qv1 = raq1->at;

    // Now loop through the data until we have encountered navg good gates
    // for the trailing average

    for (mm = 0; ndx_q0 < num_cells; ndx_ss++)
    {
      short xx = data_ptr[ndx_ss];
      if(xx == bad)
	continue;

      if (abs((int)(xx - ref0)) > scaled_thr)
      {
	// Flag this gate

	flag[ndx_ss] = 1;
      }
      else
      {
	// Add this point to the trailing average

	raq1->sum += xx;
	qv1->val = xx;
	qv1 = qv1->next;
	if(++mm >= navg)
	  break;
      }

      // Find the next good point for the leading average

      for (ndx_q0++; ndx_q0 < num_cells; ndx_q0++)
      {
	if (!bnd[ndx_q0])
	{
	  // We've gone beyond the boundary

	  out_of_bounds = true;
	  break;
	}

	short xx = data_ptr[ndx_q0];
	if (xx != bad)
	{
	  raq0->sum -= qv0->val;
	  raq0->sum += xx;
	  qv0->val = xx;
	  qv0 = qv0->next;
	  ref0 = (short)(raq0->sum * rcp_ngts);
	  break;
	}
      }

      if (out_of_bounds || ndx_q0 >= num_cells)
	break;
    }

    if (out_of_bounds || ndx_q0 >= num_cells)
    {
      ndx_ss = ndx_q0;
      continue;
    }

    short ref1 = (short)(raq1->sum * rcp_ngts);

    // Else shift to a trailing average

    for (ndx_ss++; ndx_ss < num_cells; ndx_ss++)
    {
      // Check to see if we've gone beyond the boundary

      if (!bnd[ndx_ss])
	break;

      short xx = data_ptr[ndx_ss];
      if (xx == bad)
	continue;

      if (abs((int)(xx - ref1)) > scaled_thr)
      {
	// Flag this gate

	flag[ndx_ss] = 1;
      }
      else
      {
	// Add this point to the trailing average

	raq1->sum -= qv1->val;
	raq1->sum += xx;
	qv1->val = xx;
	qv1 = qv1->next;
	ref1 = (short)(raq1->sum * rcp_ngts);
      }
    }
  }

  return 1;
}

#endif

/* c------------------------------------------------------------------------ */
/* Routine to remove discountinuities (freckles) from the data such as birds */
/* and radio towers by comparing a particular data point to a running        */
/* average that is ahead of the point of interest but switches to a trailing */
/* average once enough good points have been encountered.                    */

#ifdef USE_RADX
#else

int se_flag_glitches(const std::vector< UiCommandToken > &cmd_tokens)
{
  /* #flag-glitches# */

  static int que_size = 0;
  static int *que;
  static int *qctr;

  /* boundary mask is set to 1 if the corresponding cell satisfies
   * conditions of the boundaries
   */

  // Pull the arguments from the command.

  std::string name = cmd_tokens[1].getCommandText();

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();
  if (seds->finish_up)
    return 1;

  seds->modified = YES;
  seds->bad_flag_mask = seds->bad_flag_mask_array;
  
  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(seds->se_frame);
  
  int field_num = data_info->getParamIndex(name);
  if (field_num < 0)
  {
    printf("Field to be deglitched: %s not found\n", name.c_str());
    seds->punt = YES;
    return -1;
  }

  short *data_ptr = (short *)data_info->getParamData(field_num);
  int bad = static_cast<int>(data_info->getParamBadDataValue(field_num));

  int scaled_thr = (int)DD_SCALE(seds->deglitch_threshold,
				 data_info->getParamScale(field_num),
				 data_info->getParamBias(field_num));
  if (seds->deglitch_radius < 1)
    seds->deglitch_radius = 3;
  
  int navg = (seds->deglitch_radius * 2) + 1;
  int half = navg / 2;

  unsigned short *bnd = seds->boundary_mask;
  unsigned short *flag = seds->bad_flag_mask_array;

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

    if (navg > que_size)
    {
      if (que_size != 0)
	free(que);

      que = (int *)malloc( navg * sizeof( int ));
	  que_size = navg;
    }
  }

  int num_cells = data_info->getClipGate() + 1;

  for (int ndx_ss = 0; ndx_ss < num_cells; )
  {
    // Move the cell index to the first gate inside the next boundary

    for ( ; ndx_ss < num_cells && !bnd[ndx_ss]; ndx_ss++);

    // Set up the queue

    int ndx_qend = 0;
    int good_bins = 0;
    int sum = 0;

    // And start looking for the good gates count to equal or exceed the
    // min_bins and the center bin not be bad

    for (int mm = 0; ndx_ss < num_cells && bnd[ndx_ss]; ndx_ss++)
    {
      if (++mm > navg)
      {
	// After the que is full

	int ival = que[ndx_qend];

	if (ival != bad)
	{
	  good_bins--;
	  sum -= ival;
	}
      }

      // Raw data value

      int ival = data_ptr[ndx_ss];
      que[ndx_qend] = ival;
      if (ival != bad)
      {
	sum += ival;
	good_bins++;
      }

      int ndx_qctr = (ndx_qend - half + navg) % que_size;
      qctr = que + ndx_qctr;

      if (good_bins >= min_bins && *qctr != bad)
      {
	// Do a test

	double davg = (double)(sum - *qctr) / (double)(good_bins - 1);
	double diff = FABS(davg - *qctr);
	
	if (diff > scaled_thr)
	{
	  sum -= *qctr;
	  good_bins--;
	  *qctr = bad;
	  flag[ndx_ss - half] = 1; /* flag this gate */
	}
      }
      ++ndx_qend;
      ndx_qend %= que_size;
    }
  }

  return 1;
}

#endif

/* c------------------------------------------------------------------------ */

#ifdef USE_RADX
#else

int se_despeckle(const std::vector< UiCommandToken > &cmd_tokens)
{
  /* #despeckle# */

  // Pull the arguments from the command

  std::string dst_name = cmd_tokens[1].getCommandText();

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();
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

    printf("Field to be thresholded: %s not found\n", dst_name.c_str());
    seds->punt = YES;
    return -1;
  }

  int num_cells = data_info->getClipGate() + 1;

  short *data_ptr = (short *)data_info->getParamData(field_num);
  short *end_data_ptr = data_ptr +num_cells;
  int bad = static_cast<int>(data_info->getParamBadDataValue(field_num));

  int a_speckle = seds->a_speckle;
  unsigned short *bnd = seds->boundary_mask;

  // Do the work

  for (; data_ptr < end_data_ptr; data_ptr++, bnd++)
  {
    // Move to first good gate inside a boundary

    for (; data_ptr < end_data_ptr && (*data_ptr == bad || !(*bnd));
	 data_ptr++, bnd++);

    if (data_ptr >= end_data_ptr)
      break;

    // Now move forward to the next bad flag

    short *tt;
    int nn;
    
    for (tt = data_ptr, nn = 0;
	 data_ptr < end_data_ptr && *bnd && *data_ptr != bad;
	 nn++, data_ptr++, bnd++);

    // Check for a speckle or outside boundary

    if (!(*bnd) || nn > a_speckle)
      continue;

    // Zap speckle

    for (;tt < data_ptr;)
      *tt++ = bad;
  }

  return field_num;
}  

#endif

/* c------------------------------------------------------------------------ */
