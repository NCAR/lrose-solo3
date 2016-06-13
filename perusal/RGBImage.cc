#include "RGBImage.hh"

RGBImage::RGBImage()
{
}

RGBImage::~RGBImage()
{
}

RGBImage::RGBImage(cairo_t *cr, gint x, gint y, const guchar *data,
				gboolean has_alpha, int bits_per_sample, 
				int width, int height, int rowstride) : Graphic (cr, x, y)
{
	m_bits_per_sample = bits_per_sample;
	m_width = width;
	m_height = height;
	m_rowstride = rowstride;
	m_has_alpha = has_alpha;
	m_data = data;
}

void RGBImage::Draw()
{
	GdkPixbuf *pixbuf_src = gdk_pixbuf_new_from_data(
									m_data,
									GDK_COLORSPACE_RGB,
                                    m_has_alpha,
                                    m_bits_per_sample,
                                    m_width,
                                    m_height,
                                    m_rowstride,
                                    NULL,
                                    NULL);
	gdk_cairo_set_source_pixbuf (m_cr, pixbuf_src, m_x, m_y);
	cairo_paint (m_cr);
}
