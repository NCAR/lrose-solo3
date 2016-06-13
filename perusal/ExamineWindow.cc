#include <iostream>

#include <se_proc_data.hh>
#include <se_utils.hh>
#include <sii_exam_widgets.hh>
#include <sii_externals.h>
#include <sii_utils.hh>
#include <solo_editor_structs.h>
#include <solo2.hh>
#include <sp_basics.hh>
#include <sxm_examine.hh>

#include "ExamineWindow.hh"


/**********************************************************************
 * Constructor
 */

ExamineWindow::ExamineWindow(const Pango::FontDescription &default_font,
		       const int frame_index) :
  Gtk::Window(),
  _frameIndex(frame_index),
  _defaultFont(default_font),
  _origFields(""),
  _origFormat(""),
  _origChanges(0),
  _origRange(0.0),
  _origNyqVel(0.0),
  _origLogDir(""),
  _origRay(0),
  _origRays(0),
  _origCell(0),
  _table(4, 6, false)
{
  // NOTE: UPDATE
  // Update the examine data

  sxm_update_examine_data(_frameIndex, 0);
  _initExamData();
  
  // Create needed fonts

  Pango::FontDescription smaller_italic_font = _defaultFont;
  smaller_italic_font.set_size(_defaultFont.get_size() - (2 * 1024));
  smaller_italic_font.set_style(Pango::STYLE_ITALIC);

  // Set the window title

  char title_string[1024];
  sprintf(title_string, "Frame %d  Examine Menu", _frameIndex + 1);
  set_title(title_string);

  set_border_width(0);
  
  // Create the containers for widget positioning

  _vbox0.set_homogeneous(false);
  _vbox0.set_spacing(4);
  add(_vbox0);

  // Create the menubar

  _createMenubar(_vbox0);

  // Add all of the fields

  _vbox0.add(_table);

  int row = 0;
  _fieldsLabel.set_text(" Fields ");
  _fieldsLabel.modify_font(_defaultFont);
  _table.attach(_fieldsLabel, 0, 1, row, row + 1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);
  
  _fieldsEntry.set_text("DZ VE RHOHV PHIDP");
  _fieldsEntry.modify_font(_defaultFont);
  _fieldsEntry.signal_activate().connect(sigc::mem_fun(*this,
						       &ExamineWindow::_fieldsActivated));
  _table.attach(_fieldsEntry, 1, 4, row, row + 1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);

  row++;
  _formatLabel.set_text(" Format ");
  _formatLabel.modify_font(_defaultFont);
  _table.attach(_formatLabel, 0, 1, row, row + 1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);
  
  _formatEntry.set_text("6.1f");
  _formatEntry.modify_font(_defaultFont);
  _formatEntry.signal_activate().connect(sigc::mem_fun(*this,
						       &ExamineWindow::_formatActivated));
  _table.attach(_formatEntry, 1, 2, row, row + 1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);
  
  _rayLabel.set_text(" Ray ");
  _rayLabel.modify_font(_defaultFont);
  _table.attach(_rayLabel, 2, 3, row, row + 1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);
  
  _rayEntry.set_text("0");
  _rayEntry.modify_font(_defaultFont);
  _rayEntry.signal_activate().connect(sigc::mem_fun(*this,
						    &ExamineWindow::_rayActivated));
  _table.attach(_rayEntry, 3, 4, row, row + 1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);
  
  row++;
  _changesLabel.set_text(" Changes ");
  _changesLabel.modify_font(_defaultFont);
  _table.attach(_changesLabel, 0, 1, row, row + 1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);
  
  _changesEntry.set_text("0");
  _changesEntry.modify_font(_defaultFont);
  _changesEntry.set_editable(false);
  _table.attach(_changesEntry, 1, 2, row, row + 1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);
  
  _raysLabel.set_text(" Rays ");
  _raysLabel.modify_font(_defaultFont);
  _table.attach(_raysLabel, 2, 3, row, row + 1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);
  
  _raysEntry.set_text("5");
  _raysEntry.modify_font(_defaultFont);
  _raysEntry.signal_activate().connect(sigc::mem_fun(*this,
						     &ExamineWindow::_raysActivated));
  _table.attach(_raysEntry, 3, 4, row, row + 1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);
  
  row++;
  _rangeLabel.set_text(" Range ");
  _rangeLabel.modify_font(_defaultFont);
  _table.attach(_rangeLabel, 0, 1, row, row + 1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);
  
  _rangeEntry.set_text("0");
  _rangeEntry.modify_font(_defaultFont);
  _rangeEntry.signal_activate().connect(sigc::mem_fun(*this,
						      &ExamineWindow::_rangeActivated));
  _table.attach(_rangeEntry, 1, 2, row, row + 1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);
  
  _cellLabel.set_text(" Cell ");
  _cellLabel.modify_font(_defaultFont);
  _table.attach(_cellLabel, 2, 3, row, row + 1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);
  
  _cellEntry.set_text("0");
  _cellEntry.modify_font(_defaultFont);
  _cellEntry.signal_activate().connect(sigc::mem_fun(*this,
						     &ExamineWindow::_cellActivated));
  _table.attach(_cellEntry, 3, 4, row, row + 1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);
  
  row++;
  _nyqVelLabel.set_text(" Nyq Vel ");
  _nyqVelLabel.modify_font(_defaultFont);
  _table.attach(_nyqVelLabel, 0, 1, row, row + 1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);

  _nyqVelEntry.set_text("0");
  _nyqVelEntry.modify_font(_defaultFont);
  _nyqVelEntry.signal_activate().connect(sigc::mem_fun(*this,
						       &ExamineWindow::_nyqVelActivated));
  _table.attach(_nyqVelEntry, 1, 2, row, row + 1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);
  
  _nyqVelHelpLabel.set_text(" 0 Implies the default Nyq Vel ");
  _nyqVelHelpLabel.modify_font(smaller_italic_font);
  _table.attach(_nyqVelHelpLabel, 2, 4, row, row + 1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);
  
  row++;
  _logDirLabel.set_text(" Log Dir ");
  _logDirLabel.modify_font(_defaultFont);
  _table.attach(_logDirLabel, 0, 1, row, row + 1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);
  
  _examData.log_dir = g_string_new("./");
  _logDirEntry.set_text(_examData.log_dir->str);
  _logDirEntry.modify_font(_defaultFont);
  _logDirEntry.signal_activate().connect(sigc::mem_fun(*this,
						       &ExamineWindow::_logDirActivated));
  _table.attach(_logDirEntry, 1, 4, row, row + 1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);
  
  // Create the buttons at the botton of the window

  _hbox0.set_homogeneous(false);
  _hbox0.set_spacing(2);
  _vbox0.add(_hbox0);
  
  _clearEditsButton.set_label("Clear Edits");
  _clearEditsButton.modify_font(_defaultFont);
  _clearEditsButton.signal_clicked().connect(sigc::mem_fun(*this,
							   &ExamineWindow::_clearEdits));
   _hbox0.pack_start(_clearEditsButton, true, true, 0);
  
   _undoButton.set_label("Undo");
   _undoButton.modify_font(_defaultFont);
   _undoButton.signal_clicked().connect(sigc::mem_fun(*this,
						      &ExamineWindow::_undo));
   _hbox0.pack_start(_undoButton, true, true, 0);
   
   _applyEditsButton.set_label("Apply Edits");
   _applyEditsButton.modify_font(_defaultFont);
   _applyEditsButton.signal_clicked().connect(sigc::mem_fun(*this,
							    &ExamineWindow::_applyEdits));
   _hbox0.pack_start(_applyEditsButton, true, true, 0);
   
   _refreshButton.set_label("Refresh");
   _refreshButton.modify_font(_defaultFont);
   _refreshButton.signal_clicked().connect(sigc::mem_fun(*this,
							 &ExamineWindow::_refresh));
   _hbox0.pack_start(_refreshButton, true, true, 0);
   
   // Update the widgets

   updateWidgets();

   // Create the examine display window and set it up so the windows will
   // always show/hide together.

   _examineDisplayWindow =
     new ExamineDisplayWindow(this, _defaultFont, _examData.max_possible_cells,
			      _examData.max_chars_per_line);
   _examineDisplayWindow->refreshList(solo_return_wwptr(_frameIndex)->examine_list,
				      _examData.max_cells,
				      _examData.clicked_range_km,
				      _examData.r0_km,
				      _examData.gs_km,
				      _examData.display_state);
   
   signal_show().connect(sigc::mem_fun(*_examineDisplayWindow,
				       &ExamineDisplayWindow::show_all));
   signal_hide().connect(sigc::mem_fun(*_examineDisplayWindow,
				       &ExamineDisplayWindow::hide));
}


