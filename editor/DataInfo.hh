#ifndef DataInfo_HH
#define DataInfo_HH

#include <iostream>
#include <map>
#include <string>
#include <vector>

#ifdef USE_RADX
#include <Radx/RadxFile.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxVol.hh>
#else
#include <dd_general_info.h>
#endif

#include <DateTime.hh>
#include <FileInfo.hh>
#include <solo_window_structs.h>

class DataInfo
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  // NOTE:  See if we really need the window number.  I'm guessing we can get
  // rid of this and just send in a 0 or something.

  /**
   * @brief Constructor.
   *
   * @param[in] index   The index for this data.
   * @param[in] name    The name for this data.
   */

  DataInfo(const std::size_t index,
	   const std::string &name = "");

  /**
   * @brief Destructor.
   */

  virtual ~DataInfo();

  /**
   * @brief ???????
   */

  int alternateGecho(const double min_grad, int &zmax_cell);

  
  /**
   * @brief Established a possible new field but does not copy the data from
   *        the source field into the new field
   *
   * @param[in] src_name     The source field name.
   * @param[in] dst_name     The destination field name.
   * @param[out] src_index   The source field index.
   * @param[out] dst_index   The destination field index.
   *
   * @return Returns the destination field index on success, -1 on failure.
   */

#ifdef USE_RADX
#else

  int establishField(const std::string &src_name,
		     const std::string &dst_name,
		     int &src_index, int &dest_index);

#endif
  
  /**
   * @brief Remove the specified field.
   *
   * @param[in] field_name    The field name.
   *
   * @return Returns true if the field was found, false otherwise.
   */

#ifdef USE_RADX
#else
  inline bool removeField(const std::string &field_name)
  {
    int field_num = getParamIndex(field_name);
    if (field_num < 0)
      return false;
    
    _info->dds->field_present[field_num] = NO;
    return true;
  }
#endif
  
  /**
   * @brief Calculate the sector in degrees covered by the selected ray.
   *
   * @param[in] ray_num    The ray number, starts at 1.
   *
   * @return Returns the sector covered by the ray.
   */

#ifdef USE_RADX
#else

  struct dd_ray_sector *rayCoverage(const int ray_num);

#endif
  

  ////////////////////////
  // Command processing //
  ////////////////////////

  // NOTE: Change the command list to std::vector< std::string >

  /**
   * @brief Add the given edit commands to the command list.
   *
   * @param[in] cmd_list   The commands.
   */

  void addCommands(const std::vector< std::string > &cmd_list);

  /**
   * @brief Determine if we have any commands in our list.
   *
   * @return Returns true if we have commands, false otherwise.
   */

#ifdef USE_RADX
  inline bool haveCommands() const
  {
    return _sizeofSeds > 0 && _seds != 0;
  }
#else
  inline bool haveCommands() const
  {
    return _info->sizeof_seds > 0 && _info->seds != 0;
  }
#endif
  
  /**
   * @brief Get the pointer to the commands.
   *
   * @return Returns a pointer to the commands.
   */

#ifdef USE_RADX
  inline char *getCommandPtr()
  {
    return _seds;
  }
#else
  inline char *getCommandPtr()
  {
    return _info->seds;
  }
#endif
  
  /**
   * @brief Get the number of bytes in the commands.
   *
   * @return Returns the number of bytes in the commands.
   */

#ifdef USE_RADX
  inline int getCommandSize() const
  {
    return _sizeofSeds;
  }
#else
  inline int getCommandSize() const
  {
    return _info->sizeof_seds;
  }
#endif
  

#ifdef USE_RADX

  //////////////////
  // Radx methods //
  //////////////////

  /**
   * @brief Load the sweep data from the indicated Radx file.
   *
   * @param[in] file_path      The full path for the Radx file.
   * @param[in] sweep_index    The index of the sweep to load.
   *
   * @return Returns true on success, false on failure.
   */

  bool loadRadxFile(const std::string &file_path,
		    const int sweep_index);
  

  /**
   * @brief Get the current sweep data.
   *
   * @return Returns a pointer to the current sweep data on success,
   *         0 on failure.
   */

  const RadxSweep *getRadxSweep() const
  {
    const vector< RadxSweep* > sweeps = _radxVol.getSweeps();

    if (sweeps.size() == 0)
      return 0;
    
    return sweeps[0];
  }
  
  RadxSweep *getRadxSweep()
  {
    const vector< RadxSweep* > sweeps = _radxVol.getSweeps();

    if (sweeps.size() == 0)
      return 0;
    
    return sweeps[0];
  }
  

  /**
   * @brief Get the current volume data.
   *
   * @return Returns a reference to the current volume data.
   */

  const RadxVol &getRadxVolume() const
  {
    return _radxVol;
  }
  
  RadxVol &getRadxVolume()
  {
    return _radxVol;
  }
  
  /**
   * @brief Write the current Radx volume to disk.
   *
   * @return Returns true on success, false on failure.
   */

  bool writeRadxVolume();
  
#endif

  /**
   * @brief Get the list of time series files.
   *
   * @return Returns the list of time series files.
   */

  inline std::vector< FileInfo > getTimeSeriesFileList() const
  {
    std::vector< FileInfo > file_list;
    
    for (std::vector< FileInfo >::const_iterator file_info = _timeSeriesFileList.begin();
	 file_info != _timeSeriesFileList.end(); ++file_info)
      file_list.push_back(*file_info);
    
    return file_list;
  }
  
  /**
   * @brief Set the time serise file list to contain all files between the
   *        given data times.
   *
   * @param[in] start_time     The start time for the list.
   * @param[in] stop_time      The stop time for the list.
   *
   * @return Returns true on success, false on failure.  If no files were
   *         found between the given times, false is returned.
   */

#ifdef USE_RADX
#else

  bool setTimeSeriesFileList(const DateTime &start_time,
			     const DateTime &stop_time);

#endif
  

  ////////////////////
  // Access methods //
  ////////////////////

  /**
   * @brief Get aircraft velocity.
   *
   * @return Returns the aircraft velocity.
   */

  double getAircraftVelocity() const;

  /**
   * @brief Get the ray index for the given angle.
   *
   * @param[in] theta    The rotation angle.
   *
   * @return Returns the ray index.
   */

  int getAngleIndex(const double theta) const;


  /**
   * @brief Get the azimuth angle.
   *
   * @return Returns the azimuth angle.
   */

#ifdef USE_RADX
#else

  inline double getAzimuthAngle() const
  {
    return _info->dds->ryib->azimuth;
  }

  /**
   * @brief Get the azimuth angle, taking into account the aircraft type.
   *
   * @return Returns the azimuth angle.
   */

  double getAzimuthAngleCalc() const;
  
