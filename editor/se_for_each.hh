/* created using cproto */
/* Fri Jul 15 20:57:46 UTC 2011*/

#ifndef se_for_each_hh
#define se_for_each_hh

#include <vector>

#include <UiCommandToken.hh>

#ifdef USE_RADX
#else
extern int se_cpy_field(const std::vector< UiCommandToken > &cmds);
extern int se_funfold(const std::vector< UiCommandToken > &cmds);
extern int se_header_value(const std::vector< UiCommandToken > &cmds);
extern int se_remove_field(const std::vector< UiCommandToken > &cmds);
extern int se_radial_shear(const std::vector< UiCommandToken > &cmds);
extern int se_rewrite(const std::vector< UiCommandToken > &cmds);
extern int se_threshold_field(const std::vector< UiCommandToken > &cmds);
extern int se_hard_zap(const std::vector< UiCommandToken > &cmds);
#endif

extern int se_for_each_ray(const std::vector< UiCommandToken > &cmds);

#endif
