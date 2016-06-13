#include <iostream>

#include <se_utils.hh>
#include <se_wgt_stf.hh>
#include <sii_externals.h>
#include <sii_utils.hh>
#include <solo2.hh>
#include <sp_save_frms.hh>

#include "MainConfigWindow.hh"


/**********************************************************************
 * Constructor
 */

MainConfigWindow::MainConfigWindow(const Pango::FontDescription &default_font) :
  Gtk::Window(),
  _gsConfigFileNames(""),
  _defaultFont(default_font),
  _mainConfigText(1)
{
  // Create some fonts to use for our widgets

  Pango::FontDescription bold_font = _defaultFont;
  bold_font.set_weight(Pango::WEIGHT_SEMIBOLD);
  
  Pango::FontDescription smaller_font = _defaultFont;
  smaller_font.set_size(_defaultFont.get_size() - (2 * 1024));
  
  Pango::FontDescription smaller_italic_font = smaller_font;
  smaller_italic_font.set_style(Pango::STYLE_ITALIC);
  
  // Set the window title

  set_title("Solo3 Initialization");

  // Create the containers for widget positioning

  _vbox.set_homogeneous(false);
  _vbox.set_spacing(0);
  add(_vbox);

  _hbox.set_homogeneous(false);
  _hbox.set_spacing(2);
  _hbox.set_border_width(4);
  _vbox.pack_start(_hbox, FALSE, FALSE, 0);

  // Initialize the save config button

  _saveConfigButton.set_label(" Save Config ");
  _saveConfigButton.modify_font(_defaultFont);
  _hbox.pack_start(_saveConfigButton, FALSE, FALSE, 0);
  _saveConfigButton.signal_clicked().connect(sigc::mem_fun(*this,
							   &MainConfigWindow::_setWindowInfo));

  // Initialize the list configs button

  _listConfigsButton.set_label(" List Configs ");
  _listConfigsButton.modify_font(_defaultFont);
  _hbox.pack_start(_listConfigsButton, FALSE, FALSE, 0);
  _listConfigsButton.signal_clicked().connect(sigc::mem_fun(*this,
							    &MainConfigWindow::_loadConfigFileList));

  // Initialize the cancel button

  _cancelButton.set_label(" Cancel ");
  _cancelButton.modify_font(_defaultFont);
  _hbox.pack_end(_cancelButton, FALSE, FALSE, 0);
  _cancelButton.signal_clicked().connect(sigc::mem_fun(*this,
						       &MainConfigWindow::hide));

  // Initialize the help label

  _helpLabel.set_text(Glib::ustring("For default startup, type the sweepfile directory and hit <enter>\n") +
		      "or enter the directory with your confguration files and hit <enter>\n" +
		      "and then click on the desired file name.\n");
  _helpLabel.modify_font(smaller_font);
  _helpLabel.set_justify(Gtk::JUSTIFY_LEFT);
  _vbox.pack_start(_helpLabel, FALSE, FALSE, 0);

  // Initialize the table of objects

  _widgetTable.resize(8, 5);
  _widgetTable.set_homogeneous(false);
  _widgetTable.set_border_width(4);
  _vbox.pack_start(_widgetTable, FALSE, FALSE, 0);
  gint row = -1;
  guint xpadding = 0;
  guint ypadding = 3;

  // Add the sweepfile directory entry widgets

  row++;
  _swpfiDirLabel.set_text(" Swpfi Dir ");
  _swpfiDirLabel.modify_font(bold_font);
  _widgetTable.attach(_swpfiDirLabel, 0, 1, row, row + 1,
		      Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		      Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		      xpadding, ypadding);

  _swpfiDirEntry.modify_font(_defaultFont);
  _widgetTable.attach(_swpfiDirEntry, 1, 6, row, row + 1,
		      Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		      Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		      xpadding, ypadding);
  _swpfiDirEntry.set_text("./");
  _swpfiDirEntry.signal_activate().connect(sigc::mem_fun(*this,
							 &MainConfigWindow::_setSweepFileDir));

  // Initialize the sweepfile select button

  row++;
  _blankLabel.set_text(" ");
  _blankLabel.modify_font(_defaultFont);
  _widgetTable.attach(_blankLabel, 0, 1, row, row + 1,
		      Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		      Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		      xpadding, ypadding);

  _swpfiFileSelectButton.set_label(" Swpfi File Select ");
  _swpfiFileSelectButton.modify_font(_defaultFont);
  _widgetTable.attach(_swpfiFileSelectButton, 3, 5, row, row + 1,
		      Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		      Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		      xpadding, ypadding);
  _swpfiFileSelectButton.signal_clicked().connect(sigc::mem_fun(*this,
								&MainConfigWindow::_selectSweepFile));

  // Initialize the config directory entry widgets

  row++;
  _configDirLabel.set_text(" Config Dir ");
  _configDirLabel.modify_font(bold_font);
  _widgetTable.attach(_configDirLabel, 0, 1, row, row + 1,
		      Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		      Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		      xpadding, ypadding);

  _configDirEntry.modify_font(_defaultFont);
  _widgetTable.attach(_configDirEntry, 1, 6, row, row + 1,
		      Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		      Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		      xpadding, ypadding);
  _configDirEntry.set_text("./");
  _configDirEntry.signal_activate().connect(sigc::mem_fun(*this,
							  &MainConfigWindow::_loadConfigFileList));

  // Initialize the comment entry widgets

  row++;
  _commentLabel.set_text(" Comment ");
  _commentLabel.modify_font(bold_font);
  _widgetTable.attach(_commentLabel, 0, 1, row, row + 1,
		      Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		      Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		      xpadding, ypadding);
  _widgetTable.attach(_commentEntry, 1, 6, row, row + 1,
		      Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		      Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		      xpadding, ypadding);
  _commentEntry.set_text("_no_comment");
  _commentEntry.modify_font(_defaultFont);
  _commentEntry.signal_activate().connect(sigc::mem_fun(*this,
							&MainConfigWindow::_setWindowInfo));
  
  
  // Initialize the ignore sweepfile info button

  row++;
  _ignoreButton.set_label(" Ignore Swpfi Info ");
  _ignoreButton.modify_font(_defaultFont);
  _widgetTable.attach(_ignoreButton, 1, 3, row, row + 1,
		      Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		      Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		      xpadding, ypadding);
  _ignoreButton.set_active(false);

  // Initialize the config file select button

  _configFileSelectButton.set_label(" Config File Select ");
  _configFileSelectButton.modify_font(_defaultFont);
  _widgetTable.attach(_configFileSelectButton, 3, 5, row, row + 1,
		      Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		      Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		      xpadding, ypadding);
  _configFileSelectButton.signal_clicked().connect(sigc::mem_fun(*this,
								 &MainConfigWindow::_selectConfigFile));
  
  // Initialize the selection help label

  _selectLabel.set_text("Double-click to select configuration file");
  _selectLabel.modify_font(smaller_italic_font);
  _selectLabel.set_justify(Gtk::JUSTIFY_LEFT);
  _vbox.pack_start(_selectLabel, FALSE, FALSE, 0);

  // Initialize the config file table

  _mainConfigText.set_headers_visible(false);
  _mainConfigText.modify_font(_defaultFont);

  // Set the padding for the list.  Note that the gtkmm-2.4 installed with
  // the Centos OS has a different API for the CellRenderer class so we can't
  // do this on that OS.
#ifndef OS_IS_CENTOS
  Gtk::CellRenderer *renderer = _mainConfigText.get_column_cell_renderer(0);
  renderer->set_padding(0, 0);
#endif

  _mainConfigText.signal_row_activated().connect(sigc::mem_fun(*this,
							       &MainConfigWindow::_configFileSelectedFromList));
  
  _mainConfigScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC,
				       Gtk::POLICY_AUTOMATIC);
  
  _mainConfigScrolledWindow.add(_mainConfigText);
  _mainConfigScrolledWindow.get_vadjustment()->set_page_size(150);
  _vbox.pack_start(_mainConfigScrolledWindow, TRUE, TRUE, 0);
}


