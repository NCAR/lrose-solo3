#ifndef EditWindow_HH
#define EditWindow_HH

#include <iostream>

#include <gtkmm/actiongroup.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/listviewtext.h>
#include <gtkmm/radioaction.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/table.h>
#include <gtkmm/textview.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/window.h>

#include <EditCmdListHelpWindow.hh>
#include <EditFilesWindow.hh>

class EditWindow : public Gtk::Window
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

  EditWindow(const Pango::FontDescription &default_font,
	     const int frame_index);
  
  /**
   * @brief Destructor.
   */

  virtual ~EditWindow();


  /**
   * @brief Add the given commands to the user command buffers.
   *
   * @param[in] command_list      The list of commands.
   */

  void addCommands(const std::vector< std::string > &command_list);
  

  /**
   * @brief Get the user entered start and stop times, check that they are valid and
   *        update the given swp_file_input_control structure with their values.
   *
   * @param[in,out] sfic  The structure to update.
   *
   * @return Returns true on success, false on failure.
   */

  bool editStartStop(struct swp_file_input_control *sfic);
  

  /**
   * @brief Reset the widget start and stop times based on the current
   *        window information.
   */

  void resetTimes();
  

  ////////////////////
  // Access methods //
  ////////////////////

  /**
   * @brief Get the frame index for this window.
   *
   * @return Returns the frame index for this window.
   */

  inline int getFrameIndex() const
  {
    return _frameIndex;
  }
  
  inline double getStartTime() const
  {
    return _startTime;
  }
  
  inline double getStopTime() const
  {
    return _stopTime;
  }
  
  inline bool isFrameActive() const
  {
    return _frameActive;
  }
  
  inline void setFrameActive(const bool active_flag)
  {
    _frameActive = active_flag;
  }
  
  inline bool isDrawBoundaries() const
  {
    return _drawBoundariesToggle->get_active();
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
   * @brief Flag indicating whether the frame is active.
   */

  bool _frameActive;
  
  /**
   * @brief The original value of the sweep file directory field.
   */

  std::string _origSweepfileDir;
  
  /**
   * @brief The original value of the boundary directory field.
   */

  std::string _origBoundaryDir;
  
  /**
   * @brief The original value of the llb directory field.
   */

  std::string _origLlbDir;
  
  /**
   * @brief The original value of the start time field.
   */

  std::string _origStartTime;
  
  /**
   * @brief The original value of the stop time field.
   */

  std::string _origStopTime;
  
  /**
   * @brief The command start time.
   */

  double _startTime;

  /**
   * @brief The command stop time.
   */

  double _stopTime;

  // Menubar creation widgets //

  /**
   * @brief The user interface manager used for creating all of the menus.
   */

  Glib::RefPtr<Gtk::UIManager> _uiManager;

  /**
   * @brief The actions used by the menu items.
   */

  Glib::RefPtr<Gtk::ActionGroup> _actionGroup;

  Glib::RefPtr<Gtk::RadioAction> _boundaryEditInsideChoice;
  Glib::RefPtr<Gtk::RadioAction> _boundaryEditOutsideChoice;
  
  Glib::RefPtr<Gtk::ToggleAction> _autoReplotToggle;
  Glib::RefPtr<Gtk::ToggleAction> _drawBoundariesToggle;


  // Widgets //

  Gtk::VBox _vbox;   // Contains all of the widgets
  
  Gtk::HBox _hbox;   // Contains the top part of the window, everything except
                     //   the start/stop time widgets

  Gtk::VBox _buttonsBox;  // Contains the table of buttons on the left side
  Gtk::Table _buttonsTable; // Contains the buttons on the left side
  Gtk::Button _clearBndButton;
  Gtk::Button _okButton;
  Gtk::Button _addNextBndButton;
  Gtk::Button _prevBndSetButton;
  
  Gtk::Table _cmdsTable;  // Contains all of the cmd list widgets
  
  Gtk::HBox _hbox1;  // Contains the each ray cmds label and clear button
  Gtk::Label _eachRayCmdsLabel;
  Gtk::Button _clearEachRayButton;
  Gtk::TextView _userEachRayTextView;
  Gtk::Label _eachRayListLabel;
  Gtk::ListViewText _eachRayCmdList;
  Gtk::ScrolledWindow _eachRayCmdListScrolledWindow;
  
  Gtk::HBox _hbox2;  // Contains the one time cmds label and clear button
  Gtk::Label _oneTimeCmdsLabel;
  Gtk::Button _clearOneTimeButton;
  Gtk::TextView _userOneTimeTextView;
  Gtk::Label _oneTimeListLabel;
  Gtk::ListViewText _oneTimeCmdList;
  Gtk::ScrolledWindow _oneTimeCmdListScrolledWindow;
  
  Gtk::Label _cmdsHelpLabel;
  
  Gtk::Table _startStopTable;   // Contains start/stop time widgets
  Gtk::Label _startTimeLabel;
  Gtk::Entry _startTimeEntry;
  Gtk::Button _firstSweepButton;
  Gtk::Label _stopTimeLabel;
  Gtk::Entry _stopTimeEntry;
  Gtk::Button _lastSweepButton;
  

  // Child windows //
  
  EditCmdListHelpWindow *_editCmdListHelpWindow;
  EditFilesWindow *_editFilesWindow;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Create the menubar.
   *
   * @param[in] container   Container for the menubar.
   */

  void _createMenubar(Gtk::Box &container);
  

  /**
   * @brief Fill in the user command areas with the given command example.
   *
   * @param[in] command_list       The list of commands.
   */

  void _displayExample(const std::vector< std::string > &command_list);
  

  // Callback methods //

  void _addNextBoundary();
  void _changeBoundaryEditState();
  void _clearForEachRayCmds();
  void _clearOneTimeCmds();
  void _clearBoundaries();
  void _closeWindows();
  void _displayHelpBoundaries();
  void _displayHelpFile();
  void _displayHelpOverview();
  void _displayHelpShortcuts();
  void _displayHelpStartStop();
  void _doIt();
  void _firstSweep();
  void _lastSweep();
  void _replotLinks();
  void _selectEachRayCmd(const Gtk::TreeModel::Path &path,
			 Gtk::TreeViewColumn *column);
  void _selectOneTimeCmd(const Gtk::TreeModel::Path &path,
			 Gtk::TreeViewColumn *column);
  void _setPreviousBoundary();
  void _showBargenBrownUnfoldingExample();
  void _showEditCmdHelpWidget();
  void _showEditFilesWidget();
  void _showFlagFrecklesExample();
  void _showFlagGlitchesExample();
  void _showIrregularHistogramExample();
  void _showRadialShearExample();
  void _showRegularHistogramExample();
  void _showThresholdingExample();
  void _startTimeActivated();
  void _stopTimeActivated();
  

};

#endif
