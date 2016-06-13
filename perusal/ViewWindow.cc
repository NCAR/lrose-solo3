#include <iostream>

#include <DateTime.hh>
#include <sii_externals.h>
#include <sii_links_widget.hh>
#include <solo2.hh>
#include <sp_accepts.hh>
#include <sp_basics.hh>
#include <sp_lists.hh>

#include "ViewWindow.hh"


/**********************************************************************
 * Constructor
 */

ViewWindow::ViewWindow(const Pango::FontDescription &default_font,
		       const int frame_index) :
  Gtk::Window(),
  _frameIndex(frame_index),
  _defaultFont(default_font),
  _active(false),
  _zoomOrig(0.0),
  _azOrig(0.0),
  _rangeOrig(0.0),
  _ringsRangeOrig(0.0),
  _ringsAzOrig(0.0),
  _angularFillOrig(0.0),
  _ticMarksXOrig(10.0),
  _ticMarksYOrig(10.0),
  _centerRangeOrig(0.0),
  _centerAzOrig(0.0),
  _centerLatOrig(0.0),
  _centerLonOrig(0.0),
  _centerAltOrig(0.0),
  _landmarkLatOrig(0.0),
  _landmarkLonOrig(0.0),
  _landmarkAltOrig(0.0),
  _table(2, 5, false),
  _table2(2, 9, false)
{
  // Initialize the links associated with this window

  frame_configs[_frameIndex]->link_set[LI_VIEW] = 
    sii_new_links_info("View Links", _frameIndex, FRAME_VIEW_LINKS, FALSE);
     
  frame_configs[_frameIndex]->link_set[LI_CENTER] = 
    sii_new_links_info("Center Links", _frameIndex, FRAME_CTR_LINKS, FALSE);
     
  frame_configs[_frameIndex]->link_set[LI_LANDMARK] = 
    sii_new_links_info("Landmark Links", _frameIndex, FRAME_LMRK_LINKS, FALSE);

  // Set the window title and border width

  char title_string[1024];
  
  sprintf(title_string, "Frame %d  View, Center & Landmark Widget",
	  _frameIndex + 1);
  set_title(title_string);

  set_border_width(0);
  
  // Create a new vertical box for storing widgets

  _vbox.set_homogeneous(false);
  _vbox.set_spacing(4);
  add(_vbox);
  
  // Create the menubar

  _createMenubar(_vbox);
  
  _hbox0.set_homogeneous(false);
  _hbox0.set_spacing(0);
  _vbox.add(_hbox0);
  
  _mainLabel.set_text("  View, Center & Landmark Settings   ");
  _mainLabel.modify_font(_defaultFont);
  _hbox0.pack_start(_mainLabel, true, true, 0);
  
//  _hbbox.set_spacing_default(4);
  _hbox0.pack_end(_hbbox, true, true, 0);
  
  _replotButton.set_label("Replot");
  _replotButton.modify_font(_defaultFont);
  _hbbox.pack_start(_replotButton, true, true, 0);
  _replotButton.signal_clicked().connect(sigc::mem_fun(*this,
                                                       &ViewWindow::_replotThisFrame));
  
  _okButton.set_label("OK");
  _okButton.modify_font(_defaultFont);
  _hbbox.pack_start(_okButton, true, true, 0);
  _okButton.signal_clicked().connect(sigc::mem_fun(*this,
						   &ViewWindow::_doIt));
  
  _cancelButton.set_label("Cancel");
  _cancelButton.modify_font(_defaultFont);
  _hbbox.pack_start(_cancelButton, true, true, 0);
  _cancelButton.signal_clicked().connect(sigc::mem_fun(*this,
						       &ViewWindow::_closeWindows));
  
  // Set an hbox to contain two vertical boxes of entries

  _hbox.set_homogeneous(false);
  _hbox.set_spacing(0);
  _vbox.add(_hbox);
  _hbox.set_border_width(4);
  
  _vbox2.set_homogeneous(false);
  _vbox2.set_spacing(0);
  _hbox.add(_vbox2);
  
  _viewSettingsLabel.set_text("View Settings");
  _viewSettingsLabel.modify_font(_defaultFont);
  _vbox2.add(_viewSettingsLabel);
  
  _vbox2.pack_start(_table, true, true, 0);
  
  int row = 0;
  _zoomLabel.set_text(" Zoom ");
  _zoomLabel.modify_font(_defaultFont);
  _table.attach(_zoomLabel, 0, 1, row, row+1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);
  
  _zoomEntry.modify_font(_defaultFont);
  _table.attach(_zoomEntry, 1, 2, row, row+1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);
  
  ++row;
  _ringsLabel.set_text(" Rings(km) & Spokes(deg) ");
  _ringsLabel.modify_font(_defaultFont);
  _table.attach(_ringsLabel, 0, 1, row, row+1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);
  
  _ringsEntry.modify_font(_defaultFont);
  _table.attach(_ringsEntry, 1, 2, row, row+1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);
  
  ++row;
  _ticMarksLabel.set_text(" X & Y Tic Marks (km) ");
  _ticMarksLabel.modify_font(_defaultFont);
  _table.attach(_ticMarksLabel, 0, 1, row, row+1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);
  
  _ticMarksEntry.modify_font(_defaultFont);
  _table.attach(_ticMarksEntry, 1, 2, row, row+1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);
  
  ++row;
  _azLabel.set_text(" Azimuth Labels (km) ");
  _azLabel.modify_font(_defaultFont);
  _table.attach(_azLabel, 0, 1, row, row+1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);
  
  _azEntry.modify_font(_defaultFont);
  _table.attach(_azEntry, 1, 2, row, row+1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);
  
  ++row;
  _rangeLabel.set_text(" Range Labels (deg) ");
  _rangeLabel.modify_font(_defaultFont);
  _table.attach(_rangeLabel, 0, 1, row, row+1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);
  
  _rangeEntry.modify_font(_defaultFont);
  _table.attach(_rangeEntry, 1, 2, row, row+1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);
  
  ++row;
  _angularFillLabel.set_text(" Angular Fill (%) ");
  _angularFillLabel.modify_font(_defaultFont);
  _table.attach(_angularFillLabel, 0, 1, row, row+1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);
  
  _angularFillEntry.modify_font(_defaultFont);
  _table.attach(_angularFillEntry, 1, 2, row, row+1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);
  
  _vbox3.set_homogeneous(false);
  _vbox3.set_spacing(0);
  _hbox.add(_vbox3);
  
  _hbox.pack_start(_table2, true, true, 0);
  
  row = 0;
  _centerRangeLabel.set_text(" Center Range (km) & Az ");
  _centerRangeLabel.modify_font(_defaultFont);
  _table2.attach(_centerRangeLabel, 0, 1, row, row+1,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 0, 0);
  
  _centerRangeEntry.modify_font(_defaultFont);
  _table2.attach(_centerRangeEntry, 1, 2, row, row+1,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 0, 0);
  
  ++row;
  _centerLatLonLabel.set_text(" Center Lat & Lon ");
  _centerLatLonLabel.modify_font(_defaultFont);
  _table2.attach(_centerLatLonLabel, 0, 1, row, row+1,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 0, 0);
  
  _centerLatLonEntry.modify_font(_defaultFont);
  _table2.attach(_centerLatLonEntry, 1, 2, row, row+1,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 0, 0);
  
  ++row;
  _centerAltLabel.set_text(" Center Altitude (km) ");
  _centerAltLabel.modify_font(_defaultFont);
  _table2.attach(_centerAltLabel, 0, 1, row, row+1,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 0, 0);

  _centerAltEntry.modify_font(_defaultFont);
  _table2.attach(_centerAltEntry, 1, 2, row, row+1,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 0, 0);
  
  ++row;
  _blank1Label.set_text(" ");
  _blank1Label.modify_font(_defaultFont);
  _table2.attach(_blank1Label, 0, 1, row, row+1,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 0, 0);
  
  ++row;
  _radarLocLabel.set_text("Radar Lat: 34.5678  Lon: -123.4567  Alt: 1.234");
  _radarLocLabel.modify_font(_defaultFont);
  _table2.attach(_radarLocLabel, 0, 2, row, row+1,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 0, 0);
  
  ++row;
  _blank2Label.set_text(" ");
  _blank2Label.modify_font(_defaultFont);
  _table2.attach(_blank2Label, 0, 1, row, row+1,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 0, 0);
  
  ++row;
  _landmarkRangeLabel.set_text(" Landmark Range & Az ");
  _landmarkRangeLabel.modify_font(_defaultFont);
  _table2.attach(_landmarkRangeLabel, 0, 1, row, row+1,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 0, 0);
  
  _landmarkRangeEntry.modify_font(_defaultFont);
  _landmarkRangeEntry.set_editable(false);
  _table2.attach(_landmarkRangeEntry, 1, 2, row, row+1,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 0, 0);
  
  ++row;
  _landmarkLatLonLabel.set_text(" Landmark Lat & Lon ");
  _landmarkLatLonLabel.modify_font(_defaultFont);
  _table2.attach(_landmarkLatLonLabel, 0, 1, row, row+1,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 0, 0);
  
  _landmarkLatLonEntry.modify_font(_defaultFont);
  _table2.attach(_landmarkLatLonEntry, 1, 2, row, row+1,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 0, 0);
  
  ++row;
  _landmarkAltLabel.set_text(" Landmark Altitude ");
  _landmarkAltLabel.modify_font(_defaultFont);
  _table2.attach(_landmarkAltLabel, 0, 1, row, row+1,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 0, 0);
  
  _landmarkAltEntry.modify_font(_defaultFont);
  _table2.attach(_landmarkAltEntry, 1, 2, row, row+1,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 0, 0);
}


