/* created using cproto */
/* Thu Jul  7 17:55:05 UTC 2011*/

#ifndef sii_exam_widgets_hh
#define sii_exam_widgets_hh

#include <string>
#include <vector>

#include <sii_widget_content_structs.h>
#include <solo_window_structs.h>

extern void se_dump_examine_widget(int frame_num, struct examine_widget_info *ewi);
extern void se_refresh_examine_widgets(const int frame_num,
				       const std::vector< std::string > &examine_list);

void sii_exam_click_in_data (guint frame_num);

#endif
