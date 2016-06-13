/* created using cproto */
/* Tue Jun 21 22:05:08 UTC 2011*/

#ifndef dd_crackers_hh
#define dd_crackers_hh

/* dd_crackers.c */

extern void nssl_crack_head(char *srs, char *dst, int limit);
extern void nssl_crack_head2(char *srs, char *dst, int limit);
extern void se_crack_bdhd(char *srs, char *dst, int limit);
extern void sp_crack_sxmn(char *srs, char *dst, int limit);
extern void sp_crack_slmk0(char *srs, char *dst, int limit); //, int swap_em); //Jul 26, 2011
extern void sp_crack_slmk(char *srs, char *dst, int limit);
extern void sp_crack_sctr0(char *srs, char *dst, int limit); //, int swap_em); //Jul 26, 2011
extern void sp_crack_sctr(char *srs, char *dst, int limit);
extern void sp_crack_sptl0(char *srs, char *dst, int limit); //, int swap_em); //Jul 26, 2011
extern void sp_crack_sptl(char *srs, char *dst, int limit);
extern void sp_crack_ssfi0(char *srs, char *dst, int limit); //, int swap_em); //Jul 26, 2011
extern void sp_crack_ssfi(char *srs, char *dst, int limit);
extern void sp_crack_ssfiLE(char *srs, char *dst, int limit);
extern void sp_crack_ssfiLE0(char *srs, char *dst, int limit);
extern void sp_crack_spmi0(char *srs, char *dst, int limit); //, int swap_em); //Jul 26, 2011
extern void sp_crack_spmi(char *srs, char *dst, int limit);
extern void sp_crack_spal(char *srs, char *dst, int limit);
extern void sp_crack_swvi0(char *srs, char *dst, int limit); //, int swap_em); //Jul 26, 2011
extern void sp_crack_swvi(char *srs, char *dst, int limit);
extern void ddin_crack_rktb(char *srs, char *dst, int limit);
extern void ddin_crack_sswbLE_aligned8(char *srs, char *dst, int limit);
extern void ddin_crack_sswb_aligned8(char *srs, char *dst, int limit);
extern void ddin_crack_sswbLE(char *srs, char *dst, int limit);
extern void ddin_crack_sswb(char *srs, char *dst, int limit);
extern void eld_crack_cspd(char *srs, char *dst, int limit);
extern void eld_crack_lidr(char *srs, char *dst, int limit);
extern void eld_crack_frad(char *srs, char *dst, int limit);
extern void eld_crack_ldat(char *srs, char *dst, int limit);
extern void fof_crack_hsk(char *srs, char *dst, int limit);
extern void ddin_crack_vold(char *srs, char *dst, int limit);
extern void ddin_crack_cfac(char *srs, char *dst, int limit);
extern void ddin_crack_celv(char *srs, char *dst, int limit);
extern void ddin_crack_radd(char *srs, char *dst, int limit);
extern void ddin_crack_frib(char *srs, char *dst, int limit);
extern void ddin_crack_parm(char *srs, char *dst, int limit);
extern void ddin_crack_swib(char *srs, char *dst, int limit);
extern void ddin_crack_asib(char *srs, char *dst, int limit);
extern void ddin_crack_ryib(char *srs, char *dst, int limit);
extern void ddin_crack_qdat(char *srs, char *dst, int limit);
extern void tdwr_crack_cbdh(char *srs, char *dst, int limit);
extern void hrd_crack_hrh(char *srs, char *dst, int limit);
extern void hrd_crack_hdrh(char *srs, char *dst, int limit);
extern void hrd_crack_radar(char *srs, char *dst, int limit);
extern void hrd_crack_header(char *srs, char *dst, int limit);
extern void uf_crack_loc_use_ncar(char *srs, char *dst, int limit);
extern void uf_crack_loc_use_ucla(char *srs, char *dst, int limit);
extern void uf_crack_fhed(char *srs, char *dst, int limit);
extern void uf_crack_opt(char *srs, char *dst, int limit);
extern void uf_crack_man(char *srs, char *dst, int limit);
extern void nex_crack_drdh(char *srs, char *dst, int limit);
extern void nex_crack_nmh(char *srs, char *dst, int limit);
extern void nex_crack_rda(char *srs, char *dst, int limit);
extern void piraq_crack_header(char *srs, char *dst, int limit, int sparc_alignment);
extern void piraq_crack_radar(char *srs, char *dst, int limit, int sparc_alignment);
extern void crackers(char *srs, char *dst, const int item_count, const int ndx_cnt, int *tbl, int srs_ndx, int dst_ndx, int limit);
extern void piraq_crackers(char *srs, char *dst, int item_count, int ndx_cnt, int *tbl, int srs_ndx, int dst_ndx, int limit);
extern void swack_short(char *ss, char *tt, int nn);
extern void swack2(char *ss, char *tt);
extern void swack_long(char *ss, char *tt, int nn);
extern void swack4(char *ss, char *tt);
extern void swack_double(char *ss, char *tt, int nn);

#endif
