/*
 *	$Id$
 *
 *	Module:		 Ray.h
 *	Original Author: Richard E. K. Neitzel
 *      Copywrited by the National Center for Atmospheric Research
 *	Date:		 $Date$
 *
 * revision history
 * ----------------
 * $Log$
 * Revision 1.1  2002/07/02 14:57:34  oye
 * Initial revision
 *
 * Revision 1.2  1994/04/05 15:35:58  case
 * moved an ifdef RPC and changed an else if to make HP compiler happy.
 *
 * Revision 1.4  1992/07/28  17:33:03  thor
 * Added ray_status.
 *
 * Revision 1.3  1992/04/20  17:18:31  thor
 * Latest Eldora/Asterea revisions included.
 *
 * Revision 1.2  1991/10/15  17:56:43  thor
 * Fixed to meet latest version of tape spec.
 *
 * Revision 1.1  1991/08/30  18:39:38  thor
 * Initial revision
 *
 *
 *
 * description:
 *        
 */
#ifndef INCRayh
#define INCRayh

#ifdef OK_RPC

#if defined(UNIX) && defined(sun)
#include <rpc/rpc.h>
#else
#if defined(WRS)
#include "rpc/rpc.h"
#endif
#endif /* UNIX */

#endif /* OK_RPC */

struct ray_i {
    char  ray_info[4];		/* Identifier for a data ray info. */
				/* block (ascii characters "RYIB"). */
    int32_t ray_info_length;	/* length of a data ray info block in */
				/* bytes. */
    int32_t  sweep_num;		/* sweep number for this radar. */
    int32_t  julian_day;		/* Guess. */
    short hour;			/* Hour in hours. */
    short minute;		/* Minute in minutes. */
    short second;		/* Second in seconds. */
    short millisecond;		/* Millisecond in milliseconds. */
    float azimuth;		/* Azimuth in degrees. */
    float elevation;		/* Elevation in degrees. */
    float peak_power;		/* Last measured peak transmitted */
				/* power in kw. */
    float true_scan_rate;	/* Actual scan rate in degrees/second. */
    int32_t  ray_status;		/* 0 = normal, 1 = transition, 2 = bad. */
}; /* End of Structure */


typedef struct ray_i ray_i;
typedef struct ray_i RAY;

#ifdef OK_RPC
#if defined(sun) || defined(WRS)
bool_t xdr_ray_i(XDR *, RAY *);
#endif

#endif /* OK_RPC */

#endif /* INCRayh */

