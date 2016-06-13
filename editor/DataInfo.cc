#include <algorithm>
#include <dirent.h>
#include <sys/types.h>

#include <DataManager.hh>
#include <ddb_common.hh>
#include <dd_math.h>
#include <EarthRadiusCalculator.hh>
#include <sii_externals.h>
#include <solo2.hh>

#include <dd_catalog.hh>
#include <dd_der_flds.hh>
#include <dd_math.h>
#include <dd_swp_files.hh>
#include <dda_common.hh>
#include <ddb_common.hh>
#include <dorade_ncdf.hh>
#include <dorade_tape.hh>
#include <nssl_mrd.hh>
#include <swp_file_acc.hh>

#include "DataInfo.hh"


/*********************************************************************
 * Constructors
 */

DataInfo::DataInfo(const size_t index, const std::string &name) :
#ifdef USE_RADX
  _seds(0),
  _sizeofSeds(0),
#endif
  _diskOutput(false)
{
#ifdef USE_RADX
#else
  _info = dd_ini(index, name.c_str());
#endif
}


/*********************************************************************
 * Destructor
 */

DataInfo::~DataInfo()
{
#ifdef USE_RADX
  if (_seds != 0)
    free(_seds);
#endif
}


/*********************************************************************
 * addCommands()
 */

#ifdef USE_RADX

void DataInfo::addCommands(const std::vector< std::string > &cmd_list)
{
  // Add up the number of chars to be appended.  Remember to add the
  // final null at the end of the buffer.

  int size = 0;
  for (std::size_t i = 0; i < cmd_list.size(); ++i)
    size += cmd_list[i].size();

  size += 1;
  
  // Force size to an integer number of longs

  int long_size = LONGS_TO_BYTES(BYTES_TO_LONGS(size));

  // Don't save more than 64K of edit information.

  if (_sizeofSeds + long_size >= K64)
    return;

  int padding = long_size - size;
  char *buffer_ptr;
  struct generic_descriptor *gd = 0;
  
  if (_sizeofSeds <= 0)
  {
    long_size += sizeof(struct generic_descriptor);
    _seds = (char *)malloc(long_size);
    buffer_ptr = _seds + sizeof(struct generic_descriptor);
    gd = (struct generic_descriptor *)_seds;
    strncpy(gd->name_struct, "SEDS", 4);
  }
  else
  {
    long_size += _sizeofSeds;
    _seds = (char *)realloc(_seds, long_size);
    buffer_ptr = _seds + _sizeofSeds;
    gd = (struct generic_descriptor *)_seds;
  }

  _sizeofSeds = long_size;
  gd->sizeof_struct = long_size;

  for (std::size_t i = 0; i < cmd_list.size(); ++i)
  {
    // Copy this line

    sprintf(buffer_ptr, "%s", cmd_list[i].c_str());
    buffer_ptr += cmd_list[i].size();
  }

  // Pad it out with blanks

  if (padding > 0)
  {
    for (int i = 0; i < padding-1; ++i)
      *buffer_ptr++ = ' ';
    *buffer_ptr++ = '\n';
  }
}

#else

void DataInfo::addCommands(const std::vector< std::string > &cmd_list)
{
  // Add up the number of chars to be appended.  Remember to add the
  // final null at the end of the buffer.

  int size = 0;
  for (std::size_t i = 0; i < cmd_list.size(); ++i)
    size += cmd_list[i].size();

  size += 1;
  
  // Force size to an integer number of longs

  int long_size = LONGS_TO_BYTES(BYTES_TO_LONGS(size));

  // Don't save more than 64K of edit information.

  if (_info->sizeof_seds + long_size >= K64)
    return;

  int padding = long_size - size;
  char *buffer_ptr;
  struct generic_descriptor *gd = 0;
  
  if (_info->sizeof_seds <= 0)
  {
    long_size += sizeof(struct generic_descriptor);
    _info->seds = (char *)malloc(long_size);
    buffer_ptr = _info->seds + sizeof(struct generic_descriptor);
    gd = (struct generic_descriptor *)_info->seds;
    strncpy(gd->name_struct, "SEDS", 4);
  }
  else
  {
    long_size += _info->sizeof_seds;
    _info->seds = (char *)realloc(_info->seds, long_size);
    buffer_ptr = _info->seds + _info->sizeof_seds;
    gd = (struct generic_descriptor *)_info->seds;
  }

  _info->sizeof_seds = long_size;
  gd->sizeof_struct = long_size;

  for (std::size_t i = 0; i < cmd_list.size(); ++i)
  {
    // Copy this line

    sprintf(buffer_ptr, "%s", cmd_list[i].c_str());
    buffer_ptr += cmd_list[i].size();
  }

  // Pad it out with blanks

  if (padding > 0)
  {
    for (int i = 0; i < padding-1; ++i)
      *buffer_ptr++ = ' ';
    *buffer_ptr++ = '\n';
  }
}

#endif


/*********************************************************************
 * alternateGecho()
 */

#ifdef USE_RADX

