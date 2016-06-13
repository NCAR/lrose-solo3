/* created using cproto */
/* Tue Jun 21 22:05:15 UTC 2011*/

#ifndef dd_io_mgmt_hh
#define dd_io_mgmt_hh

#include <dd_time.h>
#include <generic_radar_info.h>
#include "input_limits.hh"
#include "dd_io_structs.h"
#include <radar_angles.h>
#include <dorade_share.h>

/* dd_io_mgmt.c */

struct source_devs_mgmt {
    struct source_devs_mgmt *last;
    struct source_devs_mgmt *next;
    int dev_count;
    char *dev_names;
    char **dev_ptrs;
    char *id;
};


extern int cdcode(char *s, int n, int *ival, float *rval);
extern void ctype16(short *a, int m, int n);
extern void xctypeu16(unsigned short *a, int m, int n);
extern void ctypeu16(unsigned char *a, int m, int n);
extern void slm_ctypeu16(struct solo_list_mgmt *slm, unsigned char *a, int m, int n);
extern void slm_xctypeu16(struct solo_list_mgmt *slm, unsigned char *a, int m, int n);
extern void Xctypeu16(unsigned char *a, int m, int n);
extern int dd_crack_datime(const char *datime, const int nc, DD_TIME *dts);
extern char *dd_est_src_dev_names_list(char *id, struct solo_list_mgmt *slm, char *src_dir);
extern char *dd_establish_source_dev_names(char *id, char *names);
extern void dd_gen_packet_info(struct input_read_info *iri, struct io_packet *dp);
extern void dd_input_read_close(struct input_read_info *iri);
extern int dd_input_read_open(struct input_read_info *iri, char *dev_name);
extern void dd_io_reset_offset(struct input_read_info *iri, int32_t offset);
extern int dd_logical_read(struct input_read_info *iri, int direction);
extern char *dd_next_source_dev_name(char *id);
extern int dd_phys_read(struct input_read_info *iri);
extern int dd_really_skip_recs(struct input_read_info *iri, int direction, int skip_count);
extern double dd_relative_time(const char *rtm);
extern void dd_reset_ios(struct input_read_info *iri, int level);
extern struct io_packet *dd_return_current_packet(struct input_read_info *iri);
extern struct io_packet *dd_return_next_packet(struct input_read_info *iri);
extern void dd_rewind_dev(struct input_read_info *iri);
extern void dd_skip_files(struct input_read_info *iri, int direction, int skip_count);
extern int dd_skip_recs(struct input_read_info *iri, int direction, int skip_count);
extern void dd_unwind_dev(struct input_read_info *iri);
extern void ezascdmp(char *b, int m, int n);
extern void slm_ezascdmp(struct solo_list_mgmt *slm, char *b, int m, int n);
extern void ezhxdmp(char *b, int m, int n);
extern void slm_ezhxdmp(struct solo_list_mgmt *slm, char *b, int m, int n);
extern int getreply(char *s, int lim);
extern int oketype(char s[], int n, float *val);
extern int okint(char s[], int n, int *ival);
extern int okreal(char s[], int n, float *val);
extern int strnlen(char *str, int n);
extern void gri_interest(struct generic_radar_info *gri, int verbose, char *preamble, char *postamble);
extern int gri_max_lines(void);
extern int gri_nab_input_filters(double current_time, struct dd_input_filters *difs, int less);
extern void gri_print_list(char **ptrs);
extern void gri_set_max_lines(int nlines);
extern int gri_start_stop_chars(int *view_char, int *view_char_count);
struct input_read_info *dd_return_ios(int index, int fmt);

int ffb_skip_one_bkw (FILE *strm);
int ffb_skip_one_fwd (FILE *strm);
int ffb_read(FILE *fin, char *buf, int count );
void QuickSleep (int ms);
void Alarm (int v);
struct input_read_info * dd_init_io_structs(int index, int fmt);


#endif
