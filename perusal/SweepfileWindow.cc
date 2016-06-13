#include <iostream>

#include <DataManager.hh>
#include <DateTime.hh>
#include <sii_externals.h>
#include <sii_links_widget.hh>
#include <sii_perusal.hh>
#include <solo2.hh>
#include <sp_accepts.hh>
#include <sp_basics.hh>
#include <sp_lists.hh>

#include "SweepfileWindow.hh"


/**********************************************************************
 * Constructor
 */

SweepfileWindow::SweepfileWindow(const Pango::FontDescription &default_font,
				 const int frame_index) :
  Gtk::Window(),
  _frameIndex(frame_index),
  _defaultFont(default_font),
  _table(4, 6, false),
  _sweepfilesListWindow(0)
{
  // Allocate space for the link information
  // NOTE: This was moved from update() so it is only done once when we allocate
  // the window.

  frame_configs[_frameIndex]->link_set[LI_SWPFI] =
    sii_new_links_info("Sweepfile Links", _frameIndex, FRAME_SWPFI_LINKS, FALSE);

  frame_configs[_frameIndex]->link_set[LI_LOCKSTEP] =
    sii_new_links_info("Lockstep Links", _frameIndex, FRAME_LOCKSTEP_LINKS, FALSE);

  // Create some fonts to use for our widgets

  Pango::FontDescription bold_font = _defaultFont;
  bold_font.set_weight(Pango::WEIGHT_SEMIBOLD);
  
  Pango::FontDescription smaller_font = _defaultFont;
  smaller_font.set_size(_defaultFont.get_size() - (2 * 1024));
  
  Pango::FontDescription smaller_italic_font = smaller_font;
  smaller_italic_font.set_style(Pango::STYLE_ITALIC);
  
  // Set the window title and border width

  char title_string[1024];
  
  sprintf(title_string, "Frame %d  Sweepfiles Widget", _frameIndex + 1);
  set_title(title_string);

  set_border_width(0);
  
  // Create a new vertical box for storing widgets

  _vbox0.set_homogeneous(false);
  _vbox0.set_spacing(4);
  add(_vbox0);
  
  // Create the menubar

  _createMenubar(_vbox0);
  
  _vbox.set_homogeneous(false);
  _vbox.set_spacing(4);
  _vbox.set_border_width(4);
  _vbox0.add(_vbox);
  
  _vbox.pack_start(_hbbox, false, false, 0);
  
  _sweepsButton.set_label("Sweeps");
  _sweepsButton.modify_font(_defaultFont);
  _hbbox.pack_start(_sweepsButton, true, true, 0);
  _sweepsButton.signal_clicked().connect(sigc::mem_fun(*this,
						       &SweepfileWindow::_showSweepsList));

  _replotButton.set_label("Replot");
  _replotButton.modify_font(_defaultFont);
  _hbbox.pack_start(_replotButton, true, true, 0);
  _replotButton.signal_clicked().connect(sigc::mem_fun(*this,
						       &SweepfileWindow::_replotThis));
  
  _okayButton.set_label("OK");
  _okayButton.modify_font(_defaultFont);
  _hbbox.pack_start(_okayButton, true, true, 0);
  _okayButton.signal_clicked().connect(sigc::mem_fun(*this,
						     &SweepfileWindow::_okay));
  
  _cancelButton.set_label("Cancel");
  _cancelButton.modify_font(_defaultFont);
  _hbbox.pack_start(_cancelButton, true, true, 0);
  _cancelButton.signal_clicked().connect(sigc::mem_fun(*this,
						       &SweepfileWindow::_closeWindows));
  
  _currSweepfileOrig = _getSweepfileLabel();
  _currSweepfileLabel.set_text(_currSweepfileOrig);
  _currSweepfileLabel.modify_font(_defaultFont);
  _vbox.pack_start(_currSweepfileLabel, false, false, 0);
  
  _vbox.pack_start(_table, true, true, 0);
  
  int row = 0;
  _directoryLabel.set_text(" Directory ");
  _directoryLabel.modify_font(_defaultFont);
  _table.attach(_directoryLabel, 0, 1, row, row+1,
		(Gtk::AttachOptions)0,
		(Gtk::AttachOptions)0,
		0, 0);
  
  _directoryOrig = sii_return_swpfi_dir(_frameIndex);
  _directoryEntry.set_text(_directoryOrig);
  _directoryEntry.modify_font(_defaultFont);
  _table.attach(_directoryEntry, 1, 4, row, row+1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		(Gtk::AttachOptions)0,
		0, 0);
  _directoryEntry.signal_activate().connect(sigc::mem_fun(*this,
							  &SweepfileWindow::_entryActivated));
  
  ++row;
  _radarLabel.set_text(" Radar ");
  _radarLabel.modify_font(_defaultFont);
  _table.attach(_radarLabel, 0, 1, row, row+1,
		(Gtk::AttachOptions)0,
		(Gtk::AttachOptions)0,
		0, 0);
  
  _radarOrig = _getSweepfileRadar();
  _radarEntry.set_text(_radarOrig);
  _radarEntry.modify_font(_defaultFont);
  _table.attach(_radarEntry, 1, 4, row, row+1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		(Gtk::AttachOptions)0,
		0, 0);
  _radarEntry.signal_activate().connect(sigc::mem_fun(*this,
						      &SweepfileWindow::_entryActivated));
  
  ++row;
  _startTimeLabel.set_text(" Start Time ");
  _startTimeLabel.modify_font(_defaultFont);
  _table.attach(_startTimeLabel, 0, 1, row, row+1,
		(Gtk::AttachOptions)0,
		(Gtk::AttachOptions)0,
		0, 0);
  
  _startTimeOrig = sii_return_swpfi_time_stamp(_frameIndex);
  _startTimeEntry.set_text(DateTime(_startTimeOrig).toString());
  _startTimeEntry.modify_font(_defaultFont);
  _table.attach(_startTimeEntry, 1, 4, row, row+1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		(Gtk::AttachOptions)0,
		0, 0);
  _startTimeEntry.signal_activate().connect(sigc::mem_fun(*this,
							  &SweepfileWindow::_entryActivated));
  
  ++row;
  _stopTimeLabel.set_text(" Stop Time ");
  _stopTimeLabel.modify_font(_defaultFont);
  _table.attach(_stopTimeLabel, 0, 1, row, row+1,
		(Gtk::AttachOptions)0,
		(Gtk::AttachOptions)0,
		0, 0);
  
  _stopTimeOrig = _startTimeOrig;
  _stopTimeEntry.set_text(DateTime(_stopTimeOrig).toString());
  _stopTimeEntry.modify_font(_defaultFont);
  _table.attach(_stopTimeEntry, 1, 4, row, row+1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		(Gtk::AttachOptions)0,
		0, 0);
  _stopTimeEntry.signal_activate().connect(sigc::mem_fun(*this,
							 &SweepfileWindow::_entryActivated));
  
  ++row;
  _scanModesLabel.set_text(" Scan Modes ");
  _scanModesLabel.modify_font(_defaultFont);
  _table.attach(_scanModesLabel, 2, 3, row, row+1,
		(Gtk::AttachOptions)0,
		(Gtk::AttachOptions)0,
		0, 0);
  
  _fixedInfoLabel.set_text(" Angle, Tolerance ");
  _fixedInfoLabel.modify_font(_defaultFont);
  _table.attach(_fixedInfoLabel, 3, 4, row, row+1,
		(Gtk::AttachOptions)0,
		(Gtk::AttachOptions)0,
		0, 0);
  
  ++row;
  _filterButton.set_label("Filter");
  _filterButton.modify_font(_defaultFont);
  _table.attach(_filterButton, 0, 1, row, row+1,
		(Gtk::AttachOptions)0,
		(Gtk::AttachOptions)0,
		0, 0);
  _filterButton.signal_toggled().connect(sigc::mem_fun(*this,
						       &SweepfileWindow::_setInfo));

  _scanModesOrig = "PPI,SUR";
  _scanModesEntry.set_text(_scanModesOrig);
  _scanModesEntry.modify_font(_defaultFont);
  _table.attach(_scanModesEntry, 2, 3, row, row+1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		(Gtk::AttachOptions)0,
		0, 0);
  
  _fixedAngleOrig = 0.5;
  _toleranceOrig = 0.25;
  _setFixedInfoEntry(_fixedAngleOrig, _toleranceOrig);
  _fixedInfoEntry.modify_font(_defaultFont);
  _table.attach(_fixedInfoEntry, 3, 4, row, row+1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		(Gtk::AttachOptions)0,
		0, 0);
  
  _timeSyncButton.set_label("Link time to frame 1");
  _timeSyncButton.modify_font(_defaultFont);
  _vbox.pack_start(_timeSyncButton, false, false, 0);
  _timeSyncButton.signal_toggled().connect(sigc::mem_fun(*this,
							 &SweepfileWindow::_setInfo));
  
  // Update the widget information based on the other window information

  update();
  
}