#endif
  
  /**
   * @brief Get the cell vector.
   *
   * @return Returns a pointer to the cell vector.
   */

#ifdef USE_RADX
#else
  inline struct cell_d *getCellVectorC()
  {
    return _info->dds->celvc;
  }
#endif
  
  // NOTE: This is used as num_gates = getClipGate() + 1, so maybe replace
  // this with getNumGates().  Although, I'm not sure what the difference is
  // between clip_gate and dds->celv->number_cells.

  /**
   * @brief Get the clip gate.
   *
   * @return Returns the clip gate.
   */

#ifdef USE_RADX
#else
  inline int getClipGate() const
  {
    return _info->clip_gate;
  }
#endif
  
  /**
   * @brief Find the gate closest to the given range.
   *
   * @param[in] range        The desired range.
   * @param[out] gate_num    The closest gate index.
   * @param[out] gate_range  The range of the gate.
   */

#ifdef USE_RADX
#else

  void getClosestGate(const double range,
		      int &gate_num, float &gate_range) const;

#endif
  
  /**
   * @brief Get the current directory.
   *
   * @return Returns the current directory.
   */

#ifdef USE_RADX
#else
  std::string getDir() const
  {
    return _info->directory_name;
  }
#endif

  /**
   * @brief Get the elevation angle.
   *
   * @return Returns the elevation angle.
   */

#ifdef USE_RADX
#else

  inline double getElevationAngle() const
  {
    return _info->dds->ryib->elevation;
  }

  inline double getElevationAngle2() const
  {
    return _info->dds->ra->elevation;
  }

  /**
   * @brief Get the elevation angle, taking into account the aircraft type
   *        and scan mode.
   *
   * @return Returns the elevation angle.
   */

  double getElevationAngleCalc() const;
  
#endif
  
  /**
   * @brief Get the east-west component of the horizontal wind.
   *
   * @return Returns the east-west component of the horizontal wind.
   */

#ifdef USE_RADX
#else
  inline double getEWHorizWind() const
  {
    return _info->dds->asib->ew_horiz_wind;
  }
#endif
  
  /**
   * @brief Get the fixed angle.
   *
   * @return Returns the fixed angle in degrees.
   */

#ifdef USE_RADX

  inline double getFixedAngle() const
  {
    const std::vector< RadxSweep* > sweeps = _radxVol.getSweeps();
    const RadxSweep *sweep = sweeps[0];
    
    return sweep->getFixedAngleDeg();
  }

#else

  inline double getFixedAngle() const
  {
    return _info->dds->swib->fixed_angle;
  }

#endif
  
  /**
   * @brief Get the indicated frequency value.
   *
   * @param[in] freq_num   The frequency index.  This value must be between
   *                         0 and 4.  If it is outside of this value, the
   *                         first frequency value (index 0) is returned.
   *
   * @return Returns the frequency value.
   */

#ifdef USE_RADX

  inline const std::vector< double > getFrequencyHz() const
  {
    return _radxVol.getFrequencyHz();
  }

#else

  inline double getFrequency(const std::size_t freq_num) const
  {
    if (freq_num == 0)
      return _info->dds->radd->freq1;
    else if (freq_num == 1)
      return _info->dds->radd->freq2;
    else if (freq_num == 2)
      return _info->dds->radd->freq3;
    else if (freq_num == 3)
      return _info->dds->radd->freq4;
    else if (freq_num == 4)
      return _info->dds->radd->freq5;
    else
      return _info->dds->radd->freq1;
  }

#endif
  
  /**
   * @brief Get the gate index for the given range.
   *
   * @param[in] range   The range in km.
   *
   * @return Returns the gate index.
   */

#ifdef USE_RADX
#else

  std::size_t getGateNum(const double range) const;

#endif
  
#ifdef USE_RADX

  /**
   * @brief Get the range for the indicated gate in km.
   *
   * @param[in] gate_num    The index of the gate.
   *
   * @return Returns the gate range.
   */

  inline double getGateRangeKm(const std::size_t gate_num) const
  {
    return _radxVol.getGateRangeKm(gate_num);
  }

#else

  // NOTE: I don't know what the difference is between celv and celvc.  I'm
  // guessing celvc measures the center of the cell and celv measures the start
  // of the cell.  Different ones are used at different parts of the code so
  // I'm providing methods for both here.  We might be able to consolidate
  // these.
  //
  // se_header_value() sets the values the same in both arrays.

  /**
   * @brief Get the range for the indicated gate in m.
   *
   * @param[in] gate_num    The index of the gate.
   *
   * @return Returns the gate range.
   */

  inline double getGateRange(const std::size_t gate_num) const
  {
    return _info->dds->celv->dist_cells[gate_num];
  }

  /**
   * @brief Get the range for the indicated gate in m.
   *
   * @param[in] gate_num    The index of the gate.
   *
   * @return Returns the gate range.
   */

  inline double getGateRangeC(const std::size_t gate_num) const
  {
    return _info->dds->celvc->dist_cells[gate_num];
  }

#endif
  
  /**
   * @brief Get the gate spacing in km.
   *
   * @return Returns the gate spacing in km.
   */

#ifdef USE_RADX
#else

  inline double getGateSpacing() const
  {
    return _info->dds->celv->dist_cells[1] -
      _info->dds->celv->dist_cells[0];
  }

#endif
  
  /**
   * @brief Get the gate spacing in km.
   *
   * @return Returns the gate spacing in km.
   */

#ifdef USE_RADX
#else

  inline double getGateSpacingC() const
  {
    return _info->dds->celvc->dist_cells[1] -
      _info->dds->celvc->dist_cells[0];
  }

#endif
  
  /**
   * @brief Get the value of the indicated interpulse period.
   *
   * @param[in] period_num    The period index.  Must be a value between 0 and
   *                            4.  If outside this value, the first interpulse
   *                            period is returned.
   *
   * @return Returns the value of the first interpulse period.
   */

#ifdef USE_RADX
#else

  inline double getInterPulsePeriod(const std::size_t period_num) const
  {
    if (period_num == 0)
      return _info->dds->radd->interpulse_per1;
    else if (period_num == 1)
      return _info->dds->radd->interpulse_per2;
    else if (period_num == 2)
      return _info->dds->radd->interpulse_per3;
    else if (period_num == 3)
      return _info->dds->radd->interpulse_per4;
    else if (period_num == 4)
      return _info->dds->radd->interpulse_per5;
    else
      return _info->dds->radd->interpulse_per1;
  }

#endif
  
  /**
   * @brief Get the north-south component of the horizontal wind.
   *
   * @return Returns the north-south component of the horizontal wind.
   */

