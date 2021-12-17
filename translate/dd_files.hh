/* created using cproto */
/* Tue Jun 21 22:05:12 UTC 2011*/

#ifndef dd_files_hh
#define dd_files_hh

# define MAX_DIRECTORIES 16
# define MAX_FILE_TYPES 16
# define MAX_EXTEND 222
# define LT -1
# define EQ  0
# define GT  1

#include <dorade_share.h>
#include <dd_defines.h>
#include <dd_files.h>

/* dd_files.c */
struct dd_radar_name_info_v3 {
    int num_sweeps;
    int num_ddfnp;
    int prev_req_type;
    struct dd_file_name_v3 **first_ddfnp;

    struct dd_file_name_v3 *top_ddfn;
    struct dd_file_name_v3 *h_node;
    struct dd_file_name_v3 *z_node;
    struct dd_file_name_v3 *prev_ddfn;
};

struct ddir_info_v3 {
    int32_t rescan_time;
    int auto_rescan_hit_max;
    int dir_num;
    int no_auto_scan;
    int num_hits;
    int num_radars;
    int rescan_urgent;
    struct dd_radar_name_info_v3 *rni[MAX_SENSORS];
    char *radar_name[MAX_SENSORS];
    char directory[128];
};

struct dd_radar_name_info {
    int num_vols;
    int num_swps;
    struct dd_file_name **swp_list;
    struct dd_file_name **vol_list;
    char *radar_name;
};

struct ddir_info {
    int dir_num;
    int max_entries;
    int extend;
    int num_radars;
    int num_swps;
    int num_vols;

    struct dd_file_name *ddfn;
    struct dd_file_name **ddfnp;
    struct dd_radar_name_info *rni[MAX_SENSORS];
    char directory_name[128];
};

extern int craack_ddfn(char *fn, struct dd_file_name_v3 *ddfn);
extern char *ddfn_file_info_line(struct dd_file_name_v3 *ddfn, char *str);
extern void ddfn_file_name(struct dd_file_name_v3 *ddfn, char *name);
//extern void ddfn_list_versions(struct dd_file_name_v3 *this);
extern void ddfn_list_versions(struct dd_file_name_v3 *ddfn);
extern struct dd_file_name_v3 *ddfn_pop_spair(void);
//extern void ddfn_ptr_list(struct dd_file_name_v3 *this, int list_type);
extern void ddfn_ptr_list(struct dd_file_name_v3 *ddfn, int list_type);
extern void ddfn_push_spair(struct dd_file_name_v3 *ddfns);
extern struct dd_file_name_v3 *ddfn_search(int dir_num, int radar_num, double d_target_time, int req_type, int version);
extern void ddfn_sort_insert(struct dd_radar_name_info_v3 *rni, struct dd_file_name_v3 *ddfn);
extern double ddfn_time(struct dd_file_name_v3 *ddfn);
extern int ddfnp_list(int dir_num, int radar_num, int list_type);
extern int ddir_files_v3(const int dir_num, const char *dir);
extern int ddir_files_from_command_line (const int dir_num, const char *dir,
                                         int argc, char *argv[]);
extern int ddir_radar_num_v3(int dir_num, char *radar_name);
extern void ddir_rescan_urgent(int dir_num);
extern double d_mddir_file_info_v3(int dir_num, int radar_num, double target_time, int req_type, int version, int *true_version_num, char *info_line, char *file_name);
extern struct dd_file_name_v3 **mddir_entire_list_v3(int dir_num, int radar_num, int *num_swps);
extern int32_t mddir_file_info_v3(int dir_num, int radar_num, int32_t target_time, int req_type, int version, int *true_version_num, char *info_line, char *file_name);
extern int mddir_file_list_v3(const int dir_num, const char *dir);
extern int mddir_gen_swp_list_v3(int dir_num, int radar_num, struct solo_list_mgmt *lm);
extern int mddir_gen_swp_str_list_v3(int dir_num, int radar_num, int full_file_name, struct solo_list_mgmt *lm);
extern int mddir_num_radars_v3(int dir_num);
extern char *mddir_radar_name_v3(int dir_num, int radar_num);
extern int mddir_radar_num_v3(const int dir_num, const char *radar_name);
extern struct dd_file_name_v3 **mddir_return_swp_list_v3(int dir_num, int radar_num, int *num_swps);
extern double ddfnp_list_entry(int dir_num, int radar_num, int ent_num, int *version, char *line, char *file_name);
extern struct ddir_info_v3 *return_ddir(int dir_num);
extern double ddir_free(char *dir);

#endif
