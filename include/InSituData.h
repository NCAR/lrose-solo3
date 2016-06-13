/*
 *	$Id$
 *
 *	Module:		 InsituData.h
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
 * Revision 1.3  1994/04/26 21:54:14  case
 * added an #endif.
 *
 * Revision 1.2  1994/04/05  15:29:49  case
 * moved an ifdef RPC and changed an else if to make HP compiler happy.
 *
 * Revision 1.2  1992/08/11  13:47:18  thor
 * Fixed typo.
 *
 * Revision 1.1  1992/07/28  17:25:10  thor
 * Initial revision
 *
 *
 *
 * description:
 *        
 */
#ifndef INCInsituDatah
#define INCInsituDatah

#ifdef OK_RPC

#if defined(UNIX) && defined(sun)
#include <rpc/rpc.h>
#else 
#if defined(WRS)
#include "rpc/rpc.h"
#endif /* UNIX */
#endif /* if defined(UNIX) */
#endif /* OK_RPC */

struct insitu_data {
    char  insitu_data_id[4];	/* Identifier = ISIT. */
    int32_t  insitu_data_len;	/* Block size in bytes. */
    short julian_day;
    short hours;
    short minutes;
    short seconds;
};


typedef struct insitu_data insitu_data;
typedef struct insitu_data INSITU_DATA;

#ifdef OK_RPC
#if defined(sun) || defined(WRS)
extern bool_t xdr_insitu_data(XDR *, insitu_data *);
#endif

#endif /* OK_RPC */
#endif /* INCInsituDatah */

