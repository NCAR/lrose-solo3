/* created using cproto */
/* Tue Jun 21 22:05:10 UTC 2011*/

#ifndef dd_examine_hh
#define dd_examine_hh

#include <dd_io_structs.h>
#include <dd_general_info.h>
#include <NavDesc.h>
#include <InSitu.h>
#include <InSituData.h>
#include <TimeSeries.h>
#include <Comment.h>
#include <CellVector.h>
#include <CellSpacing.h>
#include <CellSpacingFP.h>
#include <Correction.h>
#include <FieldParam.h>
#include <FieldRadar.h>
#include <Parameter.h>
#include <RadarDesc.h>
#include <Ray.h>
#include <super_SWIB.h>
#include <Sweep.h>
#include <Volume.h>
#include <Waveform.h>

/* dd_examine.c */

extern void solo_message(const char *message);
extern int main(int argc, char *argv[]);
extern int ddx_options(void);
extern void ddx_inventory(struct input_read_info *iri);  //Jul 26, 2011 change return type from int to void
extern int ddx_dorade_bkwd_desc_loop(char *aa, int ii, int jj, int32_t offset, int32_t sizeof_file);
extern int ddx_dorade_fwd_desc_loop(char *aa, int ii, int jj, int32_t offset, int32_t sizeof_file);
extern int ddx_start_stop_chars(void);
extern int ddx_desc_fwd_sync(char *aa, int ii, int jj);
extern int ddx_desc_bkwd_sync(char *aa, int ii, int jj);
extern char *dd_print_seds(struct generic_descriptor *gd, char *str);
extern int ddx_print_desc(char *a);
extern char *dd_print_gnrc(struct generic_descriptor *gd, char *str);
extern char *dd_print_ndds(struct nav_descript *ndds, char *str);
extern char *dd_print_situ(struct insitu_descript *situ, char *str);
extern char *dd_print_isit(struct insitu_data *isit, char *str);
extern char *dd_print_time(struct time_series *times, char *str);
extern char *dd_print_rktb(struct rot_ang_table *rktb, char *str);
extern char *dd_print_comm(struct comment_d *comm, char *str);
extern char *dd_print_asib(struct platform_i *asib, char *str);
extern char *dd_print_celv(struct cell_d *celv, char *str);
extern char *dd_print_cfac(struct correction_d *cfac, char *str);
extern char *dd_print_cspd(struct cell_spacing_d *cspd, char *str);
extern char *dd_print_csfd(struct cell_spacing_fp_d *csfd, char *str);
extern char *dd_print_frad(struct field_parameter_data *frad, char *str);
extern char *dd_print_frib(char *a, char *str);
extern char *dd_print_frib_v1(struct field_radar_i *frib, char *str);
extern char *dd_print_frib_v0(struct field_radar_i_v0 *frib, char *str);
extern char *dd_print_parm(struct parameter_d *parm, char *str);
extern char *dd_print_radd(struct radar_d *radd, char *str);
extern char *dd_print_rdat(struct paramdata_d *rdat, char *str);
extern char *dd_print_ryib(struct ray_i *ryib, char *str);
extern char *dd_print_sswb(struct super_SWIB *sswb, char *str);
extern char *dd_print_swib(struct sweepinfo_d *swib, char *str);
extern char *dd_print_vold(struct volume_d *vold, char *str);
extern char *dd_print_wave(struct waveform_d *wave, char *str);
extern int ddx_bad_vibs(char *a);

#endif
