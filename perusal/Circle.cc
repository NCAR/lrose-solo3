#include "Circle.hh"

Circle::Circle()
{
}

Circle::~Circle()
{
}

Circle::Circle(cairo_t *cr, gboolean dashflag, int x, int y, int radius) : Graphic(cr, x, y)
{
	m_radius = radius;
	m_dashflag = dashflag;
}

void Circle::Draw()
{
	// Cairo set line style
	if (!m_dashflag) {
		double dash_list[] = { };
		cairo_set_dash(m_cr, dash_list, 0, 0);
	}
	else {
	 	double dash_list[] = { 5, 3 };
		cairo_set_dash(m_cr, dash_list, sizeof (dash_list) / sizeof(dash_list[0]), 0);
	}

	cairo_set_line_width (m_cr, 1.0);
	cairo_set_line_join(m_cr, CAIRO_LINE_JOIN_MITER);  // GDK_JOIN_MITER - the sides of each line are extended to meet at an angle
	cairo_set_line_cap(m_cr, CAIRO_LINE_CAP_BUTT);  // GDK_CAP_NOT_LAST - the ends of the lines are drawn squared off and extending to the coordinates of the end point
	
	cairo_arc(m_cr, m_x, m_y, m_radius, 0, 2*M_PI);
}
