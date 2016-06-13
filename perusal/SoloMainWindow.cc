#include <iostream>

#include <gdk/gdkkeysyms.h>
#include <gdkmm/cursor.h>
#include <gtkmm/action.h>
#include <gtkmm/toggleaction.h>

#include <date_stamp.h>
#include <SeBoundaryList.hh>
#include <sii_callbacks.hh>
#include <sii_config_stuff.hh>
#include <sii_frame_menu.hh>
#include <sii_param_widgets.hh>
#include <sii_utils.hh>
#include <solo_window_structs.h>
#include <solo2.hh>
#include <soloii.hh>
#include <sp_basics.hh>
#include <SoloState.hh>

#include "SoloMainWindow.hh"


/**********************************************************************
 * Constructor
 */

SoloMainWindow::SoloMainWindow(const Pango::FontDescription &default_font) :
  Gtk::Window(),
  _rootDir("./"),
  _defaultFont(default_font),
  _table(0),
  _configWindow(_defaultFont)
{
  // Set the window title

  set_title("solo3");

  // Set the events that the main window will receive.
  //   KEY_PRESS_MASK:  KEY_PRESS
  //   STRUCTURE_MASK:  CONFIGURE, DESTROY, MAP, UNMAP

  set_events(Gdk::KEY_PRESS_MASK | Gdk::STRUCTURE_MASK);

  // Connect the key release event handler.  We are using key release rather
  // than key press because some keys don't seem to generate a key press even
  // for some reason.

  signal_key_release_event().connect(sigc::mem_fun(*this,
						   &SoloMainWindow::_keyReleaseEvent));

  // Allow the user to shrink the window, if desired

  Gdk::Geometry geometry;
  geometry.min_width = 1;
  geometry.min_height = 1;
  
  set_geometry_hints(*this, geometry, Gdk::HINT_MIN_SIZE);

  // Create the box container

  // NOTE: This is a vertical container box
  // NOTE: This should be changed from a Box to a Grid since Box will go away
  // sometime in the future.  For now, get this working properly.

  _mainVbox.set_homogeneous(false);
  _mainVbox.set_spacing(0);
  
  add(_mainVbox);
  _mainVbox.set_border_width(0);
  _mainVbox.show();
  
  // NOTE: Set up the main menu

  _createMainMenu(_mainVbox);
  
  // NOTE: A widget used to catch events for widgets which do not have their
  // own window

  // NOTE: Need to upgrade this to gtkmm and encapsulate it here.  It is used
  // multiple places now.

  _mainVbox.add(_eventBox);
  _eventBox.set_events(Gdk::EventMask(Gdk::ENTER_NOTIFY_MASK |
				      Gdk::LEAVE_NOTIFY_MASK |
				      Gdk::CONFIGURE));
  _eventBox.signal_enter_notify_event().connect(sigc::mem_fun(*this,
							      &SoloMainWindow::_eventBoxEntered));
  _eventBox.signal_configure_event().connect(sigc::mem_fun(*this,
							   &SoloMainWindow::_eventBoxConfigured));

  resetTable(2, 2);
  
  // Set up the child windows

  _configWindow.setConfigDir(_rootDir);
  _configWindow.setSweepFileDir(_rootDir);
}


/**********************************************************************
 * Destructor
 */

SoloMainWindow::~SoloMainWindow()
{
  delete _table;
}


/**********************************************************************
 * configFrames()
 */

void SoloMainWindow::changeCursor(const bool normal)
{
  Gdk::Cursor cursor(normal? Gdk::PENCIL : Gdk::WATCH);
  
  _eventBox.get_window()->set_cursor(cursor);
}


/**********************************************************************
 * configFrames()
 */

void SoloMainWindow::configFrames(const guint ncols, const guint nrows)
{
  sii_reset_config_cells();
  
  // Tag the cell with the frame no.

  guint frame_num = 0;
  
  for (guint jj = 0; jj < nrows; jj++)
  {
    for (guint ii = 0; ii < ncols; ii++)
    {
      guint kk = ii + jj * MAX_CONFIG_COLS;
      config_cells[kk]->frame_num = ++frame_num;
    }
  }

  sii_set_config();
  sii_new_frames();
}


/**********************************************************************
 * showConfigWindow()
 */

