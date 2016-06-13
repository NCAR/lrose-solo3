/* created using cproto */
/* Fri Jul 15 20:57:45 UTC 2011*/

#ifndef se_flag_ops_hh
#define se_flag_ops_hh

#include <vector>

#include <UiCommandToken.hh>

#ifdef USE_RADX
#else
extern int se_bad_flags_logic(const std::vector< UiCommandToken > &cmds);
extern int se_assert_bad_flags(const std::vector< UiCommandToken > &cmds);
extern int se_clear_bad_flags(const std::vector< UiCommandToken > &cmds);
extern int se_copy_bad_flags(const std::vector< UiCommandToken > &cmds);
extern int se_flagged_add(const std::vector< UiCommandToken > &cmds);
extern int se_set_bad_flags(const std::vector< UiCommandToken > &cmds);
#endif

extern void se_do_clear_bad_flags_array(int nn);

#endif
