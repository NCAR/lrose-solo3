#include <iostream>

#include <DateTime.hh>
#include <se_bnd.hh>
#include <se_proc_data.hh>
#include <se_utils.hh>
#include <SeBoundaryList.hh>
#include <sii_utils.hh>
#include <solo_list_widget_ids.h>
#include <solo2.hh>
#include <sp_basics.hh>
#include <UiCommandFactory.hh>

#include "EditWindow.hh"



/**********************************************************************
 * Constructor
 */

EditWindow::EditWindow(const Pango::FontDescription &default_font,
		       const int frame_index) :
  Gtk::Window(),
  _frameIndex(frame_index),
  _defaultFont(default_font),
  _frameActive(true),
  _origSweepfileDir(sii_return_swpfi_dir(frame_index)),
  _origBoundaryDir(""),
  _origLlbDir(""),
  _buttonsTable(6, 1, true),
  _cmdsTable(5, 2, false),
  _eachRayCmdList(1),
  _oneTimeCmdList(1),
  _startStopTable(2, 6, true),
  _editCmdListHelpWindow(0),
  _editFilesWindow(0)
{
  // Get pointers to the editing information

  struct solo_edit_stuff *seds = return_sed_stuff();
  
  // Create some fonts to use for our widgets

  Pango::FontDescription bold_font = _defaultFont;
  bold_font.set_weight(Pango::WEIGHT_SEMIBOLD);
  
  Pango::FontDescription smaller_italic_font = _defaultFont;
  smaller_italic_font.set_size(_defaultFont.get_size() - (2 * 1024));
  smaller_italic_font.set_style(Pango::STYLE_ITALIC);
  
  // NOTE: May need to go through the lists and get the maximum command
  // width for sizing the underlying widget

  int width = 500;
  int font_height = 20;
  int user_nlines = 6;
  int cmd_list_nlines = 11;
  
  // Set the window title and border width

  char display_string[1024];
  sprintf(display_string, "Frame %d  Edit Widget", _frameIndex + 1);
  set_title(display_string);

  set_border_width(0);
  
  // Create new vertical box for storing widgets

  _vbox.set_homogeneous(false);
  _vbox.set_spacing(4);
  add(_vbox);

  // Create the menubar

  _createMenubar(_vbox);
  
  // Create a new horizontal box for storing 3 sets of widgets

  _hbox.set_homogeneous(false);
  _hbox.set_spacing(2);
  _vbox.add(_hbox);
  
  // Menu buttons

  _buttonsBox.set_homogeneous(false);
  _buttonsBox.set_spacing(1);
  _buttonsBox.set_border_width(2);
  _hbox.pack_start(_buttonsBox, true, true, 0);
  _buttonsBox.pack_start(_buttonsTable, false, true, 0);
  
  int row = 0;
  
  _clearBndButton.set_label("Clear Bnd");
  _clearBndButton.modify_font(bold_font);
  _buttonsTable.attach(_clearBndButton, 0, 1, row, row+1,
		       Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		       Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		       2, 4);
  ++row;
  _clearBndButton.signal_clicked().connect(sigc::mem_fun(*this,
							 &EditWindow::_clearBoundaries));
  
  _okButton.set_label("OK Do It!");
  _okButton.modify_font(bold_font);
  _buttonsTable.attach(_okButton, 0, 1, row, row+2,
		       Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		       Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		       2, 4);
  row += 2;
  _okButton.signal_clicked().connect(sigc::mem_fun(*this,
						   &EditWindow::_doIt));

  _addNextBndButton.set_label(" Add Next Bnd ");
  _addNextBndButton.modify_font(bold_font);
  _buttonsTable.attach(_addNextBndButton, 0, 1, row, row+1,
		       Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		       Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		       2, 4);
  ++row;
  _addNextBndButton.signal_clicked().connect(sigc::mem_fun(*this,
							   &EditWindow::_addNextBoundary));

  _prevBndSetButton.set_label(" Prev Bnd Set ");
  _prevBndSetButton.modify_font(bold_font);
  _buttonsTable.attach(_prevBndSetButton, 0, 1, row, row+1,
		       Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		       Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		       2, 4);
  ++row;
  _prevBndSetButton.signal_clicked().connect(sigc::mem_fun(*this,
							   &EditWindow::_setPreviousBoundary));

  // For each ray commands

  _hbox.pack_start(_cmdsTable, true, true, 0);
  
  _hbox1.set_homogeneous(false);
  _hbox1.set_spacing(2);
  _cmdsTable.attach(_hbox1, 0, 1, 0, 1,
		    Gtk::EXPAND | Gtk::FILL,
		    (Gtk::AttachOptions)0,
		    2, 4);
  
  _eachRayCmdsLabel.set_text("Commands For Each Ray");
  _eachRayCmdsLabel.modify_font(bold_font);
  _eachRayCmdsLabel.set_justify(Gtk::JUSTIFY_CENTER);
  _hbox1.pack_start(_eachRayCmdsLabel, true, true, 0);
  
  _clearEachRayButton.set_label("Clear");
  _clearEachRayButton.modify_font(bold_font);
  _hbox1.pack_start(_clearEachRayButton, true, true, 0);
  _clearEachRayButton.signal_clicked().connect(sigc::mem_fun(*this,
							     &EditWindow::_clearForEachRayCmds));
  
  _userEachRayTextView.set_size_request(width,
					font_height * user_nlines);
  _userEachRayTextView.modify_font(_defaultFont);
  _userEachRayTextView.set_editable(true);
  _cmdsTable.attach(_userEachRayTextView, 0, 1, 1, 2,
		    Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		    Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		    2, 4);
  
  _eachRayListLabel.set_text("List of All For Each Ray Commands");
  _eachRayListLabel.modify_font(bold_font);
  _eachRayListLabel.set_justify(Gtk::JUSTIFY_CENTER);
  _cmdsTable.attach(_eachRayListLabel, 0, 1, 2, 3,
		    (Gtk::AttachOptions)0,
		    (Gtk::AttachOptions)0,
		    2, 4);
  
  _eachRayCmdList.set_headers_visible(false);
  _eachRayCmdList.set_size_request(width,
				   font_height * cmd_list_nlines);
  _eachRayCmdList.modify_font(_defaultFont);

  // Set the padding for the list.  Note that the gtkmm-2.4 installed with
  // the Centos OS has a different API for the CellRenderer class so we can't
  // do this on that OS.
#ifndef OS_IS_CENTOS
  Gtk::CellRenderer *each_ray_renderer =
    _eachRayCmdList.get_column_cell_renderer(0);
  each_ray_renderer->set_padding(0, 0);
#endif

  std::vector< std::string > fer_cmds =
    UiCommandFactory::getInstance()->getFERTemplates();
  
  for (size_t i = 0; i < fer_cmds.size(); ++i)
    _eachRayCmdList.append_text(fer_cmds[i]);
  _eachRayCmdList.signal_row_activated().connect(sigc::mem_fun(*this,
							       &EditWindow::_selectEachRayCmd));

  _eachRayCmdListScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC,
					   Gtk::POLICY_AUTOMATIC);
  _eachRayCmdListScrolledWindow.add(_eachRayCmdList);
  
  _cmdsTable.attach(_eachRayCmdListScrolledWindow, 0, 1, 3, 4,
		    Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		    Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		    2, 4);

  // One time only commands

  _hbox2.set_homogeneous(false);
  _hbox2.set_spacing(2);
  _cmdsTable.attach(_hbox2, 1, 2, 0, 1,
		    Gtk::EXPAND | Gtk::FILL,
		    (Gtk::AttachOptions)0,
		    2, 4);
  
  _oneTimeCmdsLabel.set_text("One Time Only Commands");
  _oneTimeCmdsLabel.modify_font(bold_font);
  _oneTimeCmdsLabel.set_justify(Gtk::JUSTIFY_CENTER);
  _hbox2.pack_start(_oneTimeCmdsLabel, true, true, 0);
  
  _clearOneTimeButton.set_label("Clear");
  _clearOneTimeButton.modify_font(bold_font);
  _hbox2.pack_start(_clearOneTimeButton, true, true, 0);
  _clearOneTimeButton.signal_clicked().connect(sigc::mem_fun(*this,
							     &EditWindow::_clearOneTimeCmds));
  
  _userOneTimeTextView.set_size_request(width,
					font_height * user_nlines);
  _userOneTimeTextView.modify_font(_defaultFont);
  _userOneTimeTextView.set_editable(true);
  _cmdsTable.attach(_userOneTimeTextView, 1, 2, 1, 2,
		    Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		    Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		    2, 4);
  
  _oneTimeListLabel.set_text("List of All One Time Only Commands");
  _oneTimeListLabel.modify_font(bold_font);
  _oneTimeListLabel.set_justify(Gtk::JUSTIFY_CENTER);
  _cmdsTable.attach(_oneTimeListLabel, 1, 2, 2, 3,
		    (Gtk::AttachOptions)0,
		    (Gtk::AttachOptions)0,
		    2, 4);
  
  _oneTimeCmdList.set_headers_visible(false);
  _oneTimeCmdList.set_size_request(width,
				   font_height * cmd_list_nlines);
  _oneTimeCmdList.modify_font(_defaultFont);

  // Set the padding for the list.  Note that the gtkmm-2.4 installed with
  // the Centos OS has a different API for the CellRenderer class so we can't
  // do this on that OS.