void SoloMainWindow::showConfigWindow()
{
  _configWindow.show_all();
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _centerFrames()
 */

void SoloMainWindow::_centerFrames()
{
  for(guint jj = 0; jj < SOLO_MAX_WINDOWS; jj++)
  {
    WW_PTR wwptr = solo_return_wwptr(jj);
    wwptr->center_of_view.setRotationAngle(0.0);
    wwptr->center_of_view.setRange(0.0);
  }

  sii_plot_data(0, REPLOT_ALL);

  for (int jj = 0; jj < sii_return_frame_count(); jj++)
  {
    ViewWindow *view_window = frame_configs[jj]->view_window;
    if (view_window != 0)
      view_window->update();
  }
}


/**********************************************************************
 * _centerOnClicks()
 */

void SoloMainWindow::_centerOnClicks(const guint num_clicks)
{
  sii_center_on_clicks(num_clicks);
}


/**********************************************************************
 * _changeConfigZoom()
 */

void SoloMainWindow::_changeConfigZoom(const gdouble factor)
{
  sii_table_widget_width = (guint)((factor * sii_table_widget_width) + 0.5);
  sii_table_widget_height = (guint)((factor * sii_table_widget_height) + 0.5);

  sii_check_def_widget_sizes();
  sii_new_frames();
}


/**********************************************************************
 * _changeDataZoom()
 */

void SoloMainWindow::_changeDataZoom(const double zoom_factor)
{
  for (guint jj = 0; jj < SOLO_MAX_WINDOWS; jj++)
  {
    WW_PTR wwptr = solo_return_wwptr(jj);
    if (zoom_factor > 0)
    {
      wwptr->view->magnification *= zoom_factor;
      wwptr->view->ts_magnification = wwptr->view->magnification;
    }
    else
    {
      wwptr->view->magnification = 1.0;
      wwptr->view->ts_magnification = 1.0;
    }
  }

  sii_plot_data(0, REPLOT_ALL);

  for (int jj = 0; jj < sii_return_frame_count(); jj++)
  {
    ViewWindow *view_window = frame_configs[jj]->view_window;
    if (view_window != 0)
      view_window->update();
  }
}


/**********************************************************************
 * _createMainMenu()
 */

void SoloMainWindow::_createMainMenu(Gtk::Box &container)
{
  // Specify the XML for the menus

  Glib::ustring ui_info =
    "<ui>"
    "  <menubar name='MenuBar'>"
    "    <menu action='FileMenu'>"
    "      <menuitem action='FileExit'/>"
    "      <separator/>"
    "      <menuitem action='FileSetSourceDir'/>"
    "      <menuitem action='FileConfigFiles'/>"
    "      <menuitem action='FileSetImageDir'/>"
    "      <separator/>"
    "      <menuitem action='FileHideDataWidgets'/>"
    "      <menuitem action='FileShowDataWidgets'/>"
    "      <menuitem action='FileExit'/>"
    "    </menu>"
    "    <menu action='ZoomMenu'>"
    "      <menuitem action='ZoomData'/>"
    "      <menuitem action='ZoomDataDefault'/>"
    "      <menuitem action='ZoomData+200%'/>"
    "      <menuitem action='ZoomData+100%'/>"
    "      <menuitem action='ZoomData+50%'/>"
    "      <menuitem action='ZoomData+25%'/>"
    "      <menuitem action='ZoomData+10%'/>"
    "      <separator/>"
    "      <menuitem action='ZoomData-10%'/>"
    "      <menuitem action='ZoomData-25%'/>"
    "      <menuitem action='ZoomData-50%'/>"
    "      <menuitem action='ZoomData-80%'/>"
    "      <separator/>"
    "      <menuitem action='ZoomConfig'/>"
    "      <menuitem action='ZoomConfig+50%'/>"
    "      <menuitem action='ZoomConfig+25%'/>"
    "      <menuitem action='ZoomConfig+17%'/>"
    "      <menuitem action='ZoomConfig+06%'/>"
    "      <separator/>"
    "      <menuitem action='ZoomConfig-05%'/>"
    "      <menuitem action='ZoomConfig-11%'/>"
    "    </menu>"
    "    <menu action='CenterMenu'>"
    "      <menuitem action='CenterOn'/>"
    "      <menuitem action='CenterLastClick'/>"
    "      <menuitem action='CenterLast2Clicks'/>"
    "      <menuitem action='CenterLast4Clicks'/>"
    "      <separator/>"
    "      <menuitem action='CenterRadar'/>"
    "      <separator/>"
    "      <menuitem action='CenterElectric'/>"
    "      <menuitem action='CenterCtrCrossHairs'/>"
    "    </menu>"
    "    <menu action='ConfigMenu'>"
    "      <menuitem action='ConfigDefaultConfig'/>"
    "      <menuitem action='Config1x1'/>"
    "      <menuitem action='Config1x2'/>"
    "      <menuitem action='Config1x3'/>"
    "      <menuitem action='Config1x4'/>"
    "      <separator/>"
    "      <menuitem action='Config2x1'/>"
    "      <menuitem action='Config2x2'/>"
    "      <menuitem action='Config2x3'/>"
    "      <menuitem action='Config2x4'/>"
    "      <separator/>"
    "      <menuitem action='Config3x1'/>"
    "      <menuitem action='Config3x2'/>"
    "      <menuitem action='Config3x3'/>"
    "      <menuitem action='Config3x4'/>"
    "      <separator/>"
    "      <menuitem action='Config4x1'/>"
    "      <menuitem action='Config4x2'/>"
    "      <menuitem action='Config4x3'/>"
    "      <separator/>"
    "      <menuitem action='ConfigSquareShaped'/>"
    "      <menuitem action='ConfigSlideShaped'/>"
    "      <menuitem action='ConfigVSlideShaped'/>"
    "      <separator/>"
    "      <menuitem action='Config1Big2VSmall'/>"
    "      <menuitem action='Config1Big2HSmall'/>"
    "      <menuitem action='Config1Big3Small'/>"
    "      <menuitem action='Config1Big5Small'/>"
    "      <menuitem action='Config1Big7Small'/>"
    "      <menuitem action='Config2Big4VSmall'/>"
    "      <menuitem action='Config2Big4HSmall'/>"
    "    </menu>"
    "    <menu action='HelpMenu'>"
    "      <menuitem action='HelpBasics'/>"
    "      <menuitem action='HelpFiles'/>"
    "      <menuitem action='HelpZooming'/>"
    "      <menuitem action='HelpCentering'/>"
    "      <menuitem action='HelpConfiguring'/>"
    "      <menuitem action='HelpShortcuts'/>"
    "      <menuitem action='HelpComparisons'/>"
    "      <menuitem action='HelpAbout'/>"
    "    </menu>"
    "  </menubar>"
    "</ui>";

  // Create the actions for the menus

  _actionGroup = Gtk::ActionGroup::create();
  
  // File menu actions

  _actionGroup->add(Gtk::Action::create("FileMenu", "File"));
  _actionGroup->add(Gtk::Action::create("FileExit", "Exit", "Exit the program"),
		    sigc::ptr_fun(gtk_main_quit));
  _actionGroup->add(Gtk::Action::create("FileSetSourceDir", "Set Source Dir"),
		    sigc::mem_fun(*this,
				  &SoloMainWindow::showConfigWindow));
  _actionGroup->add(Gtk::Action::create("FileConfigFiles", "Config Files"),
		    sigc::mem_fun(*this,
				  &SoloMainWindow::showConfigWindow));
  _actionGroup->add(Gtk::Action::create("FileSetImageDir", "Make Image Files..."),
		    sigc::mem_fun(_imageGenWindow,
				  &ImageGenWindow::show_all));
  _actionGroup->add(Gtk::Action::create("FileHideDataWidgets", "Hide Data Widgets"),
		    sigc::mem_fun(*this,
				  &SoloMainWindow::_hideAllClickDataWidgets));
  _actionGroup->add(Gtk::Action::create("FileShowDataWidgets", "Show Data Widgets"),
		    sigc::mem_fun(*this,
				  &SoloMainWindow::_showAllClickDataWidgets));
  
  // Zoom menu actions

  _actionGroup->add(Gtk::Action::create("ZoomMenu", "Zoom"));

  _actionGroup->add(Gtk::Action::create("ZoomData", "Data"));
  _actionGroup->add(Gtk::Action::create("ZoomDataDefault", "Default"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::_changeDataZoom),
			       0.0));
  _actionGroup->add(Gtk::Action::create("ZoomData+200%", " +200%"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::_changeDataZoom),
			       3.0));
  _actionGroup->add(Gtk::Action::create("ZoomData+100%", " +100%"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::_changeDataZoom),
			       2.0));
  _actionGroup->add(Gtk::Action::create("ZoomData+50%", "  +50%"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::_changeDataZoom),
			       1.5));
  _actionGroup->add(Gtk::Action::create("ZoomData+25%", "  +25%"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::_changeDataZoom),
			       1.25));
  _actionGroup->add(Gtk::Action::create("ZoomData+10%", "  +10%"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::_changeDataZoom),
			       1.1));
  _actionGroup->add(Gtk::Action::create("ZoomData-10%", "  -10%"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::_changeDataZoom),
			       0.9));
  _actionGroup->add(Gtk::Action::create("ZoomData-25%", "  -25%"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::_changeDataZoom),
			       0.75));
  _actionGroup->add(Gtk::Action::create("ZoomData-50%", "  -50%"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::_changeDataZoom),
			       0.5));
  _actionGroup->add(Gtk::Action::create("ZoomData-80%", "  -80%"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::_changeDataZoom),
			       0.2));
  
  _actionGroup->add(Gtk::Action::create("ZoomConfig", "Config"));
  _actionGroup->add(Gtk::Action::create("ZoomConfig+50%", "  +50%"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::_changeConfigZoom),
			       1.50));
  _actionGroup->add(Gtk::Action::create("ZoomConfig+25%", "  +25%"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::_changeConfigZoom),
			       1.25));
  _actionGroup->add(Gtk::Action::create("ZoomConfig+17%", "  +17%"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::_changeConfigZoom),
			       1.10));
  _actionGroup->add(Gtk::Action::create("ZoomConfig+06%", "  +06%"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::_changeConfigZoom),
			       1.06));
  _actionGroup->add(Gtk::Action::create("ZoomConfig-05%", "  -05%"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::_changeConfigZoom),
			       100.0/106.0));
  _actionGroup->add(Gtk::Action::create("ZoomConfig-11%", "  -11%"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::_changeConfigZoom),
			       100.0/112.0));

  // Center menu actions

  _actionGroup->add(Gtk::Action::create("CenterMenu", "Center"));
  _actionGroup->add(Gtk::Action::create("CenterOn", "Center on"));
  _actionGroup->add(Gtk::Action::create("CenterLastClick", "Last click"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::_centerOnClicks),
			       1));
  _actionGroup->add(Gtk::Action::create("CenterLast2Clicks", "Last 2 clicks"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::_centerOnClicks),
			       2));
  _actionGroup->add(Gtk::Action::create("CenterLast4Clicks", "Last 4 clicks"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::_centerOnClicks),
			       4));
  _actionGroup->add(Gtk::Action::create("CenterRadar", "Radar"),
		    sigc::mem_fun(*this,
				  &SoloMainWindow::_centerFrames));

  _electricCenteringToggle =
    Gtk::ToggleAction::create("CenterElectric", "Electric", "", true);
  _actionGroup->add(_electricCenteringToggle);

  _crosshairCenteringToggle =
    Gtk::ToggleAction::create("CenterCtrCrossHairs", "Ctr CrossHairs", "",
			      false);
  _actionGroup->add(_crosshairCenteringToggle);

  // Config menu actions

  _actionGroup->add(Gtk::Action::create("ConfigMenu", "Config"));
  _actionGroup->add(Gtk::Action::create("ConfigDefaultConfig", "Default Config"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::_customConfig),
			       FOUR_DEFAULT));
  _actionGroup->add(Gtk::Action::create("Config1x1", "1x1"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::configFrames),
			       1, 1));
  _actionGroup->add(Gtk::Action::create("Config1x2", "1x2"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::configFrames),
			       1, 2));
  _actionGroup->add(Gtk::Action::create("Config1x3", "1x3"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::configFrames),
			       1, 3));
  _actionGroup->add(Gtk::Action::create("Config1x4", "1x4"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::configFrames),
			       1, 4));
  _actionGroup->add(Gtk::Action::create("Config2x1", "2x1"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::configFrames),
			       2, 1));
  _actionGroup->add(Gtk::Action::create("Config2x2", "2x2"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::configFrames),
			       2, 2));
  _actionGroup->add(Gtk::Action::create("Config2x3", "2x3"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::configFrames),
			       2, 3));
  _actionGroup->add(Gtk::Action::create("Config2x4", "2x4"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::configFrames),
			       2, 4));
  _actionGroup->add(Gtk::Action::create("Config3x1", "3x1"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::configFrames),
			       3, 1));
  _actionGroup->add(Gtk::Action::create("Config3x2", "3x2"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::configFrames),
			       3, 2));
  _actionGroup->add(Gtk::Action::create("Config3x3", "3x3"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::configFrames),
			       3, 3));
  _actionGroup->add(Gtk::Action::create("Config3x4", "3x4"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::configFrames),
			       3, 4));
  _actionGroup->add(Gtk::Action::create("Config4x1", "4x1"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::configFrames),
			       4, 1));
  _actionGroup->add(Gtk::Action::create("Config4x2", "4x2"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::configFrames),
			       4, 2));
  _actionGroup->add(Gtk::Action::create("Config4x3", "4x3"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::configFrames),
			       4, 3));

  _actionGroup->add(Gtk::Action::create("ConfigSquareShaped", "Square Shaped"),
		    sigc::mem_fun(*this,
				  &SoloMainWindow::_shapeSquare));
  _actionGroup->add(Gtk::Action::create("ConfigSlideShaped", "Slide Shaped"),
		    sigc::mem_fun(*this,
				  &SoloMainWindow::_shapeSlide));
  _actionGroup->add(Gtk::Action::create("ConfigVSlideShaped", "VSlide Shaped"),
		    sigc::mem_fun(*this,
				  &SoloMainWindow::_shapeVSlide));

  _actionGroup->add(Gtk::Action::create("Config1Big2VSmall", "1 Big 2v Small"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::_customConfig),
			       ONE_BIG_TWOV_SMALL));
  _actionGroup->add(Gtk::Action::create("Config1Big2HSmall", "1 Big 2h Small"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::_customConfig),
			       ONE_BIG_TWO_SMALL));
  _actionGroup->add(Gtk::Action::create("Config1Big3Small", "1 Big 3 Small"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::_customConfig),
			       ONE_BIG_THREE_SMALL));
  _actionGroup->add(Gtk::Action::create("Config1Big5Small", "1 Big 5 Small"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::_customConfig),
			       ONE_BIG_FIVE_SMALL));
  _actionGroup->add(Gtk::Action::create("Config1Big7Small", "1 Big 7 Small"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::_customConfig),
			       ONE_BIG_SEVEN_SMALL));
  _actionGroup->add(Gtk::Action::create("Config2Big4VSmall", "2 Big 4v Small"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::_customConfig),
			       TWO_BIG_FOURV_SMALL));
  _actionGroup->add(Gtk::Action::create("Config2Big4HSmall", "2 Big 4h Small"),
		    sigc::bind(sigc::mem_fun(*this,
					     &SoloMainWindow::_customConfig),
			       TWO_BIG_FOUR_SMALL));

  // Help menu actions

  _actionGroup->add(Gtk::Action::create("HelpMenu", "Help"));
  _actionGroup->add(Gtk::Action::create("HelpBasics", "Basics"),
		    sigc::mem_fun(*this,
				  &SoloMainWindow::_displayHelpBasic));
  _actionGroup->add(Gtk::Action::create("HelpFiles", "Files"),
		    sigc::mem_fun(*this,
				  &SoloMainWindow::_displayHelpFile));
  _actionGroup->add(Gtk::Action::create("HelpZooming", "Zooming"),
		    sigc::mem_fun(*this,
				  &SoloMainWindow::_displayHelpZoom));
  _actionGroup->add(Gtk::Action::create("HelpCentering", "Centering"),
		    sigc::mem_fun(*this,
				  &SoloMainWindow::_displayHelpCentering));
  _actionGroup->add(Gtk::Action::create("HelpConfiguring", "Configuring"),
		    sigc::mem_fun(*this,
				  &SoloMainWindow::_displayHelpConfig));
  _actionGroup->add(Gtk::Action::create("HelpShortcuts", "Shortcuts"),
		    sigc::mem_fun(*this,
				  &SoloMainWindow::_displayHelpShortcuts));
  _actionGroup->add(Gtk::Action::create("HelpComparisons", "Comparisons"),
		    sigc::mem_fun(*this,
				  &SoloMainWindow::_displayHelpCompare));
  _actionGroup->add(Gtk::Action::create("HelpAbout", "About"),
		    sigc::mem_fun(*this,
				  &SoloMainWindow::_displayHelpAbout));

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
 * _customConfig()
 */

