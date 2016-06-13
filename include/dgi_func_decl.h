#ifndef INCdgi_func_declh
#define INCdgi_func_declh

void    dd_absorb_cfac();
int  	dd_absorb_header_info();
int     dd_absorb_ray_info();
double  dd_ac_vel();
void 	dd_alloc_data_field();
double 	dd_altitude();
double 	dd_altitude_agl();
double  dd_azimuth_angle();
int     dd_cell_num();
void    dd_clear_pisp();
int     dd_clip_gate();
void    dd_copy_pisp();
double 	dd_drift();
void 	dd_dump_headers();
void 	dd_dump_ray();
double  dd_earthr();
double 	dd_elevation_angle();
char *  dd_find_desc();
int     dd_find_field();
struct dd_comm_info *dd_first_comment();
void 	dd_flush();
struct dd_general_info *dd_get_structs();
int     dd_givfld();
double 	dd_heading();
struct dd_comm_info *dd_last_comment();
double  dd_latitude();
void    dd_latlon_relative();
void    dd_latlon_shift();
char *  dd_link_dev_name();
double  dd_longitude();
struct radar_d *dd_malloc_radd();
double  dd_nav_rotation_angle();
double  dd_nav_tilt_angle();
int     dd_ndx_name();
struct dd_comm_info *dd_next_comment();
int     dd_open();
int     dd_output_flag();
double 	dd_pitch();
int     dd_position();
void 	dor_print_asib();
void 	dor_print_celv();
void    dor_print_cfac();
void 	dor_print_parm();
void 	dor_print_radd();
void 	dor_print_ryib();
void 	dor_print_swib();
void 	dor_print_vold();
void 	dorade_reset_offset();
void 	dd_radar_angles();
char *  dd_radar_name();
char *  dd_radar_namec();
void    dd_rename_swp_file();
void    dd_range_gate();
void    dd_razel_to_xyz();
void 	dd_reset_comments();
struct comment_d *dd_return_comment();
int     dd_return_id_num();
struct rot_table_entry *dd_return_rotang1();
double 	dd_roll();
int     dd_rotang_seek();
void    dd_rotang_table();
double 	dd_rotation_angle();
void    dd_rng_info();
void    dd_set_uniform_cells();
void    dd_site_name();
int  	dd_sweepfile_list_v3();
void 	dd_tape();
double 	dd_tilt_angle();
void    dd_unlink();
void    dd_write();
int     dd_xy_plane_horizontal();

void 	ddfn_file_name();
int     ddir_files_v3();	/* dd_files.c */
void    ddir_rescan_urgent();
int     ddswp_last_ray();
int  	ddswp_next_ray_v3();
void 	dgi_interest_really();
int  	dgi_buf_rewind();
void    do_print_dd_open_info();
void    dont_print_dd_open_info();
char * 	eld_nimbus_fix_asib();
void 	eld_gpro_fix_asib();

int     mddir_file_list_v3();	/* dd_files.c */
int     mddir_gen_swp_list_v3();
int     mddir_gen_swp_str_list_v3();
int     mddir_num_radars_v3();
int     mddir_radar_num_v3();
   
char *  return_dd_open_info();

#endif /* INCdgi_func_declh */
