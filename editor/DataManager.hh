#ifndef DataManager_HH
#define DataManager_HH

#include <iostream>
#include <vector>

#include <DataInfo.hh>


class DataManager
{

public:

  //////////////////
  // Public types //
  //////////////////

  // NOTE: For now, these values MUST match the corresponding values in
  // include/dd_files.h

  typedef enum
  {
    FILTER_TIME_BEFORE = 4,
    TIME_BEFORE = 2,
    TIME_NEAREST = 0,
    TIME_AFTER = 1,
    FILTER_TIME_AFTER = 3
  } time_type_t;
  
  typedef enum
  {
    LATEST_VERSION = -1,
    EARLIEST_VERSION = -2,
    EXHAUSTIVE_LIST = -3
  } version_t;

     
  //////////////////////
  // Public constants //
  //////////////////////

  /**
   * @brief The maximum number of parameters allowed in a file.
   */

  static const int MAX_PARAMS;
  

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Destructor.
   */

  virtual ~DataManager();

  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @return Returns a pointer to the instance.
   */

  static DataManager *getInstance();
  
  
  /////////////////////
  // Utility methods //
  /////////////////////

  // NOTE: Replaces DD_SCALE calls.

  /**
   * @brief Apply the given scale and offset to the given data value.
   *
   * @param[in] unscaled_value     The unscaled value.
   * @param[in] scale              The scale.
   * @param[in] offset             The offset.
   *
   * @return Returns the scaled value.
   */

  static int scaleValue(const double unscaled_value, const double scale,
			const double offset)
  {
    return (int)((unscaled_value * scale) + offset + 0.5);
  }
  
  ////////////////////
  // Access methods //
  ////////////////////

  // NOTE: Replaces dd_assign_radar_num() call

  /**
   * @brief Get the index for the given radar.  If this radar isn't found,
   *        create a new info structure for it and return that index.
   *
   * @param[in] radar_name    The name of the radar
   *
   * @return Returns the index for the radar.
   */

#ifdef USE_RADX
#else

  inline std::size_t assignRadarIndex(const std::string &radar_name)
  {
    // Check for a null string

    if (radar_name == "")
      return -1;
    
    // See if we already have information about this radar.  If so, return
    // the index.

    for (size_t index = 0; index < _radarInfo.size(); ++index)
    {
      if (_radarInfo[index]->getRadarName2() == radar_name)
	return index;
    }
    
    // If we get here, this is a new radar so create a new information
    // structure.

    _radarInfo.push_back(new DataInfo(_radarInfo.size(), radar_name));
    return _radarInfo.size() - 1;
  }

#endif

  // NOTE: Replaces dd_return_radar_num() call.

  /**
   * @brief Get the index associated with the indicated radar.
   *
   * @param[in] radar_name    The name of the radar.
   *
   * @return Returns the radar index if found, -1 otherwise.
   */

#ifdef USE_RADX
#else

  int getRadarIndex(const std::string &radar_name)
  {
    // Check for the null cases
    
    if (radar_name == "" || _radarInfo.size() == 0)
      return -1;
    
    for (std::size_t index = 0; index < _radarInfo.size(); ++index)
    {
      if (_radarInfo[index]->getRadarName2() == radar_name)
	return index;
    }
    
    // If we get here, the radar name wasn't found

    return -1;
  }

#endif

  // NOTE: Replaces dd_get_structs() call

  /**
   * @brief Get access to the indicated radar data information.
   *
   * @param[in] radar_num    The radar index (0-based).
   *
   * @return Returns a pointer to the indicated information on success, 0 on
   *         failure.
   */

  inline DataInfo *getRadarInfo(const size_t radar_num)
  {
    if (radar_num >= _radarInfo.size())
      return 0;
    
    return _radarInfo[radar_num];
  }
  
  // NOTE: Replaces dd_window_dgi() call

  /**
   * @brief Get access to the indicated window data information.
   *
   * @param[in] window_num   The window index (0-based).
   *
   * @return Returns a pointer to the indicated information on success, 0 on
   *         failure.
   */

  inline DataInfo *getWindowInfo(const size_t window_num)
  {
    // Make sure we have enough frame manager pointers in the list to
    // accommodate this request.

    if (window_num >= _windowInfo.size())
    {
      for (size_t i = _windowInfo.size(); i <= window_num; ++i)
	_windowInfo.push_back((DataInfo *)0);
    }
    
    // Get the indicated frame manager from the list

    DataInfo *manager = _windowInfo[window_num];
    
    // If this manager hasn't be created yet, create it

    if (manager == 0)
    {
      manager = new DataInfo(window_num);
      
      _windowInfo[window_num] = manager;
    }
    
    return manager;
  }
  
