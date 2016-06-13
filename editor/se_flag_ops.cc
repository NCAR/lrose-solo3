/* 	$Id$	 */

#include <glib.h>
#include <string.h>

#include <DataInfo.hh>
#include <DataManager.hh>
#include "se_flag_ops.hh"
#include "se_utils.hh"
#include <solo_editor_structs.h>
#include <solo2.hh>

extern GString *gs_complaints;

/* c------------------------------------------------------------------------ */

#ifdef USE_RADX
#else

int se_assert_bad_flags(const std::vector< UiCommandToken > &cmd_tokens)
{
  /* #assert-bad-flags# */

  // Pull the arguments from the command

  std::string dst_name = cmd_tokens[1].getCommandText();

  //  Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();
  if (seds->finish_up)
    return 1;

  seds->modified = YES;

  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(seds->se_frame);
  
  int num_cells = data_info->getClipGate() + 1;

  int field_num = data_info->getParamIndex(dst_name);
  
  if (field_num < 0)
  {
    printf("Field to be asserted: %s not found\n", dst_name.c_str());
    seds->punt = YES;
    return -1;
  }

  short *data_ptr = (short *)data_info->getParamData(field_num);
  short *end_data_ptr = data_ptr + num_cells;
  int bad = static_cast<int>(data_info->getParamBadDataValue(field_num));

  unsigned short *bnd = seds->boundary_mask;
  unsigned short *flag = seds->bad_flag_mask;

  // Loop through the data

  for (; data_ptr < end_data_ptr; data_ptr++, bnd++, flag++)
  {
    if (*bnd && *flag)
      *data_ptr = bad;
  }

  return field_num;
}  

#endif

/* c------------------------------------------------------------------------ */

#ifdef USE_RADX
#else

int se_flagged_add(const std::vector< UiCommandToken > &cmd_tokens)
{
  /* #flagged-add#
   * #flagged-multiply#
   */

  // Pull the arguments from the command

  float f_const = cmd_tokens[1].getFloatParam();
  std::string name = cmd_tokens[2].getCommandText();

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();
  if (seds->finish_up)
    return 1;

  seds->bad_flag_mask = seds->bad_flag_mask_array;
  seds->modified = YES;

  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(seds->se_frame);
  
  int field_num = data_info->getParamIndex(name);

  if (field_num < 0)
  {
    // Field not found
    
    printf("Flagged add field: %s not found\n", name.c_str());
    seds->punt = YES;
    return -1;
  }

  int num_cells = data_info->getClipGate() + 1;
  short *data_ptr = (short *)data_info->getParamData(field_num);
  short *end_data_ptr = data_ptr + num_cells;

  int scaled_const = data_info->scaleValue(f_const, field_num);
  int bad = static_cast<int>(data_info->getParamBadDataValue(field_num));

  unsigned short *bnd = seds->boundary_mask;
  unsigned short *flag = seds->bad_flag_mask_array;

  // Loop through the data

  if (cmd_tokens[0].getCommandText().find("d-m") != std::string::npos)
  {
    // Multiply

    for (; data_ptr < end_data_ptr; data_ptr++, bnd++, flag++)
    {
      if (*bnd && *data_ptr != bad && *flag)
	*data_ptr *= (short)f_const;
    }
  }
  else
  {
    // Add

    for (; data_ptr < end_data_ptr; data_ptr++, bnd++, flag++)
    {
      if (*bnd && *data_ptr != bad && *flag)
	*data_ptr += scaled_const;
    }
  }

  return 1;
}

#endif

/* c------------------------------------------------------------------------ */

#ifdef USE_RADX
#else

