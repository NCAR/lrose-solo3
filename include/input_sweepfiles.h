# ifndef INPUTSWEEPFILESH
# define INPUTSWEEPFILESH

#include <string>

struct unique_sweepfile_info_v3
{
  struct unique_sweepfile_info_v3 *last;
  struct unique_sweepfile_info_v3 *next;
  struct dd_file_name_v3 **swp_list; /* list of sweepfiles for this radar
					*/
  int ddir_num_swps;
  int ddir_radar_num;
  int dir_num;
  int forget_it;
  int max_num_sweeps;
  int new_version_flag;
  int num_rays;
  int num_swps;
  int radar_num;
  int ray_num;
  int swp_count;
  int version;
  int vol_num;
  
  std::string directory;
  std::string filename;
  std::string radar_name;
  struct dd_file_name_v3 **final_swp_list;
  struct cfac_wrap *first_cfac;
};

struct dd_input_sweepfiles_v3 {
  double reference_time;
  double last_time;
  float MB;
  int editing;
  int file_count;
  int num_radars;
  int num_swps;
  int print_swp_info;
  int ray_count;
  int rec_count;
  int sweep_count;
  int vol_count;
  struct unique_sweepfile_info_v3 *usi_top;
  struct cfac_wrap *cfac_wrap[16];
  int num_cfac_sets;
};


struct unique_sweepfile_info {
    double *swp_times;
    struct unique_sweepfile_info *last;
    struct unique_sweepfile_info *next;
    struct dd_file_name **swp_list; /* list of all sweepfiles for this radar
				     */
    int *swp_indices;		/* list of indices of the sweep files
				 * we will actually use */
    int ddir_num_swps;
    int ddir_radar_num;
    int dir_num;
    int forget_it;
    int max_num_sweeps;
    int num_rays;
    int num_swps;
    int radar_num;
    int ray_num;
    int swp_count;
    int version;
    int vol_num;

    char directory[128];
    char filename[128];
    char radar_name[12];
    int print_swp_info;
};

struct dd_input_sweepfiles {
    double reference_time;
    double last_time;
    float MB;
    int editing;
    int file_count;
    int num_radars;
    int num_swps;
    int print_swp_info;
    int ray_count;
    int rec_count;
    int sweep_count;
    int vol_count;
    struct unique_sweepfile_info *usi_top;
};

# endif
