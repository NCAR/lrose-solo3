#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <DateTime.hh>
#include <dd_files.h>
#include <dd_files.hh>
#include <dd_math.h>
#include <dda_common.hh>
#include <dorade_share.hh>
#include <EarthRadiusCalculator.hh>
#include <se_utils.hh>
#include <sii_externals.h>
#include <sii_utils.hh>
#include <solo_window_structs.h>
#include <solo2.hh>
#include <sp_basics.hh>
#include <swp_file_acc.hh>

#include "DataManager.hh"

// Global variables

// NOTE: Currently, this value MUST match the MAX_PARMS value defined in
// dd_defines.h.  In the future, want to get rid of the arbitrary limit.
const int DataManager::MAX_PARAMS = 64;

DataManager *DataManager::_instance = (DataManager *)0;

/*********************************************************************
 * Constructors
 */

DataManager::DataManager()
{
  // Set the instance pointer to point to this singleton instance

  _instance = this;
}


/*********************************************************************
 * Destructor
 */

DataManager::~DataManager()
{
}


/*********************************************************************
 * getInstance()
 */

DataManager *DataManager::getInstance()
{
  if (_instance == 0)
    new DataManager();
  
  return _instance;
}


/*********************************************************************
 * checkSweepFile()
 */

void DataManager::checkSweepFile()
{
  // Get the sweepfile information

  struct dd_input_sweepfiles_v3 *dis = dd_return_sweepfile_structs_v3();
  struct unique_sweepfile_info_v3 *usi = dis->usi_top;

  // Get the window information

  struct solo_perusal_info *spi = solo_return_winfo_ptr();

  // Loop through each of the windows and see if this sweepfile is being
  // used anywhere

  for (int ww = 0; ww < SOLO_MAX_WINDOWS; ww++)
  {
    // If the window isn't active, it isn't using a sweep file

    if (!spi->active_windows[ww])
      continue;

    // I don't really understand this check

    WW_PTR wwptr = solo_return_wwptr(ww);
    if (wwptr->lead_sweep != wwptr)
      continue;

    // If this window is using this sweep file, indicate that the file has been
    // modified

    if (usi->directory == wwptr->sweep->directory_name &&
	usi->filename == wwptr->sweep->file_name)
      wwptr->sweep_file_modified = YES;
  }
}


/*********************************************************************
 * compileFileList()
 */

int DataManager::compileFileList(const int dir_num, const std::string &dir)
{
  return ddir_files_v3(dir_num, dir.c_str());
}

int DataManager::compileFileList(const int dir_num, const std::string &dir,
                                 int argc, char *argv[])
{
  return ddir_files_from_command_line(dir_num, dir.c_str(), argc, argv);
}


/*********************************************************************
 * flushFile()
 */

#ifdef USE_RADX
#else

void DataManager::flushFile(const int flush_radar_num)
{
  // Dump out the partially filled buffer for the current sweep and
  // close the file

  struct dd_input_sweepfiles_v3 *dis = dd_return_sweepfile_structs_v3();

  DataInfo *data_info = 0;

  if (dis->editing)
    data_info = getWindowInfo(flush_radar_num);
  else 
    data_info = getRadarInfo(flush_radar_num);
    
  // If the file isn't open, don't do anything

  if (data_info->getOutputFileId() == 0)
    return;
	
  if (data_info->getDDbufSize() != 0)
  {
    // Dump buffer

    dd_write(data_info->getOutputFileId(),
	     data_info->getDDbufPtr(), data_info->getDDbufSize());
    data_info->clearDDbuf();
  }

  if (data_info->getFileSize() > 0 &&
      data_info->getNumRays() >= dd_min_rays_per_sweep() &&
      !data_info->isIgnoreSweep())
  {
    // There really is some data so update the sweep file.

    data_info->updateSweepFile(dis->editing);
  }
  else if (data_info->getOutputFileId() != 0)
  {
    // Too short...zap it

    data_info->zapSweepFile();
  }
}

#endif


/*********************************************************************
 * generateSweepFileList()
 */

