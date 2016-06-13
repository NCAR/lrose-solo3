#ifndef ExamineWindow_HH
#define ExamineWindow_HH

#include <iostream>
#include <stdlib.h>

#include <gdk/gdk.h>
#include <gtkmm/actiongroup.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/radioaction.h>
#include <gtkmm/table.h>
#include <gtkmm/toggleaction.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/window.h>

#include <ExamineDisplayWindow.hh>
#include <seds.h>

class ExamineWindow : public Gtk::Window
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

  ExamineWindow(const Pango::FontDescription &default_font,
	     const int frame_index);
  
  /**
   * @brief Destructor.
   */

  virtual ~ExamineWindow();


  void refreshExamineList(const std::vector< std::string > &examine_list);
  

  /**
   * @brief Update the widgets to display the original values.
   */

  void updateWidgets();


  // Access methods //

  /**
   * @brief Get the frame index for this window.
   *
   * @return Returns the frame index for this window.
   */

  int getFrameIndex() const
  {
    return _frameIndex;
  }
  

  int getWhatsSelected() const
  {
    switch (_examData.display_state)
    {
    case ExamineDisplayWindow::DISPLAY_DATA :
      return EX_RADAR_DATA;
    case ExamineDisplayWindow::DISPLAY_RAYS :
      return EX_BEAM_INVENTORY;
    case ExamineDisplayWindow::DISPLAY_METADATA :
      return EX_DESCRIPTORS;
    case ExamineDisplayWindow::DISPLAY_EDIT_HISTORY :
      return EX_EDIT_HIST;
    }
  }
  

  int getTypeOfChange() const
  {
    switch (_examData.operation_state)
    {
    case OPERATION_DELETE :
      return EX_DELETE;
    case OPERATION_NEG_FOLD :
      return EX_MINUS_FOLD;
    case OPERATION_POS_FOLD :
      return EX_PLUS_FOLD;
    case OPERATION_ZAP_GND_SPD :
      return EX_REMOVE_AIR_MOTION;
    case OPERATION_DELETE_RAY :
      return EX_RAY_IGNORE;
    case OPERATION_NEG_FOLD_RAY :
      return EX_RAY_MINUS_FOLD;
    case OPERATION_POS_FOLD_RAY :
      return EX_RAY_PLUS_FOLD;
    case OPERATION_NEG_FOLD_GT :
      return EX_GT_MINUS_FOLD;
    case OPERATION_POS_FOLD_GT :
      return EX_GT_PLUS_FOLD;
    }

    // Should never get here

    return EX_DELETE;
  }


  // Access methods -- widgets //

  int getCellEntry() const
  {
    std::string entry_text = _cellEntry.get_text();
    return atoi(entry_text.c_str());
  }
  
  std::string getFieldsEntry() const
  {
    return _fieldsEntry.get_text();
  }
  
  std::string getFormatEntry() const
  {
    return _formatEntry.get_text();
  }
  
  double getNyqVelEntry() const
  {
    std::string entry_text = _nyqVelEntry.get_text();
    return atof(entry_text.c_str());
  }
  
  double getRangeEntry() const
  {
    std::string entry_text = _rangeEntry.get_text();
    return atof(entry_text.c_str());
  }
  
  double getRayEntry() const
  {
    std::string entry_text = _rayEntry.get_text();
    return atoi(entry_text.c_str());
  }
  
  double getRaysEntry() const
  {
    std::string entry_text = _raysEntry.get_text();
    return atoi(entry_text.c_str());
  }
  
  bool isLabelCellRay() const
  {
    return _optionsCellRayLabelsChoice->get_active();
  }
  

  // Access methods -- ExamData //

  bool isFrameActive() const
  {
    return _examData.frame_active;
  }
  
  ExamineDisplayWindow::display_state_t getDisplayState() const
  {
    return _examData.display_state;
  }
  
  void setMaxCells(const int max_cells)
  {
    _examData.max_cells = max_cells;
  }
  
  void setStartRange(const double r0_km)
  {
    _examData.r0_km = r0_km;
  }
  
  void setClickedRange(const double clicked_range_km)
  {
    _examData.clicked_range_km = clicked_range_km;
  }
  
  void setGateSpacing(const double gs_km)
  {
    _examData.gs_km = gs_km;
  }
  
  void setFrameActive(const bool active_flag)
  {
    _examData.frame_active = active_flag;
  }
  
  // Access methods -- original values //

  void setOrigRay(const int ray)
  {
    _origRay = ray;
  }
  
  void setOrigRays(const int rays)
  {
    _origRays = rays;
  }
  
  void setOrigChanges(const int changes)
  {
    _origChanges = changes;
  }
  
  void setOrigCell(const int cell)
  {
    _origCell = cell;
  }
  
  void setOrigRange(const double range)
  {
    _origRange = range;
  }
  
  void setOrigFields(const std::string &fields)
  {
    _origFields = fields;
  }
  
  void setOrigFormat(const std::string &format)
  {
    _origFormat = format;
  }
  
  void setOrigNyqVel(const double nyq_vel)
  {
    _origNyqVel = nyq_vel;
  }
  

