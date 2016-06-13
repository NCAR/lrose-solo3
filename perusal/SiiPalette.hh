#ifndef SiiPalette_hh
#define SiiPalette_hh

#include <algorithm>
#include <iostream>
#include <string.h>
#include <vector>

#include <glibmm/ustring.h>
#include <gdkmm/color.h>

#define MAX_COLOR_TABLE_SIZE 128
#define SIZEOF_COLOR_NAMES 32

class SiiPalette
{

public:

  //////////////////
  // Public types //
  //////////////////

  typedef enum
  {
    FEATURE_ZERO,
    FEATURE_RNG_AZ_GRID,
    FEATURE_TIC_MARKS,
    FEATURE_BACKGROUND,
    FEATURE_ANNOTATION,
    FEATURE_EXCEEDED,
    FEATURE_MISSING,
    FEATURE_EMPHASIS,
    FEATURE_BND,
    FEATURE_OVERLAY1,
    FEATURE_OVERLAY2,
    FEATURE_OVERLAY3,
    FEATURE_OVERLAY4,
    FEATURE_OVERLAY5,
    MAX_FEATURE_COLORS
  } features_t;

  /**
   * @brief Constructor
   */

  SiiPalette();
  
  /**
   * @brief Constructor
   *
   * @param[in] palette_name        The name of the palette.
   * @param[in] usual_parms         The usual parameters.
   */

  SiiPalette(const Glib::ustring &palette_name,
	     const Glib::ustring &usual_parms);
  
  /**
   * @brief Constructor
   *
   * @param[in] palette_name        The name of the palette.
   * @param[in] usual_parms         The usual parameters.
   * @param[in] center_data_value   The data value at the center of the palette.
   * @param[in] color_width         The width of each color in parameter units.
   * @param[in] color_table_name    The color table name.
   */

  SiiPalette(const Glib::ustring &palette_name,
	     const Glib::ustring &usual_parms,
	     const double center_data_value, const double color_width,
	     const Glib::ustring &color_table_name);
  
  /**
   * @brief Destructor
   */

  virtual ~SiiPalette();
  

  // Input/Output methods //

  /**
   * @brief Read the palette information from the specified buffer.
   *
   * @param[in] buf_ptr      Pointer to the input buffer.
   * @param[in] gotta_swap   Flag indicating whether we need to swap the data.
   */

  void read(char *buf_ptr, const bool gotta_swap);
  

  /**
   * @brief Write the palette information to the specified stream.
   *
   * @param[in,out] stream   The output stream.
   *
   * @return Returns true on success, false on failure.
   */

  bool write(FILE *stream) const;
  

  // Access methods //

  /////////////////
  // get methods //
  /////////////////

  /**
   * @brief Get the palette name.
   *
   * @return Returns the palette name.
   */

  inline Glib::ustring getPaletteName() const
  {
    return _paletteName;
  }
  
  /**
   * @brief Get the usual parameters list.
   *
   * @return Returns the usual parameters list.
   */

  inline Glib::ustring getUsualParametersStr() const
  {
    Glib::ustring param_string;
    
    std::vector< Glib::ustring >::const_iterator param;
    for (param = _usualParams.begin(); param != _usualParams.end(); ++param)
      param_string += *param + ",";
    
    return param_string;
  }
  
  /**
   * @brief See if the indicated parameter is included in the usual parameters.
   *
   * @param[in] parameter  The parameter to look for.
   *
   * @return Returns true if this parameter is in the usual parameters, false
   *         otherwise.
   */

  inline bool isParameterIncluded(const Glib::ustring parameter) const
  {
    if (std::find(_usualParams.begin(), _usualParams.end(), parameter) ==
	_usualParams.end())
      return false;
    
    return true;
  }
  
  /**
   * @brief Get the color table name.
   *
   * @return Returns the color table name.
   */

  inline Glib::ustring getColorTableName() const
  {
    return _colorTableName;
  }
  
  /**
   * @brief Get the number of colors in the color table.
   *
   * @return Returns the number of colors in the color table.
   */

  inline gint getNumColors() const
  {
    return _numColors;
  }
  