int se_bad_flags_logic(const std::vector< UiCommandToken > &cmd_tokens)
{
  /*
   * #and-bad-flags#
   * #or-bad-flags#
   * #xor-bad-flags#
   */

  // Pull the arguments out of the command

  std::string name = cmd_tokens[1].getCommandText();
  std::string where = cmd_tokens[2].getCommandText();
  float f_thr1 = cmd_tokens[3].getFloatParam();
  float f_thr2 = 0.0;
  if (cmd_tokens[4].getTokenType() != UiCommandToken::UTT_END)
    f_thr2 = cmd_tokens[4].getFloatParam();

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
    // Field not found

    printf("Bad flag logic field: %s not found\n", name.c_str());
    seds->punt = YES;
    return -1;
  }

  int num_cells = data_info->getClipGate() + 1;
  short *data_ptr = (short *)data_info->getParamData(field_num);
  short *end_data_ptr = data_ptr + num_cells;

  int scaled_thr1 = data_info->scaleValue(f_thr1, field_num);
  int scaled_thr2 = data_info->scaleValue(f_thr2, field_num);
  int bad = static_cast<int>(data_info->getParamBadDataValue(field_num));

  unsigned short *bnd = seds->boundary_mask;
  unsigned short *flag = seds->bad_flag_mask_array;

  // Loop through the data

  if (where.compare(0, 3, "below", 0, 3) == 0)
  {
    if (cmd_tokens[0].getCommandText().compare(0, 3, "and", 0, 3) == 0)
    {
      for (; data_ptr < end_data_ptr; data_ptr++, bnd++, flag++)
      {
	if (!(*bnd))
	  continue;

	if (*data_ptr == bad)
	{
	  *flag = 0;
	}
	else
	{
	  *flag &= *data_ptr < scaled_thr1;
	}
      }
    }
    else if (cmd_tokens[0].getCommandText().compare(0, 3, "xor", 0, 3) == 0)
    {
      for (; data_ptr < end_data_ptr; data_ptr++, bnd++, flag++)
      {
	if (!(*bnd))
	  continue;

	if (*data_ptr != bad)
	  *flag ^= *data_ptr < scaled_thr1;
      }
    }
    else
    {
      // or

      for (; data_ptr < end_data_ptr; data_ptr++, bnd++, flag++)
      {
	if (!(*bnd))
	  continue;

	if (*data_ptr != bad)
	  *flag |= *data_ptr < scaled_thr1;
      }
    }
  }
  else if (where.compare(0, 3, "above", 0, 3) == 0)
  {
    if (cmd_tokens[0].getCommandText().compare(0, 3, "and", 0, 3) == 0)
    {
      for (; data_ptr < end_data_ptr; data_ptr++, bnd++, flag++)
      {
	if (!(*bnd))
	  continue;

	if (*data_ptr == bad)
	  *flag = 0;
	else
	  *flag &= *data_ptr > scaled_thr1;
      }
    }
    else if (cmd_tokens[0].getCommandText().compare(0, 3, "xor", 0, 3) == 0)
    {
      for (; data_ptr < end_data_ptr; data_ptr++, bnd++, flag++)
      {
	if (!(*bnd))
	  continue;

	if (*data_ptr != bad)
	  *flag ^= *data_ptr > scaled_thr1;
      }
    }
    else
    {
      // or

      for (; data_ptr < end_data_ptr; data_ptr++, bnd++, flag++)
      {
	if (!(*bnd))
	  continue;

	if (*data_ptr != bad)
	  *flag |= *data_ptr > scaled_thr1;
      }
    }
  }
  else
  {
    // between

    if (cmd_tokens[0].getCommandText().compare(0, 3, "and", 0, 3) == 0)
    {
      for (; data_ptr < end_data_ptr; data_ptr++, bnd++, flag++)
      {
	if (!(*bnd))
	  continue;

	if (*data_ptr == bad)
	  *flag = 0;
	else
	  *flag &= *data_ptr >= scaled_thr1 && *data_ptr <= scaled_thr2;
      }
    }
    else if (cmd_tokens[0].getCommandText().compare(0, 3, "xor", 0, 3) == 0)
    {
      for (; data_ptr < end_data_ptr; data_ptr++, bnd++, flag++)
      {
	if (!(*bnd))
	  continue;

	if (*data_ptr != bad)
	  *flag ^= *data_ptr >= scaled_thr1 && *data_ptr <= scaled_thr2;
      }
    }
    else
    {
      // or

      for (; data_ptr < end_data_ptr; data_ptr++, bnd++, flag++)
      {
	if (!(*bnd))
	  continue;

	if (*data_ptr != bad)
	  *flag |= *data_ptr >= scaled_thr1 && *data_ptr <= scaled_thr2;
      }
    }
  }

  return 1;
}

#endif

/* c------------------------------------------------------------------------ */

#ifdef USE_RADX
#else

