/* 	$Id$	 */

#include <glib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <DateTime.hh>
#include "se_bnd.hh"
#include "se_catch_all.hh"
#include "se_proc_data.hh"
#include "se_utils.hh"
#include "SeBoundaryList.hh"
#include <seds.h>
#include <solo_editor_structs.h>
#include <solo_list_widget_ids.h>
#include <solo_window_structs.h>

extern GString *gs_complaints;

/* c------------------------------------------------------------------------ */

#ifdef USE_RADX
#else

int se_once_only(const std::vector< UiCommandToken > &cmd_tokens)
{
  /*
   * #a-speckle#
   * #ac-sidelobe-ring#
   * #first-good-gate#
   * #freckle-average#
   * #freckle-threshold#
   * #min-bad-count#
   * #min-notch-shear#
   * #notch-max#
   * #offset-for-radial-shear#
   * #ew-wind#
   * #ns-wind#
   * #vert-wind#
   * #optimal-beamwidth#
   * #omit-source-file-info#
   * #surface-shift#
   * #surface-gate-shift#
   * #deglitch-radius#
   * #deglitch-threshold#
   * #deglitch-min-gates#
   * ##
   * ##
   * ##
   */

  struct solo_edit_stuff *seds = return_sed_stuff();

  if (cmd_tokens[0].getCommandText().compare(0, 7, "a-speckle", 0, 7) == 0)
  {
    seds->a_speckle = cmd_tokens[1].getIntParam();
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 7, "ac-sidelobe-ring",
						  0, 7) == 0)
  {
    seds->sidelobe_ring_gate_count = cmd_tokens[1].getIntParam();
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 11, "first-good-gate",
						  0, 11) == 0)
  {
    seds->first_good_gate = cmd_tokens[1].getIntParam();
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 9, "freckle-average",
						  0, 9) == 0)
  {
    seds->freckle_avg_count = cmd_tokens[1].getIntParam();
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 11, "deglitch-radius",
						  0, 11) == 0)
  {
    seds->deglitch_radius = cmd_tokens[1].getIntParam();
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 11, "deglitch-min_gates",
						  0, 11) == 0)
  {
    seds->deglitch_min_bins = cmd_tokens[1].getIntParam();
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 9, "freckle-threshold",
						  0, 9) == 0)
  {
    seds->freckle_threshold = cmd_tokens[1].getFloatParam();
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 11, "deglitch-threshold",
						  0, 11) == 0)
  {
    seds->deglitch_threshold = cmd_tokens[1].getFloatParam();
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 7, "gates-shifted",
						  0, 7) == 0)
  {
    seds->gates_of_shift = cmd_tokens[1].getIntParam();
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 7, "min-bad-count",
						  0, 7) == 0)
  {
    seds->min_bad_count = cmd_tokens[1].getIntParam();
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 9, "min-notch-shear",
						  0, 9) == 0)
  {
    seds->notch_shear = cmd_tokens[1].getFloatParam();
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 9, "notch-max",
						  0, 9) == 0)
  {
    seds->notch_max = cmd_tokens[1].getFloatParam();
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 9,
						  "offset-for-radial-shear", 0, 9) == 0)
  {
    seds->gate_diff_interval = cmd_tokens[1].getIntParam();
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 5, "ew-wind",
						  0, 5) == 0)
  {
    seds->ew_wind = cmd_tokens[1].getFloatParam();
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 5, "ns-wind",
						  0, 5) == 0)
  {
    seds->ns_wind = cmd_tokens[1].getFloatParam();
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 7, "vert-wind",
						  0, 7) == 0)
  {
    seds->ud_wind = cmd_tokens[1].getFloatParam();
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 11, "optimal-beamwidth",
						  0, 11) == 0)
  {
    seds->optimal_beamwidth = cmd_tokens[1].getFloatParam();
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 7, "omit-source-file-info", 0, 7) == 0)
  {
    struct sed_command_files *scf = se_return_cmd_files_struct();
    scf->omit_source_file_info = !scf->omit_source_file_info;
    g_string_sprintfa(gs_complaints, "Source file info will be %s\n",
		      scf->omit_source_file_info ? "IGNORED" : "USED");
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 11, "surface-shift",
						  0, 11) == 0)
  {
    seds->surface_shift = cmd_tokens[1].getFloatParam();
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 11, "surface-gate-shift",
						  0, 11) == 0)
  {
    seds->surface_gate_shift = cmd_tokens[1].getIntParam();
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 6, "radar-", 0, 6) == 0)
  {
    struct one_boundary *ob = SeBoundaryList::getInstance()->currentBoundary;

    if (cmd_tokens[0].getCommandText().find("inside") != std::string::npos)
    {
      ob->bh->force_inside_outside = BND_FIX_RADAR_INSIDE;
    }
    else if (cmd_tokens[0].getCommandText().find("outside") !=
	     std::string::npos)
    {
      ob->bh->force_inside_outside = BND_FIX_RADAR_OUTSIDE;
    }
    else if (cmd_tokens[0].getCommandText().find("unforced") !=
	     std::string::npos)
    {
      ob->bh->force_inside_outside = 0;
    }
  }

  return 1;
}

