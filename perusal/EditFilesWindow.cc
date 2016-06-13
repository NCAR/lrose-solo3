#include <dirent.h>
#include <iostream>
#include <sys/types.h>

#include <DateTime.hh>
#include <EditWindow.hh>
#include <se_bnd.hh>
#include <se_proc_data.hh>
#include <se_utils.hh>
#include <se_wgt_stf.hh>
#include <SeBoundaryList.hh>
#include <solo_list_widget_ids.h>
#include <solo2.hh>
#include <sp_basics.hh>
#include <sp_save_frms.hh>

#include "EditFilesWindow.hh"

/**********************************************************************
 * Constructor
 */

EditFilesWindow::EditFilesWindow(EditWindow *parent,
				 const Pango::FontDescription &default_font) :
  Gtk::Window(),
  _parent(parent),
  _defaultFont(default_font),
  _table(6, 4, false),
  _cmdFilesList(1),
  _table3(6, 4, false),
  _bdryFilesList(1),
  _table5(6, 3, false),
  _latlonFilesList(1)
{
  // Create the fonts needed for the widgets

  Pango::FontDescription bold_font = _defaultFont;
  bold_font.set_weight(Pango::WEIGHT_SEMIBOLD);
  
  Pango::FontDescription larger_bold_font = bold_font;
  larger_bold_font.set_size(_defaultFont.get_size() + (1 * 1024));
  
  Pango::FontDescription smaller_italic_font = _defaultFont;
  smaller_italic_font.set_size(_defaultFont.get_size() - (2 * 1024));
  smaller_italic_font.set_style(Pango::STYLE_ITALIC);
  
  // Construct the window title

  char title_string[1024];
  sprintf(title_string, "Frame %d  Edit Command and Boundary Files",
	  _parent->getFrameIndex() + 1);
  
  set_title(title_string);
  set_border_width(2);
  
  // Set up the widgets

  _vbox0.set_homogeneous(false);
  _vbox0.set_spacing(0);
  add(_vbox0);
  
  _hbox.set_homogeneous(false);
  _hbox.set_spacing(0);
  _hbox.set_border_width(4);
  _vbox0.pack_start(_hbox, false, false);
  
  _saveCmdsButton.set_label("Save Commands");
  _saveCmdsButton.modify_font(_defaultFont);
  _hbox.pack_start(_saveCmdsButton, true, true, 0);
  _saveCmdsButton.signal_clicked().connect(sigc::mem_fun(*this,
							 &EditFilesWindow::_saveCommands));
  
  _saveBdryButton.set_label("Save Boundary");
  _saveBdryButton.modify_font(_defaultFont);
  _hbox.pack_start(_saveBdryButton, true, true, 0);
  _saveBdryButton.signal_clicked().connect(sigc::mem_fun(*this,
							 &EditFilesWindow::_saveBoundary));
  
  _cancelButton.set_label("Cancel");
  _cancelButton.modify_font(_defaultFont);
  _hbox.pack_start(_cancelButton, true, true, 0);
  _cancelButton.signal_clicked().connect(sigc::mem_fun(*this,
						       &EditFilesWindow::hide));
  
  _cmdFilesLabel.set_text("Editor Command Files");
  _cmdFilesLabel.modify_font(larger_bold_font);
  _vbox0.pack_start(_cmdFilesLabel, false, false);
  
  _table.set_border_width(0);
  _vbox0.pack_start(_table, true, true);
  
  int row = -1;
  
  ++row;
  _cmdDirectoryLabel.set_text(" Directory ");
  _cmdDirectoryLabel.modify_font(bold_font);
  _table.attach(_cmdDirectoryLabel, 0, 1, row, row+1,
		(Gtk::AttachOptions)0,
		(Gtk::AttachOptions)0,
		0, 2);

  _cmdDirectoryEntry.set_text("./");
  _cmdDirectoryEntry.modify_font(_defaultFont);
  _table.attach(_cmdDirectoryEntry, 1, 6, row, row+1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		(Gtk::AttachOptions)0,
		0, 2);
  _cmdDirectoryEntry.signal_activate().connect(sigc::mem_fun(*this,
							     &EditFilesWindow::_cmdDirectoryActivated));
  
  ++row;
  _cmdCommentLabel.set_text("Comment");
  _cmdCommentLabel.modify_font(bold_font);
  _table.attach(_cmdCommentLabel, 0, 1, row, row+1,
		(Gtk::AttachOptions)0,
		(Gtk::AttachOptions)0,
		0, 2);
  
  _cmdCommentEntry.set_text("_no_comment");
  _cmdCommentEntry.modify_font(_defaultFont);
  _table.attach(_cmdCommentEntry, 1, 6, row, row+1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		(Gtk::AttachOptions)0,
		0, 2);
  _cmdCommentEntry.signal_activate().connect(sigc::mem_fun(*this,
							   &EditFilesWindow::_saveCommands));

  std::vector< std::string > cmd_files =
    _getCmdFileList(_cmdDirectoryEntry.get_text());
  
  ++row;
  _cmdFilesHelpLabel.set_text("Double-click to select file");
  _cmdFilesHelpLabel.modify_font(smaller_italic_font);
  _table.attach(_cmdFilesHelpLabel, 0, 6, row, row+1,
		(Gtk::AttachOptions)0,
		(Gtk::AttachOptions)0,
		0, 2);
  
  ++row;
  _cmdFilesList.set_headers_visible(false);
  _cmdFilesList.modify_font(_defaultFont);

  // Set the padding for the list.  Note that the gtkmm-2.4 installed with
  // the Centos OS has a different API for the CellRenderer class so we can't
  // do this on that OS.
#ifndef OS_IS_CENTOS
  Gtk::CellRenderer *cmd_files_renderer =
    _cmdFilesList.get_column_cell_renderer(0);
  cmd_files_renderer->set_padding(0, 0);
#endif

  for (size_t i = 0; i < cmd_files.size(); ++i)
    _cmdFilesList.append_text(cmd_files[i]);
  _cmdFilesList.signal_row_activated().connect(sigc::mem_fun(*this,
							     &EditFilesWindow::_selectCommandFile));
  
  _cmdFilesScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC,
				     Gtk::POLICY_AUTOMATIC);
  _cmdFilesScrolledWindow.add(_cmdFilesList);
  
  _table.attach(_cmdFilesScrolledWindow, 0, 6, row, row+1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 2);
  ++row;
  
  _boundaryFilesLabel.set_text("Editor Boundary Files");
  _boundaryFilesLabel.modify_font(larger_bold_font);
  _vbox0.pack_start(_boundaryFilesLabel, false, false);
  
  _table3.set_border_width(0);
  _vbox0.pack_start(_table3, true, true);
  
  row = -1;
  
  ++row;
  _bdryDirectoryLabel.set_text(" Directory ");
  _bdryDirectoryLabel.modify_font(bold_font);
  _table3.attach(_bdryDirectoryLabel, 0, 1, row, row+1,
		 (Gtk::AttachOptions)0,
		 (Gtk::AttachOptions)0,
		 0, 2);
  
  _bdryDirectoryEntry.set_text("./");
  _bdryDirectoryEntry.modify_font(_defaultFont);
  _table3.attach(_bdryDirectoryEntry, 1, 6, row, row+1,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 (Gtk::AttachOptions)0,
		 0, 2);
  _bdryDirectoryEntry.signal_activate().connect(sigc::mem_fun(*this,
							      &EditFilesWindow::_bdryDirectoryActivated));
  
  ++row;
  _bdryCommentLabel.set_text("Comment");
  _bdryCommentLabel.modify_font(bold_font);
  _table3.attach(_bdryCommentLabel, 0, 1, row, row+1,
		 (Gtk::AttachOptions)0,
		 (Gtk::AttachOptions)0,
		 0, 2);
  
  _bdryCommentEntry.set_text("_no_comment");
  _bdryCommentEntry.modify_font(_defaultFont);
  _table3.attach(_bdryCommentEntry, 1, 6, row, row+1,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 (Gtk::AttachOptions)0,
		 0, 2);
  _bdryCommentEntry.signal_activate().connect(sigc::mem_fun(*this,
							    &EditFilesWindow::_saveBoundary));
  
  ++row;
  _bdryFilesHelpLabel.set_text("Double-click to select file");
  _bdryFilesHelpLabel.modify_font(smaller_italic_font);
  _table3.attach(_bdryFilesHelpLabel, 0, 6, row, row+1,
		 (Gtk::AttachOptions)0,
		 (Gtk::AttachOptions)0,
		 0, 2);
  
  ++row;
  
  std::vector< std::string > bdry_files =
    _getBdryFileList(_bdryDirectoryEntry.get_text());
  
  _bdryFilesList.set_headers_visible(false);
  _bdryFilesList.modify_font(_defaultFont);

  // Set the padding for the list.  Note that the gtkmm-2.4 installed with
  // the Centos OS has a different API for the CellRenderer class so we can't
  // do this on that OS.
