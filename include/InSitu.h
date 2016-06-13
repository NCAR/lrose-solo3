/*
 *	$Id$
 *
 *	Module:		 InSitu.h
 *	Original Author: Richard E. Neitzel
 *      Copywrited by the National Center for Atmospheric Research
 *	Date:		 $Date$
 *
 * revision history
 * ----------------
 * $Log$
 * Revision 1.1  2002/07/02 14:57:33  oye
 * Initial revision
 *
 * Revision 1.3  1994/04/26 22:03:42  case
 * added an #endif and moved the OK_RPC #ifdef.
 *
 * Revision 1.2  1994/04/05  15:27:29  case
 * moved an ifdef RPC and changed an else if to make HP compiler happy.
 *
 * Revision 1.1  1992/07/28  17:25:10  thor
 * Initial revision
 *
 *
 *
 * description:
 *        
 */
#ifndef INCInSituh
#define INCInSituh

#ifdef OK_RPC

#if defined(UNIX) && defined(sun)
#include <rpc/rpc.h>
#else
# if defined(WRS)
#include "rpc/rpc.h"
# endif /* WRS */

#endif /* UNIX */

#endif /* OK_RPC */

struct insitu_parameter {
    char  name[8];
    char  units[8];
};

struct insitu_descript {
    char  insitu_descript_id[4]; /* Identifier = SITU. */
    int32_t  insitu_descript_len;	/* Block size in bytes. */
    int32_t  number_params;	/* Number of paramters. */
    struct insitu_parameter params[256]; /* Is this enough? */
};


typedef struct insitu_descript insitu_descript;
typedef struct insitu_descript INSITUDESC;

typedef struct insitu_parameter insitu_parameter;
typedef struct insitu_parameter INSITU_PARAMETER;

#ifdef OK_RPC

#if defined(sun) || defined(WRS)
extern bool_t xdr_insitu_descript(XDR *, insitu_descript *);
extern bool_t xdr_insitu_parameter(XDR *, insitu_parameter *);
#endif

#endif /* OK_RPC */
#endif /* INCInSituh */

