/* 	$Id$	 */

# ifndef SII_IMAGE_H
# define SII_IMAGE_H


typedef struct {
  int bits_per_pixel;
  int bytes_per_line;
  int byte_order;

  // In my little testing, these values are both set to 0.  Maybe they aren't
  // used?  The width and height values in the SiiFrameConfig object are used
  // as described below.

  int width;
  int height;

  // The data values followed by the RGB values for the data.  The image data
  // is stored as guchar values, and there are (height x width) values.  These
  // are followed by the RGB values for each data value.  Each color value is
  // stored in a guchar, so there are (height x width x 3) of these values.
  // The height and width values come from the SiiFrameConfig object that points
  // here rather than from this structure.
  //
  // NOTE: We need to divide this into two separate pointers in our refactoring.

  void *data;

} SiiImage;


# endif 