int DataInfo::alternateGecho(const double min_grad, int &zmax_cell)
{
//  DDS_PTR dds = _info->dds;
//  int pn, ii, jj, nn, zmax = 0, ival;
//  int mark_max = -1, mark_grad = -1, ng_grad;
//  short *ss, scaled30;
//  double gs, smin_grad, grad, elev = dds->ra->elevation;
//  double rot_angle;
//
//  if ((pn = dd_find_field(_info, "DBZ")) < 0)
//  {
//    printf("Source parameter %s not found for surface removal\n",
//	   "DBZ");
//    return -1;
//  }
//
//  // Assume this routine is not called unless the antenna is in a position
//  // to see the ground
//
//  gs = dds->celvc->dist_cells[1] - dds->celvc->dist_cells[0];
//  ss = (short *)dds->qdat_ptrs[pn];
//  nn = _info->dds->celvc->number_cells;
//  scaled30 = (int)(_info->dds->parm[pn]->parameter_scale * 30.0);
//  smin_grad = min_grad * _info->dds->parm[pn]->parameter_scale * gs * 2.0;
//  rot_angle =
//    dds->asib->rotation_angle + dds->asib->roll + dds->cfac->rot_angle_corr;
//
//  // Scaled change in dBz over two gates
//  // min_grad = .08 => ~20 dBz over two gates.
//       
//  ng_grad = (int)(1.075 * (1.0/fabs(sin(elev)) + 1.0));
//
//  if (ng_grad < 2)
//    ng_grad = 2;
//  else if (ng_grad > 77)
//    ng_grad = 77;
//
//  for (ii = ng_grad + 1; ii < nn; ii++)
//  {
//    ival = ss[ii];
//    if (ival < scaled30)
//      continue;
//       
//    if (mark_grad < 0)
//    {
//      for (grad = 0, jj = 0; jj < ng_grad; jj++)
//      {
//	grad += ss[ii-jj] - ss[ii-jj-1];
//	if (grad > smin_grad)
//	{
//	  mark_grad = ii-jj-1;
//	  break;
//	}
//      }
//    }
//    if (ival > scaled30 && ival > zmax)
//    {
//      mark_max = ii;
//      zmax = ival;
//    }
//  }
//
//  zmax_cell = mark_max;
//
//  return mark_grad;

  return 0;
}

#else

int DataInfo::alternateGecho(const double min_grad, int &zmax_cell)
{
  DDS_PTR dds = _info->dds;
  int pn, ii, jj, nn, zmax = 0, ival;
  int mark_max = -1, mark_grad = -1, ng_grad;
  short *ss, scaled30;
  double gs, smin_grad, grad, elev = dds->ra->elevation;
  double rot_angle;

  if ((pn = dd_find_field(_info, "DBZ")) < 0)
  {
    printf("Source parameter %s not found for surface removal\n",
	   "DBZ");
    return -1;
  }

  // Assume this routine is not called unless the antenna is in a position
  // to see the ground

  gs = dds->celvc->dist_cells[1] - dds->celvc->dist_cells[0];
  ss = (short *)dds->qdat_ptrs[pn];
  nn = _info->dds->celvc->number_cells;
  scaled30 = (int)(_info->dds->parm[pn]->parameter_scale * 30.0);
  smin_grad = min_grad * _info->dds->parm[pn]->parameter_scale * gs * 2.0;
  rot_angle =
    dds->asib->rotation_angle + dds->asib->roll + dds->cfac->rot_angle_corr;

  // Scaled change in dBz over two gates
  // min_grad = .08 => ~20 dBz over two gates.
       
  ng_grad = (int)(1.075 * (1.0/fabs(sin(elev)) + 1.0));

  if (ng_grad < 2)
    ng_grad = 2;
  else if (ng_grad > 77)
    ng_grad = 77;

  for (ii = ng_grad + 1; ii < nn; ii++)
  {
    ival = ss[ii];
    if (ival < scaled30)
      continue;
       
    if (mark_grad < 0)
    {
      for (grad = 0, jj = 0; jj < ng_grad; jj++)
      {
	grad += ss[ii-jj] - ss[ii-jj-1];
	if (grad > smin_grad)
	{
	  mark_grad = ii-jj-1;
	  break;
	}
      }
    }
    if (ival > scaled30 && ival > zmax)
    {
      mark_max = ii;
      zmax = ival;
    }
  }

  zmax_cell = mark_max;

  return mark_grad;
}

#endif


/*********************************************************************
 * catalogSweepFile()
 */

#ifdef USE_RADX
#else

void DataInfo::catalogSweepFile()
{
  dd_catalog(_info, 0, YES);
  produce_nssl_mrd(_info, YES);
  produce_ncdf(_info, YES);
}

#endif


/*********************************************************************
 * establishField()
 */

#ifdef USE_RADX
#else

