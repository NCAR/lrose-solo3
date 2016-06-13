#ifndef GRAPHIC_HH_
#define GRAPHIC_HH_
#ifndef __APPLE__
#include <gdk/gdk.h>
#endif // __APPLE__
#include <cairo/cairo.h>
#include <cairo.h>
#include <pango/pangocairo.h>
#include <string.h>
#include <cairomm/cairomm.h>
#include <cairomm/context.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/widget.h>

class Graphic
{
public:

  /**
   * @brief Constructor.
   */
	Graphic();

  /**
   * @brief Constructor.
   *
   * @param[in] cr         Cairo context.
   */
	Graphic(cairo_t *cr);

  /**
   * @brief Constructor.
   *
   * @param[in] drawable  An opaque structure representing an object that can be drawn onto.
   */
	Graphic(GdkDrawable *drawable);

  /**
   * @brief Constructor.
   *
   * @param[in] cr         Cairo context.
   * @param[in] x          The x coordinate of the top-left corner in the drawable. 
   * @param[in] y          The y coordinate of the top-left corner in the drawable. 
   */
	Graphic(cairo_t *cr, int x, int y);

  /**
   * @brief Constructor.
   *
   * @param[in] drawable   An opaque structure representing an object that can be drawn onto.
   * @param[in] x          The x coordinate of the top-left corner in the drawable. 
   * @param[in] y          The y coordinate of the top-left corner in the drawable. 
   */
	Graphic(GdkDrawable *drawable, int x, int y);

  /**
   * @brief Destructor.
   */
	virtual ~Graphic();
	
  /**
   * @brief Draw function. Should be overridden in inherited class.
   */
	virtual void Draw();
	
  /**
   * @brief Set color.
   *
   * @param[in] gcolor     GdkColor.
   */
	virtual void SetForegroundColor(GdkColor *gcolor);

  /**
   * @brief Change the starting point x and y.
   *
   * @param[in] x     The x coordinate to move to.
   * @param[in] y     The y coordinate to move to..
   */
	virtual void MoveTo(gint x, gint y);

protected:  

  ///////////////////////
  // Protected members //
  ///////////////////////
  
  GdkColor *m_gcolor;
  cairo_t *m_cr;
  gint m_x;
  gint m_y;
  	
};

#endif /*GRAPHIC_HH_*/
