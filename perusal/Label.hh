#ifndef LABEL_HH_
#define LABEL_HH_

#include "Graphic.hh"
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
using namespace std;

class Label : public Graphic
{
public:

  /**
   * @brief Constructor.
   */
	Label();

  /**
   * @brief Constructor.
   * 
   * @param[in] cr     Cairo context.
   */
	Label(cairo_t *cr);

  /**
   * @brief Constructor.
   * 
   * @param[in] cr     Cairo context.
   * @param[in] text   The text needs to draw.
   * @param[in] x      The X coordinate of the position of the text.
   * @param[in] y      The Y coordinate of the position of the text.
   */

  Label(cairo_t *cr, const std::string &text, gint x, gint y);

  /**
   * @brief Destructor.
   */
	virtual ~Label();
	
  /**
   * @brief Draw function. Override super class Draw() function.
   */
	void Draw();

  /**
   * @brief Get layout pixel size. 
   *        Determines the logical width and height of a PangoLayout in device units.
   *
   * @param[in] text    The text needs to measure.
   * @param[in] width   The logical width.
   * @param[in] height  The logical height.
   */

  void GetLayoutPixelSize(const std::string &text, gint *width, gint *height);
	
  /**
   * @brief Get layout pixel size. 
   *        Determines the logical width and height of a PangoLayout in device units.
   *
   * @param[in] cr     Cairo context.
   * @param[in] text    The text needs to measure.
   * @param[in] width   The logical width.
   * @param[in] height  The logical height.
   * @param[in] font    The font name.
   */
	static void GetLayoutPixelSize(cairo_t *cr, gchar *text, gint *width, gint *height, gchar *font);

  /**
   * @brief Set font. 
   *
   * @param[in] font    The font name.
   */
	void SetPangoFont(gchar *font);
	
private:  

  /////////////////////
  // Private members //
  /////////////////////
  
  std::string m_label;
  gchar *m_font;
  
};

#endif /*LABEL_HH_*/
