/* created using cproto */
/* Fri Jul 15 20:57:37 UTC 2011*/

#ifndef se_add_mult_hh
#define se_add_mult_hh

#include <vector>

#include <UiCommandToken.hh>

#ifdef USE_RADX
#else
extern int se_absolute_value(const std::vector< UiCommandToken > &cmds);
extern int se_add_fields(const std::vector< UiCommandToken > &cmds);
extern int se_add_const(const std::vector< UiCommandToken > &cmds);
extern int se_assign_value(const std::vector< UiCommandToken > &cmds);
extern int se_mult_const(const std::vector< UiCommandToken > &cmds);
#endif

#endif