void SoloMainWindow::_customConfig(const custom_config_t config_type)
{
  g_message("Hello, Custom Config World! cols: %d  rows: %d\n",
	    config_type/10, config_type%10);

  sii_reset_config_cells();

  switch (config_type)
  {
  case FOUR_512x512 :
    config_cells[0]->frame_num = 1;
    config_cells[1]->frame_num = 2;
    config_cells[4]->frame_num = 3;
    config_cells[5]->frame_num = 4;
    sii_table_widget_width = 512;
    sii_table_widget_height = 512;
    break;

  case FOUR_DEFAULT :
    config_cells[0]->frame_num = 1;
    config_cells[1]->frame_num = 2;
    config_cells[4]->frame_num = 3;
    config_cells[5]->frame_num = 4;
    sii_table_widget_width = DEFAULT_WIDTH;
    sii_table_widget_height = DEFAULT_WIDTH;
    break;

  case ONE_BIG_TWOV_SMALL :
    config_cells[0]->frame_num = 1;
    config_cells[1]->frame_num = 1;
    config_cells[4]->frame_num = 1;
    config_cells[2]->frame_num = 2;
    config_cells[6]->frame_num = 3;
    break;
    
  case ONE_BIG_TWO_SMALL :
    config_cells[0]->frame_num = 1;
    config_cells[1]->frame_num = 1;
    config_cells[4]->frame_num = 1;
    config_cells[8]->frame_num = 2;
    config_cells[9]->frame_num = 3;
    break;
    
  case ONE_BIG_THREE_SMALL :
    config_cells[0]->frame_num = 1;
    config_cells[2]->frame_num = 1;
    config_cells[8]->frame_num = 1;
    config_cells[3]->frame_num = 2;
    config_cells[7]->frame_num = 3;
    config_cells[11]->frame_num = 4;
    break;
    
  case ONE_BIG_FIVE_SMALL :
    config_cells[0]->frame_num = 1;
    config_cells[1]->frame_num = 1;
    config_cells[4]->frame_num = 1;
    config_cells[2]->frame_num = 2;
    config_cells[6]->frame_num = 3;
    config_cells[8]->frame_num = 4;
    config_cells[9]->frame_num = 5;
    config_cells[10]->frame_num = 6;
    break;
    
  case ONE_BIG_SEVEN_SMALL :
    config_cells[0]->frame_num = 1;
    config_cells[2]->frame_num = 1;
    config_cells[8]->frame_num = 1;
    config_cells[3]->frame_num = 2;
    config_cells[7]->frame_num = 3;
    config_cells[11]->frame_num = 4;
    config_cells[12]->frame_num = 5;
    config_cells[13]->frame_num = 6;
    config_cells[14]->frame_num = 7;
    config_cells[15]->frame_num = 8;
    break;
    
  case TWO_BIG_FOURV_SMALL :
    config_cells[0]->frame_num = 1;
    config_cells[1]->frame_num = 1;
    config_cells[4]->frame_num = 1;
    
    config_cells[2]->frame_num = 2;
    config_cells[3]->frame_num = 2;
    config_cells[6]->frame_num = 3;
    config_cells[7]->frame_num = 3;
    
    config_cells[8]->frame_num = 4;
    config_cells[9]->frame_num = 4;
    config_cells[12]->frame_num = 4;
    
    config_cells[10]->frame_num = 5;
    config_cells[11]->frame_num = 5;
    config_cells[14]->frame_num = 6;
    config_cells[15]->frame_num = 6;
    break;
    
  case TWO_BIG_FOUR_SMALL :
    config_cells[0]->frame_num = 1;
    config_cells[1]->frame_num = 1;
    config_cells[4]->frame_num = 1;
    config_cells[2]->frame_num = 2;
    config_cells[3]->frame_num = 2;
    config_cells[6]->frame_num = 2;
    
    config_cells[8]->frame_num = 3;
    config_cells[9]->frame_num = 4;
    config_cells[10]->frame_num = 5;
    config_cells[11]->frame_num = 6;
    break;
    
  default:
    printf("Bogus custom config #: %d\n", config_type);
    return;
  }

  sii_set_config();
  sii_new_frames();
}


