#include <iostream>
#include <math.h>

#include <se_utils.hh>
#include <sed_shared_structs.h>
#include <seds.h>
#include <sii_exam_widgets.hh>
#include <sii_utils.hh>
#include <solo_window_structs.h>
#include <sp_basics.hh>
#include <sxm_examine.hh>

#include <ExamineWindow.hh>

#include "ExamineDisplayWindow.hh"


/**********************************************************************
 * Constructor
 */

ExamineDisplayWindow::ExamineDisplayWindow(ExamineWindow *parent,
					   const Pango::FontDescription &default_font,
					   const int max_cells,
					   const int max_chars_per_line) :
  Gtk::Window(),
  _parent(parent),
  _defaultFont(default_font),
  _priorNumEntries(0),
  _labelWidth(0),
  _labelHeight(0),
  _lastListIndex(0),
  _lastCharIndex(0)
{
  // Create the arrays of objects for the lines in the display

  _eventBoxes = new Gtk::EventBox[max_cells];
  _labelItems = new Gtk::Label[max_cells];

  // Create the fonts needed for the widgets

  Pango::FontDescription monospace_font = _defaultFont;
  monospace_font.set_family("Monospace");
  
  // Set the events that this window will receive

  set_events(Gdk::BUTTON_PRESS_MASK | Gdk::KEY_PRESS_MASK);
  signal_key_release_event().connect(sigc::mem_fun(*this,
                                                   &ExamineDisplayWindow::_keyReleaseEvent));
  
  // Construct the window title

  char title_string[1024];
  sprintf(title_string, "Frame %d  Examine Display Widget",
	  _parent->getFrameIndex() + 1);
  set_title(title_string);
  set_border_width(0);
  
  // Set up the widgets

  _vbox00.set_homogeneous(false);
  _vbox00.set_spacing(4);
  add(_vbox00);
  
  // NOTE: These are two labels at the top of the window that create a blank
  // space at the top of the window, above the scrolling window.  These are
  // not set to include any text in the original code.  I'm wondering if these
  // were supposed to be the header lines.

  _labelBar0.set_text("");
  _labelBar0.modify_font(monospace_font);
  _vbox00.pack_start(_labelBar0, false, false, 0);
  _labelBar0.set_alignment(0.0, 0.5);
  
  _labelBar1.set_text("");
  _labelBar1.modify_font(monospace_font);
  _vbox00.pack_start(_labelBar1, false, false, 0);
  _labelBar1.set_alignment(0.0, 0.5);
  

  _vbox0.set_homogeneous(false);
  _vbox0.set_spacing(4);
  _vbox00.pack_start(_vbox0, true, true, 0);
  
  _scrolledWindow.modify_font(monospace_font);
  _vbox0.add(_scrolledWindow);
  
  // Calculate the font width/height so we can calculate the window size
  // later on.

  Glib::RefPtr< Pango::Context > pango_context =
    _scrolledWindow.create_pango_context();
  Pango::FontMetrics font_metrics =
    pango_context->get_metrics(monospace_font);
  
  _labelWidth =
    (int)(((double)font_metrics.get_approximate_digit_width() / 1000.0) + 0.5);
  _labelHeight =
    (int)(((double)(font_metrics.get_ascent() +
		    font_metrics.get_descent()) / 1000.0) + 0.5);

  int len = max_chars_per_line - 1;
  _scrolledWindow.add(_layout);
  _layout.set_size(len * _labelWidth, max_cells * _labelHeight);
  
  _layout.get_hadjustment()->set_step_increment(_labelHeight);
  _layout.get_vadjustment()->set_step_increment(_labelWidth);
  
  // Initialize the labels

  char str[256];
  str[len] = '\0';
  
  for (int i = 0; i < max_cells; ++i)
  {
    char ch = 'A' + (i % 26);
    sprintf(str, "%3d.", i);
    memset(str + strlen(str), ch, len - strlen(str));

    _eventBoxes[i].set_events(Gdk::BUTTON_PRESS_MASK | Gdk::KEY_PRESS_MASK);
    _eventBoxes[i].add(_labelItems[i]);
    _labelItems[i].modify_font(monospace_font);
    _labelItems[i].set_justify(Gtk::JUSTIFY_LEFT);
    _labelItems[i].set_alignment(0.0, 0.5);

    _eventBoxes[i].signal_button_press_event().connect(sigc::bind(sigc::mem_fun(*this,
										&ExamineDisplayWindow::_buttonPressEvent),
								  i));
    
    _layout.put(_eventBoxes[i], 0, i * _labelHeight);
  } /* endfor - i */

  resize(800, 1000);
}


/**********************************************************************
 * Destructor
 */

