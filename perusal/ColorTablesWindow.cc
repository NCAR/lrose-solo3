#include <iostream>

#include <ParameterWindow.hh>

#include "ColorTablesWindow.hh"


/**********************************************************************
 * Constructor
 */

ColorTablesWindow::ColorTablesWindow(ParameterWindow *parent,
				     const Pango::FontDescription &default_font) :
  Gtk::Window(),
  _parent(parent),
  _defaultFont(default_font),
  _table(2, 16, false),
  _tableNames(1)
{
  // Create the fonts needed for the widgets

  Pango::FontDescription smaller_italic_font = _defaultFont;
  smaller_italic_font.set_size(_defaultFont.get_size() - (2 * 1024));
  smaller_italic_font.set_style(Pango::STYLE_ITALIC);
  
  // Construct the window title

  char title_string[1024];
  sprintf(title_string, "Frame %d  Color Tables Widget",
	  _parent->getFrameIndex() + 1);
  
  set_title(title_string);
  set_border_width(0);
  
  // Set up the widgets

  _vbox.set_homogeneous(false);
  _vbox.set_spacing(4);
  add(_vbox);
  
  _cancelButton.set_label("Cancel");
  _cancelButton.modify_font(_defaultFont);
  _vbox.pack_start(_cancelButton, false, false, 0);
  _cancelButton.signal_clicked().connect(sigc::mem_fun(*this,
						       &ColorTablesWindow::hide));
 
  _clickLabel.set_text("(Double-click to select)");
  _clickLabel.set_justify(Gtk::JUSTIFY_CENTER);
  _clickLabel.modify_font(smaller_italic_font);
  _vbox.pack_start(_clickLabel, false, false, 0);
  
  _vbox.pack_start(_table, true, true, 0);
  
  _tableNames.set_headers_visible(false);
  _tableNames.modify_font(_defaultFont);

  // Set the padding for the list.  Note that the gtkmm-2.4 installed with
  // the Centos OS has a different API for the CellRenderer class so we can't
  // do this on that OS.
#ifndef OS_IS_CENTOS
  Gtk::CellRenderer *renderer = _tableNames.get_column_cell_renderer(0);
  renderer->set_padding(0, 0);
#endif

  setTableNamesList(ColorTableManager::getInstance()->getTableNames());
  _tableNames.signal_row_activated().connect(sigc::mem_fun(*this,
							   &ColorTablesWindow::_selectTableName));
  
  _tableNamesScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC,
				       Gtk::POLICY_AUTOMATIC);
  
  _tableNamesScrolledWindow.add(_tableNames);
  _table.attach(_tableNamesScrolledWindow, 0, 1, 0, 1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);
  
  resize(220, 220);
}


/**********************************************************************
 * Destructor
 */

ColorTablesWindow::~ColorTablesWindow()
{
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _selectTableName()
 */

void ColorTablesWindow::_selectTableName(const Gtk::TreeModel::Path &path,
					 Gtk::TreeViewColumn *column)
{
  int row_number = atoi(path.to_string().c_str());
  
  std::string table_name = _tableNames.get_text(row_number);
  
  _parent->setColorTableName(table_name);
}
