#include <iostream>

#include <solo2.hh>

#include "ImageGenWindow.hh"


/**********************************************************************
 * Constructor
 */

ImageGenWindow::ImageGenWindow() :
  Gtk::Window()
{
  set_title("Solo3 PNG Images");

  _vbox.set_homogeneous(false);
  _vbox.set_spacing(0);
  add(_vbox);

  _hbox.set_homogeneous(false);
  _hbox.set_spacing(2);
  _hbox.set_border_width(4);
  _vbox.add(_hbox);

  _makeImageButton.set_label(" Make PNG Image ");
  _hbox.pack_start(_makeImageButton, true, true, 0);
  _makeImageButton.signal_clicked().connect(sigc::mem_fun(*this,
							  &ImageGenWindow::_makeImage));

  _cancelButton.set_label(" Done ");
  _hbox.pack_start(_cancelButton, true, true, 0);
  _cancelButton.signal_clicked().connect(sigc::mem_fun(*this,
						       &ImageGenWindow::hide));
  _table.resize(1, 5);
  _table.set_homogeneous(false);
  _table.set_border_width(4);
  _vbox.add(_table);

  gint row = 0;
  guint xpadding = 0;
  guint ypadding = 3;

  _imageDirLabel.set_text( " Image Dir " );
  _table.attach(_imageDirLabel, 0, 1, row, row+1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		xpadding, ypadding);

  _table.attach(_imageDirEntry, 1, 5, row, row+1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		xpadding, ypadding);
  _imageDirEntry.set_text("./");
}


/**********************************************************************
 * Destructor
 */

ImageGenWindow::~ImageGenWindow()
{
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _makeImage()
 */

void ImageGenWindow::_makeImage()
{
#ifdef __APPLE__
  sii_message("WARNING: images saved on MacOS will have swapped "
              "blue and red channels");
#endif
  sii_png_image_prep(_imageDirEntry.get_text().c_str());
}