ExamineDisplayWindow::~ExamineDisplayWindow()
{
}


/**********************************************************************
 * refreshList()
 */

void ExamineDisplayWindow::refreshList(const std::vector< std::string > &examine_list,
				       const int max_cells,
				       const double clicked_range_km,
				       const double r0_km,
				       const double gs_km,
				       const display_state_t display_state)
{
  if (examine_list.size() == 0)
    return;

  // The first 2 entries in the examine_list are the column header labels.
  
  gint lim;
  if (((int)examine_list.size() - 2) <= max_cells)
  {
    lim = examine_list.size() - 2;
  }
  else
  {
    // NOTE:  What is the 8 for?

    lim = max_cells - 8;

    char str[256];
    sprintf(str, "Max lines:%d Lines needed:%d",
	    max_cells, (int)examine_list.size());
    sii_message(str);
  }

  // Set the column headers

  _labelBar0.set_text(examine_list[0]);
  _labelBar1.set_text(examine_list[1]);
  
  // Now set the data lines

  int jj;
  for (jj = 0; jj < lim; jj++)
  {
    std::string entry = examine_list[jj+2];

    _labelItems[jj].set_text(entry);
  }

  if (lim < _priorNumEntries)
  {
    for (; jj < _priorNumEntries; jj++)
      _labelItems[jj].set_text("");
  }
  _priorNumEntries = lim;

  if (clicked_range_km > 0 && display_state == DISPLAY_DATA)
  {
    // Shift data display to clicked range

    int height = _layout.get_allocation().get_height();
    int nn = height / _labelHeight; // lines visible
    jj =
      (int)((clicked_range_km - r0_km) / gs_km);
    jj -= nn / 2;
    int value = (jj + 2) * _labelHeight;
    if (value < 1 )
      value = 1;

    Gtk::Adjustment *adj = _layout.get_vadjustment();
    
    if (value + nn * _labelHeight > adj->get_upper())
      value = (int)(adj->get_upper() - nn * _labelHeight);
    
    adj->set_value(value);
  }

  _parent->setClickedRange(0.0);
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _buttonPressEvent()
 */

bool ExamineDisplayWindow::_buttonPressEvent(GdkEventButton *event_button,
					     const int label_index)
{
  // Get a pointer to the click information to share with other objects

  // If we aren't displaying data, then we don't want to do anything else

  if (_parent->getDisplayState() != DISPLAY_DATA)
    return true;
  
  // Record which line was clicked.  We need to add 2 to the clicked line
  // to take into account the column label lines which we have moved above
  // the scrolling window.

  int which_list_entry = label_index + 2;
  _lastListIndex = label_index;

  // Figure out exactly where the user clicked

  int xx;
  int yy;
  Gdk::ModifierType state;
  
  _labelItems[label_index].get_window()->get_pointer(xx, yy, state);
  
  _lastCharIndex = xx / _labelWidth;

  // Now process the click

  if (solo_busy())
    return true;

  solo_set_busy_signal();
  WW_PTR wwptr = solo_return_wwptr(_parent->getFrameIndex());

  _clickInList(which_list_entry, _lastCharIndex);
  sxm_list_to_log(wwptr->examine_list);
  se_refresh_examine_widgets(_parent->getFrameIndex(), wwptr->examine_list);

  solo_clear_busy_signal();

  g_message("Exam_list: f:%d i:%d c:%d x:%d y:%d s:%d",
	    _parent->getFrameIndex(), label_index, _lastCharIndex,
	    xx, yy, state);

  return true;
}

/**********************************************************************
 * _clickInList()
 */

void ExamineDisplayWindow::_clickInList(const int which_list_entry,
					const int which_character)
{
  // This routine is called when there is a EX_CLICK_IN_LIST

  int ii, jj, nn;
  int col, itsa_run=NO, row_num;
  int rrn = 0;
  struct se_changez *chz;
  struct solo_edit_stuff *seds;
  struct examine_control *ecs;
  WW_PTR wwptr, wwptrx;
  float *fptr, val, nyq_interval;
  struct ts_ray_table *tsrt;
  struct ts_ray_info *tsri;
  double nyqv;

  seds = return_sed_stuff();
  wwptr = solo_return_wwptr(_parent->getFrameIndex());
  wwptrx = wwptr->lead_sweep;
  tsrt = wwptrx->tsrt;
  ecs = wwptr->examine_control;
  sxm_get_widget_info(_parent->getFrameIndex());


  // Which column

  for (col = 0; col < ecs->num_cols; col++)
  {
    if (which_character < ecs->col_lims[col])
      break;
  }

  if (col == 0 || col == ecs->num_cols)
    return;

  // Skip the range annotation column

  col--;

  row_num = which_list_entry - ecs->heading_row_count;

  if (row_num < 0 || row_num >= ecs->actual_num_cells)
    return;

  switch (wwptr->examine_info->typeof_change)
  {
  case EX_RAY_PLUS_FOLD:
  case EX_RAY_MINUS_FOLD:
  case EX_RAY_IGNORE:
    if ((chz = wwptr->changez_list) &&
	chz->col_num == col && !chz->second_cell_num &&
	chz->row_num != row_num &&
	chz->typeof_change == wwptr->examine_info->typeof_change)
    {
      // The assumption here is when you click a second time
      // in the same column for one of the above options,
      // you're defining a run rather than selecting the whole beam

      itsa_run = YES;
    }
    break;
  default:
    break;
  }

  if (itsa_run)
  {
    if (row_num < chz->row_num)
    {
      chz->second_cell_num = ecs->actual_at_cell + chz->row_num +1;

      // We add one for the second cell num so differencing
      // the two cell numbers give the correct number of cells
      // in the run.

      chz->row_num = row_num;
      chz->cell_num = ecs->actual_at_cell + chz->row_num;
    }
    else
    {
      chz->second_cell_num = ecs->actual_at_cell + row_num +1;
    }
  }

  if (!itsa_run)
  {
    // Get a new change struct
    
    chz = sxm_pop_spair_change();
    chz->typeof_change = wwptr->examine_info->typeof_change;
    chz->col_num = col;
    chz->row_num = row_num;

    // Relative ray number

    rrn = col / ecs->actual_num_fields;
    tsri = tsrt->tsri_sxm + rrn;
    chz->ray_num = tsri->ray_num;
    chz->sweep_num = tsri->sweep_num;
    chz->field_num =
      ecs->actual_field_num[col % ecs->actual_num_fields];
    chz->cell_num = ecs->actual_at_cell + chz->row_num;
    chz->window_num = _parent->getFrameIndex();
  }

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(wwptr->lead_sweep->window_num);
  fptr = ecs->data_ptrs[chz->col_num] +chz->row_num;

  nyqv = seds->nyquist_velocity ? seds->nyquist_velocity :
    data_info->getNyquistVelocity();
  nyq_interval = 2. * nyqv;

  chz->f_old_val = *fptr;

  switch(chz->typeof_change)
  {
  case EX_MINUS_FOLD:
    *fptr -= nyq_interval;
    chz->f_new_val = *fptr;
    sxm_change_cell_in_list(chz, chz->row_num, *fptr);
    break;
  case EX_PLUS_FOLD:
    *fptr += nyq_interval;
    chz->f_new_val = *fptr;
    sxm_change_cell_in_list(chz, chz->row_num, *fptr);
    break;
  case EX_DELETE:
    *fptr = ecs->bad_data[col];
    chz->flagged_for_deletion = YES;
    chz->f_new_val = *fptr;
    sxm_change_cell_in_list(chz, chz->row_num, *fptr);
    break;
  case EX_REPLACE:
    *fptr = wwptr->examine_info->modify_val;
    chz->f_new_val = *fptr;
    sxm_change_cell_in_list(chz, chz->row_num, *fptr);
    break;
  case EX_REMOVE_AIR_MOTION:
    *fptr += ecs->ac_vel[rrn];
    for (ii = 0; fabs(*fptr) > nyqv && ii++ < 11;)
    {
      *fptr = *fptr < 0 ? *fptr + nyq_interval : *fptr - nyq_interval;
    }
    chz->f_new_val = *fptr;
    sxm_change_cell_in_list(chz, chz->row_num, *fptr);
    break;

  case EX_RAY_PLUS_FOLD:
  case EX_RAY_MINUS_FOLD:
  case EX_GT_PLUS_FOLD:
  case EX_GT_MINUS_FOLD:
  case EX_RAY_IGNORE:
    if (chz->typeof_change == EX_RAY_IGNORE)
    {
      chz->f_new_val = val = ecs->bad_data[chz->col_num];
      chz->flagged_for_deletion = YES;
    }
    else if (chz->typeof_change == EX_RAY_PLUS_FOLD ||
	     chz->typeof_change == EX_GT_PLUS_FOLD)
      val = nyq_interval;
    else
      val = -nyq_interval;

    fptr = ecs->data_ptrs[chz->col_num];
    nn = wwptr->examine_control->actual_num_cells;

    if (itsa_run)
    {
      // We need to but the parts of the ray
      // that are not in the run back the way
      // they were

      val = -val;
      jj = chz->row_num;

      for (ii = 0; ii < jj ; ii++, fptr++)
      {
	if (chz->typeof_change != EX_RAY_IGNORE &&
	    *fptr != ecs->bad_data[chz->col_num])
	  *fptr += val;

	sxm_change_cell_in_list(chz, ii, *fptr);
      }

      ii += chz->second_cell_num - chz->cell_num;
      fptr  += chz->second_cell_num - chz->cell_num;

      for (; ii < nn; ii++, fptr++)
      {
	if (chz->typeof_change != EX_RAY_IGNORE &&
	    *fptr != ecs->bad_data[chz->col_num])
	  *fptr += val;

	sxm_change_cell_in_list(chz, ii, *fptr);
      }
    }
    else
    {
      ii = 0;
      if (chz->typeof_change == EX_GT_PLUS_FOLD ||
	  chz->typeof_change == EX_GT_MINUS_FOLD)
      {
	ii = row_num;
	fptr += row_num;
      }

      for (; ii < nn ; ii++, fptr++)
      {
	if (chz->typeof_change == EX_RAY_IGNORE)
	{
	  sxm_change_cell_in_list(chz, ii, ecs->bad_data[chz->col_num]);
	}
	else
	{
	  if (*fptr != ecs->bad_data[chz->col_num])
	    *fptr += val;
	  sxm_change_cell_in_list(chz, ii, *fptr);
	}
      }
      chz->f_new_val = val;
    }
    break;			/* ray/run operations */

  default:
    break;
  }

  if (!itsa_run)
  {
    // Push this change onto the change list

    wwptr->examine_info->change_count++;
    sxm_push_change(chz, &wwptr->changez_list);
  }

}

/**********************************************************************
 * _keyReleaseEvent()
 */

bool ExamineDisplayWindow::_keyReleaseEvent(GdkEventKey *event_key)
{
  // Print out an informational message

  std::string key_string = "unk";

  if (gdk_keyval_name(event_key->keyval) != 0)
    key_string = gdk_keyval_name(event_key->keyval);
  g_message("Got keyboard event f:%d k:%d s:%s",
	    _parent->getFrameIndex(), event_key->keyval, key_string.c_str());
  
  // Process the key press

  switch (event_key->keyval)
  {
  case GDK_Right:
  {
    /* move one ray to the right */

    if (solo_busy())
      break;

    solo_set_busy_signal();
    WW_PTR wwptr = solo_return_wwptr(_parent->getFrameIndex());

    _scrollList(EX_SCROLL_RIGHT);
    sxm_list_to_log(wwptr->examine_list);

    solo_clear_busy_signal();
    break;
  }
  
  case GDK_Left:
  {
    /* move one ray to the left */

    if (solo_busy())
      break;

    solo_set_busy_signal();
    WW_PTR wwptr = solo_return_wwptr(_parent->getFrameIndex());

    _scrollList(EX_SCROLL_LEFT);
    sxm_list_to_log(wwptr->examine_list);

    solo_clear_busy_signal();
    break;
  }
  
  case GDK_Up:
    /* shift up one screen */
  case GDK_Down:
    /* shift down one screen */

    int height = _layout.get_allocation().get_height() - _labelHeight;

    Gtk::Adjustment *adj = _layout.get_vadjustment();
    int value = (int)adj->get_value();
    if (event_key->keyval == GDK_Up)
      value -= height;
    else
      value += height;

    if (value < 1)
      value = 1;

    if (value + height >= adj->get_upper())
      value = (int)(adj->get_upper() - height);
    
    adj->set_value(value);
    
    break;

  };

  return true;
}


/**********************************************************************
 * _scrollList()
 */

void ExamineDisplayWindow::_scrollList(const int which_widget_button)
{
  struct examine_control *ecs;
  WW_PTR wwptr;
  struct solo_examine_info *sei;
    
  wwptr = solo_return_wwptr(_parent->getFrameIndex());
  ecs = wwptr->examine_control;
  sei = wwptr->examine_info;
  sxm_get_widget_info(_parent->getFrameIndex());

  switch(wwptr->examine_info->whats_selected)
  {
  case EX_RADAR_DATA:
    switch (which_widget_button)
    {
    case EX_SCROLL_LEFT:
      break;
    case EX_SCROLL_RIGHT:
      break;
    case EX_SCROLL_UP:
      sei->at_cell -= sei->scroll_increment;
      break;
    case EX_SCROLL_DOWN:
      sei->at_cell += sei->scroll_increment;
      break;
    default:
      break;
    }
    sxm_update_examine_data(_parent->getFrameIndex(), which_widget_button);
    break;
  case EX_BEAM_INVENTORY:
    break;
  case EX_EDIT_HIST:
    break;
  case EX_DESCRIPTORS:
    break;
  default:
    break;
  }
}