  /**
   * @brief Get the data value at the center of the palette.
   *
   * @return Returns the data value at the center of the palette.
   */

  inline gfloat getCenterDataValue() const
  {
    return _centerDataValue;
  }
  
  /**
   * @brief Get the width of each color in the palette in data units.
   *
   * @return Returns the width of each color in the palette.
   */

  inline gfloat getColorWidth() const
  {
    return _colorWidth;
  }
  
  /**
   * @brief Get the minimum data value in the palette.
   *
   * @return Returns the minimum data value in the palette.
   */

  inline gfloat getMinValue() const
  {
    return _minValue;
  }
  
  /**
   * @brief Get the maximum data value in the palette.
   *
   * @return Returns the maximum data value in the palette.
   */

  inline gfloat getMaxValue() const
  {
    return _maxValue;
  }
  
  /**
   * @brief Get the lower bound of the emphasis zone.
   *
   * @return Returns the lower bound of the emphasis zone.
   */

  inline gfloat getEmphasisZoneLower() const
  {
    return _emphasisZoneLower;
  }
  
  /**
   * @brief Get the upper bound of the emphasis zone.
   *
   * @return Returns the upper bound of the emphasis zone.
   */

  inline gfloat getEmphasisZoneUpper() const
  {
    return _emphasisZoneUpper;
  }
  
  /**
   * @brief Get the grid color.
   *
   * @return Returns the grid color.
   */

  inline Glib::ustring getGridColor() const
  {
    return _gridColor;
  }
  
  /**
   * @brief Get the missing data color.
   *
   * @return Returns the missing data color.
   */

  inline Glib::ustring getMissingDataColor() const
  {
    return _missingDataColor;
  }
  
  /**
   * @brief Get the exceeded color.
   *
   * @return Returns the exceeded color.
   */

  inline Glib::ustring getExceededColor() const
  {
    return _exceededColor;
  }
  
  /**
   * @brief Get the annotation color.
   *
   * @return Returns the annotation color.
   */

  inline Glib::ustring getAnnotationColor() const
  {
    return _annotationColor;
  }
  
  /**
   * @brief Get the background color.
   *
   * @return Returns the background color.
   */

  inline Glib::ustring getBackgroundColor() const
  {
    return _backgroundColor;
  }
  
  /**
   * @brief Get the emphasis color.
   *
   * @return Returns the emphasis color.
   */

  inline Glib::ustring getEmphasisColor() const
  {
    return _emphasisColor;
  }
  
  /**
   * @brief Get the boundary color.
   *
   * @return Returns the boundary color.
   */

  inline Glib::ustring getBoundaryColor() const
  {
    return _boundaryColor;
  }
  
  /**
   * @brief Get the boundary alert color.
   *
   * @return Returns the boundary alert color.
   */

  inline Glib::ustring getBoundaryAlertColor() const
  {
    return _boundaryAlertColor;
  }
  
  /**
   * @brief Get the boundary last line color.
   *
   * @return Returns the boundary last line color.
   */

  inline Glib::ustring getBoundaryLastLineColor() const
  {
    return _boundaryLastLineColor;
  }
  
  /**
   * @brief Get the indicated feature color.
   *
   * @param[in] feature_num   The index for the feature.
   *
   * @return Returns the indicated feature color.
   */

  inline const Gdk::Color &getFeatureColor(const int feature_num) const
  {
    return _featureColor[feature_num];
  }
  
  /**
   * @brief Get the indicated data color.
   *
   * @param[in] value   The scaled data value.
   *
   * @return Returns the indicated data color.
   */

  inline const Gdk::Color &getDataColor(const int value) const
  {
    return _dataColorTable[value];
  }
  
  /**
   * @brief Get the indicated data color table RGB values.
   *
   * @param[in] index   The index for the table.
   * @param[in] red     The red value.
   * @param[in] green   The green value.
   * @param[in] blue    The blue value.
   */

  inline void getColorTableRgb(const int index,
			       guchar &red, guchar &green,
			       guchar &blue) const
  {
    red = _colorTableRgbs[index][0];
    green = _colorTableRgbs[index][1];
    blue = _colorTableRgbs[index][2];
  }
  

  /////////////////
  // set methods //
  /////////////////

