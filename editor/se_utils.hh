/* created using cproto */
/* Fri Jul 15 20:57:49 UTC 2011*/

#ifndef se_utils_hh
#define se_utils_hh

#include <stdio.h>

#include <sed_shared_structs.h>
#include <solo_editor_structs.h>

extern std::vector< std::string > se_absorb_strings(const std::string &path_name);
extern void se_fix_comment(std::string &comment);
extern struct sed_command_files *se_return_cmd_files_struct(void);
extern struct solo_frame_state_files *se_return_state_files_struct(void);
extern struct solo_click_info *clear_click_info(void);
extern struct solo_click_info *return_solo_click_ptr(void);
extern struct solo_edit_stuff *return_sed_stuff(void);
extern struct swp_file_input_control *return_swp_fi_in_cntrl(void);
extern void se_print_strings(const std::vector< std::string > &strings);
extern std::string se_unquote_string(const std::string &quoted_string);
extern char *fgetz(char *aa, int nn, FILE *stream);
extern void tokenize(const std::string &line,
		     std::vector< std::string > &tokens);
extern void tokenize(const std::string &line,
		     std::vector< std::string > &tokens,
		     const std::string &delimiters);
extern double angle_diff(double a1, double a2);
extern char *read_file_lines(char *line, int nn, FILE *stream);

#endif
