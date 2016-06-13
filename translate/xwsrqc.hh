/* created using cproto */
/* Tue Jun 21 22:06:12 UTC 2011*/

#ifndef xwsrqc_hh
#define xwsrqc_hh

#include <generic_radar_info.h>
#include <stdio.h>
/* xwsrqc.c */

struct wsrqc_stuff {
    double start_time;
    double stop_time;
    float lat;
    float lon;
    float h_beamwidth;
    float v_beamwidth;
    char site[16];
    char radar_name[16];
    float dbzoff_048;		/* for a pulse width of .48 microseconds */
    float dbzoff_182;		/* for a pulse width of 1.82 microseconds */
    float dbzoff_080;		/* for a pulse width of .80 microseconds */
    float dboff;
    float radconst_048;
    float radconst_182;
    float radconst_080;
    float attenuation;
    float log_slope;
    float log_intercept;
};

struct wsrqc_stuff_ptrs {
    struct wsrqc_stuff_ptrs *next;
    struct wsrqc_stuff_ptrs *last;
    struct wsrqc_stuff *at;
    int32_t file_position;
    int last_qc_set;
};

//extern int main(void);
extern int wsrqc_correct(struct generic_radar_info *gri, char *name);
extern int wsrqc_init(struct generic_radar_info *gri);
extern struct wsrqc_stuff_ptrs *wsr_malloc(void);
extern int wsrqc_power_cor(struct generic_radar_info *gri, char *name);

int nab_wsrqc_stuff(FILE *stream, char *buf, struct wsrqc_stuff_ptrs *wsp, char *quit_at, int num_occurs);

#endif