/**********************************************************************
 * _hideAllClickDataWidgets()
 */

void SoloMainWindow::_hideAllClickDataWidgets()
{
  for (guint frame_num = 0; frame_num < sii_frame_count; frame_num++)
  {
    DataWindow *data_window = frame_configs[frame_num]->data_window;
    
    if (data_window != 0)
      data_window->hide();
  }
}


/**********************************************************************
 * _keyReleaseEvent()
 */

bool SoloMainWindow::_keyReleaseEvent(GdkEventKey *event_key)
{
  switch (event_key->keyval)
  {
  case GDK_T:
  case GDK_t:
    frame_configs[cursor_frame-1]->param_window->toggleField();
    break;

  case GDK_D:
  case GDK_d:
    SeBoundaryList::getInstance()->zapLastPoint();
    break;

  case GDK_J:
  case GDK_j:
    sii_dump_debug_stuff();
    break;

  case GDK_F1:
  case GDK_F2:
  case GDK_F3:
  case GDK_F4:
  case GDK_F5:
  case GDK_F6:
  case GDK_F7:
  case GDK_F8:
  case GDK_F9:
  case GDK_F10:
  case GDK_F11:
  case GDK_F12:
  {
    guint F_frame_id = 0;
    if (event_key->keyval == GDK_L1)
    {
      F_frame_id = 10;
    }
    else if (event_key->keyval == GDK_L2)
    {
      F_frame_id = 11;
    }
    else
    {
      F_frame_id = event_key->keyval - GDK_F1;
    }
    sii_blow_up(F_frame_id);
    break;
  }
  
  case GDK_Escape:
    // Stop display or editing
    g_message("Caught Escape ");
    SoloState::getInstance()->setHaltFlag(true);
    break;
  
  case GDK_Delete:
    // Delete the last boundary point
    break;

  case GDK_Right:
    // If the cursor is currently residing in a frame, then plot the next
    // sweep for the set of frames (lockstep) of which this frame is a member

    if (event_key->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK))
    {
      // Fast forward mode (escape interrupts this mode)

      g_message("Caught Control/Shift Right ");
      
      if (cursor_frame != 0)
	sii_plot_data(cursor_frame - 1, FORWARD_FOREVER);
    }
    else
    {
      if (cursor_frame != 0)
	sii_plot_data(cursor_frame-1, FORWARD_ONCE);
    }
    break;
    
  case GDK_Left:
    // Same as Right only the other direction

    if (event_key->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK))
    {
      g_message( "Caught Control/Shift Left " );

      if (cursor_frame != 0)
	sii_plot_data(cursor_frame-1, BACKWARDS_FOREVER);
    }
    else
    {
      if (cursor_frame != 0)
	sii_plot_data(cursor_frame-1, BACKWARDS_ONCE);
    }
    break;

  case GDK_Up:
    // Next sweep at same fixed angle
    break;

  case GDK_Down:
    // Previous sweep at same fixed angle
    break;

    
  case GDK_Return:
    g_message( "Caught Return " );
    if (cursor_frame != 0)
      sii_plot_data (cursor_frame - 1, REPLOT_LOCK_STEP);
    else
      sii_plot_data (cursor_frame - 1, REPLOT_ALL);
    break;


  default:
    break;
  };

  // Save the event information in the debug buffer

  gchar mess[256];
  std::string key_string = "unk";

  if (gdk_keyval_name(event_key->keyval) != 0)
    key_string = gdk_keyval_name(event_key->keyval);

  sprintf(mess,
	  "Key press event: frame=%d keyval=%d state=%d key=%s modifier=%d",
	  cursor_frame, event_key->keyval, event_key->state, 
	  key_string.c_str(), event_key->is_modifier);
  sii_append_debug_stuff(mess);

  return true;
}