int DataManager::generateSweepFileList(const double start_time,
				       const double stop_time)
{
  // This just gets a pointer to the directory information.  This is a pointer
  // to a single global structure used by everyone.

  struct dd_input_sweepfiles_v3 *dis = dd_return_sweepfile_structs_v3();

  // Generates the list of sweep files in the current directory.
  // Returns the number of sweep files in the list.

  int return_value = dd_sweepfile_list_v3(dis->usi_top, start_time, stop_time);
  
  {
    struct unique_sweepfile_info_v3 *usi = dis->usi_top;
    
    for (int i = usi->swp_count; i < usi->num_swps; ++i)
    {
      dd_file_name_v3 *ddfn = usi->final_swp_list[i];
      char filename[1024];
      ddfn_file_name(ddfn, filename);
    }
  }
  
  return return_value;
}


/*********************************************************************
 * generateSweepList()
 */

bool DataManager::generateSweepList(const int frame_num,
				    const std::string &dir,
				    const std::string &radar_name,
				    const int version,
				    const int new_version_flag,
				    const double start_time,
				    const double stop_time,
				    int &radar_num)
{
  // Internally compile a list of sweep files in the given directory.

  ddir_rescan_urgent(frame_num);
  
  if (ddir_files_v3(frame_num, dir.c_str()) < 1)
  {
    g_string_sprintfa(gs_complaints, "No sweep files in dir: %s\n",
		      dir.c_str());
    return false;
  }

  // Get the radar number associated with this radar name.  We are finding the
  // number in the directory list generated above.

  if ((radar_num = mddir_radar_num_v3(frame_num, radar_name.c_str())) < 0)
  {
    g_string_sprintfa(gs_complaints, "No sweep files for radar: %s\n",
		      radar_name.c_str());
    return false;
  }

  // Get a pointer to the sweepfile data structure

  struct dd_input_sweepfiles_v3 *dis = dd_return_sweepfile_structs_v3();
  struct unique_sweepfile_info_v3 *usi = dis->usi_top;

  // Set the internal structure values so that we get the appropriate sweep
  // file list

  usi->dir_num = frame_num;
  usi->radar_num = frame_num;
  usi->ddir_radar_num = radar_num;
  usi->version = version;
  usi->new_version_flag = new_version_flag;
  usi->directory = dir;

  // Create the list of sweep files for the above radar number and directory
  // that are between the given times.  The list is put in usi->final_swp_list.
  // n is the number of files in the list.

  int num_files;
  
  if ((num_files = dd_sweepfile_list_v3(usi, start_time, stop_time)) < 0)
  {
    g_string_sprintfa(gs_complaints,
		      "Unable to produce sweep file list for radar: %s in %s\n",
		      radar_name.c_str(), dir.c_str());
    return false;
  }

  return true;
}


/*********************************************************************
 * getDataInfoTime()
 */

double DataManager::getDataInfoTime(const int dir_num, const int radar_num,
				    const double target_time,
				    const time_type_t file_action,
				    const version_t version,
				    std::string &file_info,
				    std::string &file_name)
{
  int real_version;
  
  char *char_file_info = new char[file_info.size() + 1000];
  strcpy(char_file_info, file_info.c_str());
  
  char *char_file_name = new char[file_name.size() + 100];
  strcpy(char_file_name, file_name.c_str());
  
  double data_time =d_mddir_file_info_v3(dir_num, radar_num, target_time,
					 (int)file_action, (int)version,
					 &real_version,
					 char_file_info, char_file_name);
  file_info = char_file_info;
  file_name = char_file_name;
  delete [] char_file_info;
  delete [] char_file_name;
  
  return data_time;
}


/*********************************************************************
 * getClosestDataTime()
 */

double DataManager::getClosestDataTime(const int dir_num, const int radar_num,
				       const double target_time,
				       const int version)
{
  int real_version;
  char info[128];
  char name[128];
  
  return d_mddir_file_info_v3(dir_num, radar_num, target_time,
			      TIME_NEAREST, version, &real_version,
			      info, name);
}


/*********************************************************************
 * getDDopenInfo()
 */

std::string DataManager::getDDopenInfo() const
{
  return return_dd_open_info();
}


