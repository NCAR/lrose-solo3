#include <algorithm>
#include <iostream>
#include <math.h>
#include <gtk/gtk.h>
#include <gtkmm/radioaction.h>

#include <ColorManager.hh>
#include <CRectangle.hh>
#include <PaletteManager.hh>
#include <sii_externals.h>
#include <sii_links_widget.hh>
#include <sii_param_widgets.hh>
#include <sii_utils.hh>
#include <solo2.hh>
#include <sp_accepts.hh>
#include <sp_basics.hh>
#include <sp_lists.hh>

#include "ParameterWindow.hh"


const double ParameterWindow::CB_ROUND = 0.000001;
const double ParameterWindow::TOLERANCE = 0.00001;

/**********************************************************************
 * Constructor
 */

ParameterWindow::ParameterWindow(const Pango::FontDescription &default_font,
				 const int frame_index) :
  Gtk::Window(),
  _frameIndex(frame_index),
  _defaultFont(default_font),
  _origParamName(""),
  _origMin(0.0),
  _origMax(0.0),
  _origCenter(0.0),
  _origIncrement(0.0),
  _origPalette(""),
  _origGridColor(""),
  _origColorTable(""),
  _origBoundaryColor(""),
  _origExceededColor(""),
  _origMissingDataColor(""),
  _origAnnotationColor(""),
  _origBackgroundColor(""),
  _origEmphasisColor(""),
  _origEmphasisMin(0.0),
  _origEmphasisMax(0.0),
  _table(2, 1, false),
  _paramNames(1),
  _table2(2, 16, false),
  _colorPalettesWindow(0),
  _colorTablesWindow(0),
  _colorNamesWindow(0),
  _importColorTableWindow(0)
{
  char display_string[1024];
  
  // Get pointers to the frame information

  ParamData *pdata = frame_configs[_frameIndex]->param_data;
  
  // Create some fonts to use for our widgets

  Pango::FontDescription bold_font = _defaultFont;
  bold_font.set_weight(Pango::WEIGHT_SEMIBOLD);
  
  Pango::FontDescription smaller_font = _defaultFont;
  smaller_font.set_size(_defaultFont.get_size() - (2 * 1024));
  
  Pango::FontDescription smaller_italic_font = smaller_font;
  smaller_italic_font.set_style(Pango::STYLE_ITALIC);
  
  // Set the window title and border width

  sprintf(display_string, "Frame %d Parameter and Colors Widget",
	  _frameIndex + 1);
  set_title(display_string);

  set_border_width(0);
  
  // Create the vertical box for storing widgets

  _vbox.set_homogeneous(false);
  _vbox.set_spacing(4);
  add(_vbox);
  
  // Create the menubar

  _createMenubar(_vbox);
  
  // Create the horizontal box for the widgets below the menubar

  _hbox0.set_homogeneous(false);
  _hbox0.set_spacing(0);
  _vbox.add(_hbox0);
  
  _colorTest.set_size_request(96, 0);
  _hbox0.pack_start(_colorTest, true, true, 0);
  
  _setNumColors(pdata->pal->getNumColors());
  _numColorsLabel.modify_font(bold_font);
  _hbox0.pack_start(_numColorsLabel, true, true, 0);

  _hButtonBox.set_spacing(4);
  _hbox0.pack_end(_hButtonBox, true, true, 0);

  _replotButton.set_label("Replot");
  _replotButton.modify_font(_defaultFont);
  _replotButton.signal_clicked().connect(sigc::mem_fun(*this,
						       &ParameterWindow::_replotThisFrame));
  _hButtonBox.pack_start(_replotButton, true, true, 0);
  
  _okButton.set_label("OK");
  _okButton.modify_font(_defaultFont);
  _okButton.signal_clicked().connect(sigc::mem_fun(*this,
						   &ParameterWindow::_doItAndHide));
  _hButtonBox.pack_start(_okButton, true, true, 0);
  
  _cancelButton.set_label("Cancel");
  _cancelButton.modify_font(_defaultFont);
  _cancelButton.signal_clicked().connect(sigc::mem_fun(*this,
						       &ParameterWindow::_closeWindows));
  _hButtonBox.pack_start(_cancelButton, true, true, 0);
  
  _hbox.set_homogeneous(false);
  _hbox.set_spacing(0);
  _hbox.set_border_width(4);
  _vbox.add(_hbox);
  
  // Parameter names list

  _vbox2.set_homogeneous(false);
  _vbox2.set_spacing(0);
  _hbox.add(_vbox2);
  
  _parametersLabel.set_text("Parameters");
  _parametersLabel.set_justify(Gtk::JUSTIFY_CENTER);
  _parametersLabel.modify_font(bold_font);
  _vbox2.pack_start(_parametersLabel, false, false, 0);
  
  _clickLabel.set_text("(Double-click to select)");
  _clickLabel.set_justify(Gtk::JUSTIFY_CENTER);
  _clickLabel.modify_font(smaller_italic_font);
  _vbox2.pack_start(_clickLabel, false, false, 0);
  
  _vbox2.pack_start(_table, true, true, 0);
  
  _paramNames.set_headers_visible(false);
  _paramNames.modify_font(_defaultFont);

  // Set the padding for the list.  Note that the gtkmm-2.4 installed with
  // the Centos OS has a different API for the CellRenderer class so we can't
  // do this on that OS.
#ifndef OS_IS_CENTOS
  Gtk::CellRenderer *renderer = _paramNames.get_column_cell_renderer(0);
  renderer->set_padding(0, 0);
#endif

  setParameterNamesList(pdata->param_names_list);
  _paramNames.signal_row_activated().connect(sigc::mem_fun(*this,
							   &ParameterWindow::_selectParameterName));
  
  _paramNamesScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC,
				       Gtk::POLICY_AUTOMATIC);
  
  _paramNamesScrolledWindow.add(_paramNames);
  _table.attach(_paramNamesScrolledWindow, 0, 1, 0, 1,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		0, 0);
  
  _hbox.pack_start(_table2, true, true, 0);
  
  // Parameter name

  int row = 0;
  
  _paramNameLabel.set_text(" Parameter Name ");
  _paramNameLabel.modify_font(bold_font);
  _table2.attach(_paramNameLabel, 0, 1, row, row+1,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 0, 0);

  _resetParameterName();
  _paramNameEntry.modify_font(_defaultFont);
  _paramNameEntry.signal_activate().connect(sigc::mem_fun(*this,
							  &ParameterWindow::_paramNameActivated));
  _table2.attach(_paramNameEntry, 1, 2, row, row+1,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 0, 0);

  // Min

  row++;
  
  _minLabel.set_text(" Min ");
  _minLabel.modify_font(bold_font);
  _table2.attach(_minLabel, 0, 1, row, row+1,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 0, 0);
  
  _resetColorbarMin();
  _minEntry.modify_font(_defaultFont);
  _minEntry.signal_activate().connect(sigc::mem_fun(*this,
						    &ParameterWindow::_minMaxActivated));
   _table2.attach(_minEntry, 1, 2, row, row+1,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  0, 0);
   
  // Max

  row++;
  
  _maxLabel.set_text(" Max ");
  _maxLabel.modify_font(bold_font);
  _table2.attach(_maxLabel, 0, 1, row, row+1,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 0, 0);
  
  _resetColorbarMax();
  _maxEntry.modify_font(_defaultFont);
  _maxEntry.signal_activate().connect(sigc::mem_fun(*this,
						    &ParameterWindow::_minMaxActivated));
   _table2.attach(_maxEntry, 1, 2, row, row+1,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  0, 0);
   
  // Center

  row++;
  
  _centerLabel.set_text(" Center ");
  _centerLabel.modify_font(bold_font);
  _table2.attach(_centerLabel, 0, 1, row, row+1,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 0, 0);
  
  _resetColorbarCenter();
  _centerEntry.modify_font(_defaultFont);
  _centerEntry.signal_activate().connect(sigc::mem_fun(*this,
						       &ParameterWindow::_centerIncrActivated));
   _table2.attach(_centerEntry, 1, 2, row, row+1,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  0, 0);
   
  // Increment

  row++;
  
  _incrementLabel.set_text(" Increment ");
  _incrementLabel.modify_font(bold_font);
  _table2.attach(_incrementLabel, 0, 1, row, row+1,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		 0, 0);
  
  _resetColorbarIncrement();
  _incrementEntry.modify_font(_defaultFont);
  _incrementEntry.signal_activate().connect(sigc::mem_fun(*this,
							  &ParameterWindow::_centerIncrActivated));
   _table2.attach(_incrementEntry, 1, 2, row, row+1,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  0, 0);

   // Palette

   row++;
   
   _paletteLabel.set_text(" Color Palette ");
   _paletteLabel.modify_font(bold_font);
   _table2.attach(_paletteLabel, 0, 1, row, row+1,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  0, 0);
   
   _resetPaletteName();
   _paletteEntry.modify_font(_defaultFont);
   _paletteEntry.signal_activate().connect(sigc::mem_fun(*this,
							 &ParameterWindow::_paletteActivated));
   _table2.attach(_paletteEntry, 1, 2, row, row+1,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  0, 0);
   
   // Grid color

   row++;
   
   _gridColorLabel.set_text(" Grid Color ");
   _gridColorLabel.modify_font(bold_font);
   _table2.attach(_gridColorLabel, 0, 1, row, row+1,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  0, 0);
   
   _resetGridColor();
   _gridColorEntry.modify_font(_defaultFont);
   _table2.attach(_gridColorEntry, 1, 2, row, row+1,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  0, 0);
   
   // Color table name

   row++;
   
   _colorTableLabel.set_text(" Color Table Name ");
   _colorTableLabel.modify_font(bold_font);
   _table2.attach(_colorTableLabel, 0, 1, row, row+1,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  0, 0);
   
   _resetColorTable();
   _colorTableEntry.modify_font(_defaultFont);
   _table2.attach(_colorTableEntry, 1, 2, row, row+1,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  0, 0);
   
   // Spacer

   row++;
   
   _spacerLabel.set_text(" ");
   _table2.attach(_spacerLabel, 0, 1, row, row+1,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  0, 0);
   
   // Boundary color

   row++;
   
   _boundaryColorLabel.set_text(" Boundary Color ");
   _boundaryColorLabel.modify_font(bold_font);
   _table2.attach(_boundaryColorLabel, 0, 1, row, row+1,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  0, 0);

   _resetBoundaryColor();
   _boundaryColorEntry.modify_font(_defaultFont);
   _table2.attach(_boundaryColorEntry, 1, 2, row, row+1,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  0, 0);
   
   // Exceeded color

   row++;
   
   _exceededColorLabel.set_text(" Exceeded Color ");
   _exceededColorLabel.modify_font(bold_font);
   _table2.attach(_exceededColorLabel, 0, 1, row, row+1,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  0, 0);
   
   _resetExceededColor();
   _exceededColorEntry.modify_font(_defaultFont);
   _table2.attach(_exceededColorEntry, 1, 2, row, row+1,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  0, 0);
   
   // Missing data color

   row++;
   
   _missingDataColorLabel.set_text(" Missing Data Color ");
   _missingDataColorLabel.modify_font(bold_font);
   _table2.attach(_missingDataColorLabel, 0, 1, row, row+1,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  0, 0);
   
   _resetMissingDataColor();
   _missingDataColorEntry.modify_font(_defaultFont);
   _table2.attach(_missingDataColorEntry, 1, 2, row, row+1,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  0, 0);
   
   // Annotation color

   row++;
   
   _annotationColorLabel.set_text(" Annotation Color ");
   _annotationColorLabel.modify_font(bold_font);
   _table2.attach(_annotationColorLabel, 0, 1, row, row+1,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  0, 0);
   
   _resetAnnotationColor();
   _annotationColorEntry.modify_font(_defaultFont);
   _table2.attach(_annotationColorEntry, 1, 2, row, row+1,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  0, 0);
   
   // Background color

   row++;
   
   _backgroundColorLabel.set_text(" Background Color ");
   _backgroundColorLabel.modify_font(bold_font);
   _table2.attach(_backgroundColorLabel, 0, 1, row, row+1,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  0, 0);
   
   _resetBackgroundColor();
   _backgroundColorEntry.modify_font(_defaultFont);
   _table2.attach(_backgroundColorEntry, 1, 2, row, row+1,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  0, 0);
   
   // Emphasis color

   row++;
   
   _emphasisColorLabel.set_text(" Emphasis Color ");
   _emphasisColorLabel.modify_font(bold_font);
   _table2.attach(_emphasisColorLabel, 0, 1, row, row+1,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  0, 0);
   
   _resetEmphasisColor();
   _emphasisColorEntry.modify_font(_defaultFont);
   _table2.attach(_emphasisColorEntry, 1, 2, row, row+1,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  0, 0);
   
   // Emphasis min

   row++;
   
   _emphasisMinLabel.set_text(" Emphasis Min ");
   _emphasisMinLabel.modify_font(bold_font);
   _table2.attach(_emphasisMinLabel, 0, 1, row, row+1,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  0, 0);
   
   _resetEmphasisMin();
   _emphasisMinEntry.modify_font(_defaultFont);
   _table2.attach(_emphasisMinEntry, 1, 2, row, row+1,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  0, 0);
   
   // Emphasis max

   row++;
   
   _emphasisMaxLabel.set_text(" Emphasis Max ");
   _emphasisMaxLabel.modify_font(bold_font);
   _table2.attach(_emphasisMaxLabel, 0, 1, row, row+1,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  0, 0);
   
   _resetEmphasisMax();
   _emphasisMaxEntry.modify_font(_defaultFont);
   _table2.attach(_emphasisMaxEntry, 1, 2, row, row+1,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		  0, 0);
   
}