/**********************************************************************
 * _showAllClickDataWidgets()
 */

void SoloMainWindow::_showAllClickDataWidgets()
{
  for (guint frame_num = 0; frame_num < sii_frame_count; frame_num++)
  {
    DataWindow *data_window = frame_configs[frame_num]->data_window;
    
    if (data_window == 0)
    {
      data_window = new DataWindow(_defaultFont, frame_num);
      frame_configs[frame_num]->data_window = data_window;
    }
    
    data_window->show_all();
  }
}


/**********************************************************************
 * _displayHelpBasic()
 */

void SoloMainWindow::_displayHelpBasic()
{
  static const Glib::ustring help_text =
    Glib::ustring("Clicking the right mouse button brings up the frame menu\n") +
    "which enables changing sweepfile and lockstep settings;\n" +
    "changing parameter and color settings;\n" +
    "changing viewing, centering and landmark settings;\n" +
    "accessing the data editing widget;\n" +
    "accessing the data examine and point by point modification widget;\n" +
    "and popping up an individual small clicked data display widget.\n" +
    " \n" +
    "If you click the \"Data Widget\" button and the data widgit is\n" +
    "showing, it will be hidden.\n" +
    " \n" +
    "All clicks in menus and widgets should be done with the left mouse\n" +
    "button.\n" +
    " \n" +
    "Clicking the center button in a data frame causes the lockstep to\n" +
    "recenter on the clicked location. There is a crosshair in the center\n" +
    "of the frame to enable minor adjustments to the center. \n" +
    "Double clicking the left button also works. \n" +
    " \n" +
    "A lockstep is a set of frames that move forward and backward together\n" +
    "in response to the arrow keys.\n" +
    " \n" +
    "Clicking the left button in the data causes location information to be\n" +
    "sent to the small clicked data display, the examine widget, and the\n" +
    "edit widget.\n" +
    " \n" +
    "The keyboard right arrow triggers the display of the next sweepfile\n" +
    "for the lockstep. The left arrow moves backwards. You can trigger\n" +
    "continuous plotting by holding down the shift key while you press an\n" +
    "arrow key, but BEWARE, the loop can only be stopped by typing\n" +
    "Control-C with the cursor in the window where you started solo3.\n" +
    " \n" +
    "Hitting the return/enter will cause the lockstep associated with the\n" +
    "frame in which the cursor currently resides to replot. If the cursor\n" +
    "is in the menubar, all locksteps will replot.\n" +
    " \n" +
    "If you have plotted more than one field in the same frame, the \"t\"\n" +
    "key will toggle the plot between the last two fields plotted.\n" +
    " \n" +
    "For most widgets clicking either \"OK\" or one of the \"Replot\"\n" +
    "buttons will cause the changes to be accepted. Clicking the \"Close\"\n" +
    "or \"Cancel\" buttons will leave plot parameters unchanged.\n";

  sii_message(help_text);
}


