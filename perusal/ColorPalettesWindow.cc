#include <iostream>

#include <PaletteManager.hh>
#include <ParameterWindow.hh>

#include "ColorPalettesWindow.hh"


/**********************************************************************
 * Constructor
 */

ColorPalettesWindow::ColorPalettesWindow(ParameterWindow *parent,
					 const Pango::FontDescription &default_font) :
  Gtk::Window(),
  _parent(parent),
  _defaultFont(default_font),
  _table(2, 16, false),
  _paletteNames(1)
{
  // Create the fonts needed for the widgets

  Pango::FontDescription smaller_italic_font = _defaultFont;
  smaller_italic_font.set_size(_defaultFont.get_size() - (2 * 1024));
  smaller_italic_font.set_style(Pango::STYLE_ITALIC);
  
  // Construct the window title

  char title_string[1024];
  sprintf(title_string, "Frame %d  Color Palettes Widget",
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
						       &ColorPalettesWindow::hide));
 
  _clickLabel.set_text("(Double-click to select)");
  _clickLabel.set_justify(Gtk::JUSTIFY_CENTER);
  _clickLabel.modify_font(smaller_italic_font);
  _vbox.pack_start(_clickLabel, false, false, 0);
  
  _vbox.pack_start(_table, true, true, 0);
  
  _paletteNames.set_headers_visible(false);
  _paletteNames.modify_font(_defaultFont);

  // Set the padding for the list.  Note that the gtkmm-2.4 installed with
  // the Centos OS has a different API for the CellRenderer class so we can't
  // do this on that OS.
#ifndef OS_IS_CENTOS
  Gtk::CellRenderer *renderer = _paletteNames.get_column_cell_renderer(0);
  renderer->set_padding(0, 0);
#endif

  updatePaletteList();
  _paletteNames.signal_row_activated().connect(sigc::mem_fun(*this,
							     &ColorPalettesWindow::_selectPaletteName));
  
  _paletteNamesScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC,
					 Gtk::POLICY_AUTOMATIC);
  
  _paletteNamesScrolledWindow.add(_paletteNames);
  _table.attach(_paletteNamesScrolledWindow, 0, 1, 0, 1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);
  
  resize(360, 360);
}


/**********************************************************************
 * Destructor
 */

ColorPalettesWindow::~ColorPalettesWindow()
{
}


/**********************************************************************
 * updatePaletteList()
 */

void ColorPalettesWindow::updatePaletteList()
{
  std::vector< std::string > palette_names = 
    PaletteManager::getInstance()->getPalettesList();
  
  _paletteNames.clear_items();
  for (std::vector< std::string>::const_iterator palette_name =
	 palette_names.begin();
       palette_name != palette_names.end(); ++palette_name)
    _paletteNames.append_text(*palette_name);
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _selectPaletteName()
 */

void ColorPalettesWindow::_selectPaletteName(const Gtk::TreeModel::Path &path,
					     Gtk::TreeViewColumn *column)
{
  int row_number = atoi(path.to_string().c_str());
  std::string palette_text = _paletteNames.get_text(row_number);
  
  // Extract the palette name from the string

  std::string palette_name = palette_text.substr(0, palette_text.find(" "));
  
  // Update the palette in the parent window

  _parent->setPalette(palette_name);

  updatePaletteList();
}
