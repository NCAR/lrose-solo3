/* created using cproto */
/* Tue Jun 21 22:05:53 UTC 2011*/

#ifndef nssl_mrd_hh
#define nssl_mrd_hh

#include <dd_general_info.h>
#include <nssl_mrd.h>
/* nssl_mrd.c */

extern void dd_mrd_conv(void);
extern int dd_mrd_init(void);
extern void produce_nssl_mrd(struct dd_general_info *dgi, int flush);
extern void mrdp_write(struct mrd_production *mrdp, int header_only);
extern int mrd_find_param(struct dd_general_info *dgi, int find);

#endif