/*********************************************************************
 * getDirNum()
 */

int DataManager::getDirNum() const
{
  struct dd_input_sweepfiles_v3 *dis = dd_return_sweepfile_structs_v3();
  return dis->usi_top->dir_num;
}


/*********************************************************************
 * getFileBaseName()
 */

std::string DataManager::getFileBaseName(const std::string &type,
					 const int32_t time_stamp,
					 const std::string &radar,
					 const int version) const
{
  char file_name[256];
    
  DD_TIME dts;

  dts.time_stamp = time_stamp;
  d_unstamp_time(&dts);

  sprintf(file_name,
	  "%s.%02d%02d%02d%02d%02d%02d.%s.%d",
	  type.c_str(),
	  dts.year -1900,
	  dts.month,
	  dts.day,
	  dts.hour,
	  dts.minute,
	  dts.second,
	  radar.c_str(),
	  version);

  return std::string(file_name);
}


///*********************************************************************
// * getTimeFromFileName()
// */
//
//DateTime DataManager::getTimeFromFileName(const std::string &file_name) const
//{
//  // Separate the file name into tokens
//
//  std::vector< std::string > tokens;
//  tokenize(file_name, tokens, ".");
//  
//  // Parse the time.  The last 10 characters are MMDDHHMMSS.  The first
//  // characters are the year, which can be specified explicitly (e.g. "2012")
//  // or as the number of years since 1900 (e.g. "112")
//
//  std::string time_string = tokens[1];
//  std::string day_time_string = time_string.substr(time_string.size()-10);
//  std::string year_string = time_string.substr(0, time_string.size()-10);
//  
//  int month, day, hr, min, sec;
//
//  sscanf(day_time_string.c_str(), "%2d%2d%2d%2d%2d",
//	 &month, &day, &hr, &min, &sec);
//
//  int year = atoi(year_string.c_str());
//  if (year < 1900)
//    year += 1900;
//  
//  return DateTime(year, month, day, hr, min, sec);
//}


/*********************************************************************
 * getFileList()
 */

void DataManager::getFileList(const int dir_num,
			      const std::string &dir,
			      const int radar_num,
			      std::vector< std::string > &file_list)
{
  ddir_rescan_urgent(dir_num);

  if (compileFileList(dir_num, dir.c_str()) < 1)
  {
    g_string_sprintfa(gs_complaints,"Directory %s contains no sweep files\n",
		      dir.c_str());
    return;
  }

  struct solo_list_mgmt *slm = SLM_malloc_list_mgmt(50);
  mddir_gen_swp_str_list_v3(dir_num, radar_num, YES, slm);

  file_list.clear();
  for (int i = 0; i < slm->num_entries; ++i)
    file_list.push_back(SLM_list_entry(slm, i));
  SLM_unmalloc_list_mgmt(slm);
}
  

/*********************************************************************
 * getFileInfoList()
 */

void DataManager::getFileInfoList(const int dir_num,
				  const int radar_num,
				  std::vector< std::string > &file_list)
{
  struct solo_list_mgmt *slm = SLM_malloc_list_mgmt(50);
  mddir_gen_swp_str_list_v3(dir_num, radar_num, NO, slm);

  for (int i = 0; i < slm->num_entries; ++i)
    file_list.push_back(SLM_list_entry(slm, i));
  
  SLM_unmalloc_list_mgmt(slm);
}
  

/*********************************************************************
 * getDataInfo()
 */

double DataManager::getDataInfo(const int dir_num, const int radar_num,
				const int sweep_num, int32_t &version,
				std::string &file_info, std::string &file_name)
{
  // Get the data information

  int version_int = version;
  
  char *char_file_info = new char[file_info.size() + 1000];
  strcpy(char_file_info, file_info.c_str());
  
  char *char_file_name = new char[file_name.size() + 100];
  strcpy(char_file_name, file_name.c_str());
  
  double data_time = ddfnp_list_entry(dir_num, radar_num, sweep_num,
				      &version_int,
				      char_file_info, char_file_name);

  file_info = char_file_info;
  file_name = char_file_name;
  delete [] char_file_info;
  delete [] char_file_name;
  
  // Set the return values not set above

  version = version_int;
  return data_time;
}


