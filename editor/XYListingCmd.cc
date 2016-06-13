#ifdef USE_RADX
#else
#include <se_histog.hh>
#endif

#include <dd_time.h>
#include <se_utils.hh>

#include "XYListingCmd.hh"


/*********************************************************************
 * Constructors
 */

XYListingCmd::XYListingCmd() :
  ForEachRayCmd("xy-listing", "xy-listing of <field> and <field>")
{
}


/*********************************************************************
 * Destructor
 */

XYListingCmd::~XYListingCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool XYListingCmd::doIt(const int frame_num, RadxRay &ray) const
{
  // Pull the arguments out of the command

  std::string src_name = _cmdTokens[1].getCommandText();
  std::string dst_name = _cmdTokens[2].getCommandText();

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();
  
  // Get the pointer to the general field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(frame_num);

  // NOTE:  Can we make this a one-time-only command instead?

  if (seds->process_ray_count == 1)
  {
    // Open file and write headers

    DD_TIME dts;
      
    seds->histo_start_time = data_info->getTime();
    dts.time_stamp = seds->histo_start_time;

    seds->histo_radar_name = data_info->getRadarName();
      
    if (seds->histo_directory[0] == '\0')
      seds->histo_directory = seds->sfic->directory_text;

    std::string file_path;
      
    std::string file_name =
      DataManager::getInstance()->getFileBaseName("xyp",
						  static_cast<int32_t>(seds->histo_start_time),
						  seds->histo_radar_name,
						  0);

    file_path = seds->histo_directory;
    if (file_path[file_path.size()-1] != '/')
      file_path += "/";
    file_path += file_name;

    if (seds->histo_comment != "")
    {
      file_path += ".";
      se_fix_comment(seds->histo_comment);
      file_path += seds->histo_comment;
    }

    // Tack on the variable names

    file_path += "," + src_name + "," + dst_name;
      
    if (!HistogramMgr::getInstance()->openXYStream(file_path))
    {
      printf("Could not open xy-listing file : %s\n", file_path.c_str());
      seds->punt = YES;
      return 0;
    }
  }

  seds->histo_stop_time = data_info->getTime();

  return true;
}

#else

bool XYListingCmd::doIt() const
{
  if (se_xy_stuff(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
