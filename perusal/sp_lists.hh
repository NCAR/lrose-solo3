/* created using cproto */
/* Thu Jul  7 17:55:16 UTC 2011*/

#ifndef sp_lists_hh
#define sp_lists_hh

#include <string>
#include <vector>

#include <solo_window_structs.h>

extern void solo_gen_parameter_list(int frme);
extern void solo_gen_radar_list(int frme);
extern void solo_gen_swp_list(int frme);
extern int solo_get_files(char *dir, struct solo_list_mgmt *lm);

#endif