  // NOTE:  Replaces ddfnp_list_entry() calls.

  /**
   * @brief Get information about the indicated data file.
   *
   * @param[in] dir_num      The directory number.
   * @param[in] radar_num    The radar number.
   * @param[in] sweep_num    The sweep number.
   * @param[out] version     The version.
   * @param[out] file_info   The file information.
   * @param[out] file_name   The file name.
   *
   * @return Returns the data time of the file.
   */

  double getDataInfo(const int dir_num, const int radar_num,
		     const int sweep_num, int32_t &version,
		     std::string &file_info, std::string &file_name);
  
  // NOTE: Replaces d_mddir_file_info_v3() call where true_version_num
  // is ignored but info and name are returned.

  /**
   * @brief Get the information about the indicated file based on time.
   *
   * @param[in] dir_num       The directory number.
   * @param[in] radar_num     The radar number.
   * @param[in] target_time   The target time.
   * @param[in] file_action   The file action.
   * @param[in] version       The version.
   * @param[out] file_info    The file information.
   * @param[out] file_name    The file name.
   *
   * @return Returns the data time of the closest file.
   */

  double getDataInfoTime(const int dir_num, const int radar_num,
			 const double target_time,
			 const time_type_t file_action,
			 const version_t version,
			 std::string &file_info, std::string &file_name);

  // NOTE: Replaces d_mddir_file_info_v3() call where true_version_num,
  // info_line and file_name are ignored.

  /**
   * @brief Get the data time of the file closest to the indicated file.
   *
   * @param[in] dir_num       The directory number.
   * @param[in] radar_num     The radar number.
   * @param[in] target_time   The target time.
   * @param[in] version       The version.
   *
   * @return Returns the data time of the closest file.
   */

  double getClosestDataTime(const int dir_num, const int radar_num,
			    const double target_time, const int version);

  /**
   * @brief Get the number of radars in the given directory.
   *
   * @param[in] dir_num    The directory number.
   *
   * @return Returns the number of radars in the directory.
   */

  int getNumRadars(const int dir_num) const;
  
  // NOTE: Replaces dd_ts_start() calls where version, info and name are
  // ignored.

  /**
   * @brief Get the time series start time.
   *
   * @param[in] dir_num       The directory number.
   * @param[in] radar_num     The radar number.
   * @param[in] target_time   The target time.
   *
   * @return Returns the time series start time.
   */

  double getTimeSeriesStartTime(const int dir_num, const int radar_num,
				const double target_time);
  
  // NOTE: Replaces dd_ts_start() calls where version is ignored.

  /**
   * @brief Get the time series start time.
   *
   * @param[in] dir_num       The directory number.
   * @param[in] radar_num     The radar number.
   * @param[in] target_time   The target time.
   * @param[out] file_info    The file information.
   * @param[out] file_name    The file name.
   *
   * @return Returns the time series start time.
   */

  double getTimeSeriesStartTimeInfo(const int dir_num, const int radar_num,
				    const double target_time,
				    std::string &file_info,
				    std::string &file_name);
  
  /**
   * @brief Get information about the open data file.  The returned string
   *        will be of the form " file <filename>:<filedesc>" where
   *        <filename> is the filename of the file without the directory and
   *        <filedesc> is the file descriptor assigned to the file when it was
   *        created.
   *
   * @return Returns information about the open data file.
   */

  std::string getDDopenInfo() const;
  
  /**
   * @brief Set the edit flag.
   *
   * @param[in] flag   The new flag value.
   */

  void setEditFlag(const bool flag);
  
  // NOTE: I'm not sure how this differs from the directory name in the
  // window information (DataInfo).

  /**
   * @brief Get the radar directory.
   *
   * @return Returns the directory.
   */

  std::string getRadarDir() const;

  /**
   * @brief Set the radar directory.
   *
   * @param[in] dir  The directory.
   */

  void setRadarDir(const std::string &dir);

  /**
   * @brief Get the radar filename.
   *
   * @return Returns the filename.
   */

  std::string getRadarFilename() const;

  /**
   * @brief Get the radar number.
   *
   * @return Returns the current radar number.
   */

  int getRadarNum() const;
  
  // NOTE: Update to return a string