/**********************************************************************
 * _displayHelpFile()
 */

void SoloMainWindow::_displayHelpFile()
{
  static const Glib::ustring help_text =
    Glib::ustring("If you have not set any of the environment variables \"DORADE_DIR\",\n") +
    "\"SOLO_WINDOW_INFO\" or \"swi\" (swi and SOLO_WINDOW_INFO are\n" +
    "synonomous), the \"Solo3 Initialization\" widget will appear allowing\n" +
    "you to set the directory containing the source sweepfiles or the\n" +
    "configuration directory. This same widget will appear when you click\n" +
    "\"Set Source Dir\", \"Config Files\", or \"Set Image Dir\".\n" +
    " \n" +
    "It is not necessary to set \"DORADE_DIR\" if you start solo3 from a\n" +
    "directory containing sweepfiles. \n" +
    " \n" +
    "Subsequent changes to the source directory can and should be done\n" +
    "in the \"Sweepfiles Widget\"\n" +
    " \n" +
    "To save a configuration, you can modify the \"Comment\" and then click\n" +
    "\"Save Config\".\n" +
    " \n" +
    "To initialize the file selection widgets with a directory name, set the \n" +
    "environment variable \"INIT_FILESEL\" to a directory name\n" +
    " \n" +
    "To make a PNG image of the solo3 display click the button labeled\n" +
    "\"Make PNG Image\" in the Initialization widget. Be sure the data\n" +
    "display window is completely visible and that no other widgets are\n" +
    "encroaching on the display You can also set the destination directory\n" +
    "for PNG images. Setting the environment variable \"WINDOW_DUMP_DIR\"\n" +
    "also sets the directory for PNG images.\n" +
    " \n" +
    "If you have set the \"BATCH_MODE\" environment variable to blast out\n" +
    "images of all the sweepfiles in the source directory, you will likely\n" +
    "want to set the SOLO_WINDOW_INFO environment variable and the\n" +
    "WINDOW_DUMP_DIR variable.\n";

  sii_message(help_text);
}