/*********************************************************************
 * getNumRadars()
 */

int DataManager::getNumRadars(const int dir_num) const
{
  return mddir_num_radars_v3(dir_num);
}


/*********************************************************************
 * getNumRays()
 */

int DataManager::getNumRays() const
{
  struct dd_input_sweepfiles_v3 *dis = dd_return_sweepfile_structs_v3();
  return dis->usi_top->num_rays;
}


/*********************************************************************
 * getRadarDir()
 */

std::string DataManager::getRadarDir() const
{
  struct dd_input_sweepfiles_v3 *dis = dd_return_sweepfile_structs_v3();
  return std::string(dis->usi_top->directory);
}


/*********************************************************************
 * getRadarFilename()
 */

std::string DataManager::getRadarFilename() const
{
  struct dd_input_sweepfiles_v3 *dis = dd_return_sweepfile_structs_v3();
  return std::string(dis->usi_top->filename);
}


/*********************************************************************
 * getRadarNameData()
 */

char *DataManager::getRadarNameData(const int dir_num, const int radar_num)
{
  return mddir_radar_name_v3(dir_num, radar_num);
}


/*********************************************************************
 * getRadarNum()
 */

int DataManager::getRadarNum() const
{
  struct dd_input_sweepfiles_v3 *dis = dd_return_sweepfile_structs_v3();
  return dis->usi_top->radar_num;
}


/*********************************************************************
 * getRayNumData()
 */

int DataManager::getRadarNumData(const int dir_num,
				 const std::string &radar_name)
{
  return mddir_radar_num_v3(dir_num, radar_name.c_str());
}


/*********************************************************************
 * getRayNum()
 */

int DataManager::getRayNum() const
{
  struct dd_input_sweepfiles_v3 *dis = dd_return_sweepfile_structs_v3();
  return dis->usi_top->ray_num;
}


/*********************************************************************
 * getSweepNum()
 */

int DataManager::getSweepNum() const
{
  struct dd_input_sweepfiles_v3 *dis = dd_return_sweepfile_structs_v3();
  return dis->usi_top->swp_count;
}


/*********************************************************************
 * getTimeSeriesStartTime()
 */

double DataManager::getTimeSeriesStartTime(const int dir_num,
					   const int radar_num,
					   const double target_time)
{
  int version;
  char info[128];
  char name[128];
  
  return dd_ts_start(dir_num, radar_num, target_time,
		     &version, info, name);
}


/*********************************************************************
 * getTimeSeriesStartTimeInfo()
 */

double DataManager::getTimeSeriesStartTimeInfo(const int dir_num,
					       const int radar_num,
					       const double target_time,
					       std::string &file_info,
					       std::string &file_name)
{
  // Call dd_ts_start() with the given parameters.  file_info and file_name
  // can be changed by the call, so convert them to char*'s and reset the 
  // values on return.  version isn't used by us.

  int version;
  
  char *char_file_info = new char[file_info.size() + 1000];
  strcpy(char_file_info, file_info.c_str());
  
  char *char_file_name = new char[file_name.size() + 100];
  strcpy(char_file_name, file_name.c_str());
  
  double start_time = dd_ts_start(dir_num, radar_num, target_time,
				  &version, char_file_info, char_file_name);

  file_name = char_file_name;
  file_info = char_file_info;
  delete [] char_file_name;
  delete [] char_file_info;
  
  return start_time;
}


/*********************************************************************
 * getNumSweeps()
 */

int DataManager::getNumSweeps() const
{
  struct dd_input_sweepfiles_v3 *dis = dd_return_sweepfile_structs_v3();
  return dis->usi_top->num_swps;
}


/*********************************************************************
 * isLastRayInSweep()
 */

bool DataManager::isLastRayInSweep() const
{
  struct dd_input_sweepfiles_v3 *dis = dd_return_sweepfile_structs_v3();
  return ddswp_last_ray(dis->usi_top);
}


/*********************************************************************
 * loadNextRay()
 */

