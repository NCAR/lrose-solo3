/* 	$Id$	 */

#include <glib.h>
#include <stdio.h>

#include <dd_math.h>
#include <se_utils.hh>
#include <sii_overlays.h>
#include <gtkmm/messagedialog.h>
#include "sii_utils.hh"
#include "soloii.h"
#include "soloii.hh"

static const int MAX_DEBUG_NLINES = 777;
static const int DEBUG_NLINES_SLACK = 111;

static std::string debug_string;
static gint debug_string_nlines = 0;

/* c---------------------------------------------------------------------- */

void sii_dump_debug_stuff(void)
{
//  if (debug_string && debug_string->len > 0)
  if (debug_string != "")
  {
//    printf ("%s", debug_string->str);
    printf ("%s", debug_string.c_str());
//    g_string_truncate (debug_string, 0);
//    g_string_append (debug_string, "\n");
    debug_string = "";
    debug_string_nlines = 0;
  }
}

/* c---------------------------------------------------------------------- */

void sii_append_debug_stuff(const std::string &new_debug_info)
{
  // If the addition string is empty, don't do anything.

  if (new_debug_info == "")
    return;
  
//  gchar *aa;
//  guint mm, nn;
   
//  if (!debug_string)
//  { debug_string = g_string_new (""); }

  // See how many lines we are adding to the debug string.  Increment an extra
  // time for the newline we add later on.

  ++debug_string_nlines;
    
  for (size_t i = 0; i < new_debug_info.size(); ++i)
  {
    if (new_debug_info[i] == '\n')
      ++debug_string_nlines;
  }
    
  // Add the new debug info to the debug string

  debug_string += new_debug_info + "\n";
  
  // Make sure our debug string doesn't get too long.

  if (debug_string_nlines > MAX_DEBUG_NLINES + DEBUG_NLINES_SLACK)
  {
    while (debug_string_nlines > MAX_DEBUG_NLINES)
    {
      std::size_t newline_pos = debug_string.find('\n');
      if (newline_pos == std::string::npos)
      {
	// We should never get here, but do something reasonable if we do

	debug_string_nlines = 0;
	break;
      }
      
      debug_string = debug_string.substr(newline_pos+1);
      --debug_string_nlines;
    }	 
  }
}

/* c------------------------------------------------------------------------ */

void sii_glom_strings (const gchar **cptrs, int nn, GString *gs)
{
   g_string_truncate (gs, 0);
   for (; nn--; cptrs++) {
      g_string_append (gs, *cptrs);
      if (nn)
	{ g_string_append (gs, "\n"); }
   }
}

/* c------------------------------------------------------------------------ */

int sii_absorb_zeb_map (gchar *path_name, siiOverlay *overlay)
{
  /* routine to read in a zeb map overlay */

    int nn, count = 0;
    char *aa, str[256];
    FILE *stream;
    gboolean gnew = TRUE;
    siiPolyline *top, *next, *prev=NULL;


    if(!(stream = fopen(path_name, "r"))) {
	printf("Unable to open %s", path_name);
	return(-1);
    }
    top = next = overlay->plines;

    /* read in the new strings
     */
    for (nn = 0;; nn++)
    {
      if (!(aa = fgets(str, (int)128, stream)))
	break;

      std::vector< std::string > tokens;
      tokenize(str, tokens);
      
      if (tokens.size() < 2)
	continue;
      int jj = 0;
      int np = 0;
	
      if (gnew)
      {
	np = atoi(tokens[0].c_str())/2;
	if (!next)
	{
	  next = (siiPolyline *) g_malloc0 (sizeof (siiPolyline));
	  if (!top)
	  {
	    top = next;
	    overlay->plines = next;
	  }
	  next->lat = (gdouble *)g_malloc0 (np * sizeof (gdouble));
	  next->lon = (gdouble *)g_malloc0 (np * sizeof (gdouble));
	  next->x = (int *)g_malloc0 (np * sizeof (int));
	  next->y = (int *)g_malloc0 (np * sizeof (int));
	}
	next->urc.lat = atof(tokens[1].c_str());
	next->urc.lon = atof(tokens[3].c_str());
	next->llc.lat = atof(tokens[2].c_str());
	next->llc.lon = atof(tokens[4].c_str());

	next->num_points = np;
	if (np > overlay->max_pline_length)
	  overlay->max_pline_length = np;

	overlay->num_plines++;
	if (prev)
	  prev->next = next;
	prev = next;
	gnew = FALSE;
	jj = 0;
	continue;
      }
      for (std::size_t ii = 0; ii < tokens.size(); ii += 2, jj++, np--)
      {
	next->lat[jj] = atof(tokens[ii].c_str());
	next->lon[jj] = atof(tokens[ii+1].c_str());
	count++;
      }
      if (np == 0)
      { gnew = TRUE; next = next->next; }
    }
    fclose(stream);
    return(count);
}