  /**
   * @brief Get the name for this radar in the given data directory.
   *
   * @param[in] dir_num     The directory number.
   * @param[in] radar_num   The radar number.
   *
   * @return Returns the radar name if the radar exists in the given data
   *         directory, 0 otherwise.
   */

  char *getRadarNameData(const int dir_num, const int radar_num);
  
  /**
   * @brief Get the number for this radar in the given data directory.
   *
   * @param[in] dir_num     The directory number.
   * @param[in] radra_name  The radar name.
   *
   * @return Returns the radar number if the radar exists in the given data
   *         directory, -1 otherwise.
   */

  int getRadarNumData(const int dir_num, const std::string &radar_name);
  
  /**
   * @brief Get the ray number.
   *
   * @return Returns the current ray number.
   */

  int getRayNum() const;
  
  /**
   * @brief Get the current sweep number.
   *
   * @return Return the current sweep number.
   */

  int getSweepNum() const;
  
  /**
   * @brief Get the number of sweeps.
   *
   * @return Returns the number of sweeps.
   */

  int getNumSweeps() const;
  
  /**
   * @brief Get the directory number.
   *
   * @return Returns the current directory number.
   */

  int getDirNum() const;
  
  /**
   * @brief Set the directory number.
   *
   * @param[in] dir_num  The current directory number.
   */

  void setDirNum(const int dir_num);
  
  /**
   * @brief Set the version.
   *
   * @param[in] version  The version.
   */

  void setVersion(const version_t version);
  
  /**
   * @brief Set the new version flag.
   *
   * @param[in] new_version  The new version flag.
   */

  void setNewVersionFlag(const bool new_version);
  
  /**
   * @brief Get the number of rays.
   *
   * @return Returns the number of rays.
   */

  int getNumRays() const;
  
  /**
   * @brief Set the radar number.
   *
   * @param[in] radar_num  The current radar number.
   */

  void setRadarNum(const int radar_num);
  
  /**
   * @brief Set the radar number used for the data directory.
   *
   * @param[in] radar_num     The radar number.
   */

  void setRadarNumData(const int radar_num);
  

  /////////////////////////
  // File naming methods //
  /////////////////////////

  /**
   * @brief Get the standard base file name using the given information.
   *
   * @param[in] type        Type of file.
   * @param[in] time_stamp  The file time stamp in seconds.
   * @param[in] radar       The radar name.
   * @param[in] version     
   */

  std::string getFileBaseName(const std::string &type, const int32_t time_stamp,
			      const std::string &radar,
			      const int version) const;
  
  /**
   * @brief Extract the data time from the given file name.
   *
   * @param[in] file_name    The file name.
   *
   * @return Returns the data time.
   */

//  int32_t getTimeFromFileName(const std::string &file_name) const;
  DateTime getTimeFromFileName(const std::string &file_name) const;


  //////////////////////////
  // Input/output methods //
  //////////////////////////

  // NOTE: Replaces code in se_gen_sweep_list()

  /**
   * @brief Generate a list of sweep files for the given parameters.  The
   *        list is stored internally.
   *
   * @param[in] frame_num         The frame number.
   * @param[in] dir               The directory path.
   * @param[in] radar_name        The radar name.
   * @param[in] version
   * @param[in] new_version_flag
   * @param[in] start_time        The data start time.
   * @param[in] stop_time         The data stop time.
   * @param[out] radar_num        The number for the given radar.
   *
   * @return Returns true on success, false on failure.
   */

  bool generateSweepList(const int frame_num,
			 const std::string &dir,
			 const std::string &radar_name,
			 const int version,
			 const int new_version_flag,
			 const double start_time,
			 const double stop_time,
			 int &radar_num);
  
  /**
   * @brief Generate the list of sweep files.  The list is maintained
   *        internally.
   *
   * @param[in] start_time    The data start time.
   * @param[in] stop_time     The data stop time.
   *
   * @return Returns the number of files found.
   */

  int generateSweepFileList(const double start_time,
				   const double stop_time);
  
  // NOTE: Replaces dd_flush() calls.

  /**
   * @brief Flush the indicated file.
   *
   * @param[in] flush_radar_num    The radar to flush.
   */

#ifdef USE_RADX
#else

  void flushFile(const int flush_radar_num);

#endif
  
  // NOTE: Replaces sp_sweep_file_check() calls.

  /**
   * @brief Determine if the current file is being used in any of the windows
   *        and set a flag that says it might have changed since the last
   *        access.
   */

  void checkSweepFile();