  /**
   * @brief Set the palette name.
   *
   * @param[in] name The palette name.
   */

  inline void setPaletteName(const Glib::ustring &name)
  {
    _paletteName = name;
  }
  
  /**
   * @brief Set the usual parameters list.
   *
   * @param[in] params The usual parameters list.
   */

  inline void setUsualParameters(const Glib::ustring &params)
  {
    // Clear out any existing entries in the vector

    _usualParams.clear();
    
    // Parse the param string and add the parameters to the vector

    gchar str[512];
    gchar *sptrs[64];
    
    strcpy(str, params.c_str());
    int num_tokens = _tokenize(str, sptrs, ",");
    
    for (int i = 0; i < num_tokens; ++i)
      _usualParams.push_back(sptrs[i]);
  }
  
  /**
   * @brief Append the given parameter to the usual parameters.  In this case,
   *        no checking is done to see if the parameter is already in the
   *        list.
   *
   * @param[in] new_parameter  The new parameter name.
   */

  void appendToUsualParameters(const Glib::ustring &new_parameter)
  {
    _usualParams.push_back(new_parameter);
  }
  
  /**
   * @brief Prepend the given parameter to the usual parameters.  If this
   *        parameter is already in the parameter list, move it from it's
   *        current location to the front of the list.
   *
   * @param[in] new_parameter  The new parameter name.
   */

  void prependToUsualParameters(const Glib::ustring &new_parameter);
  
  /**
   * @brief Remove the given parameter from the usual parameters.
   *
   * @param[in] parameter  The parameter to remove.
   */

  void removeFromUsualParameters(const Glib::ustring &parameter);
  
  /**
   * @brief Set the color table name.
   *
   * @param[in] name   The color table name.
   */

  inline void setColorTableName(const Glib::ustring &name)
  {
    _colorTableName = name;
  }
  
  /**
   * @brief Set the number of colors in the color table.
   *
   * @param[in] num_colors  The number of colors in the color table.
   */

  inline void setNumColors(const gint num_colors)
  {
    _numColors = num_colors;
  }
  
  /**
   * @brief Set the data value at the center of the palette.
   *
   * @param[in] value  The data value at the center of the palette.
   */

  inline void setCenterDataValue(const gfloat value)
  {
    _centerDataValue = value;
  }
  
  /**
   * @brief Set the width of each color in the palette in data units.
   *
   * @param[in] width  The width of each color in the palette.
   */

  inline void setColorWidth(const gfloat width)
  {
    _colorWidth = width;
  }
  
  /**
   * @brief Set the minimum data value in the palette.
   *
   * @param[in] value   The minimum data value in the palette.
   */

  inline void setMinValue(const gfloat value)
  {
    _minValue = value;
  }
  
  /**
   * @brief Set the maximum data value in the palette.
   *
   * @param[in] value   The maximum data value in the palette.
   */

  inline void setMaxValue(const gfloat value)
  {
    _maxValue = value;
  }
  
  /**
   * @brief Set the bounds of the emphasis zone.
   *
   * @param[in] lower   The lower bound of the emphasis zone.
   * @param[in] upper   The upper bound of the emphasis zone.
   */

  inline void setEmphasisZone(const gfloat lower, const gfloat upper)
  {
    _emphasisZoneLower = lower;
    _emphasisZoneUpper = upper;
  }
  
  /**
   * @brief Set the grid color.
   *
   * @param[in] color_name  The grid color name.
   */

  inline void setGridColor(const Glib::ustring &color_name)
  {
    _gridColor = color_name;
  }
  
  /**
   * @brief Set the missing data color.
   *
   * @param[in] color_name  The missing data color name.
   */

  inline void setMissingDataColor(const Glib::ustring &color_name)
  {
    _missingDataColor = color_name;
  }
  
  /**
   * @brief Set the exceeded color.
   *
   * @param[in] color_name  The exceeded color name.
   */

  inline void setExceededColor(const Glib::ustring &color_name)
  {
    _exceededColor = color_name;
  }
  
  /**
   * @brief Set the annotation color.
   *
   * @param[in] color_name  The annotation color name.
   */