/**********************************************************************
 * Destructor
 */

ExamineWindow::~ExamineWindow()
{
}


/**********************************************************************
 * refreshExamineList()
 */

void ExamineWindow::refreshExamineList(const std::vector< std::string > &examine_list)
{
  _examineDisplayWindow->refreshList(examine_list,
				     _examData.max_cells,
				     _examData.clicked_range_km,
				     _examData.r0_km,
				     _examData.gs_km,
				     _examData.display_state);
}


/**********************************************************************
 * updateWidgets()
 */

void ExamineWindow::updateWidgets()
{
  _formatEntry.set_text(_origFormat);
  _setRayEntry(_origRay);
  _setRaysEntry(_origRays);
  _fieldsEntry.set_text(_origFields);
  _setCellEntry(_origCell);
  _setRangeEntry(_origRange);
  _setChangesEntry(_origChanges);

  struct solo_edit_stuff *seds = return_sed_stuff();
  double nyq_vel = seds->nyquist_velocity;
  if (nyq_vel == 0.0)
    _origNyqVel = 0.0;
  else
    _origNyqVel = nyq_vel;

  _setNyqVelEntry(_origNyqVel);
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _applyChanges()
 */

#ifdef USE_RADX

void ExamineWindow::_applyChanges()
{
  std::cerr << "*** Entering ExamineWindow::_applyChanges()" << std::endl;
  
  struct solo_edit_stuff *seds = return_sed_stuff();
  WW_PTR wwptr = solo_return_wwptr(_frameIndex);
  if (!wwptr->examine_info->change_count)
    return;

  WW_PTR wwptrx = wwptr->lead_sweep;
  bool time_series = wwptr->view->type_of_plot & SOLO_TIME_SERIES;

  solo_set_busy_signal();
  main_window->changeCursor(false);

  struct examine_control *ecs = wwptr->examine_control;

  DataManager *data_mgr = DataManager::getInstance();
  data_mgr->setPrintDDopenInfo(false);
  data_mgr->setEditFlag(true);
  seds->process_ray_count = 0;
  seds->volume_count = 0;
  seds->sweep_count = 0;
  seds->setup_input_ndx = 0;
  seds->modified = NO;
  data_mgr->setOutputFlag(true);
    
  se_set_sfic_from_frame(_frameIndex);
  seds->sfic->clicked_frame = 0;
  g_string_truncate(gs_complaints, 0);

  seds->num_radars = 1;
  seds->radar_stack.push_back(seds->sfic->radar_names_text);
  seds->sfic->directory = seds->sfic->directory_text;

  // Set up to pass through this sweep.
  // Setting clicked_frame fakes se_gen_sweep_list() into setting up
  // to pass through the sweep in this frame if not time series

  if (!se_gen_sweep_list() || seds->punt)
  {
    main_window->changeCursor(true);
    solo_clear_busy_signal();
    if (strlen(gs_complaints->str))
      sii_message(gs_complaints->str);

    return;
  }

  DataInfo *data_info =
    data_mgr->getWindowInfo(data_mgr->getRadarNum());
  data_info->setDiskOutput(true);
  data_info->setDir(data_mgr->getRadarDir());
    
  // Find the sweep numbers that bracket the changes

  struct se_changez *changes = wwptr->changez_list;

  std::cerr << "    Change list:" << std::endl;
  int i = 0;
  
  for (struct se_changez *change = changes; change != 0;
       change = change->next, ++i)
  {
    std::cerr << "      Change number " << i << std::endl;
    std::cerr << "        field_num = " << change->field_num << std::endl;
    std::cerr << "        f_old_val = " << change->f_old_val << ", f_new_val = " << change->f_new_val << std::endl;
    std::cerr << "        ray_num = " << change->ray_num << ", cell_num = " << change->cell_num << std::endl;
    std::cerr << "        sweep_num = " << change->sweep_num << std::endl;
    std::cerr << "        row_num = " << change->row_num << ", col_num = " << change->col_num << std::endl;
    std::cerr << "        flagged_for_deletion = " << change->flagged_for_deletion << std::endl;
    std::cerr << "        window_num = " << change->window_num << std::endl;
    std::cerr << "        typeof_change = " << change->typeof_change << std::endl;
    std::cerr << "        second_cell_num = " << change->second_cell_num << std::endl;
  }
  
//  int sn_first_change = changes->sweep_num;
//  int sn_last_change = changes->sweep_num;
//
//  for (changes = changes->next; changes; changes = changes->next)
//  {
//    if (changes->sweep_num < sn_first_change)
//      sn_first_change = changes->sweep_num;
//
//    if (changes->sweep_num > sn_last_change)
//      sn_last_change = changes->sweep_num;
//  }
//
//  // Process the changes
//
//  while (true)
//  {
//    bool ignore_it = false;
//    if (!data_mgr->loadNextRay())
//      break;
//
//    int nn = wwptr->examine_info->change_count;
//    
//    if (nn > 0 &&
//	sn_first_change <= data_mgr->getSweepNum() &&
//	data_mgr->getSweepNum() <= sn_last_change)
//    {
//      changes = wwptr->changez_list->last;
//
//      // Changes are applied in reverse order
//      // see if we can find a change for this ray
//
//      for (; nn--; changes = changes->last)
//      {
//	if (changes->ray_num == data_mgr->getRayNum() - 1 &&
//	    changes->sweep_num == data_mgr->getSweepNum())
//	{
//	  // Apply the change...
//	  // Find the right cell in the right field
//	  // Scale the new value and stuff it in if it's
//	  // not a bad data flag; otherwise stuff in
//	  // a bad data flag
//	  
//	  int param_num = changes->field_num;
//	  short *data_ptr = (short *)data_info->getParamData(param_num);
//	  switch (changes->typeof_change)
//	  {
//	  case EX_DELETE :
//	    *(data_ptr + changes->cell_num) =
//	      static_cast<short>(data_info->getParamBadDataValue(param_num));
//	    break;
//
//	  case EX_MINUS_FOLD:
//	  case EX_PLUS_FOLD:
//	  case EX_REPLACE:
//	    *(data_ptr + changes->cell_num) =
//	      (short)DD_SCALE(changes->f_new_val,
//			      data_info->getParamScale(param_num),
//			      data_info->getParamBias(param_num));
//	    break;
//
//	  case EX_RAY_PLUS_FOLD:
//	  case EX_RAY_MINUS_FOLD:
//	  case EX_GT_PLUS_FOLD:
//	  case EX_GT_MINUS_FOLD:
//	  {
//	    int ival = (int)DD_SCALE(changes->f_new_val,
//				     data_info->getParamScale(param_num),
//				     data_info->getParamBias(param_num));
//	    int bad =
//	      static_cast<int>(data_info->getParamBadDataValue(param_num));
//
//	    short *data_end_ptr;
//	    
//	    if (changes->second_cell_num)
//	    {
//	      data_end_ptr = data_ptr + changes->second_cell_num;
//	      data_ptr += changes->cell_num;
//	    }
//	    else if (changes->typeof_change == EX_GT_PLUS_FOLD ||
//		     changes->typeof_change == EX_GT_MINUS_FOLD)
//	    {
//	      data_end_ptr = data_ptr + data_info->getNumCells();
//	      data_ptr += changes->cell_num;
//	    }
//	    else
//	    {
//	      data_end_ptr = data_ptr + data_info->getNumCells();
//	    }
//
//	    for (; data_ptr < data_end_ptr ; data_ptr++)
//	    {
//	      if (*data_ptr != bad)
//		*data_ptr += ival;
//	    }
//	    break;
//	  }
//	  
//	  case EX_RAY_IGNORE:
//	    if (changes->second_cell_num)
//	    {
//	      int bad =
//		static_cast<int>(data_info->getParamBadDataValue(param_num));
//	      short *data_end_ptr = data_ptr + changes->second_cell_num;
//	      data_ptr += changes->cell_num;
//
//	      for (; data_ptr < data_end_ptr ; *data_ptr++ = bad);
//	    }
//	    else
//	    {
//	      ignore_it = true;
//	    }
//	    break;
//
//	  default:
//	    break;
//	  }
//	  sxm_remove_change(&wwptr->changez_list, changes);
//	  --wwptr->examine_info->change_count;
//	}
//      }
//    }
//
//    seds->process_ray_count++;
//    if (!ignore_it)
//      data_info->writeRay();	/* to a new output file */
//	    
//    if (time_series && data_mgr->isLastRayInSweep())
//    {
//      // End of current sweep
//
//      data_mgr->flushFile(data_mgr->getRadarNum());
//    }
//
//    if (data_mgr->getRayNum() <= 1)
//    {
//      data_mgr->checkSweepFile();
//    }
//
//    if (data_info->isNewSweep())
//    {
//      printf ("%s", data_mgr->getDDopenInfo().c_str());
//    }
//
//    if (data_mgr->getSweepNum() > data_mgr->getNumSweeps())
//      break;
//
//    if (!(data_info->isNewSweep() && ignore_it))
//    {
//      data_info->setNewSweep(false);
//      data_info->setNewVolume(false);
//    }
//  }

//  data_mgr->flushFile(data_mgr->getRadarNum());
  seds->time_modified = time(0);
  wwptr->lead_sweep->sweep_file_modified = YES;
  data_mgr->setEditFlag(false);
  printf("Finished!\n");
  if (strlen (gs_complaints->str))
    sii_message (gs_complaints->str);

  main_window->changeCursor(true);
  solo_clear_busy_signal();
}

#else

void ExamineWindow::_applyChanges()
{
  struct solo_edit_stuff *seds = return_sed_stuff();
  WW_PTR wwptr = solo_return_wwptr(_frameIndex);
  if (!wwptr->examine_info->change_count)
    return;

  WW_PTR wwptrx = wwptr->lead_sweep;
  bool time_series = wwptr->view->type_of_plot & SOLO_TIME_SERIES;

  solo_set_busy_signal();
  main_window->changeCursor(false);

  struct examine_control *ecs = wwptr->examine_control;

  DataManager *data_mgr = DataManager::getInstance();
  data_mgr->setPrintDDopenInfo(false);
  data_mgr->setEditFlag(true);
  seds->process_ray_count = 0;
  seds->volume_count = 0;
  seds->sweep_count = 0;
  seds->setup_input_ndx = 0;
  seds->modified = NO;
  seds->finish_up = NO;
  data_mgr->setOutputFlag(true);
    
  se_set_sfic_from_frame(_frameIndex);
  seds->sfic->clicked_frame = 0;
  g_string_truncate(gs_complaints, 0);

  seds->num_radars = 1;
  seds->radar_stack.push_back(seds->sfic->radar_names_text);
  seds->sfic->directory = seds->sfic->directory_text;

  // Set up to pass through this sweep.
  // Setting clicked_frame fakes se_gen_sweep_list() into setting up
  // to pass through the sweep in this frame if not time series

  if (!se_gen_sweep_list() || seds->punt)
  {
    main_window->changeCursor(true);
    solo_clear_busy_signal();
    if (strlen(gs_complaints->str))
      sii_message(gs_complaints->str);

    return;
  }

  DataInfo *data_info =
    data_mgr->getWindowInfo(data_mgr->getRadarNum());
  data_info->setCompressionScheme(HRD_COMPRESSION);
  data_info->setDiskOutput(true);
  data_info->setDir(data_mgr->getRadarDir());
    
  // Find the sweep numbers that bracket the changes

  struct se_changez *changes = wwptr->changez_list;
  int sn_first_change = changes->sweep_num;
  int sn_last_change = changes->sweep_num;

  for (changes = changes->next; changes; changes = changes->next)
  {
    if (changes->sweep_num < sn_first_change)
      sn_first_change = changes->sweep_num;

    if (changes->sweep_num > sn_last_change)
      sn_last_change = changes->sweep_num;
  }

  for (;;)
  {
    bool ignore_it = false;
    if (!data_mgr->loadNextRay())
      break;

    int nn = wwptr->examine_info->change_count;
    
    if (nn > 0 &&
	sn_first_change <= data_mgr->getSweepNum() &&
	data_mgr->getSweepNum() <= sn_last_change)
    {
      
      changes = wwptr->changez_list->last;

      // Changes are applied in reverse order
      // see if we can find a change for this ray

      for (; nn--; changes = changes->last)
      {
	if (changes->ray_num == data_mgr->getRayNum() - 1 &&
	    changes->sweep_num == data_mgr->getSweepNum())
	{
	  // Apply the change...
	  // Find the right cell in the right field
	  // Scale the new value and stuff it in if it's
	  // not a bad data flag; otherwise stuff in
	  // a bad data flag
	  
	  int param_num = changes->field_num;
	  short *data_ptr = (short *)data_info->getParamData(param_num);
	  switch (changes->typeof_change)
	  {
	  case EX_DELETE :
	    *(data_ptr + changes->cell_num) =
	      static_cast<short>(data_info->getParamBadDataValue(param_num));
	    break;

	  case EX_MINUS_FOLD:
	  case EX_PLUS_FOLD:
	  case EX_REPLACE:
	    *(data_ptr + changes->cell_num) =
	      (short)DD_SCALE(changes->f_new_val,
			      data_info->getParamScale(param_num),
			      data_info->getParamBias(param_num));
	    break;

	  case EX_RAY_PLUS_FOLD:
	  case EX_RAY_MINUS_FOLD:
	  case EX_GT_PLUS_FOLD:
	  case EX_GT_MINUS_FOLD:
	  {
	    int ival = (int)DD_SCALE(changes->f_new_val,
				     data_info->getParamScale(param_num),
				     data_info->getParamBias(param_num));
	    int bad =
	      static_cast<int>(data_info->getParamBadDataValue(param_num));

	    short *data_end_ptr;
	    
	    if (changes->second_cell_num)
	    {
	      data_end_ptr = data_ptr + changes->second_cell_num;
	      data_ptr += changes->cell_num;
	    }
	    else if (changes->typeof_change == EX_GT_PLUS_FOLD ||
		     changes->typeof_change == EX_GT_MINUS_FOLD)
	    {
	      data_end_ptr = data_ptr + data_info->getNumCells();
	      data_ptr += changes->cell_num;
	    }
	    else
	    {
	      data_end_ptr = data_ptr + data_info->getNumCells();
	    }

	    for (; data_ptr < data_end_ptr ; data_ptr++)
	    {
	      if (*data_ptr != bad)
		*data_ptr += ival;
	    }
	    break;
	  }
	  
	  case EX_RAY_IGNORE:
	    if (changes->second_cell_num)
	    {
	      int bad =
		static_cast<int>(data_info->getParamBadDataValue(param_num));
	      short *data_end_ptr = data_ptr + changes->second_cell_num;
	      data_ptr += changes->cell_num;

	      for (; data_ptr < data_end_ptr ; *data_ptr++ = bad);
	    }
	    else
	    {
	      ignore_it = true;
	    }
	    break;

	  default:
	    break;
	  }
	  sxm_remove_change(&wwptr->changez_list, changes);
	  --wwptr->examine_info->change_count;
	}
      }
    }

    seds->process_ray_count++;
    if (!ignore_it)
      data_info->writeRay();	/* to a new output file */
	    
    if (time_series && data_mgr->isLastRayInSweep())
    {
      // End of current sweep

      data_mgr->flushFile(data_mgr->getRadarNum());
    }

    if (data_mgr->getRayNum() <= 1)
    {
      data_mgr->checkSweepFile();
    }

    if (data_info->isNewSweep())
    {
      printf ("%s", data_mgr->getDDopenInfo().c_str());
    }

    if (data_mgr->getSweepNum() > data_mgr->getNumSweeps())
      break;

    if (!(data_info->isNewSweep() && ignore_it))
    {
      data_info->setNewSweep(false);
      data_info->setNewVolume(false);
    }
  }

  data_mgr->flushFile(data_mgr->getRadarNum());
  seds->time_modified = time(0);
  wwptr->lead_sweep->sweep_file_modified = YES;
  data_mgr->setEditFlag(false);
  printf("Finished!\n");
  if (strlen (gs_complaints->str))
    sii_message (gs_complaints->str);

  main_window->changeCursor(true);
  solo_clear_busy_signal();
}

#endif

/**********************************************************************
 * _applyEdits()
 */

void ExamineWindow::_applyEdits()
{
  if (solo_busy())
    return;

  solo_set_busy_signal();
  WW_PTR wwptr = solo_return_wwptr(_frameIndex);

  _applyChanges();
  sxm_update_examine_data(_frameIndex, 0);
  se_refresh_examine_widgets(_frameIndex, wwptr->examine_list);
  solo_clear_busy_signal();
}


/**********************************************************************
 * _azRangeLabels()
 */

void ExamineWindow::_azRangeLabels()
{
  if (!_optionsAzRngLabelsChoice->get_active())
    return;
  
  sxm_refresh_list(_frameIndex);
}


/**********************************************************************
 * _cancel()
 */

void ExamineWindow::_cancel()
{
  _examData.frame_active = false;
  hide();
}


/**********************************************************************
 * _cellActivated()
 */

void ExamineWindow::_cellActivated()
{
  int height = _examineDisplayWindow->getLayoutHeight();

  // Get the entered cell value

  int gate_index = getCellEntry();
  if (gate_index < 0)
    gate_index = 0;
  
  // Get the number of cells being displayed

  int cells = height / _examineDisplayWindow->getLabelHeight();

  // We want to display as much data as possible, so if moving to this gate
  // would put us too close to the end of the beam, back up so that the display
  // window is filled.

  if (gate_index + cells >= _examData.max_cells)
    gate_index = _examData.max_cells - cells;
  
  // I don't understand this calculation.

  int value = (gate_index + 2) * _examineDisplayWindow->getLabelHeight() + 1;

  // Update the cell entry value since it could have changed

  _setCellEntry(gate_index);

  // Calculate and display the range

  _setRangeEntry(_examData.r0_km + (gate_index * _examData.gs_km));

  _examineDisplayWindow->setLayoutVadjValue(value);
}


/**********************************************************************
 * _cellRayLabels()
 */

void ExamineWindow::_cellRayLabels()
{
  if (_optionsCellRayLabelsChoice->get_active())
    sxm_refresh_list(_frameIndex);
}


/**********************************************************************
 * _clearEdits()
 */

void ExamineWindow::_clearEdits()
{
  if (solo_busy())
    return;

  solo_set_busy_signal();
  WW_PTR wwptr = solo_return_wwptr(_frameIndex);

  sxm_clear_changes(_frameIndex);
  sxm_list_to_log(wwptr->examine_list);
  se_refresh_examine_widgets(_frameIndex, wwptr->examine_list);

  solo_clear_busy_signal();
}


/**********************************************************************
 * _closeLogFile()
 */

void ExamineWindow::_closeLogFile()
{
  sxm_close_log_stream();
}


/**********************************************************************
 * _createMenubar()
 */

void ExamineWindow::_createMenubar(Gtk::Box &container)
{
  // Specify the XML for the menus

  Glib::ustring ui_info =
    "<ui>"
    "  <menubar name='MenuBar'>"
    "    <menu action='DisplayMenu'>"
    "      <menuitem action='DisplayClose'/>"
    "      <separator/>"
    "      <menu action='DisplayChoiceMenu'>"
    "        <menuitem action='DisplayChoiceCellValues'/>"
    "        <menuitem action='DisplayChoiceRayInfo'/>"
    "        <menuitem action='DisplayChoiceMetadata'/>"
    "        <menuitem action='DisplayChoiceEditHist'/>"
    "      </menu>"
    "    </menu>"
    "    <menu action='EditMenu'>"
    "      <menuitem action='EditChoiceDelete'/>"
    "      <menuitem action='EditChoiceMinusFold'/>"
    "      <menuitem action='EditChoicePlusFold'/>"
    "      <menuitem action='EditChoiceDeleteRay'/>"
    "      <menuitem action='EditChoiceMinusFoldRay'/>"
    "      <menuitem action='EditChoicePlusFoldRay'/>"
    "      <menuitem action='EditChoiceMinusFoldRayGT'/>"
    "      <menuitem action='EditChoicePlusFoldRayGT'/>"
    "      <menuitem action='EditChoiceZapGndSpd'/>"
    "    </menu>"
    "    <menu action='OptionsMenu'>"
    "      <menu action='OptionsLabelsChoiceMenu'>"
    "        <menuitem action='OptionsLabelsAzRng'/>"
    "        <menuitem action='OptionsLabelsCellRay'/>"
    "      </menu>"
    "      <separator/>"
    "      <menuitem action='OptionsLoggingActive'/>"
    "      <menuitem action='OptionsCloseLogFile'/>"
    "      <menuitem action='OptionsFlushToLogFile'/>"
    "    </menu>"
    "    <menu action='ReplotMenu'>"
    "      <menuitem action='ReplotReplotThis'/>"
    "      <menuitem action='ReplotReplotLinks'/>"
    "      <menuitem action='ReplotReplotAll'/>"
    "    </menu>"
    "    <menu action='HelpMenu'>"
    "      <menuitem action='HelpOverview'/>"
    "      <menuitem action='HelpEdit'/>"
    "      <menuitem action='HelpOptions'/>"
    "      <menuitem action='HelpNyqVel'/>"
    "      <menuitem action='HelpLogDir'/>"
    "    </menu>"
    "    <menu action='CancelMenu'>"
    "      <menuitem action='CancelCancel'/>"
    "    </menu>"
    "  </menubar>"
    "</ui>";

  // Create the actions for the menus

  _actionGroup = Gtk::ActionGroup::create();
  
  // Display menu actions

  _actionGroup->add(Gtk::Action::create("DisplayMenu", "Display"));
  _actionGroup->add(Gtk::Action::create("DisplayClose", "Close"),
		    sigc::mem_fun(*this,
				  &ExamineWindow::_cancel));

  _actionGroup->add(Gtk::Action::create("DisplayChoiceMenu",
					"Display..."));
  Gtk::RadioAction::Group group_display_choice;
  _displayCellValuesChoice =
    Gtk::RadioAction::create(group_display_choice,
			     "DisplayChoiceCellValues",
			     "Cell Values");
  _actionGroup->add(_displayCellValuesChoice,
		    sigc::mem_fun(*this,
				  &ExamineWindow::_displayCellValues));
  _displayRayInfoChoice =
    Gtk::RadioAction::create(group_display_choice,
			     "DisplayChoiceRayInfo",
			     "Ray Info");
  _actionGroup->add(_displayRayInfoChoice,
		    sigc::mem_fun(*this,
				  &ExamineWindow::_displayRayInfo));
  _displayMetadataChoice =
    Gtk::RadioAction::create(group_display_choice,
			     "DisplayChoiceMetadata",
			     "Metadata");
  _actionGroup->add(_displayMetadataChoice,
		    sigc::mem_fun(*this,
				  &ExamineWindow::_displayMetadata));
  _displayEditHistChoice =
    Gtk::RadioAction::create(group_display_choice,
			     "DisplayChoiceEditHist",
			     "Edit Hist");
  _actionGroup->add(_displayEditHistChoice,
		    sigc::mem_fun(*this,
				  &ExamineWindow::_displayEditHistory));

  // Edit menu actions

  _actionGroup->add(Gtk::Action::create("EditMenu", "Edit"));

  Gtk::RadioAction::Group group_edit_choice;
  _editDeleteChoice =
    Gtk::RadioAction::create(group_edit_choice,
			     "EditChoiceDelete",
			     "Delete");
  _actionGroup->add(_editDeleteChoice,
		    sigc::mem_fun(*this,
				  &ExamineWindow::_editDelete));
  _editMinusFoldChoice =
    Gtk::RadioAction::create(group_edit_choice,
			     "EditChoiceMinusFold",
			     "- Fold");
  _actionGroup->add(_editMinusFoldChoice,
		    sigc::mem_fun(*this,
				  &ExamineWindow::_editMinusFold));
  _editPlusFoldChoice =
    Gtk::RadioAction::create(group_edit_choice,
			     "EditChoicePlusFold",
			     "+ Fold");
  _actionGroup->add(_editPlusFoldChoice,
		    sigc::mem_fun(*this,
				  &ExamineWindow::_editPlusFold));
  _editDeleteRayChoice =
    Gtk::RadioAction::create(group_edit_choice,
			     "EditChoiceDeleteRay",
			     "Delete Ray");
  _actionGroup->add(_editDeleteRayChoice,
		    sigc::mem_fun(*this,
				  &ExamineWindow::_editDeleteRay));
  _editMinusFoldRayChoice =
    Gtk::RadioAction::create(group_edit_choice,
			     "EditChoiceMinusFoldRay",
			     "- Fold Ray");
  _actionGroup->add(_editMinusFoldRayChoice,
		    sigc::mem_fun(*this,
				  &ExamineWindow::_editMinusFoldRay));
  _editPlusFoldRayChoice =
    Gtk::RadioAction::create(group_edit_choice,
			     "EditChoicePlusFoldRay",
			     "+ Fold Ray");
  _actionGroup->add(_editPlusFoldRayChoice,
		    sigc::mem_fun(*this,
				  &ExamineWindow::_editPlusFoldRay));
  _editMinusFoldRayGTChoice =
    Gtk::RadioAction::create(group_edit_choice,
			     "EditChoiceMinusFoldRayGT",
			     "- Fold Ray >");
  _actionGroup->add(_editMinusFoldRayGTChoice,
		    sigc::mem_fun(*this,
				  &ExamineWindow::_editMinusFoldRayGT));
  _editPlusFoldRayGTChoice =
    Gtk::RadioAction::create(group_edit_choice,
			     "EditChoicePlusFoldRayGT",
			     "+ Fold Ray >");
  _actionGroup->add(_editPlusFoldRayGTChoice,
		    sigc::mem_fun(*this,
				  &ExamineWindow::_editPlusFoldRayGT));
  _editZapGndSpdChoice =
    Gtk::RadioAction::create(group_edit_choice,
			     "EditChoiceZapGndSpd",
			     "Zap Gnd Spd");
  _actionGroup->add(_editZapGndSpdChoice,
		    sigc::mem_fun(*this,
				  &ExamineWindow::_editZapGroundSpeed));

  // Options menu actions

  _actionGroup->add(Gtk::Action::create("OptionsMenu", "Options"));
  _actionGroup->add(Gtk::Action::create("OptionsLabelsChoiceMenu",
					"Labels..."));
  Gtk::RadioAction::Group group_options_labels;
  _optionsAzRngLabelsChoice =
    Gtk::RadioAction::create(group_options_labels,
			     "OptionsLabelsAzRng",
			     "Az,Rng Labels");
  _actionGroup->add(_optionsAzRngLabelsChoice,
		    sigc::mem_fun(*this,
				  &ExamineWindow::_azRangeLabels));
  _optionsCellRayLabelsChoice =
    Gtk::RadioAction::create(group_options_labels,
			     "OptionsLabelsCellRay",
			     "Cell,Ray Labels");
  _actionGroup->add(_optionsCellRayLabelsChoice,
		    sigc::mem_fun(*this,
				  &ExamineWindow::_cellRayLabels));

  _loggingActiveToggle =
    Gtk::ToggleAction::create("OptionsLoggingActive",
			      "Logging Active", "", false);
  _actionGroup->add(_loggingActiveToggle,
		    sigc::mem_fun(*this,
				  &ExamineWindow::_toggleLogging));
  _actionGroup->add(Gtk::Action::create("OptionsCloseLogFile",
					"Close log file"),
		    sigc::mem_fun(*this,
				  &ExamineWindow::_closeLogFile));
  _actionGroup->add(Gtk::Action::create("OptionsFlushToLogFile",
					"Flush to log file"),
		    sigc::mem_fun(*this,
				  &ExamineWindow::_flushLogFile));

  // Replot menu actions

  _actionGroup->add(Gtk::Action::create("ReplotMenu", "Replot"));
  _actionGroup->add(Gtk::Action::create("ReplotReplotThis", "Replot This"),
		    sigc::mem_fun(*this,
				  &ExamineWindow::_replotThis));
  _actionGroup->add(Gtk::Action::create("ReplotReplotLinks", "Replot Links"),
		    sigc::mem_fun(*this,
				  &ExamineWindow::_replotLinks));
  _actionGroup->add(Gtk::Action::create("ReplotReplotAll", "Replot All"),
		    sigc::mem_fun(*this,
				  &ExamineWindow::_replotAll));

  // Help menu actions

  _actionGroup->add(Gtk::Action::create("HelpMenu", "Help"));
  _actionGroup->add(Gtk::Action::create("HelpOverview", "Overview"),
		    sigc::mem_fun(*this,
				  &ExamineWindow::_displayHelpOverview));
  _actionGroup->add(Gtk::Action::create("HelpEdit", "Edit"),
		    sigc::mem_fun(*this,
				  &ExamineWindow::_displayHelpEdit));
  _actionGroup->add(Gtk::Action::create("HelpOptions", "Options"),
		    sigc::mem_fun(*this,
				  &ExamineWindow::_displayHelpOptions));
  _actionGroup->add(Gtk::Action::create("HelpNyqVel", "Nyq Vel"),
		    sigc::mem_fun(*this,
				  &ExamineWindow::_displayHelpNyqVel));
  _actionGroup->add(Gtk::Action::create("HelpLogDir", "Log Dir"),
		    sigc::mem_fun(*this,
				  &ExamineWindow::_displayHelpLogDir));

  // Cancel menu actions

  _actionGroup->add(Gtk::Action::create("CancelMenu", "Cancel"));
  _actionGroup->add(Gtk::Action::create("CancelCancel", "Cancel"),
		    sigc::mem_fun(*this,
				  &ExamineWindow::_cancel));

  // Create the UI manager

  _uiManager = Gtk::UIManager::create();
  _uiManager->insert_action_group(_actionGroup);
  
  add_accel_group(_uiManager->get_accel_group());
  
  // Parse the XML

  try
  {
    _uiManager->add_ui_from_string(ui_info);
  }
  catch (const Glib::Error &ex)
  {
    std::cerr << "Building menus failed: " << ex.what();
  }
  
  // Get the menubar and add it to the container widget

  Gtk::Widget *menubar = _uiManager->get_widget("/MenuBar");
  if (menubar != 0)
    container.pack_start(*menubar,
			 false,      // expand
			 true,       // fill - ignored since expand is false
			 0);         // padding
}


/**********************************************************************
 * _displayCellValues()
 */

void ExamineWindow::_displayCellValues()
{
  if (!_displayCellValuesChoice->get_active())
    return;
  
  _examData.display_state = ExamineDisplayWindow::DISPLAY_DATA;

  if (solo_busy())
    return;

  solo_set_busy_signal();
  WW_PTR wwptr = solo_return_wwptr(_frameIndex);

  wwptr->examine_info->whats_selected = EX_RADAR_DATA;
  sxm_refresh_list(_frameIndex);
  sxm_list_to_log(wwptr->examine_list);

  solo_clear_busy_signal();
}


/**********************************************************************
 * _displayEditHistory()
 */

void ExamineWindow::_displayEditHistory()
{
  if (!_displayEditHistChoice->get_active())
    return;
  
  _examData.display_state = ExamineDisplayWindow::DISPLAY_EDIT_HISTORY;

  if (solo_busy())
    return;

  solo_set_busy_signal();

  WW_PTR wwptr = solo_return_wwptr(_frameIndex);

  wwptr->examine_info->whats_selected = EX_EDIT_HIST;
  sxm_refresh_list(_frameIndex);
  sxm_list_to_log(wwptr->examine_list);

  solo_clear_busy_signal();
}


/**********************************************************************
 * _displayHelpEdit()
 */

void ExamineWindow::_displayHelpEdit()
{
  static const Glib::ustring help_text =
    Glib::ustring("The \"Edit\" submenu enables flagging individual cells as bad\n") +
    "or subtracting or adding the Nyquist interval to clicked cells or\n" +
    "removing the aircraft motion from a velocity cell of aircraft data.\n" +
    "\n" +
"One can also cause a ray to be removed from the sweepfile by clicking\n" +
    "the \"Delete Ray\" radio button and clicking in one of the data\n" +
    "columns for that ray.\n" +
    "\n" +
    "Subtracting or adding the Nyquist Interval to the whole ray\n" +
    "is also an option.\n" +
    "\n" +
    "If it is desireable to unfold some contiguous range of cells rather\n" +
    "than the whole ray and the user notes the cell at which unfolding\n" +
    "should end and then clicks the cell where the unfolding should start,\n" +
    "the whole ray will initially be modified, but if the user subsequently\n" +
    "clicks on the noted final cell the unfolding will be only within the\n" +
    "range defined by the first and second cells clicked as long as the two\n" +
    "cells clicked are in the same column.\n" +
    "\n" +
    "Folding operations on a ray from the clicked cell to the end are\n" +
    "accomplished with the \"- Fold Ray >\" and \"+ Fold Ray >\" options.\n";

  sii_message(help_text);
}


/**********************************************************************
 * _displayHelpLogDir()
 */

void ExamineWindow::_displayHelpLogDir()
{
  static const Glib::ustring help_text =
    Glib::ustring("The \"Log Dir\" entry defines the directory into which log files will\n") +
    "be placed. Hitting Return/Enter in the Log Dir entry activates logging\n" +
    "of the contents of the \"Examine Display Widget\".\n" +
    "\n" +
    "Changing the directory name and hitting Return closes the current file\n" +
    "if there is one and opens a new file in the new directory.\n";
  
  sii_message(help_text);
}


/**********************************************************************
 * _displayHelpNyqVel()
 */

void ExamineWindow::_displayHelpNyqVel()
{
  static const Glib::ustring help_text =
    Glib::ustring("When \"Nyq Vel\" is set to zero the Nyquist Velocity from the data is\n") +
    "used. To change that, enter the temporary Nyquist Velocity and hit\n" +
    "Return/Enter. To return to using the default data Nyquist Velocity\n" +
    "type \"0\" and hit Return.\n" +
    "\n" +
    "The default data Nyquest Velocity can be found by clicking \"Metadata\"\n" +
    "int the Display menu and looking for \"eff_unamb_vel\" in the radar\n" +
    "(\"RADD\") descriptor.\n";
  
  sii_message(help_text);
}


/**********************************************************************
 * _displayHelpOptions()
 */

void ExamineWindow::_displayHelpOptions()
{
  static const Glib::ustring help_text =
    Glib::ustring("The \"Options\" submenu provides for switching the display reference\n") +
    "information between (rotation angle, range) and (ray number, cell number).\n" +
    "\n" +
    "Also the user can toggle logging updates of the Examine Display Widget\n" +
    "on and off plus closing the current log file to begin a new one or\n" +
    "flushing the current logging information to the file in order to be\n" +
    "visable to other processes.\n";

  sii_message(help_text);
}


/**********************************************************************
 * _displayHelpOverview()
 */

void ExamineWindow::_displayHelpOverview()
{
  static const Glib::ustring help_text =
    Glib::ustring("The Examine widget enables the \"Display\" of cell values for several\n") +
    "variables and several rays for the sweepfile displayed in the frame\n" +
    "plus an inventory of all the rays in the sweepfile, the contents of\n" +
    "the DORADE sweepfile headers \"Metadata\" and a list of the editing\n" +
    "commands applied to the sweepfile.\n" +
    "\n"
    "It is also possible to edit individual cells by applying selected\n" +
    "operations under the \"Edit\" menu.\n" +
    "\n" +
    "When displaying cell values, hitting the left arrow key changes\n" +
    "the display to start one ray earlier and right arrow moves every\n" +
    "colummn to one ray later. The up arrow scrolls the display to\n" +
    "previous lines and the down arrow moves to the next set of lines.\n" +
    "\n" +
    "All the cells in each the rays selected are accessible\n" +
    "for display and editing.\n" +
    "\n" +
    "Hitting Return/Enter in the entry fields of the \"Examine Menu\"\n" +
    "widget causes a redisplay of the data based on the new contents\n" +
    "of the entry.\n" +
    "\n" +
    "The \"Ray\" entry selects the first ray of the display.  Typing a\n" +
    "\"Range\" value will display the cell nearest the range to be\n" +
    "displayed at the top of the \"Examine Display Widget\".  Likewise\n" +
    "typing a \"Cell\" number shifts the display be begin at the cell\n" +
    "entered.\n";

  sii_message(help_text);
}


/**********************************************************************
 * _displayMetadata()
 */

void ExamineWindow::_displayMetadata()
{
  if (!_displayMetadataChoice->get_active())
    return;
  
  _examData.display_state = ExamineDisplayWindow::DISPLAY_METADATA;

  if (solo_busy())
    return;

  solo_set_busy_signal();
  WW_PTR wwptr = solo_return_wwptr(_frameIndex);

  wwptr->examine_info->whats_selected = EX_DESCRIPTORS;
  sxm_refresh_list(_frameIndex);
  sxm_list_to_log(wwptr->examine_list);

  solo_clear_busy_signal();
}


/**********************************************************************
 * _displayRayInfo()
 */

void ExamineWindow::_displayRayInfo()
{
  if (!_displayRayInfoChoice->get_active())
    return;
  
  _examData.display_state = ExamineDisplayWindow::DISPLAY_RAYS;

  if (solo_busy())
    return;
  
  solo_set_busy_signal();
  WW_PTR wwptr = solo_return_wwptr(_frameIndex);

  wwptr->examine_info->whats_selected = EX_BEAM_INVENTORY;
  sxm_refresh_list(_frameIndex);
  sxm_list_to_log(wwptr->examine_list);

  solo_clear_busy_signal();
}


/**********************************************************************
 * _editDelete()
 */

void ExamineWindow::_editDelete()
{
  if (!_editDeleteChoice->get_active())
    return;
  
  _examData.operation_state = OPERATION_DELETE;

  if (solo_busy())
    return;

  solo_set_busy_signal();
  WW_PTR wwptr = solo_return_wwptr(_frameIndex);

  sxm_get_widget_info(_frameIndex);
  wwptr->examine_info->typeof_change = EX_DELETE;

  solo_clear_busy_signal();
}


/**********************************************************************
 * _editDeleteRay()
 */

void ExamineWindow::_editDeleteRay()
{
  if (!_editDeleteRayChoice->get_active())
    return;
  
  _examData.operation_state = OPERATION_DELETE_RAY;

  if (solo_busy())
    return;

  WW_PTR wwptr = solo_return_wwptr(_frameIndex);

  solo_set_busy_signal();

  sxm_get_widget_info(_frameIndex);
  wwptr->examine_info->typeof_change = EX_RAY_IGNORE;

  solo_clear_busy_signal();
}


/**********************************************************************
 * _editMinusFold()
 */

void ExamineWindow::_editMinusFold()
{
  if (!_editMinusFoldChoice->get_active())
    return;
  
  _examData.operation_state = OPERATION_NEG_FOLD;

  if (solo_busy())
    return;

  WW_PTR wwptr = solo_return_wwptr(_frameIndex);

  solo_set_busy_signal();

  sxm_get_widget_info(_frameIndex);
  wwptr->examine_info->typeof_change = EX_MINUS_FOLD;

  solo_clear_busy_signal();
}


/**********************************************************************
 * _editMinusFoldRay()
 */

void ExamineWindow::_editMinusFoldRay()
{
  if (!_editMinusFoldRayChoice->get_active())
    return;
  
  _examData.operation_state = OPERATION_NEG_FOLD_RAY;

  WW_PTR wwptr = solo_return_wwptr(_frameIndex);

  if (solo_busy())
    return;

  solo_set_busy_signal();

  sxm_get_widget_info(_frameIndex);
  wwptr->examine_info->typeof_change = EX_RAY_MINUS_FOLD;

  solo_clear_busy_signal();
}


/**********************************************************************
 * _editMinusFoldRayGT()
 */

void ExamineWindow::_editMinusFoldRayGT()
{
  if (!_editMinusFoldRayGTChoice->get_active())
    return;
  
  _examData.operation_state = OPERATION_NEG_FOLD_GT;

  if (solo_busy())
    return;

  WW_PTR wwptr = solo_return_wwptr(_frameIndex);

  solo_set_busy_signal();

  sxm_get_widget_info(_frameIndex);
  wwptr->examine_info->typeof_change = EX_GT_MINUS_FOLD;

  solo_clear_busy_signal();
}


/**********************************************************************
 * _editPlusFold()
 */

void ExamineWindow::_editPlusFold()
{
  if (!_editPlusFoldChoice->get_active())
    return;
  
  _examData.operation_state = OPERATION_POS_FOLD;

  if (solo_busy())
    return;

  WW_PTR wwptr = solo_return_wwptr(_frameIndex);

  solo_set_busy_signal();

  sxm_get_widget_info(_frameIndex);
  wwptr->examine_info->typeof_change = EX_PLUS_FOLD;

  solo_clear_busy_signal();
}


/**********************************************************************
 * _editPlusFoldRay()
 */

void ExamineWindow::_editPlusFoldRay()
{
  if (!_editPlusFoldRayChoice->get_active())
    return;
  
  _examData.operation_state = OPERATION_POS_FOLD_RAY;

  if (solo_busy())
    return;

  WW_PTR wwptr = solo_return_wwptr(_frameIndex);

  solo_set_busy_signal();

  sxm_get_widget_info(_frameIndex);
  wwptr->examine_info->typeof_change = EX_RAY_PLUS_FOLD;

  solo_clear_busy_signal();
}


/**********************************************************************
 * _editPlusFoldRayGT()
 */

void ExamineWindow::_editPlusFoldRayGT()
{
  if (!_editPlusFoldRayGTChoice->get_active())
    return;
  
  _examData.operation_state = OPERATION_POS_FOLD_GT;

  if (solo_busy())
    return;

  WW_PTR wwptr = solo_return_wwptr(_frameIndex);

  solo_set_busy_signal();

  sxm_get_widget_info(_frameIndex);
  wwptr->examine_info->typeof_change = EX_GT_PLUS_FOLD;

  solo_clear_busy_signal();
}


/**********************************************************************
 * _editZapGroundSpeed()
 */

void ExamineWindow::_editZapGroundSpeed()
{
  if (!_editZapGndSpdChoice->get_active())
    return;
  
  _examData.operation_state = OPERATION_ZAP_GND_SPD;

  if (solo_busy())
    return;

  WW_PTR wwptr = solo_return_wwptr(_frameIndex);

  solo_set_busy_signal();

  sxm_get_widget_info(_frameIndex);
  wwptr->examine_info->typeof_change = EX_REMOVE_AIR_MOTION;

  solo_clear_busy_signal();
}


/**********************************************************************
 * _fieldsActivated()
 */

void ExamineWindow::_fieldsActivated()
{
  sxm_refresh_list(_frameIndex);
}


/**********************************************************************
 * _flushLogFile()
 */

void ExamineWindow::_flushLogFile()
{
  sxm_flush_log_stream();
}


/**********************************************************************
 * _formatActivated()
 */

void ExamineWindow::_formatActivated()
{
  sxm_refresh_list(_frameIndex);
}


/**********************************************************************
 * _initExamData()
 */

void ExamineWindow::_initExamData()
{
  _examData.frame_active = true;
  _examData.display_state = ExamineDisplayWindow::DISPLAY_DATA;
  _examData.operation_state = OPERATION_DELETE;
  _examData.max_cells = 2064;
  _examData.max_possible_cells = 2064;
  _examData.max_chars_per_line = 256;
  _examData.r0_km = 0.0;
  _examData.gs_km = 0.0;
  _examData.clicked_range_km = 0.0;
  _examData.nyq_vel = 0.0;
  _examData.log_dir = 0;
}


/**********************************************************************
 * _logDirActivated()
 */

void ExamineWindow::_logDirActivated()
{
  std::string log_dir = _logDirEntry.get_text();
  _origLogDir = log_dir;
  sxm_set_log_dir(log_dir);
  _loggingActiveToggle->set_active(true);
}


/**********************************************************************
 * _nyqVelActivated()
 */

void ExamineWindow::_nyqVelActivated()
{
  // Get the user-entered Nyquist velocity

  double nyq_vel = getNyqVelEntry();

  // Update the internal values.  We redisplay the nyquist velocity so that it
  // is formatted consistently

  setOrigNyqVel(nyq_vel);
  _setNyqVelEntry(nyq_vel);
  
  // Update the nyquist velocity used by the editor

  struct solo_edit_stuff *seds = return_sed_stuff();
  seds->nyquist_velocity = nyq_vel;
}


/**********************************************************************
 * _rangeActivated()
 */

void ExamineWindow::_rangeActivated()
{
  int height = _examineDisplayWindow->getLayoutHeight();

  // Get the entered range value

  double range = getRangeEntry();

  // Calculate the gate index for this range

  int gate_index = (gint)((range - _examData.r0_km) / _examData.gs_km + 0.5);
  if (gate_index < 0)
    gate_index = 0;

  // Get the number of cells being displayed

  int cells = height / _examineDisplayWindow->getLabelHeight();

  // We want to display as much data as possible, so if moving to this gate
  // would put us too close to the end of the beam, back up so that the display
  // window is filled.

  if (gate_index + cells >= _examData.max_cells)
    gate_index = _examData.max_cells - cells;
  
  // I don't understand this calculation.

  int value = (gate_index + 2) * _examineDisplayWindow->getLabelHeight() + 1;

  // Update the cell entry value

  _setCellEntry(gate_index);

  // Redisplay the range since we might have changed the gate index above

  _setRangeEntry(_examData.r0_km + (gate_index * _examData.gs_km));

  _examineDisplayWindow->setLayoutVadjValue(value);
}


/**********************************************************************
 * _rayActivated()
 */

void ExamineWindow::_rayActivated()
{
  sxm_refresh_list(_frameIndex);
}


/**********************************************************************
 * _raysActivated()
 */

void ExamineWindow::_raysActivated()
{
  sxm_refresh_list(_frameIndex);
}


/**********************************************************************
 * _refresh()
 */

void ExamineWindow::_refresh()
{
  sxm_refresh_list(_frameIndex);
  updateWidgets();
}


/**********************************************************************
 * _replotAll()
 */

void ExamineWindow::_replotAll()
{
  sii_plot_data(_frameIndex, REPLOT_ALL);
}


/**********************************************************************
 * _replotLinks()
 */

void ExamineWindow::_replotLinks()
{
  sii_plot_data(_frameIndex, REPLOT_LOCK_STEP);
}


/**********************************************************************
 * _replotThis()
 */

void ExamineWindow::_replotThis()
{
  sii_plot_data(_frameIndex, REPLOT_THIS_FRAME);
}


/**********************************************************************
 * _toggleLogging()
 */

void ExamineWindow::_toggleLogging()
{
  sxm_toggle_log_dir(_loggingActiveToggle->get_active());
}


/**********************************************************************
 * _undo()
 */

void ExamineWindow::_undo()
{
  if (solo_busy())
    return;

  solo_set_busy_signal();
  WW_PTR wwptr = solo_return_wwptr(_frameIndex);

  sxm_undo_last(_frameIndex);
  sxm_list_to_log(wwptr->examine_list);
  se_refresh_examine_widgets(_frameIndex, wwptr->examine_list);

  solo_clear_busy_signal();
}