#ifdef USE_RADX
#else

  inline double getNSHorizWind() const
  {
    return _info->dds->asib->ns_horiz_wind;
  }

#endif
  
#ifdef USE_RADX

  /**
   * @brief Get the number of cells (gates?) in the data.
   *
   * @return Returns the number of cells.
   */

  inline int getMaxNGates() const
  {
    return _radxVol.getMaxNGates();
  }

#else

  /**
   * @brief Get the number of cells (gates?) in the data.
   *
   * @return Returns the number of cells.
   */

  inline int getNumCells() const
  {
    return _info->dds->celv->number_cells;
  }

  /**
   * @brief Get the number of cells (gates?) in the data.
   *
   * @return Returns the number of cells.
   */

  inline int getNumCellsC() const
  {
    return _info->dds->celvc->number_cells;
  }

#endif
  
  /**
   * @brief Get the number of frequencies transmitted.
   *
   * @return Returns the number of frequencies transmitted.
   */

#ifdef USE_RADX
#else

  inline int getNumFreqTrans() const
  {
    return _info->dds->radd->num_freq_trans;
  }

#endif
  
  /**
   * @brief Get the number of different inter-pulse periods transmitted.
   *
   * @return Returns the number of different inter-pulse periods.
   */

#ifdef USE_RADX
#else
  
  inline int getNumInterPulsePeriods() const
  {
    return _info->dds->radd->num_ipps_trans;
  }

#endif
  
  // NOTE:  How is this different from _info->source_num_parms???

  /**
   * @brief Get the number of parameters in the data.
   *
   * @return Returns the number of parameters.
   */

#ifdef USE_RADX

  inline std::size_t getNumParams() const
  {
    return _radxVol.getNFields();
  }

#else

  inline std::size_t getNumParams() const
  {
    return _info->num_parms;
  }

#endif
  
  // NOTE: What is the difference between the different num_rays values???

  /**
   * @brief Get the number of rays.
   *
   * @return Returns the number of rays.
   */

#ifdef USE_RADX

  inline int getNumRays() const
  {
    const std::vector< RadxSweep* > sweeps = _radxVol.getSweeps();
    const RadxSweep *sweep = sweeps[0];
    
    return sweep->getNRays();
  }

#else

  inline int getNumRays() const
  {
    return _info->dds->swib->num_rays;
  }

#endif
  
  /**
   * @brief Get the number of rays in the RAT.
   *
   * @return Returns the number of rays.
   */

#ifdef USE_RADX
#else

  inline int getNumRaysRAT() const
  {
    return _info->source_rat->num_rays;
  }

#endif
  
  /**
   * @brief Get the number of rays in the source.
   *
   * @return Returns the number of rays.
   */

#ifdef USE_RADX
#else

  inline int getNumRaysSource() const
  {
    return _info->source_num_rays;
  }

#endif
  
  /**
   * @brief Get the nyquist velocity.
   *
   * @return Returns the nyquist velocity.
   */

#ifdef USE_RADX
#else

  inline double getNyquistVelocity() const
  {
    return _info->dds->radd->eff_unamb_vel;
  }

#endif
  
  /**
   * @brief Get the original sweep file name.
   *
   * @return Returns the original sweep file name.
   */

#ifdef USE_RADX
#else

  std::string getOrigSweepfileName() const
  {
    return _info->orig_sweep_file_name;
  }

#endif

  /**
   * @brief Get the bad data value for the indicated parameter.
   *
   * @param[in] param_num   The parameter index.
   *
   * @return Returns the bad data value.
   */

#ifdef USE_RADX
#else

  inline double getParamBadDataValue(const std::size_t param_num) const
  {
    return _info->dds->parm[param_num]->bad_data;
  }

#endif
  
  /**
   * @brief Get the bias value for the indicated parameter.
   *
   * @param[in] param_num   The parameter index.
   *
   * @return Returns the bias value.
   */

#ifdef USE_RADX
#else

  inline double getParamBias(const std::size_t param_num) const
  {
    return _info->dds->parm[param_num]->parameter_bias;
  }

#endif
  
  /**
   * @brief Get the binary format value for the indicated parameter.
   *
   * @param[in] param_num    The parameter index.
   *
   * @return Returns the binary format value.
   */

#ifdef USE_RADX
#else

  inline short getParamBinaryFormat(const std::size_t param_num) const
  {
    return _info->dds->parm[param_num]->binary_format;
  }

#endif
  
  /**
   * @brief Get the data for the indicated parameter.  The pointer is returned
   *        as stored -- no unscaling is performed.
   *
   * @param[in] param_num   The parameter index.
   *
   * @return Returns an array of the paramter data.
   */

#ifdef USE_RADX
#else

  inline char *getParamData(const std::size_t param_num)
  {
    return _info->dds->qdat_ptrs[param_num];
  }

#endif
  
  /**
   * @brief Get the unscaled data for the indicated parameter.
   *
   * @param[in] param_num          The parameter index.
   * @param[in] start_gate         The starting gate number (1-based).
   * @param[in] num_gates          The number of gates desired.
   * @param[out] buffer            The data buffer.
   * @param[out] bad_data_value    The bad data value.
   *
   * @return Returns the number of gates retrieved.
   */

  int getParamDataUnscaled(const std::size_t param_num,
			   const std::size_t start_gate,
			   const std::size_t num_gates,
			   float *buffer, float &bad_data_value);
  
  // NOTE:  Replace all of the "Param" methods with calls to a "Field" or
  // "Param" class of some sort.
  /**
   * @brief Get the index for the given parameter.
   *
   * @param[in] param_name    The name of the parameter.
   *
   * @return Returns the parameter's index on success, -1 on failure.
   */


  int getParamIndex(const std::string &param_name) const;
  
  /**
   * @brief Get the list of indices for the available parameters.
   *
   * @return Returns the list of indices.
   */


#ifdef USE_RADX
#else

  inline std::vector< std::size_t > getParamIndices() const
  {
    std::vector< std::size_t > index_list;
    
    for (int ii = 0; ii < _info->num_parms; ii++)
    {
      if (!_info->dds->field_present[ii])
	continue;
      index_list.push_back(ii);
    }

    return index_list;
  }

#endif

  // NOTE:  Need to change parameter_d to a new parameter information object.

  /**
   * @brief Get a pointer to the parameter information.
   *
   * @param[in] param_num   The parameter index.
   *
   * @return Returns a pointer to the parameter information.
   */

#ifdef USE_RADX
#else

  inline struct parameter_d *getParamInfo(const std::size_t param_num)
  {
    return _info->dds->parm[param_num];
  }

#endif
  
  /**
   * @brief Get the name of the indicated parameter.
   *
   * @param[in] param_num  The parameter index.
   *
   * @return Returns the parameter name.
   */

