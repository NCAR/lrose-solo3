#include "Line.hh"

Line::Line()
{
}

Line::~Line()
{
}

Line::Line(cairo_t *cr, gboolean dashflag, gint x1, gint y1, gint x2, gint y2) : Graphic(cr, x1, y1)
{
	m_dashflag = dashflag;
	m_xt = x2;
	m_yt = y2;
}

Line::Line(GdkDrawable *drawable, gboolean dashflag, gint x1, gint y1, gint x2, gint y2) : Graphic(drawable, x1, y1)
{
	m_dashflag = dashflag;
	m_xt = x2;
	m_yt = y2;
}

void Line::Draw()
{
  if (!m_dashflag) {
    double dash_list[] = { };
  	cairo_set_dash(m_cr, dash_list, 0, 0);
  }
  else {
  	double dash_list[] = { 4, 5 };
	cairo_set_dash(m_cr, dash_list, sizeof (dash_list) / sizeof(dash_list[0]), 0);
  }
  cairo_set_line_width (m_cr, 1.0);
  cairo_set_line_join(m_cr, CAIRO_LINE_JOIN_MITER);
  cairo_set_line_cap(m_cr, CAIRO_LINE_CAP_BUTT);
  cairo_move_to (m_cr, m_x, m_y);
  cairo_line_to (m_cr, m_xt, m_yt);
  cairo_stroke(m_cr);	
}

