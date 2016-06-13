#include "Label.hh"

Label::Label()
{
}

Label::~Label()
{
}

Label::Label(cairo_t *cr) : Graphic (cr)
{
}

Label::Label(cairo_t *cr, const std::string &text,
	     gint x, gint y) : Graphic (cr, x, y)
{
  m_label = text;
}

void Label::SetPangoFont(gchar *font)
{
	m_font = font;	
}

void Label::Draw()
{
  PangoLayout *layout = pango_cairo_create_layout(m_cr);

  cairo_move_to(m_cr,m_x,m_y);

  PangoFontDescription *fontdesc = pango_font_description_from_string(m_font);
  pango_layout_set_font_description(layout, fontdesc);
  pango_font_description_free(fontdesc);
  pango_cairo_update_layout(m_cr, layout);
  
  pango_layout_set_text(layout, m_label.c_str(), m_label.size());
  pango_cairo_show_layout(m_cr, layout);
    
  g_object_unref (layout);
  cairo_stroke (m_cr);
}


void Label::GetLayoutPixelSize(const std::string &text,
			       gint *width, gint *height)
{		
  PangoLayout *layout = pango_cairo_create_layout(m_cr);
  PangoFontDescription *fontdesc = pango_font_description_from_string(m_font);
  pango_layout_set_font_description(layout, fontdesc);
  pango_cairo_update_layout(m_cr, layout);
	
  pango_layout_set_text(layout, text.c_str(), text.size());
  pango_layout_get_pixel_size(layout,width,height);
  
  pango_font_description_free(fontdesc);
  g_object_unref (layout);
}

void Label::GetLayoutPixelSize(cairo_t *cr, gchar *text, gint *width, gint *height, gchar *font)
{		
	PangoLayout *layout = pango_cairo_create_layout(cr);
	PangoFontDescription *fontdesc = pango_font_description_from_string(font);
	pango_layout_set_font_description(layout, fontdesc);
	pango_cairo_update_layout(cr, layout);
	
    pango_layout_set_text(layout,text,strlen(text));
	pango_layout_get_pixel_size(layout,width,height);

	pango_font_description_free(fontdesc);
    g_object_unref (layout);
}
