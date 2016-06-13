#ifndef RGBIMAGE_HH_
#define RGBIMAGE_HH_
#include "Graphic.hh"

class RGBImage : public Graphic
{
public:

  /**
   * @brief Constructor.
   */
	RGBImage();

  /**
   * @brief Constructor.
   *
   * @param[in] cr               Cairo context.
   * @param[in] x                The x coordinate of the top-left corner in the drawable. 
   * @param[in] y                The y coordinate of the top-left corner in the drawable. 
   * @param[in] data             Image data in 8-bit/sample packed format.
   * @param[in] has_alpha        Whether the data has an opacity channel.
   * @param[in] bits_per_sample  Number of bits per sample.
   * @param[in] width            Width of the image in pixels, must be > 0.
   * @param[in] height           Height of the image in pixels, must be > 0.
   * @param[in] rowstride        Distance in bytes between row starts.
   */
	RGBImage(cairo_t *cr, gint x, gint y, const guchar *data,
				gboolean has_alpha, int bits_per_sample, 
				int width, int height, int rowstride);

  /**
   * @brief Destructor.
   */
	virtual ~RGBImage();
	
  /**
   * @brief Draw function. Override super class Draw() function.
   */
	void Draw();
	
private:  

  /////////////////////
  // Private members //
  /////////////////////
  
  int m_bits_per_sample;
  int m_width;
  int m_height;
  int m_rowstride;
  gboolean m_has_alpha;
  const guchar *m_data;
};

#endif /*RGBIMAGE_HH_*/