/**********************************************************************
 * Destructor
 */

ViewWindow::~ViewWindow()
{
}


/**********************************************************************
 * getTicInfo()
 */

bool ViewWindow::getTicInfo(WW_PTR wwptr,
			    double &xinc, double &yinc,
			    bool &xtics, bool &ytics,
			    bool &annot)
{
  bool doit;

  xinc = wwptr->view->horiz_tic_mark_km;
  yinc = wwptr->view->vert_tic_mark_km;
  xtics = xinc > 0;
  ytics = yinc > 0;

  doit = xtics || ytics;

  if (xinc <= 0)
    xinc = _ticMarksXOrig;
  if (yinc <= 0)
    yinc = _ticMarksYOrig;

  annot = _xyTicLabelsToggle->get_active();

  return doit;
}


/**********************************************************************
 * update()
 */

void ViewWindow::update()
{
  // Get a pointer to the window information

  WW_PTR wwptr = solo_return_wwptr(_frameIndex);

  // Azimuth/range overlays

  if (wwptr->view->rng_ring_int_km <= 0 &&
      wwptr->view->az_line_int_deg <= 0)
    _overlayNoAzRngChoice->set_active(true);
  else if (wwptr->view->rng_ring_int_km <= 0)
    _overlayAzOnlyChoice->set_active(true);
  else if (wwptr->view->az_line_int_deg <= 0)
    _overlayRngOnlyChoice->set_active(true);
  else
    _overlayAzRngChoice->set_active(true);
  
  // Azimuth/range labels

  if (wwptr->view->az_annot_at_km <= 0 &&
      _overlayAzOnlyChoice->get_active())
    _azRangeLabelsToggle->set_active(false);
  else if (wwptr->view->rng_annot_at_deg < 0 &&
	   _overlayRngOnlyChoice->get_active())
    _azRangeLabelsToggle->set_active(false);
  else if (wwptr->view->az_annot_at_km <= 0 &&
	   wwptr->view->rng_annot_at_deg < 0)
    _azRangeLabelsToggle->set_active(false);
  else
    _azRangeLabelsToggle->set_active(true);
  
  // Magic range labels

  _magicRangeLabelsToggle->set_active(wwptr->magic_rng_annot);
  
  // Tick marks

  if (wwptr->view->horiz_tic_mark_km <= 0 &&
      wwptr->view->vert_tic_mark_km <= 0 )
    _noTicsChoice->set_active(true);
  else if (wwptr->view->horiz_tic_mark_km > 0 &&
	   wwptr->view->vert_tic_mark_km <= 0)
    _xOnlyTicsChoice->set_active(true);
  else if (wwptr->view->vert_tic_mark_km > 0 &&
	   wwptr->view->horiz_tic_mark_km <= 0)
    _yOnlyTicsChoice->set_active(true);
  else
    _xyTicsChoice->set_active(true);
  
  // Time series mode

  if (wwptr->view->type_of_plot & SOLO_TIME_SERIES)
    _timeSeriesModeToggle->set_active(true);

  // Time series plot direction

  if (wwptr->view->type_of_plot & TS_PLOT_RIGHT_TO_LEFT)
    _tsPlotRightLeftChoice->set_active(true);
  else
    _tsPlotLeftRightChoice->set_active(true);
  
  // Time series relative to

  if (wwptr->view->type_of_plot & TS_MSL_RELATIVE)
    _tsRelativeToMSLChoice->set_active(true);
  else
    _tsRelativeToRadarChoice->set_active(true);
  
  // Time series direction

  if (wwptr->view->type_of_plot & TS_PLOT_DOWN)
    _tsPointingDownChoice->set_active(true);
  else if (wwptr->view->type_of_plot & TS_AUTOMATIC)
    _tsPointingAutoChoice->set_active(true);
  else
    _tsPointingUpChoice->set_active(true);
  
  // Zoom

  _zoomOrig = wwptr->view->magnification;
  _setZoomEntry(_zoomOrig);
  
  // Azimuth lines/range rings

  if (wwptr->view->rng_ring_int_km > 0.0)
    _ringsRangeOrig = wwptr->view->rng_ring_int_km;
  if (wwptr->view->az_line_int_deg > 0.0)
    _ringsAzOrig = wwptr->view->az_line_int_deg;
  _setRingsEntry(_ringsRangeOrig, _ringsAzOrig);
  
  // Tick mark spacing

  if (wwptr->view->horiz_tic_mark_km > 0.0)
    _ticMarksXOrig = wwptr->view->horiz_tic_mark_km;
  if (wwptr->view->vert_tic_mark_km > 0.0)
    _ticMarksYOrig = wwptr->view->vert_tic_mark_km;
  _setTicMarksEntry(_ticMarksXOrig, _ticMarksYOrig);
  
  // Azimuth annotation distance

  if (wwptr->view->az_annot_at_km > 0.0)
    _azOrig = wwptr->view->az_annot_at_km;
  _setAzEntry(_azOrig);
  
  // Range annotation distance

  if (wwptr->view->rng_annot_at_deg > 0.0)
    _rangeOrig = wwptr->view->rng_annot_at_deg;
  _setRangeEntry(_rangeOrig);

  // Angular fill

  _angularFillOrig = wwptr->view->angular_fill_pct;
  _setAngularFillEntry(_angularFillOrig);

  // Radar location

  _setRadarLocLabel(wwptr->radar_location.getLatitude(),
		    wwptr->radar_location.getLongitude(),
		    wwptr->radar_location.getAltitude());
  
  // Centering

  switch (wwptr->frame_ctr_info->centering_options)
  {
  case SOLO_LOCAL_POSITIONING:
    _centerLocRadarChoice->set_active(true);
    break;

  case SOLO_FIXED_POSITIONING:
    _centerLocFixedChoice->set_active(true);
    break;

  case SOLO_LINKED_POSITIONING:
    _centerLocOtherChoice->set_active(true);
    break;
  };

  // Center range

  _centerRangeOrig = wwptr->center_of_view.getRange();
  _centerAzOrig = wwptr->center_of_view.getRotationAngle();
  _setCenterRangeEntry(_centerRangeOrig, _centerAzOrig);
  
  // Center lat/lon

  _centerLatOrig = wwptr->center_of_view.getLatitude();
  _centerLonOrig = wwptr->center_of_view.getLongitude();
  _setCenterLatLonEntry(_centerLatOrig, _centerLonOrig);
  
  // Center altitude

  _centerAltOrig = wwptr->center_of_view.getAltitude();
  _setCenterAltEntry(_centerAltOrig);
  

  // Landmark range/azimuth

  _setLandmarkRangeEntry(wwptr->landmark.getRange(),
			 wwptr->landmark.getAzimuth());
  
  // Landmark lat/lon

  _landmarkLatOrig = wwptr->landmark.getLatitude();
  _landmarkLonOrig = wwptr->landmark.getLongitude();
  _setLandmarkLatLonEntry(_landmarkLatOrig, _landmarkLonOrig);
  

  // Landmark altitude

  _landmarkAltOrig = wwptr->landmark.getAltitude();
  _setLandmarkAltEntry(_landmarkAltOrig);
  
  // Landmark location

  switch (wwptr->landmark_info->landmark_options)
  {
  case SOLO_LOCAL_POSITIONING:
    _landmarkLocRadarChoice->set_active(true);
    break;

  case SOLO_FIXED_POSITIONING:
    _landmarkLocFixedChoice->set_active(true);
    break;

  case SOLO_LINKED_POSITIONING:
    _landmarkLocOtherChoice->set_active(true);
    break;
  };

  // Set the links

  _setLinks(LI_VIEW, wwptr->view->linked_windows);
  _setLinks(LI_CENTER, wwptr->frame_ctr_info->linked_windows);
  _setLinks(LI_LANDMARK, wwptr->landmark_info->linked_windows);

}


