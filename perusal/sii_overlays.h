/* 	$Id$	 */

# ifndef SII_OVERLAYS_H
# define SII_OVERLAYS_H

#include <soloii.h>

/* c---------------------------------------------------------------------- */

typedef struct {
  gdouble lat;
  gdouble lon;
  gfloat  alt;
} siiLatLonAlt;

/* c---------------------------------------------------------------------- */

struct sii_polyline {
				/* rectangle containing the polyline */
  siiLatLonAlt llc;		/* lower left corner */
  siiLatLonAlt urc;		/* upper right corner */

  gdouble *lat;			
  gdouble *lon;

  int *x;			/* local pixel addresses */
  int *y;			/* may require blowup factor */

  int num_points;
  int max_points;
  struct sii_polyline *next;
};

typedef struct sii_polyline siiPolyline;

/* c---------------------------------------------------------------------- */

struct sii_overlay_symbols {
  siiLatLonAlt location;
  gint type;
  gchar label[16];
  gfloat azimuth_of_label;
  gchar rgb[4];
};

/* How to save and retrieve overlay information. XML? */

/* c---------------------------------------------------------------------- */

struct sii_overlay {

  siiLatLonAlt *ulc;		/* upper left corner */
  gfloat zoom;

  GString *name;
  GString *state;

  gint type;

  gint num_plines;
  gint max_plines;
  gint max_pline_length;
  
  gint num_symbols;
  gint max_symbols;

  struct sii_polyline *plines; 
  struct sii_overlay_symbols *symbols;
};

typedef struct sii_overlay siiOverlay;

/* c---------------------------------------------------------------------- */
/* c---------------------------------------------------------------------- */


# endif /* SII_OVERLAYS_H */
