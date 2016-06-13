#include <iostream>

#include <sii_externals.h>
#include <sii_param_widgets.hh>
#include <sii_utils.hh>
#include <solo2.hh>
#include <sp_save_frms.hh>

#include "DataWindow.hh"


const std::string DataWindow::_clickTemplate =
  "W0000.00km.-000.00 -000.00 -000.00W";
const std::string DataWindow::_sampleClickText =
  "Click in a data window to select a grid point.";

/**********************************************************************
 * Constructor
 */

DataWindow::DataWindow(const Pango::FontDescription &default_font,
		       const int frame_index) :
  Gtk::Window(),
  _frameIndex(frame_index),
  _defaultFont(default_font)
{
  // Create the fonts needed for the widgets

  Pango::FontDescription monospace_font = _defaultFont;
  monospace_font.set_family("Monospace");
  
  // Set the window title

  char title_string[1024];
  sprintf(title_string, "Frame %d  Data", _frameIndex + 1);
  set_title(title_string);

  // NOTE: Need to update calculating the height/width of the window

//  gtk_widget_realize(window);
//  gint hheight, hwidth;
//  cairo_t *cr = gdk_cairo_create(GDK_DRAWABLE(window->window));
//  Label::GetLayoutPixelSize(cr, const_cast <gchar*> (click_tmplt),
//			    &hwidth, &hheight,med_fxd_fonta);
//  width =  hwidth + 86;
//  height = hheight;

  int width = 500;
  int height = 20;
  int nlines = 11;
  
  // Create the containers for widget positioning

  _vbox.set_homogeneous(false);
  _vbox.set_spacing(4);
  add(_vbox);

  _textView.set_size_request(width, nlines * height);
  _vbox.pack_start(_textView, 1, 1, 0);
  
  setText(_sampleClickText);
  
  _textView.modify_font(monospace_font);
  _textView.set_editable(false);
}


/**********************************************************************
 * Destructor
 */

DataWindow::~DataWindow()
{
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