/**********************************************************************
 * Destructor
 */

MainConfigWindow::~MainConfigWindow()
{
}


/**********************************************************************
 * setConfigDir()
 */

void MainConfigWindow::setConfigDir(const Glib::ustring &config_dir)
{
  // Set the directory
  
  _configDirEntry.set_text(config_dir);
  
  // Get the configuration files in that directory and put them in the
  // file list
  
  _setMainConfigText(config_dir);
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _configFileSelected()
 */

void MainConfigWindow::_configFileSelected(Gtk::FileSelection *file_selector)
{
  // Get the selected file path from the window, then delete the file
  // selection object since we don't need it anymore

  Glib::ustring file_path = file_selector->get_filename();
  delete file_selector;

  // Extract the directory path from the full file path
  
  Glib::ustring dir_path;
  Glib::ustring file_name;
  
  Glib::ustring::size_type slash_pos = file_path.rfind('/');
   
  if (slash_pos == Glib::ustring::npos)
  {
    dir_path = "";
    file_name = file_path;
  }
  else
  {
    dir_path = file_path.substr(0, slash_pos);
    file_name = file_path.substr(slash_pos+1);
  }
  
  // Get the list of configuration files in the new directory path and put
  // them in the text widget

  _setMainConfigText(dir_path);
  
  if (file_name != "")
  {
    solo_absorb_window_info(dir_path, file_name,
			    sii_frame_count ? _isIgnoreSweepFileInfo() : 0);
  }
  
  // Update the selected config directory in the entry widget

  _configDirEntry.set_text(dir_path);
}


/**********************************************************************
 * _configFileSelectedFromList()
 */

void MainConfigWindow::_configFileSelectedFromList(const Gtk::TreeModel::Path &path,
						   Gtk::TreeViewColumn *column)
{
  int row_number = atoi(path.to_string().c_str());
  
  Glib::ustring file_name = _mainConfigText.get_text(row_number);
  Glib::ustring dir_path = _configDirEntry.get_text();
  
  solo_absorb_window_info(dir_path, file_name,
			  sii_frame_count ? _isIgnoreSweepFileInfo() : 0);
}


/**********************************************************************
 * _deleteFileSelectionPointer()
 */

void MainConfigWindow::_deleteFileSelectionPointer(Gtk::FileSelection *file_selector)
{
  delete file_selector;
}


/**********************************************************************
 * _listConfigFiles()
 */

std::vector< std::string > MainConfigWindow::_listConfigFiles(const std::string &dir)
{
  struct solo_edit_stuff *seds = return_sed_stuff();

  se_nab_files(dir, seds->list_winfo_files, "wds");
  sort(seds->list_winfo_files.begin(), seds->list_winfo_files.end());
  
  std::vector< std::string > file_list;
  
  for (size_t i = 0; i < seds->list_winfo_files.size(); ++i)
  {
    std::string file_name = seds->list_winfo_files[i];
    
    // Remove trailing whitespace

    std::size_t nonblank_pos =
      file_name.find_last_not_of(" \t\n\r");
    if (nonblank_pos != std::string::npos)
      file_name = file_name.substr(0, nonblank_pos+1);
    
    file_list.push_back(file_name);
  }
  
  return file_list;
}


/**********************************************************************
 * _loadConfigFileList()
 */

void MainConfigWindow::_loadConfigFileList()
{
  Glib::ustring config_dir = _configDirEntry.get_text();

  _setMainConfigText(config_dir);
}


/**********************************************************************
 * _selectConfigFile()
 */

void MainConfigWindow::_selectConfigFile()
{
  const gchar *root_dir = getenv("INIT_FILESEL");

  if (root_dir == 0)
    root_dir = "/";

  // Create the file selector widget

  Gtk::FileSelection *file_selector =
    new Gtk::FileSelection("config_dir file selection dialog");
  file_selector->hide_fileop_buttons();
  file_selector->set_filename(root_dir);

  // Connect callbacks to the buttons

  file_selector->get_ok_button()->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this,
										    &MainConfigWindow::_configFileSelected),
								      file_selector));
  file_selector->get_cancel_button()->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this,
											&MainConfigWindow::_deleteFileSelectionPointer),
									  file_selector));

  // Show the selector

  file_selector->show();
}
  