#ifdef USE_RADX

  inline std::string getParamName(const std::size_t param_num) const
  {
    RadxField *field = _radxVol.getField(param_num);
    
    if (field == 0)
      return "NONE";
    
    return field->getName();
  }

#else

  inline std::string getParamName(const std::size_t param_num) const
  {
    return _info->dds->parm[param_num]->parameter_name;
  }


  // NOTE: Replaces str_terminate(getParamName()) calls.

  /**
   * @brief Get a cleaned up version of the parameter name.
   *
   * @param[in] param_num  The parameter index.
   * @param[in] max_chars  The maximum string length to return.
   *
   * @return Returns the parameter name.
   */

  inline std::string getParamNameClean(const std::size_t param_num,
				       const std::size_t max_chars) const
  {
    std::string param_name = getParamName(param_num);

    // Remove leading blanks.  If no non-blank characters are found then the
    // string is blank.

    std::size_t first_nonblank_pos =
      param_name.find_first_not_of(' ');
    
    if (first_nonblank_pos == std::string::npos)
      param_name.clear();
    else
      param_name.substr(first_nonblank_pos);
    
    // Truncate to the given number of characters

    param_name = param_name.substr(0, max_chars);
    
    // Remove trailing blanks.  If no non-blank characters are found then the
    // string is blank.
    
    std::size_t last_nonblank_pos =
      param_name.find_last_not_of(' ');
    
    if (last_nonblank_pos == std::string::npos)
      param_name.clear();
    else
      param_name.erase(last_nonblank_pos + 1);

    return param_name;
  }

#endif
  
  /**
   * @brief Get the scale value for the indicated parameter.
   *
   * @param[in] param_num   The parameter index.
   *
   * @return Returns the scale value.
   */

#ifdef USE_RADX
#else

  inline double getParamScale(const std::size_t param_num) const
  {
    return _info->dds->parm[param_num]->parameter_scale;
  }

#endif
  
  /**
   * @brief Get the project name.
   *
   * @return Returns the project name.
   */

#ifdef USE_RADX
#else

  inline std::string getProjectName() const
  {
    return _info->dds->vold->proj_name;
  }

#endif
  
  /**
   * @brief Get the platform altitude in AGL.
   *
   * @return Returns the platform altitude.
   */

#ifdef USE_RADX

  inline double getPlatformAltitudeAGL() const
  {
    return _radxVol.getAltitudeKm();
  }

#else

  inline double getPlatformAltitudeAGL() const
  {
    return _info->dds->asib->altitude_agl;
  }

#endif
  
  /**
   * @brief Get the platform altitude in MSL.
   *
   * @return Returns the platform altitude.
   */

#ifdef USE_RADX

  inline double getPlatformAltitudeMSL() const
  {
    return _radxVol.getAltitudeKm();
  }

#else

  double getPlatformAltitudeMSL() const;

#endif
  
  /**
   * @brief Get the platform drift.
   *
   * @return Returns the platform drift.
   */

#ifdef USE_RADX
#else

  double getPlatformDrift() const;

#endif
  
  /**
   * @brief Get the platform heading.
   *
   * @return Returns the platform heading.
   */

#ifdef USE_RADX

  inline double getPlatformHeading() const
  {
    const std::vector< RadxRay* > rays = _radxVol.getRays();
    
    return rays[0]->getGeoreference()->getHeading();
  }
  

#else

  double getPlatformHeading() const;

#endif
  
  /**
   * @brief Get the platform latitude.
   *
   * @return Returns the platform latitude.
   */

#ifdef USE_RADX

  inline double getPlatformLatitude() const
  {
    _radxVol.getLatitudeDeg();
  }

#else

  double getPlatformLatitude() const;

#endif
  
  /**
   * @brief Get the platform longitude.
   *
   * @return Returns the platform longitude.
   */

#ifdef USE_RADX

  inline double getPlatformLongitude() const
  {
    _radxVol.getLongitudeDeg();
  }

#else

  double getPlatformLongitude() const;

#endif
  
  // NOTE: Move the RAT stuff into a contained object.
  
  /**
   * @brief Get the indicated rotation angle from the angle table.
   *
   * @param[in] angle_index    The angle index.
   *
   * @return Returns the rotation angle.
   */

#ifdef USE_RADX
#else

  double getRATangle(const size_t angle_index) const;

#endif
  
  // NOTE:  What is the difference between radar_name, dds->swib->radar_name
  //        and dds->radd->radar_name???

  /**
   * @brief Get the radar name.
   *
   * @return Returns the radar name.
   */

#ifdef USE_RADX

  inline std::string getRadarName() const
  {
    // NOTE: Don't know which one to use

//    return _radxVol.getInstrumentName();
    return _radxVol.getSiteName();
  }

#else

  inline std::string getRadarName() const
  {
    return _info->dds->swib->radar_name;
  }

  inline std::string getRadarName2() const
  {
    return _info->radar_name;
  }

  inline std::string getRadarName3() const
  {
    return _info->dds->radd->radar_name;
  }

  /**
   * @brief Get a cleaned up version of the radar name.
   *
   * @param[in] max_chars   The maximum string length to return.
   *
   * @return Returns the radar name.
   */

  inline std::string getRadarNameClean(const std::size_t max_chars) const
  {
    std::string radar_name = getRadarName();

    // Remove leading blanks.  If no non-blank characters are found then the
    // string is blank.

    std::size_t first_nonblank_pos =
      radar_name.find_first_not_of(' ');
    
    if (first_nonblank_pos == std::string::npos)
      radar_name.clear();
    else
      radar_name.substr(first_nonblank_pos);
    
    // Truncate to the given number of characters

    radar_name = radar_name.substr(0, max_chars);
    
    // Remove trailing blanks.  If no non-blank characters are found then the
    // string is blank.
    
    std::size_t last_nonblank_pos =
      radar_name.find_last_not_of(' ');
    
    if (last_nonblank_pos == std::string::npos)
      radar_name.clear();
    else
      radar_name.erase(last_nonblank_pos + 1);

    return radar_name;
  }

  inline std::string getRadarName3Clean(const std::size_t max_chars) const
  {
    std::string radar_name = getRadarName3();

    // Remove leading blanks.  If no non-blank characters are found then the
    // string is blank.

    std::size_t first_nonblank_pos =
      radar_name.find_first_not_of(' ');
    
    if (first_nonblank_pos == std::string::npos)
      radar_name.clear();
    else
      radar_name.substr(first_nonblank_pos);
    
    // Truncate to the given number of characters

    radar_name = radar_name.substr(0, max_chars);
    
    // Remove trailing blanks.  If no non-blank characters are found then the
    // string is blank.
    
    std::size_t last_nonblank_pos =
      radar_name.find_last_not_of(' ');
    
    if (last_nonblank_pos == std::string::npos)
      radar_name.clear();
    else
      radar_name.erase(last_nonblank_pos + 1);

    return radar_name;
  }

  std::string getRadarNameClean() const;
  
