/* created using cproto */
/* Tue Jun 21 22:05:37 UTC 2011*/

#ifndef eldb_dd_hh
#define eldb_dd_hh

#include <eldh.h>
#include <dd_general_info.h>
/* eldb_dd.c */

extern void eld_thr_flds(struct eldora_unique_info *eui);
extern int eld_tweak_vel(struct eldora_unique_info *eui, int fn, short *dst, int fgg, int ok_options);
extern int eld_az_continuity(DGI_PTR dgi, struct ve_que_val *vqv, short *new_ray, short *zz, int bad, short *prev, int fold_shear, int scaled_nyqi, int qsize, int a_fold);  //Jul 26, 2011 *new issue
extern int eld_denotch2(DGI_PTR dgi, short *aa, short *zz, int bad, int mbc, int notch_max, int small_shear, int notch_shear, int scaled_nyqv, int qsize);
extern int eld_denotch(DGI_PTR dgi, short *ss, short *zz, int bad, int mbc, int notch_max, int small_shear, int notch_shear, int scaled_nyqv, int dir);
extern short *non_shear_segment(short *aa, short *zz, int shear, int num, int bad, int dir, int *offs);
extern short *bad_segment(short *aa, short *zz, int num, int bad, int dir);
extern int wlee_unfolding(DGI_PTR dgi, struct ve_que_val *vqv, short *aa, short *zz, int bad, int qsize, int scaled_nyqi, int a_fold, int scaled_wind, int scaled_ac_vel);
extern int eld_unfolding(DGI_PTR dgi, struct ve_que_val *vqv, short *aa, short *zz, int bad, int qsize, int scaled_nyqi, int a_fold, int dir);
extern short *non_notch_segment(short *aa, short *zz, int shear, int num, int bad, int dir, int min_val, int *offs);
extern int eld_mark_shear(short *aa, short *zz, short *ss, int shear, int bad);
extern int cctype16(short *aa, short *zz);

#endif
