#ifndef ColorTable_HH
#define ColorTable_HH

#include <iostream>
#include <vector>

class ColorTable
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   */

  ColorTable();

  /**
   * @brief Destructor.
   */

  virtual ~ColorTable();


  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Set the colors in the table.
   *
   * @param[in] data_min              Minimum data value.
   * @param[in] data_max              Maximum data value.
   * @param[in] num_colors_in_table   The number of colors in the table.
   */

  void setColors(const double data_min, const double data_max,
		 const int num_colors_in_table);
  
  
  /**
   * @brief Set the exceeded color index.
   *
   * @param[in] color_index    The exceeded color index.
   */

  inline void setExceededColorIndex(const std::size_t color_index)
  {
    _exceededColorIndex = color_index;
  }
  

  /**
   * @brief Set the emphasis range for the color table.
   *
   * @param[in] data_min     Minimum data value for emphasis.
   * @param[in] data_max     Maximum data value for emphasis.
   * @param[in] color_index  Emphasis color index.
   */

  inline void setEmphasisRange(const double data_min, const double data_max,
			       const std::size_t color_index)
  {
    _emphasisRange.min = data_min;
    _emphasisRange.max = data_max;
    _emphasisRange.color_index = color_index;
  }
  
  /**
   * @brief Set the missing value range for the color table.
   *
   * @param[in] data_min     Minimum data value for missing data.
   * @param[in] data_max     Maximum data value for missing data.
   * @param[in] color_index  Emphasis color index.
   */

  inline void setMissingValueRange(const double data_min, const double data_max,
				   const std::size_t color_index)
  {
    _missingValueRange.min = data_min;
    _missingValueRange.max = data_max;
    _missingValueRange.color_index = color_index;
  }
  
  /**
   * @brief Get the color index associated with the given data value.
   *
   * @param[in] data_value     The data value.
   *
   * @return Returns the color index for this data value.
   */

  inline std::size_t getColorIndex(const double data_value) const
  {
    // First, see if this is in the missing value range

    if (data_value >= _missingValueRange.min &&
	data_value <= _missingValueRange.max)
      return _missingValueRange.color_index;

    // Next, see if this is in the emphasis range

    if (data_value >= _emphasisRange.min &&
	data_value <= _emphasisRange.max)
      return _emphasisRange.color_index;

    // Now, loop through the color table and return the index if the 
    // color is found.

    for (std::size_t i = 0; i < _colorRanges.size(); ++i)
    {
      if (data_value >= _colorRanges[i].min &&
	  data_value <= _colorRanges[i].max)
	return _colorRanges[i].color_index;
    }
    
    // If we get here, the data value isn't in the table

    return _exceededColorIndex;
  }
  
protected:

  /////////////////////
  // Protected types //
  /////////////////////

  typedef struct
  {
    double min;
    double max;
    std::size_t color_index;
  } color_range_t;
  
     
  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief The actual color table.  The ranges are guaranteed to be sorted
   *        with the lowest data value range at the beginning of the list
   *        and the highest at the end.
   */

  std::vector< color_range_t > _colorRanges;
  
  /**
   * @brief The exceeded color index.  This is the index that is used if the
   *        data value isn't found in the table.
   */

  std::size_t _exceededColorIndex;
  
  /**
   * @brief Emphasis range.  This range overrides and colors in the color
   *        table.
   */

  color_range_t _emphasisRange;
  
  /**
   * @brief The missing data value range.  This overrides the emphasis range.
   */

  color_range_t _missingValueRange;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

};

#endif