int DataInfo::establishField(const std::string &src_name,
			     const std::string &dst_name,
			     int &src_index, int &dst_index)
{
  // Established a possible new field but does not copy the data from the
  // source field into the new field

  struct dds_structs *dds = _info->dds;

  // Find the source field

  // NOTE: Can we just use src_index throughout and get rid of fnsx?  That
  // would change the functionality slightly if the field isn't found so not
  // doing that yet.

  int fnsx;
  
  if ((fnsx = dd_find_field(_info, src_name.c_str())) < 0)
  {	
    g_string_sprintfa(gs_complaints,
		      "Source parameter %s not found for establis field\n",
		      src_name.c_str());
    return -1;
  }



  // NOTE: Again, can we replace fndx with dst_index???

  int first_empty = -1;
  int fndx = -1;	

  for (int fn = 0; fn < MAX_PARMS; fn++)
  {
    if (!dds->field_present[fn])
    {
      if(first_empty < 0)
	first_empty = fn;
      continue;
    }

    // Get the length of this parameter's name. In struct parameter_d, 
    // parameter_name is a char[8] array. The string contained there 
    // is NULL-terminated UNLESS it is exactly 8 characters long, in which
    // case the name fills the available space. I.e., if there's a NULL in the 
    // array, use strlen, otherwise use the full array length.
    int arrayLen = sizeof(dds->parm[fn]->parameter_name); // this is 8 right now...
    int paramNameLen = memchr(dds->parm[fn]->parameter_name, '\0', arrayLen) ?
            strlen(dds->parm[fn]->parameter_name) : arrayLen;

    // See if this parameter name matches the given destination name
    if (!dst_name.compare(0, dst_name.size(), 
                          dds->parm[fn]->parameter_name, paramNameLen))
    {
      // Destination field!

      fndx = fn;
      break;
    }
  }

  if (fndx < 0)
  {
    // Create the new parameter

    if (first_empty < 0)
    {
      g_string_sprintfa(gs_complaints,
			"You have exceeded the %d parameter limit!\n",
			(int)MAX_PARMS);
      exit(1);
    }

    fndx = first_empty;
    int fn = first_empty;

    memcpy((char *)dds->parm[fn], (char *)dds->parm[fnsx],
	   sizeof(struct parameter_d));
    strncpy(dds->parm[fn]->parameter_name, dst_name.c_str(), 8);
    
    int ncopy = *dds->qdat[fn]->pdata_desc == 'R' ?
      sizeof(struct paramdata_d) : dds->parm[fn]->offset_to_data;
    memcpy(dds->qdat[fn], dds->qdat[fnsx], ncopy);
    strncpy(dds->qdat[fn]->pdata_name, dst_name.c_str(), 8);
    dd_alloc_data_field(_info, fndx);

    dds->field_id_num[fn] = dd_return_id_num(dds->parm[fn]->parameter_name);
    _info->parm_type[fn] = _info->parm_type[fnsx];
    dds->number_cells[fn] = dds->number_cells[fnsx];
    dds->field_present[fn] = YES;
    dds->last_cell_num[fn] = dds->last_cell_num[fnsx];
  }

  src_index = fnsx;
  dst_index = fndx;

  return fndx;
}

#endif


/*********************************************************************
 * getAircraftVelocity()
 */

#ifdef USE_RADX

double DataInfo::getAircraftVelocity() const
{
  // NOTE: For now, we're assuming that the aircraft velocity isn't changing
  // through the volume so are just getting the information from the first
  // ray in the volume.  Need to look at where this is used to see if we need
  // to get it from each ray separately.

  // Get access to the first ray in the volume

  const std::vector< RadxRay* > rays = _radxVol.getRays();
  const RadxRay *ray = rays[0];
  const RadxGeoref *georef = ray->getGeoreference();
  
  double vert =  georef->getVertVelocity();
  double d = sqrt(SQ(georef->getEwVelocity()) +
		  SQ(georef->getNsVelocity()));
//  d += dds->cfac->ew_gndspd_corr; /* klooge to correct ground speed */
  return (d * sin(georef->getTilt())) +
    (vert * sin(RADIANS(ray->getElevationDeg())));
}

#else

double DataInfo::getAircraftVelocity() const
{
  return dd_ac_vel(_info);
}

#endif


/*********************************************************************
 * getAngleIndex()
 */

#ifdef USE_RADX

int DataInfo::getAngleIndex(const double theta_deg) const
{
  const RadxSweep *sweep = getRadxSweep();
  
  if (sweep == 0)
    return -1;
  
  const std::vector< RadxRay* > rays = _radxVol.getRays();
  
  std::size_t min_ray_index = sweep->getStartRayIndex();
  double min_angle_diff =
    _angleDifference(rays[min_ray_index]->getAzimuthDeg(), theta_deg);
  
  for (std::size_t ray_index = sweep->getStartRayIndex() + 1;
       ray_index <= sweep->getEndRayIndex(); ++ray_index)
  {
    double new_angle_diff =
      _angleDifference(rays[ray_index]->getAzimuthDeg(), theta_deg);
    
    if (min_angle_diff > new_angle_diff)
    {
      min_angle_diff = new_angle_diff;
      min_ray_index = ray_index;
    }
  }
  
  return min_ray_index;
}

#else

int DataInfo::getAngleIndex(const double theta) const
{
  return dd_rotang_seek(_info->source_rat, theta);
}

#endif

/*********************************************************************
 * getAzimuthAngleCalc()
 */

#ifdef USE_RADX
#else

double DataInfo::getAzimuthAngleCalc() const
{
  return dd_azimuth_angle(_info);
}

#endif


/*********************************************************************
 * getClosestGate()
 */

#ifdef USE_RADX
#else

void DataInfo::getClosestGate(const double range,
				       int &gate_num, float &gate_range) const
{
  dd_range_gate(_info, range, &gate_num, &gate_range);
}

#endif


/*********************************************************************
 * getElevationAngleCalc()
 */

#ifdef USE_RADX
#else

double DataInfo::getElevationAngleCalc() const
{
  return dd_elevation_angle(_info);
}

