/* created using cproto */
/* Fri Jul 15 20:57:42 UTC 2011*/

#ifndef se_catch_all_hh
#define se_catch_all_hh

#include <vector>

#include <UiCommandToken.hh>

#ifdef USE_RADX
#else
extern int se_once_only(const std::vector< UiCommandToken > &cmds);
extern int se_dir(const std::vector< UiCommandToken > &cmds);
#endif


#endif
