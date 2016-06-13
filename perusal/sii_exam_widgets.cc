/* 	$Id$	 */

#include <string>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtktext.h>

#include "soloii.h"
#include "soloii.hh"
#include "sii_utils.hh"

#include <solo_editor_structs.h>
#include <sed_shared_structs.h>
#include <seds.h>
#include <se_utils.hh>
#include "sii_exam_widgets.hh"
#include "sp_basics.hh"
#include "solo2.hh"
#include <sxm_examine.hh>
#include <DataManager.hh>
#include <DataInfo.hh>

/* c------------------------------------------------------------------------ */

void se_dump_examine_widget(int frame_num, struct examine_widget_info *ewi)
{
  ExamineWindow *examine_window = frame_configs[frame_num]->examine_window;
  
  ewi->ray_num = (int)examine_window->getRayEntry();
  ewi->ray_count = (int)examine_window->getRaysEntry();
  ewi->display_format = examine_window->getFormatEntry();
  ewi->fields_list = examine_window->getFieldsEntry();
  ewi->whats_selected = examine_window->getWhatsSelected();
  ewi->typeof_change = examine_window->getTypeOfChange();

  if (examine_window->isLabelCellRay())
  {
    ewi->row_annotation = EX_VIEW_CELL_NUMS;
    ewi->col_annotation = EX_VIEW_RAY_NUMS; 
  }
  else
  {
    ewi->row_annotation = EX_VIEW_RANGES;
    ewi->col_annotation = EX_VIEW_ROT_ANGS; 
  }
  ewi->at_cell = 0;
  ewi->cell_count = -1;
}

/* c------------------------------------------------------------------------ */

void se_refresh_examine_widgets(const int frame_num,
				const std::vector< std::string > &examine_list)
{
   WW_PTR wwptr = solo_return_wwptr (frame_num);

   // NOTE:  I'm not sure why the cell range info was coming from the window 0
   // information rather than the window information for this frame.  Perhaps
   // all of the frames are set to use the data from window 0???

   // Get the cell range info.

   DataInfo *data_info0 =
     DataManager::getInstance()->getWindowInfo(0);
   
   int ncells = data_info0->getNumCellsC();
   double r0 = data_info0->getStartRangeC() * 0.001;
   double gs = data_info0->getGateSpacingC() * 0.001;
   
   // NOTE: This one comes from the window information for the current frame.
   // Why the difference?  See above.

   ExamineWindow *examine_window = frame_configs[frame_num]->examine_window;
   
   if (examine_window != 0)
   {
     examine_window->setMaxCells(ncells);
     examine_window->setStartRange(r0);
     examine_window->setGateSpacing(gs);
     
     examine_window->refreshExamineList(examine_list);
     
     examine_window->setOrigRay(wwptr->examine_info->ray_num);
     examine_window->setOrigRays(wwptr->examine_info->ray_count);
     examine_window->setOrigChanges(wwptr->examine_info->change_count);
     examine_window->setOrigCell(wwptr->examine_info->at_cell);
     examine_window->setOrigRange(r0 + wwptr->examine_info->at_cell * gs);
     examine_window->setOrigFields(wwptr->examine_info->fields_list);
     examine_window->setOrigFormat(wwptr->examine_info->display_format);
     examine_window->setOrigNyqVel(return_sed_stuff()->nyquist_velocity);

     examine_window->updateWidgets();
   }
}

/* c---------------------------------------------------------------------- */

void sii_exam_click_in_data(guint frame_num)
{
  struct solo_click_info *sci = clear_click_info();
  WW_PTR wwptr = solo_return_wwptr (frame_num);
  ExamineWindow *examine_window = frame_configs[frame_num]->examine_window;
  
  if (examine_window == 0 || !examine_window->isFrameActive())
    return;

  sci->frame = frame_num;
  examine_window->setClickedRange(wwptr->clicked_range);
  sxm_click_in_data(sci);
}