/* c---------------------------------------------------------------------- */

guint sii_inc_master_seq_num ()
{
   return ++sii_seq_num;
}

/* c---------------------------------------------------------------------- */

guint sii_get_master_seq_num ()
{
   return sii_seq_num;
}

/* c---------------------------------------------------------------------- */

void sii_zap_quick_message (GtkWidget *w, gpointer data )
{
  gtk_widget_destroy(GTK_WIDGET(data));
}


/* c---------------------------------------------------------------------- */

void sii_message(const Glib::ustring &message)
{
  if (batch_mode)
    return;

  // Pop up a dialog window with the given text
  Gtk::MessageDialog dialog(*main_window, message);
  dialog.run();
}

/* c---------------------------------------------------------------------- */

int sii_str_seek ( char **sptrs, int count, const char *sought )
{
  if (!sptrs || count < 1 || !sought || strlen (sought) < 1)
    return -1;

  for (int ii = 0; ii < count; ii++)
  {
    if (sptrs[ii] && strcmp(sptrs[ii], sought) == 0)
      return ii;
  }
  return -1;
}

/* c---------------------------------------------------------------------- */

char *sii_glom_tokens ( char *str, char **sptrs, int count, char *joint )
{
   /* tokens with a NULL pointer or zero length will be omitted
    */
   int ii = 0;

   if (!str || !sptrs || count < 1 )
     { return NULL; }

   str[0] = '\0';

   for(; ii < count; ii++ ) {
      if (sptrs[ii] && strlen (sptrs[ii])) {
	 strcat (str, sptrs[ii]);
	 if (joint && strlen (joint))
	   { strcat (str, joint); }
      }
   }
   return str;
}

/* c---------------------------------------------------------------------- */

gchar *sii_generic_gslist_insert ( gpointer name, gpointer data )
{
   gen_gslist = g_slist_insert_sorted
     ( gen_gslist, name, (GCompareFunc)strcmp );

   return( FALSE );		/* to continue a tree traverse */
}

/* c---------------------------------------------------------------------- */

void sii_set_widget_frame_num ( GtkWidget *w, gpointer data )
{
   guint frame_num = GPOINTER_TO_UINT (data);
   gtk_object_set_data (GTK_OBJECT(w),
			"frame_num",
			GINT_TO_POINTER(frame_num));
}

/* c---------------------------------------------------------------------- */

gboolean sii_str_values(const gchar *line, const guint nvals,
			gfloat &f1, gfloat &f2)
{
  // Separate the line into tokens

  std::vector< std::string > tokens;
  tokenize(line, tokens, " \t\n,");

  if (tokens.size() < nvals)
    return FALSE;

  // Extract the first value

  if (sscanf(tokens[0].c_str(), "%f", &f1) != 1)
    return FALSE;

  if (nvals == 1)
    return TRUE;

  // If there is a second value, extract it

  if (sscanf(tokens[1].c_str(), "%f", &f2) != 1)
    return FALSE;

  return TRUE;
}

/* c---------------------------------------------------------------------- */

void sii_nullify_widget_cb (GtkWidget *widget, gpointer data)
{
  guint num = GPOINTER_TO_UINT(data);

  guint frame_num = num/TASK_MODULO;
  guint window_id = num % TASK_MODULO;

  frame_configs[frame_num]->toplevel_windows[window_id] = NULL;
}

/* c---------------------------------------------------------------------- */

gchar *sii_nab_line_from_text (const gchar *txt, guint position )
{
   static gchar line[256];
   gint mm, nn = position;

   if( !txt || strlen( txt ) <= position || nn < 0 )
      { return NULL; }
   
   for (; txt[nn] && txt[nn] != '\n'; nn++ ); /* find end of line */
   for (mm = nn; mm > 0 && txt[mm-1] != '\n'; mm-- ); /* find BOL */

   line[0] = '\0';
   strncpy (line, &txt[mm], nn-mm );
   line[nn-mm] = '\0';
   return line;
}

/* c---------------------------------------------------------------------- */

gboolean sii_nab_region_from_text (const gchar *txt, guint position
				   , gint *start, gint *end)
{
   gint mm, nn = position;

   if( !txt || strlen( txt ) <= position || position < 0 )
      { return FALSE; }
   
   for (; txt[nn] && txt[nn] != '\n'; nn++ ); /* find end of line */
   for (mm = nn; mm > 0 && txt[mm-1] != '\n'; mm-- ); /* find BOL */

   *start = mm; *end = nn;
   return TRUE;
}

/* c---------------------------------------------------------------------- */

