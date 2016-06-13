/* created using cproto */
/* Fri Jul 15 20:57:39 UTC 2011*/

#ifndef se_BB_unfold_hh
#define se_BB_unfold_hh

#include <vector>

#include <UiCommandToken.hh>

#ifdef USE_RADX
#else
extern int se_BB_setup(const std::vector< UiCommandToken > &cmd_tokens);
extern int se_BB_ac_unfold(const std::vector< UiCommandToken > &cmd_tokens);
extern int se_remove_ac_motion(const std::vector< UiCommandToken > &cmd_tokens);
extern int se_remove_storm_motion(const std::vector< UiCommandToken > &cmd_tokens);
extern int se_ac_surface_tweak(const std::vector< UiCommandToken > &cmd_tokens);
#endif



#endif