  inline void setAnnotationColor(const Glib::ustring &color_name)
  {
    _annotationColor = color_name;
  }
  
  /**
   * @brief Set the background color.
   *
   * @param[in] color_name  The background color name.
   */

  inline void setBackgroundColor(const Glib::ustring &color_name)
  {
    _backgroundColor = color_name;
  }
  
  /**
   * @brief Set the emphasis color.
   *
   * @param[in] color_name  The emphasis color name.
   */

  inline void setEmphasisColor(const Glib::ustring &color_name)
  {
    _emphasisColor = color_name;
  }
  
  /**
   * @brief Set the boundary color.
   *
   * @param[in] color_name  The boundary color name.
   */

  inline void setBoundaryColor(const Glib::ustring &color_name)
  {
    _boundaryColor = color_name;
  }
  
  /**
   * @brief Set the boundary alert color.
   *
   * @param[in] color_name  The boundary alert color name.
   */

  inline void setBoundaryAlertColor(const Glib::ustring &color_name)
  {
    _boundaryAlertColor = color_name;
  }
  
  /**
   * @brief Set the boundary last line color.
   *
   * @param[in] color_name  The boundary last line color name.
   */

  inline void setBoundaryLastLineColor(const Glib::ustring &color_name)
  {
    _boundaryLastLineColor = color_name;
  }
  
  /**
   * @brief Set the indicated feature color.
   *
   * @param[in] feature_num   The index for the feature.
   * @param[in] color         The feature color.
   */

  inline void setFeatureColor(const int feature_num,
			      const Gdk::Color &color)
  {
    _featureColor[feature_num] = color;
  }
  
  /**
   * @brief Set the indicated feature RGB values.
   *
   * @param[in] feature_num   The index for the feature.
   * @param[in] red           The red value.
   * @param[in] green         The green value.
   * @param[in] blue          The blue value.
   */

  inline void setFeatureRgb(const int feature_num,
			    const guchar red, const guchar green,
			    const guchar blue)
  {
    _featureRgbs[feature_num][0] = red;
    _featureRgbs[feature_num][1] = green;
    _featureRgbs[feature_num][2] = blue;
  }
  
  /**
   * @brief Set the indicated data color.
   *
   * @param[in] value   The scaled data value.
   * @param[in] color   The data color.
   */

  inline void setDataColor(const int value,
			   const Gdk::Color &color)
  {
    _dataColorTable[value] = color;
  }
  
  /**
   * @brief Set the indicated data color table RGB values.
   *
   * @param[in] index   The index for the table.
   * @param[in] red     The red value.
   * @param[in] green   The green value.
   * @param[in] blue    The blue value.
   */

  inline void setColorTableRgb(const int index,
			       const guchar red, const guchar green,
			       const guchar blue)
  {
    _colorTableRgbs[index][0] = red;
    _colorTableRgbs[index][1] = green;
    _colorTableRgbs[index][2] = blue;
  }
  

protected:

  /////////////////////
  // Protected types //
  /////////////////////

  struct solo_palette {
    char name_struct[4];	/* "SPAL" */
    int32_t sizeof_struct;
    int32_t window_num;
    int32_t time_modified;
    int32_t changed;

    float center;
    float increment;
    float emphasis_zone_lower;
    float emphasis_zone_upper;
    float bnd_line_width_pix;
    int32_t num_colors;
    char usual_parms[128];
    char palette_name[16];
    char units[16];
    char color_table_dir[128];
    char color_table_name[128];
    char emphasis_color[SIZEOF_COLOR_NAMES];
    char exceeded_color[SIZEOF_COLOR_NAMES];
    char missing_data_color[SIZEOF_COLOR_NAMES];
    char background_color[SIZEOF_COLOR_NAMES];
    char annotation_color[SIZEOF_COLOR_NAMES];
    char grid_color[SIZEOF_COLOR_NAMES];
    /* boundary info */
    char bnd_color[SIZEOF_COLOR_NAMES];
    char bnd_alert[SIZEOF_COLOR_NAMES];
    char bnd_last_line[SIZEOF_COLOR_NAMES];
    char message[48];		/* display min/max colorbar vals */
  };

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief The name of the palette.
   */