SiiLinkedList *sii_ll_queue (SiiLinkedList **list, SiiLinkedList *link)
{
   if (!link)
     { return link; }

   if (!(*list))  {
      *list = link;
      link->previous = link->next = link;
   }
   else {			/* add to end of que */
      link->previous = (*list)->previous; /* (*list)->previous is of queue */
      link->next = NULL;
      (*list)->previous->next = link;
      (*list)->previous = link;
   }
   return link;
}

/* c---------------------------------------------------------------------- */

SiiLinkedList *sii_ll_dequeue (SiiLinkedList **list)
{
  SiiLinkedList *thisll;
  
  if (!(*list))  {
    return NULL;
  }
  /* remove top item  */
  thisll = *list;
  if (!(*list)->next)		/* last item in queue */
    { *list = NULL; }
  else {
    (*list)->next->previous = (*list)->previous;
    *list = (*list)->next;
  }
  thisll->previous = thisll->next = NULL;
  return thisll;
}

/* c---------------------------------------------------------------------- */

SiiLinkedList *sii_ll_push (SiiLinkedList **list, SiiLinkedList *link)
{
   if (!link)
     { return link; }

   if (!(*list))  {
      *list = link;
      link->previous = link->next = NULL;
   }
   else {
      link->previous = NULL;
      link->next = *list;
      (*list)->previous = link;
      *list = link;
   }
   return link;
}

/* c---------------------------------------------------------------------- */

SiiLinkedList *sii_ll_remove (SiiLinkedList **list, SiiLinkedList *link)
{
   if (!link)
     { return NULL; }

   if (link == *list ) {		/* top of list */
      *list = link->next;
      if (link->next)
	{ link->next->previous = NULL; }
   }
   else if (!link->next) {	/* bottom of list */
      link->previous->next = NULL;
   }
   else {
      link->previous->next = link->next;
      link->next->previous = link->previous;
   }
   return link;
}

/* c---------------------------------------------------------------------- */

SiiLinkedList *sii_ll_pop (SiiLinkedList **list )
{
   SiiLinkedList *thisll=sii_ll_remove (list, *list);
   return thisll;
}

/* c---------------------------------------------------------------------- */

SiiLinkedList *sii_init_linked_list (guint length)
{
  SiiLinkedList *thisll, *first=NULL, *previous = NULL;
  guint cn = 0;

  for( ; cn < length; cn++ ) {
    thisll = (SiiLinkedList *)g_malloc0 (sizeof (SiiLinkedList));

    if( previous ) {
      previous->next = thisll;
      thisll->previous = previous;
    }
    else {
      first = thisll;
    }
    previous = thisll;
  }
  return first;
}

/* c---------------------------------------------------------------------- */

SiiLinkedList *
sii_ll_malloc_item (void)
{
   SiiLinkedList *item = (SiiLinkedList *)g_malloc0 (sizeof (SiiLinkedList));;
   return item;
}

/* c---------------------------------------------------------------------- */

SiiLinkedList *sii_init_circ_que (guint length)
{
  SiiLinkedList *thisll, *first=NULL, *previous = NULL;

  first = sii_init_linked_list (length);
  thisll = previous = first;

  for(; thisll; previous = thisll, thisll = thisll->next ); /* loop to end */

  if (previous) {
     first->previous = previous;
     previous->next = first;
  }
  return first;
}

/* c------------------------------------------------------------------------ */

const gchar *
sii_item_delims ()
{
  return " \t\n,";
}

/* c---------------------------------------------------------------------- */

gchar *sii_set_string_from_vals (GString *gs, guint nvals,
				 gfloat f1, gfloat f2,
				 guint precision)
{
   gchar fmt[64], prec[16];

   g_string_truncate (gs, 0);
   sprintf (prec, ".%df", precision);
   strcpy (fmt, "%");
   strcat (fmt, prec);
   if (nvals > 1) {
      strcat (fmt, ", %");
      strcat (fmt, prec);
      g_string_sprintfa (gs, fmt, f1, f2);
   }
   else {
      g_string_sprintfa (gs, fmt, f1);
   }
   return gs->str;
}

/* c---------------------------------------------------------------------- */

void sii_bad_entry_message ( const gchar *ee, guint items )
{
   gchar str[256];

   str[0] = '\0';
   if (ee && strlen (ee))
     { strcat( str, ee ); }
   sprintf (str+strlen(str)
	    , "\n Entry could not be interpreted. Requires %d item(s)"
	    , items );
   sii_message (str);
}

/* c---------------------------------------------------------------------- */

GtkWidget *sii_submenu ( const gchar *name, GtkWidget *mbar )
{
   GtkWidget *menuitem, *submenu;
   menuitem = gtk_menu_item_new_with_label (name);
   gtk_menu_bar_append (GTK_MENU_BAR (mbar), menuitem );
   submenu = gtk_menu_new ();
   gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), submenu);
   return submenu;
}

