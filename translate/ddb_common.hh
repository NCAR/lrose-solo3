/* created using cproto */
/* Tue Jun 21 22:05:06 UTC 2011*/

#ifndef ddb_common_hh
#define ddb_common_hh

#include "input_limits.hh"
#include <radar_angles.h>
#include <dd_general_info.h>
#include <point_in_space.h>
#include <dorade_share.h>
#include "dorade_share.hh"
#include <Platform.h>
#include <CellVector.h>
#include <Correction.h>
#include <stdio.h>
#include <Sweep.h>
#include <Parameter.h>
#include <RadarDesc.h>
#include <FieldRadar.h>
#include <super_SWIB.h>
#include <Volume.h>
#include <Ray.h>

extern int dd_get_scan_mode(const char *mne);
extern struct cfac_wrap *dd_absorb_cfacs(const char *list,
					 const char *radar_name);
extern void dd_absorb_cfac(const char *list, const char *radar_name,
			   struct correction_d *cfac);
extern char *dd_align_structs(char *c, int n);
extern double dd_altitude(struct dd_general_info *dgi);
extern double dd_altitude_agl(struct dd_general_info *dgi);
extern void dd_alloc_data_field(DGI_PTR dgi, int pn);
extern double dd_azimuth_angle(struct dd_general_info *dgi);
extern int dd_cell_num(struct dds_structs *dds, double range);
extern void dd_clear_pisp(struct point_in_space *pisp);
extern int dd_clip_gate(DGI_PTR dgi, double elev, double alt, double lower, double upper);
extern int dd_compress(unsigned short *src, unsigned short *dst, int flag, int n);
extern void dd_copy_pisp(struct point_in_space *p0, struct point_in_space *p1);
extern int dd_datum_size(int binary_format);
extern char *dd_delimit(char *c);
extern double dd_drift(struct dd_general_info *dgi);
extern double dd_earthr(double lat);
extern double dd_elevation_angle(struct dd_general_info *dgi);
extern char *dd_find_desc(char *a, char *b, char *desc);
extern int dd_find_field(struct dd_general_info *dgi, const char *name);
extern void dd_get_difs(struct dd_input_filters *difs);
extern void dd_get_lims(char *str_ptrs[], int ii, int nt, struct d_limits **dlims);
extern int dd_givfld(DGI_PTR dgi, int ndx, int g1, int n, float *dd, float *badval);
extern double dd_heading(struct dd_general_info *dgi);
extern struct dd_general_info *dd_ini(const int rn, const char *radar_name);
extern void dd_input_strings(void);
extern void dgi_interest_really(struct dd_general_info *dgi, int verbosity, char *preamble, char *postamble, struct sweepinfo_d *swib);
extern int dd_itsa_physical_dev(char *s);
extern double dd_latitude(struct dd_general_info *dgi);
extern void dd_latlon_relative(struct point_in_space *p0, struct point_in_space *p1);
extern int loop_ll2xy_v3(double *plat, double *plon, double *palt, double *x, double *y, double *z, double olat, double olon, double oalt, double R_earth, int num_pts);
extern void dd_latlon_shift(struct point_in_space *p0, struct point_in_space *p1);
extern int loop_xy2ll_v3(double *plat, double *plon, double *palt, double *x, double *y, double *z, double olat, double olon, double oalt, double R_earth, int num_pts);
extern char *dd_link_dev_name(const char *radar_name,
			      const char *dev_links, char *str);
extern double dd_longitude(struct dd_general_info *dgi);
extern struct radar_d *dd_malloc_radd(DGI_PTR dgi, char *config_name);
extern double dd_nav_rotation_angle(struct dd_general_info *dgi);
extern double dd_nav_tilt_angle(struct dd_general_info *dgi);
extern int dd_ndx_name(DGI_PTR dgi, char *name2);
extern double dd_pitch(struct dd_general_info *dgi);
extern void dor_print_asib(struct platform_i *asib, struct solo_list_mgmt *slm);
extern void dor_print_celv(struct cell_d *celv, struct solo_list_mgmt *slm);
extern void dor_print_cfac(struct correction_d *cfac, struct solo_list_mgmt *slm);
extern void dor_print_parm(struct parameter_d *parm, struct solo_list_mgmt *slm);
extern void dor_print_frib(struct field_radar_i *frib, struct solo_list_mgmt *slm);
extern void dor_print_radd(struct radar_d *radd, struct solo_list_mgmt *slm);
extern void dor_print_rktb(struct rot_ang_table *rktb, struct solo_list_mgmt *slm);
extern void dor_print_ryib(struct ray_i *ryib, struct solo_list_mgmt *slm);
extern void dor_print_swib(struct sweepinfo_d *swib, struct solo_list_mgmt *slm);
extern void dor_print_sswb(struct super_SWIB *sswb, struct solo_list_mgmt *slm);
extern void dor_print_vold(struct volume_d *vold, struct solo_list_mgmt *slm);
extern void dd_radar_angles(struct platform_i *asib, struct correction_d *cfac, struct radar_angles *ra, struct dd_general_info *dgi);
extern char *dd_radar_name(struct dds_structs *dds);
extern char *dd_radar_namec(char *name0);
extern void dd_radar_selected(char *radar_name, int radar_num, struct dd_input_filters *difs);
extern void dd_range_gate(DGI_PTR dgi, const float req_range, int *gate, float *val);
extern void dd_razel_to_xyz(struct point_in_space *p0);
extern void dd_reset_d_limits(struct d_limits **at);
extern int dd_return_id_num(char *name);
extern struct rot_table_entry *dd_return_rotang1(struct rot_ang_table *rat);
extern double dd_roll(struct dd_general_info *dgi);
extern int dd_rotang_seek(struct rot_ang_table *rat, double rotang);
extern void dd_rotang_table(DGI_PTR dgi, int func);
extern double dd_rotation_angle(struct dd_general_info *dgi);
extern void dd_rng_info(DGI_PTR dgi, int *g1, int *m, float *rngs, int *ng);
extern void dd_set_uniform_cells(struct dds_structs *dds);
extern void dd_site_name(char *proj_name, char *site_name);
extern int dd_src_fid(int *io_type);
extern double dd_tilt_angle(struct dd_general_info *dgi);
extern int dd_tokens(char *att, char *str_ptrs[]);
extern int dd_tokenz(char *att, char *str_ptrs[], const char *dlims);
extern void dd_uniform_cell_spacing(float src_cells[], int num_cells, double cell_spacing, int *cell_lut, double r0, int nuc);
extern char *dd_whiteout(char *c);
extern int dd_xy_plane_horizontal(struct dd_general_info *dgi);
extern int fp_bin_search(float *f, int nf, double val);
extern int dgi_buf_rewind(DGI_PTR dgi);
extern int HRD_recompress(unsigned short *src, unsigned short *dst, int flag, int n, int compression);
extern int RDP_compress_bytes(unsigned char *src, unsigned char *dst, int flag, int n, int compression);
extern short swap2(char *ov);
extern int32_t swap4(char *ov);
extern int32_t xswap4(char *v_lower, char *v_upper);
void dd_get_cfac(FILE *stream, struct correction_d *cfac);
#endif
