/* created using cproto */
/* Tue Jun 21 22:05:03 UTC 2011*/

#ifndef dap_common_hh
#define dap_common_hh

#include <dd_general_info.h>

/* dap_common.c */


extern int process_cappi_beam(void);
extern void solo_message(const char *message);
extern struct dd_general_info *dap_return_dgi(void);
extern void dap_givfld_(int *ndx, int *g1, int *n, float *dd, float *badval);
extern int dap_mnt_f_(char *input_file_name, char *radar_name, int32_t *start_time, int32_t *stop_time, int *ivol, int *iscan, int *irec);
extern int dap_mnt_s_(char *dir, char *radar_name, int32_t *start_time, int32_t *stop_time, int *ivol, int *iscan, int *irec);
extern int dap_ndx_name_(char *name2);
extern int dap_next_ray_(int *lun, char *buf);
extern int dap_parm_present_(char *name2);
extern void dap_range_gate_(float *req_range, int *gate, float *val);
extern int dap_read(int fid, char *buf, int size);
extern void dap_rng_info_(int *g1, int *m, float *rngs, int *ng);
extern void dap_setdpr_(int *val);
extern int dap_next_ray_loop(void);
extern DGI_PTR dap_another_ray(void);
extern void kdate_(char *buf, char *name8);
extern int kday_(void);
extern void kfld1_(char *buf, char *name2);
extern void kfldtn_(char *buf, char *name8);
extern int khour_(void);
extern int klrect_(void);
extern int kminit_(void);
extern int kminut_(void);
extern int kmonth_(void);
extern int kprnv_(void);
extern void kproj_(char *buf, char *name8);
extern void kradar_(char *buf, char *name8);
extern int ksecnd_(void);
extern void ksite_(char *buf, char *name8);
extern int kswepm_(void);
extern int kswepn_(void);
extern void ktapen_(char *buf, char *name8);
extern void ktime_(char *buf, char *name10);
extern void ktimez_(int *yy, int *mo, int *dd, int *hh, int *mm, int *ss, int *ms);
extern void ktzone_(char *buf, char *name2);
extern int kvoln_(void);
extern int kyear_(void);
extern void cuazim_(float *x);
extern void cudmrc_(float *x);
extern void cudmprf_(char *name2, float *x);
extern void cuelev_(float *x);
extern void cufixed_(float *x);
extern void cugealt_(float *x);
extern void cuhbwid_(float *x);
extern void cuhight_(float *x);
extern void culatit_(float *x);
extern void culongt_(float *x);
extern void cuvenyq_(float *x);
extern void curotat_(float *x);
extern void time2unix_(int time[6], int32_t *unixtime);
extern int dap_ndx_name__(char *name2);
extern void dap_rng_info__(int *g1, int *m, float *rngs, int *ng);
extern void dap_givfld__(int *ndx, int *g1, int *n, float *dd, float *badval);
extern void dap_range_gate__(float *req_range, int *gate, float *val);
extern int dap_parm_present__(char *name2);
extern int dap_next_ray__(int *lun, char *buf);
extern int dap_mnt_s__(char *dir, char *radar_name, int32_t *start_time, int32_t *stop_time, int *ivol, int *iscan, int *irec);
extern int dap_mnt_f__(char *input_file_name, char *radar_name, int32_t *start_time, int32_t *stop_time, int *ivol, int *iscan, int *irec);
extern void dap_setdpr__(int *val);

#endif
