#ifndef ViewWindow_HH
#define ViewWindow_HH

#include <iostream>

#include <gtkmm/actiongroup.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/radioaction.h>
#include <gtkmm/table.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/window.h>

#include <solo_window_structs.h>

class ViewWindow : public Gtk::Window
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   *
   * @param[in] default_font   The default font to use for the widgets.
   * @param[in] frame_num      The index of the assocated frame.
   */

  ViewWindow(const Pango::FontDescription &default_font,
	     const int frame_index);
  
  /**
   * @brief Destructor.
   */

  virtual ~ViewWindow();


  bool getTicInfo(WW_PTR wwptr,
		  double &xinc, double &yinc,
		  bool &xtics, bool &ytics,
		  bool &annot);
  
  void update();
  
  void updateLinkedWidgets();
  

  ////////////////////
  // Access methods //
  ////////////////////

  /**
   * @brief Get the frame index for this window.
   *
   * @return Returns the frame index for this window.
   */

  int getFrameIndex() const
  {
    return _frameIndex;
  }
  
  
  /**
   * @brief Set the active flag on the window.
   *
   * @param[in] active_flag    The new state of the window.
   */

  void setActive(const bool active_flag)
  {
    _active = active_flag;
  }
  

  /**
   * @brief Check the active flag.
   *
   * @return Returns true if the window is active, false otherwise.
   */

  bool isActive() const
  {
    return _active;
  }
  

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief The index for the associated frame.
   */

  int _frameIndex;
  
  /**
   * @brief The default font to use for rendering.
   */

  Pango::FontDescription _defaultFont;
  
  /**
   * @brief Active flag.  This is set when the window is activated, which
   *        happens the first time it is displayed.  This was added to fix
   *        a problem that was causing bad interaction between the windows
   *        when updates were being done on windows that haven't been used
   *        yet.  I think this will be able to go away when the frame_configs
   *        stuff is refactored.
   */

  bool _active;
  

  double _zoomOrig;
  double _azOrig;
  double _rangeOrig;
  double _ringsRangeOrig;
  double _ringsAzOrig;
  double _angularFillOrig;
  double _ticMarksXOrig;
  double _ticMarksYOrig;
  double _centerRangeOrig;
  double _centerAzOrig;
  double _centerLatOrig;
  double _centerLonOrig;
  double _centerAltOrig;
  double _landmarkLatOrig;
  double _landmarkLonOrig;
  double _landmarkAltOrig;
  
  // Menubar creation widgets //

  /**
   * @brief The user interface manager used for creating all of the menus.
   */

  Glib::RefPtr<Gtk::UIManager> _uiManager;

  /**
   * @brief The actions used by the menu items.
   */

  Glib::RefPtr<Gtk::ActionGroup> _actionGroup;

  Glib::RefPtr<Gtk::RadioAction> _overlayAzRngChoice;
  Glib::RefPtr<Gtk::RadioAction> _overlayNoAzRngChoice;
  Glib::RefPtr<Gtk::RadioAction> _overlayAzOnlyChoice;
  Glib::RefPtr<Gtk::RadioAction> _overlayRngOnlyChoice;

  Glib::RefPtr<Gtk::ToggleAction> _azRangeLabelsToggle;
  Glib::RefPtr<Gtk::ToggleAction> _magicRangeLabelsToggle;
   
  Glib::RefPtr<Gtk::RadioAction> _xyTicsChoice;
  Glib::RefPtr<Gtk::RadioAction> _xOnlyTicsChoice;
  Glib::RefPtr<Gtk::RadioAction> _yOnlyTicsChoice;
  Glib::RefPtr<Gtk::RadioAction> _noTicsChoice;

  Glib::RefPtr<Gtk::ToggleAction> _xyTicLabelsToggle;

  Glib::RefPtr<Gtk::RadioAction> _centerLocRadarChoice;
  Glib::RefPtr<Gtk::RadioAction> _centerLocFixedChoice;
  Glib::RefPtr<Gtk::RadioAction> _centerLocOtherChoice;

  Glib::RefPtr<Gtk::RadioAction> _landmarkLocRadarChoice;
  Glib::RefPtr<Gtk::RadioAction> _landmarkLocFixedChoice;
  Glib::RefPtr<Gtk::RadioAction> _landmarkLocOtherChoice;

  Glib::RefPtr<Gtk::ToggleAction> _timeSeriesModeToggle;

  Glib::RefPtr<Gtk::RadioAction> _tsPlotLeftRightChoice;
  Glib::RefPtr<Gtk::RadioAction> _tsPlotRightLeftChoice;

  Glib::RefPtr<Gtk::RadioAction> _tsRelativeToRadarChoice;
  Glib::RefPtr<Gtk::RadioAction> _tsRelativeToMSLChoice;

  Glib::RefPtr<Gtk::RadioAction> _tsPointingUpChoice;
  Glib::RefPtr<Gtk::RadioAction> _tsPointingDownChoice;
  Glib::RefPtr<Gtk::RadioAction> _tsPointingAutoChoice;

  // Widgets //

  Gtk::VBox _vbox;
  Gtk::HBox _hbox0;
  Gtk::Label _mainLabel;

  Gtk::HButtonBox _hbbox;
  Gtk::Button _replotButton;   // VIEW_REPLOT_THIS
  Gtk::Button _okButton;       // VIEW_OK
  Gtk::Button _cancelButton;   // VIEW_CANCEL
  
  Gtk::HBox _hbox;
  Gtk::VBox _vbox2;
  Gtk::Label _viewSettingsLabel;
  
  Gtk::Table _table;
  Gtk::Label _zoomLabel;
  Gtk::Entry _zoomEntry;       // VIEW_ZOOM
  Gtk::Label _ringsLabel;
  Gtk::Entry _ringsEntry;      // VIEW_VALUES_AZRNG
  Gtk::Label _ticMarksLabel;
  Gtk::Entry _ticMarksEntry;   // VIEW_VALUES_XYTICS
  Gtk::Label _azLabel;
  Gtk::Entry _azEntry;         // VIEW_AZ_LBL_RNG
  Gtk::Label _rangeLabel;
  Gtk::Entry _rangeEntry;      // VIEW_RNG_LBL_AZ
  Gtk::Label _angularFillLabel;
  Gtk::Entry _angularFillEntry;   // VIEW_ANG_FILL
  
  Gtk::VBox _vbox3;
  Gtk::Table _table2;
  Gtk::Label _centerRangeLabel;
  Gtk::Entry _centerRangeEntry;   // VIEW_CTR_RNG_AZ
  Gtk::Label _centerLatLonLabel;
  Gtk::Entry _centerLatLonEntry;  // VIEW_CTR_LAT_LON
  Gtk::Label _centerAltLabel;
  Gtk::Entry _centerAltEntry;     // VIEW_CTR_ALT
  Gtk::Label _blank1Label;
  Gtk::Label _radarLocLabel;      // VIEW_RADAR_LAT_LON_ALT
  Gtk::Label _blank2Label;
  Gtk::Label _landmarkRangeLabel;
  Gtk::Entry _landmarkRangeEntry; // VIEW_LMRK_RNG_AZ
  Gtk::Label _landmarkLatLonLabel;
  Gtk::Entry _landmarkLatLonEntry;  // VIEW_LMRK_LAT_LON
  Gtk::Label _landmarkAltLabel;
  Gtk::Entry _landmarkAltEntry;     // VIEW_LMRK_ALT
  
  
  // Child Windows //


  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Create the menubar.
   *
   * @param[in] container   Container for the menubar.
   */

  void _createMenubar(Gtk::Box &container);
  
  bool _setInfo();
  
  void _setLinks(int links_id, int32_t *linked_windows);
  

  // Widget access methods //

  bool _getAngularFillEntry(double &angular_fill_pct) const
  {
    std::string display_string = _angularFillEntry.get_text();
    
    if (sscanf(display_string.c_str(), "%lf", &angular_fill_pct) != 1)
      return false;
    
    return true;
  }
  
  void _setAngularFillEntry(const double angular_fill_pct)
  {
    char display_string[80];
    sprintf(display_string, "%.1f", angular_fill_pct);

    _angularFillEntry.set_text(display_string);
  }
  
  bool _getAzEntry(double &az_annot_at_km) const
  {
    std::string display_string = _azEntry.get_text();
    
    if (sscanf(display_string.c_str(), "%lf", &az_annot_at_km) != 1)
      return false;
    
    return true;
  }
  
  void _setAzEntry(const double az_annot_at_km)
  {
    char display_string[80];
    sprintf(display_string, "%.1f", az_annot_at_km);

    _azEntry.set_text(display_string);
  }
  
  bool _getCenterAltEntry(double &alt) const
  {
    std::string display_string = _centerAltEntry.get_text();
    
    if (sscanf(display_string.c_str(), "%lf", &alt) != 1)
      return false;
    
    return true;
  }
  
  void _setCenterAltEntry(const double alt)
  {
    char display_string[80];
    sprintf(display_string, "%.3f", alt);

    _centerAltEntry.set_text(display_string);
  }
  
  bool _getCenterLatLonEntry(double &lat, double &lon) const
  {
    std::string display_string = _centerLatLonEntry.get_text();
    
    std::size_t nonwhite_pos = display_string.find_first_not_of(" \t");
    if (nonwhite_pos != std::string::npos)
      display_string = display_string.substr(nonwhite_pos);
    
    std::size_t delim_pos = display_string.find_first_of(" \t,");
    if (delim_pos == std::string::npos)
      return false;
    
    if (sscanf(display_string.substr(0, delim_pos).c_str(), "%lf",
	       &lat) != 1)
      return false;
    
    display_string = display_string.substr(delim_pos);
    display_string =
      display_string.substr(display_string.find_first_not_of(" \t,"));
    
    if (sscanf(display_string.c_str(), "%lf", &lon) != 1)
      return false;
    
    return true;
  }
  
  void _setCenterLatLonEntry(const double lat, const double lon)
  {
    char display_string[80];
    sprintf(display_string, "%.4f, %.4f", lat, lon);

    _centerLatLonEntry.set_text(display_string);
  }
  
  bool _getCenterRangeEntry(double &range, double &az) const
  {
    std::string display_string = _centerRangeEntry.get_text();
    
    if (display_string.find_first_not_of(" ") == std::string::npos)
    {
      range = 0.0;
      az = 0.0;
      return true;
    }
    
    std::size_t nonwhite_pos = display_string.find_first_not_of(" \t");
    if (nonwhite_pos != std::string::npos)
      display_string = display_string.substr(nonwhite_pos);
    
    std::size_t delim_pos = display_string.find_first_of(" \t,");
    if (delim_pos == std::string::npos)
      return false;
    
    if (sscanf(display_string.substr(0, delim_pos).c_str(), "%lf",
	       &range) != 1)
      return false;
    
    display_string = display_string.substr(delim_pos);
    display_string =
      display_string.substr(display_string.find_first_not_of(" \t,"));
    
    if (sscanf(display_string.c_str(), "%lf", &az) != 1)
      return false;
    
    return true;
  }
  
  void _setCenterRangeEntry(const double range, const double az)
  {
    char display_string[80];
    sprintf(display_string, "%.3f, %.3f", range, az);

    _centerRangeEntry.set_text(display_string);
  }
  
  bool _getLandmarkAltEntry(double &alt) const
  {
    std::string display_string = _landmarkAltEntry.get_text();
    
    if (sscanf(display_string.c_str(), "%lf", &alt) != 1)
      return false;
    
    return true;
  }
  
  void _setLandmarkAltEntry(const double alt)
  {
    char display_string[80];
    sprintf(display_string, "%.3f", alt);

    _landmarkAltEntry.set_text(display_string);
  }
  
  bool _getLandmarkLatLonEntry(double &lat, double &lon) const
  {
    std::string display_string = _landmarkLatLonEntry.get_text();
    
    std::size_t nonwhite_pos = display_string.find_first_not_of(" \t");
    if (nonwhite_pos != std::string::npos)
      display_string = display_string.substr(nonwhite_pos);
    
    std::size_t delim_pos = display_string.find_first_of(" \t,");
    if (delim_pos == std::string::npos)
      return false;
    
    if (sscanf(display_string.substr(0, delim_pos).c_str(), "%lf",
	       &lat) != 1)
      return false;
    
    display_string = display_string.substr(delim_pos);
    display_string =
      display_string.substr(display_string.find_first_not_of(" \t,"));
    
    if (sscanf(display_string.c_str(), "%lf", &lon) != 1)
      return false;
    
    return true;
  }
  
  void _setLandmarkLatLonEntry(const double lat, const double lon)
  {
    char display_string[80];
    sprintf(display_string, "%.4f, %.4f", lat, lon);

    _landmarkLatLonEntry.set_text(display_string);
  }
  
  void _setLandmarkRangeEntry(const double range, const double az)
  {
    char display_string[80];
    sprintf(display_string, "%.3f, %.3f", range, az);

    _landmarkRangeEntry.set_text(display_string);
  }
  
  void _setRadarLocLabel(const double radar_lat, const double radar_lon,
			 const double radar_alt)
  {
    char display_string[80];
    sprintf(display_string, "Radar Lat: %.4f  Lon: %.4f  Alt: %.3f",
	    radar_lat, radar_lon, radar_alt);

    _radarLocLabel.set_text(display_string);
  }
  
  bool _getRangeEntry(double &range_annot_at_deg) const
  {
    std::string display_string = _rangeEntry.get_text();
    
    if (sscanf(display_string.c_str(), "%lf", &range_annot_at_deg) != 1)
      return false;
    
    return true;
  }
  
  void _setRangeEntry(const double range_annot_at_deg)
  {
    char display_string[80];
    sprintf(display_string, "%.1f", range_annot_at_deg);

    _rangeEntry.set_text(display_string);
  }
  
  bool _getRingsEntry(double &range_ring_int_km,
		      double &az_line_int_deg) const
  {
    std::string display_string = _ringsEntry.get_text();
    
    std::size_t nonwhite_pos = display_string.find_first_not_of(" \t");
    if (nonwhite_pos != std::string::npos)
      display_string = display_string.substr(nonwhite_pos);
    
    std::size_t delim_pos = display_string.find_first_of(" \t,");
    if (delim_pos == std::string::npos)
      return false;
    
    if (sscanf(display_string.substr(0, delim_pos).c_str(), "%lf",
	       &range_ring_int_km) != 1)
      return false;
    
    display_string = display_string.substr(delim_pos);
    display_string =
      display_string.substr(display_string.find_first_not_of(" \t,"));
    
    if (sscanf(display_string.c_str(), "%lf", &az_line_int_deg) != 1)
      return false;
    
    return true;
  }
  
  void _setRingsEntry(const double range_ring_int_km,
		      const double az_line_int_deg)
  {
    char display_string[80];
    sprintf(display_string, "%.1f, %.1f", range_ring_int_km, az_line_int_deg);

    _ringsEntry.set_text(display_string);
  }
  
  bool _getTicMarksEntry(double &x_tic_mark_km,
			 double &y_tic_mark_km) const
  {
    std::string display_string = _ticMarksEntry.get_text();
    
    std::size_t nonwhite_pos = display_string.find_first_not_of(" \t");
    if (nonwhite_pos != std::string::npos)
      display_string = display_string.substr(nonwhite_pos);
    
    std::size_t delim_pos = display_string.find_first_of(" \t,");
    if (delim_pos == std::string::npos)
      return false;
    
    if (sscanf(display_string.substr(0, delim_pos).c_str(), "%lf",
	       &x_tic_mark_km) != 1)
      return false;
    
    display_string = display_string.substr(delim_pos);
    display_string =
      display_string.substr(display_string.find_first_not_of(" \t,"));
    
    if (sscanf(display_string.c_str(), "%lf", &y_tic_mark_km) != 1)
      return false;
    
    return true;
  }
  
  void _setTicMarksEntry(const double x_tic_mark_km,
			 const double y_tic_mark_km)
  {
    char display_string[80];
    sprintf(display_string, "%.1f, %.1f", x_tic_mark_km, y_tic_mark_km);

    _ticMarksEntry.set_text(display_string);
  }
  
  bool _getZoomEntry(double &zoom) const
  {
    std::string display_string = _zoomEntry.get_text();
    
    if (sscanf(display_string.c_str(), "%lf", &zoom) != 1)
      return false;
    
    return true;
  }
  
  void _setZoomEntry(const double zoom)
  {
    char display_string[80];
    sprintf(display_string, "%.3f", zoom);

    _zoomEntry.set_text(display_string);
  }
  

  // Callback methods //

  void _centerOnLast2Clicks();
  void _centerOnLast4Clicks();
  void _centerOnLastClick();
  void _centerOnRadar();
  void _closeWindows();
  void _displayHelpCenter();
  void _displayHelpLandmark();
  void _displayHelpLinks();
  void _displayHelpOptions();
  void _displayHelpOverview();
  void _displayHelpTimeSeries();
  void _doIt();
  void _replotAll();
  void _replotLinks();
  void _replotThisFrame();
  void _showCenterLinks();
  void _showLandmarkLinks();
  void _showViewLinks();
  
  
};

#endif
