#ifndef se_histog_hh
#define se_histog_hh

#include <solo_editor_structs.h>
#include <UiCommandToken.hh>

#ifdef USE_RADX
#else
extern int se_histog_setup(const std::vector< UiCommandToken > &cmds);
extern int se_xy_stuff(const std::vector< UiCommandToken > &cmds);
extern int se_histo_ray(const std::vector< UiCommandToken > &cmds);
extern void se_histo_output(void);
#endif

extern void se_histo_fin_irreg_areas(void);
extern void se_histo_fin_irreg_counts(void);
extern void se_histo_fin_areas();
extern void se_histo_fin_counts(void);
extern void se_histo_init(struct solo_edit_stuff *seds);

#endif
