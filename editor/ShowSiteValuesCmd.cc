#ifdef USE_RADX
#else
#include <se_histog.hh>
#endif

#include <HistogramMgr.hh>
#include <se_histog.hh>
#include <se_utils.hh>

#include "ShowSiteValuesCmd.hh"


/*********************************************************************
 * Constructors
 */

ShowSiteValuesCmd::ShowSiteValuesCmd() :
  OneTimeOnlyCmd("show-site-values", "")
{
}


/*********************************************************************
 * Destructor
 */

ShowSiteValuesCmd::~ShowSiteValuesCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool ShowSiteValuesCmd::doIt() const
{
  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  // Process the command based on which command was received

  if (!seds->top_zmap_list)
  {
    printf( "No zmap lists exist!\n" );
    return true;
  }

  if (!seds->curr_zmap_list)
    seds->curr_zmap_list = seds->top_zmap_list;

  int cmd_frame_num;
  std::string field_name = _cmdTokens[1].getCommandText();
  if (_cmdTokens[2].getTokenType() == UiCommandToken::UTT_END)
  {
    cmd_frame_num = 0;
  }
  else
  {
    cmd_frame_num = _cmdTokens[2].getIntParam();
    if (cmd_frame_num < 0)
    {
      cmd_frame_num = 0;
    }
    else if (cmd_frame_num > 0 && cmd_frame_num <= 6)
    {
      cmd_frame_num--;
    }
    else
    {
      cmd_frame_num = 0;
    }
  }
  _listZmapValues(field_name, cmd_frame_num);

  return true;
}

#else

bool ShowSiteValuesCmd::doIt() const
{
  if (se_histog_setup(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

int ShowSiteValuesCmd::_listZmapValues(const std::string &fname,
				       const int frame) const
{
//  int nn, pn, gate, one=1, point_count = 0;
//  int numRatios = 0;
//  char mess[256];
//  float rng, cell_val, val, bad;
//  WW_PTR wwptr;
//  double theta, correlation, ratio;
//  double sumx=0, sumxx=0, sumy=0, sumyy=0, sumxy=0, dnum, dnom;
//  double sumRatios = 0, ratio2;
//  struct zmap_points * zmpc;
//  struct solo_edit_stuff *seds;
//
//
//  seds = return_sed_stuff();
//  seds->h_output.clear();
//    
//  WW_PTR wwptr_temp = solo_return_wwptr( frame );
//	
//  wwptr = wwptr_temp->lead_sweep;
//  DataInfo *data_info =
//    DataManager::getInstance()->getWindowInfo(wwptr->window_num);
//  PointInSpace p1 = wwptr->radar_location;
//  PointInSpace pisp = wwptr->radar_location;
//  zmpc = seds->curr_zmap_list;
//
//  char print_buffer[2048];
//
//  for (; zmpc ; zmpc = zmpc->next )
//  {
//    if( zmpc->src_val < 0 )
//      continue;
//
//    // Loop through the points in this list
//
//    p1.setLatitude(zmpc->lat);
//    p1.setLongitude(zmpc->lon);
//
//    // The (x,y,z) for p1 relative to the origin are going to come back in pisp
//
//    pisp.latLonRelative(p1);
//    theta = zmpc->azm;
//    theta = atan2(pisp.getY(), pisp.getX());
//    theta = FMOD360(CART_ANGLE(DEGREES(theta)));
//    data_info->loadHeaders();
//    data_info->loadRay(theta);
//    pn = data_info->getParamIndex(fname);
//    if (pn < 0)
//    {
//      sprintf(mess, "Field: %s cannot be accesses for %s\n",
//	      fname.c_str(), zmpc->id);
//      printf(mess);
//      continue;
//    }
//
//    rng = KM_TO_M(zmpc->rng);
//    rng = KM_TO_M(sqrt((SQ(pisp.getX())+SQ(pisp.getY()))))/
//      COS(RADIANS(data_info->getElevationAngleCalc()));
//    data_info->getClosestGate(rng, gate, cell_val);
//    nn = data_info->getParamDataUnscaled(pn, gate, one, &val, bad);
//    zmpc->curr_val = val;
//    sumx += zmpc->src_val;
//    sumxx += SQ(zmpc->src_val);
//    sumy += zmpc->curr_val;
//    sumyy += SQ(zmpc->curr_val);
//    sumxy += zmpc->src_val * zmpc->curr_val;
//    ratio = zmpc->curr_val ? zmpc->src_val/zmpc->curr_val : 0;
//    if (ratio != 0.0)
//    {
//      ++numRatios;
//      sumRatios += ratio;
//    }
//
//    if(!point_count++)
//    {
//      // Top line
//
//      DateTime data_time;
//      data_time.setTimeStamp(data_info->getTime());
//      std::string file_name =
//	DataManager::getInstance()->getFileBaseName("rgc",
//						    (int32_t)data_info->getTime(),
//						    data_info->getRadarName(),
//						    0);
//      seds->histo_filename = file_name;
//      sprintf(print_buffer, "\nSite values for %s for %s for",
//	      data_time.toString().c_str(),
//	      data_info->getRadarName().c_str());
//      sprintf(print_buffer + strlen(print_buffer), " %s in frame %d\n",
//	      data_info->getParamName(pn).c_str(),
//	      frame + 1);
//
//      seds->h_output.push_back(print_buffer);
//    }
//
//    sprintf(print_buffer,
//	    "%5s %7.3f %8.3f   rg:%5.1f rd:%5.1f d:%5.1f   da:%5.1f dr:%6.3f\n",
//	    zmpc->id, zmpc->lat, zmpc->lon, zmpc->src_val,
//	    zmpc->curr_val, zmpc->src_val - zmpc->curr_val,
//	    zmpc->azm-theta, zmpc->rng - M_TO_KM(rng));
//    
//    seds->h_output.push_back(print_buffer);
//  }
//
//  nn = point_count;
//  dnum = nn * sumxy - sumx * sumy;
//  dnom = sqrt((double)(( nn*sumxx - SQ(sumx) ) *
//		       ( nn*sumyy - SQ(sumy))));
//  correlation = nn > 0 && dnom > 0 ? dnum/dnom : 0;
//  ratio = sumy ? sumx/sumy : 0;
//  ratio2 = numRatios ? sumRatios/(float)numRatios : 0;
//  sprintf(print_buffer,
//	  "\nCorrelation:%6.3f g/r:%6.3f %6.3f for %s\n",
//	  correlation,
//	  ratio,
//	  ratio2,
//	  fname.c_str());
//  seds->h_output.push_back(print_buffer);
//    
//  HistogramMgr::getInstance()->histoOutput();

  return 0;
}