/**********************************************************************
 * Destructor
 */

ParameterWindow::~ParameterWindow()
{
  // Delete the child window objects.

  delete _colorPalettesWindow;
  delete _colorTablesWindow;
  delete _colorNamesWindow;
  delete _importColorTableWindow;
  
  // If we do ever delete this object, then we need to delete the associated
  // pointer.  This is not the right way to do this, but leave this in until
  // the frame_configs structure is refactored.

  frame_configs[_frameIndex]->param_window = 0;
}


/**********************************************************************
 * initialize()
 */

void ParameterWindow::initialize(const std::string &param_name)
{
  // If the parameter data hasn't been initialized yet, then initialize it.

  sii_initialize_parameter(_frameIndex, param_name);
  
  ParamData *pd = frame_configs[_frameIndex]->param_data;
  SiiPalette *pal = pd->pal;
  
  // Set all the original values and copies from the palette

  _origParamName = param_name;
  _origGridColor = pal->getGridColor();
  _origColorTable = pal->getColorTableName();
  _origPalette = pal->getPaletteName();
  _origExceededColor = pal->getExceededColor();
  _origMissingDataColor = pal->getMissingDataColor();
  _origBoundaryColor = pal->getBoundaryColor();
  _origAnnotationColor = pal->getAnnotationColor();
  _origBackgroundColor = pal->getBackgroundColor();
  _origEmphasisColor = pal->getEmphasisColor();

  _origCenter = pal->getCenterDataValue();
  _origIncrement = pal->getColorWidth();
  
  double min, max;
  _minMaxFromCenterIncr(pd->pal->getNumColors(),
			pal->getCenterDataValue(), pal->getColorWidth(),
			min, max);
  pal->setMinValue(min);
  pal->setMaxValue(max);
  _origMin = pal->getMinValue();
  _origMax = pal->getMaxValue();

  _origEmphasisMin = pal->getEmphasisZoneLower();
  _origEmphasisMax = pal->getEmphasisZoneUpper();
  
  _displayOrigValues();
  
  /* set up circular que of toggle info */

  pd->fields_list = sii_init_circ_que(MAX_FRAMES);
  pd->field_toggle_count = 2;
}


