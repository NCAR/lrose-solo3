/* $Id$ */

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <dd_crackers.hh>
#include <DataManager.hh>
#include <PaletteManager.hh>
#include <PointInSpace.hh>
#include <se_utils.hh>
#include <sed_shared_structs.h>
#include "sii_colors_stuff.hh"
#include "sii_config_stuff.hh"
#include "sii_param_widgets.hh"
#include "sii_utils.hh"
#include <solo_defs.h>
#include <solo_window_structs.h>
#include "solo2.hh"
#include "soloii.h"
#include "soloii.hh"
#include "sp_accepts.hh"
#include "sp_basics.hh"
#include "sp_save_frms.hh"

/* c------------------------------------------------------------------------ */

struct solo_sii_extras {
    char name_struct[4];        /* "SXTR" */
    int32_t sizeof_struct;
    int32_t window_num;
    char attributes[256];
};
static struct solo_sii_extras *siiex, *sse = NULL;

/* c------------------------------------------------------------------------ */

int solo_absorb_window_info(const std::string &dir, const std::string &fname,
                            const int ignore_swpfi_info)
{
  // Set all of the windows inactive

  struct solo_perusal_info *spi = solo_return_winfo_ptr();
  for (int kk=0; kk < SOLO_MAX_WINDOWS; ++kk)
    spi->active_windows[kk] = NO;

  // Make sure the correct type of file was given, based on the file name.
  // The window information files must start with "wds".

  if (fname.compare(0, 3, "wds") != 0)
  {
    // Trashed file name!

    sii_message("**** Illegal file name: " + fname + " ****\n");

    return -1;
  }

  // Read the file

  std::string file_path = dir + "/" + fname;
  
  struct stat file_stat;
  
  if (stat(file_path.c_str(), &file_stat) != 0)
  {
    sii_message("Unable to stat frame state file " + file_path + "\n");
    return -1;
  }
  
  FILE *input_file;
  
  if ((input_file = fopen(file_path.c_str(), "r")) == 0)
  {
    sii_message("Unable to open frame state file " + file_path + "\n");
    return -1;
  }
  
  std::size_t file_size = file_stat.st_size;
  char *input_buffer = new char[file_size];
  
  if (fread(input_buffer, sizeof(char), file_size, input_file) != file_size)
  {
    sii_message("Error reading frame state file " + file_path + "\n");
    return -1;
  }

  fclose(input_file);

  char *buf_ptr = input_buffer;
  char *end_ptr = input_buffer + file_size;

  // Reset the global solo3 configuration cells

  sii_reset_config_cells();
  struct solo_generic_window_struct gws;
  
  // Count the number of frames

  int frame_count = 0;
  bool gottaSwap = false;
  
  for (buf_ptr = input_buffer; ; )
  {
    memcpy(&gws, buf_ptr, sizeof(struct solo_generic_window_struct));
    u_int32_t gdsos = gws.sizeof_struct;
    u_int32_t gww = gws.window_num;

    // See if we need to swap the buffer as we use it

    if (gdsos > MAX_REC_SIZE)
    {
      gottaSwap = true;
      swack4((char *)&gws.sizeof_struct, (char *)&gdsos);
      swack4((char *)&gws.window_num, (char *)&gww);
    }

    // Sweep file info

    if (strncmp(buf_ptr, "SSFI", 4) == 0)
    {
      frame_count++;
    }
    buf_ptr += gdsos; 

    if (buf_ptr >= end_ptr)
      break;

    if (frame_count > 1000)
      break;
  } /* endfor - count the number of frames */

  // Read in window info

  int count = 0;
  int config_count = 0;
  
  sii_table_parameters stp;
        
  for (buf_ptr = input_buffer; ; )
  {
    count++;
    memcpy(&gws, buf_ptr, sizeof(struct solo_generic_window_struct));
    u_int32_t record_size = gws.sizeof_struct;
    u_int32_t gww = gws.window_num;

    // See if we need to swap the records.

    if (record_size > MAX_REC_SIZE)
    {
      gottaSwap = true;
      swack4((char *)&gws.sizeof_struct, (char *)&record_size);
      swack4((char *)&gws.window_num, (char *)&gww);
    }

    if (strncmp(buf_ptr, "SPAL", 4) == 0)
    {
      // Read the color palette info

      PaletteManager::getInstance()->readPalette(buf_ptr, gottaSwap);
    }
    else if (strncmp(buf_ptr, "SCTB", 4) == 0)
    {
      // Ascii color table

      int nn = record_size - 8;
      char *cbuf = (char *)malloc(nn + 1);
      char str[256];
      std::vector< std::string > tokens;
      
      memcpy(cbuf, buf_ptr + 8, nn);
      strncpy(str, buf_ptr + 8, 80);
      cbuf[nn] = '\0';
      str[80] = '\0';
      tokenize(str, tokens); /* color table name is second token */
      ColorTableManager::getInstance()->putAsciiColorTable(tokens[1].c_str(),
                                                           cbuf);
    }
    else if(strncmp(buf_ptr, "SSFI", 4) == 0)
    {
      // Sweep file info

      if (!ignore_swpfi_info)
      {
        WW_PTR wwptr = solo_return_wwptr(gww);
        bool old_sos = (record_size != sizeof(struct solo_sweep_file));

        struct solo_sweep_file0 ssfi0;
        struct res_solo_sweep_file0 res_ssfi0;
        
        if (gottaSwap)
        {
          if (old_sos)
          {
            sp_crack_ssfi0(buf_ptr, (char *)&ssfi0, (int)0);
          }
          else
          {
            sp_crack_ssfi(buf_ptr, (char *)wwptr->sweep, (int)0);
          }
        }
        else
        {
          if (old_sos)
          {
            memcpy (&ssfi0, buf_ptr, sizeof(ssfi0));
          }
          else
          {
            memcpy(wwptr->sweep, buf_ptr, sizeof(*wwptr->sweep));
          }
        }

        if (old_sos)
        {
          memcpy(wwptr->sweep, &ssfi0, sizeof(ssfi0));
          memcpy(&wwptr->sweep->sweep_count, &ssfi0.sweep_count,
                 sizeof (res_ssfi0));
          for (int jj = frame_count; jj < SOLO_MAX_WINDOWS; jj++)
          {
            wwptr->sweep->linked_windows[jj] = 1;
          }
        }

        wwptr->sweep->sizeof_struct = sizeof(struct solo_sweep_file);
        wwptr->sweep->changed = YES;
        DataManager::getInstance()->setRescanUrgent(gww);
        wwptr->d_sweepfile_time_stamp = wwptr->sweep->start_time;
      }
    }
    else if (strncmp(buf_ptr, "SPTL", 4) == 0)
    {
      // Lock step

      WW_PTR wwptr = solo_return_wwptr(gww);
      bool old_sos = (record_size != sizeof(struct solo_plot_lock));
      
      struct solo_plot_lock0 sptl0;
      
      if (gottaSwap)
      {
        if (old_sos)
        {
          sp_crack_sptl0(buf_ptr, (char *)&sptl0, (int)0);
        }
        else
        {
          sp_crack_sptl(buf_ptr, (char *)wwptr->lock, (int)0);
        }
      }
      else
      {
        if (old_sos)
        {
          memcpy (&sptl0, buf_ptr, sizeof (sptl0)); }
        else
        {
          memcpy(wwptr->lock, buf_ptr, sizeof(*wwptr->lock));
        }
      }

      if (old_sos)
      {
        for (int jj = frame_count; jj < SOLO_MAX_WINDOWS; jj++)
        {
          wwptr->lock->linked_windows[jj] = 1;
        }
      }

      wwptr->lock->sizeof_struct = sizeof(struct solo_plot_lock);
      wwptr->active = YES;
      spi->active_windows[gww] = YES;
      wwptr->lock->changed = YES;
    }
    else if (strncmp(buf_ptr, "SPMI", 4) == 0)
    {
      // Parameter info

      WW_PTR wwptr = solo_return_wwptr(gww);
      bool old_sos = (record_size != sizeof(struct solo_parameter_info));

      struct solo_parameter_info0 spmi0;
      struct res_solo_parameter_info0 res_spmi0;
      
      if (gottaSwap)
      {
        if (old_sos)
        {
          sp_crack_spmi0(buf_ptr, (char *)(&wwptr->parameter), (int)0);
        }
        else
        {
          sp_crack_spmi(buf_ptr, (char *)(&wwptr->parameter), (int)0);
        }
      }
      else
      {
        if (old_sos)
        {
          memcpy (&spmi0, buf_ptr, sizeof (spmi0));
        }
        else
        {
          memcpy((char *)(&wwptr->parameter), buf_ptr,
                 sizeof(wwptr->parameter));
        }
      }

      if (old_sos)
      {
        memcpy((char *)(&wwptr->parameter), &spmi0, sizeof(spmi0));
        memcpy(wwptr->parameter.parameter_name, spmi0.parameter_name,
               sizeof(res_spmi0));
        for (int jj = frame_count; jj < SOLO_MAX_WINDOWS; jj++)
        {
          wwptr->parameter.linked_windows[jj] = 0;
        }
      }

      wwptr->parameter.sizeof_struct = sizeof(struct solo_parameter_info);
      wwptr->parameter.changed = YES;
    }
    else if(strncmp(buf_ptr, "SWVI", 4) == 0)
    {
      // View info

      WW_PTR wwptr = solo_return_wwptr(gww);

      struct solo_view_info view;
      struct solo_view_info0 swvi0;
      struct res_solo_view_info0 res_swvi0;
      
      bool new_config = false;
      
      if (gottaSwap)
        sp_crack_swvi(buf_ptr, (char *)&view, (int)0);
      else
        memcpy(&view, buf_ptr, sizeof(view));

      // Smallest value in the new configuration is 0101 and luckily the
      // frame_config is before the linked_windows variable

      if (view.frame_config > 100)
        new_config = true;

      if (gottaSwap)
      {
        if (new_config)
          memcpy(wwptr->view, &view, sizeof(view));
        else
          sp_crack_swvi0(buf_ptr, (char *)&swvi0, (int)0);
      }
      else
      {
        if (new_config)
          memcpy(wwptr->view, buf_ptr, sizeof(*wwptr->view));
        else
          memcpy(&swvi0, buf_ptr, sizeof (swvi0));
      }

      if (!new_config)
      {
        memcpy(wwptr->view, &swvi0, sizeof(swvi0));
        memcpy(&wwptr->view->type_of_annot, &swvi0.type_of_annot,
               sizeof(res_swvi0));
        for (int jj = frame_count; jj < SOLO_MAX_WINDOWS; jj++)
        {
          wwptr->view->linked_windows[jj] = 1;
        }
      }
      
      wwptr->view->sizeof_struct = sizeof(struct solo_view_info);
      wwptr->view->changed = YES;
      
      char *pisp_buf_ptr = buf_ptr + wwptr->view->offset_to_pisps;
      
      for (int ii=0; ii < wwptr->view->num_pisps; ii++)
      {
        
        PointInSpace pisp;
        pisp.readFromBuffer(pisp_buf_ptr, gottaSwap);
        
        if (strstr(pisp.getId().c_str(), "SOL_RAD_LOC") != 0)
          wwptr->radar_location = pisp;
        else if (strstr(pisp.getId().c_str(), "SOL_WIN_LOC") != 0)
          wwptr->center_of_view = pisp;
        else if(strstr(pisp.getId().c_str(), "SOL_LANDMARK") != 0)
          wwptr->landmark = pisp;

        pisp_buf_ptr += pisp.getBufLen();
      }

      // Extract and setup the configuration info

      int nn = wwptr->view->frame_config;
      
      if (nn > 100)
      {
        // Smallest value is 0101

        stp.bottom_attach = nn % 10;
        nn /= 10;
        stp.top_attach = nn % 10;
        nn /= 10;
        stp.right_attach = nn % 10;
        nn /= 10;
        stp.left_attach = nn % 10;
        nn /= 10;
            
        int mm = stp.right_attach - stp.left_attach;
        nn = stp.bottom_attach - stp.top_attach;

        if (gww == 0)
        {
          // NOTE: these are globals

          sii_table_widget_width = wwptr->view->width_in_pix/mm;
          sii_table_widget_height = wwptr->view->height_in_pix/nn;
        }

        int ii = stp.top_attach * MAX_CONFIG_COLS;
        for (; nn--; ii += MAX_CONFIG_COLS)
        {
          int jj = ii + stp.left_attach;
          int kk = jj +mm;
          for (; jj < kk ; ++jj)
            config_cells[jj]->frame_num = gww+1;
        }
      }
      else if (!new_config)
      {
        config_count++;
      }
    }
    else if (strncmp(buf_ptr, "SLMK", 4) == 0)
    {
      // Landmark use info

      WW_PTR wwptr = solo_return_wwptr(gww);
      bool old_sos = (record_size != sizeof(struct landmark_info));

      struct landmark_info0 slmk0;
      struct res_landmark_info0 res_slmk0;
      
      if (gottaSwap)
      {
        if (old_sos)
        {
          sp_crack_slmk0 (buf_ptr, (char *)wwptr->landmark_info, (int)0);
        }
        else
        {
          sp_crack_slmk(buf_ptr, (char *)wwptr->landmark_info, (int)0);
        }
      }
      else
      {
        if (old_sos)
        {
          memcpy (&slmk0, buf_ptr, sizeof (slmk0));
        }
        else
        {
          memcpy(wwptr->landmark_info, buf_ptr,
                 sizeof(*wwptr->landmark_info));
        }
      }
      if (old_sos)
      {
        memcpy(wwptr->landmark_info, &slmk0, sizeof (slmk0));
        memcpy(&wwptr->landmark_info->landmark_options,
               &slmk0.landmark_options, sizeof(res_slmk0));

        for (int jj = frame_count; jj < SOLO_MAX_WINDOWS; jj++)
        {
          wwptr->landmark_info->linked_windows[jj] = 1;
        }
      }
      wwptr->landmark_info->sizeof_struct = sizeof(struct landmark_info);
    }
    else if (strncmp(buf_ptr, "SCTR", 4) == 0)
    {
      // Center of frame info

      WW_PTR wwptr = solo_return_wwptr(gww);
      bool old_sos = (record_size != sizeof(struct frame_ctr_info));

      struct frame_ctr_info0 sctr0;
      struct res_frame_ctr_info0 res_sctr0;
      
      if (gottaSwap)
      {
        if (old_sos)
        {
          sp_crack_sctr0(buf_ptr, (char *)wwptr->frame_ctr_info, (int)0);
        }
        else
        {
          sp_crack_sctr(buf_ptr, (char *)wwptr->frame_ctr_info, (int)0);
        }
      }
      else
      {
        if (old_sos)
        {
          memcpy(&sctr0, buf_ptr, sizeof (sctr0));
        }
        else
        {
          memcpy(wwptr->frame_ctr_info, buf_ptr,
                 sizeof(*wwptr->frame_ctr_info));
        }
      }

      if (old_sos)
      {
        memcpy(wwptr->frame_ctr_info, &sctr0, sizeof (sctr0));
        memcpy(&wwptr->frame_ctr_info->centering_options,
               &sctr0.centering_options, sizeof(res_sctr0));

        for (int jj = frame_count; jj < SOLO_MAX_WINDOWS; jj++)
        {
          wwptr->frame_ctr_info->linked_windows[jj] = 1;
        }
      }

      wwptr->frame_ctr_info->sizeof_struct = sizeof(struct frame_ctr_info);
    }
    else if(strncmp(buf_ptr, "SXMN", 4) == 0)
    {
      // Examine info

      WW_PTR wwptr = solo_return_wwptr(gww);

      if (gottaSwap)
      {
        // NOTE: This didn't have a function specified.  Should be one of the
        // "crack" functions.

        sp_crack_sxmn(buf_ptr, (char *)wwptr->examine_info, (int)0);
      }
      else
      {
        memcpy(wwptr->examine_info, buf_ptr, sizeof(*wwptr->examine_info));
      }
      wwptr->examine_info->sizeof_struct = sizeof(struct solo_examine_info);

      // Just in case there's a non-zero change count lurking out there

      wwptr->examine_info->change_count = 0;
    }
    else if (strncmp(buf_ptr, "SXTR", 4) == 0)
    {
      // New sii options

      WW_PTR wwptr = solo_return_wwptr(gww);
      siiex = (struct solo_sii_extras*)buf_ptr;

      std::vector< std::string > attributes;
      
      tokenize(siiex->attributes, attributes, ";");

      for (std::size_t ii = 0; ii < attributes.size(); ii++)
      {
        std::string attribute = attributes[ii];

        if (attribute.find("TSYNC") != std::string::npos)
        {
          // Swpfi time sync of frame 1

          wwptr->swpfi_time_sync_set = true;
        }
        else if (attribute.find("SWPFLT") != std::string::npos)
        {
          // Swpfi filter

          wwptr->swpfi_filter_set = gww +1;

          std::vector< std::string > tokens;
          tokenize(attribute, tokens, ": \t\n,");

          sscanf(tokens[1].c_str(), "%f", &wwptr->filter_fxd_ang);
          sscanf(tokens[2].c_str(), "%f", &wwptr->filter_tolerance);
          wwptr->filter_scan_modes = "";
          for (std::size_t kk = 3; kk < tokens.size(); kk++)
            wwptr->filter_scan_modes += std::string(tokens[kk]) + ",";
        }
        else if (attribute.find("CB_LOC") != std::string::npos)
        {
          // Color bar location
          // Default is bottom

          if (attribute.find("-1") != std::string::npos)
          {
            wwptr->color_bar_location = -1; // left side
          }
          else
          {
            wwptr->color_bar_location = 1;        // right side
          }
          param_set_cb_loc(gww, wwptr->color_bar_location);
        }
        else if (attribute.find("CB_SYMS") != std::string::npos)
        {
          // CB symbols

          wwptr->color_bar_symbols = YES;
        }
      }
      SweepfileWindow *sweepfile_window = frame_configs[gww]->sweepfile_window;
      if (sweepfile_window != 0)
        sweepfile_window->update();
      ParameterWindow *param_window = frame_configs[gww]->param_window;
      if (param_window != 0)
        param_window->update();
    }

    buf_ptr += record_size;

    if (buf_ptr >= end_ptr)
      break;

    if (count > 1000)
      break;
  }
  delete [] input_buffer;

  // Setup the window from the old configuration if config_count > 0
  
  if (config_count > 0)
  {
    int solo_width = 372;
    
    int h_default = solo_width;
    int v_default = (int)(solo_width * .75 +.5); /* slide proportions */
    v_default = ((v_default + 3) / 4) * 4;

    int cfg = solo_return_wwptr(0)->view->frame_config;
    if (cfg == LARGE_SQUARE_FRAME || cfg == LARGE_SLIDE_FRAME)
    {
      config_count = 1;
    }
    switch (cfg)
    {
    case LARGE_SQUARE_FRAME:
    case SQUARE_FRAME:
    case LARGE_HALF_HEIGHT_FRAME:
      v_default = h_default;
      break;
    }
       
    for (int ww = 0; ww < config_count; ww++)
    {
      WW_PTR wwptr = solo_return_wwptr(ww);
      cfg = wwptr->view->frame_config;

      int ba = 0;
      int ta = 0;
      int la = 0;
      int ra = 0;
      
      switch (cfg)
      {
      case SMALL_RECTANGLE:
      case SQUARE_FRAME:
        if (ww & 1)
        {
          // odd => right frame

          la = 1;
          ra = 2;
        }
        else
        {
          // Left frame

          la = 0;
          ra = 1;
        }

        if (ww / 2 == 0)
        {
          // Always 2 frames wide

          ta = 0;
          ba = 1;
        }
        else if (ww / 2 == 1)
        {
          ta = 1;
          ba = 2;
        }
        else
        {
          ta = 2;
          ba = 3;
        }
        break;

      case LARGE_SLIDE_FRAME:
        la = 0;
        ra = 3;
        ta = 0;
        ba = 3;
        break;
      case LARGE_SQUARE_FRAME:
        la = 0;
        ra = 2;
        ta = 0;
        ba = 2;
        break;
      case DOUBLE_WIDE_FRAME:
        la = 0;
        ra = 2;
        if (ww == 0)
        {
          // One frame per row

          ta = 0;
          ba = 1;
        }
        else if (ww == 1)
        {
          ta = 1;
          ba = 2;
        }
        else
        {
          ta = 2;
          ba = 3;
        }
        break;
      case LARGE_HALF_HEIGHT_FRAME:
        la = 0;
        ra = 3;
        if (ww == 0)
        {
          // One frame per row

          ta = 0;
          ba = 1;
        }
        else
        {
          ta = 1;
          ba = 2;
        }
        break;
      };
        
      stp.bottom_attach = ba;
      stp.top_attach = ta;
      stp.left_attach = la;
      stp.right_attach = ra;
        
      int mm = stp.right_attach - stp.left_attach;
      int nn = stp.bottom_attach - stp.top_attach;
        
      if (ww == 0)
      {
        sii_table_widget_width = h_default;
        sii_table_widget_height = v_default;
      }
      int ii = stp.top_attach * MAX_CONFIG_COLS;
      for (; nn--; ii += MAX_CONFIG_COLS)
      {
        int jj = ii + stp.left_attach;
        int kk = jj +mm;
        for (; jj < kk ; jj++)
          config_cells[jj]->frame_num = ww+1;
      }
    }        /* end if (config_count > 0) */
  }

  /* Set dangling frames */

  for (int ww = frame_count; ww < SOLO_MAX_WINDOWS; ww++)
  {
    solo_cpy_sweep_info(0, ww);
    solo_cpy_lock_info(0, ww);
    solo_cpy_view_info(0, ww);
    sp_cpy_lmrk_info(0, ww);
    sp_cpy_ctr_info(0, ww);
  }

  for (int kk = 0; kk < SOLO_MAX_WINDOWS; kk++)
    spi->active_windows[kk] = YES;

  sii_set_config ();
  sii_new_frames ();                /* triggers plots from expose callback */

  return 1;
}