/**********************************************************************
 * updateLinkedWidgets()
 */

void ViewWindow::updateLinkedWidgets()
{
  for (int jj = 0; jj < MAX_FRAMES; jj++)
  {
    if (!frame_configs[_frameIndex]->link_set[LI_VIEW]->link_set[jj])
      continue;

    ViewWindow *view_window = frame_configs[jj]->view_window;
    if (view_window != 0 && view_window->isActive())
    {
      view_window->_setInfo();
      view_window->update();
    }
  }
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _centerOnLast2Clicks()
 */

void ViewWindow::_centerOnLast2Clicks()
{
  sii_center_on_clicks((TASK_MODULO * _frameIndex) + 2);
}


/**********************************************************************
 * _centerOnLast4Clicks()
 */

void ViewWindow::_centerOnLast4Clicks()
{
  sii_center_on_clicks((TASK_MODULO * _frameIndex) + 4);
}


/**********************************************************************
 * _centerOnLastClick()
 */

void ViewWindow::_centerOnLastClick()
{
  sii_center_on_clicks((TASK_MODULO * _frameIndex) + 1);
}


/**********************************************************************
 * _centerOnRadar()
 */

void ViewWindow::_centerOnRadar()
{
  sii_center_on_clicks(TASK_MODULO * _frameIndex);
}


/**********************************************************************
 * _closeWindows()
 */

void ViewWindow::_closeWindows()
{
  // Close the link windows

  GtkWidget *widget;
  
  widget = frame_configs[_frameIndex]->toplevel_windows[FRAME_VIEW_LINKS];
  if (widget != 0)
    gtk_widget_hide(widget);

  widget = frame_configs[_frameIndex]->toplevel_windows[FRAME_CTR_LINKS];
  if (widget != 0)
    gtk_widget_hide(widget);

  widget = frame_configs[_frameIndex]->toplevel_windows[FRAME_LMRK_LINKS];
  if (widget != 0)
    gtk_widget_hide(widget);

  // Close this window

  hide();
}


/**********************************************************************
 * _createMenubar()
 */

void ViewWindow::_createMenubar(Gtk::Box &container)
{
  // Specify the XML for the menus

  Glib::ustring ui_info =
    "<ui>"
    "  <menubar name='MenuBar'>"
    "    <menu action='FileMenu'>"
    "      <menuitem action='FileClose'/>"
    "    </menu>"
    "    <menu action='OptionsMenu'>"
    "      <menuitem action='OptionsAzRngOverlay'/>"
    "      <menuitem action='OptionsNoAzRngOverlay'/>"
    "      <menuitem action='OptionsAzOnlyOverlay'/>"
    "      <menuitem action='OptionsRngOnlyOverlay'/>"
    "      <separator/>"
    "      <menuitem action='OptionsAzRngLabels'/>"
    "      <separator/>"
    "      <menuitem action='OptionsMagicRngLabels'/>"
    "      <separator/>"
    "      <menuitem action='OptionsXYTics'/>"
    "      <menuitem action='OptionsXOnlyTics'/>"
    "      <menuitem action='OptionsYOnlyTics'/>"
    "      <menuitem action='OptionsNoTics'/>"
    "      <separator/>"
    "      <menuitem action='OptionsXYTicLabels'/>"
    "    </menu>"
    "    <menu action='ViewLinksMenu'>"
    "      <menuitem action='ViewLinksSetLinks'/>"
    "    </menu>"
    "    <menu action='CenterMenu'>"
    "      <menu action='CenterOnMenu'>"
    "        <menuitem action='CenterOnLastClick'/>"
    "        <menuitem action='CenterOnLast2Clicks'/>"
    "        <menuitem action='CenterOnLast4Clicks'/>"
    "        <menuitem action='CenterOnRadar'/>"
    "      </menu>"
    "      <menu action='CenterLocMenu'>"
    "        <menuitem action='CenterLocRadar'/>"
    "        <menuitem action='CenterLocFixed'/>"
    "        <menuitem action='CenterLocOther'/>"
    "      </menu>"
    "      <menuitem action='CenterSetLinks'/>"
    "    </menu>"
    "    <menu action='LandmarkMenu'>"
    "      <menu action='LandmarkLocMenu'>"
    "        <menuitem action='LandmarkLocRadar'/>"
    "        <menuitem action='LandmarkLocFixed'/>"
    "        <menuitem action='LandmarkLocOther'/>"
    "      </menu>"
    "      <menuitem action='LandmarkSetLinks'/>"
    "    </menu>"
    "    <menu action='TimeSeriesMenu'>"
    "      <menuitem action='TimeSeriesMode'/>"
    "      <menu action='TimeSeriesPlotMenu'>"
    "        <menuitem action='TimeSeriesPlotLeftRight'/>"
    "        <menuitem action='TimeSeriesPlotRightLeft'/>"
    "      </menu>"
    "      <menu action='TimeSeriesRelativeToMenu'>"
    "        <menuitem action='TimeSeriesRelativeToRadar'/>"
    "        <menuitem action='TimeSeriesRelativeToMSL'/>"
    "      </menu>"
    "      <menu action='TimeSeriesPointingMenu'>"
    "        <menuitem action='TimeSeriesPointingUp'/>"
    "        <menuitem action='TimeSeriesPointingDown'/>"
    "        <menuitem action='TimeSeriesPointingAuto'/>"
    "      </menu>"
    "    </menu>"
    "    <menu action='ReplotMenu'>"
    "      <menuitem action='ReplotLinks'/>"
    "      <menuitem action='ReplotAll'/>"
    "    </menu>"
    "    <menu action='HelpMenu'>"
    "      <menuitem action='HelpOverview'/>"
    "      <menuitem action='HelpOptions'/>"
    "      <menuitem action='HelpLinks'/>"
    "      <menuitem action='HelpCenter'/>"
    "      <menuitem action='HelpLandmark'/>"
    "      <menuitem action='HelpTimeSeries'/>"
    "    </menu>"
    "  </menubar>"
    "</ui>";
  
  // Create the actions for the menus

  _actionGroup = Gtk::ActionGroup::create();
  
  // File menu actions

  _actionGroup->add(Gtk::Action::create("FileMenu", "File"));
  _actionGroup->add(Gtk::Action::create("FileClose", "Close"),
                    sigc::mem_fun(*this,
                                  &ViewWindow::_closeWindows));

  // Options menu actions

  _actionGroup->add(Gtk::Action::create("OptionsMenu", "Options"));

  Gtk::RadioAction::Group group_overlay;
  _overlayAzRngChoice = Gtk::RadioAction::create(group_overlay,
						 "OptionsAzRngOverlay",
						 "Az-Rng Overlay");
  _actionGroup->add(_overlayAzRngChoice);
  _overlayNoAzRngChoice = Gtk::RadioAction::create(group_overlay,
						   "OptionsNoAzRngOverlay",
						   "NO Az-Rng");
  _actionGroup->add(_overlayNoAzRngChoice);
  _overlayAzOnlyChoice = Gtk::RadioAction::create(group_overlay,
						  "OptionsAzOnlyOverlay",
						  "Azimuth Only");
  _actionGroup->add(_overlayAzOnlyChoice);
  _overlayRngOnlyChoice = Gtk::RadioAction::create(group_overlay,
						   "OptionsRngOnlyOverlay",
						   "Range Only");
  _actionGroup->add(_overlayRngOnlyChoice);

  _azRangeLabelsToggle = Gtk::ToggleAction::create("OptionsAzRngLabels",
						   "Az-Rng Labels", "",
						   true);
  _actionGroup->add(_azRangeLabelsToggle);

  _magicRangeLabelsToggle = Gtk::ToggleAction::create("OptionsMagicRngLabels",
						      "Magic Rng Lbls", "",
						      true);
  _actionGroup->add(_magicRangeLabelsToggle);

  Gtk::RadioAction::Group group_tic_overlay;
  _xyTicsChoice = Gtk::RadioAction::create(group_tic_overlay,
						 "OptionsXYTics",
						 "X-Y Tics Marks");
  _actionGroup->add(_xyTicsChoice);
  _xOnlyTicsChoice = Gtk::RadioAction::create(group_tic_overlay,
					      "OptionsXOnlyTics",
					      "X Tics Only");
  _actionGroup->add(_xOnlyTicsChoice);
  _yOnlyTicsChoice = Gtk::RadioAction::create(group_tic_overlay,
					      "OptionsYOnlyTics",
					      "Y Tics Only");
  _actionGroup->add(_yOnlyTicsChoice);
  _noTicsChoice = Gtk::RadioAction::create(group_tic_overlay,
					   "OptionsNoTics",
					   "NO X-Y Tics");
  _actionGroup->add(_noTicsChoice);
  _noTicsChoice->set_active(true);
  
  _xyTicLabelsToggle = Gtk::ToggleAction::create("OptionsXYTicLabels",
						 "X-Y Tic Labels", "",
						 true);
  _actionGroup->add(_xyTicLabelsToggle);

  // ViewLinks menu actions

  _actionGroup->add(Gtk::Action::create("ViewLinksMenu", "ViewLinks"));
  _actionGroup->add(Gtk::Action::create("ViewLinksSetLinks", "Set Links"),
                    sigc::mem_fun(*this,
                                  &ViewWindow::_showViewLinks));

  // Center menu actions

  _actionGroup->add(Gtk::Action::create("CenterMenu", "Center"));
  _actionGroup->add(Gtk::Action::create("CenterOnMenu",
                                        "Center On..."));
  _actionGroup->add(Gtk::Action::create("CenterOnLastClick", "Last click"),
                    sigc::mem_fun(*this,
                                  &ViewWindow::_centerOnLastClick));
  _actionGroup->add(Gtk::Action::create("CenterOnLast2Clicks", "Last 2 clicks"),
                    sigc::mem_fun(*this,
                                  &ViewWindow::_centerOnLast2Clicks));
  _actionGroup->add(Gtk::Action::create("CenterOnLast4Clicks", "Last 4 clicks"),
                    sigc::mem_fun(*this,
                                  &ViewWindow::_centerOnLast4Clicks));
  _actionGroup->add(Gtk::Action::create("CenterOnRadar", "Local Radar"),
                    sigc::mem_fun(*this,
                                  &ViewWindow::_centerOnRadar));

  _actionGroup->add(Gtk::Action::create("CenterLocMenu",
                                        "Location..."));
  Gtk::RadioAction::Group group_center_loc;
  _centerLocRadarChoice = Gtk::RadioAction::create(group_center_loc,
						   "CenterLocRadar",
						   "Local Radar");
  _actionGroup->add(_centerLocRadarChoice);
  _centerLocFixedChoice = Gtk::RadioAction::create(group_center_loc,
						   "CenterLocFixed",
						   "Fixed Lat Lon Alt");
  _actionGroup->add(_centerLocFixedChoice);
  _centerLocOtherChoice = Gtk::RadioAction::create(group_center_loc,
						   "CenterLocOther",
						   "Same as Frame 1");
  _actionGroup->add(_centerLocOtherChoice);

  _actionGroup->add(Gtk::Action::create("CenterSetLinks", "Set Links"),
                    sigc::mem_fun(*this,
                                  &ViewWindow::_showCenterLinks));

  // Landmark menu actions

  _actionGroup->add(Gtk::Action::create("LandmarkMenu", "Landmark"));
  _actionGroup->add(Gtk::Action::create("LandmarkLocMenu",
                                        "Location..."));
  Gtk::RadioAction::Group group_landmark_loc;
  _landmarkLocRadarChoice = Gtk::RadioAction::create(group_landmark_loc,
						     "LandmarkLocRadar",
						     "Local Radar");
  _actionGroup->add(_landmarkLocRadarChoice);
  _landmarkLocFixedChoice = Gtk::RadioAction::create(group_landmark_loc,
						     "LandmarkLocFixed",
						     "Fixed Lat Lon Alt");
  _actionGroup->add(_landmarkLocFixedChoice);
  _landmarkLocOtherChoice = Gtk::RadioAction::create(group_landmark_loc,
						     "LandmarkLocOther",
						     "Same as Frame 1");
  _actionGroup->add(_landmarkLocOtherChoice);

  _actionGroup->add(Gtk::Action::create("LandmarkSetLinks", "Set Links"),
                    sigc::mem_fun(*this,
                                  &ViewWindow::_showLandmarkLinks));

  // TimeSeries menu actions

  _actionGroup->add(Gtk::Action::create("TimeSeriesMenu", "TimeSeries"));
  _timeSeriesModeToggle = Gtk::ToggleAction::create("TimeSeriesMode",
						    "TimeSeries Mode", "",
						    false);
  _actionGroup->add(_timeSeriesModeToggle);

  _actionGroup->add(Gtk::Action::create("TimeSeriesPlotMenu",
                                        "Plot..."));
  Gtk::RadioAction::Group group_ts_plot;
  _tsPlotLeftRightChoice = Gtk::RadioAction::create(group_ts_plot,
						    "TimeSeriesPlotLeftRight",
						    "Left-Right");
  _actionGroup->add(_tsPlotLeftRightChoice);
  _tsPlotRightLeftChoice = Gtk::RadioAction::create(group_ts_plot,
						    "TimeSeriesPlotRightLeft",
						    "Right-Left");
  _actionGroup->add(_tsPlotRightLeftChoice);

  _actionGroup->add(Gtk::Action::create("TimeSeriesRelativeToMenu",
                                        "Relative to..."));
  Gtk::RadioAction::Group group_ts_relative_to;
  _tsRelativeToRadarChoice = Gtk::RadioAction::create(group_ts_relative_to,
						      "TimeSeriesRelativeToRadar",
						      "Radar");
  _actionGroup->add(_tsRelativeToRadarChoice);
  _tsRelativeToMSLChoice = Gtk::RadioAction::create(group_ts_relative_to,
						    "TimeSeriesRelativeToMSL",
						    "MSL");
  _actionGroup->add(_tsRelativeToMSLChoice);

  _actionGroup->add(Gtk::Action::create("TimeSeriesPointingMenu",
                                        "Pointing..."));
  Gtk::RadioAction::Group group_ts_pointing;
  _tsPointingUpChoice = Gtk::RadioAction::create(group_ts_pointing,
						 "TimeSeriesPointingUp",
						 "Up");
  _actionGroup->add(_tsPointingUpChoice);
  _tsPointingDownChoice = Gtk::RadioAction::create(group_ts_pointing,
						   "TimeSeriesPointingDown",
						   "Down");
  _actionGroup->add(_tsPointingDownChoice);
  _tsPointingAutoChoice = Gtk::RadioAction::create(group_ts_pointing,
						   "TimeSeriesPointingAuto",
						   "Automatic");
  _actionGroup->add(_tsPointingAutoChoice);

  // Replot menu actions

  _actionGroup->add(Gtk::Action::create("ReplotMenu", "Replot"));
  _actionGroup->add(Gtk::Action::create("ReplotLinks", "Linked Frames"),
                    sigc::mem_fun(*this,
                                  &ViewWindow::_replotLinks));
  _actionGroup->add(Gtk::Action::create("ReplotAll", "All Frames"),
                    sigc::mem_fun(*this,
                                  &ViewWindow::_replotAll));

  // Help menu actions

  _actionGroup->add(Gtk::Action::create("HelpMenu", "Help"));
  _actionGroup->add(Gtk::Action::create("HelpOverview", "Overview"),
                    sigc::mem_fun(*this,
                                  &ViewWindow::_displayHelpOverview));
  _actionGroup->add(Gtk::Action::create("HelpOptions", "Options"),
                    sigc::mem_fun(*this,
                                  &ViewWindow::_displayHelpOptions));
  _actionGroup->add(Gtk::Action::create("HelpLinks", "ViewLinks"),
                    sigc::mem_fun(*this,
                                  &ViewWindow::_displayHelpLinks));
  _actionGroup->add(Gtk::Action::create("HelpCenter", "Center"),
                    sigc::mem_fun(*this,
                                  &ViewWindow::_displayHelpCenter));
  _actionGroup->add(Gtk::Action::create("HelpLandmark", "Landmark"),
                    sigc::mem_fun(*this,
                                  &ViewWindow::_displayHelpLandmark));
  _actionGroup->add(Gtk::Action::create("HelpTimeSeries", "TimeSeries"),
                    sigc::mem_fun(*this,
                                  &ViewWindow::_displayHelpTimeSeries));

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
  {
    menubar->modify_font(_defaultFont);
    container.pack_start(*menubar,
                         false,      // expand
                         true,       // fill - ignored since expand is false
                         0);         // padding
  }
  
}


/**********************************************************************
 * _displayHelpCenter()
 */

void ViewWindow::_displayHelpCenter()
{
  static const Glib::ustring help_text =
    Glib::ustring("The \"Center\" submenu enables recentering based on previous clicks\n") +
    "in the data plus options to define how centering is done in this and\n" +
    "other frames.\n" +
    "\n" +
    "The default is to center on the local radar and be independent of what\n" +
    "is happening in other frames. It is possible to force all linked\n" +
    "frames to center on a fixed latitude and longitude or to force all\n" +
    "linked frames to center on a reference frame. Otherwise the reference\n" +
    "frame has no effect.\n" +
    "\n" +
    "The \"Set Links\" item brings up a links widget to define which frames\n" +
    "share this centering information.\n";
  
  sii_message(help_text);
}


/**********************************************************************
 * _displayHelpLandmark()
 */

void ViewWindow::_displayHelpLandmark()
{
  static const Glib::ustring help_text =
    Glib::ustring("The \"Landmark\" submenu enables referencing the landmark to locations\n") +
    "other than the radar. Referencing the radar is the default.\n" +
    "\n" +
    "It is possible to force all linked frames to reference a fixed\n" +
    "latitude and longitude or to force all linked frames to share the\n" +
    "landmark in a reference frame. Otherwise the reference frame has no\n" +
    "effect.\n" +
    "\n" +
    "The \"Set Links\" item brings up a links widget to define which frames\n" +
    "information.\n";
  
  sii_message(help_text);
}


/**********************************************************************
 * _displayHelpLinks()
 */

void ViewWindow::_displayHelpLinks()
{
  static const Glib::ustring help_text =
    Glib::ustring("The \"ViewLinks\" submenu brings up a links widget which identifies\n") +
    "which frames are to share this view information.\n";
  
  sii_message(help_text);
}


/**********************************************************************
 * _displayHelpOptions()
 */

void ViewWindow::_displayHelpOptions()
{
  static const Glib::ustring help_text =
    Glib::ustring("The \"Options\" submenu contains two sets of radio buttons and two\n") +
    "toggles. The first set is a choice amoung options for range rings and\n" +
    "azimuth spokes followed by a toggle to enable or disable labeling.\n" +
    "\n" +
    "The second set of radio buttons affects tic mark options.\n";

  sii_message(help_text);
}


/**********************************************************************
 * _displayHelpOverview()
 */

void ViewWindow::_displayHelpOverview()
{
  static const Glib::ustring help_text =
    Glib::ustring("The View widget enables adjustments to the zoom, shape and location\n") +
    "of the plot plus how, where and if range overlays are drawn.\n" +
    "\n" +
    "A zoom of 1.0 implies 300 meters/pixel (roughly 10 km. per cm. of screen).\n" +
    "\n" +
    "The location of the landmark determines the origin of range overlay\n" +
    "information.\n";

  sii_message(help_text);
}


/**********************************************************************
 * _displayHelpTimeSeries()
 */

void ViewWindow::_displayHelpTimeSeries()
{
  static const Glib::ustring help_text =
    Glib::ustring("The \"TimeSeries\" submenu enables color bscan plots based on\n") +
    "time. Each pixel along the horizontal is assigned a constant interval\n" +
    "of time which enables compression or expansion of time plus revealing\n" +
    "gaps in the data.\n" +
    "\n" +
    "You have to click \"OK\" or one of the Replot options to get the\n" +
    "time series parameters activated.\n" +
    "\n" +
    "Start and stop times are set in the \"Sweepfiles\" widget. A relative\n" +
    "time in the \"Start Time\" entry shifts the start of the plot. A\n" +
    "relative time in the \"Stop Time\" entry can be used to define the\n" +
    "time span.\n" +
    "\n" +
    "The default is to plot increasing time from left to right relative to\n" +
    "the indicated times with the range increasing upward.\n" +
    "\n" +
    "For aircraft data it is possible to fix the center of the plot on an\n" +
    "msl altitude and shift the data such the data bin closest to this\n" +
    "altitude is at the center of the screen.\n" +
    "\n" +
    "In flights where the instrument's fixed angle is changing between nadir\n" +
    "and zenith, the \"Automatic\" option changes increasing range to be\n" +
    "either up or down depending on the fixed angle of the instrument.\n";
  
  sii_message(help_text);
}


/**********************************************************************
 * _doIt()
 */

void ViewWindow::_doIt()
{
  _closeWindows();

  _setInfo();
  updateLinkedWidgets();
}


/**********************************************************************
 * _replotAll()
 */

void ViewWindow::_replotAll()
{
  if (_setInfo())
    sii_plot_data(_frameIndex, REPLOT_ALL);
  updateLinkedWidgets();
}


/**********************************************************************
 * _replotLinks()
 */

void ViewWindow::_replotLinks()
{
  if (_setInfo())
    sii_plot_data(_frameIndex, REPLOT_LOCK_STEP);
  updateLinkedWidgets();
}


/**********************************************************************
 * _replotThisFrame()
 */

void ViewWindow::_replotThisFrame()
{
  if (_setInfo())
    sii_plot_data(_frameIndex, REPLOT_THIS_FRAME);
  updateLinkedWidgets();
}


/**********************************************************************
 * _setInfo()
 */

bool ViewWindow::_setInfo()
{
  ///////////////
  // View info //
  ///////////////

  struct view_widget_info vwi;
  memset (&vwi, 0, sizeof (vwi));

  // Frame number

  vwi.frame_num = _frameIndex;

  // Plot type

  vwi.type_of_plot = 0;
  
  bool time_series = _timeSeriesModeToggle->get_active();

  if (time_series)
    vwi.type_of_plot |= SOLO_TIME_SERIES;

  if (_tsPlotRightLeftChoice->get_active())
    vwi.type_of_plot |= TS_PLOT_RIGHT_TO_LEFT;

  if (_tsRelativeToMSLChoice->get_active())
    vwi.type_of_plot |= TS_MSL_RELATIVE;

  if (_tsPointingDownChoice->get_active())
    vwi.type_of_plot |= TS_PLOT_DOWN;

  if (_tsPointingAutoChoice->get_active())
    vwi.type_of_plot |= TS_AUTOMATIC;

  // Zoom

  double zoom;
  if (_getZoomEntry(zoom))
    _zoomOrig = zoom;
  vwi.magnification = _zoomOrig;
  vwi.ts_magnification = _zoomOrig;

  // Angular fill

  double angular_fill_pct;
  if (_getAngularFillEntry(angular_fill_pct))
    _angularFillOrig = angular_fill_pct;
  vwi.angular_fill_pct = _angularFillOrig;

  // Azimuth/range ring values

  double range_ring_int_km;
  double az_line_int_deg;
  if (_getRingsEntry(range_ring_int_km, az_line_int_deg))
  {
    _ringsRangeOrig = range_ring_int_km;
    _ringsAzOrig = az_line_int_deg;
  }
    
  if (time_series)
  {
    vwi.az_line_int_deg = -1.0;
    vwi.rng_ring_int_km = -1.0;
  }
  else if (_overlayAzRngChoice->get_active())
  {
    vwi.rng_ring_int_km = _ringsRangeOrig;
    vwi.az_line_int_deg = _ringsAzOrig;
  }
  else if (_overlayNoAzRngChoice->get_active())
  {
    vwi.az_line_int_deg = -1.0;
    vwi.rng_ring_int_km = -1.0;
  }
  else if (_overlayAzOnlyChoice->get_active())
  {
    vwi.az_line_int_deg = _ringsAzOrig;
    vwi.rng_ring_int_km = -1.0;
  }
  else
  {
    vwi.rng_ring_int_km = _ringsRangeOrig;
    vwi.az_line_int_deg = -1.0;
  }

  // Azimuth/range labels

  vwi.az_annot_at_km = -1.0;
  vwi.rng_annot_at_deg = -1.0;

  if (_azRangeLabelsToggle->get_active())
  {
    double az_annot_at_deg;
    if (_getAzEntry(az_annot_at_deg))
      _azOrig = az_annot_at_deg;
    vwi.az_annot_at_km = _azOrig;
    
    double range_annot_at_deg;
    if (_getRangeEntry(range_annot_at_deg))
      _rangeOrig = range_annot_at_deg;
    vwi.rng_annot_at_deg = _rangeOrig;
  }

  // Magic range labels flag

  if (_magicRangeLabelsToggle->get_active())
    vwi.magic_rng_annot = 1;

  // Tic marks

  double x_tic_mark_km;
  double y_tic_mark_km;
  if (_getTicMarksEntry(x_tic_mark_km, y_tic_mark_km))
  {
    _ticMarksXOrig = x_tic_mark_km;
    _ticMarksYOrig = y_tic_mark_km;
  }
  
  if (time_series || _xyTicsChoice->get_active())
  {
    vwi.horiz_tic_mark_km = _ticMarksXOrig;
    vwi.vert_tic_mark_km = _ticMarksYOrig;
  }
  else if (_noTicsChoice->get_active())
  {
    vwi.horiz_tic_mark_km = 0.0;
    vwi.vert_tic_mark_km = 0.0;
  }
  else if (_xOnlyTicsChoice->get_active())
  {
    vwi.horiz_tic_mark_km = _ticMarksXOrig;
    vwi.vert_tic_mark_km = 0.0;
  }
  else
  {
    vwi.horiz_tic_mark_km = 0.0;
    vwi.vert_tic_mark_km = _ticMarksYOrig;
  }

  // View links -- make sure this window is linked

  for (int jj = 0; jj < MAX_FRAMES; jj++)
    vwi.linked_windows[jj] =
      (frame_configs[_frameIndex]->link_set[LI_VIEW]->link_set[jj]) ? 1 : 0;
  vwi.linked_windows[_frameIndex] = 1;


  ////////////////////
  // Centering info //
  ////////////////////

  struct centering_widget_info cwi;
  memset (&cwi, 0, sizeof (cwi));

  // Frame number

  cwi.frame_num = _frameIndex;

  // Reference frame

  cwi.reference_frame = 1;

  // Center location flag

  cwi.options = SOLO_LOCAL_POSITIONING;
  if (_centerLocOtherChoice->get_active())
    cwi.options = SOLO_LINKED_POSITIONING;
  else if (_centerLocFixedChoice->get_active())
    cwi.options = SOLO_FIXED_POSITIONING;

  // Center range/azimuth

  double center_range;
  double center_az;
  if (_getCenterRangeEntry(center_range, center_az))
  {
    _centerRangeOrig = center_range;
    _centerAzOrig = center_az;
  }

  cwi.rng_of_ctr = _centerRangeOrig;
  vwi.ts_ctr_km = _centerRangeOrig;
  cwi.az_of_ctr = _centerAzOrig;

  // Center lat/lon

  double center_lat;
  double center_lon;
  if (_getCenterLatLonEntry(center_lat, center_lon))
  {
    _centerLatOrig = center_lat;
    _centerLonOrig = center_lon;
  }
  
  cwi.latitude = _centerLatOrig;
  cwi.longitude = _centerLonOrig;

  // Center altitude

  double center_alt;
  if (_getCenterAltEntry(center_alt))
    _centerAltOrig = center_alt;
  
  cwi.altitude = _centerAltOrig;

  // Center links

  for (int jj = 0; jj < MAX_FRAMES; jj++)
    cwi.linked_windows[jj] =
      (frame_configs[_frameIndex]->link_set[LI_CENTER]->link_set[jj]) ? 1 : 0;


  ///////////////////
  // Landmark info //
  ///////////////////

  struct landmark_widget_info lwi;
  memset (&lwi, 0, sizeof (lwi));

  // Frame number

  lwi.frame_num = _frameIndex;

  // Reference frame

  lwi.reference_frame = 1;

  // Landmark location

  lwi.options = SOLO_LOCAL_POSITIONING;
  if (_landmarkLocOtherChoice->get_active())
    lwi.options = SOLO_LINKED_POSITIONING;
  else if (_landmarkLocFixedChoice->get_active())
    lwi.options = SOLO_FIXED_POSITIONING;

  // Landmark lat/lon

  double landmark_lat;
  double landmark_lon;
  if (_getLandmarkLatLonEntry(landmark_lat, landmark_lon))
  {
    _landmarkLatOrig = landmark_lat;
    _landmarkLonOrig = landmark_lon;
  }
  
  lwi.latitude = _landmarkLatOrig;
  lwi.longitude = _landmarkLonOrig;

  // Landmark altitude

  double landmark_alt;
  if (_getLandmarkAltEntry(landmark_alt))
    _landmarkAltOrig = landmark_alt;
  
  lwi.altitude = _landmarkAltOrig;

  // Landmark links

  for (int jj = 0; jj < MAX_FRAMES; jj++)
    lwi.linked_windows[jj] =
      (frame_configs[_frameIndex]->link_set[LI_LANDMARK]->link_set[jj]) ? 1 : 0;

  // Update the window frames based on the view information

  // NOTE:  This is the only place solo_set_view_info() is called so maybe
  // it should be integrated into this code.

  return solo_set_view_info(vwi, lwi, cwi) == 0;
}


/**********************************************************************
 * _setLinks()
 */

void ViewWindow::_setLinks(int links_id, int32_t *linked_windows)
{
  LinksInfo *li = frame_configs[_frameIndex]->link_set[links_id];

  for (int jj = 0; jj < MAX_FRAMES; ++jj)
    li->link_set[jj] = (linked_windows[jj]) ? TRUE : FALSE;
}


/**********************************************************************
 * _showCenterLinks()
 */

void ViewWindow::_showCenterLinks()
{
  show_links_widget(frame_configs[_frameIndex]->link_set[LI_VIEW]); 
}


/**********************************************************************
 * _showLandmarkLinks()
 */

void ViewWindow::_showLandmarkLinks()
{
  show_links_widget(frame_configs[_frameIndex]->link_set[LI_LANDMARK]); 
}


/**********************************************************************
 * _showViewLinks()
 */

void ViewWindow::_showViewLinks()
{
  show_links_widget(frame_configs[_frameIndex]->link_set[LI_VIEW]); 
}