#endif


/*********************************************************************
 * getGateNum()
 */

#ifdef USE_RADX
#else

std::size_t DataInfo::getGateNum(const double range) const
{
  return dd_cell_num(_info->dds, range);
}

#endif


/*********************************************************************
 * getParamDataUnscaled()
 */

#ifdef USE_RADX
#else

int DataInfo::getParamDataUnscaled(const std::size_t param_num,
				   const std::size_t start_gate,
				   const std::size_t num_gates,
				   float *buffer,
				   float &bad_data_value)
{
  return dd_givfld(_info, param_num, start_gate, num_gates,
		   buffer, &bad_data_value);
}

#endif


/*********************************************************************
 * getParamIndex()
 */

#ifdef USE_RADX

int DataInfo::getParamIndex(const std::string &param_name) const
{
  if (param_name == "")
    return -1;
    
  const vector< RadxField* > fields = _radxVol.getFields();

  for (size_t i = 0; i < fields.size(); ++i)
  {
    if (fields[i]->getName() == param_name)
      return (int)i;
  }
  
  // If we get here, the parameter wasn't found.

  return -1;
}

#else

int DataInfo::getParamIndex(const std::string &param_name) const
{
  if (param_name == "")
    return -1;
    
  return dd_find_field(_info, param_name.c_str());
}

#endif
  

/*********************************************************************
 * getPlatformAltitudeMSL()
 */

#ifdef USE_RADX
#else

double DataInfo::getPlatformAltitudeMSL() const
{
  return dd_altitude(_info);
}

#endif


/*********************************************************************
 * getPlatformDrift()
 */

#ifdef USE_RADX
#else

double DataInfo::getPlatformDrift() const
{
  return dd_drift(_info);
}

#endif


/*********************************************************************
 * getPlatformHeading()
 */

#ifdef USE_RADX
#else

double DataInfo::getPlatformHeading() const
{
  return dd_heading(_info);
}

#endif


/*********************************************************************
 * getPlatformLatitude()
 */

#ifdef USE_RADX
#else

double DataInfo::getPlatformLatitude() const
{
  return dd_latitude(_info);
}

#endif


/*********************************************************************
 * getPlatformLongitude()
 */

#ifdef USE_RADX
#else

double DataInfo::getPlatformLongitude() const
{
  return dd_longitude(_info);
}

#endif


/*********************************************************************
 * getRATangle()
 */

#ifdef USE_RADX
#else

double DataInfo::getRATangle(const size_t angle_index) const
{
  struct rot_table_entry *entry1 = dd_return_rotang1(_info->source_rat);
  return (entry1 + angle_index)->rotation_angle;
}

#endif


/*********************************************************************
 * getRadarNameClean()
 */

#ifdef USE_RADX
#else

std::string DataInfo::getRadarNameClean() const
{
  return dd_radar_namec(_info->dds->radd->radar_name);
}

#endif


/*********************************************************************
 * getPlatformPitch()
 */

#ifdef USE_RADX
#else

double DataInfo::getPlatformPitch() const
{
  return dd_pitch(_info);
}

#endif


/*********************************************************************
 * getPlatformRoll()
 */

#ifdef USE_RADX
#else

double DataInfo::getPlatformRoll() const
{
  return dd_roll(_info);
}

#endif


/*********************************************************************
 * getRotationAngleCalc()
 */

#ifdef USE_RADX
#else

double DataInfo::getRotationAngleCalc() const
{
  return dd_rotation_angle(_info);
}

#endif


/*********************************************************************
 * getTiltAngle()
 */

#ifdef USE_RADX
#else

double DataInfo::getTiltAngle() const
{
  return dd_tilt_angle(_info);
}

#endif


#ifdef USE_RADX

/*********************************************************************
 * loadRadxFile()
 */

bool DataInfo::loadRadxFile(const std::string &file_path,
			    const int sweep_index)
{
  RadxFile radx_file;
  
  radx_file.clearRead();
  radx_file.setReadSweepNumLimits(sweep_index, sweep_index);

  if (radx_file.readFromPath(file_path, _radxVol) != 0)
    return false;
  
  _radxPath = file_path;
  
  // Convert the data to floats so to simplify the code for processing
  // this dataset.

  _radxVol.convertToFl32();

  // NOTE: I'm not sure if this call is necessary

  _radxVol.loadFieldsFromRays();
  
  const std::vector< RadxSweep* > sweeps = _radxVol.getSweeps();
  
  return true;
}

/*********************************************************************
 * writeRadxVolume()
 */

bool DataInfo::writeRadxVolume()
{
  static const string method_name = "DataInfo::writeRadxVolume()";
  
//  // If we don't have a current file, we can't write the data
//
//  if (_currentFile == _fileList.end())
//  {
//    std::cerr << "ERROR: " << method_name << std::endl;
//    std::cerr << "No current file pointer, can't write file" << std::endl;
//    
//    return false;
//  }
//  
//  // Construct the file path
//
//  std::string file_path = _directoryName + _currentFile->getFileName();
  
  std::string file_path = _radxPath;
  
  // Set up the write

  RadxFile radx_file;
  
  radx_file.setFileFormat(RadxFile::FILE_FORMAT_DORADE);

  if (radx_file.writeToPath(_radxVol, file_path) != 0)
    return false;
  
  return true;
}

