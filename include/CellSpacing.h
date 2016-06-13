/*
 *	$Id$
 *
 *	Module:		 CellSpacing.h
 *	Original Author: Richard E. K. Neitzel
 *      Copywrited by the National Center for Atmospheric Research
 *	Date:		 $Date$
 *
 * revision history
 * ----------------
 * $Log$
 * Revision 1.1  2002/07/02 14:57:33  oye
 * Initial revision
 *
 * Revision 1.2  1994/04/05 15:22:38  case
 * moved an ifdef RPC and changed else if to else and if on another line
 * >> to keep the HP compiler happy.
 * >> .
 *
 * Revision 1.3  1991/10/15  17:54:21  thor
 * Fixed to meet latest version of tape spec.
 *
 * Revision 1.2  1991/10/11  15:32:10  thor
 * Added variable for offset to first gate.
 *
 * Revision 1.1  1991/08/30  18:39:19  thor
 * Initial revision
 *
 *
 *
 * description:
 *        
 */
#ifndef INCCellSpacingh
#define INCCellSpacingh

#ifdef OK_RPC

#if defined(UNIX) && defined(sun)
#include <rpc/rpc.h>
#else 
# if defined(WRS)
#   include "rpc/rpc.h"
# endif
#endif

#endif

struct cell_spacing_d {
    char   cell_spacing_des[4]; /* Identifier for a cell spacing descriptor */
				/* (ascii characters CSPD). */
    int32_t   cell_spacing_des_len; /* Cell Spacing descriptor length in bytes. */
    short  num_segments;	/* Number of segments that contain cells of */
				/* equal widths. */
    short  distToFirst;		/* Distance to first gate in meters. */
    short  spacing[6];		/* Width of cells in each segment in m. */
    short  num_cells[6];	/* Number of cells in each segment. */
};				/* End of Structure */


typedef struct cell_spacing_d cell_spacing_d;
typedef struct cell_spacing_d CELLSPACING;

#ifdef OK_RPC
#if defined(sun) || defined(WRS)
bool_t xdr_cell_spacing_d(XDR *, cell_spacing_d *);
#endif

#endif

#endif