#ifdef USE_RADX
#else

bool DataManager::loadNextRay()
{
  struct dd_input_sweepfiles_v3 *dis = dd_return_sweepfile_structs_v3();
  struct unique_sweepfile_info_v3 *usi = dis->usi_top;
  
  // Keep sucking up input until all headers (if applicable )
  // and a complete ray have been read in.

  // Absorb the next ray

  // NOTE: I don't think we need this here, but this does have the side effect
  // of allocating space for the stats pointer if it hasn't been allocated yet.

  dd_return_stats_ptr();
  
  DataInfo *data_info = getWindowInfo(usi->radar_num);
  data_info->setFirstCfac(usi->first_cfac);

  if (usi->ray_num >= usi->num_rays)
  {
    // Get the next file name and open it

    if (!_getNewSweep(usi))
      return false;
    
    usi->ray_num = 0;
    if (usi->vol_num != data_info->getVolumeNum())
      usi->vol_num = data_info->getVolumeNum();
  }	

  if (!data_info->loadNextRay())
    return false;
  
  usi->ray_num++;
  data_info->stuffRay();

  return true;
}

#endif


/*********************************************************************
 * nabNextFile()
 */

bool DataManager::nabNextFile(const int frme, const time_type_t file_action,
			      const version_t version, const bool replot)
{
  // Convert the internal enum values to the values used in the translate
  // module

  int dd_file_action = 0;

  switch (file_action)
  {
  case FILTER_TIME_BEFORE :
    dd_file_action = DD_FILTER_TIME_BEFORE;
    break;
  case TIME_BEFORE :
    dd_file_action = DD_TIME_BEFORE;
    break;
  case TIME_NEAREST :
    dd_file_action = DD_TIME_NEAREST;
    break;
  case TIME_AFTER :
    dd_file_action = DD_TIME_AFTER;
    break;
  case FILTER_TIME_AFTER :
    dd_file_action = DD_FILTER_TIME_AFTER;
    break;
  }
  
  int dd_version = 0;
  switch (version)
  {
  case LATEST_VERSION :
    dd_version = DD_LATEST_VERSION;
    break;
  case EARLIEST_VERSION :
    dd_version = DD_EARLIEST_VERSION;
    break;
  case EXHAUSTIVE_LIST :
    dd_version = DD_EXHAUSTIVE_LIST;
    break;
  }
  
  // Get the window pointer

  WW_PTR wwptr = solo_return_wwptr(frme);

  // The assumption is that the frame number is the first frame of a set of
  // linked frames sharing sweepfile information

  bool swpfi_time_sync =
    wwptr->swpfi_time_sync_set && wwptr->lead_sweep->window_num != 0;

  if (swpfi_time_sync)
    dd_file_action = TIME_NEAREST;

  // See if a filter has been set

  bool filter_set = false;
  float ffxd = 0.0;
  float tol = 0.0;

  std::string scan_modes_string;
  
  if (dd_file_action != TIME_NEAREST && wwptr->swpfi_filter_set)
  {
    filter_set = true;
    scan_modes_string = wwptr->filter_scan_modes;
    ffxd = wwptr->filter_fxd_ang;
    tol = wwptr->filter_tolerance;
  }
    
  if (wwptr->sweep->changed)
  {
    wwptr->sweep->changed = NO;
    ddir_rescan_urgent(frme);
  }

  // If the editor has created new version of files for this radar, rescan
  // the data directory

  if (replot && dd_version == LATEST_VERSION && wwptr->sweep_file_modified)
    ddir_rescan_urgent(frme);
  
  if (mddir_file_list_v3(frme, wwptr->sweep->directory_name) < 1)
  {
    char mess[256];
    
    sprintf(mess, "Directory %s contains no sweep files\n",
	    wwptr->sweep->directory_name);
    g_message(mess);
    return false;
  }

  if ((wwptr->sweep->radar_num =
       mddir_radar_num_v3(frme, wwptr->sweep->radar_name)) < 0)
  {
    char mess[256];
    
    sprintf(mess, "No sweep files for radar %s\n",
	    wwptr->sweep->radar_name);
    g_message(mess);
    return false;
  }
  
  int vn_ref = 0;
  
  std::vector< std::string > scan_modes;
  
  if (filter_set)
  {
    int vv;
    float fxd;
    
    // Call d_mddir_file_info_v3().  Because this call could change the value
    // of info_line but we don't want to waste our time updating that code,
    // copy the string into a char array and then copy the new value back after
    // the call.

    char *char_info_line = new char[wwptr->show_file_info.size() + 1000];
    strcpy(char_info_line, wwptr->show_file_info.c_str());

    d_mddir_file_info_v3(frme, wwptr->sweep->radar_num,
			 wwptr->d_sweepfile_time_stamp,
			 TIME_NEAREST, dd_version, &vv,
			 char_info_line,
			 wwptr->sweep->file_name);
    wwptr->show_file_info = char_info_line;
    delete [] char_info_line;
    
    std::string file_scan_mode;
    vn_ref =
      _extractFileInfo(wwptr->sweep->file_name, fxd, file_scan_mode);
    if (vn_ref < 0)
      filter_set = false;

    tokenize(scan_modes_string, scan_modes, sii_item_delims());
  }
  
  for (int ii = 0; ii < 1 || filter_set; ii++)
  {
    wwptr->d_prev_time_stamp = wwptr->d_sweepfile_time_stamp;

    if (swpfi_time_sync)
    {
      wwptr->d_sweepfile_time_stamp =
	solo_return_wwptr(0)->d_sweepfile_time_stamp;
    }

    int vv;
    
    char *char_info_line = new char[wwptr->show_file_info.size() + 1000];
    strcpy(char_info_line, wwptr->show_file_info.c_str());
    
    wwptr->d_sweepfile_time_stamp = 
      d_mddir_file_info_v3(frme, wwptr->sweep->radar_num,
			   wwptr->d_sweepfile_time_stamp,
			   dd_file_action, dd_version, &vv,
			   char_info_line,
			   wwptr->sweep->file_name);
    wwptr->show_file_info = char_info_line;
    delete [] char_info_line;
    
    wwptr->sweep->start_time = wwptr->d_sweepfile_time_stamp;
    wwptr->sweep->time_stamp = (int32_t)wwptr->sweep->start_time;

    // If this is the last sweep in the directory for this radar, return

    if (!swpfi_time_sync && !replot &&
	wwptr->d_prev_time_stamp == wwptr->d_sweepfile_time_stamp)
      return false;
    
    if (filter_set)
    {
      float fxd;
      std::string file_scan_mode;
      
      int vn =
	_extractFileInfo(wwptr->sweep->file_name, fxd, file_scan_mode);
      
      if (vn == vn_ref)
	continue;

      double diff = FABS(fxd - ffxd);
      if (diff > tol)
	continue;

      if (scan_modes.size() == 0)
	break;

      size_t jj;
      for (jj = 0; jj < scan_modes.size(); jj++)
      {
	if (file_scan_mode == scan_modes[jj])
	  break;
      }

      if (jj < scan_modes.size())
	break;
    }
  }

  wwptr->sweep->start_time = wwptr->d_sweepfile_time_stamp;
  wwptr->sweep->stop_time = wwptr->d_sweepfile_time_stamp;
  wwptr->sweep->time_stamp = (int32_t)wwptr->sweep->start_time;

  // Open the sweep file

  if (_openDataFile(wwptr) < 0)
    return false;

  // Update the window information

  DataInfo *data_info = getWindowInfo(frme);
  
#ifdef USE_RADX

  // Read the file using Radx

  std::string file_path = wwptr->sweep->directory_name;
  if (file_path[file_path.size()-1] != '/')
    file_path += "/";
  file_path += wwptr->sweep->file_name;
  
  data_info->loadRadxFile(file_path, 0);
  
#else

  data_info->setDir(wwptr->sweep->directory_name);
  
  data_info->setInputFileId(wwptr->file_id);
  // Load the headers and the first ray from this file.
  if (! data_info->loadHeaders() || ! data_info->loadNextRay()) {
      printf("Failed to load headers or first ray from '%s'\n",
              wwptr->sweep->file_name);
      return false;
  }

#endif
  
  wwptr->radar_location.setLatitude(data_info->getPlatformLatitude());
  wwptr->radar_location.setLongitude(data_info->getPlatformLongitude());
  wwptr->radar_location.setAltitude(data_info->getPlatformAltitudeMSL());
  wwptr->radar_location.setHeading(data_info->getPlatformHeading());
  wwptr->radar_location.setTilt(data_info->getFixedAngle());
  double earth_radius =
    EarthRadiusCalculator::getInstance()->getRadius(wwptr->radar_location.getLatitude());
  wwptr->radar_location.setEarthRadius(earth_radius);
  wwptr->radar_location.clearState();
  wwptr->radar_location.setStateEarth(true);
  wwptr->scan_mode = data_info->getScanMode();
  wwptr->sweep->radar_type = data_info->getRadarType();

  strncpy(wwptr->sweep->scan_type,
	  data_info->scanMode2Str(data_info->getScanMode()).c_str(), 16);
  wwptr->time_modified = time_now();

  struct ts_sweep_info *tssi = wwptr->tsrt->tssi_top + 1;
  tssi->dir_num = frme;
  tssi->ray_count = data_info->getNumRays();
  strncpy(tssi->directory, wwptr->sweep->directory_name, 128);
  strncpy(tssi->filename, wwptr->sweep->file_name, 88);
  wwptr->tsrt->sweep_count = 1;

  return true;
}


