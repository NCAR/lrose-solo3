/* 	$Id$	 */

#include <ColorManager.hh>
#include <se_utils.hh>
#include "sii_colors_stuff.hh"
#include "sii_param_widgets.hh"
#include "soloii.h"
#include "soloii.hh"

/* c---------------------------------------------------------------------- */

gboolean sii_print_str( gpointer a_str, gpointer nada )
{
  printf( "It's: %s\n", (gchar *)a_str );
  return( FALSE );		/* to continue a tree traverse */
}

/* c---------------------------------------------------------------------- */

void sii_print_hash_str( gpointer key, gpointer value, gpointer user_data )
{
  printf( "It's: %s\n", (gchar *)key );
}

/* c---------------------------------------------------------------------- */

void sii_set_default_colors (void)
{
  ColorManager *color_manager = ColorManager::getInstance();
  gint ii = 0;
  frame_border_colors[ii++] = color_manager->getColor("white");
  frame_border_colors[ii++] = color_manager->getColor("red");
  frame_border_colors[ii++] = color_manager->getColor("green");
  frame_border_colors[ii++] = color_manager->getColor("blue");
  frame_border_colors[ii++] = color_manager->getColor("magenta");
  frame_border_colors[ii++] = color_manager->getColor("cyan");
  frame_border_colors[ii++] = color_manager->getColor("orange");
  frame_border_colors[ii++] = color_manager->getColor("chocolate");
  frame_border_colors[ii++] = color_manager->getColor("wheat");
  frame_border_colors[ii++] = color_manager->getColor("seagreen");
  frame_border_colors[ii++] = color_manager->getColor("lavender");
  frame_border_colors[ii++] = color_manager->getColor("salmon");
				/* should be a 12 by now */

}
