#ifndef ParameterWindow_HH
#define ParameterWindow_HH

#include <iostream>

#include <gtkmm/actiongroup.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#if GTKMM_DISABLE_DEPRECATED
#  undef GTKMM_DISABLE_DEPRECATED
#  include <gtkmm/listviewtext.h>
#  define GTKMM_DISABLE_DEPRECATED
#else
#  include <gtkmm/listviewtext.h>
#endif
#include <gtkmm/radioaction.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/table.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/window.h>

#include <ColorPalettesWindow.hh>
#include <ColorTablesWindow.hh>
#include <ColorNamesWindow.hh>
#include <ImportColorTableWindow.hh>
#include <soloii.h>

class ParameterWindow : public Gtk::Window
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

  ParameterWindow(const Pango::FontDescription &default_font,
		  const int frame_index);
  
  /**
   * @brief Destructor.
   */

  virtual ~ParameterWindow();


  void initialize(const std::string &param_name);
  

  /**
   * @brief Process the changes made to the parameters.
   */

  void processChanges();
  

  void setEntriesFromPalette(const std::string &param_name, 
			     const std::string &palette_name);
  
  bool setParamInfo();
  
  /**
   * @brief Update the parameter window information from the associated
   *        window pointer.
   */

  void update();
  

  void toggleField();
  

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
   * @brief Get the original parameter name for this window.
   *
   * @return Returns the parameter name for this window.
   */

  std::string getOrigParamName() const
  {
    return _origParamName;
  }
  
  /**
   * @brief Set the value of the colorbar symbols toggle.
   *
   * @param[in] toggle_value    The toggle value.
   */

  inline void setColorbarSymbols(const bool toggle_value)
  {
    _colorbarSymbolsToggle->set_active(toggle_value);
  }

  /**
   * @brief Get value of the colorbar symbols toggle.
   *
   * @return Returns true of colorbar symbols is active, false otherwise.
   */

  inline bool isColorbarSymbols() const
  {
    return _colorbarSymbolsToggle->get_active();
  }

  /**
   * @brief Set the value of the highlight labels toggle.
   *
   * @param[in] toggle_value    The toggle value.
   */

  inline void setHighlightLabels(const bool toggle_value)
  {
    _highlightLabelsToggle->set_active(toggle_value);
  }

  /**
   * @brief Get value of the highlight labels toggle.
   *
   * @return Returns true if highlight labels is active, false otherwise.
   */

  inline bool isHighlightLabels() const
  {
    return _highlightLabelsToggle->get_active();
  }

  /**
   * @brief Set the value of the electric parameters toggle.
   *
   * @param[in] toggle_value    The toggle value.
   */

  inline void setElectricParameters(const bool toggle_value)
  {
    _electricParamsToggle->set_active(toggle_value);
  }

  /**
   * @brief Get value of the electric parameters toggle.
   *
   * @return Returns true if electric parameters is active, false otherwise.
   */

  inline bool isElectricParams() const
  {
    return _electricParamsToggle->get_active();
  }

  /**
   * @brief Set the color palette to use.
   *
   * @param[in] palette_name    The name of the palette to use.
   */

  void setPalette(const std::string palette_name);
  
  /**
   * @brief Set the color table name.
   *
   * @param[in] name   The color table name.
   */

  inline void setColorTableName(const std::string &name)
  {
    _colorTableEntry.set_text(name);
  }
  
  /**
   * @brief Set the list of parameter names
   *
   * @param[in] param_names   The list of parameter names.
   */

  inline void setParameterNamesList(const std::vector< std::string > &param_names)
  {
    _paramNames.clear_items();
    for (std::vector< std::string>::const_iterator param_name =
	   param_names.begin();
	 param_name != param_names.end(); ++param_name)
      _paramNames.append_text(*param_name);
  }
  
  
  /**
   * @brief Set the color displayed in the color test rectangle.
   *
   * @param[in] color_name    The name of the color to use.
   */

  void setTestColor(const std::string color_name);

  
  /**
   * @brief Set the color bar location to the bottom of the window.
   */

  inline void setColorbarBottom()
  {
    _colorBarBottomChoice->set_active(true);
  }
  
  /**
   * @brief Set the color bar location to the left of the window.
   */

  inline void setColorbarLeft()
  {
    _colorBarLeftChoice->set_active(true);
  }
  
  /**
   * @brief Set the color bar location to the right of the window.
   */

  inline void setColorbarRight()
  {
    _colorBarRightChoice->set_active(true);
  }
  

protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  /**
   * @brief Value used for rounding color bar ranges.
   */

  static const double CB_ROUND;
  
  /**
   * @brief Tolerance used when checking for changes in float values.
   */

  static const double TOLERANCE;
  

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
   * @brief The original parameter name.
   */

  std::string _origParamName;
  
  /**
   * @brief The original minimum color bar value.
   */

  double _origMin;
  
  /**
   * @brief The original maximum color bar value.
   */

  double _origMax;
  
  /**
   * @brief The original color bar center value.
   */

  double _origCenter;
  
  /**
   * @brief The original color bar increment value.
   */
  
  double _origIncrement;
  
  /**
   * @brief The original palette name.
   */

  std::string _origPalette;
  
  /**
   * @brief The original grid color name.
   */

  std::string _origGridColor;
  
  /**
   * @brief The original color table name.
   */
  
  std::string _origColorTable;
  
  /**
   * @brief The original boundary color name.
   */

  std::string _origBoundaryColor;
  
  /**
   * @brief The original exceeded color name.
   */

  std::string _origExceededColor;
  
  /**
   * @brief The original missing data color name.
   */

  std::string _origMissingDataColor;
  
  /**
   * @brief The original annotation color name.
   */

  std::string _origAnnotationColor;
  
  /**
   * @brief The original background color name.
   */

  std::string _origBackgroundColor;
  
  /**
   * @brief The original emphasis color name.
   */

  std::string _origEmphasisColor;
  
  /**
   * @brief The original emphasis minimum value.
   */

  double _origEmphasisMin;
  
  /**
   * @brief The original emphasis maximum value.
   */

  double _origEmphasisMax;
  

  // Menubar creation widgets //

  /**
   * @brief The user interface manager used for creating all of the menus.
   */

  Glib::RefPtr<Gtk::UIManager> _uiManager;

  /**
   * @brief The actions used by the menu items.
   */

  Glib::RefPtr<Gtk::ActionGroup> _actionGroup;

  Glib::RefPtr<Gtk::RadioAction> _colorBarBottomChoice;
  Glib::RefPtr<Gtk::RadioAction> _colorBarLeftChoice;
  Glib::RefPtr<Gtk::RadioAction> _colorBarRightChoice;
  
  Glib::RefPtr<Gtk::ToggleAction> _highlightLabelsToggle;
  Glib::RefPtr<Gtk::ToggleAction> _electricParamsToggle;
  Glib::RefPtr<Gtk::ToggleAction> _colorbarSymbolsToggle;
  
  // Widgets //

  Gtk::VBox _vbox;
  Gtk::HBox _hbox0;
  Gtk::DrawingArea _colorTest;
  Gtk::Label _numColorsLabel;

  Gtk::HButtonBox _hButtonBox;
  Gtk::Button _replotButton;
  Gtk::Button _okButton;
  Gtk::Button _cancelButton;
  
  Gtk::HBox _hbox;
  Gtk::VBox _vbox2;
  Gtk::Label _clickLabel;
  Gtk::Label _parametersLabel;
  
  Gtk::Table _table;
  Gtk::ScrolledWindow _paramNamesScrolledWindow;
  Gtk::ListViewText _paramNames;
  
  Gtk::Table _table2;
  Gtk::Label _paramNameLabel;
  Gtk::Entry _paramNameEntry;
  Gtk::Label _minLabel;
  Gtk::Entry _minEntry;
  Gtk::Label _maxLabel;
  Gtk::Entry _maxEntry;
  Gtk::Label _centerLabel;
  Gtk::Entry _centerEntry;
  Gtk::Label _incrementLabel;
  Gtk::Entry _incrementEntry;
  Gtk::Label _paletteLabel;
  Gtk::Entry _paletteEntry;
  Gtk::Label _gridColorLabel;
  Gtk::Entry _gridColorEntry;
  Gtk::Label _colorTableLabel;
  Gtk::Entry _colorTableEntry;
  
  Gtk::Label _spacerLabel;
  
  Gtk::Label _boundaryColorLabel;
  Gtk::Entry _boundaryColorEntry;
  Gtk::Label _exceededColorLabel;
  Gtk::Entry _exceededColorEntry;
  Gtk::Label _missingDataColorLabel;
  Gtk::Entry _missingDataColorEntry;
  Gtk::Label _annotationColorLabel;
  Gtk::Entry _annotationColorEntry;
  Gtk::Label _backgroundColorLabel;
  Gtk::Entry _backgroundColorEntry;
  Gtk::Label _emphasisColorLabel;
  Gtk::Entry _emphasisColorEntry;
  Gtk::Label _emphasisMinLabel;
  Gtk::Entry _emphasisMinEntry;
  Gtk::Label _emphasisMaxLabel;
  Gtk::Entry _emphasisMaxEntry;
  
  // Child Windows //

  ColorPalettesWindow *_colorPalettesWindow;
  ColorTablesWindow *_colorTablesWindow;
  ColorNamesWindow *_colorNamesWindow;
  ImportColorTableWindow *_importColorTableWindow;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Calculate the color bar center and increment values from the
   *        supplied min and max values.
   *
   * @param[in] ncolors     The number of colors in the color bar.
   * @param[out] center     The color bar center.
   * @param[out] incr       The color bar increment.
   * @param[in,out] min     The color bar minimum.  Could be changed if the
   *                          given min value is greater than the given max
   *                          value.
   * @param[in,out] max     The color bax maximum.  Could be changed if the
   *                          given min value is greater than the given max
   *                          value.
   */

  void _centerIncrFromMinMax(const int ncolors,
			     double &center, double &incr,
			     double &min, double &max);
  

  /**
   * @brief Calculate the color bar min and max values from the supplied
   *        center and increment values.
   *
   * @param[in] ncolors     The number of colors in the color bar.
   * @param[in] center      The color bar center.
   * @param[in] incr        The color bar increment.
   * @param[out] min        The color bar minimum.
   * @param[out] max        The color bax maximum.
   */

  void _minMaxFromCenterIncr(const int ncolors,
			     const double center, const double incr,
			     double &min, double &max );
  
  void _displayOrigValues();
  
  /**
   * @brief Display the original colorbar values (min, max, center and
   *        increment) in the entry widgets.  It is assumed that before
   *        this call, the center/increment/min/max values have been checked
   *        so that they are consistent.
   *
   * @param[in] center_value    The center value to display.
   * @param[in] incr_value      The increment value to display.
   * @param[in] min_value       The minimum value to display.
   * @param[in] max_value       The maximum value to display.
   */

  void _displayColorbarValues(const double center_value,
			      const double incr_value,
			      const double min_value,
			      const double max_value);
  
  /**
   * @brief Create the menubar.
   *
   * @param[in] container   Container for the menubar.
   */

  void _createMenubar(Gtk::Box &container);
  

  /**
   * @brief Set the number of colors for the label.
   *
   * @param[in] num_colors  The number of colors.
   */

  void _setNumColors(const int num_colors)
  {
    char num_colors_string[80];
    sprintf(num_colors_string, "   %d Colors   ", num_colors);
    _numColorsLabel.set_text(num_colors_string);
  }


  // Access Methods //

  /**
   * @brief Get the annotation color name.
   *
   * @return Returns the annotation color name.
   */

  inline std::string _getAnnotationColor() const
  {
    return _annotationColorEntry.get_text();
  }
  
  /**
   * @brief Set the annotation color name.
   *
   * @param[in] color_name    The annotation color name.
   */

  inline void _setAnnotationColor(const std::string &color_name)
  {
    _origAnnotationColor = color_name;
    _resetAnnotationColor();
  }
  
  /**
   * @brief Reset the annotation color name.
   */

  inline void _resetAnnotationColor()
  {
    _annotationColorEntry.set_text(_origAnnotationColor);
  }
  
  /**
   * @brief Get the background color name.
   *
   * @return Returns the background color name.
   */

  inline std::string _getBackgroundColor() const
  {
    return _backgroundColorEntry.get_text();
  }
  
  /**
   * @brief Set the background color name.
   *
   * @param[in] color_name    The background color name.
   */

  inline void _setBackgroundColor(const std::string &color_name)
  {
    _origBackgroundColor = color_name;
    _resetBackgroundColor();
  }
  
  /**
   * @brief Reset the background color name.
   */

  inline void _resetBackgroundColor()
  {
    _backgroundColorEntry.set_text(_origBackgroundColor);
  }
  
  /**
   * @brief Get the boundary color name.
   *
   * @return Returns the boundary color name.
   */

  inline std::string _getBoundaryColor() const
  {
    return _boundaryColorEntry.get_text();
  }
  
  /**
   * @brief Set the boundary color name.
   *
   * @param[in] color_name    The boundary color name.
   */

  inline void _setBoundaryColor(const std::string &color_name)
  {
    _origBoundaryColor = color_name;
    _resetBoundaryColor();
  }
  
  /**
   * @brief Reset the boundary color name.
   */

  inline void _resetBoundaryColor()
  {
    _boundaryColorEntry.set_text(_origBoundaryColor);
  }
  
  /**
   * @brief Get the current colorbar center entry value.
   *
   * @return Returns the colorbar center value.
   */

  inline double _getColorbarCenter() const
  {
    return atof(_centerEntry.get_text().c_str());
  }
  
  /**
   * @brief Set the current colorbar center and increment values.
   *        Update the colorbar min/max values to be consistent
   *
   * @param[in] center   The colorbar center value.
   * @param[in] incr     The colorbar increment value.
   */

  void _setColorbarCenterIncrement(const double center,
				   const double incr);
  
  /**
   * @brief Set the current colorbar center entry value.  It is assumed
   *        that the given value has be checked for consistency with the
   *        other colorbar values so the value is displayed as is.
   *
   * @param[in] center   The colorbar center value.
   */

  inline void _setColorbarCenter(const double center)
  {
    _origCenter = center;
    _resetColorbarCenter();
  }
  
  /**
   * @brief Reset the current colorbar center entry value.
   */

  inline void _resetColorbarCenter()
  {
    char entry_string[80];
    sprintf(entry_string, "%.3f", _origCenter);
    _centerEntry.set_text(entry_string);
  }
  
  /**
   * @brief Get the current colorbar increment entry value.
   *
   * @return Returns the colorbar increment value.
   */

  inline double _getColorbarIncrement() const
  {
    return atof(_incrementEntry.get_text().c_str());
  }
  
  /**
   * @brief Set the current colorbar increment entry value.  It is assumed
   *        that the given value has be checked for consistency with the
   *        other colorbar values so the value is displayed as is.
   *
   * @param[in] increment   The colorbar increment value.
   */

  inline void _setColorbarIncrement(const double increment)
  {
    _origIncrement = increment;
    _resetColorbarIncrement();
  }
  
  /**
   * @brief Reset the current colorbar increment entry value.
   */

  inline void _resetColorbarIncrement()
  {
    char entry_string[80];
    sprintf(entry_string, "%.3f", _origIncrement);
    _incrementEntry.set_text(entry_string);
  }
  
  /**
   * @brief Get the current colorbar maximum entry value.
   *
   * @return Returns the colorbar maximum value.
   */

  inline double _getColorbarMax()
  {
    std::string max_str = _maxEntry.get_text();
    double max_value;
  
    if (sscanf(max_str.c_str(), "%lf", &max_value) != 1)
    {
      _resetColorbarMax();
      return _origMax;
    }

    return max_value;
  }
  
  /**
   * @brief Set the current colorbar maximum entry value.  It is assumed
   *        that the given value has be checked for consistency with the
   *        other colorbar values so the value is displayed as is.
   *
   * @param[in] max   The colorbar maximum value.
   */

  inline void _setColorbarMax(const double max)
  {
    _origMax = max;
    _resetColorbarMax();
  }
  
  /**
   * @brief Reset the current colorbar maximum entry value.
   */

  inline void _resetColorbarMax()
  {
    char entry_string[80];
    sprintf(entry_string, "%.3f", _origMax);
    _maxEntry.set_text(entry_string);
  }
  
  /**
   * @brief Get the current colorbar minimum entry value.
   *
   * @return Returns the colorbar minimum value.
   */

  inline double _getColorbarMin()
  {
    std::string min_str = _minEntry.get_text();
    double min_value;
  
    if (sscanf(min_str.c_str(), "%lf", &min_value) != 1)
    {
      _resetColorbarMin();
      return _origMin;
    }

    return min_value;
  }
  
  /**
   * @brief Set the current colorbar minimum entry value.  It is assumed
   *        that the given value has be checked for consistency with the
   *        other colorbar values so the value is displayed as is.
   *
   * @param[in] min   The colorbar minimum value.
   */

  inline void _setColorbarMin(const double min)
  {
    _origMin = min;
    _resetColorbarMin();
  }
  
  /**
   * @brief Reset the current colorbar minimum entry value.
   */

  inline void _resetColorbarMin()
  {
    char entry_string[80];
    sprintf(entry_string, "%.3f", _origMin);
    _minEntry.set_text(entry_string);
  }
  
  /**
   * @brief Get the color table name.
   *
   * @return Returns the color table name entry.
   */

  inline std::string _getColorTable()
  {
    return _colorTableEntry.get_text();
  }
  
  /**
   * @brief Set the color table name.
   *
   * @param[in] color_table    The color table name.
   */

  inline void _setColorTable(const std::string &color_table)
  {
    _origColorTable = color_table;
    _resetColorTable();
  }
  
  /**
   * @brief Reset the color table name.
   */

  inline void _resetColorTable()
  {
    _colorTableEntry.set_text(_origColorTable);
  }
  
  /**
   * @brief Get the emphasis color name.
   *
   * @return Returns the emphasis color name.
   */

  inline std::string _getEmphasisColor() const
  {
    return _emphasisColorEntry.get_text();
  }
  
  /**
   * @brief Set the emphasis color name.
   *
   * @param[in] color_name    The emphasis color name.
   */

  inline void _setEmphasisColor(const std::string &color_name)
  {
    _origEmphasisColor = color_name;
    _resetEmphasisColor();
  }
  
  /**
   * @brief Reset the emphasis color name.
   */

  inline void _resetEmphasisColor()
  {
    _emphasisColorEntry.set_text(_origEmphasisColor);
  }
  
  /**
   * @brief Get the emphasis value range from the user entries.
   *
   * @param[out] emphasis_min    The minimum emphasis value.
   * @param[out] emphasis_max    The maximum emphasis value.
   */

  inline void _getEmphasisRange(float &emphasis_min,
				float &emphasis_max) const
  {
    double emphasis_min_double;
    double emphasis_max_double;

    _getEmphasisRange(emphasis_min_double, emphasis_max_double);

    emphasis_min = emphasis_min_double;
    emphasis_max = emphasis_max_double;
  }
  
  inline void _getEmphasisRange(double &emphasis_min,
				double &emphasis_max) const
  {
    emphasis_min = atof(_emphasisMinEntry.get_text().c_str());
    emphasis_max = atof(_emphasisMaxEntry.get_text().c_str());
  }
  
  /**
   * @brief Set the emphasis minimum entry value.
   *
   * @param[in] min   The emphasis minimum value.
   */

  inline void _setEmphasisMin(const double min)
  {
    _origEmphasisMin = min;
    _resetEmphasisMin();
  }
  
  /**
   * @brief Reset the emphasis minimum entry value.
   */

  inline void _resetEmphasisMin()
  {
    char entry_string[80];
    sprintf(entry_string, "%.3f", _origEmphasisMin);
    _emphasisMinEntry.set_text(entry_string);
  }
  
  /**
   * @brief Set the emphasis maximum entry value.
   *
   * @param[in] max   The emphasis maximum value.
   */

  inline void _setEmphasisMax(const double max)
  {
    _origEmphasisMax = max;
    _resetEmphasisMax();
  }
  
  /**
   * @brief Reset the emphasis maximum entry value.
   */

  inline void _resetEmphasisMax()
  {
    char entry_string[80];
    sprintf(entry_string, "%.3f", _origEmphasisMax);
    _emphasisMaxEntry.set_text(entry_string);
  }
  
  /**
   * @brief Get the exceeded color name.
   *
   * @return Returns the exceeded color name.
   */

  inline std::string _getExceededColor() const
  {
    return _exceededColorEntry.get_text();
  }
  
  /**
   * @brief Set the exceeded color name.
   *
   * @param[in] color_name    The exceeded color name.
   */

  inline void _setExceededColor(const std::string &color_name)
  {
    _origExceededColor = color_name;
    _resetExceededColor();
  }
  
  /**
   * @brief Reset the exceeded color name.
   */

  inline void _resetExceededColor()
  {
    _exceededColorEntry.set_text(_origExceededColor);
  }
  
  /**
   * @brief Get the grid color name.
   *
   * @return Returns the grid color name.
   */

  inline std::string _getGridColor() const
  {
    return _gridColorEntry.get_text();
  }
  
  /**
   * @brief Set the grid color name.
   *
   * @param[in] color_name    The grid color name.
   */

  inline void _setGridColor(const std::string &color_name)
  {
    _origGridColor = color_name;
    _resetGridColor();
  }
  
  /**
   * @brief Reset the grid color name.
   */

  inline void _resetGridColor()
  {
    _gridColorEntry.set_text(_origGridColor);
  }
  
  /**
   * @brief Get the missing data color name.
   *
   * @return Returns the missing data color name.
   */

  inline std::string _getMissingDataColor() const
  {
    return _missingDataColorEntry.get_text();
  }
  
  /**
   * @brief Set the missing data color name.
   *
   * @param[in] color_name    The missing data color name.
   */

  inline void _setMissingDataColor(const std::string &color_name)
  {
    _origMissingDataColor = color_name;
    _resetMissingDataColor();
  }
  
  /**
   * @brief Reset the missing data color name.
   */

  inline void _resetMissingDataColor()
  {
    _missingDataColorEntry.set_text(_origMissingDataColor);
  }
  
  /**
   * @brief Get the current text in the palette name entry.
   *
   * @return Returns the palette name.
   */

  inline std::string _getPaletteName() const
  {
    return _paletteEntry.get_text();
  }
  
  /**
   * @brief Set the current palette name.
   *
   * @param[in] palette_name  The palette name to use.
   */

  inline void _setPaletteName(const std::string &palette_name)
  {
    _paletteEntry.set_text(palette_name);
    _origPalette = palette_name;
  }
  
  /**
   * @brief Reset the current palette name.
   */

  inline void _resetPaletteName()
  {
    _paletteEntry.set_text(_origPalette);
  }
  
  /**
   * @brief Get the current text in the parameter name entry.
   *
   * @return Returns the parameter name.
   */

  inline std::string _getParameterName() const
  {
    return _paramNameEntry.get_text();
  }
  
  /**
   * @brief Set the current parameter name.
   *
   * @param[in] param_name  The parameter name to use.
   */

  inline void _setParameterName(const std::string &param_name)
  {
    _paramNameEntry.set_text(param_name);
    _origParamName = param_name;
  }
  
  /**
   * @brief Reset the current parameter name.
   */

  inline void _resetParameterName()
  {
    _paramNameEntry.set_text(_origParamName);
  }
  

  // Callback methods //

  void _broadcastAnnotationColor();
  void _broadcastBackgroundColor();
  void _broadcastBoundaryColor();
  void _broadcastColorBarSettings();
  void _broadcastEmphasisColor();
  void _broadcastExceededColor();
  void _broadcastGridColor();
  void _broadcastMissingDataColor();
  void _centerIncrActivated();
  void _closeWindows();
  void _displayHelpFile();
  void _displayHelpMinMax();
  void _displayHelpOptions();
  void _displayHelpOverview();
  void _displayHelpPalettes();
  void _displayHelpParamLinks();
  void _doItAndHide();
  void _importColorTable();
  void _listColors();
  void _listColorTables();
  void _listPalettes();
  void _minMaxActivated();
  void _paletteActivated();
  void _paramNameActivated();
  void _replotAll();
  void _replotLinks();
  void _replotThisFrame();
  void _selectParameterName(const Gtk::TreeModel::Path &path,
			    Gtk::TreeViewColumn *column);
  void _setColorBarLocation();
  void _setLinks();
  
};

#endif
