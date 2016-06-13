/* created using cproto */
/* Thu Jul  7 17:55:17 UTC 2011*/

#ifndef sp_save_frms_hh
#define sp_save_frms_hh

#include <string>

extern int solo_absorb_window_info(const std::string &dir,
				   const std::string &fname,
				   const int ignore_swpfi_info);
extern void solo_save_window_info(const std::string &dir,
				  const std::string &a_comment);

#endif