#ifndef OS_IS_CENTOS
  Gtk::CellRenderer *one_time_renderer =
    _oneTimeCmdList.get_column_cell_renderer(0);
  one_time_renderer->set_padding(0, 0);
#endif

  std::vector< std::string > other_cmds =
    UiCommandFactory::getInstance()->getOtherTemplates();
  
  for (size_t i = 0; i < other_cmds.size(); ++i)
    _oneTimeCmdList.append_text(other_cmds[i]);
  _oneTimeCmdList.signal_row_activated().connect(sigc::mem_fun(*this,
							       &EditWindow::_selectOneTimeCmd));

  _oneTimeCmdListScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC,
					   Gtk::POLICY_AUTOMATIC);
  _oneTimeCmdListScrolledWindow.add(_oneTimeCmdList);
  
  _cmdsTable.attach(_oneTimeCmdListScrolledWindow, 1, 2, 3, 4,
		    Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		    Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		    2, 4);

  // Commands help label

  _cmdsHelpLabel.set_text("To add command, position cursor in window then double-click command");
  _cmdsHelpLabel.modify_font(smaller_italic_font);
  _cmdsTable.attach(_cmdsHelpLabel, 0, 2, 4, 5,
		    (Gtk::AttachOptions)0,
		    (Gtk::AttachOptions)0,
		    2, 4);
  
  // Start/stop time widgets

  _vbox.pack_start(_startStopTable, false, true, 0);

  row = 0;
  gdouble dd = sii_return_swpfi_time_stamp(_frameIndex);
  DateTime data_time;
  data_time.setTimeStamp(dd);
  std::string data_time_string = data_time.toString();
  
  _startTimeLabel.set_text(" Start Time ");
  _startTimeLabel.modify_font(bold_font);
  _startStopTable.attach(_startTimeLabel, 0, 1, row, row+1,
			 (Gtk::AttachOptions)0,
			 (Gtk::AttachOptions)0,
			 2, 4);

  _origStartTime = data_time_string;
  _startTimeEntry.set_text(_origStartTime);
  _startTimeEntry.modify_font(_defaultFont);
  _startTimeEntry.signal_activate().connect(sigc::mem_fun(*this,
							  &EditWindow::_startTimeActivated));
  _startStopTable.attach(_startTimeEntry, 1, 3, row, row+1,
			 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
			 (Gtk::AttachOptions)0,
			 2, 4);
  
  _firstSweepButton.set_label("First Sweep");
  _firstSweepButton.modify_font(bold_font);
  _startStopTable.attach(_firstSweepButton, 3, 4, row, row+1,
			 (Gtk::AttachOptions)0,
			 (Gtk::AttachOptions)0,
			 2, 4);
  _firstSweepButton.signal_clicked().connect(sigc::mem_fun(*this,
							   &EditWindow::_firstSweep));

  ++row;

  _stopTimeLabel.set_text(" Stop Time ");
  _stopTimeLabel.modify_font(bold_font);
  _startStopTable.attach(_stopTimeLabel, 0, 1, row, row+1,
			 (Gtk::AttachOptions)0,
			 (Gtk::AttachOptions)0,
			 2, 4);

  _origStopTime = data_time_string;
  _stopTimeEntry.set_text(_origStopTime);
  _stopTimeEntry.modify_font(_defaultFont);
  _stopTimeEntry.signal_activate().connect(sigc::mem_fun(*this,
							 &EditWindow::_stopTimeActivated));
  _startStopTable.attach(_stopTimeEntry, 1, 3, row, row+1,
			 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
			 (Gtk::AttachOptions)0,
			 2, 4);
  
  _lastSweepButton.set_label("Last Sweep");
  _lastSweepButton.modify_font(bold_font);
  _startStopTable.attach(_lastSweepButton, 3, 4, row, row+1,
			 (Gtk::AttachOptions)0,
			 (Gtk::AttachOptions)0,
			 2, 4);
  _lastSweepButton.signal_clicked().connect(sigc::mem_fun(*this,
							  &EditWindow::_lastSweep));

  resetTimes();
}


