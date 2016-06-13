/* created using cproto */
/* Fri Jul 15 20:57:47 UTC 2011*/

#ifndef se_proc_data_hh
#define se_proc_data_hh

#ifdef USE_RADX
#include <Radx/RadxRay.hh>
#endif

#include <solo_window_structs.h>
#include <ForEachRayCmd.hh>
#include <OneTimeOnlyCmd.hh>

extern int se_write_sed_cmds(const std::vector< UiCommandToken > &cmd_tokens);

extern void se_cull_setup_input(std::vector< std::string > &top_ssm);
extern void dd_edd(const int time_series, const int automatic, const int down,
		   const double d_ctr, const int frame_num);
extern void se_edit_summary(void);
extern bool se_gen_sweep_list(void);
extern std::vector< std::string > se_text_to_vector(const std::string &txt);
#ifdef USE_RADX
extern void se_perform_fer_cmds(std::vector< ForEachRayCmd* > &the_ucm,
				const int frame_num, RadxRay &ray);
extern void se_perform_oto_cmds(std::vector< OneTimeOnlyCmd* > &the_ucm);
extern void se_perform_fer_finish_up(std::vector< ForEachRayCmd* > &cmd_list);
#else
extern void se_perform_fer_cmds(std::vector< ForEachRayCmd* > &the_ucm);
extern void se_perform_oto_cmds(std::vector< OneTimeOnlyCmd* > &the_ucm);
#endif
extern bool se_process_data(const int time_series, const int automatic,
			    const int down, const double d_ctr,
			    const int frame_num);
extern void se_really_readin_cmds(void);
extern void se_setup_input_cmds(void);
extern void se_shift_setup_inputs(const std::vector< std::string > &input_list);
extern void se_tack_on_cmds(char **where, int *how_much);
extern double ts_pointing_angle(struct dd_general_info *dgi);

#endif