/* c------------------------------------------------------------------------ */

void solo_save_window_info(const std::string &dir, const std::string &a_comment)
{
  struct solo_perusal_info *spi = solo_return_winfo_ptr();
  int frame_count = sii_return_frame_count();

  // Generate the file name

  WW_PTR wwptr = 0;
  
  for (int ww = 0; ww < SOLO_MAX_WINDOWS; ww++)
  {
    // Find first active window

    if (spi->active_windows[ww])
    {
      wwptr = solo_return_wwptr(ww);
      break;
    }
  }

  std::string file_path = dir;
  
  if (file_path == "")
    file_path = wwptr->sweep->directory_name;

  if (file_path[file_path.size()-1] != '/')
    file_path += "/";
  
  std::string file_name =
    DataManager::getInstance()->getFileBaseName("wds", time(0),
                                                wwptr->sweep->radar_name,
                                                getuid());
  file_path += file_name;

  if (a_comment != "")
  {
    std::string my_comment = a_comment;
    se_fix_comment(my_comment);
    
    file_path += ".";
    file_path += my_comment;
  }
  
  FILE *stream;
  
  if ((stream = fopen(file_path.c_str(), "w")) == 0)
  {
    char message[256];
    
    sprintf(message, "Problem opening %s for writing\n", file_path.c_str());
    sii_message(message);
    return;
  }

  fpos_t pos;
  fgetpos(stream, &pos);

  // Save the palettes to the file

  if (!PaletteManager::getInstance()->writePalettes(stream))
    return;

  // Save the direct window descriptors

  for (int ww = 0; ww < sii_return_frame_count(); ww++)
  {
    if (!spi->active_windows[ww])
      continue;

    SiiFrameConfig *sfc = frame_configs[ww];
    WW_PTR wwptr = solo_return_wwptr(ww);
    
    wwptr->sweep->sizeof_struct= sizeof(struct solo_sweep_file);
    if (fwrite(wwptr->sweep, sizeof(char),
               (size_t)wwptr->sweep->sizeof_struct, stream)
        < (std::size_t)wwptr->sweep->sizeof_struct)
    {
      char message[256];
      
      sprintf(message, "Problem writing sweep info %d\n", ww);
      solo_message(message);
      return;
    }

    wwptr->lock->sizeof_struct= sizeof(struct solo_plot_lock);
    if (fwrite(wwptr->lock, sizeof(char),
               (size_t)wwptr->lock->sizeof_struct, stream)
        < (std::size_t)wwptr->lock->sizeof_struct)
    {
      char message[256];
      
      sprintf(message, "Problem writing lock info %d\n", ww);
      solo_message(message);
      return;
    }

    wwptr->parameter.sizeof_struct = sizeof(struct solo_parameter_info);
    if (fwrite((char *)(&wwptr->parameter), sizeof(char),
               (size_t)wwptr->parameter.sizeof_struct, stream)
        < (std::size_t)wwptr->parameter.sizeof_struct)
    {
      char message[256];
      
      sprintf(message, "Problem writing radar parameter %d\n", ww);
      solo_message(message);
      return;
    }

    wwptr->frame_ctr_info->sizeof_struct = sizeof(struct frame_ctr_info);
    if (fwrite(wwptr->frame_ctr_info, sizeof(char),
               (size_t)wwptr->frame_ctr_info->sizeof_struct, stream)
        < (std::size_t)wwptr->frame_ctr_info->sizeof_struct)
    {
      char message[256];
      
      sprintf(message, "Problem writing frame_ctr info %d\n", ww);
      solo_message(message);
      return;
    }

    wwptr->landmark_info->sizeof_struct = sizeof(struct landmark_info);
    if (fwrite(wwptr->landmark_info, sizeof(char),
               (size_t)wwptr->landmark_info->sizeof_struct, stream)
        < (std::size_t)wwptr->landmark_info->sizeof_struct)
    {
      char message[256];
      
      sprintf(message, "Problem writing landmark info %d\n", ww);
      solo_message(message);
      return;
    }

    wwptr->examine_info->sizeof_struct = sizeof(struct solo_examine_info);
    if (fwrite(wwptr->examine_info, sizeof(char),
               (size_t)wwptr->examine_info->sizeof_struct, stream)
        < (std::size_t)wwptr->examine_info->sizeof_struct)
    {
      char message[256];
      
      sprintf(message, "Problem writing solo_examine_info %d\n", ww);
      solo_message(message);
      return;
    }

    // The view struct is messier

    wwptr->view->num_pisps = 3;
    wwptr->view->offset_to_pisps = sizeof(struct solo_view_info);
    wwptr->view->sizeof_struct = sizeof(struct solo_view_info) +
      (3 * PointInSpace::NUM_BYTES);
        
    wwptr->view->width_in_pix = sfc->width;
    wwptr->view->height_in_pix = sfc->height;

    if (ww < frame_count)
    {
      int nn = sfc->tbl_parms.left_attach;
      nn = nn * 10 + sfc->tbl_parms.right_attach;
      nn = nn * 10 + sfc->tbl_parms.top_attach;
      wwptr->view->frame_config = nn * 10 + sfc->tbl_parms.bottom_attach;
    }
    else
    {
      wwptr->view->frame_config = 0;
    }
        

    if (fwrite(wwptr->view, sizeof(char),
               sizeof(struct solo_view_info), stream)
        < sizeof(struct solo_view_info))
    {
      char message[256];
      
      sprintf(message, "Problem writing view info %d\n", ww);
      solo_message(message);
      return;
    }

    if (fwrite(wwptr->radar_location.getBufPtr(), sizeof(char),
               wwptr->radar_location.getBufLen(), stream)
        < PointInSpace::NUM_BYTES)
    {
      char message[256];
      
      sprintf(message, "Problem writing radar_location info %d\n", ww);
      solo_message(message);
      return;
    }

    if (fwrite(wwptr->center_of_view.getBufPtr(), sizeof(char),
               wwptr->center_of_view.getBufLen(), stream)
        < PointInSpace::NUM_BYTES)
    {
      char message[256];
      
      sprintf(message, "Problem writing center_of_view info %d\n", ww);
      solo_message(message);
      return;
    }

    if (fwrite(wwptr->landmark.getBufPtr(), sizeof(char),
               wwptr->landmark.getBufLen(), stream)
        < PointInSpace::NUM_BYTES)
    {
      char message[256];
      
      sprintf(message, "Problem writing landmark info %d\n", ww);
      solo_message(message);
      return;
    }

    // New solo3 features

    if (sse == 0)
    {
      sse = (struct solo_sii_extras*)malloc(sizeof(*sse));
      memset(sse, 0, sizeof (*sse));
      strncpy(sse->name_struct, "SXTR", 4);
      sse->sizeof_struct = sizeof (*sse);
    }

    sse->window_num = ww;
    sse->attributes[0] = '\0';

    if (wwptr->swpfi_time_sync_set)
    {
      strcat(sse->attributes, "TSYNC;");
    }
        
    if (wwptr->swpfi_filter_set)
    {
      char value_string[1024];
      
      strcat(sse->attributes, "SWPFLT:");
      sprintf(value_string, "%.2f,%.3f,", wwptr->filter_fxd_ang,
              wwptr->filter_tolerance);
      strcat(sse->attributes, value_string);
      strcat(sse->attributes, wwptr->filter_scan_modes.c_str());
      strcat(sse->attributes, ";");
    }

    if (wwptr->color_bar_location)
    {
      strcat(sse->attributes, "CB_LOC:");
      strcat(sse->attributes, (wwptr->color_bar_location > 0) ? "1" : "-1");
      strcat(sse->attributes, ";");
    }

    if (wwptr->color_bar_symbols)
    {
      strcat(sse->attributes, "CB_SYMS;");
    }
        
    if (fwrite(sse, sizeof(char), sizeof(*sse), stream) < sizeof(*sse))
    {
      char message[256];
      
      sprintf(message, "Problem writing solo_ssi_extras info %d\n", ww);
      solo_message(message);
      return;
    }
  }

  // Write color tables here (odd byte counts)
  
  if (!ColorTableManager::getInstance()->dumpTables(stream))
    return;

  fclose(stream);
}
