#include <iostream>

#include <sii_externals.h>
#include <sii_param_widgets.hh>
#include <sii_utils.hh>
#include <solo2.hh>
#include <sp_save_frms.hh>

#include "ImportColorTableWindow.hh"


/**********************************************************************
 * Constructor
 */

ImportColorTableWindow::ImportColorTableWindow(ParameterWindow *parent,
					       const Pango::FontDescription &default_font) :
  Gtk::Window(),
  _parent(parent),
  _defaultFont(default_font)
{
  // Set the window title

  char title_string[1024];
  sprintf(title_string, "Frame %d  Entry Widget",
	  _parent->getFrameIndex() + 1);
  set_title(title_string);

  // Create the containers for widget positioning

  _vbox.set_homogeneous(false);
  _vbox.set_spacing(2);
  add(_vbox);

  _promptLabel.set_text("\n Type the full path name of the color table file and press return \n");
  _promptLabel.modify_font(_defaultFont);
  _vbox.add(_promptLabel);
  
  _colorTablePathEntry.modify_font(_defaultFont);
  _vbox.add(_colorTablePathEntry);
  _colorTablePathEntry.signal_activate().connect(sigc::mem_fun(*this,
							       &ImportColorTableWindow::_setColorTablePath));

  // Now set up the buttons across the bottom of the window

  _hbox.set_homogeneous(false);
  _hbox.set_spacing(2);
  _vbox.add(_hbox);
  
  _okButton.set_label("OK");
  _okButton.modify_font(_defaultFont);
  _hbox.pack_start(_okButton, true, true, 0);
  _okButton.signal_clicked().connect(sigc::mem_fun(*this,
						   &ImportColorTableWindow::_setColorTablePath));
  
  _fileSelButton.set_label("Colors FileSelect");
  _fileSelButton.modify_font(_defaultFont);
  _hbox.pack_start(_fileSelButton, true, true, 0);
  _fileSelButton.signal_clicked().connect(sigc::mem_fun(*this,
							&ImportColorTableWindow::_selectColorTablePath));

  _cancelButton.set_label("Cancel");
  _cancelButton.modify_font(_defaultFont);
  _hbox.pack_start(_cancelButton, true, true, 0);
  _cancelButton.signal_clicked().connect(sigc::mem_fun(*this,
						       &ImportColorTableWindow::hide));
}


/**********************************************************************
 * Destructor
 */

ImportColorTableWindow::~ImportColorTableWindow()
{
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _colorTableFileSelected()
 */

void ImportColorTableWindow::_colorTableFileSelected(Gtk::FileSelection *file_selector)
{
  // Get the selected file path from the window, then delete the file
  // selection object since we don't need it anymore

  Glib::ustring file_path = file_selector->get_filename();
  delete file_selector;

  // Set the file path in the entry widget

  _colorTablePathEntry.set_text(file_path);
  _setColorTablePath();
}


/**********************************************************************
 * _deleteFileSelectionPointer()
 */

void ImportColorTableWindow::_deleteFileSelectionPointer(Gtk::FileSelection *file_selector)
{
  delete file_selector;
}


/**********************************************************************
 * _setColorTablePath()
 */

void ImportColorTableWindow::_setColorTablePath()
{
  std::string color_table_path = _colorTablePathEntry.get_text();
  
  std::string color_table_name =
    ColorTableManager::getInstance()->absorbTable(color_table_path.c_str());

  _parent->setColorTableName(color_table_name);

  hide();
}

/**********************************************************************
 * _selectColorTablePath()
 */

void ImportColorTableWindow::_selectColorTablePath()
{
  const gchar *root_dir = getenv("COLORS_FILESEL");

  if (root_dir == 0)
    root_dir = "/";

  // Create the file selector widget

  Gtk::FileSelection *file_selector =
    new Gtk::FileSelection("colors_dir file selection dialog");
  file_selector->hide_fileop_buttons();
  file_selector->set_filename(root_dir);

  // Connect callbacks to the buttons

  file_selector->get_ok_button()->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this,
										    &ImportColorTableWindow::_colorTableFileSelected),
								      file_selector));
  file_selector->get_cancel_button()->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this,
											&ImportColorTableWindow::_deleteFileSelectionPointer),
									  file_selector));

  // Show the selector

  file_selector->show();

  // Hide this window

  hide();
}