/* c---------------------------------------------------------------------- */

GtkWidget *
sii_submenu_item ( const gchar *name, GtkWidget *submenu, guint widget_id
		  , GtkSignalFunc sigfun, guint frame_num )
{
   GtkWidget *next_item;
   guint nn = frame_num * TASK_MODULO + widget_id;

   if (!name) {
      next_item = gtk_menu_item_new ();	/* seperater */
   }
   else {
      next_item = gtk_menu_item_new_with_label (name);
      gtk_signal_connect (GTK_OBJECT(next_item)
			  ,"activate"
			  , (GtkSignalFunc) sigfun
			  , GINT_TO_POINTER(nn));
      gtk_object_set_data (GTK_OBJECT(next_item)
			   , "widget_id", GINT_TO_POINTER(widget_id));
      gtk_object_set_data (GTK_OBJECT(next_item)
			   , "frame_num", GINT_TO_POINTER(frame_num));
   }
   gtk_menu_append (GTK_MENU (submenu), next_item );

   return next_item;
}
/* c---------------------------------------------------------------------- */

GtkWidget *
sii_toggle_submenu_item (const gchar *name, GtkWidget *submenu, guint widget_id
			 , GtkSignalFunc sigfun, guint frame_num
			 , guint radio_item, GSList **radio_group_ptr )
{
   /* be sure and set radio_group = NULL;
    * before creating the first radio button
    */
   GtkWidget *next_item;
   GSList *radio_group;
   guint nn = frame_num * TASK_MODULO + widget_id;

   
   if (radio_item) {
      if (radio_item == 1 )
	{ *radio_group_ptr = NULL; }

      radio_group = *radio_group_ptr;
      next_item = gtk_radio_menu_item_new_with_label (radio_group, name);
      radio_group = gtk_radio_menu_item_group
	(GTK_RADIO_MENU_ITEM (next_item));
      *radio_group_ptr = radio_group;
   }
   else {
      next_item = gtk_check_menu_item_new_with_label (name);
   }
   gtk_signal_connect (GTK_OBJECT(next_item)
		       ,"toggled"
		       , (GtkSignalFunc) sigfun
		       , GINT_TO_POINTER(nn));
  gtk_object_set_data (GTK_OBJECT(next_item)
		       , "widget_id", GINT_TO_POINTER(widget_id));
  gtk_object_set_data (GTK_OBJECT(next_item)
		       , "frame_num", GINT_TO_POINTER(frame_num));

   gtk_menu_append (GTK_MENU (submenu), next_item );
   return next_item;
}

/* c---------------------------------------------------------------------- */

int loop_xy2ll(double *plat, double *plon, double *palt,
	       double *x, double *y, double *z,
	       double olat, double olon, double oalt,
	       double R_earth, int num_pts)
{
    /* calculate (plat,plon) of a point at (x,y) relative to (olat,olon) */
    /* all dimensions in km. */

    /* transform to earth coordinates and then to lat/lon/alt */

    /* These calculations are from the book
     * "Aerospace Coordinate Systems and Transformations"
     * by G. Minkler/J. Minkler
     * these are the ECEF/ENU point transformations
     */

    int nn = num_pts;
    double delta_o, lambda_o, delta_p, lambda_p;
    double xe, ye, ze, sinLambda, cosLambda, sinDelta, cosDelta;
    double h;


    h = R_earth + oalt;
    delta_o = RADIANS( olat );	/* read delta sub oh */
    lambda_o = RADIANS( olon );

    sinLambda = sin( lambda_o );
    cosLambda = cos( lambda_o );
    sinDelta = sin( delta_o );
    cosDelta = cos( delta_o );
    /*
    printf( "\n" );
     */
    
    for(; nn--; plat++, plon++, palt++, x++, y++, z++ ) {
    	
	/* transform to earth coordinates */

	xe = h * sinDelta + cosDelta * (*y) + sinDelta * (*z);

	ye = -h * cosDelta * sinLambda   -cosLambda * (*x)
	  + sinLambda * sinDelta * (*y) -sinLambda * cosDelta * (*z);

	ze = h * cosDelta * cosLambda   -sinLambda * (*x)
	  -cosLambda * sinDelta * (*y) + cosLambda * cosDelta * (*z);

	lambda_p = atan2( -ye, ze );
	delta_p = atan2( xe, sqrt( ye * ye + ze * ze ));

	*plat = DEGREES( delta_p );
	*plon = DEGREES( lambda_p );
	*palt = sqrt( xe * xe + ye * ye + ze * ze ) - R_earth;
	/*
	printf( "%f %f %f      %f %f %f\n", *plat, *plon, *palt, *x, *y, *z );
	 */
    }	
    return num_pts;
}

/* c---------------------------------------------------------------------- */