#ifndef OS_IS_CENTOS
  Gtk::CellRenderer *bdry_files_renderer =
    _bdryFilesList.get_column_cell_renderer(0);
  bdry_files_renderer->set_padding(0, 0);
#endif

  for (size_t i = 0; i < bdry_files.size(); ++i)
    _bdryFilesList.append_text(bdry_files[i]);
  _bdryFilesList.signal_row_activated().connect(sigc::mem_fun(*this,
							      &EditFilesWindow::_selectBoundaryFile));
  
  _bdryFilesScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC,
				      Gtk::POLICY_AUTOMATIC);
  _bdryFilesScrolledWindow.add(_bdryFilesList);
  
  _table3.attach(_bdryFilesScrolledWindow, 0, 6, row, row+1,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 0, 2);
  ++row;
  
  _latlonFilesLabel.set_text("Import ASCII Lat/Lon Boundary Files");
  _latlonFilesLabel.modify_font(larger_bold_font);
  _vbox0.pack_start(_latlonFilesLabel, false, false);
  
  _table5.set_border_width(0);
  _vbox0.pack_start(_table5, true, true);
  
  row = -1;
  
  ++row;
  _latlonDirectoryLabel.set_text(" Directory ");
  _latlonDirectoryLabel.modify_font(bold_font);
  _table5.attach(_latlonDirectoryLabel, 0, 1, row, row+1,
		 (Gtk::AttachOptions)0,
		 (Gtk::AttachOptions)0,
		 0, 2);
  
  _latlonDirectoryEntry.set_text("./");
  _latlonDirectoryEntry.modify_font(_defaultFont);
  _table5.attach(_latlonDirectoryEntry, 1, 6, row, row+1,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 (Gtk::AttachOptions)0,
		 0, 2);
  _latlonDirectoryEntry.signal_activate().connect(sigc::mem_fun(*this,
								&EditFilesWindow::_latlonDirectoryActivated));

  ++row;
  _latlonFilesHelpLabel.set_text("Double-click to select file");
  _latlonFilesHelpLabel.modify_font(smaller_italic_font);
  _table5.attach(_latlonFilesHelpLabel, 0, 6, row, row+1,
		 (Gtk::AttachOptions)0,
		 (Gtk::AttachOptions)0,
		 0, 2);
  
  ++row;

  std::vector< std::string > latlon_files =
    _getLatlonFileList(_latlonDirectoryEntry.get_text());
  
  _latlonFilesList.set_headers_visible(false);
  _latlonFilesList.modify_font(_defaultFont);

  // Set the padding for the list.  Note that the gtkmm-2.4 installed with
  // the Centos OS has a different API for the CellRenderer class so we can't
  // do this on that OS.