#endif


/*********************************************************************
 * setDir()
 */

#ifdef USE_RADX
#else

void DataInfo::setDir(const std::string &dir_path)
{
  // Set the directory in the window information structure

  _info->directory_name = dir_path;
  if (_info->directory_name[_info->directory_name.size()-1] != '/')
    _info->directory_name += "/";

  _directoryName = dir_path;
  if (_directoryName[_directoryName.size()-1] != '/')
    _directoryName += "/";
  
  // Compile the file list from this directory

  _fileList.clear();
  
  DIR *dir_ptr = opendir(dir_path.c_str());
  if (dir_ptr == 0)
  {
    _currentFile = _fileList.begin();
    return;
  }
  
  struct dirent *entry;
  
  while ((entry = readdir(dir_ptr)) != 0)
  {
    if (strncmp(entry->d_name, "swp.", 4) == 0)
      _fileList.push_back(FileInfo(entry->d_name));
  }
  
  sort(_fileList.begin(), _fileList.end());
  
  _currentFile = _fileList.begin();
}

#endif


/*********************************************************************
 * setGateSpacing()
 */

#ifdef USE_RADX

void DataInfo::setGateSpacing(const double gate_spacing_m)
{
  _radxVol.setGateSpacingKm(gate_spacing_m / 1000.0);
}

#else

void DataInfo::setGateSpacing(const double gate_spacing_m)
{
  float rng = getStartRange();
  
  float *fptr = _info->dds->celv->dist_cells;
  float *cfptr = _info->dds->celvc->dist_cells;
  int num_cells = getNumCells();
    
  for (int i = 0; i < num_cells; ++i, ++fptr, ++cfptr, rng += gate_spacing_m)
  {
    *fptr = rng;
    *cfptr = *fptr;
  }

  dd_set_uniform_cells(_info->dds);
}

#endif

/*********************************************************************
 * setScanMode()
 */

#ifdef USE_RADX
#else

void DataInfo::setScanMode(const std::string &scan_mode)
{
  _info->dds->radd->scan_mode = dd_get_scan_mode(scan_mode.c_str());
}

#endif


/*********************************************************************
 * setStartRange()
 */

#ifdef USE_RADX

void DataInfo::setStartRange(const double start_range_m)
{
  _radxVol.setStartRangeKm(start_range_m / 1000.0);
}

#else

void DataInfo::setStartRange(const double start_range_m)
{
  float diff = start_range_m - _info->dds->celv->dist_cells[0];
    
  float *fptr = _info->dds->celv->dist_cells;
  float *cfptr = _info->dds->celvc->dist_cells;
    
  for (int i = 0; i < getNumCells(); ++i, ++fptr, ++cfptr)
  {
    *fptr += diff;
    *cfptr = *fptr;
  }
  dd_set_uniform_cells(_info->dds);
}

#endif

/*********************************************************************
 * scaleValue()
 */

#ifdef USE_RADX
#else

int DataInfo::scaleValue(const double &value,
			 const std::size_t field_index) const
{
  return (int)DD_SCALE(value,
		       _info->dds->parm[field_index]->parameter_scale,
		       _info->dds->parm[field_index]->parameter_bias);
}

#endif


/*********************************************************************
 * rewindBuffer()
 */

#ifdef USE_RADX
#else

void DataInfo::rewindBuffer()
{
  dgi_buf_rewind(_info);
}

#endif


/*********************************************************************
 * setTimeSeriesFileList()
 */

#ifdef USE_RADX
#else

bool DataInfo::setTimeSeriesFileList(const DateTime &start_time,
				     const DateTime &stop_time)
{
  // Clear out the current file list

  _timeSeriesFileList.clear();
  
  // Open the current directory

  DIR *dir_ptr = opendir(_info->directory_name.c_str());
  if (dir_ptr == 0)
  {
    _currentFile = _timeSeriesFileList.begin();
    return false;
  }
  
  // Read the directory entries, adding every file to the list.  After all of
  // the files are added, we'll go back through and get rid of the files not
  // between the given times.  We need to do this because we want to keep
  // the last file that is before start_time.

  struct dirent *entry;
  
  while ((entry = readdir(dir_ptr)) != 0)
  {
    // Only process sweep files

    if (strncmp(entry->d_name, "swp.", 4) != 0)
      continue;
    
    // Check the file time.  If it is older than stop time, don't add it
    // to the list

    FileInfo file_info(entry->d_name);
    
    if (file_info.getFileTime() > stop_time)
      continue;
    
    // If we get here, the file should be added to the list

    _timeSeriesFileList.push_back(file_info);
  }
  
  // Sort the file list so the files will be in chronological order

  sort(_timeSeriesFileList.begin(), _timeSeriesFileList.end());
  
  // Remove all files up to start_time.  This means keeping the file just
  // before start_time if we don't have one starting exactly at start_time.

  std::vector< FileInfo >::iterator info_ptr;
  for (info_ptr = _timeSeriesFileList.begin();
       info_ptr != _timeSeriesFileList.end(); ++info_ptr)
  {
    if (info_ptr->getFileTime() >= start_time)
    {
      if (info_ptr->getFileTime() != start_time)
	--info_ptr;
      break;
    }
  }
  
  if (info_ptr != _timeSeriesFileList.end())
    _timeSeriesFileList.erase(_timeSeriesFileList.begin(), info_ptr);
}