#endif

/* c------------------------------------------------------------------------ */
/* This routine will process all commands associated with the setup-input    */
/* procedure. It also updates the sfic texts.                                */

#ifdef USE_RADX
#else

int se_dir(const std::vector< UiCommandToken > &cmd_tokens)
{
  // Get a pointer to the command arguments.  These will be pulled out
  // below based on the actual command

  int token_num = 1;
  
  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  // Process the command

  if (cmd_tokens[0].getCommandText().compare(0, 7, "sweep-dir", 0, 7) == 0)
  {
    std::string command;
    
    command = (cmd_tokens[token_num].getCommandText());

    if (command.compare(0, 8, "dorade-directory") == 0)
    {
      char *a;
      
      if ((a = getenv("DD_DIR")) || (a = getenv("DORADE_DIR")))
      {
	seds->sfic->directory = a;
	if (seds->sfic->directory[seds->sfic->directory.size()-1] != '/')
	  seds->sfic->directory += "/";
      }
      else
      {
	g_string_sprintfa(gs_complaints,  "DORADE_DIR undefined\n");
	std::string cmd_text = cmd_tokens[token_num].getCommandText();
	if (cmd_text[cmd_text.size()-1] != '/')
	  cmd_text += "/";
	seds->sfic->directory = cmd_text;
      }
    }
    else
    {
      seds->sfic->directory = command;
    }
    seds->sfic->directory_text = seds->sfic->directory;
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 7,
						  "radar-names", 0, 7) == 0)
  {
    seds->num_radars = 0;
    seds->radar_stack.clear();
    seds->sfic->radar_names_text[0] = '\0';
    for (int ii = 0;
	 cmd_tokens[token_num].getTokenType() != UiCommandToken::UTT_END;
	 token_num++,ii++)
    {
      seds->radar_stack.push_back(cmd_tokens[token_num].getCommandText());
      seds->num_radars++;
      if (ii != 0)
	seds->sfic->radar_names_text += " ";
      seds->sfic->radar_names_text += cmd_tokens[token_num].getCommandText();
    }
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 7, "first-sweep", 0, 7) == 0)
  {
    if (cmd_tokens[token_num].getCommandText().compare("first") == 0)
    {
      seds->sfic->start_time = DAY_ZERO;   
      seds->sfic->first_sweep_text = "first";
    }
    else if (cmd_tokens[token_num].getCommandText().compare("last") == 0)
    {
      seds->sfic->start_time = ETERNITY;   
      seds->sfic->first_sweep_text = "last";
    }
    else
    {
      // Try to interpret this as a date of the form mm/dd/yy:hh:mm:ss.ms

      DateTime data_time;
      
      seds->sfic->first_sweep_text = cmd_tokens[token_num].getCommandText();
      data_time.parse(cmd_tokens[token_num].getCommandText());
      seds->sfic->start_time = data_time.getTimeStamp();
    }
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 7, "last-sweep", 0, 7) == 0)
  {
    if (cmd_tokens[token_num].getCommandText().compare("first") == 0)
    {
      seds->sfic->stop_time = DAY_ZERO;   
      seds->sfic->last_sweep_text = "first";
    }
    else if (cmd_tokens[token_num].getCommandText().compare("last") == 0)
    {
      seds->sfic->stop_time = ETERNITY;   
      seds->sfic->last_sweep_text = "last";
    }
    else
    {
      // Try to interpret this as a number

      DateTime data_time;
      
      seds->sfic->last_sweep_text = cmd_tokens[token_num].getCommandText();
      data_time.parse2(cmd_tokens[token_num].getCommandText());
      seds->sfic->stop_time = data_time.getTimeStamp();
    }
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 4, "version", 0, 4) == 0)
  {
    if (cmd_tokens[token_num].getCommandText().compare("first") == 0)
    {
      seds->sfic->version = -1;
      seds->sfic->version_text = "first";
    }
    else if (cmd_tokens[token_num].getCommandText().compare("last") == 0)
    {
      seds->sfic->version = 99999;
      seds->sfic->version_text = "last";
    }
    else
    {
      // Try to interpret this as a number

      seds->sfic->version = 99999;
      int nn;
      
      if ((nn = sscanf(cmd_tokens[token_num].getCommandText().c_str(), "%d",
		       &seds->sfic->version)) == 0)
      {
	// Not a number

	g_string_sprintfa(gs_complaints, 
		 "\nCould not interpret version number: %s  Using: %d\n",
			  cmd_tokens[token_num].getCommandText().c_str(),
			  seds->sfic->version);
      }
      char version_text[80];
      sprintf(version_text, "%d", seds->sfic->version);
      seds->sfic->version_text = version_text;
    }
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 7, "new-version", 0, 7) == 0)
  {
    seds->sfic->new_version_flag = YES;
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 9, "no-new-version", 0, 9) == 0)
  {
    seds->sfic->new_version_flag = NO;
  }

  return 1;
}

#endif
