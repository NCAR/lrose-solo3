#ifndef SweepfileWindow_HH
#define SweepfileWindow_HH

#include <iostream>

#include <gtkmm/actiongroup.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/radioaction.h>
#include <gtkmm/table.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/window.h>

#include <SweepfilesListWindow.hh>

//#include <sii_utils.hh>
extern void sii_message (const Glib::ustring &message);

class SweepfileWindow : public Gtk::Window
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

  SweepfileWindow(const Pango::FontDescription &default_font,
		  const int frame_index);
  
  /**
   * @brief Destructor.
   */

  virtual ~SweepfileWindow();


  bool setInfo(const int sweep_num);
  
  /**
   * @brief Update the sweepfile information.
   */

  void update();
  

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
   * @brief Get the current value of the electric toggle.
   *
   * @return Returns true if the toggle is on, false otherwise.
   */

  bool isElectric() const
  {
    return _electricToggle->get_active();
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
  
  // NOTE: Check the use of these original values.  If we never restore things to
  // the original values, then we can remove them and simplify the code.

  std::string _currSweepfileOrig;
  std::string _directoryOrig;
  std::string _radarOrig;
  double _startTimeOrig;
  double _stopTimeOrig;
  std::string _scanModesOrig;
  double _fixedAngleOrig;
  double _toleranceOrig;
  
  // Menubar creation widgets //

  /**
   * @brief The user interface manager used for creating all of the menus.
   */

  Glib::RefPtr<Gtk::UIManager> _uiManager;

  /**
   * @brief The actions used by the menu items.
   */

  Glib::RefPtr<Gtk::ActionGroup> _actionGroup;

  Glib::RefPtr<Gtk::ToggleAction> _electricToggle;
   
  // Widgets //

  Gtk::VBox _vbox0;
  Gtk::VBox _vbox;
  
  Gtk::HButtonBox _hbbox;
  Gtk::Button _sweepsButton;     // SWPFI_LIST_SWPFIS
  Gtk::Button _replotButton;     // SWPFI_REPLOT_THIS
  Gtk::Button _okayButton;       // SWPFI_OK
  Gtk::Button _cancelButton;     // SWPFI_CANCEL
  
  Gtk::Label _currSweepfileLabel;    // SWPFI_CURRENT_SWPFI
  
  Gtk::Table _table;
  Gtk::Label _directoryLabel;
  Gtk::Entry _directoryEntry;    // SWPFI_DIR
  Gtk::Label _radarLabel;
  Gtk::Entry _radarEntry;        // SWPFI_RADAR_NAME
  Gtk::Label _startTimeLabel;
  Gtk::Entry _startTimeEntry;    // SWPFI_START_TIME
  Gtk::Label _stopTimeLabel;
  Gtk::Entry _stopTimeEntry;     // SWPFI_STOP_TIME
  Gtk::Label _scanModesLabel;
  Gtk::Label _fixedInfoLabel;
  Gtk::CheckButton _filterButton;   // SWPFI_FILTER
  Gtk::Entry _scanModesEntry;       // SWPFI_SCAN_MODES
  Gtk::Entry _fixedInfoEntry;       // SWPFI_FIXED_INFO
  Gtk::CheckButton _timeSyncButton; // SWPFI_TIME_SYNC
  
  
  // Child Windows //

  SweepfilesListWindow *_sweepfilesListWindow;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Create the menubar.
   *
   * @param[in] container   Container for the menubar.
   */

  void _createMenubar(Gtk::Box &container);
  
  void _getFilterInfo(std::string &scan_modes,
		      float &fixed_angle, float &tolerance);
  
  std::string _getSweepfileLabel() const;
  std::string _getSweepfileRadar() const;
  
  bool _getFixedInfoEntry(double &fixed_angle, double &tolerance) const
  {
    // Initialize the error message, in case we need it

    char error_message[1024];
    sprintf(error_message, "Unusable swpfi filter infor for frame %d", _frameIndex);
    
    // Get the user-entered string

    std::string fixed_info = _fixedInfoEntry.get_text();
    
    // Pull out the fixed angle

    std::size_t delim_pos = fixed_info.find_first_of(" \t,");
    if (delim_pos == std::string::npos)
    {
      sii_message(error_message);
      return false;
    }

    if (sscanf(fixed_info.substr(0, delim_pos).c_str(),
	       "%lf", &fixed_angle) != 1)
    {
      sii_message(error_message);
      return false;
    }
    
    // Pull out the tolerance

    if (sscanf(fixed_info.substr(delim_pos+1).c_str(), "%lf", &tolerance) != 1)
    {
      sii_message(error_message);
      return false;
    }
    
    return true;
  }
  
  void _setFixedInfoEntry(const double fixed_angle, const double tolerance)
  {
    char my_string[1024];
    sprintf(my_string, "%.2f,%.3f", fixed_angle, tolerance);
    _fixedInfoEntry.set_text(my_string);
  }
  

  void _setLockInfo();
  bool _setSweepInfo(const int sweep_num);
  

  // Callback methods //

  void _closeWindows();
  void _displayHelpFilter();
  void _displayHelpLinks();
  void _displayHelpLockstep();
  void _displayHelpOverview();
  void _displayHelpTimeLink();
  void _displayHelpTimes();
  void _entryActivated();
  void _listRadars();
  void _okay();
  void _replotAll();
  void _replotLinks();
  void _replotThis();
  void _setInfo();
  void _setLockstepLinks();
  void _setSweepfileLinks();
  void _showSweepsList();

  
};

#endif