/**********************************************************************
 * processChanges()
 */

void ParameterWindow::processChanges()
{
  // If you want to associate the current parameter name with a
  // different palette, type in the new palette name and press <enter>.
  //
  // Or if you want to define a new name and a new palette,
  // type in the name but DO NOT press enter. Then type in
  // the new palette name and press <enter>.
  //
  // Change the other parameters to suite your needs and click
  // "Replot" or "OK" and the parameters will persist.

  ParamData *pd = frame_configs[_frameIndex]->param_data;

  std::string param_name_entered = _paramNameEntry.get_text();
  std::string palette_name_entered = _paletteEntry.get_text();

  if (palette_name_entered != _origPalette)
  {
    PaletteManager::getInstance()->newPaletteForParam(palette_name_entered,
						      param_name_entered);
  }
  else if (param_name_entered != _origParamName)
  {
    // The parameter name changed but the palette didn't

    pd->pal = PaletteManager::getInstance()->setPalette(param_name_entered);
  }
   
  // If the user changed any of the palette parameters, update the palette
  // object to reflect these changes

  SiiPalette *pal = pd->pal;
  
  double center_value_entered = _getColorbarCenter();
  double incr_value_entered = _getColorbarIncrement();
  
  double min_value_entered = _getColorbarMin();
  double max_value_entered = _getColorbarMax();
  
  if (fabs(center_value_entered - _origCenter) > TOLERANCE ||
      fabs(incr_value_entered - _origIncrement) > TOLERANCE)
  {
    pal->setCenterDataValue(center_value_entered);
    pal->setColorWidth(incr_value_entered);
    
    double min, max;
    _minMaxFromCenterIncr(pal->getNumColors(),
			  pal->getCenterDataValue(),
			  pal->getColorWidth(),
			  min, max);
    pal->setMinValue(min);
    pal->setMaxValue(max);
  }
  else if (fabs(min_value_entered - _origMin) > TOLERANCE ||
	   fabs(max_value_entered - _origMax) > TOLERANCE)
  {
    // Calculate the ctr/inc values from the min/max values.  Note that this
    // can change the min/max values if min > max to begin with.

    double center, incr;
    double min = min_value_entered;
    double max = max_value_entered;
    _centerIncrFromMinMax(pal->getNumColors(), center, incr, min, max);

    pal->setCenterDataValue(center);
    pal->setColorWidth(incr);
    pal->setMinValue(min);
    pal->setMaxValue(max);
  }

  // Now process any color changes

  std::string grid_color_entered = _getGridColor();
  if (_gridColorEntry.get_text() != _origGridColor)
  {
    pal->setGridColor(grid_color_entered);
  }
  
  std::string color_table_entered = _getColorTable();
  if (color_table_entered != _origColorTable)
  {
    pal->setColorTableName(color_table_entered);
  }
  
  std::string boundary_color_entered = _getBoundaryColor();
  if (boundary_color_entered != _origBoundaryColor)
  {
    pal->setBoundaryColor(boundary_color_entered);
  }
  
  std::string exceeded_color_entered = _getExceededColor();
  if (exceeded_color_entered != _origExceededColor)
  {
    pal->setExceededColor(exceeded_color_entered);
  }
  
  std::string missing_color_entered = _getMissingDataColor();
  if (missing_color_entered != _origMissingDataColor)
  {
    pal->setMissingDataColor(missing_color_entered);
  }
  
  std::string annot_color_entered = _getAnnotationColor();
  if (annot_color_entered != _origAnnotationColor)
  {
    pal->setAnnotationColor(annot_color_entered);
  }
  
  std::string background_color_entered = _getBackgroundColor();
  if (background_color_entered != _origBackgroundColor)
  {
    pal->setBackgroundColor(background_color_entered);
  }
  
  double emphasis_min_entered, emphasis_max_entered;
  _getEmphasisRange(emphasis_min_entered, emphasis_max_entered);
  if (emphasis_min_entered != _origEmphasisMin ||
      emphasis_max_entered != _origEmphasisMax)
  {
    pal->setEmphasisZone(emphasis_min_entered, emphasis_max_entered);
  }
  
  std::string emphasis_color_entered = _getEmphasisColor();
  if (emphasis_color_entered != _origEmphasisColor)
  {
    pal->setEmphasisColor(emphasis_color_entered);
  }
  
  // Update the user entries to match the internal palette.  This is really
  // just needed if the user changed the current parameter which caused a change
  // in the palette used.

  setEntriesFromPalette(param_name_entered, "");
}


/**********************************************************************
 * setEntriesFromPalette()
 */

void ParameterWindow::setEntriesFromPalette(const std::string &param_name, 
					    const std::string &palette_name)
{
  ParamData *pd = frame_configs[_frameIndex]->param_data;
   
  // Get the palette

  SiiPalette *pal = NULL;

  if (param_name != "")
  {
    pal = PaletteManager::getInstance()->setPalette(param_name);
  }
  else if (palette_name != "")
  {

    pal = PaletteManager::getInstance()->seek(palette_name);
  }

  if (!pal)
    pal = pd->pal;
  pd->pal = pal;

  // Set the original color fields based on the palette

  _setGridColor(pal->getGridColor());
  _setColorTable(pal->getColorTableName());
  _setPaletteName(pal->getPaletteName());
  _setBoundaryColor(pal->getBoundaryColor());
  _setExceededColor(pal->getExceededColor());
  _setMissingDataColor(pal->getMissingDataColor());
  _setAnnotationColor(pal->getAnnotationColor());
  _setBackgroundColor(pal->getBackgroundColor());
  _setEmphasisColor(pal->getEmphasisColor());
   
  _setColorbarCenterIncrement(pal->getCenterDataValue(),
			      pal->getColorWidth());

  double min, max;
  _minMaxFromCenterIncr(pal->getNumColors(),
			pal->getCenterDataValue(), pal->getColorWidth(),
			min, max);
  pal->setMinValue(min);
  pal->setMaxValue(max);

  _setEmphasisMin(pal->getEmphasisZoneLower());
  _setEmphasisMax(pal->getEmphasisZoneUpper());
}


