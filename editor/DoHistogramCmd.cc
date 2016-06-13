#ifdef USE_RADX
#else
#include <se_histog.hh>
#endif

#include <HistogramMgr.hh>
#include <se_histog.hh>
#include <se_utils.hh>

#include "DoHistogramCmd.hh"


/*********************************************************************
 * Constructors
 */

DoHistogramCmd::DoHistogramCmd() :
  ForEachRayCmd("do-histogram", "do-histogram")
{
}


/*********************************************************************
 * Destructor
 */

DoHistogramCmd::~DoHistogramCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool DoHistogramCmd::doIt(const int frame_num, RadxRay &ray) const
{
#define PIOVR360 .00872664626

  // Boundary mask is set to 1 if the corresponding cell satisfies
  // conditions of the boundaries

  struct solo_edit_stuff *seds = return_sed_stuff();
  std::string param_name = seds->histogram_field;

  static bool first_time = false;
  
  if (seds->process_ray_count == 1)
  {
    first_time = true;
  }

  if (seds->use_boundary && seds->boundary_exists && !seds->num_segments)
  {
    // Ray does not intersect the boundary

    return true;
  }

  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(frame_num);
  
  RadxField *field = ray.getField(param_name);
  
  if (field == 0)
  {
    // Field not found

    printf("Source parameter %s not found for copy\n", param_name.c_str());
    seds->punt = 1;
    return false;
  }

  std::size_t num_gates = ray.getNGates();
  const Radx::fl32 *data = field->getDataFl32();
  double bad = field->getMissingFl32();
  
  if (first_time)
  {
    first_time = false;

    // NOTE: Need to update these for Radx

    seds->histo_start_time = data_info->getTime();
    seds->histo_fixed_angle = data_info->getFixedAngle();
    seds->histo_radar_name = data_info->getRadarName();

    // Set up for this run
    // Create a file name from the first ray

    if (seds->histo_directory == "")
      seds->histo_directory = seds->sfic->directory_text;

    std::string file_name =
      DataManager::getInstance()->getFileBaseName("hgm",
						  static_cast<int32_t>(seds->histo_start_time),
						  seds->histo_radar_name,
						  0);
    seds->histo_filename = file_name;
    // NOTE: This is the only place where se_histo_init() is called
    se_histo_init(seds);
    HistogramMgr::getInstance()->initAreaXValue();
  }

  double rcp_inc = 1.0 / seds->histo_increment;
  seds->histo_stop_time = data_info->getTime();

  // Loop through the data

  short *boundary_mask = (short *) seds->boundary_mask;
  
  for (std::size_t i = 0; i < num_gates; ++i)
  {
    if (boundary_mask[i] == 0)
      continue;
    
    double value = data[i];
    
    if (value == bad)
    {
      seds->histo_missing_count++;
      continue;
    }

    if (seds->histo_key & H_BINS)
    {
      if (value < seds->histo_low)
      {
	seds->low_count++;
      }
      else if (value >= seds->histo_high)
      {
	seds->high_count++;
      }
      else
      {
	seds->medium_count++;
	int kk = (int)((value - seds->histo_low) / seds->histo_increment);
	seds->counts_array[kk]++;
	seds->histo_sum += value;
	seds->histo_sum_sq += value * value;
      }
    }
    else
    {
      // Areas!

      double r1;
      double r2;
      
      if (i > 0)
      {
	r1 = ray.getGateRangeKm(i-1) * 1000.0;
	r2 = ray.getGateRangeKm(i) * 1000.0;
      }
      else
      {
	r1 = 0;
	r2 = ray.getStartRangeKm() * 1000.0;
      }

      // NOTE: Need to calculate sector for this beam.  See
      // sii_perusal::_calcSector()

//      struct dd_ray_sector *ddrc =
//	data_info->rayCoverage(seds->sweep_ray_count);
//      float area = fabs(ddrc->sector) * PIOVR360 * (SQ(r2) - SQ(r1));
      float area = 1.0;
		
      if (value < seds->histo_low)
      {
	seds->low_count++;
	seds->low_area += area;
      }
      else if (value >= seds->histo_high)
      {
	seds->high_count++;
	seds->high_area += area;
      }
      else
      {
	seds->medium_count++;
	HistogramMgr::getInstance()->addAreaXValue(area * value);
	int kk = (int)((value - seds->histo_low) / seds->histo_increment);
	seds->areas_array[kk] += area;

	seds->histo_sum += value;
	seds->histo_sum_sq += value * value;
      }
    }
  }

  return true;
}

void DoHistogramCmd::finishUp() const
{
  struct solo_edit_stuff *seds = return_sed_stuff();

  // NOTE: Move se_histo_fin_counts() and se_histo_fin_areas() here.  This
  // is the only place they are called.  Or maybe create a Histogram object
  // and put them there.

  if (H_BINS & seds->histo_key) 
    se_histo_fin_counts();
  else
    se_histo_fin_areas();
  HistogramMgr::getInstance()->histoOutput();
}

#else

bool DoHistogramCmd::doIt() const
{
  if (se_histo_ray(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
