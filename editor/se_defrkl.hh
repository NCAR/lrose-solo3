/* created using cproto */
/* Fri Jul 15 20:57:44 UTC 2011*/

#ifndef se_defrkl_hh
#define se_defrkl_hh

#include <vector>

#include <UiCommandToken.hh>

#ifdef USE_RADX
#else
extern int se_despeckle(const std::vector< UiCommandToken > &cmd_tokens);
extern int se_fix_vortex_vels(const std::vector< UiCommandToken > &cmd_tokens);
extern int se_flag_freckles(const std::vector< UiCommandToken > &cmd_tokens);
extern int se_flag_glitches(const std::vector< UiCommandToken > &cmd_tokens);
extern int se_ring_zap(const std::vector< UiCommandToken > &cmd_tokens);
extern int se_rescale_field(const std::vector< UiCommandToken > &cmd_tokens);
#endif

#endif
