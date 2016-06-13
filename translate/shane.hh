/* created using cproto */
/* Tue Jun 21 22:06:01 UTC 2011*/

#ifndef shane_hh
#define shane_hh

#include <dd_general_info.h>
/* shane.c */

struct shane_header {
    float hour;
    float minute;
    float second;
    float gps_altitude;
    float first_alt_altitude;
    float second_alt_altitude;
    float azimuth;
    float elevation;
    float roll;
    float pitch;
    float yaw;
    float month;
    float day;
    float year;
    float prf;
    float bad_shot_count;
    float data_type_id;		/* fortran word 17 */
    float correction_to_GMT;	/* fortran word 18 */
    float latitude;		/* fortran word 19 */
    float longitude;		/* fortran word 20 */
    float range_bin_1;		/* fortran word 21 */
    float bin_spacing;		/* fortran word 22 */
    float platform_id;		/* fortran word 23 */
    float words_in_header;	/* fortran word 24 */
    float no_data_flag;		/* fortran word 25 */
    float word_26;
    float shots_per_ray;
    float intended_shot_count;
    float num_cells;
    float transect_num;
};

struct field_mgmt {
    struct field_mgmt *next;
    int fid_num;
    int parm_num;
    int sizeof_buf;
    int transect_num;
    char filename[64];
    char field_name[16];
    char *buf;
};

struct shane_info {
    int shots_per_ray;
    float data_id;
    float platform_id;
    struct field_mgmt *top_field;
    char directory[128];
    int num_cells;
    float bin_spacing;
};


extern void dd_shane_conv(void);
extern void produce_shanes_data(DGI_PTR dgi);

void stuff_shanes_hsk (DGI_PTR dgi, float *hsk, struct field_mgmt *fm);

#endif