/*********************************************************************
 * setEditFlag()
 */

void DataManager::setEditFlag(const bool flag)
{
  struct dd_input_sweepfiles_v3 *dis = dd_return_sweepfile_structs_v3();

  if (flag)
    dis->editing = YES;
  else
    dis->editing = NO;
}


/*********************************************************************
 * setDirNum()
 */

void DataManager::setDirNum(const int dir_num)
{
  struct dd_input_sweepfiles_v3 *dis = dd_return_sweepfile_structs_v3();
  dis->usi_top->dir_num = dir_num;
}


/*********************************************************************
 * setNewVersionFlag()
 */

void DataManager::setNewVersionFlag(const bool new_version)
{
  struct dd_input_sweepfiles_v3 *dis = dd_return_sweepfile_structs_v3();
  if (new_version)
    dis->usi_top->new_version_flag = YES;
  else
    dis->usi_top->new_version_flag = NO;
}


/*********************************************************************
 * setOutputFlag()
 */

void DataManager::setOutputFlag(const bool flag)
{
  if (flag)
    dd_output_flag(YES);
  else
    dd_output_flag(NO);
}


/*********************************************************************
 * setPrintDDopenInfo()
 */

void DataManager::setPrintDDopenInfo(const bool print_flag)
{
  if (print_flag)
    do_print_dd_open_info();
  else
    dont_print_dd_open_info();
}


