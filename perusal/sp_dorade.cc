/* 	$Id$	 */

#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>

#ifdef USE_RADX
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#endif

#include <solo_window_structs.h>
#include <solo_defs.h>
#include <sed_shared_structs.h>
#include <math.h>
#include "sp_dorade.hh"
#include "soloii.h"
#include "soloii.hh"
#include "sp_basics.hh"
#include "solo2.hh"
#include <DataManager.hh>
#include <DataInfo.hh>


/* c------------------------------------------------------------------------ */
/* This routine creates a lookup table of source data cell numbers           */
/* corresponding to a cell number based on uniform cell spacing.  This is    */
/* necessary because we are likely to have nonuniform cell spacing and the   */
/* rasterization algorithm needs uniform cell spacing.                       */

void solo_cell_lut(int frame_num)
{
  // Get the needed frame information

  WW_PTR wwptr = solo_return_wwptr(frame_num);
  short *lut = wwptr->data_cell_lut;
  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(wwptr->lead_sweep->window_num);
  
  // Get the corrected cell vector.  I'm guessing that this structure contains
  // information about a single beam in the scan, and the beams are assumed to
  // be consistent throughout the scan.

  struct cell_d *celv = data_info->getCellVectorC();

  // NOTE: It seems like we really want num_cells to equal celv->number_cells
  // without subtracting one.  Then we should subtrack 1 when using it as an
  // index.  Below it looks to me like we are assuming we don't subtract the
  // 1.

  int num_cells = celv->number_cells - 1;
  float max_range = celv->dist_cells[num_cells];

  // Get the uniform gate spacing to use.  This will be the first positive
  // distance between consecutive cells.

  float gate_spacing;
  
  if ((gate_spacing = wwptr->uniform_cell_spacing) == 0)
  {
    // Find an early gate spacing > 0.
    // NOTE: Should MAXCVGATES be num_cells instead?  It seems like we could
    // go past the end of the useful data here.

    for (int ii = 0; ii < MAXCVGATES - 1; ii++)
    {
      gate_spacing = celv->dist_cells[ii+1] - celv->dist_cells[ii];
      if (gate_spacing > 0)
	break;
    }
    wwptr->uniform_cell_spacing = gate_spacing;
  }

  // Get out in front of the radar

  // NOTE: If we truly do have uneven gate spacing, can we be guaranteed that
  // we'll have a cell that is within half the gate spacing of the radar?
  // Shouldn't we instead increment get until dist_cells for that gate is
  // positive?  Of course, in this comment I'm assuming that dist_cells is
  // negative on one side of the radar and positive on the other.  I don't
  // really know what it means for the radar location not to be at the
  // beginning of the beam.

  int gate;
  
  for (gate = 0;
       gate < num_cells && celv->dist_cells[gate] < 0.5 * gate_spacing;
       gate++);

  // Now create the lookup table.  The lookup table maps the index of the
  // actual data cell to the uniform gate spacing gate index.

  int ii;
  float uniform_gate_dist = celv->dist_cells[gate];
  
  // NOTE: It seems like we are also not handling things quite correctly here.
  // I think this really only works if the radar is at the beginnig of the beam
  // (so dist_cells[0] == 0.0) and the cell spacing is already fairly uniform.
  // Don't know if we really need to fix this.

  for (ii = 0; ; uniform_gate_dist += gate_spacing)
  {
    if (fabs((double)(uniform_gate_dist - celv->dist_cells[gate])) >
	0.5 * fabs((double)(celv->dist_cells[gate+1] - celv->dist_cells[gate])))
    {
      // The distance between the target range and the current cell is
      // greater that half the distance between the current cell and the
      // next cell so start using the next cell.

      gate++;
      if (gate >= num_cells)
	break;
    }
    *(lut+ii++) = gate;
  }

  for (; uniform_gate_dist < max_range + 0.5 * gate_spacing;
       uniform_gate_dist += gate_spacing)
  {
    *(lut+ii++) = gate;
  }
  wwptr->number_cells = ii;
  wwptr->uniform_cell_one = celv->dist_cells[*lut];
}

/* c------------------------------------------------------------------------ */
/* Associate a color number with each data cell value for the particular     */
/* parameter for this ray.                                                   */

#ifdef USE_RADX