/**********************************************************************
 * _displayHelpZoom()
 */

void SoloMainWindow::_displayHelpZoom()
{
  static const Glib::ustring help_text =
    Glib::ustring("With this menu you can zoom or unzoom the data or the size of the data\n") +
    "widgets (i.e. the whole configuration).\n" +
    " \n" +
    "The data zoom applies to all frames. The default zoom/magnification\n" +
    "value is 1.0 which implies 300 meters per pixel (roughly 10 km. per\n" +
    "cm. of screen).\n" +
    " \n" +
    "There is a minimum height and width for a frame. The frame may change\n" +
    "proportions as these minimums are reached. The minimum width is based\n" +
    "on the worst case scenario for the title line for the smallest font\n" +
    "used.\n" +
    " \n" +
    "At this point the only way to shrink the configuration is to use the\n" +
    "negative zooms under \"Config\".\n";

  sii_message(help_text);
}


/**********************************************************************
 * _displayHelpCentering()
 */

void SoloMainWindow::_displayHelpCentering()
{
  static const Glib::ustring help_text =
    Glib::ustring("For centering on two or four clicks, the new center is based on the\n") +
    "center of the extremes in the horizontal and vertical. The zoom is\n" +
    "adjusted such the all the clicks fit in the current frame shape.\n" +
    " \n" +
    "The \"Electric\" toggle button enables or disables replotting when the\n" +
    "center mouse button is clicked in a frame.\n";

  sii_message(help_text);
}


/**********************************************************************
 * _displayHelpConfig()
 */

void SoloMainWindow::_displayHelpConfig()
{
  static const Glib::ustring help_text =
    Glib::ustring("The configuration of the frames of data is based on a 4x4 matrix.  All\n") +
    "data frames must be rectangular but they can be of different sizes as\n" +
    "long as the fit the matrix and the final overal configuration is\n" +
    "rectangluar. There can never be more than 12 frames.\n" +
    " \n" +
    "The top part of the menu consists of an exhaustive list of the\n" +
    "possible combinations uniformly shaped frames.\n" +
    " \n" +
    "Next are options to force the frames to specified proportions.\n" +
    " \n" +
    "Finally a set of unusual configurations just for grins.\n";

  sii_message(help_text);
}


/**********************************************************************
 * _displayHelpShortcuts()
 */

