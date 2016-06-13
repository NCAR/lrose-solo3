/* 	$Id$	 */

#ifndef dorade_h
#define dorade_h

#include<gtk/gtk.h>

#ifndef SCOPE
#define SCOPE extern
#endif


 SCOPE PLOT  plot;
 SCOPE PLOT  *pltptr;

 SCOPE WIDGET_STRUCT widgetinfo;
 SCOPE WIDGET_STRUCT *widgetptr;

 SCOPE PIXMAP_STRUCT dispbitmaps;
 SCOPE PIXMAP_STRUCT *pixptr;

 SCOPE LABELBAR_STRUCT labelbar;
 SCOPE LABELBAR_STRUCT *labelbrptr;

 SCOPE WINDOW_STRUCT windata;
 SCOPE WINDOW_STRUCT *windptr;

 SCOPE DISPLAY_STRUCT displayinfo;
 SCOPE DISPLAY_STRUCT *displayptr;

 SCOPE DISK_STRUCT  disk;
 SCOPE DISK_STRUCT  *diskptr;

 SCOPE DATA_INFO data_info;
 SCOPE DATA_INFO *dinfoptr;

 SCOPE EDIT_DISPLAY edit_display_struct;
 SCOPE EDIT_DISPLAY *edit_displayptr;
 
#endif