void solo_color_cells(int frame_num, const RadxRay *ray)
{
  // Get the needed frame information

  WW_PTR wwptr = solo_return_wwptr(frame_num);
  const RadxField *field = ray->getField(wwptr->parameter_num);
  
  // Get a pointer to the cell colors.  This is where the colors will ultimately
  // reside.

  u_int32_t *cell_colors = wwptr->cell_colors;

  // Get a pointer to the data color and data cell look-up tables.  These tables
  // combine to give us a mapping from the data value to the color.
  //
  // The locations in the data cell look-up table map directly to the raster
  // image.  The value in this table gives us the index into the actual data
  // that gives us the data value for that point.  The data value is then used
  // as an index into the color table to get the actual color for the point.

//  u_int32_t *data_color_lut = wwptr->data_color_lut;
  short *data_cell_lut = wwptr->data_cell_lut; 

  // Pointer to start of data

  std::size_t num_gates = field->getNPoints();
  const Radx::fl32 *data = field->getDataFl32();
  
  for (register std::size_t ii = 0; ii < num_gates; ii++)
  {
    // Get the cell number for this data cell
    
    register int cell_num = data_cell_lut[ii];

    *cell_colors++ = wwptr->color_table.getColorIndex(data[cell_num]);

  }

    // See if we need to threshold the data

    // NOTE: RadxField has a threshold field in it.  Can we use this for our
    // thresholding?

//    int thr_ndx;
//    
//    if (batch_threshold_field != 0 &&
//	(thr_ndx = data_info->getParamIndex(batch_threshold_field->str)) >= 0)
//    {
//      double data_scale =  data_info->getParamScale(thr_ndx);
//      int scaled_thr_val = (int)(data_scale * batch_threshold_value);
//      int bad = (int)data_info->getParamBadDataValue(wwptr->parameter_num);
//      short *thresh_data = (short *)data_info->getParamData(thr_ndx);
//
//      for (register int ii = 0; ii < num_gates; ii++)
//      {
//	// Get the cell number for this data cell
//
//	register int cell_num = data_cell_lut[ii];
//
//	// Get the data value for this data cell
//
//	Radx::si16 sval = si16_data[cell_num];
//
//	// Threshold the value
//
//	if (thresh_data[cell_num] < scaled_thr_val)
//	{ 
//	  sval = bad;
//	}
//	*cell_colors++ = data_color_lut[sval];
//      }
}

#else

void solo_color_cells(int frame_num)
{
  // Get the needed frame information

  WW_PTR wwptr = solo_return_wwptr(frame_num);
  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(wwptr->lead_sweep->window_num);
  short binary_format = data_info->getParamBinaryFormat(wwptr->parameter_num);
  
  // Get a pointer to the cell colors.  This is where the colors will ultimately
  // reside.

  u_int32_t *cell_colors = wwptr->cell_colors;

  // Get a pointer to the data color and data cell look-up tables.  These tables
  // combine to give us a mapping from the data value to the color.
  //
  // The locations in the data cell look-up table map directly to the raster
  // image.  The value in this table gives us the index into the actual data
  // that gives us the data value for that point.  The data value is then used
  // as an index into the color table to get the actual color for the point.

  u_int32_t *data_color_lut = wwptr->data_color_lut;
  short *data_cell_lut = wwptr->data_cell_lut; 

  // Pointer to start of data

  unsigned char *param_data =
    (unsigned char *)data_info->getParamData(wwptr->parameter_num);
  int num_gates = wwptr->number_cells;

  switch (binary_format)
  {
  case DD_8_BITS:
  {
    for (register int ii = 0; ii < num_gates; ii++)
    {
      // Get the cell number for this data cell

      register int cell_num = data_cell_lut[ii];
      *cell_colors++ = data_color_lut[param_data[cell_num]];
    }
    break;
  }
  
  case DD_16_BITS:
  {
    short *param_data_ptr = (short *)param_data;

    // See if we need to threshold the data
    // NOTE: batch_threshold_field is a global GString pointer

    int thr_ndx;
    
    if (batch_threshold_field != 0 &&
	(thr_ndx = data_info->getParamIndex(batch_threshold_field->str)) >= 0)
    {
      double data_scale =  data_info->getParamScale(thr_ndx);
      int scaled_thr_val = (int)(data_scale * batch_threshold_value);
      int bad = (int)data_info->getParamBadDataValue(wwptr->parameter_num);
      short *thresh_data = (short *)data_info->getParamData(thr_ndx);

      for (register int ii = 0; ii < num_gates; ii++)
      {
	// Get the cell number for this data cell

	register int cell_num = data_cell_lut[ii];

	// Get the data value for this data cell

	short sval = param_data_ptr[cell_num];

	// Threshold the value

	if (thresh_data[cell_num] < scaled_thr_val)
	{ 
	  sval = bad;
	}
	*cell_colors++ = data_color_lut[sval];
      }
    }
    else
    {
      for (register int ii = 0; ii < num_gates; ii++)
      {
	// Get the cell number for this data cell

	register int cell_num = data_cell_lut[ii];
	*cell_colors++ = data_color_lut[param_data_ptr[cell_num]];
      }
    }
    break;
  }
  
  default:
  {
    // Desperation message, which unfortunately prints for every ray.
    // However, this is better than just plotting a constant field with
    // no indication as to why it happens...

    char msg[256];
    sprintf(msg, 
	    "DORADE binary format %d unsupported. %s will display as a constant field!\n",
	    binary_format,
	    data_info->getParamName(wwptr->parameter_num).c_str());
    solo_message(msg);
  }
  
  } /* endswitch - binary_format */
}

#endif
