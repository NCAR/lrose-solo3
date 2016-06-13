#ifndef CRECTANGLE_HH_
#define CRECTANGLE_HH_

#include "Graphic.hh"

class CRectangle : public Graphic
{
public:

  /**
   * @brief Constructor.
   */
	CRectangle();

  /**
   * @brief Constructor.
   *
   * @param[in] cr         Cairo context.
   * @param[in] fill       Flag indicating whether to fill the rectangle.
   *                       True for fill, false for not fill.
   * @param[in] x          The X coordinate of the top left corner of the rectangle.
   * @param[in] y          The Y coordinate of the top left corner of the rectangle.
   * @param[in] width      The width of the rectangle.
   * @param[in] height     The height of the rectangle.
   */
	CRectangle(cairo_t *cr, gboolean fill, gint x, gint y, gint width, gint height);

  /**
   * @brief Constructor.
   *
   * @param[in] drawable   An opaque structure representing an object that can be drawn onto.
   * @param[in] fill       Flag indicating whether to fill the rectangle.
   *                       True for fill, false for not fill.
   * @param[in] x          The X coordinate of the top left corner of the rectangle.
   * @param[in] y          The Y coordinate of the top left corner of the rectangle.
   * @param[in] width      The width of the rectangle.
   * @param[in] height     The height of the rectangle.
   */
	CRectangle(GdkDrawable *drawable, gboolean fill, gint x, gint y, gint width, gint height) ;

  /**
   * @brief Destructor.
   */
	virtual ~CRectangle();
	
  /**
   * @brief Draw function. Override super class Draw() function.
   */
	void Draw();

  /**
   * @brief Set line width.
   *
   * @param[in] width      Line width.
   */
	void SetLineWidth(int width);

  /**
   * @brief Change width and height.
   *
   * @param[in] width      The width of the rectangle.
   * @param[in] height     The height of the rectangle.
   */
	void SetWidthHeight(gint width, gint height);

private:  

  /////////////////////
  // Private members //
  /////////////////////
  
  int m_width;
  int m_height;
  gboolean m_fill;		
};

#endif /*CRECTANGLE_HH_*/