/**********************************************************************
 * Destructor
 */

SweepfileWindow::~SweepfileWindow()
{
  // Delete child windows

  delete _sweepfilesListWindow;
}


/**********************************************************************
 * setInfo()
 */

bool SweepfileWindow::setInfo(const int sweep_num)
{
  // I don't really understand the lock structures yet so not sure what
  // this is doing.  It does seem to copy the lock information from the
  // current window to all linked windows.  Of course, I don't know what
  // these locks are locking.

  _setLockInfo();

  // Update the window sweep information if the user entries are okay.

  if (!_setSweepInfo(sweep_num))
    return false;
  
  return true;
}


/**********************************************************************
 * update()
 */

void SweepfileWindow::update()
{
  WW_PTR wwptr = solo_return_wwptr(_frameIndex);

  _currSweepfileOrig = wwptr->show_file_info;
  _currSweepfileLabel.set_text(_currSweepfileOrig);
  
  _directoryOrig = wwptr->sweep->directory_name;
  _directoryEntry.set_text(_directoryOrig);
  
  _radarOrig = wwptr->sweep->radar_name;
  _radarEntry.set_text(_radarOrig);
  
  std::string start_time = _startTimeEntry.get_text();
  if (DateTime::relativeTime(start_time) == 0.0)
  {
    _startTimeOrig = wwptr->sweep->start_time;
    _startTimeEntry.set_text(DateTime(_startTimeOrig).toString());
  }
  
  _stopTimeOrig = wwptr->sweep->stop_time;
  _stopTimeEntry.set_text(DateTime(_stopTimeOrig).toString());
  
  _filterButton.set_active(wwptr->swpfi_filter_set);

  _scanModesOrig = wwptr->filter_scan_modes;
  _scanModesEntry.set_text(_scanModesOrig);

  _fixedAngleOrig = wwptr->filter_fxd_ang;
  _toleranceOrig = wwptr->filter_tolerance;
  _setFixedInfoEntry(_fixedAngleOrig, _toleranceOrig);

  _timeSyncButton.set_active(wwptr->swpfi_time_sync_set);
  
  // Set the links in the windows

  for (int jj = 0; jj < MAX_FRAMES; jj++)
  {
    frame_configs[_frameIndex]->link_set[LI_SWPFI]->link_set[jj] =
      (wwptr->sweep->linked_windows[jj]) ? TRUE : FALSE;
    frame_configs[_frameIndex]->link_set[LI_LOCKSTEP]->link_set[jj] =
      (wwptr->lock->linked_windows[jj]) ? TRUE : FALSE;
  }
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _closeWindows()
 */

void SweepfileWindow::_closeWindows()
{
  // Close the child windows

  if (_sweepfilesListWindow != 0)
    _sweepfilesListWindow->hide();
  
  // Close the associated link windows

  GtkWidget *widget =
    frame_configs[_frameIndex]->toplevel_windows[FRAME_SWPFI_LINKS];
  if (widget != 0)
    gtk_widget_hide (widget);

  widget = frame_configs[_frameIndex]->toplevel_windows[FRAME_LOCKSTEP_LINKS];
  if (widget != 0)
    gtk_widget_hide(widget);

  // Clear out the start time in case we've been doing relative time.

  _startTimeEntry.set_text("");
  
  hide();
}


/**********************************************************************
 * _createMenubar()
 */

void SweepfileWindow::_createMenubar(Gtk::Box &container)
{
  // Specify the XML for the menus

  Glib::ustring ui_info =
    "<ui>"
    "  <menubar name='MenuBar'>"
    "    <menu action='FileMenu'>"
    "      <menuitem action='FileClose'/>"
    "      <menuitem action='FileList'/>"
    "      <menuitem action='FileElectric'/>"
    "    </menu>"
    "    <menu action='LinksMenu'>"
    "      <menuitem action='LinksSet'/>"
    "    </menu>"
    "    <menu action='LockstepMenu'>"
    "      <menuitem action='LockstepSetLinks'/>"
    "    </menu>"
    "    <menu action='ReplotMenu'>"
    "      <menuitem action='ReplotLinks'/>"
    "      <menuitem action='ReplotAll'/>"
    "    </menu>"
    "    <menu action='HelpMenu'>"
    "      <menuitem action='HelpOverview'/>"
    "      <menuitem action='HelpLinks'/>"
    "      <menuitem action='HelpLockstep'/>"
    "      <menuitem action='HelpTimes'/>"
    "      <menuitem action='HelpFilter'/>"
    "      <menuitem action='HelpTimeLink'/>"
    "    </menu>"
    "  </menubar>"
    "</ui>";
  
  // Create the actions for the menus

  _actionGroup = Gtk::ActionGroup::create();
  
  // File menu actions

  _actionGroup->add(Gtk::Action::create("FileMenu", "File"));
  _actionGroup->add(Gtk::Action::create("FileClose", "Close"),
                    sigc::mem_fun(*this,
                                  &SweepfileWindow::_closeWindows));
  _actionGroup->add(Gtk::Action::create("FileList", "List Radars"),
                    sigc::mem_fun(*this,
                                  &SweepfileWindow::_listRadars));
  _electricToggle = Gtk::ToggleAction::create("FileElectric",
					      "Electric Swpfis", "",
					      true);
  _actionGroup->add(_electricToggle);

  // SwpfiLinks menu actions

  _actionGroup->add(Gtk::Action::create("LinksMenu", "SwpfiLinks"));
  _actionGroup->add(Gtk::Action::create("LinksSet", "Set Links"),
                    sigc::mem_fun(*this,
                                  &SweepfileWindow::_setSweepfileLinks));

  // Lockstep menu actions

  _actionGroup->add(Gtk::Action::create("LockstepMenu", "Lockstep"));
  _actionGroup->add(Gtk::Action::create("LockstepSetLinks", "Set Links"),
                    sigc::mem_fun(*this,
                                  &SweepfileWindow::_setLockstepLinks));

  // Replot menu actions

  _actionGroup->add(Gtk::Action::create("ReplotMenu", "Replot"));
  _actionGroup->add(Gtk::Action::create("ReplotLinks", "Replot Links"),
                    sigc::mem_fun(*this,
                                  &SweepfileWindow::_replotLinks));
  _actionGroup->add(Gtk::Action::create("ReplotAll", "Replot All"),
                    sigc::mem_fun(*this,
                                  &SweepfileWindow::_replotAll));

  // Help menu actions

  _actionGroup->add(Gtk::Action::create("HelpMenu", "Help"));
  _actionGroup->add(Gtk::Action::create("HelpOverview", "Overview"),
                    sigc::mem_fun(*this,
                                  &SweepfileWindow::_displayHelpOverview));
  _actionGroup->add(Gtk::Action::create("HelpLinks", "With Links"),
                    sigc::mem_fun(*this,
                                  &SweepfileWindow::_displayHelpLinks));
  _actionGroup->add(Gtk::Action::create("HelpLockstep", "With Lockstep"),
                    sigc::mem_fun(*this,
                                  &SweepfileWindow::_displayHelpLockstep));
  _actionGroup->add(Gtk::Action::create("HelpTimes", "With Times"),
                    sigc::mem_fun(*this,
                                  &SweepfileWindow::_displayHelpTimes));
  _actionGroup->add(Gtk::Action::create("HelpFilter", "With the Filter"),
                    sigc::mem_fun(*this,
                                  &SweepfileWindow::_displayHelpFilter));
  _actionGroup->add(Gtk::Action::create("HelpTimeLink", "Time Link"),
                    sigc::mem_fun(*this,
                                  &SweepfileWindow::_displayHelpTimeLink));

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
  {
    menubar->modify_font(_defaultFont);
    container.pack_start(*menubar,
                         false,      // expand
                         true,       // fill - ignored since expand is false
                         0);         // padding
  }
  
}


/**********************************************************************
 * _displayHelpFilter()
 */

void SweepfileWindow::_displayHelpFilter()
{
  static const Glib::ustring help_text =
    Glib::ustring("The \"Filter\" requires later versions of sweepfile names that\n") +
    "contain the fixed angle, scan mode and volume number. Pressing\n" +
    "one of the arrow keys with the filter on causes solo3 to look\n" +
    "for the next volume where the criteria are satisfied.\n";
  
  sii_message(help_text);
}


/**********************************************************************
 * _displayHelpLinks()
 */

void SweepfileWindow::_displayHelpLinks()
{
  static const Glib::ustring help_text =
    Glib::ustring("The SwpfiLinks submenu enables the selection of a set frames that\n") +
    "display data from the same sweepfile and to enable the display of data\n" +
    "from more than one radar or directory in the same set of frames.\n" +
    "\n" +
    "Buttons that appear sunken or pushed in indicate frames that are part\n" +
    "of the link set. Buttons that appear to stand out or raised indicate\n" +
    "frames that are not part of the link set.\n" +
    "\n" +
    "Clicking the \"OK\" button will save information from this widget.\n";
  
  sii_message(help_text);
}


/**********************************************************************
 * _displayHelpLockstep()
 */

void SweepfileWindow::_displayHelpLockstep()
{
  static const Glib::ustring help_text =
    Glib::ustring("The Lockstep submenu enables the selection of a set of frames\n") +
    "that will plot in response to an action that plots a lockstep\n" +
    "such as the <return> key or left or right arrows on the keyboard.\n" +
    "\n" +
    "Buttons that appear sunken or pushed in indicate frames that are part\n" +
    "of the link set. Buttons that appear to stand out or raised indicate\n" +
    "frames that are not part of the link set.\n" +
    "\n" +
    "Clicking the \"OK\" button will save information from this widget.\n";
  
  sii_message(help_text);
}


/**********************************************************************
 * _displayHelpOverview()
 */

void SweepfileWindow::_displayHelpOverview()
{
  static const Glib::ustring help_text =
    Glib::ustring("The \"File\" menu provides the option to \"Close\" or remove the\n") +
    "Sweepfiles widget. This is the same as clicking the \"Cancel\"\n" +
    "button. No information from the widget is saved.\n" +
    "\n" +
    "Selecting the \"List Radars\" File menu item will generate a list of\n" +
    "the radars for which there are sweepfiles in the current directory.\n" +
    "If there are more than one, edit the \"Radar\" entry to select a radar.\n" +
    "\n" +
    "The \"Electric Swpfis\" File menu toggle item causes an immediate plot\n" +
    "of the clicked sweepfile in the Sweepfiles List widget. If the toggle\n" +
    "button appears raised, it is inactive. If it appears pushed in or\n" +
    "depressed it is active.\n" +
    "\n" +
    "The \"Sweeps\" button shows a list of sweepfiles for the radar.\n" +
    "Clicking the button again will remove the list widget. Clicking an\n" +
    "item in the list selects that sweepfile for plotting.\n" +
    "\n" +
    "The \"Replot\" button saves the current entries and causes an\n" +
    "immediate replot of the frame. The \"Replot \" menu bar item\n" +
    "enables replotting all the frames in the current lockstep\n" +
    "or all locksteps.\n" +
    "\n" +
    "The \"OK\" button saves the current entries and removes the\n" +
    "widget.\n";

  sii_message(help_text);
}


/**********************************************************************
 * _displayHelpTimeLink()
 */

void SweepfileWindow::_displayHelpTimeLink()
{
  static const Glib::ustring help_text =
    Glib::ustring("The \"Link Time\" toggle button enables sweep sets to be synchronized\n") +
    "by the start time of the sweepfile displayed in frame 1. e.g. the\n" +
    "display of two or more radars could advance together in time. Also\n" +
    "lower time resolution datasets such as the rainfall accumulations\n" +
    "could move (stay synchronized) with the data in frame 1.\n" +
    "\n" +
    "A sweep set is set of sweeps distinguished by the radar name or a set\n" +
    "of sweeps residing in a different directory from the the set of sweeps\n" +
    "selected in frame 1. \"SwpfiLinks\" settings are used to differentiate\n" +
    "sweep sets.\n" +
    "\n" +
    "The Link Time button must be toggled in one frame of each sweep set\n" +
    "where synchronization is desired.\n";
  
  sii_message(help_text);
}


/**********************************************************************
 * _displayHelpTimes()
 */

void SweepfileWindow::_displayHelpTimes()
{
  static const Glib::ustring help_text =
    Glib::ustring("The start and stop times are mainly for controlling time series (color\n") +
    "bscan) plots. Times can be absolute or relative. Relative times are\n" +
    "preceeded by either a \"+\" or \"-\" with no space between the sign\n" +
    "and the following number. Relative times such as \"+5m\" imply a 5\n" +
    "minute shift forward or \"-1h\" a backward shift of one hour. The\n" +
    "absence of an \"m\" or \"h\" implies seconds.\n" +
    "\n" +
    "Absolute time should follow the example in the entry window\n" +
    "but can be truncated to the nearest hour.\n" +
    "\n" +
    "If you're not in time series mode and you type \"+5m\" in the\n" +
    "start time entry followed by a <return>. The next sweepfile\n" +
    "closest to 5 minutes ahead of the current time will be plotted.\n";
  
  sii_message(help_text);
}


/**********************************************************************
 * _entryActivated()
 */

void SweepfileWindow::_entryActivated()
{
  if (setInfo(-1))
  {
    sii_plot_data(_frameIndex, REPLOT_LOCK_STEP);
    update();
  }
}


/**********************************************************************
 * _getFilterInfo()
 */

void SweepfileWindow::_getFilterInfo(std::string &scan_modes,
				     float &fixed_angle, float &tolerance)
{
  // If the filter toggle isn't set, then don't do anything

  if (!_filterButton.get_active())
    return;
  
  // Get the scan modes

  _scanModesOrig = _scanModesEntry.get_text();
  scan_modes = _scanModesOrig;
  
  // Get the fixed info

  if (!_getFixedInfoEntry(_fixedAngleOrig, _toleranceOrig))
    return;
  
  fixed_angle = _fixedAngleOrig;
  tolerance = _toleranceOrig;
}


/**********************************************************************
 * _getSweepfileLabel()
 */

std::string SweepfileWindow::_getSweepfileLabel() const
{
  WW_PTR wwptr = solo_return_wwptr(_frameIndex);
  return wwptr->lead_sweep->show_file_info;
}


/**********************************************************************
 * _getSweepfileRadar()
 */

std::string SweepfileWindow::_getSweepfileRadar() const
{
  WW_PTR wwptr = solo_return_wwptr(_frameIndex)->lead_sweep;
  return wwptr->sweep->radar_name;
}


/**********************************************************************
 * _listRadars()
 */

void SweepfileWindow::_listRadars()
{
  // Generate the radar list from the data.  This call puts the radar list
  // into the solo_perusal_info.

  solo_gen_radar_list(_frameIndex);
  struct solo_perusal_info *spi = solo_return_winfo_ptr();

  // Construct the radar list

  std::string radar_list;
     
  for (size_t i = 0; i < spi->list_radars.size(); ++i)
  {
    if (i != 0)
      radar_list += " ";
       
    radar_list += spi->list_radars[i];
  }
     
  _radarEntry.set_text(radar_list);
}


/**********************************************************************
 * _okay()
 */

void SweepfileWindow::_okay()
{
  setInfo(-1);
  _closeWindows();
}


/**********************************************************************
 * _replotAll()
 */

void SweepfileWindow::_replotAll()
{
  if (setInfo(-1))
    sii_plot_data(_frameIndex, REPLOT_ALL);
  update();
}


/**********************************************************************
 * _replotLinks()
 */

void SweepfileWindow::_replotLinks()
{
  if (setInfo(-1))
    sii_plot_data(_frameIndex, REPLOT_LOCK_STEP);
  update();
}


/**********************************************************************
 * _replotThis()
 */

void SweepfileWindow::_replotThis()
{
  if (setInfo(-1))
    sii_plot_data(_frameIndex, REPLOT_THIS_FRAME);
  update();
}


/**********************************************************************
 * _setInfo()
 */

void SweepfileWindow::_setInfo()
{
  setInfo(-1);
}


/**********************************************************************
 * _setLockInfo()
 */

void SweepfileWindow::_setLockInfo()
{
  // Get a local list of the lockstep links

  bool lockstep_links[MAX_FRAMES];
  
  for (int jj = 0; jj < MAX_FRAMES; ++jj)
  {
    if (jj == _frameIndex ||
	frame_configs[_frameIndex]->link_set[LI_LOCKSTEP]->link_set[jj])
      lockstep_links[jj] = true;
    else
      lockstep_links[jj] = false;
  }

  // Get pointers to the window information

  WW_PTR wwptr = solo_return_wwptr(_frameIndex);
  wwptr->lock->changed = YES;
  struct solo_perusal_info *spi = solo_return_winfo_ptr();

  // Copy the links to the window structure.  While there, look for dangling
  // links.

  int dangling_links[SOLO_MAX_WINDOWS];

  for (int ww = 0; ww < SOLO_MAX_WINDOWS; ww++)
  {
    if (!spi->active_windows[ww])
      continue;

    if (wwptr->lock->linked_windows[ww] && !lockstep_links[ww])
      dangling_links[ww] = YES;
    else
      dangling_links[ww] = NO;

    wwptr->lock->linked_windows[ww]  = lockstep_links[ww];
  }

  // Now copy this lock struct to the other linked windows

  for (int ww = 0; ww < SOLO_MAX_WINDOWS; ww++)
  {
    if (!spi->active_windows[ww])
      continue;

    WW_PTR wwptrc = solo_return_wwptr(ww); /* pointer to next linked frame */
    wwptrc->lock->changed = YES;

    if (ww == _frameIndex || !wwptr->lock->linked_windows[ww])
      continue;

    memcpy(wwptrc->lock, wwptr->lock, sizeof(struct solo_plot_lock));
    wwptrc->lock->window_num = ww;
  }

  // Now deal with the dangling links

  for (int ww = 0; ww < SOLO_MAX_WINDOWS; ww++)
  {
    if (!spi->active_windows[ww])
      continue;
    if (!dangling_links[ww])
      continue;

    // The dangling list becomes the links for the dangling frames

    WW_PTR wwptrc = solo_return_wwptr(ww);
    for (int j = 0; j < SOLO_MAX_WINDOWS; j++)
      wwptrc->lock->linked_windows[j] = dangling_links[j];
  }
}


/**********************************************************************
 * _setLockstepLinks()
 */

void SweepfileWindow::_setLockstepLinks()
{
  show_links_widget(frame_configs[_frameIndex]->link_set[LI_LOCKSTEP]); 
}


/**********************************************************************
 * _setSweepfileLinks()
 */

void SweepfileWindow::_setSweepfileLinks()
{
  show_links_widget(frame_configs[_frameIndex]->link_set[LI_SWPFI]); 
}


/**********************************************************************
 * _setSweepInfo()
 */

bool SweepfileWindow::_setSweepInfo(const int sweep_num)
{
  // This routine examines the info from the sweep widget and
  // updates the window sweep info if it is ok

  // Get the pointers associated with this frame

  WW_PTR wwptr = solo_return_wwptr(_frameIndex);
  struct solo_perusal_info *spi = solo_return_winfo_ptr();

  // Check for a time series plot.

  int time_series = wwptr->view->type_of_plot & SOLO_TIME_SERIES;

  // Set the time sync flag based on the user's choice

  wwptr->swpfi_time_sync_set = _timeSyncButton.get_active();
  
  // Set the filter flag based on the user's choice

  if (_filterButton.get_active())
    wwptr->swpfi_filter_set = _frameIndex + 1;
  else
    wwptr->swpfi_filter_set = 0;
  
  if (wwptr->swpfi_filter_set)
    _getFilterInfo(wwptr->filter_scan_modes,
		   wwptr->filter_fxd_ang, wwptr->filter_tolerance);

  // Construct the data directory path

  std::string new_dir_name = _directoryEntry.get_text();
  
  if (new_dir_name[new_dir_name.size()-1] != '/')
    new_dir_name += "/";

  // Set the sweepfile start time

  std::string start_time_text = _startTimeEntry.get_text();
  
  if (start_time_text == wwptr->start_time_text)
  {
    // Clear out this text, we don't want to analyse this string
    // in the time series logic below unless it's been changed

    start_time_text = "";
  }
  else if (!time_series)
  {
    DateTime start_time;
    start_time.setTimeStamp(wwptr->d_sweepfile_time_stamp);
      
    double start_time_relative = start_time.relativeTime(start_time_text);

    if (start_time_relative != 0.0)
      wwptr->d_sweepfile_time_stamp += start_time_relative;
    else if (start_time.parse(start_time_text))
      wwptr->d_sweepfile_time_stamp = start_time.getTimeStamp();
  }

  // Set the sweepfile stop time

  std::string stop_time_text = _stopTimeEntry.get_text();
  
  if (stop_time_text == wwptr->stop_time_text)
  {
    stop_time_text = "";
  }
    
  // Trigger rescans of the data directory for every window using this
  // directory.
  // NOTE: This seems to be triggering rescans of the old data directory
  // rather than the new one???  Not sure why this is done.

  solo_trigger_dir_rescans(wwptr->sweep->directory_name);

  // Compile the list of sweep files in the new sweepfile directory

  int num_files =
    DataManager::getInstance()->compileFileList(_frameIndex, new_dir_name);
  if (num_files <= 0)
  {
    char message[256];
    
    sprintf(message, "No sweep files in dir: %s\n", new_dir_name.c_str());
    sii_message(message);
    return false;
  }
  strncpy(wwptr->sweep->directory_name, new_dir_name.c_str(), 128);

  // See if the current radar exists in the new directory.  If it doesn't,
  // switch to the first radar in the directory

  std::string radar_name;
  std::string radar_names = _radarEntry.get_text();
  std::size_t delim_pos = radar_names.find_first_of(" \t,");

  if (delim_pos == std::string::npos)
    radar_name = radar_names;
  else
    radar_name = radar_names.substr(0, delim_pos);
  
  int radar_num = DataManager::getInstance()->getRadarNumData(_frameIndex,
							      radar_name);
  
  if (radar_num < 0)
  {
    char message[256];
    
    sprintf(message, "No data exists for radar %s in %s\n",
	    DataManager::getInstance()->getRadarNameData(_frameIndex, radar_num),
	    new_dir_name.c_str());
    radar_num = 0;
    sprintf(message+strlen(message), "switching to radar %s\n",
	    DataManager::getInstance()->getRadarNameData(_frameIndex, radar_num));
    g_message(message);
  }

  wwptr->sweep->radar_num = radar_num;
  wwptr->ddir_radar_num = radar_num;
  strcpy(wwptr->radar->radar_name, radar_name.c_str());
  strncpy(wwptr->sweep->radar_name, radar_name.c_str(), 16);
  wwptr->sweep->version_num = DataManager::LATEST_VERSION;

  std::string file_name = wwptr->sweep->file_name;
  wwptr->sweep->start_time =
    DataManager::getInstance()->getDataInfoTime(_frameIndex,
						wwptr->sweep->radar_num,
						wwptr->d_sweepfile_time_stamp,
						DataManager::TIME_NEAREST,
						DataManager::LATEST_VERSION,
						wwptr->show_file_info,
						file_name);
  strncpy(wwptr->sweep->file_name, file_name.c_str(), 128);
  wwptr->d_sweepfile_time_stamp = wwptr->sweep->start_time;
  wwptr->sweep->time_stamp = (int32_t) wwptr->sweep->start_time;

  double time_span = wwptr->sweep->stop_time - wwptr->sweep->start_time;
  
  if (time_span < .001)
  {
    time_span = 20.0;
  }
  wwptr->sweep->stop_time = wwptr->sweep->start_time + time_span;

  if (sweep_num >= 0)
  {
    // Select this sweep.

    std::string file_name = wwptr->sweep->file_name;
    double dtime = 
      DataManager::getInstance()->getDataInfo(_frameIndex, wwptr->sweep->radar_num,
					      sweep_num,
					      wwptr->sweep->version_num,
					      wwptr->show_file_info,
					      file_name);
    strncpy(wwptr->sweep->file_name, file_name.c_str(), 128);
    wwptr->sweep->start_time = dtime;
    wwptr->d_sweepfile_time_stamp = dtime;
    wwptr->sweep->stop_time = (time_series) ? dtime + time_span : dtime;
    wwptr->sweep->time_stamp = (int32_t)dtime;
  }
    
  if (time_series)
  {
    // Look at start and end time text

    DateTime start_time;
    start_time.setTimeStamp(wwptr->sweep->start_time);
    start_time.setTime(0, 0, 0);

    double start_time_relative = start_time.relativeTime(start_time_text);
    
    if (start_time_relative != 0.0)
    {
      wwptr->sweep->start_time += start_time_relative;
    }
    else if (start_time.parse(start_time_text))
    {
      wwptr->sweep->start_time = start_time.getTimeStamp();
    }

    // NOTE: Should setTimeStamp() be using stop time instead of start
    // time???

    wwptr->sweep->stop_time = wwptr->sweep->start_time + time_span;
    DateTime stop_time;
    stop_time.setTimeStamp(wwptr->sweep->start_time);
    stop_time.setTime(0, 0, 0);

    double stop_time_relative = stop_time.relativeTime(stop_time_text);
    
    if (stop_time_relative != 0.0)
    {
      wwptr->sweep->stop_time = wwptr->sweep->start_time + stop_time_relative;
    }
    else if (stop_time.parse(stop_time_text))
    {
      wwptr->sweep->stop_time = stop_time.getTimeStamp();
    }
    if (wwptr->sweep->stop_time <= wwptr->sweep->start_time)
    {
      wwptr->sweep->stop_time = wwptr->sweep->start_time + time_span;
    }
    start_time.setTimeStamp(wwptr->sweep->start_time);
    wwptr->start_time_text = start_time.toString();

    stop_time.setTimeStamp(wwptr->sweep->stop_time);
    wwptr->stop_time_text = stop_time.toString();
  }
  wwptr->sweep->changed = YES;
  wwptr->sweep->time_modified = time(0);

  wwptr->sweep->sweep_skip = 1;

  // Deal with the links

  bool linked_windows[MAX_FRAMES];
    
  for (int jj = 0; jj < MAX_FRAMES; ++jj)
  {
    if (jj == _frameIndex ||
	frame_configs[_frameIndex]->link_set[LI_SWPFI]->link_set[jj])
      linked_windows[jj] = true;
    else
      linked_windows[jj] = false;
  }

  int dangling_links[SOLO_MAX_WINDOWS];
  
  for (int ii = 0; ii < SOLO_MAX_WINDOWS; ii++)
  {
    if (!(spi->active_windows[ii]))
      continue;

    if (wwptr->sweep->linked_windows[ii] && !linked_windows[ii])
      dangling_links[ii] = YES;
    else
      dangling_links[ii] = NO;

    wwptr->sweep->linked_windows[ii]  = linked_windows[ii];
  }

  // Now copy this sweep struct to the other linked windows

  for (int ii = 0; ii < SOLO_MAX_WINDOWS; ii++)
  {
    if (!(spi->active_windows[ii]))
      continue;

    if (ii == _frameIndex || !wwptr->sweep->linked_windows[ii])
      continue;
    solo_cpy_sweep_info(_frameIndex, ii);
  }

  // Now deal with the dangling links

  for (int ii = 0; ii < SOLO_MAX_WINDOWS; ii++)
  {
    if (!(spi->active_windows[ii]))
      continue;
    
    if (!dangling_links[ii])
      continue;

    // The dangling list becomes the links for the dangling frames

    WW_PTR wwptrc = solo_return_wwptr(ii);
    for ( int j = 0; j < SOLO_MAX_WINDOWS; j++) 
      wwptrc->sweep->linked_windows[j] = dangling_links[j];
  }

  // Update the sweepfiles list, if it is activated

  if (_sweepfilesListWindow)
    _sweepfilesListWindow->resetSweepfileList();
  
  solo_worksheet();

  return true;
}


/**********************************************************************
 * _showSweepsList()
 */

void SweepfileWindow::_showSweepsList()
{
  if (_sweepfilesListWindow == 0)
    _sweepfilesListWindow = new SweepfilesListWindow(this, _defaultFont);
  
  // For some reason, the gtkmm 2.4 version of the Window widget doesn't have
  // the get_visible() method.  So, on that OS we can't check if the window
  // is currently visible and will just bring the window up if the user chooses
  // it.  To close the window, the user will have to use the window controls.
#ifndef OS_IS_CENTOS
  if (_sweepfilesListWindow->get_visible())
    _sweepfilesListWindow->hide();
  else
#endif
    _sweepfilesListWindow->show_all();
}
