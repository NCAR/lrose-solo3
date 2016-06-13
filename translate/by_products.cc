/* 	$Id: by_products.c 2 2002-07-02 14:57:33Z oye $	 */

#ifndef lint
static char vcid[] = "$Id: by_products.c 2 2002-07-02 14:57:33Z oye $";
#endif /* lint */
#include <dorade_headers.h>
#include "by_products.hh"
#include "dd_der_flds.hh"
#include "gecho.hh"
#include "product_x.hh"
#include "dorade_uf.hh"
#include "nssl_mrd.hh"
#include "shane.hh"
#include "dorade_ncdf.hh"

/* c------------------------------------------------------------------------ */

void by_products(struct dd_general_info *dgi,time_t ztime)
{
    dd_derived_fields(dgi);
    dd_pct_stats(dgi);		/* this routine is in the same file with
				 * dd_derived_fields
				 */
    produce_uf(dgi);
    dd_gecho(dgi);
    dd_product_x(dgi);
    produce_nssl_mrd(dgi, NO);
    produce_shanes_data(dgi);
    produce_ncdf( dgi, NO );

}
/* c------------------------------------------------------------------------ */