#endif


/*********************************************************************
 * setFirstCfac()
 */

#ifdef USE_RADX
#else

void DataInfo::setFirstCfac(struct cfac_wrap *cfac_ptr)
{
  _info->dds->first_cfac = cfac_ptr;
}

#endif


/*********************************************************************
 * stuffRay()
 */

#ifdef USE_RADX
#else

void DataInfo::stuffRay()
{
  ddswp_stuff_ray(_info);
}

#endif


/*********************************************************************
 * loadHeaders()
 */

#ifdef USE_RADX
#else

bool DataInfo::loadHeaders()
{
  int status = dd_absorb_header_info(_info);
  if (status != 0) {
      printf("Could not read header from sweep file '%s'\n", 
              _currentFile->getFileName().c_str());
      return false;
  }
  
  return true;
}

#endif


/*********************************************************************
 * loadNextRay()
 */

#ifdef USE_RADX
#else

bool DataInfo::loadNextRay()
{
  return dd_absorb_ray_info(_info) > 0;
}

#endif


/*********************************************************************
 * loadRay()
 */

#ifdef USE_RADX
#else

void DataInfo::loadRay(const int ray_num)
{
  struct rot_table_entry *entry1 = dd_return_rotang1(_info->source_rat);
  dgi_buf_rewind(_info);
  lseek(_info->in_swp_fid,
	(long)(entry1 + ray_num)->offset, SEEK_SET);
  dd_absorb_ray_info(_info);
}

#endif


/*********************************************************************
 * startNewVolume()
 */

#ifdef USE_RADX
#else

void DataInfo::startNewVolume()
{
  dd_new_vol(_info);
}

#endif


/*********************************************************************
 * writeRay()
 */

#ifdef USE_RADX
#else

void DataInfo::writeRay()
{
  dd_dump_ray(_info);
}

#endif


/*********************************************************************
 * listDescriptors()
 */

#ifdef USE_RADX
#else

void DataInfo::listDescriptors(std::vector< std::string > &list)
{
  // Put everything into an SLM for calling the translate functions.  These
  // will go away when Radx is used.

  struct solo_list_mgmt *slm= SLM_malloc_list_mgmt(256);
  for (size_t i = 0; i < list.size(); ++i)
    SLM_add_list_entry(slm, list[i].c_str());
  
  // Now call the translate functions

  dor_print_vold(_info->dds->vold, slm);
  dor_print_radd(_info->dds->radd, slm);

  for (int pn = 0; pn < _info->num_parms; pn++)
    dor_print_parm(_info->dds->parm[pn], slm);

  if (_info->dds->frib)
    dor_print_frib(_info->dds->frib, slm);

  dor_print_cfac(_info->dds->cfac, slm);
  dor_print_celv(_info->dds->celvc, slm);
  dor_print_swib(_info->dds->swib, slm);
  dor_print_ryib(_info->dds->ryib, slm);
  dor_print_asib(_info->dds->asib, slm);
  dor_print_sswb(_info->dds->sswb, slm);
  dor_print_rktb((struct rot_ang_table *)_info->source_rat, slm);

  // I believe these translate functions change the list entries, so replace
  // the original list with the new values

  list.clear();
  for (int i = 0; i < slm->num_entries; ++i)
    list.push_back(SLM_list_entry(slm, i));

  // Reclaim memory

  SLM_unmalloc_list_mgmt(slm);
}

#endif


/*********************************************************************
 * printStatusLine()
 */

#ifdef USE_RADX
#else

