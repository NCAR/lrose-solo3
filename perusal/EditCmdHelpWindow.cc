#include <iostream>

#include "EditCmdHelpWindow.hh"

/**********************************************************************
 * Constructor
 */

EditCmdHelpWindow::EditCmdHelpWindow(const Pango::FontDescription &default_font,
				     const std::string help_text) :
  Gtk::Window(),
  _defaultFont(default_font)
{
  // Create the fonts needed for the widgets

  Pango::FontDescription smaller_italic_font = _defaultFont;
  smaller_italic_font.set_size(_defaultFont.get_size() - (2 * 1024));
  smaller_italic_font.set_style(Pango::STYLE_ITALIC);
  
  // Construct the window title

  set_title("Edit Command Help");
  set_border_width(0);
  
  // Set up the widgets

  _vbox.set_homogeneous(false);
  _vbox.set_spacing(4);
  add(_vbox);
  
  // NOTE: It looks like the user can click in the text and put some of it
  // on the clipboard.  I'm not sure exactly how this works or how to do this
  // with the latest widgets.  Leave it alone for now.

  _textView.set_size_request(480, 300);
  _textView.modify_font(default_font);
  _textView.set_editable(false);

  _textViewWindow.set_policy(Gtk::POLICY_AUTOMATIC,
			     Gtk::POLICY_AUTOMATIC);
  _textViewWindow.add(_textView);
  
  _vbox.pack_start(_textViewWindow, true, true, 0);
  
  Glib::RefPtr< Gtk::TextBuffer > buffer = _textView.get_buffer();
  buffer->set_text(help_text);
  
  _dismissButton.set_label("Dismiss");
  _dismissButton.modify_font(_defaultFont);
  _vbox.pack_start(_dismissButton, false, false, 0);
  _dismissButton.signal_clicked().connect(sigc::mem_fun(*this,
							&EditCmdHelpWindow::_dismiss));
  
  show_all();
}


/**********************************************************************
 * Destructor
 */

EditCmdHelpWindow::~EditCmdHelpWindow()
{
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _dismiss()
 */

void EditCmdHelpWindow::_dismiss()
{
  delete this;
}