#ifndef OS_IS_CENTOS
  Gtk::CellRenderer *latlon_files_renderer =
    _latlonFilesList.get_column_cell_renderer(0);
  latlon_files_renderer->set_padding(0, 0);
#endif

  for (size_t i = 0; i < latlon_files.size(); ++i)
    _latlonFilesList.append_text(latlon_files[i]);
  _latlonFilesList.signal_row_activated().connect(sigc::mem_fun(*this,
								&EditFilesWindow::_selectLatlonFile));
  
  _latlonFilesScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC,
					Gtk::POLICY_AUTOMATIC);
  _latlonFilesScrolledWindow.add(_latlonFilesList);
  
  _table5.attach(_latlonFilesScrolledWindow, 0, 6, row, row+1,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 0, 2);
  ++row;
  
}


/**********************************************************************
 * Destructor
 */

EditFilesWindow::~EditFilesWindow()
{
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _bdryDirectoryActivated()
 */

void EditFilesWindow::_bdryDirectoryActivated()
{
  // Determine the directory to use

  std::string dir = _bdryDirectoryEntry.get_text();
  if (dir == "")
  {
    dir = sii_return_swpfi_dir(_parent->getFrameIndex());
    _bdryDirectoryEntry.set_text(dir);
  }
  
  // Update the list of files

  _bdryFilesList.clear_items();

  std::vector< std::string > file_list = _getBdryFileList(dir);
  for (size_t i = 0; i < file_list.size(); ++i)
    _bdryFilesList.append_text(file_list[i]);
}


/**********************************************************************
 * _cmdDirectoryActivated()
 */

void EditFilesWindow::_cmdDirectoryActivated()
{
  // Determine the directory to use

  std::string dir = _cmdDirectoryEntry.get_text();
  if (dir == "")
  {
    dir = sii_return_swpfi_dir(_parent->getFrameIndex());
    _cmdDirectoryEntry.set_text(dir);
  }
  
  // Update the list of files

  _cmdFilesList.clear_items();
  
  std::vector< std::string > file_list = _getCmdFileList(dir);
  for (size_t i = 0; i < file_list.size(); ++i)
    _cmdFilesList.append_text(file_list[i]);
}


/**********************************************************************
 * _getBdryFileList()
 */

std::vector< std::string > EditFilesWindow::_getBdryFileList(const std::string &dir) const
{
  std::vector< std::string > file_list = _getFileList(dir, "bnd");
  sort(file_list.begin(), file_list.end());
  
  return file_list;
}


/**********************************************************************
 * _getCmdFileList()
 */

std::vector< std::string > EditFilesWindow::_getCmdFileList(const std::string &dir) const
{
  std::vector< std::string > file_list = _getFileList(dir, "sed");
  sort(file_list.begin(), file_list.end());
  
  return file_list;
}


/**********************************************************************
 * _getFileList()
 */

std::vector< std::string > EditFilesWindow::_getFileList(const std::string &dir,
							 const std::string &type) const
{
  std::vector< std::string > file_list;
  
  // Open the directory

  DIR *dir_ptr;

  if ((dir_ptr = opendir(dir.c_str())) == 0)
  {
    char message[256];
    
    sprintf(message, "Cannot open directory %s\n", dir.c_str());
    solo_message(message);
    return file_list;
  }

  // Read the directory entries and add the appropriate ones to the list

  while (true)
  {
    struct dirent *entry = readdir(dir_ptr);
    if (entry == 0)
      break;

    std::string file_name =  entry->d_name;

    // If the file name doesn't contain the specified type, skip it

    if (file_name.find(type) == std::string::npos)
      continue;

    file_list.push_back(file_name);
  }

  // Close the directory

  closedir(dir_ptr);

  return file_list;
}


/**********************************************************************
 * _getLatlonFileList()
 */

std::vector< std::string > EditFilesWindow::_getLatlonFileList(const std::string &dir) const
{
  std::vector< std::string > file_list = _getFileList(dir, "llb");
  sort(file_list.begin(), file_list.end());
  
  return file_list;
}


/**********************************************************************
 * _latlonDirectoryActivated()
 */

void EditFilesWindow::_latlonDirectoryActivated()
{
  // Determine the directory to use

  std::string dir = _latlonDirectoryEntry.get_text();
  if (dir == "")
  {
    dir = sii_return_swpfi_dir(_parent->getFrameIndex());
    _latlonDirectoryEntry.set_text(dir);
  }
  
  // Update the list of files

  _latlonFilesList.clear_items();
  
  std::vector< std::string > file_list = _getLatlonFileList(dir);
  for (size_t i = 0; i < file_list.size(); ++i)
    _latlonFilesList.append_text(file_list[i]);
}


/**********************************************************************
 * _saveBoundary()
 */

void EditFilesWindow::_saveBoundary()
{
  // Get the directory name and comment

  std::string dir = _bdryDirectoryEntry.get_text();
  std::string comment = _bdryCommentEntry.get_text();
  
  if (dir == "")
  {
    dir = sii_return_swpfi_dir(_parent->getFrameIndex());
    _bdryDirectoryEntry.set_text(dir);
  }
  
  // Save the boundary

  SeBoundaryList *sebs = SeBoundaryList::getInstance();

  sebs->directoryText = dir;
  sebs->commentText = comment;

  SeBoundaryList *boundaries = SeBoundaryList::getInstance();
  boundaries->lastOperation = SeBoundaryList::BND_OTHER;
  xse_save_bnd();
  
  // Update our internal information

  _bdryDirectoryActivated();
}


/**********************************************************************
 * _saveCommands()
 */

void EditFilesWindow::_saveCommands()
{
  // Get the user-entered directory and comment

  std::string dir = _cmdDirectoryEntry.get_text();
  std::string comment = _cmdCommentEntry.get_text();
  
  if (dir == "")
  {
    dir = sii_return_swpfi_dir(_parent->getFrameIndex());
    _cmdDirectoryEntry.set_text(dir);
  }

  // Save the commands

  struct sed_command_files *scf = se_return_cmd_files_struct();

  scf->directory_text = dir;
  scf->comment_text = comment;

  std::vector< UiCommandToken > cmd_tokens;
  se_write_sed_cmds(cmd_tokens);
  SeBoundaryList *boundaries = SeBoundaryList::getInstance();
  boundaries->lastOperation = SeBoundaryList::BND_OTHER;

  _cmdDirectoryActivated();
}


/**********************************************************************
 * _selectBoundaryFile()
 */

void EditFilesWindow::_selectBoundaryFile(const Gtk::TreeModel::Path &path,
					  Gtk::TreeViewColumn *column)
{
  // Get the selected file name

  int row_number = atoi(path.to_string().c_str());
  std::string selected_file = _bdryFilesList.get_text(row_number);

  // Get the boundary from the file

  SeBoundaryList *sebs = SeBoundaryList::getInstance();

  sebs->directoryText = _bdryDirectoryEntry.get_text();
  sebs->fileNameText = selected_file;
  
  SeBoundaryList *boundaries = SeBoundaryList::getInstance();
  boundaries->lastOperation = SeBoundaryList::BND_OTHER;
  se_clr_all_bnds();
  se_redraw_all_bnds();
  xse_absorb_bnd();
}


/**********************************************************************
 * _selectCommandFile()
 */

void EditFilesWindow::_selectCommandFile(const Gtk::TreeModel::Path &path,
					 Gtk::TreeViewColumn *column)
{
  // Get the selected file name

  int row_number = atoi(path.to_string().c_str());
  std::string selected_file = _cmdFilesList.get_text(row_number);

  // Update the editor information

  struct sed_command_files *scf = se_return_cmd_files_struct();
  scf->directory_text = _cmdDirectoryEntry.get_text();
  scf->file_name_text = selected_file;

  // Read in the commands.  The commands from the file will end up in slm

  se_really_readin_cmds();

  std::vector< std::string > commands = se_update_list(CURRENT_CMDS_LIST);
  
  _parent->addCommands(commands);
}


/**********************************************************************
 * _selectLatlonFile()
 */

void EditFilesWindow::_selectLatlonFile(const Gtk::TreeModel::Path &path,
					Gtk::TreeViewColumn *column)
{
  // Get the selected file name

  int row_number = atoi(path.to_string().c_str());
  std::string selected_file = _latlonFilesList.get_text(row_number);

  std::string file_path =
    _latlonDirectoryEntry.get_text() + "/" + selected_file;
  
  int size;
  char *buf = absorb_zmap_bnd(file_path, 0, size);
  if (buf != 0)
  {
    se_unpack_bnd_buf(buf, size);
    free(buf);
  }
}