#endif
  
  /**
   * @brief Get the platform pitch
   *
   * @return Returns the platform pitch.
   */

#ifdef USE_RADX
#else

  double getPlatformPitch() const;

#endif
  
  /**
   * @brief Get the platform roll.
   *
   * @return Returns the platform roll.
   */

#ifdef USE_RADX
#else

  double getPlatformRoll() const;

#endif
  
  // NOTE: The radar type values are defined in dd_defines.hh.  Need to update
  // things to use a type either defined here or somewhere else in the soloii
  // system instead of in the dd stuff, but need to see where all this is used
  // to make sure we fix it everywhere.

  /**
   * @brief Get the type of the radar.
   *
   * @return Returns the radar type.
   */

#ifdef USE_RADX

  inline int getRadarType() const
  {
    return _radxVol.getPlatformType();
  }

#else

  inline int getRadarType() const
  {
    return _info->dds->radd->radar_type;
  }

#endif
  
  /**
   * @brief Get the rotation angle.
   *
   * @return Returns the rotation angle.
   */

#ifdef USE_RADX
#else

  inline double getRotationAngle() const
  {
    return _info->dds->asib->rotation_angle;
  }

  /**
   * @brief Get the rotation angle, taking into account the scan mode.
   *
   * @return Returns the rotation angle.
   */

  double getRotationAngleCalc() const;
  
#endif
  
  /**
   * @brief Get the scan mode.
   *
   * @return Returns the scan mode.
   */

#ifdef USE_RADX

  inline int getScanMode() const
  {
    const std::vector< RadxSweep* > sweeps = _radxVol.getSweeps();
    const RadxSweep *sweep = sweeps[0];
    
    return sweep->getSweepMode();
  }

#else

  inline int getScanMode() const
  {
    return _info->dds->radd->scan_mode;
  }

#endif
  
  /**
   * @brief Get the source ray number.  These numbers start at 1.
   *
   * @return Returns the source ray number.
   */

#ifdef USE_RADX
#else

  inline int getSourceRayNum() const
  {
    return _info->source_ray_num;
  }

#endif
  
  /**
   * @brief Get the start range (distance to the first gate) in km.
   *
   * @return Returns the start range in km.
   */

#ifdef USE_RADX
#else

  inline double getStartRange() const
  {
    return _info->dds->celv->dist_cells[0];
  }

#endif
  
  /**
   * @brief Get the start range (distance to the first gate) in km.
   *
   * @return Returns the start range in km.
   */

#ifdef USE_RADX
#else

  inline double getStartRangeC() const
  {
    return _info->dds->celvc->dist_cells[0];
  }

#endif
  
  /**
   * @brief Get the sweep file name.
   *
   * @return Returns the sweep file name.
   */

#ifdef USE_RADX
#else

  std::string getSweepfileName() const
  {
    return _info->sweep_file_name;
  }

#endif

  /**
   * @brief Get the tilt angle.
   *
   * @return Returns the tilt angle.
   */

#ifdef USE_RADX
#else

  double getTiltAngle() const;

#endif
  
  // NOTE: Need to find out what time this is.

  /**
   * @brief Get the data time.
   *
   * @return Returns the data time.
   */

#ifdef USE_RADX

  inline double getTime() const
  {
    // NOTE:  I'm guessing this method should return the start time, but
    // don't know for sure.

    DateTime start_time(_radxVol.getStartTimeSecs(),
			(int)(_radxVol.getStartNanoSecs() / 10.0e6));
    return start_time.getTimeStamp();
  }

#else

  inline double getTime() const
  {
    return _info->time;
  }

#endif
  
  /**
   * @brief Get the pointing angle to use for time series plots.
   *
   * @return Returns the pointing angle.
   */

#ifdef USE_RADX
#else

  inline double getTimeSeriesPointingAngle()
  {
    double d_rot;

    switch (_info->dds->radd->scan_mode)
    {
    case DD_SCAN_MODE_AIR:
      d_rot = getTiltAngle();
      break;
    case DD_SCAN_MODE_RHI:
      d_rot = getAzimuthAngle();
      break;
    case DD_SCAN_MODE_TAR:
    case DD_SCAN_MODE_VER:
      d_rot = getRotationAngle();
      break;
    default:
      d_rot = getTiltAngle();
      break;
    }
    return d_rot;
  }

#endif

  /**
   * @brief Get the vertical beam width.
   *
   * @return Returns the vertical beam width.
   */

  inline double getVertBeamWidth() const
  {
#ifdef USE_RADX
    return _radxVol.getRadarBeamWidthDegV();
#else
    return _info->dds->radd->vert_beam_width;
#endif
  }
  
  /**
   * @brief Get the vertical wind.
   *
   * @return Returns the vertical wind.
   */

#ifdef USE_RADX
#else

  inline double getVertWind() const
  {
    return _info->dds->asib->vert_wind;
  }

#endif
  
  /**
   * @brief Get the current volume number.
   *
   * @return Returns the current volume number.
   */

#ifdef USE_RADX
#else

  inline int getVolumeNum() const
  {
    return _info->source_vol_num;
  }

#endif
  
  /**
   * @brief Check to see if we are ignoring this sweep.
   *
   * @return Returns true if we are ignoring this sweep, false otherwise.
   */

#ifdef USE_RADX
#else

  inline bool isIgnoreSweep() const
  {
    return _info->ignore_this_sweep;
  }

#endif
  
  /**
   * @brief See if the current rotation angle is in the given sector.
   *
   * @param[in] angle          The angle to check.
   * @param[in] begin_angle    The beginning angle of the sector.
   * @param[in] end_angle      The ending angle of the sector.
   *
   * @return Returns true if the angle is in the sector, false otherwise.
   */

#ifdef USE_RADX
#else

  inline bool isInSector(const double angle,
			 const double begin_angle, const double end_angle) const
  {
    if (begin_angle > end_angle)
      return angle >= begin_angle || angle < end_angle;

    return angle >= begin_angle && angle < end_angle;
  }
  
  inline bool isInSector(const double begin_angle, const double end_angle) const
  {
    return isInSector(getRotationAngleCalc(), begin_angle, end_angle);
  }

#endif
  
  /**
   * @brief Check to see if we are starting a new sweep.
   *
   * @return Returns true if we are starting a new sweep, false otherwise.
   */

