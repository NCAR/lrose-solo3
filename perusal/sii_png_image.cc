/* 	$Id$	 */

#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "sii_externals.h"      // for main_window
#include "sii_utils.hh"


/* c---------------------------------------------------------------------- */

int sii_png_image(char *fname)
{
  Glib::ustring msg;

  // Get the main GdkWindow
  GdkWindow * w = main_window->gdkWindow();

  // Extract a pixbuf from the main window contents
  gint width, height;
  gdk_drawable_get_size(GDK_DRAWABLE(w), &width, &height);

  GdkPixbuf *pb = gdk_pixbuf_get_from_drawable(NULL, GDK_DRAWABLE(w), NULL,
                                               0, 0, 0, 0, width, height);
  if (! pb) {
    sii_message("Could not extract image from window.");
    return 1;
  }

  // Write the pixbuf to the specified PNG file
  std::string image_path = std::string(fname) + ".png";
  GError * err = NULL;
  if (gdk_pixbuf_save(pb, image_path.c_str(), "png", &err, NULL) == FALSE) {
    sii_message(err->message);
    return 1;
  }

  msg = Glib::ustring("Wrote image to ") + image_path.c_str();
  sii_message(msg);
  return 0;
}
