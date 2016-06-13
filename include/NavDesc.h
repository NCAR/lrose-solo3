/*
 *	$Id$
 *
 *	Module:		 NavDesc.h
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
 * Revision 1.3  1994/04/26 21:57:28  case
 * added an #endif and moved an #ifdef OK_RPC.
 *
 * Revision 1.2  1994/04/05  15:32:01  case
 * moved an ifdef RPC and changed an else if to make HP compiler happy.
 *
 * Revision 1.1  1992/07/28  17:23:46  thor
 * Initial revision
 *
 *
 *
 * description:
 *        
 */
#ifndef INCNavDesch
#define INCNavDesch

#ifdef OK_RPC

#if defined(UNIX) && defined(sun)
#include <rpc/rpc.h>
#else
#if defined(WRS)
#include "rpc/rpc.h"
#endif /* WRS */
#endif /* UNIX */
#endif /* OK_RPC */

struct nav_descript {
    char  nav_descript_id[4];	/* Identifier = NDDS. */
    int32_t  nav_descript_len;	/* Block size in bytes. */
    short ins_flag;		/* 0 = no INS data, 1 = data recorded. */
    short gps_flag;		/* 0 = no GPS data, 1 = data recorded. */
    short minirims_flag;	/* 0 = no MiniRIMS data, 1 = data recorded. */
    short kalman_flag;		/* 0 = no kalman data, 1 = data recorded. */
};


typedef struct nav_descript nav_descript;
typedef struct nav_descript NAVDESC;

#ifdef OK_RPC
#if defined(sun) || defined(WRS)
extern bool_t xdr_nav_descript(XDR *, nav_descript *);
#endif

#endif /* OK_RPC */
#endif /* INCNavDesch */