#ifdef USE_RADX
#else

  inline bool isNewSweep() const
  {
    return _info->new_sweep;
  }

#endif
  
  /**
   * @brief Check to see if we are starting a new volume.
   *
   * @return Returns true if we are starting a new volume, false otherwise.
   */

#ifdef USE_RADX
#else

  inline bool isNewVolume() const
  {
    return _info->new_vol;
  }

#endif
  
  /**
   * @brief Check if the X/Y plane of data is horizontal.
   *
   * @return Returns true if the plane is horizontal, false otherwise.
   */

#ifdef USE_RADX
#else

  inline bool isPlaneHorizontal() const
  {
    if (_info->dds->radd->radar_type == DD_SCAN_MODE_AIR)
      return false;
    
    return true;
  }

#endif
  
  /**
   * @brief Put the indicated field values into the given field values object.
   *
   * @param[in] param_num     The parameter index.
   * @param[in] angle         The desired angle.
   * @param[in] range         The desired range.
   * @param[out] sfv          The field values.
   * @param[in] state         The state.
   */

#ifdef USE_RADX
#else

  void putFieldValues(const int param_num,
		      const double angle, const double range,
		      struct solo_field_vals &sfv,
		      const int state);

#endif
  
  /**
   * @brief Set the azimuth angle.
   *
   * @param[in] angle   The azimuth angle.
   */

#ifdef USE_RADX
#else

  inline void setAzimuthAngle(const double angle)
  {
    _info->dds->ryib->azimuth = angle;
  }

#endif
  
  // NOTE:  The compression schemes are defined in dd_defines.h.

  /**
   * @brief Set the compression scheme.
   *
   * @param[in] scheme   The compression scheme.
   */

#ifdef USE_RADX
#else

  inline void setCompressionScheme(const int scheme)
  {
    _info->compression_scheme = scheme;
  }

#endif
  
  /**
   * @brief Set the directory for this data.
   *
   * @param[in] dir   The directory for this data.
   */

#ifdef USE_RADX

  inline void setDir(const std::string &dir)
  {
    _dataDir = dir;
  }

#else

  void setDir(const std::string &dir);

#endif
  
  /**
   * @brief Set the disk output flag to the given value.
   *
   * @param[in] disk_output    Disk output flag.
   */

  inline void setDiskOutput(const bool disk_output)
  {
#ifdef USE_RADX
    _diskOutput = disk_output;
#else
    _info->disk_output = disk_output;
#endif
  }
  
  /**
   * @brief Set the elevation angle.
   *
   * @param[in] angle   The elevation angle.
   */

#ifdef USE_RADX
#else

  inline void setElevationAngle(const double angle)
  {
    _info->dds->ryib->elevation = angle;
  }

#endif
  
  /**
   * @brief Set the file qualifier value.
   */

#ifdef USE_RADX

  inline void setFileQualifier(const std::string &qualifier)
  {
    _fileQualifier = qualifier;
  }

#else

  inline void setFileQualifier(const std::string &qualifier)
  {
    _info->file_qualifier = qualifier;
  }

#endif
  
  /**
   * @brief Set the fixed angle.
   *
   * @param[in] angle  The fixed angle.
   */

#ifdef USE_RADX

  inline void setFixedAngle(const double angle)
  {
    RadxSweep *sweep = getRadxSweep();
    
    if (sweep != 0)
      sweep->setFixedAngleDeg(angle);
  }

#else

  inline void setFixedAngle(const double angle)
  {
    _info->dds->swib->fixed_angle = angle;
  }

#endif
  
  /**
   * @brief Set the gate spacing.
   *
   * @param[in] gate_spacing    The gate spacing.
   */

  void setGateSpacing(const double gate_spacing_m);
  
  /**
   * @brief Set the value of the indicated interpulse period.
   *
   * @param[in] period_num    The period index.  Must be a value between 0 and
   *                            4.  If outside this value, the first interpulse
   *                            period is updated.
   * @param[in] period        The value of the interpulse period.
   */

#ifdef USE_RADX
#else

  inline void setInterPulsePeriod(const std::size_t period_num,
				  const double period) const
  {
    if (period_num == 0)
      _info->dds->radd->interpulse_per1 = period;
    else if (period_num == 1)
      _info->dds->radd->interpulse_per2 = period;
    else if (period_num == 2)
      _info->dds->radd->interpulse_per3 = period;
    else if (period_num == 3)
      _info->dds->radd->interpulse_per4 = period;
    else if (period_num == 4)
      _info->dds->radd->interpulse_per5 = period;
    else
      _info->dds->radd->interpulse_per1 = period;
  }

#endif
  
  /**
   * @brief Set the new sweep flag to the indicated value.
   *
   * @param[in] new_sweep   New sweep flag.
   */

#ifdef USE_RADX
#else

  inline void setNewSweep(const bool new_sweep)
  {
    if (new_sweep)
      _info->new_sweep = YES;
    else
      _info->new_sweep = NO;
  }

#endif
  
  /**
   * @brief Set the new volume flag to the indicated value.
   *
   * @param[in] new_volume   New volume flag.
   */

#ifdef USE_RADX
#else

  inline void setNewVolume(const bool new_volume)
  {
    if (new_volume)
      _info->new_vol = YES;
    else
      _info->new_vol = NO;
  }

#endif
  
  /**
   * @brief Set the nyquist velocity.
   *
   * @param[in] vel    The nyquist velocity.
   */

#ifdef USE_RADX
#else

  inline void setNyquistVelocity(const double vel)
  {
    _info->dds->radd->eff_unamb_vel = vel;
  }

#endif
  
  /**
   * @brief Set the bias value for the indicated parameter.
   *
   * @param[in] param_num    The parameter index.
   * @param[in] bias         The bias value.
   */

#ifdef USE_RADX
#else

  inline void setParamBias(const std::size_t param_num, const double bias)
  {
    _info->dds->parm[param_num]->parameter_bias = bias;
  }

#endif
  
  /**
   * @brief Set the scale value for the indicated parameter.
   *
   * @param[in] param_num    The parameter index.
   * @param[in] scale        The scale value.
   */

#ifdef USE_RADX
#else

  inline void setParamScale(const std::size_t param_num, const double scale)
  {
    _info->dds->parm[param_num]->parameter_scale = scale;
  }

#endif
  
  /**
   * @brief Set the threshold information fo rthe indicated parameter.
   *
   * @param[in] param_num            The parameter index.
   * @param[in] thresh_field_name    The threshold field name.
   * @param[in] thresh_value         The threshold value.
   */

