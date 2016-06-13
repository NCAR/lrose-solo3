#include "CRectangle.hh"

CRectangle::CRectangle()
{
}

CRectangle::~CRectangle()
{
}

CRectangle::CRectangle(cairo_t *cr, gboolean fill, gint x, gint y, gint width, gint height) : Graphic(cr, x, y)
{
	m_width = width;
	m_height = height;
	m_fill = fill;
}

CRectangle::CRectangle(GdkDrawable *drawable, gboolean fill, gint x, gint y, gint width, gint height) : Graphic(drawable, x, y)
{
	m_width = width;
	m_height = height;
	m_fill = fill;
}

void CRectangle::Draw()
{
	double dash_list[] = { };
	cairo_set_dash(m_cr, dash_list, 0, 0);
	cairo_rectangle(m_cr, m_x, m_y, m_width, m_height);
	if (m_fill)
		cairo_fill(m_cr);
	cairo_stroke(m_cr);
}

void CRectangle::SetLineWidth(int width)
{
	cairo_set_line_width (m_cr, width);
}

void CRectangle::SetWidthHeight(gint width, gint height)
{
	m_width = width;
	m_height = height;
}