/**********************************************************************
 * _selectSweepFile()
 */

void MainConfigWindow::_selectSweepFile()
{
  const gchar *root_dir = getenv("INIT_FILESEL");

  if (root_dir == 0)
    root_dir = "/";

  // Create the file selector widget

  Gtk::FileSelection *file_selector =
    new Gtk::FileSelection("swpfi_dir file selection dialog");
  file_selector->hide_fileop_buttons();
  file_selector->set_filename(root_dir);

  // Connect callbacks to the buttons

  file_selector->get_ok_button()->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this,
										    &MainConfigWindow::_sweepFileSelected),
								      file_selector));
  file_selector->get_cancel_button()->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this,
											&MainConfigWindow::_deleteFileSelectionPointer),
									  file_selector));

  // Show the selector

  file_selector->show();
}
  

/**********************************************************************
 * _setSweepFileDir()
 */

void MainConfigWindow::_setSweepFileDir()
{
  if (sii_frame_count > 0)
  {
    sii_message("Use Sweepfiles Widget to change directories");
    return;
  }

  Glib::ustring sweepfile_dir = _swpfiDirEntry.get_text();
    
  if (sii_default_startup(sweepfile_dir.c_str()) < 1)
  {
    sii_message("\nNo sweepfiles in " + sweepfile_dir + "\n");
    return;
  }

  // NOTE:  We currently don't have access to the main window here.  Since it
  // seems weird to me that we would reconfigure the display when setting the
  // sweep file dir, so I just commented this out.  If we really do want to
  // reconfigure the display we can add this back in.

//  main_window->configFrames(2, 2);
}
  

/**********************************************************************
 * _setWindowInfo()
 */

void MainConfigWindow::_setWindowInfo()
{
  Glib::ustring config_dir = _configDirEntry.get_text();
  Glib::ustring comment = _commentEntry.get_text();

  solo_save_window_info(config_dir.c_str(), comment.c_str());

  // Update the configuration file list

  _setMainConfigText(config_dir);
}

  
/**********************************************************************
 * _sweepFileSelected()
 */

void MainConfigWindow::_sweepFileSelected(Gtk::FileSelection *file_selector)
{
  // Get the selected file path from the window, then delete the file
  // selection object since we don't need it anymore

  Glib::ustring file_path = file_selector->get_filename();
  delete file_selector;

  // Set the file path in the entry widget

  _swpfiDirEntry.set_text(file_path);

  // Reconfigure the display

  sii_default_startup(file_path.c_str());

  // NOTE:  We currently don't have access to the main window here.  Since it
  // seems weird to me that we would reconfigure the display when selecting a
  // new sweep file, so I just commented this out.  If we really do want to
  // reconfigure the display we can add this back in.

//  main_window->configFrames(2, 2);
}