/**********************************************************************
 * Destructor
 */

EditWindow::~EditWindow()
{
  // Delete the child windows

  delete _editCmdListHelpWindow;
  delete _editFilesWindow;
}


/**********************************************************************
 * addCommands()
 */

void EditWindow::addCommands(const std::vector< std::string > &command_list)
{
  // Get pointers to the command text buffers

  Glib::RefPtr< Gtk::TextBuffer > for_each_buffer =
    _userEachRayTextView.get_buffer();
  Glib::RefPtr< Gtk::TextBuffer > one_time_buffer =
    _userOneTimeTextView.get_buffer();
  
  // Loop through the text filling in the appropriate text buffers

  Glib::RefPtr< Gtk::TextBuffer > buffer = one_time_buffer;

  for (std::vector< std::string >::const_iterator line = command_list.begin();
       line != command_list.end(); ++line)
  {
    // Skip blank lines

    if (line->find_first_not_of(" \t") == std::string::npos)
      continue;
    
    // See if this line changes the type of commands that follow.

    if (line->find("only once") != std::string::npos)
    {
      buffer = one_time_buffer;
      continue;
    }
    
    if (line->find("for-each-ray") != std::string::npos)
    {
      buffer = for_each_buffer;
      continue;
    }

    // Now, add the command to the correct list

    // NOTE:  May need to keep iterator and use insert() instead, just in
    // case the user clicks the mouse in the buffer in the middle of this
    // processing

    buffer->insert_at_cursor(*line + "\n");
  }
}


/**********************************************************************
 * editStartStop()
 */

bool EditWindow::editStartStop(struct swp_file_input_control *sfic)
{
  bool okay = true;
  char message[256];
  
  // Process the start time

  std::string start_time_string = _startTimeEntry.get_text();
  std::size_t first_nonblank_pos = start_time_string.find_first_not_of(" \t\n");
  if (first_nonblank_pos == std::string::npos)
    start_time_string.clear();
  else
    start_time_string = start_time_string.substr(first_nonblank_pos);
  std::size_t last_nonblank_pos = start_time_string.find_last_not_of(" \t\n");
  if (last_nonblank_pos != std::string::npos)
    start_time_string =
      start_time_string.substr(0, last_nonblank_pos - first_nonblank_pos + 1);
  
  if (start_time_string == "0")
  {
    _startTime = DAY_ZERO;
    sfic->first_sweep_text = "first";
  }
  else
  {
    _startTime = DateTime::dtimeFromString(start_time_string);
    if (_startTime == 0) {
      sprintf(message, "%s is an unusable start time string\n",
	      start_time_string.c_str());
      okay = false;
    }
  }
  sfic->start_time = _startTime;

  // Process the stop time

  std::string stop_time_string = _stopTimeEntry.get_text();
  first_nonblank_pos = stop_time_string.find_first_not_of(" \t\n");
  if (first_nonblank_pos == std::string::npos)
    stop_time_string.clear();
  else
    stop_time_string = stop_time_string.substr(first_nonblank_pos);
  last_nonblank_pos = stop_time_string.find_last_not_of(" \t\n");
  if (last_nonblank_pos != std::string::npos)
    stop_time_string =
      stop_time_string.substr(0, last_nonblank_pos - first_nonblank_pos + 1);
  
  if (stop_time_string == "-1")
  {
    _stopTime = ETERNITY;
    sfic->last_sweep_text = "last";
  }
  else
  {
    _stopTime = DateTime::dtimeFromString(stop_time_string);
    if (_stopTime == 0)
    {
      sprintf (message, "%s is an unusable stop time string\n",
	       stop_time_string.c_str());
      okay = false;
    }
  }

  if (!okay)
  {
    sii_message (message);
    return false;
  }

  sfic->stop_time = _stopTime;

  return 1;
}


/**********************************************************************
 * resetTimes()
 */