void SoloMainWindow::_displayHelpShortcuts()
{
  static const Glib::ustring help_text =
    Glib::ustring("Text Editing Shortcuts\n") +
    " \n" +
    "Alt may not work on some systems. Try the diamond key instead.\n" +
    " \n" +
    "Ctrl+A -- Beginning of Line\n" +
    "Ctrl+E -- End of Line\n" +
    "Ctrl+N -- Next Line\n" +
    "Ctrl+P -- Previous Line\n" +
    "Ctrl+B -- Backward one character\n" +
    "Ctrl+F -- Forward one character\n" +
    " Alt+B -- Backward one word\n" +
    " Alt+f -- Forward one word\n" +
    " \n" +
    "Ctrl+H -- Delete backward character (backspace)\n" +
    "Ctrl+D -- Delete forward character (delete)\n" +
    "Ctrl+W -- Delete backward word\n" +
    " Alt+D -- Delete forward word\n" +
    "Ctrl+K -- Delete to end of line\n" +
    "Ctrl+U -- Delete line\n" +
    " \n" +
    "Ctrl+X -- Cut to clipboard\n" +
    "Ctrl+C -- Copy to clipboard\n";

  sii_message(help_text);
}


/**********************************************************************
 * _displayHelpCompare()
 */

void SoloMainWindow::_displayHelpCompare()
{
  static const Glib::ustring help_text =
    Glib::ustring("The procedure for aligning two or more radars to do comparisons\n") +
    "involves resetting the \"SwpfiLinks\" and most likely the \"Lockstep\"\n" +
    "links that reside in the \"Sweepfiles\" widget.\n" +
    " \n" +
    "This should be done in several steps. First starting with the\n" +
    "Sweepfiles widget for frame 1, click the SwpfiLinks->Set_Links\n" +
    "button. Clear all links and then click the frames that share the same\n" +
    "set of sweepfiles with frame 1 including frame 1. You will most likely\n" +
    "want to control the display of each radar seperately rather than\n" +
    "having them advance together; therefore, repeat this procedure for the\n" +
    "\"Lockstep\". Be sure to click \"OK\" in each when you are finished\n" +
    "setting the links.\n" +
    " \n" +
    "Now click \"OK\" in the Sweepfiles widget and replot all frames by\n" +
    "moving the cursor up to main menubar and pressing \"Return\". It is\n" +
    "important to do this for each new radar because the sorting out and\n" +
    "finalizing of the links for all frames happens during the plot phase.\n" +
    " \n" +
    "Repeat the above steps in the first frame in which the next radar will\n" +
    "be plotted but also type the directory containing the sweepfiles of\n" +
    "the next radar before you replot all frames.\n" +
    " \n" +
    "You will then select the fields plotted for each radar in each frame\n" +
    "with the \"Parameters\" widget.\n" +
    " \n" +
    "Comparisons usually involves referencing the centers of frames\n" +
    "containing the other radars to the center of Frame 1 or fixing the\n" +
    "centers of all frames at a specified lat/lon. The widget that enables\n" +
    "this is the \"Center\" button in the \"View\" widget.\n";

  sii_message(help_text);
}


/**********************************************************************
 * _displayHelpAbout()
 */

void SoloMainWindow::_displayHelpAbout()
{
  static const Glib::ustring help_text =
    Glib::ustring(
      "Solo3 was specifically targeted to work using the GTK+ graphics "
      "libraries and the GNU compilers that come with RedHat Linux "
      "operating system and in EOL/RSF systems environment in the "
      "S-Pol radar and at the EOL Boulder Foothills Lab."
      "\n\n"
      "Resonable attempts will be made to help users with different "
      "environments and OSs get Solo3 installed, compiled and running, "
      "but Solo3 comes with no guarentees of working in any other "
      "environments."
      "\n\n"
      "date: ");

  Glib::ustring display_string = help_text + sii_date_stamp;
  
  sii_message(display_string);
}


/**********************************************************************
 * _eventBoxEntered()
 */

bool SoloMainWindow::_eventBoxEntered(GdkEventCrossing *event)
{
  changeCursor(solo_busy() ? false : true);
  
  return true;
}


/**********************************************************************
 * _eventBoxConfigured()
 */

bool SoloMainWindow::_eventBoxConfigured(GdkEventConfigure *event)
{
  g_message("main event box configured %d x %d ",
	    _eventBox.get_allocation().get_width(),
	    _eventBox.get_allocation().get_height());
  
  return true;
}


/**********************************************************************
 * _shapeSquare()
 */

void SoloMainWindow::_shapeSquare()
{
  sii_table_widget_height = sii_table_widget_width;

  sii_check_def_widget_sizes();
  sii_new_frames();
}


/**********************************************************************
 * _shapeSlide()
 */

void SoloMainWindow::_shapeSlide()
{
  sii_table_widget_height = (guint)(0.75 * sii_table_widget_width);

  sii_check_def_widget_sizes();
  sii_new_frames();
}


/**********************************************************************
 * _shapeVSlide()
 */

void SoloMainWindow::_shapeVSlide()
{
  sii_table_widget_height =
    (guint)((gdouble)4.0/3.0 * sii_table_widget_width);

  sii_check_def_widget_sizes();
  sii_new_frames();
}