/*********************************************************************
 * setRadarDir()
 */

void DataManager::setRadarDir(const std::string &dir)
{
  struct dd_input_sweepfiles_v3 *dis = dd_return_sweepfile_structs_v3();
  dis->usi_top->directory = dir;
}


/*********************************************************************
 * setRadarNum()
 */

void DataManager::setRadarNum(const int radar_num)
{
  struct dd_input_sweepfiles_v3 *dis = dd_return_sweepfile_structs_v3();
  dis->usi_top->radar_num = radar_num;
}


/*********************************************************************
 * setRadarNumData()
 */

void DataManager::setRadarNumData(const int radar_num)
{
  struct dd_input_sweepfiles_v3 *dis = dd_return_sweepfile_structs_v3();
  dis->usi_top->ddir_radar_num = radar_num;
}


/*********************************************************************
 * setRescanUrgent()
 */

void DataManager::setRescanUrgent(const int dir_num)
{
  ddir_rescan_urgent(dir_num);
}
  

/*********************************************************************
 * setVersion()
 */

void DataManager::setVersion(const version_t version)
{
  struct dd_input_sweepfiles_v3 *dis = dd_return_sweepfile_structs_v3();
  dis->usi_top->version = version;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _getNewSweep()
 */

#ifdef USE_RADX
#else

bool DataManager::_getNewSweep(struct unique_sweepfile_info_v3 *usi)
{
  int ierr=0;
  struct dd_input_sweepfiles_v3 *dis;

  if (usi->swp_count >= usi->num_swps)
  {
    usi->forget_it = YES;
    return false;
  }

  // Construct the new sweep file name

  dd_file_name_v3 *ddfn = *(usi->final_swp_list + usi->swp_count++);
  char *char_filename = new char[usi->filename.size()+100];
  strcpy(char_filename, usi->filename.c_str());
  ddfn_file_name(ddfn, char_filename);
  usi->filename = char_filename;
  delete [] char_filename;

  DataInfo *info = getWindowInfo(usi->radar_num);

  // Save the qualifier

  if (*ddfn->comment)
    info->setFileQualifier(ddfn->comment);
  else
    info->setFileQualifier("");

  dis = dd_return_sweepfile_structs_v3();
  if (dis->editing)
  {
    if (usi->new_version_flag)
      info->setVersion(ddfn->version + 1);
    else
      info->setVersion(ddfn->version);
  }

  // Close the current input file

  if (info->getInputFileId() > 0)
    close(info->getInputFileId());

  // Open the new file
  // NOTE: Where to usi->directory and usi->filename get set???

  std::string file_path = usi->directory + usi->filename;
  
  info->setOriginalSweepFileName(usi->filename);
  info->setInputFileId(open(file_path.c_str(), 0));
  
  if (info->getInputFileId() < 1)
  {
    ierr = YES;
    printf("Could not open file %s in dap_new_swp status: %d\n",
	   file_path.c_str(), info->getInputFileId());
    usi->forget_it = YES;
    return false;
  }
  info->loadHeaders();
  usi->num_rays = info->getNumRaysSource();
  info->setSweepQueSegmentNum(ddfn->milliseconds);

  return true;
}

#endif


/*********************************************************************
 * _extractFileInfo()
 */

int DataManager::_extractFileInfo(const std::string &filename,
				  float &fixed_angle, std::string &scan_mode)
{
  // Separate the filename into the underscore-delimited sections.  The first
  // substring should be something like:
  //    swp.1111016023031.SPOL.575.0.5

  std::vector< std::string > tokens;
  tokenize(filename, tokens, "_");

  // The last part of the first token is the fixed angle.  In the example
  // above, that would be the "0.5" at the end of token 0.  To get this value,
  // look for the second "." from the end, then convert that value to float.

  std::size_t dot_pos = tokens[0].rfind(".");
  if (dot_pos == std::string::npos)
    return -1;
  
  dot_pos = tokens[0].rfind(".", dot_pos-1);
  if (dot_pos == std::string::npos)
    return -1;
  
  std::string fixed_angle_string = tokens[0].substr(dot_pos+1);
  fixed_angle = atof(fixed_angle_string.c_str());
  
  // Now pull out the scan mode.  The scan mode will be the token before the
  // volume number

  std::size_t jj;
  
  for (jj = 1; jj < tokens.size(); jj++)
  {
    if (tokens[jj][0] == 'v')
      break;
  }

  if (jj >= tokens.size())
    return -1;
  
  scan_mode = tokens[jj-1];
  
  // Now pull the volume number out of that token and return it

  return atoi(tokens[jj].substr(1).c_str());
}


/*********************************************************************
 * _openDataFile()
 */

int DataManager::_openDataFile(struct solo_window_ptrs *wwptr) const
{
  // Construct the file path

  std::string file_path = wwptr->sweep->directory_name;
  if (file_path[file_path.size()-1] != '/')
    file_path += "/";
  file_path += wwptr->sweep->file_name;

  // Close the previous file

  if (wwptr->file_id > 0)
    close(wwptr->file_id);

  // Open the new file

  if ((wwptr->file_id = open(file_path.c_str(), O_RDONLY, 0)) < 0)
  {
    std::string message = "Unable to open sweep "+ file_path + "\n";
    solo_message(message.c_str());
  }
  
  return wwptr->file_id;
}
