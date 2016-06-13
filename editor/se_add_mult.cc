/* 	$Id$	 */

#include <glib.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include <dd_math.h>

#include <DataInfo.hh>
#include <DataManager.hh>
#include "se_add_mult.hh"
#include "se_catch_all.hh"
#include "se_utils.hh"
#include <solo_editor_structs.h>

extern GString *gs_complaints;

/* c------------------------------------------------------------------------ */

#ifdef USE_RADX
#else

int se_absolute_value(const std::vector< UiCommandToken > &cmd_tokens)
{
  /* #absolute-value# */

  // Pull the arguments from the command

  std::string name = cmd_tokens[1].getCommandText();

  struct solo_edit_stuff *seds = return_sed_stuff();
  if (seds->finish_up)
    return 1;

  seds->modified = YES;

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(seds->se_frame);
  
  // Find the field 

  int field_num = data_info->getParamIndex(name);
  
  if (field_num < 0)
  {
    // Field not found

    g_string_sprintfa(gs_complaints, "Field to be abs()'d: %s not found\n",
		      name.c_str());
    seds->punt = YES;
    return -1;
  }

  // Loop through the data

  short *data_ptr = (short *)data_info->getParamData(field_num);
  int num_cells = data_info->getClipGate() + 1;
  short *end_data_ptr = data_ptr + num_cells;
  int bad = static_cast<int>(data_info->getParamBadDataValue(field_num));

  unsigned short *bnd = seds->boundary_mask;
  
  for (; data_ptr < end_data_ptr; data_ptr++, bnd++)
  {
    if (!(*bnd) || *data_ptr == bad)
      continue;
    *data_ptr = abs((int)(*data_ptr));
  }

  return field_num;
}  

#endif

/* c------------------------------------------------------------------------ */

#ifdef USE_RADX
#else

int se_assign_value(const std::vector< UiCommandToken > &cmd_tokens)
{
  /* #assign-value# */
  /* flagged-assign */

  // Pull the arguments from the command

  float f_const = cmd_tokens[1].getFloatParam();
  std::string name = cmd_tokens[2].getCommandText();

  struct solo_edit_stuff *seds = return_sed_stuff();
  if (seds->finish_up)
    return 1;

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(seds->se_frame);
  
  seds->bad_flag_mask = seds->bad_flag_mask_array;

  // Find the field 

  int field_num = data_info->getParamIndex(name);
  
  if (field_num < 0)
  {
    // Field not found

    g_string_sprintfa(gs_complaints, "Field to be assigned: %s not found\n",
		      name.c_str());
    seds->punt = YES;
    return -1;
  }

  seds->modified = YES;

  short *data_ptr = (short *)data_info->getParamData(field_num);
  int scaled_const = data_info->scaleValue(f_const, field_num);
  short *end_data_ptr = data_ptr + data_info->getNumCells();

  unsigned short *bnd = seds->boundary_mask;

  // Do it!

  if (cmd_tokens[0].getCommandText().find("flag") != std::string::npos)
  {
    // flagged-assign

    unsigned short *flag = seds->bad_flag_mask_array;

    for (; data_ptr < end_data_ptr; data_ptr++, bnd++, flag++)
    {
      if (*bnd && *flag)
      {
	*data_ptr = scaled_const;
      }
    }
  }
  else
  {
    for (; data_ptr < end_data_ptr; data_ptr++, bnd++)
    {
      if (*bnd)
	*data_ptr = scaled_const;
    }
  }
  
  return 1;
}  

#endif

/* c------------------------------------------------------------------------ */

#ifdef USE_RADX
#else

int se_add_const(const std::vector< UiCommandToken > &cmd_tokens)
{
  /* #add-value# */

  // Pull the arguments from the command

  float f_const = cmd_tokens[1].getFloatParam();
  std::string src_name = cmd_tokens[2].getCommandText();
  std::string dst_name = cmd_tokens[3].getCommandText();

  struct solo_edit_stuff *seds = return_sed_stuff();
  if (seds->finish_up)
    return 1;

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

  seds->modified = YES;

  short *src_data_ptr = (short *)data_info->getParamData(src_field_num);
  int num_cells = data_info->getClipGate() + 1;
  short *end_srt_data_ptr = src_data_ptr + num_cells;
  short *dst_data_ptr = (short *)data_info->getParamData(dst_field_num);
  int bad = static_cast<int>(data_info->getParamBadDataValue(src_field_num));
  int scaled_const =
    data_info->scaleValue(f_const, src_field_num);
  
  // Here's where we finally do the work

  unsigned short *bnd = seds->boundary_mask;
  
  for (; src_data_ptr < end_srt_data_ptr; src_data_ptr++, dst_data_ptr++, bnd++)
  {
    if (*bnd)
      *dst_data_ptr =
	(*src_data_ptr == bad) ? bad : *src_data_ptr + scaled_const;
  }

  return 1;
}  

