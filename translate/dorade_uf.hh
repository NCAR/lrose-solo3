/* created using cproto */
/* Tue Jun 21 22:05:33 UTC 2011*/

#ifndef dorade_uf_hh
#define dorade_uf_hh

#include <dd_general_info.h>
#include "dd_uf.hh"

/* dorade_uf.c */
struct ufp_control_struct {
    int encountered[MAX_SENSORS];
    struct uf_production *ufp[MAX_SENSORS];
    int single_file;
    int last_radar_num;
    struct uf_out_dev *dev_list;
    char out_devs[512];
    int options;
};

struct field_name_aliases {
    struct field_name_aliases *next;
    char fname[16];
    char alias[16];
};

extern void produce_uf(struct dd_general_info *dgi);
extern int ufp_check_media(struct uf_production *ufp);
extern void ufp_init(struct dd_general_info *dgi, struct uf_production *ufp);
extern void ufp_stuff_ray(struct dd_general_info *dgi, struct uf_production *ufp);
extern void ufp_uf_stuff(struct dd_general_info *dgi, struct uf_production *ufp);
extern int ufp_write(struct uf_production *ufp, int size);

#endif
