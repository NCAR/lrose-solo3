#ifndef CIRCLE_HH_
#define CIRCLE_HH_

#include "Graphic.hh"
#include <dd_math.h>

class Circle : public Graphic
{
public:

  /**
   * @brief Constructor.
   */
	Circle();

  /**
   * @brief Constructor.
   *
   * @param[in] cr        Cairo context.
   * @param[in] dashflag  Flag indicating whether the line is dashed or not.
   *                      True for dashed line, false for normal line.
   * @param[in] x         The x coordinate of the center.
   * @param[in] y         The y coordinate of the center.
   * @param[in] radius    The radius of the circle. 
   */
	Circle(cairo_t *cr, gboolean dashflag, int x, int y, int radius);

  /**
   * @brief Destructor.
   */
	virtual ~Circle();
	
  /**
   * @brief Draw function. Override super class Draw() function.
   */
	void Draw();
	
private:  

  /////////////////////
  // Private members //
  /////////////////////
  
  int m_radius;
  gboolean m_dashflag;
  
};

#endif /*CIRCLE_HH_*/
