#include "Graphic.hh"

Graphic::Graphic()
{
}

Graphic::~Graphic()
{
}

Graphic::Graphic(cairo_t *cr)
{
	m_cr = cr;
}

Graphic::Graphic(GdkDrawable *drawable)
{
	m_cr = gdk_cairo_create(drawable);
}

Graphic::Graphic(cairo_t *cr, int x, int y)
{
	m_x = x;
	m_y = y;
	m_cr = cr;
}

Graphic::Graphic(GdkDrawable *drawable, int x, int y)
{
	m_x = x;
	m_y = y;
	m_cr = gdk_cairo_create(drawable);
}

void Graphic::SetForegroundColor(GdkColor *gcolor)
{
  gdk_cairo_set_source_color(m_cr,gcolor);
}

void Graphic::MoveTo(gint x, gint y)
{
	m_x = x;
	m_y = y;	
}

void Graphic::Draw()
{
}