#ifdef USE_RADX
  inline void setParamThreshold(const std::string &field_name,
				const std::string &thresh_field_name,
				const double thresh_value)
    {
      thresh_t thresh_info;
      thresh_info.field_name = thresh_field_name;
      thresh_info.value = thresh_value;
      
      _threshMap[field_name] = thresh_info;
    }
#endif

#ifdef USE_RADX
#else

  inline void setParamThreshold(const std::size_t param_num,
				const std::string &thresh_field_name,
				const double thresh_value)
  {
    strncpy(_info->dds->parm[param_num]->threshold_field,
	    thresh_field_name.c_str(), thresh_field_name.size());
    _info->dds->parm[param_num]->threshold_value = thresh_value;
  }

#endif
  
  /**
   * @brief Set the platform altitude in AGL.
   *
   * @param[in] alt   The platform altitude.
   */

#ifdef USE_RADX
#else

  inline void setPlatformAltitudeAGL(const double alt)
  {
    _info->dds->asib->altitude_agl = alt;
  }

#endif
  
  /**
   * @brief Set the platform altitude in MSL.
   *
   * @param[in] alt   The platform altitude.
   */

#ifdef USE_RADX
#else

  inline void setPlatformAltitudeMSL(const double alt)
  {
    _info->dds->asib->altitude_msl = alt;
  }

#endif
  
  /**
   * @brief Set the platform latitude.
   *
   * @param[in] lat   The platform latitude.
   */

#ifdef USE_RADX
#else

  inline void setPlatformLatitude(const double lat)
  {
    _info->dds->asib->latitude = lat;
  }

#endif
  
  /**
   * @brief Set the platform longitude.
   *
   * @param[in] lat   The platform longitude.
   */

#ifdef USE_RADX
#else

  inline void setPlatformLongitude(const double lon)
  {
    _info->dds->asib->longitude = lon;
  }

#endif
  
  /**
   * @brief Set the radar altitude.
   *
   * @param[in] lat   The radar altitude.
   */

  inline void setRadarAltitude(const double alt)
  {
#ifdef USE_RADX
    _radxVol.setAltitudeKm(alt);
#else
    _info->dds->radd->radar_altitude = alt;
#endif
  }
  
  /**
   * @brief Set the radar latitude.
   *
   * @param[in] lat   The radar latitude.
   */

  inline void setRadarLatitude(const double lat)
  {
#ifdef USE_RADX
    _radxVol.setLatitudeDeg(lat);
#else
    _info->dds->radd->radar_latitude = lat;
#endif
  }

  /**
   * @brief Set the radar longitude.
   *
   * @param[in] lat   The radar longitude.
   */

  inline void setRadarLongitude(const double lon)
  {
#ifdef USE_RADX
    _radxVol.setLongitudeDeg(lon);
#else
    _info->dds->radd->radar_longitude = lon;
#endif
  }
  
  /**
   * @brief Set the rotation angle.
   *
   * @param[in] angle   The rotation angle.
   */

#ifdef USE_RADX
#else

  inline void setRotationAngle(const double angle)
  {
    _info->dds->asib->rotation_angle = angle;
  }

#endif
  
  /**
   * @brief Set the scan mode.
   *
   * @param[in] scan_mode   The scan mode.
   */

#ifdef USE_RADX
#else

  void setScanMode(const std::string &scan_mode);

#endif
  
  /**
   * @brief Set the sweep mode.
   *
   * @param[in] sweep_mode   The sweep mode.
   */

#ifdef USE_RADX

  void setSweepMode(const Radx::SweepMode_t sweep_mode)
  {
    RadxSweep *sweep = getRadxSweep();
  
    if (sweep != 0)
      sweep->setSweepMode(sweep_mode);
  }
#endif

  /**
   * @brief Set the start range value.
   *
   * @param[in] start_range   Start range value.
   */

  void setStartRange(const double start_range_m);

  /**
   * @brief Set the original sweep file name.
   *
   * @param[in] file_name    The original sweep file name.
   */

#ifdef USE_RADX
#else

  inline void setOriginalSweepFileName(const std::string &file_name)
  {
    _info->orig_sweep_file_name = file_name;

    for (_currentFile = _fileList.begin(); _currentFile != _fileList.end();
	 ++_currentFile)
    {
      if (_currentFile->getFileName() == file_name)
	break;
    }
  }

#endif
  
  /**
   * @brief Set the sweep queue segment number.
   *
   * @param[in] segment_num   The segment number.
   */

#ifdef USE_RADX
#else

  inline void setSweepQueSegmentNum(const int segment_num)
  {
    _info->swp_que->segment_num = segment_num;
  }

#endif
  
  /**
   * @brief Set the tilt angle.
   *
   * @param[in] angle   The tilt angle.
   */

#ifdef USE_RADX
#else

  inline void setTiltAngle(const double angle)
  {
    _info->dds->asib->tilt = angle;
  }

#endif
  
  /**
   * @brief Set the version number.
   */

#ifdef USE_RADX
#else

  inline void setVersion(const int32_t version)
  {
    _info->version = version;
  }

#endif
  

  //////////////////
  // Data methods //
  //////////////////

  /**
   * @brief Scale the given data value using the scale and bias for the
   *        indicated field.
   *
   * @param[in] value          The unscaled data value.
   * @param[in] field_index    The index for the desired field.
   *
   * @return Returns the scaled data value.
   */

#ifdef USE_RADX
#else

  int scaleValue(const double &value, const std::size_t field_index) const;

#endif
  

  //////////////////////////
  // Input/output methods //
  //////////////////////////

  /**
   * @brief Get the input file identifier.
   *
   * @return Returns the input file identifier.
   */

#ifdef USE_RADX
#else

  inline int32_t getInputFileId() const
  {
    return _info->in_swp_fid;
  }

#endif
  
  /**
   * @brief Set the input file identifier.
   *
   * @param[in] file_id    The new file identifier.
   */

#ifdef USE_RADX
#else

  inline void setInputFileId(const int32_t file_id)
  {
    _info->in_swp_fid = file_id;
  }

#endif
  
  /**
   * @brief Get the output file identifier.
   *
   * @return Returns the output file identifier.
   */

#ifdef USE_RADX
#else

  inline int getOutputFileId() const
  {
    return _info->sweep_fid;
  }

#endif
  
  /**
   * @brief Save the current sweep file information to disk.
   *
   * @param[in] editing   Flag indicating whether we are editing the file.
   */

#ifdef USE_RADX
#else

  void updateSweepFile(const bool editing);

#endif
  
  /**
   * @brief Close the current sweep file without saving.
   */

#ifdef USE_RADX
#else

  void zapSweepFile();

#endif
  
  /**
   * @brief Catalog the sweep file.
   */

#ifdef USE_RADX
#else

  void catalogSweepFile();

