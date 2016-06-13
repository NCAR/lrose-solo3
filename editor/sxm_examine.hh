/* created using cproto */
/* Fri Jul 15 20:57:51 UTC 2011*/

#ifndef sxm_examine_hh
#define sxm_examine_hh

#include <string>
#include <vector>

#include <DataInfo.hh>
#include <sed_shared_structs.h>
#include <solo_window_structs.h>

extern int sxm_append_to_log_stream(const char *stuff, const int nchar);
extern void sxm_change_cell_in_list(struct se_changez *chz,
				    const int data_row_num, const double val);
extern void sxm_clear_changes(int frme);
extern void sxm_click_in_data(struct solo_click_info *sci);
extern void sxm_close_log_stream(void);
extern void sxm_flush_log_stream(void);
extern void sxm_set_log_dir(const std::string &dir);
extern void sxm_toggle_log_dir(const bool active);
extern void sxm_gen_all_files_list(int frme, std::vector< std::string > &file_list);
extern void sxm_gen_delete_lists(struct solo_click_info *sci);
extern void sxm_get_widget_info(int frme);
extern void sxm_list_beams(const int frame_index);
extern void sxm_list_descriptors(const int frame_index);
extern void sxm_list_edit_hist(const int frame_index);
extern void sxm_list_to_log(const std::vector< std::string > &list);
extern void sxm_log_stat_line(const DataInfo &data_info);
extern void sxm_open_log_stream(void);
extern struct se_changez *sxm_pop_change(struct se_changez **topptr);
extern struct se_changez *sxm_pop_spair_change(void);
extern void sxm_push_change(struct se_changez *chz, struct se_changez **topptr);
extern void sxm_push_spair_change(struct se_changez *chz);
extern void sxm_refresh_list(const int frame_index);
extern struct se_changez *sxm_remove_change(struct se_changez **topptr, struct se_changez *this_se);
extern void sxm_set_click_frame(int frme, double theta, double range);
extern void sxm_undo_last(int frme);
extern void sxm_unlink_files_list(const std::vector< std::string > &file_list,
				  const char *dir, const int frame_index);
extern void sxm_update_examine_data(int frme, int button);

#endif