/**********************************************************************
 * setPalette()
 */

void ParameterWindow::setPalette(const std::string palette_name)
{
  _paletteEntry.set_text(palette_name);
  const std::string param_name = _paramNameEntry.get_text();
  PaletteManager::getInstance()->newPaletteForParam(palette_name, param_name);
  processChanges();
}


/**********************************************************************
 * setParamInfo()
 */

bool ParameterWindow::setParamInfo()
{
  ParamData *pd = frame_configs[_frameIndex]->param_data;

  WW_PTR wwptr = solo_return_wwptr(_frameIndex);

  processChanges();

  // Set the window colorbar location

  wwptr->color_bar_location = 0;
  if (pd->cb_loc == PARAM_CB_LEFT)
    wwptr->color_bar_location = -1;
  else if (pd->cb_loc == PARAM_CB_RIGHT)
    wwptr->color_bar_location = 1;
  
  // Set the window color bar symbols flag

  wwptr->color_bar_symbols = isColorbarSymbols();

  pd->pal->setCenterDataValue(_getColorbarCenter());
  pd->pal->setColorWidth(_getColorbarIncrement());
  double min, max;
  _minMaxFromCenterIncr(pd->pal->getNumColors(),
			pd->pal->getCenterDataValue(),
			pd->pal->getColorWidth(),
			min, max);
  pd->pal->setMinValue(min);
  pd->pal->setMaxValue(max);

  // Get pointers to the internal structures

  wwptr->parameter.changed = 1;
  struct solo_perusal_info *spi = solo_return_winfo_ptr();

  strncpy(wwptr->parameter.parameter_name, _getParameterName().c_str(), 16);
  strncpy(wwptr->parameter.palette_name, _getPaletteName().c_str(), 16);

  _getEmphasisRange(wwptr->emphasis_min, wwptr->emphasis_max);

  // See what other windows use this palatte and be sure to mark them
  // as changed
  
  std::string palette_name = wwptr->parameter.palette_name;

  for (int ww = 0; ww < SOLO_MAX_WINDOWS; ww++)
  {
    WW_PTR wwptrc = solo_return_wwptr(ww);
    if (palette_name.compare(wwptrc->parameter.palette_name) == 0)
      wwptrc->parameter.changed = 1;
  }    

  // Deal with the links

  bool linked_windows[SOLO_MAX_WINDOWS];
  
  LinksInfo *li = frame_configs[_frameIndex]->link_set[LI_PARAM];
  for (int jj = 0; jj < MAX_FRAMES; jj++)
    linked_windows[jj] = li->link_set[jj];
  linked_windows[_frameIndex] = true;
  
  bool dangling_links[SOLO_MAX_WINDOWS];
  
  for (int ww = 0; ww < SOLO_MAX_WINDOWS; ww++)
  {
    if (!(spi->active_windows[ww]))
      continue;

    if (wwptr->parameter.linked_windows[ww] && !linked_windows[ww])
      dangling_links[ww] = true;
    else
      dangling_links[ww] = false;

    wwptr->parameter.linked_windows[ww] = linked_windows[ww] ? 1 : 0;
  }

  // Now copy this parameter struct to the other linked windows

  for (int ww = 0; ww < SOLO_MAX_WINDOWS; ww++)
  {
    if (!spi->active_windows[ww])
      continue;

    if (ww == _frameIndex || !wwptr->parameter.linked_windows[ww])
      continue;

    WW_PTR wwptrc = solo_return_wwptr(ww); /* pointer to next linked frame */
    wwptrc->parameter = wwptr->parameter;
    wwptrc->parameter.window_num = ww;
  }

  // Now deal with the dangling links

  for (int ww = 0; ww < SOLO_MAX_WINDOWS; ww++)
  {
    if (!spi->active_windows[ww])
      continue;

    if (!dangling_links[ww])
      continue;

    // The dangling list becomes the links for the dangling frames

    WW_PTR wwptrc = solo_return_wwptr(ww);
    for (int jj = 0; jj < SOLO_MAX_WINDOWS; jj++)
      wwptrc->parameter.linked_windows[jj] = dangling_links[jj] ? 1 : 0;
  }

  update();

  return true;
}


/**********************************************************************
 * setTestColor()
 */

void ParameterWindow::setTestColor(const std::string color_name)
{
  // Get the color from the manager

  GdkColor *test_color =
    ColorManager::getInstance()->getColor(color_name.c_str());

  // Now draw the rectangle

  Glib::RefPtr< Gdk::Window > window = _colorTest.get_window();
  CRectangle rectangle(window->gobj(), true, 0, 0,
		       _colorTest.get_width(), _colorTest.get_height());
  rectangle.Graphic::SetForegroundColor(test_color);
  rectangle.Draw();
}


/**********************************************************************
 * toggleField()
 */

void ParameterWindow::toggleField()
{
  ParamData *pd = frame_configs[_frameIndex]->param_data;
  SiiLinkedList *sll = pd->toggle_field;

  if (sll == 0)
    return;

  ParamFieldInfo *pfi = (ParamFieldInfo *)sll->data;
  _paramNameEntry.set_text(pfi->name);
  if (setParamInfo())
    sii_plot_data(_frameIndex, REPLOT_THIS_FRAME);

  if (_colorPalettesWindow != 0)
    _colorPalettesWindow->updatePaletteList();
  
  pd->toggle_field = pd->fields_list->previous;
}


/**********************************************************************
 * update()
 */