#endif

/* c------------------------------------------------------------------------ */

#ifdef USE_RADX
#else

int se_add_fields(const std::vector< UiCommandToken > &cmd_tokens)
{
  /*
   * #add-field#
   * #subtract#
   * #mult-fields#
   * #merge-fields#
   */

  // Pull the arguments from the command

  std::string add_name = cmd_tokens[1].getCommandText();
  std::string src_name = cmd_tokens[2].getCommandText();
  std::string dst_name = cmd_tokens[3].getCommandText();

  struct solo_edit_stuff *seds = return_sed_stuff();
  if (seds->finish_up)
    return 1;

  seds->modified = YES;

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(seds->se_frame);
  
  // Find the add field

  int add_field_num = data_info->getParamIndex(add_name);
  
  if (add_field_num < 0)
  {
    g_string_sprintfa(gs_complaints,
		      "First parameter %s not found for add/sub\n", add_name.c_str());
    seds->punt = YES;
    return -1;
  }

  // Find the source field
  // NOTE:  I think we can get rid of this code.  I think all of this is done
  // again in se_establish_field below.  Verify and then remove.

  int src_field_num = data_info->getParamIndex(src_name);
  
  if (src_field_num < 0)
  {	
    g_string_sprintfa(gs_complaints,
		      "Second parameter %s not found for add/sub\n", src_name.c_str());
    seds->punt = YES;
    return -1;
  }

  // Establish the destination field.  This will create the field if it doesn't
  // already exist

  int ii;
  int dst_field_num;
  
  if ((ii = data_info->establishField(src_name, dst_name,
				     src_field_num, dst_field_num)) < 0)
  {
    seds->punt = YES;
    return ii;
  }

  // Get all of the data pointers and information

  int num_cells = data_info->getClipGate() + 1;

  short *add_data_ptr = (short *)data_info->getParamData(add_field_num);
  float add_scale = data_info->getParamScale(add_field_num);
  float add_rcp_scale = 1.0 / add_scale;
  float add_bias = data_info->getParamBias(add_field_num);

  short *src_data_ptr = (short *)data_info->getParamData(src_field_num);
  short *end_src_data_ptr = src_data_ptr + num_cells;
  int src_bad = static_cast<int>(data_info->getParamBadDataValue(src_field_num));
  float src_scale = data_info->getParamScale(src_field_num);
  float src_rcp_scale = 1.0 / src_scale;
  float src_bias = data_info->getParamBias(src_field_num);

  short *dst_data_ptr = (short *)data_info->getParamData(dst_field_num);
  float dst_scale = data_info->getParamScale(dst_field_num);
  float dst_bias = data_info->getParamBias(dst_field_num);

  // See if we will need to rescale the data

  bool rescale = false;
  
  if (add_scale != src_scale ||
      src_scale != dst_scale ||
      add_bias != src_bias ||
      src_bias != dst_bias)
  {
    rescale = true;
  }

  // Loop through the data

  unsigned short *bnd = seds->boundary_mask;

  if (cmd_tokens[0].getCommandText().compare(0, 3, "add", 0, 3) == 0)
  {
    // For "add" we do a substitude when only one of the fields is flagged bad

    for (; src_data_ptr < end_src_data_ptr;
	 add_data_ptr++, src_data_ptr++, dst_data_ptr++, bnd++)
    {
      if (*bnd)
      {
	if (rescale)
	{
	  // Nightmare time!

	  if (*add_data_ptr != src_bad)
	  {
	    // *add_data_ptr ok

	    float x =
	      DD_UNSCALE((float)(*add_data_ptr), add_rcp_scale, add_bias);

	    if (*src_data_ptr != src_bad)
	    {
	      // *src_data_ptr ok

	      float y =
		DD_UNSCALE((float)(*src_data_ptr), src_rcp_scale, src_bias);
	      *dst_data_ptr = (short)DD_SCALE(x + y, dst_scale, dst_bias);
	    }
	    else
	    {
	      // src_data_ptr !ok

	      *dst_data_ptr = (short)DD_SCALE(x, dst_scale, dst_bias);
	    }
	  }
	  else if (*src_data_ptr != src_bad)
	  {
	    // *src_data_ptr ok

	    float y =
	      DD_UNSCALE((float)(*src_data_ptr), src_rcp_scale, src_bias);
	    *dst_data_ptr = (short)DD_SCALE(y, dst_scale, dst_bias);
	  }
	  else
	  {
	    *dst_data_ptr = src_bad;
	  }
	}
	else
	{
	  // No rescaling necessary

	  if (*add_data_ptr != src_bad)
	  {
	    // *add_data_ptr ok

	    if (*src_data_ptr != src_bad)
	    {
	      *dst_data_ptr = *add_data_ptr + *src_data_ptr;
	    }
	    else
	    {
	      *dst_data_ptr = *add_data_ptr;
	    }
	  }
	  else if (*src_data_ptr != src_bad)
	  {
	    *dst_data_ptr = *src_data_ptr;
	  }
	  else
	  {
	    *dst_data_ptr = src_bad;
	  }
	} /* !rescale */
      } /* end bnd */
    }	/* end for */
  } /* end add */
  else if (cmd_tokens[0].getCommandText().compare(0, 3, "merge", 0, 3) == 0)
  {
    // In this case the first field "add_name" represented by "add_data_ptr"
    // is considered the dominant field over the second field "src_name"
    // represented by "src_data_ptr" and if (*add_data_ptr) does not contain
    // a bad flag it is plunked into the destination field "dst_data_ptr"
    // otherwise (*src_data_ptr) is plunked into the destination field

    for (; src_data_ptr < end_src_data_ptr;
	 add_data_ptr++, src_data_ptr++, dst_data_ptr++, bnd++)
    {
      if (*bnd)
      {
	if (rescale)
	{
	  // Nightmare time!

	  if (*add_data_ptr != src_bad)
	  {
	    float x =
	      DD_UNSCALE((float)(*add_data_ptr), add_rcp_scale, add_bias);
	    *dst_data_ptr = (short)DD_SCALE(x, dst_scale, dst_bias);
	  } /* *add_data_ptr ok */
	  else if (*src_data_ptr != src_bad)
	  {
	    float y =
	      DD_UNSCALE((float)(*src_data_ptr), src_rcp_scale, src_bias);
	    *dst_data_ptr = (short)DD_SCALE(y, dst_scale, dst_bias);
	  }	/* *src_data_ptr ok */
	  else
	  {
	    *dst_data_ptr = src_bad;
	  }
	} /* rescale */
	else if (*add_data_ptr != src_bad)
	{
	  *dst_data_ptr = *add_data_ptr;
	}
	else
	{
	  *dst_data_ptr = *src_data_ptr;
	} /* !rescale */
      } /* end bnd */
    }	/* end for */
  } /* end "merge" */
  else if (cmd_tokens[0].getCommandText().compare(0, 3, "mult-fields", 0, 3) == 0)
  {
    // Multiply two fields together
	
    for (; src_data_ptr < end_src_data_ptr;
	 add_data_ptr++, src_data_ptr++, dst_data_ptr++, bnd++)
    {
      if (*bnd)
      {
	if (*add_data_ptr != src_bad && *src_data_ptr != src_bad)
	{
	  float x =
	    DD_UNSCALE((float)(*add_data_ptr), add_rcp_scale, add_bias);
	  float y =
	    DD_UNSCALE((float)(*src_data_ptr), src_rcp_scale, src_bias);
	  *dst_data_ptr = (short)DD_SCALE(x * y, dst_scale, dst_bias);
	}
	else
	{
	  *dst_data_ptr = src_bad;
	}
      } /* end bnd */
    } /* end for */
  } /* end "mult-fields" */
  else
  {
    // Subtract

    // For subtract we only subtract when both fields are not flagged bad

    for (; src_data_ptr < end_src_data_ptr;
	 add_data_ptr++, src_data_ptr++, dst_data_ptr++, bnd++)
    {
      if (*bnd)
      {
	if (rescale)
	{	
	  if (*add_data_ptr != src_bad && *src_data_ptr != src_bad)
	  {
	    float x =
	      DD_UNSCALE((float)(*add_data_ptr), add_rcp_scale, add_bias);
	    float y =
	      DD_UNSCALE((float)(*src_data_ptr), src_rcp_scale, src_bias);
	    *dst_data_ptr = (short)DD_SCALE(y - x, dst_scale, dst_bias);
	  }
	  else
	  {
	    *dst_data_ptr = src_bad;
	  }
	} /* rescale */
	else if (*add_data_ptr != src_bad && *src_data_ptr != src_bad)
	{
	  *dst_data_ptr = *src_data_ptr - *add_data_ptr;
	} 
	else
	{
	  *dst_data_ptr = src_bad;
	}
      } /* end bnd */
    }	/* end for */
  } /* end "subtract" */

  return 1;
}  