protected:

  /////////////////////
  // Protected types //
  /////////////////////

  typedef enum
  {
    OPERATION_DELETE,
    OPERATION_NEG_FOLD,
    OPERATION_POS_FOLD,
    OPERATION_ZAP_GND_SPD,
    OPERATION_DELETE_RAY,
    OPERATION_NEG_FOLD_RAY,
    OPERATION_POS_FOLD_RAY,
    OPERATION_NEG_FOLD_GT,
    OPERATION_POS_FOLD_GT
  } operation_state_t;

  typedef struct
  {
    bool frame_active;
    ExamineDisplayWindow::display_state_t display_state;
    operation_state_t operation_state;

    int max_cells;
    int max_possible_cells;
    int max_chars_per_line;

    double r0_km;
    double gs_km;
    double clicked_range_km;
    double nyq_vel;

    GString *log_dir;
  } ExamData_t;
  

  /////////////////////////
  // Protected constants //
  /////////////////////////

  /**
   * @brief Template for the text that goes in this window.
   */

  static const std::string _clickTemplate;
  
  /**
   * @brief Text to display when there isn't a current data selection.
   */

  static const std::string _sampleClickText;
  

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
   * @brief Information about the window.  This should go away in the
   *        refactoring.
   */

  ExamData_t _examData;
  

  // Menubar creation widgets //

  /**
   * @brief The user interface manager used for creating all of the menus.
   */

  Glib::RefPtr<Gtk::UIManager> _uiManager;

  // The original field values

  std::string _origFields;
  std::string _origFormat;
  int _origChanges;
  double _origRange;
  double _origNyqVel;
  std::string _origLogDir;
  
  int _origRay;
  int _origRays;
  int _origCell;
  
  /**
   * @brief The actions used by the menu items.
   */

  Glib::RefPtr<Gtk::ActionGroup> _actionGroup;

  Glib::RefPtr<Gtk::RadioAction> _displayCellValuesChoice;
  Glib::RefPtr<Gtk::RadioAction> _displayRayInfoChoice;
  Glib::RefPtr<Gtk::RadioAction> _displayMetadataChoice;
  Glib::RefPtr<Gtk::RadioAction> _displayEditHistChoice;

  Glib::RefPtr<Gtk::RadioAction> _editDeleteChoice;
  Glib::RefPtr<Gtk::RadioAction> _editMinusFoldChoice;
  Glib::RefPtr<Gtk::RadioAction> _editPlusFoldChoice;
  Glib::RefPtr<Gtk::RadioAction> _editDeleteRayChoice;
  Glib::RefPtr<Gtk::RadioAction> _editMinusFoldRayChoice;
  Glib::RefPtr<Gtk::RadioAction> _editPlusFoldRayChoice;
  Glib::RefPtr<Gtk::RadioAction> _editMinusFoldRayGTChoice;
  Glib::RefPtr<Gtk::RadioAction> _editPlusFoldRayGTChoice;
  Glib::RefPtr<Gtk::RadioAction> _editZapGndSpdChoice;

  Glib::RefPtr<Gtk::RadioAction> _optionsAzRngLabelsChoice;
  Glib::RefPtr<Gtk::RadioAction> _optionsCellRayLabelsChoice;
  
  Glib::RefPtr<Gtk::ToggleAction> _loggingActiveToggle;
  
  // Widgets //

  Gtk::VBox _vbox0;
  Gtk::Table _table;
  
  Gtk::Label _fieldsLabel;
  Gtk::Entry _fieldsEntry;
  
  Gtk::Label _formatLabel;
  Gtk::Entry _formatEntry;
  
  Gtk::Label _rayLabel;
  Gtk::Entry _rayEntry;
  
  Gtk::Label _changesLabel;
  Gtk::Entry _changesEntry;
  
  Gtk::Label _raysLabel;
  Gtk::Entry _raysEntry;
  
  Gtk::Label _rangeLabel;
  Gtk::Entry _rangeEntry;
  
  Gtk::Label _cellLabel;
  Gtk::Entry _cellEntry;

  Gtk::Label _nyqVelLabel;
  Gtk::Entry _nyqVelEntry;
  Gtk::Label _nyqVelHelpLabel;
  
  Gtk::Label _logDirLabel;
  Gtk::Entry _logDirEntry;
  
  Gtk::HBox _hbox0;
  Gtk::Button _clearEditsButton;
  Gtk::Button _undoButton;
  Gtk::Button _applyEditsButton;
  Gtk::Button _refreshButton;
  

  // Child windows //

  ExamineDisplayWindow *_examineDisplayWindow;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  void _applyChanges();
  void _createMenubar(Gtk::Box &container);
  void _initExamData();
  

  // Access methods -- widgets //

  void _setChangesEntry(const int changes)
  {
    char text[80];
    
    sprintf(text, "%d", changes);
    _changesEntry.set_text(text);
  }
  
  void _setCellEntry(const int cell)
  {
    char text[80];
    
    sprintf(text, "%d", cell);
    _cellEntry.set_text(text);
  }
  
  void _setNyqVelEntry(const double nyq_vel)
  {
    char text[80];
    
    if (nyq_vel == 0.0)
      sprintf(text, "0");
    else
      sprintf(text, "%.3f", nyq_vel);

    _nyqVelEntry.set_text(text);
  }
  
  void _setRangeEntry(const double range)
  {
    char text[80];
    
    sprintf(text, "%.2f", range);
    _rangeEntry.set_text(text);
  }
  
  void _setRayEntry(const int ray)
  {
    char text[80];
    
    sprintf(text, "%d", ray);
    _rayEntry.set_text(text);
  }
  
  void _setRaysEntry(const int rays)
  {
    char text[80];
    
    sprintf(text, "%d", rays);
    _raysEntry.set_text(text);
  }
  

  // Callback methods //

  void _applyEdits();
  void _azRangeLabels();
  void _cancel();
  void _cellActivated();
  void _cellRayLabels();
  void _clearEdits();
  void _closeLogFile();
  void _displayCellValues();
  void _displayEditHistory();
  void _displayHelpEdit();
  void _displayHelpLogDir();
  void _displayHelpNyqVel();
  void _displayHelpOptions();
  void _displayHelpOverview();
  void _displayMetadata();
  void _displayRayInfo();
  void _editDelete();
  void _editDeleteRay();
  void _editMinusFold();
  void _editMinusFoldRay();
  void _editMinusFoldRayGT();
  void _editPlusFold();
  void _editPlusFoldRay();
  void _editPlusFoldRayGT();
  void _editZapGroundSpeed();
  void _fieldsActivated();
  void _flushLogFile();
  void _formatActivated();
  void _logDirActivated();
  void _nyqVelActivated();
  void _rangeActivated();
  void _rayActivated();
  void _raysActivated();
  void _refresh();
  void _replotAll();
  void _replotLinks();
  void _replotThis();
  void _toggleLogging();
  void _undo();
  
};

#endif