void DataInfo::printStatusLine(const bool verbose,
			       const char *preamble,
			       const char *postamble, char *line) const
{
  // Get pointers to the internal structure for ease of programming

  struct dds_structs *dds = _info->dds;
  struct sweepinfo_d *swib = _info->dds->swib;
  struct platform_i *asib = _info->dds->asib;
  struct ray_i *ryib = _info->dds->ryib;

  // Get a pointer into the output line to use for constructing the line

  char *aa = line;
  *aa = '\0';

  char radar_name[12];
  char str[2048];
  
  sprintf(aa+strlen(aa), "%s ", preamble);
  sprintf(aa+strlen(aa), "%s ", dts_print(d_unstamp_time(dds->dts)));
  strcpy(radar_name, str_terminate(str, dds->radd->radar_name, 8));
  sprintf(aa+strlen(aa), "%s ", radar_name);
  aa += strlen(aa);

  if (dds->radd->scan_mode == DD_SCAN_MODE_AIR)
  {
    sprintf(aa+strlen(aa), "rt:%6.1f ", FMOD360(getRotationAngleCalc()));
    sprintf(aa+strlen(aa), "t:%5.1f ", getTiltAngle());
    sprintf(aa+strlen(aa), "rl:%5.1f ", asib->roll);
    sprintf(aa+strlen(aa), "h:%6.1f ", asib->heading);
    sprintf(aa+strlen(aa), "al:%6.3f ", asib->altitude_msl);
  }
  else if(dds->radd->radar_type == DD_RADAR_TYPE_AIR_LF)
  {
    sprintf(aa+strlen(aa), "az:%6.1f ", getAzimuthAngleCalc());
    sprintf(aa+strlen(aa), "el:%6.2f ", getElevationAngleCalc());
    sprintf(aa+strlen(aa), "h:%6.1f ", getPlatformHeading());
    sprintf(aa+strlen(aa), "rl:%5.1f ", getPlatformRoll());
    sprintf(aa+strlen(aa), "p:%5.1f ", getPlatformPitch());
  }
  else
  {
    sprintf(aa+strlen(aa), "fx:%6.1f ", swib->fixed_angle);
    sprintf(aa+strlen(aa), "az:%6.1f ", getAzimuthAngleCalc());
    sprintf(aa+strlen(aa), "el:%6.2f ", getElevationAngleCalc());
  }
  aa += strlen(aa);
  sprintf(aa+strlen(aa), "swp: %2d ", ryib->sweep_num);
  sprintf(aa+strlen(aa), "%s ", postamble);
  if(verbose)
  {
    sprintf(aa+strlen(aa), "\n");

    sprintf(aa+strlen(aa), "la:%9.4f ", asib->latitude);
    sprintf(aa+strlen(aa), "lo:%9.4f ", asib->longitude);
    if(dds->radd->scan_mode == DD_SCAN_MODE_AIR)
    {
      sprintf(aa+strlen(aa), "p:%5.1f ", asib->pitch);
      sprintf(aa+strlen(aa), "d:%5.1f ", asib->drift_angle);
      sprintf(aa+strlen(aa), "ag:%6.3f ", asib->altitude_agl);
      sprintf(aa+strlen(aa), "\n");
      sprintf(aa+strlen(aa), "ve:%6.1f ", asib->ew_velocity);
      sprintf(aa+strlen(aa), "vn:%6.1f ", asib->ns_velocity);
      sprintf(aa+strlen(aa), "vv:%6.1f ", asib->vert_velocity);
      sprintf(aa+strlen(aa), "we:%6.1f ", asib->ew_horiz_wind);
      sprintf(aa+strlen(aa), "wn:%6.1f ", asib->ns_horiz_wind);
      sprintf(aa+strlen(aa), "wv:%6.1f ", asib->vert_wind);
    }
    else
    {
      sprintf(aa+strlen(aa), "al:%6.3f ", asib->altitude_msl);
    }

    sprintf(aa+strlen(aa), "\n");

    sprintf(aa+strlen(aa), "Num parameters: %d  ",
	    _info->source_num_parms);
    for (int ii = 0; ii < _info->source_num_parms; ii++)
    {
      sprintf(aa+strlen(aa), "%s ",
	      str_terminate(str, dds->parm[ii]->parameter_name, 8));
    }
    sprintf(aa+strlen(aa), "\n");
  }
}

#endif


/*********************************************************************
 * putFieldValues()
 */

#ifdef USE_RADX
#else

void DataInfo::putFieldValues(const int param_num,
			      const double angle,
			      const double range,
			      struct solo_field_vals &sfv,
			      const int state)
{
  int nc = _info->dds->celvc->number_cells;
  float max_r = _info->dds->celvc->dist_cells[nc-1];
  sfv.state = state;

  for (int j = 0; j < 5; j++)
  {
    sfv.ranges[j] = -999;
    sfv.field_vals[j] = -999;
  }
  
  // Get the data into a floating point array for this routine.
  // Gate # begins at 1 instead of 0

  float rng = KM_TO_M(range);
  int gate;
  float cell_val;
  
  dd_range_gate(_info, rng, &gate, &cell_val);
  gate -= 2;

  int pn = param_num;
  int n;
  int five = 5;
  float f_data[7];
  float f_bad;
  
  if (pn < _info->num_parms)
    n = dd_givfld(_info, pn, gate, five, f_data, &f_bad);

  float f_rngs[7];
  
  dd_rng_info(_info, &gate, &five, f_rngs, &n);
  int gg = rng > max_r ? nc + 1 : gate;
    
  for (int j = 0; j < 5; j++, gg++)
  {
    if (gg > nc)
      break;

    if (gg < 1)
      continue;

    if (sfv.state != SFV_GARBAGE)
      sfv.field_vals[j] = f_data[j];

    sfv.ranges[j] = f_rngs[j];
  }

  sfv.num_vals = 5;
  sfv.bad_flag = f_bad;
  sfv.rng = range;
  sfv.rot_ang = _info->dds->radd->scan_mode == DD_SCAN_MODE_RHI ?
    getElevationAngleCalc() : getRotationAngleCalc();
  sfv.tilt = getTiltAngle();
  sfv.az = getAzimuthAngleCalc();
  sfv.el = getElevationAngleCalc();
  sfv.lat = getPlatformLatitude();
  sfv.lon = getPlatformLongitude();
  sfv.alt = getPlatformAltitudeMSL();
  sfv.agl = _info->dds->asib->altitude_agl;
  sfv.earth_r =
    EarthRadiusCalculator::getInstance()->getRadius(sfv.lat);
  sfv.time = _info->time;
  sfv.heading = _info->dds->asib->heading;
}

#endif


/*********************************************************************
 * rayCoverage()
 */

#ifdef USE_RADX
#else

struct dd_ray_sector *DataInfo::rayCoverage(const int req_ray_num)
{
  // This routine returns the sector in degrees covered by the request ray

  struct rot_ang_table *rat = _info->source_rat;
  