  // NOTE:  This method replaces ddswp_next_ray_v3().  It uses
  // dd_return_sweepfile_structs_v3()->usi_top as the usi pointer.

  /**
   * @brief Load the next ray from the data file and stuff it into the
   *        current window.
   *
   * @return Returns true if there was a new ray, false otherwise.
   */

#ifdef USE_RADX
#else

  bool loadNextRay();

#endif  

  // NOTE:  This method replaces ddswp_last_ray().  It uses
  // dd_return_sweepfile_structs_v3()->usi_top as the usi pointer.

  /**
   * @brief Determines whether we are working with the last ray in a sweep.
   *
   * @return Returns true if it is the last ray in the sweep, false otherwise.
   */

  bool isLastRayInSweep() const;
  
  /**
   * @brief Nab the next data file.
   *
   * @param[in] frme
   * @param[in] file_action
   * @param[in] version
   * @param[in] replot
   *
   * @return Returns true on success, false otherwise.
   */

  bool nabNextFile(const int frme, time_type_t file_action,
		   const version_t version, const bool replot);
  

  /**
   * @brief Compile the list of files in the indicated data directory.
   *
   * @param[in] dir_num  The directory number.
   * @param[in] dir      The directory path.
   *
   * @return Returns the number of data files available in the directory.
   */

  int compileFileList(const int dir_num, const std::string &dir);
  
  // NOTE: Change this to return a vector of strings and get rid of the
  // solo_list_mgmt object.

  /**
   * @brief Compile and return the list of data files in the indicated
   *        directory.
   *
   * @param[in] dir_num    The directory number.
   * @param[in] dir        The directory path.
   * @param[in] radar_num  The radar number.
   * @param[out] file_list The list of the file names.
   */

  void getFileList(const int dir_num,
		   const std::string &dir,
		   const int radar_num,
		   std::vector< std::string > &file_list);
  
  // NOTE: Replaces mddir_gen_swp_list_v3() calls.

  /**
   * @brief Return the list of information about the data files in the 
   *        indicated directory.  Does not trigger a new compilation of the
   *        file list.
   *
   * @param[in] dir_num     The directory number.
   * @param[in] radar_num   The radar number.
   * @param[out] file_list  The list of file information.
   */

  void getFileInfoList(const int dir_num,
		       const int radar_num,
		       std::vector< std::string > &file_list);

  /**
   * @brief Set the flag for printing the data file open information.
   *
   * @param[in] print_flag    The new flag value.
   */

  void setPrintDDopenInfo(const bool print_flag);
  
  /**
   * @brief Set the rescan urgent flag for the indicated directory.
   *
   * @param[in] dir_num   The directory number.
   */

  void setRescanUrgent(const int dir_num);
  
  // NOTE: I'm not sure what this output flag value is used for.

  /**
   * @brief Set the output flag value.
   *
   * @param[in] flag   The new flag value.
   */

  void setOutputFlag(const bool flag);

  
protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Singleton instance pointer.
   */

  static DataManager *_instance;
  
  /**
   * @brief List of window data information, indexed by frame number.
   */

  std::vector< DataInfo* > _windowInfo;
  
  /**
   * @brief List of radar data information, indexed by radar number.
   */

  std::vector< DataInfo* > _radarInfo;

  
  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Constructor -- Protected since this is a singleton object.
   */

  DataManager();

  // NOTE: Replaces ddswp_new_sweep_v3(struct unique_sweepfile_info_v3 *usi) calls

  /**
   * @brief Get the next sweep in the input dataset.
   *
   * @param[in,out] usi   The unique sweepfile information.
   *
   * @return Returns true on success, false on failure.
   */

#ifdef USE_RADX
#else

  bool _getNewSweep(struct unique_sweepfile_info_v3 *usi);

#endif  
  
  /**
   * @brief Extract the file information (volume number, fixed angle and
   *        scan mode) from the given file name.
   *
   * @param[in] filename     The file name.
   * @param[out] fixed_angle The fixed angle.
   * @param[out] scan_mode   The scan mode.
   *
   * @return Returns the volume number on success, -1 on failure.
   */

  static int _extractFileInfo(const std::string &filename,
			      float &fixed_angle, std::string &scan_mode);
  

  /**
   * @brief Open the current data file.
   *
   * @param[in] wwptr   The pointer to the window information.
   *
   * @returns Returns the file descriptor for the newly opened file on
   *          success, -1 on failure.
   */

  int _openDataFile(struct solo_window_ptrs *wwptr) const;

};

#endif
