/* created using cproto */
/* Fri Jul 15 20:57:50 UTC 2011*/

#ifndef se_wgt_stf_hh
#define se_wgt_stf_hh

#include <string>
#include <vector>

#include <sed_shared_structs.h>

extern std::vector< std::string > se_update_list(int list_id);
extern void se_nab_files(const std::string &dir,
			 std::vector< std::string > &file_list,
			 const std::string &type);
extern int se_nab_all_files(char *dir, struct solo_list_mgmt *slm);

#endif