  struct rot_table_entry *ray0, *ray1, *ray2;
  static struct dd_ray_sector rs;
    
  int ray_num = req_ray_num;

  // NOTE:  Shouldn't we have a special case handling ray_num == 0?
  // In testing, this method doesn't seem to ever be called with ray_num
  // set to 0, but it would be good to make sure that case is handled here.

  ray0 = (struct rot_table_entry *)((char *)rat + rat->first_key_offset);
  if (ray_num == 1)
  {
    ray1 = ray0 + 1;
    rs.sector = angdiff(ray0->rotation_angle, ray1->rotation_angle);
    rs.angle0 = FMOD360(ray0->rotation_angle - (0.5 * rs.sector) + 360.0);
    rs.angle1 = FMOD360(ray0->rotation_angle + (0.5 * rs.sector) + 360.0);
    rs.rotation_angle = ray0->rotation_angle;
  }
  else if (ray_num >= rat->num_rays)
  {
    ray_num = rat->num_rays;
    ray0 += ray_num - 2;
    ray1 = ray0 + 1;
    rs.sector = angdiff(ray0->rotation_angle, ray1->rotation_angle);
    rs.angle0 = FMOD360(ray1->rotation_angle - (0.5 * rs.sector) + 360.0);
    rs.angle1 = FMOD360(ray1->rotation_angle + (0.5 * rs.sector) + 360.0);
    rs.rotation_angle = ray1->rotation_angle;
  }
  else
  {
    // NOTE: Shouldn't this be "ray0 += ray_num - 1;" ???  Or is ray_num
    // 1-based?

    ray0 += ray_num - 2;
    ray1 = ray0 + 1;
    ray2 = ray0 + 2;
	
    rs.angle0 = ray0->rotation_angle +
      (0.5 * angdiff(ray0->rotation_angle, ray1->rotation_angle));
    rs.angle1 = ray1->rotation_angle +
      (0.5 * angdiff(ray1->rotation_angle, ray2->rotation_angle));
	
    rs.sector = angdiff(rs.angle0, rs.angle1);
    rs.rotation_angle = ray1->rotation_angle;
  }

  return &rs;
}

#endif


/*********************************************************************
 * tape()
 */

#ifdef USE_RADX
#else

void DataInfo::tape(const bool flush_all)
{
  if (flush_all)
    dd_tape(_info, DD_FLUSH_ALL);
  else
    dd_tape(_info, NO);
}

#endif


/*********************************************************************
 * updateSweepFile()
 */

#ifdef USE_RADX
#else

void DataInfo::updateSweepFile(const bool editing)
{
  struct dds_structs *dds = _info->dds;	
  struct super_SWIB *sswb = dds->sswb;
  struct sweepinfo_d *swib = dds->swib;

  dd_write(_info->sweep_fid, (char *)dds->NULL_d, sizeof(struct null_d));
  sswb->sizeof_file += sizeof(struct null_d);

  sswb->num_key_tables = 2;

  // Write the rotation angle table and reset it

  sswb->key_table[NDX_ROT_ANG].offset = sswb->sizeof_file;
  sswb->key_table[NDX_ROT_ANG].size = _info->rat->sizeof_struct;
  sswb->key_table[NDX_ROT_ANG].type = KEYED_BY_ROT_ANG;
  dd_write(_info->sweep_fid, (char *)_info->rat, _info->rat->sizeof_struct);
  sswb->sizeof_file += _info->rat->sizeof_struct;
  dd_rotang_table(_info, RESET);

  // Save the solo edit summary

  if (_info->sizeof_seds > 0)
  {
    struct generic_descriptor *gd = (struct generic_descriptor *)_info->seds;
    
    // Save the solo edit summary

    sswb->key_table[NDX_SEDS].offset = sswb->sizeof_file;
    sswb->key_table[NDX_SEDS].type = SOLO_EDIT_SUMMARY;
    sswb->key_table[NDX_SEDS].size = gd->sizeof_struct;
    dd_write(_info->sweep_fid, _info->seds, gd->sizeof_struct);
    sswb->sizeof_file += gd->sizeof_struct;
  }
      
  // Rewrite the SSWB and SWIB

  dd_position(_info->sweep_fid, 0);  // Rewind
  dd_write(_info->sweep_fid, (char *)sswb, sizeof(struct super_SWIB));
  dd_position(_info->sweep_fid, _info->offset_to_swib);
  dd_write(_info->sweep_fid, (char *)swib, sizeof(struct sweepinfo_d));
  dd_close(_info->sweep_fid);

  _info->sweep_fid = 0;
  if (editing)
  {
    std::string path = _info->directory_name;
    if (path[path.size()-1] != '/')
      path += "/";
    path += _info->orig_sweep_file_name;
    
    dd_unlink(path.c_str());
  }

  // Rename the sweepfile by removing the ".tmp" suffix

  dd_rename_swp_file(_info);
}

#endif


/*********************************************************************
 * zapSweepFile()
 */

#ifdef USE_RADX
#else

void DataInfo::zapSweepFile()
{
  dd_close(_info->sweep_fid);
  _info->sweep_fid = 0;
  dd_rotang_table(_info, RESET);

  std::string path = _info->directory_name;
  if (path[path.size()-1] != '/')
    path += "/";
  path += _info->sweep_file_name;
  dd_unlink(path);
}

#endif


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
