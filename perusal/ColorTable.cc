#include <iostream>

#include "ColorTable.hh"

/*********************************************************************
 * Constructors
 */

ColorTable::ColorTable()
{
  // Initialize the missing value and emphasis ranges.  Having the min greater
  // than the max will cause these ranges to have no effect.

  _missingValueRange.min = 0.0;
  _missingValueRange.max = -1.0;
  _missingValueRange.color_index = 0;

  _emphasisRange.min = 0.0;
  _emphasisRange.max = -1.0;
  _emphasisRange.color_index = 0;
}


/*********************************************************************
 * Destructor
 */

ColorTable::~ColorTable()
{
}


/*********************************************************************
 * setColors()
 */

void ColorTable::setColors(const double data_min, const double data_max,
			   const int num_colors_in_table)
{
  // Clear out the old color ranges

  _colorRanges.clear();
  
  // Add the new ranges to the table

  double range_width = (data_max - data_min) / (double)num_colors_in_table;
  
  for (std::size_t i = 0; i < num_colors_in_table; ++i)
  {
    color_range_t color_range;
    
    color_range.min = data_min + ((double)i * range_width);
    color_range.max = data_min + ((double)(i+1) * range_width);
    color_range.color_index = i;
    
    _colorRanges.push_back(color_range);
  }
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
