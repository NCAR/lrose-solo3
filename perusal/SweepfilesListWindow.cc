#include <iostream>

#include <solo2.hh>
#include <SweepfileWindow.hh>

#include "SweepfilesListWindow.hh"

/**********************************************************************
 * Constructor
 */

SweepfilesListWindow::SweepfilesListWindow(SweepfileWindow *parent,
					   const Pango::FontDescription &default_font) :
  Gtk::Window(),
  _parent(parent),
  _defaultFont(default_font),
  _sweepfileListView(1)
{
  // Create the fonts needed for the widgets

  Pango::FontDescription smaller_italic_font = _defaultFont;
  smaller_italic_font.set_size(_defaultFont.get_size() - (2 * 1024));
  smaller_italic_font.set_style(Pango::STYLE_ITALIC);
  
  // Construct the window title

  char display_string[1024];
  sprintf(display_string, "Frame %d  Sweepfiles List Widget",
	  _parent->getFrameIndex());
  
  set_title(display_string);
  set_border_width(0);
  
  // Set up the widgets

  _vbox.set_homogeneous(false);
  _vbox.set_spacing(2);
  add(_vbox);
  
  _helpLabel.set_text("(Double-click to select)");
  _helpLabel.set_justify(Gtk::JUSTIFY_CENTER);
  _helpLabel.modify_font(smaller_italic_font);
  _vbox.pack_start(_helpLabel, false, false, 0);
  
  _sweepfileListView.set_headers_visible(false);
  _sweepfileListView.modify_font(_defaultFont);

  // Set the padding for the list.  Note that the gtkmm-2.4 installed with
  // the Centos OS has a different API for the CellRenderer class so we can't
  // do this on that OS.
#ifndef OS_IS_CENTOS
  Gtk::CellRenderer *renderer = _sweepfileListView.get_column_cell_renderer(0);
  renderer->set_padding(0, 0);
#endif

  resetSweepfileList();
  _sweepfileListView.signal_row_activated().connect(sigc::mem_fun(*this,
								  &SweepfilesListWindow::_selectSweepfile));

  _sweepfileListViewWindow.set_policy(Gtk::POLICY_AUTOMATIC,
				      Gtk::POLICY_AUTOMATIC);
  _sweepfileListViewWindow.add(_sweepfileListView);
  _vbox.pack_start(_sweepfileListViewWindow, true, true, 0);

  set_size_request(500, 500);
}


/**********************************************************************
 * Destructor
 */

SweepfilesListWindow::~SweepfilesListWindow()
{
}


/**********************************************************************
 * resetSweepfileList()
 */

void SweepfilesListWindow::resetSweepfileList()
{
  std::vector< std::string > sweepfile_list =
    sii_return_swpfi_list(_parent->getFrameIndex());
  
  _sweepfileListView.clear_items();
  for (std::vector< std::string >::const_iterator sweepfile =
	 sweepfile_list.begin();
       sweepfile != sweepfile_list.end(); ++sweepfile)
    _sweepfileListView.append_text(*sweepfile);
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _selectSweepfile()
 */

void SweepfilesListWindow::_selectSweepfile(const Gtk::TreeModel::Path &path,
					    Gtk::TreeViewColumn *column)
{
  int row_number = atoi(path.to_string().c_str());
  std::string selected_sweepfile = _sweepfileListView.get_text(row_number);

  _parent->setInfo(row_number);
  if (_parent->isElectric())
    sii_plot_data(_parent->getFrameIndex(), REPLOT_LOCK_STEP);
  _parent->update();
}