void ParameterWindow::update()
{
  ParamData *pd = frame_configs[_frameIndex]->param_data;

  WW_PTR wwptr = solo_return_wwptr(_frameIndex);
  pd->toggle[PARAM_CB_SYMBOLS] = (wwptr->color_bar_symbols) ? TRUE : FALSE;

  if (wwptr->color_bar_location != 0)
    pd->cb_loc = (wwptr->color_bar_location > 0) ?
      PARAM_CB_RIGHT : PARAM_CB_LEFT;
  else
    pd->cb_loc = PARAM_CB_BOTTOM;

  // Parameter name

  std::string param_name = wwptr->parameter.parameter_name;
  _setParameterName(param_name);

  // Palette name

  std::string palette_name = wwptr->parameter.palette_name;
  _setPaletteName(palette_name);

  // Set the palette list

  setEntriesFromPalette(param_name, palette_name);

  // Number of colors

  _setNumColors(pd->pal->getNumColors());
   
  // Links

  LinksInfo *li = frame_configs[_frameIndex]->link_set[LI_PARAM];
  for (guint jj = 0; jj < MAX_FRAMES; jj++)
    li->link_set[jj] = (wwptr->parameter.linked_windows[jj]) ? TRUE : FALSE;

  // Parameter list.  solo_gen_parameter_list() puts the current list of
  // parameters into the spi structure.

  solo_gen_parameter_list(_frameIndex);
  struct solo_perusal_info *spi = solo_return_winfo_ptr();
  sort(spi->list_parameters.begin(), spi->list_parameters.end());

  setParameterNamesList(spi->list_parameters);
  pd->param_names_list = spi->list_parameters;
    
  // Palettes list

  if (_colorPalettesWindow != 0)
    _colorPalettesWindow->updatePaletteList();
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _broadcastAnnotationColor()
 */

void ParameterWindow::_broadcastAnnotationColor()
{
  ParamData *param_data = frame_configs[_frameIndex]->param_data;
  
  // Set the annotation color for all of the palettes to match the color
  // used for this frame's palette.

  PaletteManager::getInstance()->broadcastAnnotationColor(param_data->pal);
  
  // Update the highlight labels toggle in each of the other frames

  for (gint curr_frame_num = 0; curr_frame_num < MAX_FRAMES; curr_frame_num++)
  {
    if (curr_frame_num == _frameIndex)
      continue;

    // Update the highlight labels toggle

    ParameterWindow *curr_param_window =
      frame_configs[curr_frame_num]->param_window;
    
    if (curr_param_window != 0)
      curr_param_window->setHighlightLabels(isHighlightLabels());
  }
}


/**********************************************************************
 * _broadcastBackgroundColor()
 */

void ParameterWindow::_broadcastBackgroundColor()
{
  ParamData *pd = frame_configs[_frameIndex]->param_data;
  
  PaletteManager::getInstance()->broadcastBackgroundColor(pd->pal);
}


/**********************************************************************
 * _broadcastBoundaryColor()
 */

void ParameterWindow::_broadcastBoundaryColor()
{
  ParamData *pd = frame_configs[_frameIndex]->param_data;

  PaletteManager::getInstance()->broadcastBoundaryColor(pd->pal);
}


/**********************************************************************
 * _broadcastColorBarSettings()
 */

void ParameterWindow::_broadcastColorBarSettings()
{
  ParamData *pd = frame_configs[_frameIndex]->param_data;
  
  for (gint jj = 0; jj < MAX_FRAMES; jj++)
  {
    if (jj == _frameIndex)
      continue;

    ParamData *curr_param_data = frame_configs[jj]->param_data;
    curr_param_data->cb_loc = pd->cb_loc;

    ParameterWindow *curr_param_window = frame_configs[jj]->param_window;
    if (curr_param_window != 0)
    {
      if (pd->cb_loc == PARAM_CB_BOTTOM)
	curr_param_window->setColorbarBottom();
      else if (pd->cb_loc == PARAM_CB_LEFT)
	curr_param_window->setColorbarLeft();
      else if (pd->cb_loc == PARAM_CB_RIGHT)
	curr_param_window->setColorbarRight();
    }
    
    solo_return_wwptr(jj)->parameter.changed = 1;
    setParamInfo();
  }
}


/**********************************************************************
 * _broadcastEmphasisColor()
 */

void ParameterWindow::_broadcastEmphasisColor()
{
  ParamData *pd = frame_configs[_frameIndex]->param_data;
  
  PaletteManager::getInstance()->broadcastEmphasisColor(pd->pal);
}


/**********************************************************************
 * _broadcastExceededColor()
 */

void ParameterWindow::_broadcastExceededColor()
{
  ParamData *pd = frame_configs[_frameIndex]->param_data;

  PaletteManager::getInstance()->broadcastExceededColor(pd->pal);
}


/**********************************************************************
 * _broadcastGridColor()
 */

void ParameterWindow::_broadcastGridColor()
{
  ParamData *pd = frame_configs[_frameIndex]->param_data;
  
  PaletteManager::getInstance()->broadcastGridColor(pd->pal);
}


/**********************************************************************
 * _broadcastMissingDataColor()
 */

void ParameterWindow::_broadcastMissingDataColor()
{
  ParamData *pd = frame_configs[_frameIndex]->param_data;
  
  PaletteManager::getInstance()->broadcastMissingDataColor(pd->pal);
}


/**********************************************************************
 * _centerIncrActivated()
 */

void ParameterWindow::_centerIncrActivated()
{
  // Get a pointer to the parameter data

  ParamData *pd = frame_configs[_frameIndex]->param_data;

  // Get the user's entries for both the center and increment fields, even
  // though only one might have been changed.  We do this because both values
  // are needed for updating the related fields.

  double center_value = _getColorbarCenter();
  double incr_value = _getColorbarIncrement();

  // Update the min and max values based on the entered center and increment
  // values

  double min_value;
  double max_value;
  
  _minMaxFromCenterIncr(pd->pal->getNumColors(), center_value, incr_value,
			min_value, max_value);

  // Reset the display values based on the calculated min and max values.
  // Note that the center/increment values could have changed above so they
  // must also be updated.

  _displayColorbarValues(center_value, incr_value, min_value, max_value);
}


/**********************************************************************
 * _centerIncrFromMinMax()
 */

void ParameterWindow::_centerIncrFromMinMax(const int ncolors,
					    double &center, double &incr,
					    double &min, double &max)
{
  double tmp;

  if (min > max)
  {
    tmp = min;
    min = max;
    max = tmp;
  }
   
  incr = (max - min)/ncolors + CB_ROUND;
  center = min + (incr * ncolors * 0.5);
  center += (center < 0) ? -CB_ROUND : CB_ROUND;
}


/**********************************************************************
 * _closeWindows()
 */

void ParameterWindow::_closeWindows()
{
  if (_colorPalettesWindow != 0)
    _colorPalettesWindow->hide();
  if (_colorTablesWindow != 0)
    _colorTablesWindow->hide();
  if (_colorNamesWindow != 0)
    _colorNamesWindow->hide();
  if (_importColorTableWindow != 0)
    _importColorTableWindow->hide();
  
  hide();
}


/**********************************************************************
 * _createMenubar()
 */

void ParameterWindow::_createMenubar(Gtk::Box &container)
{
  // Specify the XML for the menus

  Glib::ustring ui_info =
    "<ui>"
    "  <menubar name='MenuBar'>"
    "    <menu action='FileMenu'>"
    "      <menuitem action='FileClose'/>"
    "      <separator/>"
    "      <menuitem action='FileListPalettes'/>"
    "      <menuitem action='FileListColorTables'/>"
    "      <menuitem action='FileListColors'/>"
    "      <separator/>"
    "      <menuitem action='FileImportColorTable'/>"
    "      <separator/>"
    "      <menu action='FileBroadcastMenu'>"
    "        <menuitem action='FileBroadcastGridColor'/>"
    "        <menuitem action='FileBroadcastBoundaryColor'/>"
    "        <menuitem action='FileBroadcastExceededColor'/>"
    "        <menuitem action='FileBroadcastMissingDataColor'/>"
    "        <menuitem action='FileBroadcastAnnotationColor'/>"
    "        <menuitem action='FileBroadcastBackgroundColor'/>"
    "        <menuitem action='FileBroadcastEmphasisColor'/>"
    "        <menuitem action='FileBroadcastColorBarSettings'/>"
    "      </menu>"
    "    </menu>"
    "    <menu action='OptionsMenu'>"
    "      <menu action='OptionsColorBarMenu'>"
    "        <menuitem action='OptionsColorBarBottom'/>"
    "        <menuitem action='OptionsColorBarLeft'/>"
    "        <menuitem action='OptionsColorBarRight'/>"
    "      </menu>"
    "      <separator/>"
    "      <menuitem action='OptionsUseSymbols'/>"
    "      <menuitem action='OptionsHiLightLabels'/>"
    "      <menuitem action='OptionsElectricParams'/>"
    "    </menu>"
    "    <menu action='ReplotMenu'>"
    "      <menuitem action='ReplotReplotLinks'/>"
    "      <menuitem action='ReplotReplotAll'/>"
    "    </menu>"
    "    <menu action='ParamLinksMenu'>"
    "      <menuitem action='ParamLinksSetLinks'/>"
    "    </menu>"
    "    <menu action='HelpMenu'>"
    "      <menuitem action='HelpOverview'/>"
    "      <menuitem action='HelpFile'/>"
    "      <menuitem action='HelpOptions'/>"
    "      <menuitem action='HelpParamLinks'/>"
    "      <menuitem action='HelpPalettes'/>"
    "      <menuitem action='HelpMinMax'/>"
    "    </menu>"
    "  </menubar>"
    "</ui>";

  // Create the actions for the menus

  _actionGroup = Gtk::ActionGroup::create();
  
  // File menu actions

  _actionGroup->add(Gtk::Action::create("FileMenu", "File"));
  _actionGroup->add(Gtk::Action::create("FileClose", "Close"),
		    sigc::mem_fun(*this,
				  &ParameterWindow::hide));

  _actionGroup->add(Gtk::Action::create("FileListPalettes", "List Palettes"),
		    sigc::mem_fun(*this,
				  &ParameterWindow::_listPalettes));
  _actionGroup->add(Gtk::Action::create("FileListColorTables",
					"List Color Tables"),
		    sigc::mem_fun(*this,
				  &ParameterWindow::_listColorTables));
  _actionGroup->add(Gtk::Action::create("FileListColors", "List Colors"),
		    sigc::mem_fun(*this,
				  &ParameterWindow::_listColors));

  _actionGroup->add(Gtk::Action::create("FileImportColorTable",
					"Import a Color Table"),
		    sigc::mem_fun(*this,
				  &ParameterWindow::_importColorTable));

  _actionGroup->add(Gtk::Action::create("FileBroadcastMenu",
					"Broadcast..."));
  _actionGroup->add(Gtk::Action::create("FileBroadcastGridColor",
					"Grid Color"),
		    sigc::mem_fun(*this,
				  &ParameterWindow::_broadcastGridColor));
  _actionGroup->add(Gtk::Action::create("FileBroadcastBoundaryColor",
					"Boundary Color"),
		    sigc::mem_fun(*this,
				  &ParameterWindow::_broadcastBoundaryColor));
  _actionGroup->add(Gtk::Action::create("FileBroadcastExceededColor",
					"Exceeded Color"),
		    sigc::mem_fun(*this,
				  &ParameterWindow::_broadcastExceededColor));
  _actionGroup->add(Gtk::Action::create("FileBroadcastMissingDataColor",
					"Missing Data Color"),
		    sigc::mem_fun(*this,
				  &ParameterWindow::_broadcastMissingDataColor));
  _actionGroup->add(Gtk::Action::create("FileBroadcastAnnotationColor",
					"Annotation Color"),
		    sigc::mem_fun(*this,
				  &ParameterWindow::_broadcastAnnotationColor));
  _actionGroup->add(Gtk::Action::create("FileBroadcastBackgroundColor",
					"Background Color"),
		    sigc::mem_fun(*this,
				  &ParameterWindow::_broadcastBackgroundColor));
  _actionGroup->add(Gtk::Action::create("FileBroadcastEmphasisColor",
					"Emphasis Color"),
		    sigc::mem_fun(*this,
				  &ParameterWindow::_broadcastEmphasisColor));
  _actionGroup->add(Gtk::Action::create("FileBroadcastColorBarSettings",
					"Color Bar Settings"),
		    sigc::mem_fun(*this,
				  &ParameterWindow::_broadcastColorBarSettings));

  // Options menu actions

  _actionGroup->add(Gtk::Action::create("OptionsMenu", "Options"));

  _actionGroup->add(Gtk::Action::create("OptionsColorBarMenu",
					"Color Bar..."));
  Gtk::RadioAction::Group group_color_bar_loc;
  _colorBarBottomChoice = Gtk::RadioAction::create(group_color_bar_loc,
					     "OptionsColorBarBottom",
						   "At Bottom");
  _actionGroup->add(_colorBarBottomChoice,
		    sigc::mem_fun(*this,
				  &ParameterWindow::_setColorBarLocation));
  _colorBarLeftChoice = Gtk::RadioAction::create(group_color_bar_loc,
					     "OptionsColorBarLeft",
						 "At Left");
  _actionGroup->add(_colorBarLeftChoice,
		    sigc::mem_fun(*this,
				  &ParameterWindow::_setColorBarLocation));
  _colorBarRightChoice = Gtk::RadioAction::create(group_color_bar_loc,
					     "OptionsColorBarRight",
						  "At Right");
  _actionGroup->add(_colorBarRightChoice,
		    sigc::mem_fun(*this,
				  &ParameterWindow::_setColorBarLocation));
  
  _colorbarSymbolsToggle =
    Gtk::ToggleAction::create("OptionsUseSymbols",
			      "Use Symbols", "", false);
  _actionGroup->add(_colorbarSymbolsToggle);
  _highlightLabelsToggle = Gtk::ToggleAction::create("OptionsHiLightLabels",
						     "HiLight Labels", "",
						     true);
  _actionGroup->add(_highlightLabelsToggle);
  _electricParamsToggle = Gtk::ToggleAction::create("OptionsElectricParams",
						    "Electric Params", "",
						    true);
  _actionGroup->add(_electricParamsToggle);

  // Replot menu actions

  _actionGroup->add(Gtk::Action::create("ReplotMenu", "Replot"));
  _actionGroup->add(Gtk::Action::create("ReplotReplotLinks", "Replot Links"),
		    sigc::mem_fun(*this,
				  &ParameterWindow::_replotLinks));
  _actionGroup->add(Gtk::Action::create("ReplotReplotAll", "Replot All"),
		    sigc::mem_fun(*this,
				  &ParameterWindow::_replotAll));

  // ParamLinks menu actions

  _actionGroup->add(Gtk::Action::create("ParamLinksMenu", "ParamLinks"));
  _actionGroup->add(Gtk::Action::create("ParamLinksSetLinks", "Set Links"),
		    sigc::mem_fun(*this,
				  &ParameterWindow::_setLinks));

  // Help menu actions

  _actionGroup->add(Gtk::Action::create("HelpMenu", "Help"));
  _actionGroup->add(Gtk::Action::create("HelpOverview", "Overview"),
		    sigc::mem_fun(*this,
				  &ParameterWindow::_displayHelpOverview));
  _actionGroup->add(Gtk::Action::create("HelpFile", "File"),
		    sigc::mem_fun(*this,
				  &ParameterWindow::_displayHelpFile));
  _actionGroup->add(Gtk::Action::create("HelpOptions", "Options"),
		    sigc::mem_fun(*this,
				  &ParameterWindow::_displayHelpOptions));
  _actionGroup->add(Gtk::Action::create("HelpParamLinks", "ParamLinks"),
		    sigc::mem_fun(*this,
				  &ParameterWindow::_displayHelpParamLinks));
  _actionGroup->add(Gtk::Action::create("HelpPalettes", "Palettes"),
		    sigc::mem_fun(*this,
				  &ParameterWindow::_displayHelpPalettes));
  _actionGroup->add(Gtk::Action::create("HelpMinMax", "Min & Max"),
		    sigc::mem_fun(*this,
				  &ParameterWindow::_displayHelpMinMax));

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
 * _displayColorbarValues()
 */

void ParameterWindow::_displayColorbarValues(const double center_value,
					     const double incr_value,
					     const double min_value,
					     const double max_value)
{
  // When this is called, the center/increment/min/max values have already
  // been made consistent, so we can just update the entry values and the
  // orig values.

  _setColorbarCenter(center_value);
  _setColorbarIncrement(incr_value);
  _setColorbarMin(min_value);
  _setColorbarMax(max_value);
}


/**********************************************************************
 * _displayHelpFile()
 */

void ParameterWindow::_displayHelpFile()
{
  static const Glib::ustring help_text =
    Glib::ustring("The \"File\" menu enables closing or cancelling the Parameter widget\n") +
    "and displaying lists of palettes, color tables, and colors. Clicking\n" +
    "in any of these lists selects an entry. With the exception of colors\n" +
    "this causes an immediate change in the entries in this widget.\n" +
    "\n" +
    "A clicked color name is placed in the clipboard and can be used to\n" +
    "replace any color entry in the palette by clicking in the entry and\n" +
    "typing Ctrl+V.\n" +
    "\n" +
    "For \"Import a Color Table\" the FileSelect widget can be\n" +
    "initialized with the \"COLORS_FILESEL\" environment variable.\n" +
    "\n" +
    "Under \"Broadcast...\" there are options to broadcast the grid color,\n" +
    "the boundary color and color bar orientation to all the other windows.\n" +
    "Before broadcasting make sure the source frame for broadcasting has\n" +
    "been replotted with any new settings since hardware colors are\n" +
    "established only during plotting.\n";

  sii_message(help_text);
}


/**********************************************************************
 * _displayHelpMinMax()
 */

void ParameterWindow::_displayHelpMinMax()
{
  static const Glib::ustring help_text =
    Glib::ustring("Solo3 always uses the center, increment and the number of colors\n") +
    "to define the limits and resolution of the data displayed. Typing\n" +
    "a min. and max. values in the \"Min & Max\" entry and hitting\n" +
    "<return> causes the center and increment to be adjusted to fit\n" +
    "the min. and max. The center and increment can then be adjusted\n" +
    "if necessary to provide more desirable labeling.\n";

  sii_message(help_text);
}


/**********************************************************************
 * _displayHelpOptions()
 */

void ParameterWindow::_displayHelpOptions()
{
  static const Glib::ustring help_text =
    Glib::ustring("The \"Options\" menu enables choosing the location of the color bar,\n") +
    "assigning a set of symbols to the colors of the color bar, removing\n" +
    "highlighting from the title and colorbar labels, and immediately\n" +
    "plotting the clicked field in the \"Parameters\" list.\n" +
    "\n" +
    "There are two sets of symbols currently in place. One for the \"PD\"\n" +
    "and \"WPD\" fields and another for the fields \"RR_DXD,RNX,RZD,RKD\"\n" +
    "which are log10 of the rain rate. The symbols are the associated linear\n" +
    "values which give the appearance of a log scale.\n" +
    "\n" +
    "The particle fields symbol set assumes a 17 color table and the\n" +
    "rain rate fields an 11 color table. The particle symbols have the\n" +
    "following meanings:\n" +
    "\n" +
    "cld  = Cloud                          \n" +
    "drz  = Drizzle                         \n" +
    "lrn  = Light Rain                      \n" +
    "mrn  = Moderate Rain                   \n" +
    "hrn  = Heavy Rain                      \n" +
    "hail = Hail                            \n" +
    "rhm  = Rain Hail Mixture               \n" +
    "gsh  = Graupel Small Hail              \n" +
    "grn  = Graupel Rain                    \n" +
    "dsn  = Dry Snow                        \n" +
    "wsn  = Wet Snow                        \n" +
    "icr  = Ice Crystals                    \n" +
    "iicr = Irreg Ice Crystals              \n" +
    "sld  = Super Cooled Liquid Droplets    \n" +
    "bgs  = Insects                         \n" +
    "2tr  = 2nd Trip Echoes (positive Ldr only)                \n" +
    "gclt = Ground Clutter                  \n";

  sii_message(help_text);
}


/**********************************************************************
 * _displayHelpOverview()
 */

void ParameterWindow::_displayHelpOverview()
{
  static const Glib::ustring help_text =
    Glib::ustring("The colors that are used to depict a data field are controlled by a\n") +
    "palette. This widget enables adjusting the palette for each field.\n" +
    "\n" +
    "The exceeded color is assigned to data values beyond the limits of\n" +
    "the color bar. The missing data color is assigned to data values\n" +
    "flagged as missing or bad.\n" +
    "\n" +
    "Entries containing two values must have the values seperated by a\n" +
    "comma and/or a space.\n" +
    "\n" +
    "When the \"Emphasis Min/Max\" values are not equal, the emphasis\n" +
    "color is assigned to data values within the emphasis limits.\n";

  sii_message(help_text);
}


/**********************************************************************
 * _displayHelpPalettes()
 */

void ParameterWindow::_displayHelpPalettes()
{
  static const Glib::ustring help_text =
    Glib::ustring("A palette contains most of the information you see in this widget\n") +
    "with the exception of the parameters list and the parameter name.\n" +
    "In addition it contains a list of fields that are likely to use\n" +
    "this palette. This list appears next to the palette name in the\n" +
    "palettes list.\n" +
    "\n" +
    "If you wish to define a new palette for a parameter, change all\n" +
    "items you wish to be different, type a new name in the\n" +
    "\"Color Palette\" entry and hit <return>.\n" +
    "\n" +
    "The last palette selected or modified is always at the top of the\n" +
    "list of palettes. Since a field can exist is more than one palette,\n" +
    "the search for a palette to be associated with a field is always\n" +
    "based on the first encounter of a field.\n";

  sii_message(help_text);
}


/**********************************************************************
 * _displayHelpParamLinks()
 */

void ParameterWindow::_displayHelpParamLinks()
{
  static const Glib::ustring help_text =
    Glib::ustring("As a default the parameter information is not linked to any other\n") +
    "frame.  Links are useful when you are plotting the same field from\n" +
    "different sweepfiles and want to be able to change the field in\n" +
    "several frames at once.\n";

  sii_message(help_text);
}


/**********************************************************************
 * _displayOrigValues()
 */

void ParameterWindow::_displayOrigValues()
{
  _resetParameterName();
  _resetPaletteName();
  _resetColorbarMin();
  _resetColorbarMax();
  _resetColorbarCenter();
  _resetColorbarIncrement();
  _resetGridColor();
  _resetColorTable();
  _resetBoundaryColor();
  _resetExceededColor();
  _resetMissingDataColor();
  _resetAnnotationColor();
  _resetBackgroundColor();
  _resetEmphasisColor();
  _resetEmphasisMin();
  _resetEmphasisMax();
}


/**********************************************************************
 * _doItAndHide()
 */

void ParameterWindow::_doItAndHide()
{
  // Implement the changes made by the user

  setParamInfo();

  // Hide the window

  hide();
}


/**********************************************************************
 * _importColorTable()
 */

void ParameterWindow::_importColorTable()
{
  if (_importColorTableWindow == 0)
    _importColorTableWindow = new ImportColorTableWindow(this, _defaultFont);
  
  _importColorTableWindow->show_all();
}


/**********************************************************************
 * _listColors()
 */

void ParameterWindow::_listColors()
{
  if (_colorNamesWindow == 0)
    _colorNamesWindow = new ColorNamesWindow(this, _defaultFont);
  
  _colorNamesWindow->show_all();
}


/**********************************************************************
 * _listColorTables()
 */

void ParameterWindow::_listColorTables()
{
  if (_colorTablesWindow == 0)
    _colorTablesWindow = new ColorTablesWindow(this, _defaultFont);
  
  _colorTablesWindow->show_all();
}


/**********************************************************************
 * _listPalettes()
 */

void ParameterWindow::_listPalettes()
{
  if (_colorPalettesWindow == 0)
    _colorPalettesWindow = new ColorPalettesWindow(this, _defaultFont);
  
  _colorPalettesWindow->show_all();
}


/**********************************************************************
 * _minMaxActivated()
 */

void ParameterWindow::_minMaxActivated()
{
  // Get a pointer to the parameter data

  ParamData *pd = frame_configs[_frameIndex]->param_data;

  // Get the user's entries for both the min and max fields, even though only
  // one might have been changed.  We do this because both values are needed
  // for updating the related fields.

  double min_value = _getColorbarMin();
  double max_value = _getColorbarMax();

  // Update the center and increment values based on the entered min/max
  // values

  double center_value;
  double incr_value;
  
  _centerIncrFromMinMax(pd->pal->getNumColors(),
			center_value, incr_value,
			min_value, max_value);

  // Reset the displayed values based on the calculated center and increment.
  // Note that the min/max values could have changed above if the min value
  // entered was greater than the max value.

  _displayColorbarValues(center_value, incr_value,
			 min_value, max_value);
}


/**********************************************************************
 * _minMaxFromCenterIncr()
 */

void ParameterWindow::_minMaxFromCenterIncr(const int ncolors,
					    const double center,
					    const double incr,
					    double &min, double &max )
{
  min = center - (incr * ncolors * 0.5);
  max = min + (incr * ncolors);

  min += (min < 0) ? -CB_ROUND : CB_ROUND;
  max += (max < 0) ? -CB_ROUND : CB_ROUND;
}


/**********************************************************************
 * _paletteActivated()
 */

void ParameterWindow::_paletteActivated()
{
  // Get the user's entry

  PaletteManager::getInstance()->newPaletteForParam(_paletteEntry.get_text(),
						    _paramNameEntry.get_text());
  processChanges();

  if (_colorPalettesWindow != 0)
    _colorPalettesWindow->updatePaletteList();
}


/**********************************************************************
 * _paramNameActivated()
 */

void ParameterWindow::_paramNameActivated()
{
  // Get the user's entry

  Glib::ustring param_name = _paramNameEntry.get_text();

  // If the user didn't put anything in the field, don't do anything

  if (param_name == "")
    return;
  
  processChanges();
//  sii_redisplay_list(_frameIndex, PARAM_PALETTES_TEXT);
}


/**********************************************************************
 * _replotAll()
 */

void ParameterWindow::_replotAll()
{
  if (setParamInfo())
    sii_plot_data(_frameIndex, REPLOT_ALL);
}


/**********************************************************************
 * _replotLinks()
 */

void ParameterWindow::_replotLinks()
{
  if (setParamInfo())
    sii_plot_data(_frameIndex, REPLOT_LOCK_STEP);
}


/**********************************************************************
 * _replotThisFrame()
 */

void ParameterWindow::_replotThisFrame()
{
  if (setParamInfo())
    sii_plot_data(_frameIndex, REPLOT_THIS_FRAME);
}


/**********************************************************************
 * _selectParameterName()
 */

void ParameterWindow::_selectParameterName(const Gtk::TreeModel::Path &path,
					   Gtk::TreeViewColumn *column)
{
  int row_number = atoi(path.to_string().c_str());
  std::string selected_param = _paramNames.get_text(row_number);
  
  // Process the selection

  _paramNameEntry.set_text(selected_param);
  
  if (isElectricParams())
    processChanges();
}


/**********************************************************************
 * _setColorbarCenterIncrement()
 */

void ParameterWindow::_setColorbarCenterIncrement(const double center,
						  const double incr)
{
  _origCenter = center;
  _origIncrement = incr;
  
  ParamData *pd = frame_configs[_frameIndex]->param_data;
  _minMaxFromCenterIncr(pd->pal->getNumColors(),
			_origCenter, _origIncrement, _origMin, _origMax);
  
  _resetColorbarCenter();
  _resetColorbarIncrement();
  _resetColorbarMin();
  _resetColorbarMax();
}


/**********************************************************************
 * _setColorBarLocation()
 */

void ParameterWindow::_setColorBarLocation()
{
  ParamData *pd = frame_configs[_frameIndex]->param_data;
  
  if (_colorBarBottomChoice->get_active())
  {
    pd->cb_loc = PARAM_CB_BOTTOM;
  
    pd->toggle[PARAM_CB_BOTTOM] = TRUE;
    pd->toggle[PARAM_CB_LEFT] = FALSE;
    pd->toggle[PARAM_CB_RIGHT] = FALSE;
  }
  else if (_colorBarLeftChoice->get_active())
  {
    pd->cb_loc = PARAM_CB_LEFT;
  
    pd->toggle[PARAM_CB_BOTTOM] = FALSE;
    pd->toggle[PARAM_CB_LEFT] = TRUE;
    pd->toggle[PARAM_CB_RIGHT] = FALSE;
  }
  else if (_colorBarRightChoice->get_active())
  {
    pd->cb_loc = PARAM_CB_RIGHT;
  
    pd->toggle[PARAM_CB_BOTTOM] = FALSE;
    pd->toggle[PARAM_CB_LEFT] = FALSE;
    pd->toggle[PARAM_CB_RIGHT] = TRUE;
  }
}


/**********************************************************************
 * _setLinks()
 */

void ParameterWindow::_setLinks()
{
  show_links_widget(frame_configs[_frameIndex]->param_data->param_links); 
}
