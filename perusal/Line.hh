#ifndef LINE_HH_
#define LINE_HH_

#include "Graphic.hh"

class Line : public Graphic
{
public:

  /**
   * @brief Constructor.
   */
	Line();

  /**
   * @brief Constructor.
   *
   * @param[in] drawable  An opaque structure representing an object that can be drawn onto.
   * @param[in] dashflag  Flag indicating whether the line is dashed or not.
   *                      True for dashed line, false for normal line.
   * @param[in] x1        The x-axis of the starting point.
   * @param[in] y1        The y-axis of the starting point.
   * @param[in] x2        The x-axis of the ending point.
   * @param[in] y2        The y-axis of the ending point.
   */
	Line(GdkDrawable *drawable, gboolean dashflag, gint x1, gint y1, gint x2, gint y2);

  /**
   * @brief Constructor.
   *
   * @param[in] cr        Cairo context.
   * @param[in] dashflag  Flag indicating whether the line is dashed or not.
   *                      True for dashed line, false for normal line.
   * @param[in] x1        The x-axis of the starting point.
   * @param[in] y1        The y-axis of the starting point.
   * @param[in] x2        The x-axis of the ending point.
   * @param[in] y2        The y-axis of the ending point.
   */
	Line(cairo_t *cr, gboolean dashflag, gint x1, gint y1, gint x2, gint y2);

  /**
   * @brief Destructor.
   */
	virtual ~Line();
	
  /**
   * @brief Draw function. Override super class Draw() function.
   */
	void Draw();
	
private:  

  /////////////////////
  // Private members //
  /////////////////////
  
  gboolean m_dashflag;	
  int m_xt;
  int m_yt;	
};

#endif /*LINE_HH_*/