#endif

/* c------------------------------------------------------------------------ */

#ifdef USE_RADX
#else

int se_mult_const(const std::vector< UiCommandToken > &cmd_tokens)
{
  /* #multiply#
   * #rain-rate#
   * #exponentiate#
   */

  // Pull the arguments from the command

  std::string src_name = cmd_tokens[1].getCommandText();
  float f_const = cmd_tokens[2].getFloatParam();
  std::string dst_name = cmd_tokens[3].getCommandText();

  // NOTE:  Why do we need both d_const and f_const?  Can we get rid of one
  // of these?

  double d_const = f_const;
    
  struct solo_edit_stuff *seds = return_sed_stuff();
  if (seds->finish_up)
    return 1;

  seds->modified = YES;

  // Get the information about the fields

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

  int num_cells = data_info->getClipGate() + 1;
  short *src_data_ptr = (short *)data_info->getParamData(src_field_num);
  short *end_src_data_ptr = src_data_ptr + num_cells;
  int src_bad = static_cast<int>(data_info->getParamBadDataValue(src_field_num));
  double src_scale = data_info->getParamScale(src_field_num);
  double src_rcp_scale = 1.0 / src_scale;
  double src_bias = data_info->getParamBias(src_field_num);

  short *dst_data_ptr = (short *)data_info->getParamData(dst_field_num);
  int dst_bad = static_cast<int>(data_info->getParamBadDataValue(dst_field_num));
  double dst_scale = data_info->getParamScale(dst_field_num);
  double dst_bias = data_info->getParamBias(dst_field_num);

  bool rescale = false;
  
  if (src_scale != dst_scale ||
      src_bias != dst_bias)
  {
    rescale = true;
  }

  // Here's where we finally do the work

  unsigned short *bnd = seds->boundary_mask;

  if (cmd_tokens[0].getCommandText().compare(0, 4, "rain-rate", 0, 4) == 0 ||
      cmd_tokens[0].getCommandText().compare(0, 4, "exponentiate", 0, 4) == 0)
  {
    if (cmd_tokens[0].getCommandText().compare(0, 4, "rain-rate", 0, 4) == 0)
    {
      double A = 0.003333333;
      double X = 0.7142857;

      if (d_const != 0.0)
      {
	X = d_const;
      }

      for (; src_data_ptr < end_src_data_ptr;
	   src_data_ptr++, dst_data_ptr++, bnd++)
      {
	if (*bnd)
	{
	  if (*src_data_ptr == src_bad)
	  {
	    *dst_data_ptr = dst_bad;
	  }
	  else
	  {
	    double src_value =
	      DD_UNSCALE((double)(*src_data_ptr), src_rcp_scale, src_bias);
	    double d = A * pow((double)10.0, (double)(0.1 * src_value * X));
	    *dst_data_ptr = (short)DD_SCALE(d, dst_scale, dst_bias);
	  }
	}
      }
    }
    else
    {
      // Exponentiate

      for (; src_data_ptr < end_src_data_ptr;
	   src_data_ptr++, dst_data_ptr++, bnd++)
      {
	if (*bnd)
	{
	  if (*src_data_ptr == src_bad)
	  {
	    *dst_data_ptr = dst_bad;
	  }
	  else
	  {
	    double src_value =
	      DD_UNSCALE((double)(*src_data_ptr), src_rcp_scale, src_bias);
	    double d = pow(src_value, d_const);
	    *dst_data_ptr = (short)DD_SCALE(d, dst_scale, dst_bias);
	  }
	}
      }
    }
  }
  else
  {
    if (rescale)
    {
      for (; src_data_ptr < end_src_data_ptr;
	   src_data_ptr++, dst_data_ptr++, bnd++)
      {
	if (*bnd)
	{
	  if (*src_data_ptr == src_bad)
	  {
	    *dst_data_ptr = dst_bad;
	    continue;
	  }
	  double xx =
	    DD_UNSCALE((float)(*src_data_ptr), src_rcp_scale, src_bias);
	  xx *= f_const;
	  *dst_data_ptr = (short)DD_SCALE(xx, dst_scale, dst_bias);
	}
      }
    }
    else
    {
      for (; src_data_ptr < end_src_data_ptr;
	   src_data_ptr++, dst_data_ptr++, bnd++)
      {
	if (*bnd)
	{
	  *dst_data_ptr =
	    (short)((*src_data_ptr == src_bad) ? src_bad :
		    *src_data_ptr * f_const);
	}
      }
    }
  }

  return 1;
}

#endif

/* c------------------------------------------------------------------------ */