int se_clear_bad_flags(const std::vector< UiCommandToken > &cmd_tokens)
{
  /* #clear-bad-flags#
   * #complement-bad-flags#
   */

  // This command doesn't have any arguments

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  if (seds->finish_up)
    return 1;

  // Do it

  if (cmd_tokens[0].getCommandText().compare(0, 3, "complement-bad-flags",
					     0, 3) == 0)
  {
    unsigned short *flag = seds->bad_flag_mask_array;
    int num_cells = seds->max_gates;

    for (; num_cells--; flag++)
    {
      *flag = *flag ? 0 : 1;
    }
  }
  else
  {
    se_do_clear_bad_flags_array(0);
  }

  return 1;
}  

#endif

/* c------------------------------------------------------------------------ */
/* Creates a bad flag mask corresponding to the bad flagged gates in the     */
/*  test field                                                               */

#ifdef USE_RADX
#else

int se_copy_bad_flags(const std::vector< UiCommandToken > &cmd_tokens)
{
  /* #copy-bad-flags# */

  // Pull the arguments out of the command

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
    // thr field not found

    printf("Copy bad flags field: %s not found\n", name.c_str());
    seds->punt = YES;
    return -1;
  }

  int num_cells = data_info->getClipGate() + 1;

  short *data_ptr = (short *)data_info->getParamData(field_num);
  short *end_data_ptr = data_ptr +num_cells;
  int bad = static_cast<int>(data_info->getParamBadDataValue(field_num));

  unsigned short *bnd = seds->boundary_mask;
  unsigned short *flag = seds->bad_flag_mask_array;

  // Loop through the data

  for (; data_ptr < end_data_ptr; data_ptr++, bnd++, flag++)
  {
    if (!(*bnd))
      continue;

    *flag = (*data_ptr == bad ? 1 : 0);
  }

  return field_num;
}  

#endif

/* c------------------------------------------------------------------------ */

void se_do_clear_bad_flags_array (int nn)
{
  struct solo_edit_stuff *seds= return_sed_stuff();

  memset(seds->bad_flag_mask_array, 0,
	 (nn > 0 ? nn : seds->max_gates) * sizeof(*seds->bad_flag_mask_array));
}

/* c------------------------------------------------------------------------ */

#ifdef USE_RADX
#else

int se_set_bad_flags(const std::vector< UiCommandToken > &cmd_tokens)
{
  /* #set-bad-flags# */

  // Pull the arguments out of the command

  std::string name = cmd_tokens[1].getCommandText();
  std::string where = cmd_tokens[2].getCommandText();
  float f_thr1 = cmd_tokens[3].getFloatParam();
  float f_thr2 = 0.0;
  if (cmd_tokens[4].getTokenType() != UiCommandToken::UTT_END)
    f_thr2 = cmd_tokens[4].getFloatParam();

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
    // thr field not found

    printf("Set bad flags field: %s not found\n", name.c_str());
    seds->punt = YES;
    return -1;
  }

  int num_cells = data_info->getClipGate() + 1;

  short *data_ptr = (short *)data_info->getParamData(field_num);
  short *end_data_ptr = data_ptr +num_cells;

  int scaled_thr1 = data_info->scaleValue(f_thr1, field_num);
  int scaled_thr2 = data_info->scaleValue(f_thr2, field_num);

  int bad = static_cast<int>(data_info->getParamBadDataValue(field_num));

  se_do_clear_bad_flags_array(num_cells);

  unsigned short *bnd = seds->boundary_mask;
  unsigned short *flag = seds->bad_flag_mask_array;

  // Loop through the data

  if (where.compare(0, 3, "below", 0, 3) == 0)
  {
    for (; data_ptr < end_data_ptr; data_ptr++, bnd++, flag++)
    {
      if (!(*bnd) || *data_ptr == bad)
	continue;

      if (*data_ptr < scaled_thr1)
	*flag = 1;
    }
  }
  else if (where.compare(0, 3, "above", 0, 3) == 0)
  {
    for (; data_ptr < end_data_ptr; data_ptr++, bnd++, flag++)
    {
      if (!(*bnd) || *data_ptr == bad)
	continue;

      if(*data_ptr > scaled_thr1)
	*flag = 1;
    }
  }
  else
  {
    // between

    for (; data_ptr < end_data_ptr; data_ptr++, bnd++, flag++)
    {
      if (!(*bnd) || *data_ptr == bad)
	continue;

      if (*data_ptr >= scaled_thr1 && *data_ptr <= scaled_thr2)
	*flag = 1;
    }
  }

  return field_num;
}  

#endif

/* c------------------------------------------------------------------------ */