  Glib::ustring _paletteName;

  /**
   * @brief The list of usual parameters using this palette.  This list is a
   *        string of the parameter names separated by commas.
   */

  std::vector< Glib::ustring > _usualParams;

  /**
   * @brief The name of the color table.
   */

  Glib::ustring _colorTableName;

  /**
   * @brief The number of colors in the table.
   */

  guint _numColors;

  /**
   * @brief The data value at the center of the color palette.
   */

  gfloat _centerDataValue;
  
  /**
   * @brief The width of each color in the palette in data units.
   */

  gfloat _colorWidth;
  
  /**
   * @brief Minimum data value for the palette.
   */

  gfloat _minValue;
  
  /**
   * @brief Maximum data value for the palette.
   */

  gfloat _maxValue;
  
  /**
   * @brief Lower bound of the emphasis zone.
   */

  gfloat _emphasisZoneLower;
  
  /**
   * @brief Upper bound of the emphasis zone.
   */

  gfloat _emphasisZoneUpper;
  
  /**
   * @brief The name of the color used for the grid overlay.
   */

  Glib::ustring _gridColor;

  /**
   * @brief The name of the color used to signal missing data.
   */

  Glib::ustring _missingDataColor;

  /**
   * @brief The name of the exceeded color.
   */

  Glib::ustring _exceededColor;

  /**
   * @brief The name of the color used for annotations.
   */

  Glib::ustring _annotationColor;

  /**
   * @brief The name of the background color.
   */

  Glib::ustring _backgroundColor;

  /**
   * @brief The name of the emphasis color.
   */

  Glib::ustring _emphasisColor;

  /**
   * @brief The name of the boundary color.
   */

  Glib::ustring _boundaryColor;
 
  /**
   * @brief The name of the boundary alert color.
   */

  Glib::ustring _boundaryAlertColor;
 
  /**
   * @brief The name of the boundary last line color.
   */

  Glib::ustring _boundaryLastLineColor;
 
  /**
   * @brief List of feature colors, in the order of the values in the features_t
   *        enumeration.
   */

  std::vector< Gdk::Color > _featureColor;

  guchar _featureRgbs[MAX_FEATURE_COLORS][3];

  /**
   * @brief List of data colors.
   */

  std::vector< Gdk::Color > _dataColorTable;

  /**
   * @brief The RGB values for the color palette.  They are stored by scaled
   *        data value.  So, the values are accessed as:
   *             red = color_table_rgbx[(scaled_data_value * 3)]
   *             green = color_table_rgbx[(scaled_data_value * 3) + 1]
   *             blue = color_table_rgbx[(scaled_data_value * 3) + 2]
   */

  guchar _colorTableRgbs[MAX_COLOR_TABLE_SIZE][3];


  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Check that the color table name is valid.  If it isn't valid,
   *        change it to a valid name.
   */

  void _checkColorTableName();
  

  /**
   * @brief Fill the given write buffer with the palette information.
   *
   * @param[in] write_buffer     The buffer to fill.
   */

  void _fillWriteBuffer(struct solo_palette &write_buffer) const;
  

  /**
   * @brief "Fix" the color.  For some reason, we need "gray" rather than
   *        "grey".
   *
   * @param[in] old_color    The old color name.
   *
   * @return Returns the "fixed" color name.
   */

  static std::string _fixColor(const std::string old_color)
  {
    if (old_color.find("grey") == std::string::npos)
      return old_color;

    return "gray";
  }

  /**
   * @brief Initialize the internal color tables.
   */

  void _initColorTables();
  
  /**
   * @brief Tokenize the given string.
   *
   * @param[in] token_line    The line to tokenize.
   * @param[out] token_ptrs   The pointers to the tokens.
   * @param[in] delimiters    The delimiters.
   *
   * @return Returns the number of tokens found.
   */

  static int _tokenize(char *token_line, char *token_ptrs[],
		       const char *delimiters)
  {
    int num_tokens = 0;
    char *b = token_line;

    for (b = strtok(b, delimiters); b != 0; b = strtok(NULL, delimiters))
      token_ptrs[num_tokens++] = b;

    return num_tokens;
  }

};

#endif