void EditWindow::resetTimes()
{
  WW_PTR wwptr = solo_return_wwptr(_frameIndex);
  _startTime =  wwptr->sweep->start_time;
  _stopTime = (wwptr->view->type_of_plot & SOLO_TIME_SERIES)
    ? wwptr->sweep->stop_time : wwptr->sweep->start_time;
   
  // Set the start time

  DateTime start_time;
  start_time.setTimeStamp(_startTime);
  _origStartTime = start_time.toString();
  _startTimeEntry.set_text(_origStartTime);
   
  // Set the stop time

  DateTime stop_time;
  stop_time.setTimeStamp(_stopTime);
  _origStopTime = stop_time.toString();
  _stopTimeEntry.set_text(_origStopTime);
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _addNextBoundary()
 */

void EditWindow::_addNextBoundary()
{
  SeBoundaryList *sebs = SeBoundaryList::getInstance();

  se_cycle_bnd();
  sebs->lastOperation = MOVE_TO_NEXT_BOUNDARY;
}


/**********************************************************************
 * _changeBoundaryEditState()
 */

void EditWindow::_changeBoundaryEditState()
{
  SeBoundaryList *sebs = SeBoundaryList::getInstance();
  sebs->operateOutsideBnd = _boundaryEditOutsideChoice->get_active();
}


/**********************************************************************
 * _clearForEachRayCmds()
 */

void EditWindow::_clearForEachRayCmds()
{
  Glib::RefPtr< Gtk::TextBuffer > text_buffer =
    _userEachRayTextView.get_buffer();
  text_buffer->set_text("");
}


/**********************************************************************
 * _clearOneTimeCmds()
 */

void EditWindow::_clearOneTimeCmds()
{
  Glib::RefPtr< Gtk::TextBuffer > text_buffer =
    _userOneTimeTextView.get_buffer();
  text_buffer->set_text("");
}


/**********************************************************************
 * _clearBoundaries()
 */

void EditWindow::_clearBoundaries()
{
  SeBoundaryList *sebs = SeBoundaryList::getInstance();

  if (sebs->lastOperation == SeBoundaryList::BND_CLEARED)
  {
    sebs->lastOperation = SeBoundaryList::BND_OTHER;
    se_clr_all_bnds();
  }
  else
  {
    sebs->lastOperation = SeBoundaryList::BND_CLEARED;
    se_clr_current_bnd();
  }
}


/**********************************************************************
 * _closeWindows()
 */

void EditWindow::_closeWindows()
{
  _frameActive = false;
  
  if (_editCmdListHelpWindow != 0)
    _editCmdListHelpWindow->hide();
  if (_editFilesWindow != 0)
    _editFilesWindow->hide();

  hide();
}


/**********************************************************************
 * _createMenubar()
 */

void EditWindow::_createMenubar(Gtk::Box &container)
{
  // Specify the XML for the menus

  Glib::ustring ui_info =
    "<ui>"
    "  <menubar name='MenuBar'>"
    "    <menu action='FileMenu'>"
    "      <menuitem action='FileClose'/>"
    "      <separator/>"
    "      <menuitem action='FileResetTimes'/>"
    "      <separator/>"
    "      <menuitem action='FileSaveList'/>"
    "      <menuitem action='FileImportLatLon'/>"
    "      <separator/>"
    "      <menuitem action='FileAutoReplot'/>"
    "    </menu>"
    "    <menu action='BoundaryMenu'>"
    "      <menuitem action='BoundaryChoiceEditInside'/>"
    "      <menuitem action='BoundaryChoiceEditOutside'/>"
    "      <separator/>"
    "      <menuitem action='BoundaryDraw'/>"
    "    </menu>"
    "    <menu action='ReplotMenu'>"
    "      <menuitem action='ReplotReplot'/>"
    "    </menu>"
    "    <menu action='ExamplesMenu'>"
    "      <menuitem action='ExamplesBargenBrownUnfolding'/>"
    "      <menuitem action='ExamplesFlagFreckles'/>"
    "      <menuitem action='ExamplesFlagGlitches'/>"
    "      <menuitem action='ExamplesRegularHistogram'/>"
    "      <menuitem action='ExamplesIrregularHistogram'/>"
    "      <menuitem action='ExamplesRadialShear'/>"
    "      <menuitem action='ExamplesThresholding'/>"
    "    </menu>"
    "    <menu action='HelpMenu'>"
    "      <menuitem action='HelpOverview'/>"
    "      <menuitem action='HelpFile'/>"
    "      <menuitem action='HelpShortcuts'/>"
    "      <menuitem action='HelpBoundaries'/>"
    "      <menuitem action='HelpStartStop'/>"
    "      <menuitem action='HelpCommands'/>"
    "    </menu>"
    "    <menu action='CancelMenu'>"
    "      <menuitem action='CancelCancel'/>"
    "    </menu>"
    "  </menubar>"
    "</ui>";

  // Create the actions for the menus

  _actionGroup = Gtk::ActionGroup::create();
  
  // File menu actions

  _actionGroup->add(Gtk::Action::create("FileMenu", "File"));
  _actionGroup->add(Gtk::Action::create("FileClose", "Close"),
		    sigc::mem_fun(*this,
				  &EditWindow::_closeWindows));

  _actionGroup->add(Gtk::Action::create("FileResetTimes", "Reset Times"),
		    sigc::mem_fun(*this,
				  &EditWindow::resetTimes));

  _actionGroup->add(Gtk::Action::create("FileSaveList",
					"Save/List Cmds & Bnds "),
		    sigc::mem_fun(*this,
				  &EditWindow::_showEditFilesWidget));
  _actionGroup->add(Gtk::Action::create("FileImportLatLon",
					"Import Lat/Lon Bnds"),
		    sigc::mem_fun(*this,
				  &EditWindow::_showEditFilesWidget));

  _autoReplotToggle = Gtk::ToggleAction::create("FileAutoReplot",
						"Auto Replot", "",
						true);
  _actionGroup->add(_autoReplotToggle);

  // Boundary menu actions

  _actionGroup->add(Gtk::Action::create("BoundaryMenu", "Boundary"));

  Gtk::RadioAction::Group group_bndry_edit_choice;
  _boundaryEditInsideChoice =
    Gtk::RadioAction::create(group_bndry_edit_choice,
			     "BoundaryChoiceEditInside",
			     "Edit Inside");
  _actionGroup->add(_boundaryEditInsideChoice,
		    sigc::mem_fun(*this,
				  &EditWindow::_changeBoundaryEditState));
  _boundaryEditOutsideChoice =
    Gtk::RadioAction::create(group_bndry_edit_choice,
			     "BoundaryChoiceEditOutside",
			     "Edit Outside");
  _actionGroup->add(_boundaryEditOutsideChoice,
		    sigc::mem_fun(*this,
				  &EditWindow::_changeBoundaryEditState));
  

  _drawBoundariesToggle = Gtk::ToggleAction::create("BoundaryDraw",
                                                     "Draw Boundary", "",
                                                     true);
  _actionGroup->add(_drawBoundariesToggle);

  // Replot menu actions

  _actionGroup->add(Gtk::Action::create("ReplotMenu", "Replot"));

  _actionGroup->add(Gtk::Action::create("ReplotReplot",
					"Replot"),
		    sigc::mem_fun(*this,
				  &EditWindow::_replotLinks));

  // Examples menu actions

  _actionGroup->add(Gtk::Action::create("ExamplesMenu", "Examples"));

  _actionGroup->add(Gtk::Action::create("ExamplesBargenBrownUnfolding",
					"Bargen-Brown-unfolding"),
		    sigc::mem_fun(*this,
				  &EditWindow::_showBargenBrownUnfoldingExample));
  _actionGroup->add(Gtk::Action::create("ExamplesFlagFreckles",
					"flag-freckles"),
		    sigc::mem_fun(*this,
				  &EditWindow::_showFlagFrecklesExample));
  _actionGroup->add(Gtk::Action::create("ExamplesFlagGlitches",
					"flag-glitches"),
		    sigc::mem_fun(*this,
				  &EditWindow::_showFlagGlitchesExample));
  _actionGroup->add(Gtk::Action::create("ExamplesRegularHistogram",
					"regular-histogram"),
		    sigc::mem_fun(*this,
				  &EditWindow::_showRegularHistogramExample));
  _actionGroup->add(Gtk::Action::create("ExamplesIrregularHistogram",
					"irregular-histogram"),
		    sigc::mem_fun(*this,
				  &EditWindow::_showIrregularHistogramExample));
  _actionGroup->add(Gtk::Action::create("ExamplesRadialShear",
					"radial-shear"),
		    sigc::mem_fun(*this,
				  &EditWindow::_showRadialShearExample));
  _actionGroup->add(Gtk::Action::create("ExamplesThresholding",
					"thresholding"),
		    sigc::mem_fun(*this,
				  &EditWindow::_showThresholdingExample));

  // Help menu actions

  _actionGroup->add(Gtk::Action::create("HelpMenu", "Help"));

  _actionGroup->add(Gtk::Action::create("HelpOverview",
					"Overview Frame"),
		    sigc::mem_fun(*this,
				  &EditWindow::_displayHelpOverview));
  _actionGroup->add(Gtk::Action::create("HelpFile",
					"With File Menu"),
		    sigc::mem_fun(*this,
				  &EditWindow::_displayHelpFile));
  _actionGroup->add(Gtk::Action::create("HelpShortcuts",
					"With Cmd Editing"),
		    sigc::mem_fun(*this,
				  &EditWindow::_displayHelpShortcuts));
  _actionGroup->add(Gtk::Action::create("HelpBoundaries",
					"With Boundaries"),
		    sigc::mem_fun(*this,
				  &EditWindow::_displayHelpBoundaries));
  _actionGroup->add(Gtk::Action::create("HelpStartStop",
					"With Start-Stop Times"),
		    sigc::mem_fun(*this,
				  &EditWindow::_displayHelpStartStop));
  _actionGroup->add(Gtk::Action::create("HelpCommands",
					"Cmds with Help"),
		    sigc::mem_fun(*this,
				  &EditWindow::_showEditCmdHelpWidget));

  // Cancel menu actions

  _actionGroup->add(Gtk::Action::create("CancelMenu", "Cancel"));

  _actionGroup->add(Gtk::Action::create("CancelCancel",
					"Cancel"),
		    sigc::mem_fun(*this,
				  &EditWindow::_closeWindows));

  // Create the UI manager

  _uiManager = Gtk::UIManager::create();
  _uiManager->insert_action_group(_actionGroup);
  
  add_accel_group(_uiManager->get_accel_group());
  
  // Parse the XML

  try
  {
    _uiManager->add_ui_from_string(ui_info);
  }
  catch (const Glib::Error &ex)
  {
    std::cerr << "Building menus failed: " << ex.what();
  }
  
  // Get the menubar and add it to the container widget

  Gtk::Widget *menubar = _uiManager->get_widget("/MenuBar");
  if (menubar != 0)
    container.pack_start(*menubar,
			 false,      // expand
			 true,       // fill - ignored since expand is false
			 0);         // padding
}


/**********************************************************************
 * _displayExample()
 */

void EditWindow::_displayExample(const std::vector< std::string > &command_list)
{
  // Clear out the buffers

  _userEachRayTextView.get_buffer()->set_text("");
  _userOneTimeTextView.get_buffer()->set_text("");
  
  // Add the commands to the user buffers

  addCommands(command_list);
}


/**********************************************************************
 * _displayHelpBoundaries()
 */

void EditWindow::_displayHelpBoundaries()
{
  static const Glib::ustring help_text =
    Glib::ustring("When the user has popped up the edit widget, clicks in the data are\n") +
    "assumed to be defining a boundary. A line will be drawn from the\n" +
    "previous click to the next click. Typing a \"d\" will remove the last\n" +
    "point (and the last line). The color of the boundary is controled by\n" +
    "the \"Parameters + Colors\" widget.\n" +
    "\n" +
    "More than one boundary can be drawn for use in a pass through the\n" +
    "data.  The \"Add Next Bnd\" starts the next boundary.\n" +
    "\n" +
    "The \"Clear Bnd\" button clears the current boundary and if clicked twice\n" +
    "in succession will clear all boundaries in the set.\n" +
    " \n" +
    "Once a boundary set is used it is eligible to be saved as a file. Once\n" +
    "used the boundary set is added to a queue of boundary sets that can be\n" +
    "retrieved with the \"Prev Bnd Set\" button.\n" +
    "\n" +
    "To save a boundary set, click File->\"List Cmd & Bnd Files\" and click\n" +
    "the \"Save Boundary\" button. Solo assigns a file name based on the\n" +
    "current time, the user's id number, and the radar name. The comment\n" +
    "which the user can edit before clicking the \"Save Boundary\" button\n" +
    "will be appended to the file name.\n" +
    "\n" +
    "An ASCII boundary file should contain sets of pairs (Lat Lon) with\n" +
    "whitespace seperating the values\n" +
    "\n" +
    "CAUTION!\n" +
    "The editor responds to both left clicks and center mouse button clicks\n" +
    "(recentering the frame) in the data. The user may temporarily inhibit\n" +
    "responding to mouse clicks in the data by toggling the\n" +
    "\"Boundary->Draw Boundary\" menubar item.\n";
    
  sii_message(help_text);
}


/**********************************************************************
 * _displayHelpFile()
 */

void EditWindow::_displayHelpFile()
{
  static const Glib::ustring help_text =
    Glib::ustring("\"Reset Times\" will reset the start and stop times if they become\n") +
    "hopelessly mangled\n" +
    "\n" +
    "\"List Cmd & Bnd Files\" brings up a widget that allows you save\n" +
    "and/or retrieve files containing editor commands and boundaries.  The\n" +
    "editor command files are created with the prefix \"sed.\" and the\n" +
    "boundary files are created with the prefix \"bnd.\". Boundary files\n" +
    "contain binary data. The \"asb.\" files created along with the\n" +
    "boundary files are text files of boundary information to be used\n" +
    "elsewhere and cannot be imported into solo3.\n" +
    "\n" +
    "If a list of files appears below the directory for one of the three\n" +
    "types of files, you should be able to click on the file name and solo3\n" +
    "will import the contents of the file. To get files from other\n" +
    "directories, type the directory name in the directory window followed\n" +
    "enter \"Enter\". A list should appear and then you can click on the\n" +
    "file name\n" +
    "\n" +
    "\"Import Lat/Lon Bnds\" brings up this same widget. The files listed\n" +
    "are text files created externally that contain ordered pairs of\n" +
    "latitude and longitude seperated by whitespace. These ordered pairs\n" +
    "are used to generate a boundary. There should be one or more ordered\n" +
    "pairs per line. There should be at least three sets of ordered\n" +
    "pairs. And the file names should have the prefix \"llb.\".\n" +
    "\n" +
    "The \"Auto Replot\" toggle cause an automatic replot of the edited\n" +
    "data after a pass through the data caused by clicking \"Ok Do It!\"\n";

  sii_message(help_text);
}


/**********************************************************************
 * _displayHelpOverview()
 */

void EditWindow::_displayHelpOverview()
{
  static const Glib::ustring help_text =
    Glib::ustring("The editor applies the editing commands to the sweepfiles associated\n") +
    "with the frame from which the editor widgets were popped up. A user\n" +
    "must be careful to pop up the editor widget from the proper frame if\n" +
    "there is more than one radar displayed.\n" +
    "\n" +
    "The editing code is based on the previous version of solo and editing\n" +
    "command sets and boundaries are not unique to a particular frame.\n" +
    "\n" +
    "The editing of the data in the sweepfiles is controlled by a set of\n" +
    "commands typed in by the user. The source directory is determined by\n" +
    "the \"Sweepfiles Widget\" and the last sweepfile plotted. The user\n" +
    "can change the number of files edited via the \"Start Time\" and\n" +
    "\"Stop Time\" entries.\n" +
    "\n" +
    "There is a distinction between commands that must be executed for each\n" +
    "ray and commands that need to be executed only once at the beginning\n" +
    "of each pass through a set of sweepfiles.\n" +
    "\n" +
    "Almost all operators can use boundaries. See Help->\"With Boundaries\"\n" +
    "for more information about using boundaries.\n" +
    "\n" +
    " \"above\", \"below\" and \"between\" are to be substituted for\n" +
    "<where> in some of the commands.  \"between\" is inclusive of both\n" +
    "limits whereas \"above\" and \"below\" are exclusive of the limit.\n" +
    "\n" +
    "\n" +
    "Click Help->\"Cmds with help\" to get a list of commands with help\n" +
    "information. Click on the command to see the help information.\n" +
    "\n" +
    "The help information for the \"bad-flags-mask\" contains an\n" +
    "explanation of bad flag mask operations. The help information for\n" +
    "\"one-time\" contains explanations for many \"one-time\" commands\n";

  sii_message(help_text);
}


/**********************************************************************
 * _displayShortcuts()
 */

void EditWindow::_displayHelpShortcuts()
{
  static const Glib::ustring help_text =
    Glib::ustring("Clicking commands in the lists of commands bring them into a text\n") +
    "editing widget for modification. See Help->Shortcuts in the main\n" +
    "widget for editing shortcuts.\n" +
    "\n" +
    "Moving commands around with the cut and past commands can be done, but\n" +
    "care must be taken to capture the invisible end of line characters.\n" +
    "This can be done by clicking at the beginning of the line to be moved\n" +
    "and dragging the cursor down the the beginning of the next line and\n" +
    "typing the \"cut\" keystrokes (Ctrl+X). Next move the cursor to the\n" +
    "beginning of the line where the cut line is to be place and type the\n" +
    "\"paste\" keystrokes (Ctrl+V).\n" +
    "\n" +
    "The required elements of an editor command are the name of the command\n" +
    "which should occur at the beginning of each line and the arguments if\n" +
    "there are any which are enclosed in angle brackets \"<>\" as viewed in\n" +
    "the list of commands. All other modifiers can be eliminated. It is\n" +
    "also possible to use only enough characters of the command name to\n" +
    "distinguish it from others. e.g. \"flag\" or even \"flagged\" are not\n" +
    "unique.\n";

  sii_message(help_text);
}


/**********************************************************************
 * _displayHelpStartStop()
 */

void EditWindow::_displayHelpStartStop()
{
  static const Glib::ustring help_text =
    Glib::ustring("When the edit widget appears the start and stop times are for the\n") +
    "last plot in the frame. To cause the editor to process all the\n" +
    "files in the directory, the user can enter a \"0\" for the start\n" +
    "time and a \"-1\" for the stop time or click the buttons to the\n" +
    "right of the entries. Other time spans can be selected by editing\n" +
    "the existing entries.\n" +
    "\n" +
    "It is not necessary to include fractions of a second or even\n" +
    "minutes in order to provide a usable time.\n";
    
  sii_message(help_text);
}


/**********************************************************************
 * _doIt()
 */

void EditWindow::_doIt()
{
  struct solo_edit_stuff *seds = return_sed_stuff();
  SeBoundaryList *sebs = SeBoundaryList::getInstance();

  // For some reason, the gtkmm 2.4 on Centos has the simple get_text()
  // method defined in the include file, but it is not included in the
  // library.  So, for Centos I'm using the more complicated code that
  // seems to be required.  I'm leaving in the simpler code for other OS's
  // since I'm hoping we can remove it at some point in the future.

#ifdef OS_IS_CENTOS
  Glib::RefPtr< Gtk::TextBuffer > for_each_ray_buffer =
    _userEachRayTextView.get_buffer();
  seds->fer_lines =
    for_each_ray_buffer->get_text(for_each_ray_buffer->begin(),
				  for_each_ray_buffer->end());

  Glib::RefPtr< Gtk::TextBuffer > one_time_buffer =
    _userOneTimeTextView.get_buffer();
  seds->oto_lines =
    one_time_buffer->get_text(one_time_buffer->begin(),
			      one_time_buffer->end());
#else
  Glib::RefPtr< const Gtk::TextBuffer > for_each_ray_buffer =
    _userEachRayTextView.get_buffer();
  seds->fer_lines = for_each_ray_buffer->get_text();

  Glib::RefPtr< const Gtk::TextBuffer > one_time_buffer =
    _userOneTimeTextView.get_buffer();
  seds->oto_lines = one_time_buffer->get_text();
#endif
	
  sebs->lastOperation = SeBoundaryList::BND_OTHER;
  seds->pbs = NULL;
  WW_PTR wwptr = solo_return_wwptr(_frameIndex);
  bool time_series = wwptr->view->type_of_plot & SOLO_TIME_SERIES;
  bool automatic = wwptr->view->type_of_plot & TS_AUTOMATIC;
  bool down = wwptr->view->type_of_plot & TS_PLOT_DOWN;
  gdouble d_ctr = wwptr->view->ts_ctr_km;

  se_process_data(time_series, automatic, down, d_ctr, _frameIndex);
  resetTimes();
  if (_autoReplotToggle->get_active())
    sii_plot_data(_frameIndex, REPLOT_LOCK_STEP);
}


/**********************************************************************
 * _firstSweep()
 */

void EditWindow::_firstSweep()
{
  _startTime = DAY_ZERO;
  _startTimeEntry.set_text("0");
}


/**********************************************************************
 * _lastSweep()
 */

void EditWindow::_lastSweep()
{
  _stopTime = ETERNITY;
  _stopTimeEntry.set_text("-1");
}


/**********************************************************************
 * _replotLinks()
 */

void EditWindow::_replotLinks()
{
  sii_plot_data(_frameIndex, REPLOT_LOCK_STEP);
}


/**********************************************************************
 * _selectEachRayCmd()
 */

void EditWindow::_selectEachRayCmd(const Gtk::TreeModel::Path &path,
				   Gtk::TreeViewColumn *column)
{
  int row_number = atoi(path.to_string().c_str());
  std::string selected_cmd = _eachRayCmdList.get_text(row_number);
  
  Glib::RefPtr< Gtk::TextBuffer > text_buffer =
    _userEachRayTextView.get_buffer();
  text_buffer->insert_at_cursor(selected_cmd + "\n");
}


/**********************************************************************
 * _selectOneTimeCmd()
 */

void EditWindow::_selectOneTimeCmd(const Gtk::TreeModel::Path &path,
				   Gtk::TreeViewColumn *column)
{
  int row_number = atoi(path.to_string().c_str());
  std::string selected_cmd = _oneTimeCmdList.get_text(row_number);
  
  Glib::RefPtr< Gtk::TextBuffer > text_buffer =
    _userOneTimeTextView.get_buffer();
  text_buffer->insert_at_cursor(selected_cmd + "\n");
}


/**********************************************************************
 * _setPreviousBoundary()
 */

void EditWindow::_setPreviousBoundary()
{
  SeBoundaryList *sebs = SeBoundaryList::getInstance();

  se_prev_bnd_set();
  sebs->lastOperation = PREV_BND_SET;
}


/**********************************************************************
 * _showBargenBrownUnfoldingExample()
 */

void EditWindow::_showBargenBrownUnfoldingExample()
{
  static std::vector< std::string > example_text;
  
  if (example_text.size() == 0)
  {
    example_text.push_back("!  commands that need to be executed only once");
    example_text.push_back(" ");
    example_text.push_back("    BB-gates-averaged is 4");
    example_text.push_back("    BB-max-pos-folds is 1");
    example_text.push_back("    BB-max-neg-folds is 1");
    example_text.push_back("    BB-use-ac-wind");
    example_text.push_back(" ");
    example_text.push_back("!  operations executed for-each-ray ");
    example_text.push_back(" ");
    example_text.push_back("    duplicate VE in VG");
    example_text.push_back("    remove-aircraft-motion in VG");
    example_text.push_back("    duplicate VG in VU");
    example_text.push_back("    BB-unfolding in VU");
    example_text.push_back(" ");
  }

  _displayExample(example_text);
}


/**********************************************************************
 * _showEditCmdHelpWidget()
 */

void EditWindow::_showEditCmdHelpWidget()
{
  if (_editCmdListHelpWindow== 0)
    _editCmdListHelpWindow = new EditCmdListHelpWindow(this, _defaultFont);
  
  _editCmdListHelpWindow->show_all();
}


/**********************************************************************
 * _showEditFilesWidget()
 */

void EditWindow::_showEditFilesWidget()
{
  if (_editFilesWindow== 0)
    _editFilesWindow = new EditFilesWindow(this, _defaultFont);
  
  _editFilesWindow->show_all();
}


/**********************************************************************
 * _showFlagFrecklesExample()
 */

void EditWindow::_showFlagFrecklesExample()
{
  static std::vector< std::string > example_text;
  
  if (example_text.size() == 0)
  {
    example_text.push_back("!  commands that need to be executed only once");
    example_text.push_back(" ");
    example_text.push_back("    freckle-threshold is 5.0");
    example_text.push_back("    freckle-average is 3 gates");
    example_text.push_back(" ");
    example_text.push_back("!  operations executed for-each-ray ");
    example_text.push_back(" ");
    example_text.push_back("    clear-bad-flags");
    example_text.push_back("    flag-freckles in VD");
    example_text.push_back("    assert-bad-flags in VQ");
    example_text.push_back(" ");
  }

  _displayExample(example_text);
}


/**********************************************************************
 * _showFlagGlitchesExample()
 */

void EditWindow::_showFlagGlitchesExample()
{
  static std::vector< std::string > example_text;
  
  if (example_text.size() == 0)
  {
    example_text.push_back("!  commands that need to be executed only once");
    example_text.push_back(" ");
    example_text.push_back("    deglitch-min-gates is 9");
    example_text.push_back("    deglitch-radius is 5");
    example_text.push_back("    deglitch-threshold is 15");
    example_text.push_back(" ");
    example_text.push_back("!  operations executed for-each-ray ");
    example_text.push_back(" ");
    example_text.push_back("    duplicate DBZ DY");
    example_text.push_back("    clear-bad-flags");
    example_text.push_back("    flag-glitches in DBZ");
    example_text.push_back("    assert-bad-flags in DY");
  }

  _displayExample(example_text);
}


/**********************************************************************
 * _showIrregularHistogramExample()
 */

void EditWindow::_showIrregularHistogramExample()
{
  static std::vector< std::string > example_text;
  
  if (example_text.size() == 0)
  {
    example_text.push_back("!  commands that need to be executed only once");
    example_text.push_back(" ");
    example_text.push_back("area-histogram on DZ");
    example_text.push_back("irregular-histogram-bin from -40. to 10");
    example_text.push_back("irregular-histogram-bin from 10 to 40.");
    example_text.push_back("irregular-histogram-bin from 25 to 50.");
    example_text.push_back("irregular-histogram-bin from 40 to 80.");
    example_text.push_back(" ");
    example_text.push_back("!  operations executed for-each-ray");
    example_text.push_back(" ");
    example_text.push_back("do-histogram");
    example_text.push_back(" ");
  }

  _displayExample(example_text);
}


/**********************************************************************
 * _showRadialShearExample()
 */

void EditWindow::_showRadialShearExample()
{
  static std::vector< std::string > example_text;
  
  if (example_text.size() == 0)
  {
    example_text.push_back("!  commands that need to be executed only once");
    example_text.push_back(" ");
    example_text.push_back("offset-for-radial-shear is 5 gates");
    example_text.push_back(" ");
    example_text.push_back("!  operations executed for-each-ray");
    example_text.push_back(" ");
    example_text.push_back("radial-shear in VU put-in RS");
    example_text.push_back(" ");
  }

  _displayExample(example_text);
}


/**********************************************************************
 * _showRegularHistogramExample()
 */

void EditWindow::_showRegularHistogramExample()
{
  static std::vector< std::string > example_text;
  
  if (example_text.size() == 0)
  {
    example_text.push_back("!  commands that need to be executed only once");
    example_text.push_back(" ");
    example_text.push_back("count-histogram on VE");
    example_text.push_back("regular-histogram-parameters low -13. high 13. increment 1.");
    example_text.push_back(" ");
    example_text.push_back("!  operations executed for-each-ray");
    example_text.push_back(" ");
    example_text.push_back("do-histogram");
    example_text.push_back(" ");
  }

  _displayExample(example_text);
}


/**********************************************************************
 * _showThresholdingExample()
 */

void EditWindow::_showThresholdingExample()
{
  static std::vector< std::string > example_text;
  
  if (example_text.size() == 0)
  {
    example_text.push_back("!  operations executed for-each-ray (put one-time cmds before this line)");
    example_text.push_back(" ");
    example_text.push_back("set-bad-flags when NCP below .333");
    example_text.push_back("assert-bad-flags in VT");
    example_text.push_back(" ");
    example_text.push_back("!  could also say \"when SW above 4.\"");
    example_text.push_back(" ");
  }

  _displayExample(example_text);
}


/**********************************************************************
 * _startTimeActivated()
 */

void EditWindow::_startTimeActivated()
{
  // Get the user's entry

  Glib::ustring entered_string = _startTimeEntry.get_text();
  size_t first_non_white = entered_string.find_first_not_of(" \t");
  if (first_non_white != std::string::npos)
    entered_string = entered_string.substr(first_non_white);
  size_t last_non_white = entered_string.find_last_not_of(" \t");
  if (last_non_white != std::string::npos)
    entered_string = entered_string.substr(0, last_non_white+1);
  
  gdouble dtime;
  std::string time_string;
    
  if (entered_string.compare("0") == 0)
  {
    dtime = DAY_ZERO;
    time_string = "0";
  }
  else if (entered_string.compare("-1") == 0)
  {
    dtime = ETERNITY;
    time_string = "-1";
  }
  else
  {
    dtime = DateTime::dtimeFromString(entered_string);
    if (dtime == 0)
      return;
    DateTime time_value;
    time_value.setTimeStamp(dtime);
    time_string = time_value.toString();
  }

  _startTime = dtime;
  _startTimeEntry.set_text(time_string);
}


/**********************************************************************
 * _stopTimeActivated()
 */

void EditWindow::_stopTimeActivated()
{
  // Get the user's entry

  Glib::ustring entered_string = _stopTimeEntry.get_text();
  size_t first_non_white = entered_string.find_first_not_of(" \t");
  if (first_non_white != std::string::npos)
    entered_string = entered_string.substr(first_non_white);
  size_t last_non_white = entered_string.find_last_not_of(" \t");
  if (last_non_white != std::string::npos)
    entered_string = entered_string.substr(0, last_non_white+1);
  
  gdouble dtime;
  std::string time_string;
    
  if (entered_string.compare("0") == 0)
  {
    dtime = DAY_ZERO;
    time_string = "0";
  }
  else if (entered_string.compare("-1") == 0)
  {
    dtime = ETERNITY;
    time_string = "-1";
  }
  else
  {
    dtime = DateTime::dtimeFromString(entered_string);
    if (dtime == 0)
      return;
    DateTime time_value;
    time_value.setTimeStamp(dtime);
    time_string = time_value.toString();
  }

  _stopTime = dtime;
  _stopTimeEntry.set_text(time_string);
}