#endif
  
  // NOTE: Replaces dd_tape() calls.

  /**
   * @brief Control the output of DORADE tape format data by assembling a list
   *        of sweeps that constitute a volume and then cycling through them
   *        when the volume is complete.
   *
   * @param[in] flush_all
   */

#ifdef USE_RADX
#else

  void tape(const bool flush_all);

#endif
  
  /**
   * @brief Get the pointer to the DD buffer (output buffer???).
   *
   * @return Returns a pointer to the DD buffer.
   */

#ifdef USE_RADX
#else

  inline char *getDDbufPtr()
  {
    return _info->dd_buf;
  }

#endif
  
  /**
   * @brief Get the size of the DD buffer (output buffer???).
   *
   * @return Returns the size of the buffer.
   */

#ifdef USE_RADX
#else

  inline std::size_t getDDbufSize() const
  {
    return _info->sizeof_dd_buf;
  }

#endif
  
  /**
   * @brief Clear the DD buffer (output buffer???).
   */

#ifdef USE_RADX
#else

  inline void clearDDbuf()
  {
    _info->sizeof_dd_buf = 0;
  }

#endif
  
  /**
   * @brief Get the file size.
   *
   * @return Returns the file size.
   */

#ifdef USE_RADX
#else

  inline int32_t getFileSize() const
  {
    return _info->dds->sswb->sizeof_file;
  }

#endif
  
  /**
   * @brief Rewind the buffer.
   */

#ifdef USE_RADX
#else

  void rewindBuffer();

#endif

  /**
   * @brief Load the headers into the manager.
   *
   * @return Returns true on success, false on failure.
   */

#ifdef USE_RADX
#else

  bool loadHeaders();

#endif
  
  /**
   * @brief Load the next ray into the manager.
   *
   * @return Returns true if there is another ray, false otherwise.
   */

#ifdef USE_RADX
#else

  bool loadNextRay();

#endif
  
  /**
   * @brief Load the indicated ray into the manager.
   *
   * @param[in] ray_num  The index of the ray to load.
   */

#ifdef USE_RADX
#else

  void loadRay(const int ray_num);

  /**
   * @brief Load the indicated ray into the manager.
   *
   * @param[in] ray_angle   The angle of the ray to load.
   *
   * @return Returns true on success, false on failure.
   */

  inline bool loadRay(const double angle)
  {
    int ray_index = getAngleIndex(angle);
    loadRay(ray_index);
    return true;
  }
  
#endif
  
  /**
   * @brief Stuff the current ray information into the window for rendering.
   *        (I could be misunderstanding what this does...)
   */

#ifdef USE_RADX
#else

  void stuffRay();

#endif
  
  /**
   * @brief ???
   *
   * @param[in] cfac_ptr   The cfac pointer.
   */

#ifdef USE_RADX
#else

  void setFirstCfac(struct cfac_wrap *cfac_ptr);

#endif
  
  /**
   * @brief Start a new volume.
   */

#ifdef USE_RADX
#else

  void startNewVolume();

#endif
  
  /**
   * @brief Write the current ray to the output file.
   */

#ifdef USE_RADX
#else

  void writeRay();

#endif
  
  /**
   * @brief List all of the descriptors in the manager.  This is used to fill
   *        in the "metadata" information in the "examine" window.
   *
   * @param[out] list    The list to contain the descriptors.
   */

#ifdef USE_RADX
#else

  void listDescriptors(std::vector< std::string > &list);

#endif
  
  /**
   * @brief Print the status line into the given line.
   *
   * @param[in] verbose   Verbose flag.
   * @param[in] preamble  Text printed at beginning of line.
   * @param[in] postamble Text printed at end of line.
   * @param[out] line     The status line.
   */

#ifdef USE_RADX
#else

  void printStatusLine(const bool verbose,
		       const char *preamble,
		       const char *postamble, char *line) const;

#endif
  
  /**
   * @brief Convert the given scan mode to its mnemonic.
   *
   * @param[in] scan_mode   The scan mode index.
   *
   * @return Returns the mnemonic associated with the given scan mode.
   */

  static std::string scanMode2Str(const int scan_mode)
  {
    static std::vector< std::string > mnemonics;

    if (mnemonics.size() == 0)
    {
      mnemonics.push_back("CAL");
      mnemonics.push_back("PPI");
      mnemonics.push_back("COP");
      mnemonics.push_back("RHI");
      mnemonics.push_back("VER");
      mnemonics.push_back("TAR");
      mnemonics.push_back("MAN");
      mnemonics.push_back("IDL");
      mnemonics.push_back("SUR");
      mnemonics.push_back("AIR");
      mnemonics.push_back("???");
    }

    int mnemonic_index = scan_mode;
    
    if (mnemonic_index < 0 || mnemonic_index >= mnemonics.size())
      mnemonic_index = mnemonics.size() - 1;
    
    return mnemonics[mnemonic_index];
  }
  
protected:

  /////////////////////
  // Protected types //
  /////////////////////

  typedef struct
  {
    std::string field_name;
    double value;
  } thresh_t;
  
    
  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief The list of files in the current directory.
   */

  std::vector< FileInfo > _fileList;
  
  /**
   * @brief The current data file.
   */

  std::vector< FileInfo >::iterator _currentFile;
  
  /**
   * @brief The list of files in the current time series plot.
   */

  std::vector< FileInfo > _timeSeriesFileList;
  

#ifdef USE_RADX

  /**
   * @brief The current data in Radx format.
   */

  RadxVol _radxVol;

  /**
   * @brief The full path of the current Radx file.
   */

  std::string _radxPath;

  /**
   * @brief The data directory.
   */

  std::string _dataDir;
  
  /**
   * @brief File qualifier.
   */

  std::string _fileQualifier;
  
  /**
   * @brief The edit command buffer.
   */

  char *_seds;
  
  /**
   * @brief The size of the edit command buffer.
   */

  std::size_t _sizeofSeds;
  
#else
  
  /**
   * @brief Pointer to the actual data information.
   */

  dd_general_info *_info;

#endif
  
  /**
   * @brief Flag indicating whether we need to write something to disk.
   */

  bool _diskOutput;
  
  /**
   * @brief The current directory name.
   */

  std::string _directoryName;
  
  /**
   * @brief The threshold field mapping.
   */

  std::map< std::string, thresh_t > _threshMap;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  static double _angleDifference(const double theta1_deg,
				 const double theta2_deg)
  {
    double diff;
    
    if (theta1_deg >= theta2_deg)
      diff = theta1_deg - theta2_deg;
    else
      diff = theta2_deg - theta1_deg;

    if (diff <= 180.0)
      return 180.0;
      
    return 360.0 - diff;
  }
  
      
};

#endif
