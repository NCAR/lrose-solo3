/* created using cproto */
/* Tue Jun 21 22:05:29 UTC 2011*/

#ifndef dorade_share_hh
#define dorade_share_hh

#include <dd_general_info.h>
#include <dorade_share.h>
#include <dd_time.hh>
#include <stdio.h>
#include <run_sum.h>
#include <point_in_space.h>
#include <Volume.h>
#include <Ray.h>
#include <stdio.h>
/* dorade_share.c */

extern int dd_text_to_slm(const char *txt, struct solo_list_mgmt *slm);
extern int dd_strings_to_slm(const char **lines, struct solo_list_mgmt *slm, int nn);
extern int dd_absorb_strings(char *path_name, struct solo_list_mgmt *slm);
extern void solo_sort_strings(char **sptr, int ns);
extern int generic_sweepfiles(char *dir, struct solo_list_mgmt *lm, char *prefix, char *suffix, int not_this_suffix);
extern int dd_return_interpulse_periods(struct dd_general_info *dgi, int field_num, double *ipps);
extern int dd_return_frequencies(struct dd_general_info *dgi, int field_num, double *freqs);
extern int dd_alias_index_num(struct dd_general_info *dgi, char *name);
extern double angdiff(double a1, double a2);
extern double d_angdiff(double a1, double a2);
extern void cascdmp(char *b, int m, int n);
extern void chexdmp(char *b, int m, int n);
extern char *cpy_bfill_str(CHAR_PTR dst, CHAR_PTR srs, int n);
extern int dcdatime(const char *str, int n, short *yy, short *mon, short *dd, short *hh, short *mm, short *ss, short *ms);
extern char *dd_blank_fill(CHAR_PTR srs, int n, CHAR_PTR dst);
extern char *dd_chop(char *aa);
extern int dd_crack_file_name(char *type, int32_t *time_stamp, char *radar, int *version, const char *fn);
extern int dd_crack_file_name_ms(char *type, double *time_stamp, char *radar, int *version, char *fn, int *ms);
extern void dd_file_name(const char *type, const int32_t time_stamp, const char *radar, const int version, char *fn);
extern void dd_file_name_ms(const char *type, const int32_t time_stamp, const char *radar, const int version, char *fn, const int ms);
extern void dd_file_name_ms(const char *type, const int32_t time_stamp,
			    const char *radar, const int version,
			    std::string &fn, const int ms);
extern int dd_hrd16(char *buf, short *dd, int flag, int *empty_run);
extern int dd_hrd16_uncompressx(short *ss, short *dd, int flag, int *empty_run, int wmax);
extern int dd_hrd16LE_uncompressx(unsigned short *ss, unsigned short *dd, int flag, int *empty_run, int wmax);
extern int dd_nab_floats(char *str, float *vals);
extern int dd_nab_ints(char *str, int *vals);
extern int dd_rdp_uncompress8(unsigned char *buf, unsigned char *dd, int bad_val);
extern int dd_scan_mode(char *str);
extern char *dd_scan_mode_mne(int scan_mode, char *str);
extern char *dd_radar_type_ascii(int radar_type, char *str);
extern char *dd_unquote_string(char *uqs, const char *qs);
extern double dorade_time_stamp(struct volume_d *vold, struct ray_i *ryib, DD_TIME *dts);
extern char *dts_print(DD_TIME *dts);
extern double eldora_time_stamp(struct volume_d *vold, struct ray_i *ryib);
extern int fb_read(int fin, char *buf, int count);
extern int fb_write(int fout, char *buf, int count);
extern int32_t file_time_stamp(const char *fn);
extern char *get_tagged_string(const char *tag);
extern int gp_read(int fid, char *buf, int size, int io_type);
extern int gp_write(int fid, char *buf, int size, int io_type);
extern int in_sector(double ang, double ang1, double ang2);
extern char *put_tagged_string(char *tag, char *string);
extern int se_free_raqs(void);
extern struct running_avg_que *se_return_ravgs(int nvals);
extern char *slash_path(char *dst, const char *srs);
extern void slm_dump_list(struct solo_list_mgmt *slm);
extern void slm_print_list(struct solo_list_mgmt *slm);
extern void solo_add_list_entry(struct solo_list_mgmt *which, const char *entry);
extern char *solo_list_entry(struct solo_list_mgmt *which, int entry_num);
extern void solo_list_remove_dups(struct solo_list_mgmt *slm);
extern void solo_list_remove_entry(struct solo_list_mgmt *slm, int ii, int jj);
extern void solo_list_sort_file_names(struct solo_list_mgmt *slm);
extern struct solo_list_mgmt *solo_malloc_list_mgmt(int sizeof_entries);
extern struct point_in_space *solo_malloc_pisp(void);
extern char *solo_modify_list_entry(struct solo_list_mgmt *which, char *entry, int len, int entry_num);
extern void solo_reset_list_entries(struct solo_list_mgmt *which);
extern void solo_unmalloc_list_mgmt(struct solo_list_mgmt *slm);
extern void solo_sort_slm_entries(struct solo_list_mgmt *slm);
extern char *str_terminate(char *dst, const char *srs, int n);
extern int32_t time_now(void);
extern struct run_sum *init_run_sum(int length, int short_len);
extern void reset_running_sum(struct run_sum *rs);
extern double short_running_sum(struct run_sum *rs);
extern double running_sum(struct run_sum *rs, double val);
extern double avg_running_sum(struct run_sum *rs);
extern double short_avg_running_sum(struct run_sum *rs);

char *fgetsx(unsigned char *aa, int nn, FILE *stream);
char *fstring(char *line, int nn, FILE *stream);
time_t todays_date( short date_time[]);

#endif
