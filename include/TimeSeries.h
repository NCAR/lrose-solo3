/*
 *	$Id$
 *
 *	Module:		 TimeSeries.h
 *	Original Author: Richard E. Neitzel
 *      Copywrited by the National Center for Atmospheric Research
 *	Date:		 $Date$
 *
 * revision history
 * ----------------
 * $Log$
 * Revision 1.1  2002/07/02 14:57:34  oye
 * Initial revision
 *
 * Revision 1.2  1994/04/26 21:53:37  case
 * added an #endif and moved an OK_RPC define.
 *
 * Revision 1.1  1994/04/26  21:51:37  case
 * Initial revision
 *
 * Revision 1.1  1992/07/28  17:23:46  thor
 * Initial revision
 *
 *
 *
 * description:
 *        
 */
#ifndef INCTimeSeriesh
#define INCTimeSeriesh

#ifdef OK_RPC

#if defined(UNIX) && defined(sun)
#include <rpc/rpc.h>
#else 
#if defined(WRS)
#include "rpc/rpc.h"
#endif /* WRS */
#endif /* UNIX */
#endif /* OK_RPC */

struct time_series {
    char  time_series_id[4];	/* Identifier = TIME. */
    int32_t  time_series_len;	/* Block size in bytes. */
};


typedef struct time_series time_series;
typedef struct time_series TIME_SERIES;

#ifdef OK_RPC
#if defined(sun) || defined(WRS)
extern bool_t xdr_time_series(XDR *, time_series *);
#endif

#endif /* OK_RPC */
#endif /* INCTimeSeriesh */

