/* 	$Id$	 */

#include <glib.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dd_math.h>
#include <DateTime.hh>
#include <DataInfo.hh>
#include <DataManager.hh>
#include <HistogramMgr.hh>
#include <se_bnd.hh>
#include <se_histog.hh>
#include <se_utils.hh>
#include <sii_utils.hh>
#include <solo2.hh>
#include <sp_basics.hh>

extern GString *gs_complaints;

#define SQM2KM 1.e-6

static FILE *histo_stream;
static FILE *x_y_stream;


/* c------------------------------------------------------------------------ */

#ifdef USE_RADX
#else

int list_zmap_values(const std::string &fname, int frame)
{
    int nn, pn, gate, one=1, point_count = 0;
    int numRatios = 0;
    char mess[256];
    float rng, cell_val, val, bad;
    WW_PTR wwptr;
    double theta, correlation, ratio;
    double sumx=0, sumxx=0, sumy=0, sumyy=0, sumxy=0, dnum, dnom;
    double sumRatios = 0, ratio2;
    struct zmap_points * zmpc;
    struct solo_edit_stuff *seds;


    seds = return_sed_stuff();
    seds->h_output.clear();
    
    WW_PTR wwptr_temp = solo_return_wwptr( frame );
	
    wwptr = wwptr_temp->lead_sweep;
    DataInfo *data_info =
      DataManager::getInstance()->getWindowInfo(wwptr->window_num);
    PointInSpace p1 = wwptr->radar_location;
    PointInSpace pisp = wwptr->radar_location;
    zmpc = seds->curr_zmap_list;

    char print_buffer[2048];

    for(; zmpc ; zmpc = zmpc->next ) {
	if( zmpc->src_val < 0 )
	    { continue; }

	/* loop through the points in this list
	 */
	p1.setLatitude(zmpc->lat);
	p1.setLongitude(zmpc->lon);

	/* the (x,y,z) for p1 relative to the origin
	 * are going to come back in pisp
	 */
	pisp.latLonRelative(p1);
	theta = zmpc->azm;
	theta = atan2( pisp.getY(), pisp.getX() );
	theta = FMOD360(CART_ANGLE(DEGREES(theta)));
	data_info->loadHeaders();
	data_info->loadRay(theta);
	pn = data_info->getParamIndex(fname);
	if( pn < 0 ) {
	    sprintf( mess, "Field: %s cannot be accesses for %s\n"
		     , fname.c_str(), zmpc->id );
	    printf( mess );
	    continue;
	}
	rng = KM_TO_M( zmpc->rng );
	rng = KM_TO_M( sqrt((SQ(pisp.getX())+SQ(pisp.getY()))) )/
	  COS( RADIANS(data_info->getElevationAngleCalc()) );
	data_info->getClosestGate(rng, gate, cell_val);
	nn = data_info->getParamDataUnscaled(pn, gate, one, &val, bad);
	zmpc->curr_val = val;
	sumx += zmpc->src_val;
	sumxx += SQ( zmpc->src_val );
	sumy += zmpc->curr_val;
	sumyy += SQ( zmpc->curr_val );
	sumxy += zmpc->src_val * zmpc->curr_val;
	ratio = zmpc->curr_val ? zmpc->src_val/zmpc->curr_val : 0;
	if( ratio ) {
	    ++numRatios;
	    sumRatios += ratio;
	}
	if( !point_count++ ) {	/* top line */
	   DateTime data_time;
	   data_time.setTimeStamp(data_info->getTime());
	   std::string file_name =
	     DataManager::getInstance()->getFileBaseName("rgc",
							 (int32_t)data_info->getTime(),
							 data_info->getRadarName2(),
							 0);
	   seds->histo_filename = file_name;
	   sprintf(print_buffer, "\nSite values for %s for %s for",
		   data_time.toString().c_str(),
		   data_info->getRadarName3Clean(8).c_str());
	   sprintf(print_buffer + strlen(print_buffer), " %s in frame %d\n",
		   data_info->getParamNameClean(pn, 8).c_str(),
		   frame + 1);

	   seds->h_output.push_back(print_buffer);
	}

	sprintf(print_buffer,
		"%5s %7.3f %8.3f   rg:%5.1f rd:%5.1f d:%5.1f   da:%5.1f dr:%6.3f\n",
		zmpc->id, zmpc->lat, zmpc->lon, zmpc->src_val,
		zmpc->curr_val, zmpc->src_val - zmpc->curr_val,
		zmpc->azm-theta, zmpc->rng - M_TO_KM(rng));

	seds->h_output.push_back(print_buffer);
    }
    nn = point_count;
    dnum = nn * sumxy - sumx * sumy;
    dnom = sqrt((double)(( nn*sumxx - SQ(sumx) ) *
			 ( nn*sumyy - SQ(sumy))));
    correlation = nn > 0 && dnom > 0 ? dnum/dnom : 0;
    ratio = sumy ? sumx/sumy : 0;
    ratio2 = numRatios ? sumRatios/(float)numRatios : 0;
    sprintf(print_buffer,
	    "\nCorrelation:%6.3f g/r:%6.3f %6.3f for %s\n",
	    correlation,
	    ratio,
	    ratio2,
	    fname.c_str());
    seds->h_output.push_back(print_buffer);
    
    se_histo_output();

    return 0;
}

#endif

/* c------------------------------------------------------------------------ */

#ifdef USE_RADX
#else

void se_histo_output (void)
{
  struct solo_edit_stuff *seds = return_sed_stuff();
  se_print_strings(seds->h_output);

  if (seds->histo_output_key == SE_HST_COPY)
  {
    if (!histo_stream)
    {
      std::string histo_file_path = seds->histo_directory;
      if (histo_file_path[histo_file_path.size()-1] != '/')
	histo_file_path += "/";
      histo_file_path += seds->histo_filename;

      if (seds->histo_comment.size() > 0)
      {
	histo_file_path += ".";
	se_fix_comment(seds->histo_comment);
	histo_file_path += seds->histo_comment;
      }

      if ((histo_stream = fopen(histo_file_path.c_str(), "w")) == 0)
      {
	char message[1024];
	
	sprintf(message, "Could not open histogram file : %s\n",
		histo_file_path.c_str());
	sii_message(message);
	return;
      }
    }
    else
    {
      fprintf(histo_stream, "\n\n");
    }

    for (size_t i = 0; i < seds->h_output.size(); ++i)
      fprintf(histo_stream, "%s", seds->h_output[i].c_str());
  }

  if (seds->histo_flush && histo_stream)
    fflush(histo_stream);

  return;
}

#endif

/* c------------------------------------------------------------------------ */

void 
se_histo_fin_irreg_areas (void)
{
    int ii, nn;
    float f_val, f_sum=0, rcp_inc, f_min, f_max;
    float f_inc;
    char *c, *e, H='H', str[88], number[16];
    struct se_pairs *pair;
    struct solo_edit_stuff *seds;

    seds = return_sed_stuff();	/* solo editing struct */
    f_inc = seds->histo_increment;

    pair = seds->h_pairs;

    for(; pair; pair=pair->next) {
	f_val = seds->histo_low + .5*f_inc;
	pair->f_sum = 0;
	for(ii=0; ii < seds->histo_num_bins; ii++, f_val+=f_inc) {
	    if(f_val < pair->f_low)
		  continue;
	    if(f_val > pair->f_high)
		  break;
	    pair->f_sum += *(seds->areas_array+ii) * SQM2KM;
	}	
	f_sum += pair->f_sum;
    }
    char print_buffer[1024];
    
    pair = seds->h_pairs;
    f_min = f_max = pair->f_sum;

    for (; pair; pair=pair->next)
    {
      if(pair->f_sum >= f_max)
      {
	f_max = pair->f_sum;
	sprintf(print_buffer,
		" From %7.1f to %7.1f contains the largest area: %8.1f sq.km.\n",
		pair->f_low, pair->f_high, f_max);
      }

      if (pair->f_sum < f_min)
	f_min = pair->f_sum;
    }
    seds->h_output.push_back(print_buffer);
    
    sprintf(print_buffer,
	    "      Low area: %8.1f sq.km.\n", seds->low_area * SQM2KM);
    seds->h_output.push_back(print_buffer);
    
    sprintf(print_buffer,
	    "     High area: %8.1f sq.km.\n", seds->high_area * SQM2KM);
    seds->h_output.push_back(print_buffer);
    
    sprintf(print_buffer,
	    "Histogram area: %8.1f sq.km.\n", f_sum);
    seds->h_output.push_back(print_buffer);
    
    f_inc = (f_max -f_min)/50.;
    rcp_inc = 1./f_inc;;
    sprintf(str, "%7.1f to %7.1f ", f_min, f_max); e = str +strlen(str);

    memset(str, ' ', sizeof(str));
    for(f_val=f_min,c=e-8; f_val <= f_max; f_val += 10*f_inc, c+=10) {
	sprintf(number, "%9.1f", f_val);
	strncpy(c, number, strlen(number));
    }
    *c++ = '\n'; *c++ = '\0';
    seds->h_output.push_back(str);
    
    memset(str, ' ', sizeof(str));
    for(f_val=f_min,c=e; f_val <= f_max; f_val += 10*f_inc, c+=10) {
	*c = '|';
    }
    *c++ = '\n'; *c++ = '\0';
    seds->h_output.push_back(str);
    
    pair = seds->h_pairs;

    for(; pair; pair=pair->next) {
      c = print_buffer;
      nn = (int)((pair->f_sum -f_min)/f_inc);
      sprintf(c, "%7.1f to %7.1f ", pair->f_low, pair->f_high); 
      c += strlen(c);
      
      if( nn <= 0  && f_sum )
	nn = 1;
      for(; nn--; *c++ = H);
      *c++ = '\n'; *c++ = '\0';
      seds->h_output.push_back(print_buffer);
    }
}

/* c------------------------------------------------------------------------ */

void 
se_histo_fin_irreg_counts (void)
{
    int ii, nn=0, min, max;
    float f_val, f_inc, rcp_inc, f_mode = 0.0;
    char *c, *e, H='H', str[88], number[16];
    struct se_pairs *pair;
    struct solo_edit_stuff *seds;

    seds = return_sed_stuff();	/* solo editing struct */

    pair = seds->h_pairs;
    f_inc = seds->histo_increment;

    for(; pair; pair=pair->next) {
	f_val = seds->histo_low + 0.5 * f_inc;
	pair->sum = 0;
	for(ii=0; ii < seds->histo_num_bins; ii++, f_val+=f_inc) {
	    if(f_val < pair->f_low)
		  continue;
	    if(f_val > pair->f_high)
		  break;
	    pair->sum += *(seds->counts_array+ii);
	}	
    }
    pair = seds->h_pairs;

    min = max = pair->sum;
    for(; pair; pair=pair->next) {
	if(pair->sum >= max) {
	    max = pair->sum;
	    f_mode = .5*(pair->f_low +pair->f_high);
	}
	if(pair->sum < min)
	      min = pair->sum;
    }
    char print_buffer[1024];
    sprintf(print_buffer,
	    "  Mode: %8.2f\n", f_mode);
    seds->h_output.push_back(print_buffer);
    
    f_inc = (max -min)/50.;
    if(f_inc < 1./50.) {
	f_inc = 1./50.;
    }
    rcp_inc = 1./f_inc;
    sprintf(str, "%8.2f to %8.2f", f_inc, f_inc); e = str +strlen(str);

    memset(str, ' ', sizeof(str));
    for(ii=min,c=e-6; ii <= max; ii += (int)(10*f_inc+.5), c+=10) {
	sprintf(number, "%7d", ii);
	strncpy(c, number, strlen(number));
    }
    *c++ = '\n'; *c++ = '\0';
    seds->h_output.push_back(str);
    
    memset(str, ' ', sizeof(str));
    for(ii=min,c=e+1; ii <= max; ii += (int)(10*f_inc +.5), c+=10) {
	*c = '|';
    }
    *c++ = '\n'; *c++ = '\0';
    seds->h_output.push_back(str);
    pair = seds->h_pairs;
    
    for(; pair; pair=pair->next) {
      c = print_buffer;
      nn = (int)((pair->sum -min)/f_inc);
      sprintf(c, "%8.2f to %8.2f ", pair->f_low, pair->f_high); 
      c += strlen(c);
      if (nn <= 0 && pair->sum > 0)
	nn = 1;
      for(; nn--; *c++ = H);
      *c++ = '\n'; *c++ = '\0';
      seds->h_output.push_back(print_buffer);
    }
}

/* c------------------------------------------------------------------------ */

void se_histo_fin_areas()
{
    int ii, nn, mark;
    float f, f_val, f_sum, rcp_inc, f_min, f_max, f_mode, f_median = 0.0;
    float f_mean, f_sdev, f_sum2, f_inc;
    char *a, *b, *c, *e, H='H', str[88], number[16];
    struct solo_edit_stuff *seds;

    seds = return_sed_stuff();	/* solo editing struct */

    seds->h_output.clear();

    char print_buffer[1024];
    
    b = (char *) ((H_IRREG & seds->histo_key) ? "irregular intervals of " :
	  "regular intervals of ");
    sprintf(print_buffer, "Areas Histogram in %s%s\n",
	    b, seds->histogram_field.c_str());
    seds->h_output.push_back(print_buffer);
    
    a = print_buffer;
    DateTime start_time;
    start_time.setTimeStamp(seds->histo_start_time);
    sprintf(a, "From %s to ", start_time.toString().c_str());
    a += strlen(a);
    DateTime stop_time;
    stop_time.setTimeStamp(seds->histo_stop_time);
    sprintf(a, "%s for %s at %.1f deg.\n",
	    stop_time.toString().c_str(),
	    seds->histo_radar_name.c_str(),
	    seds->histo_fixed_angle);

    seds->h_output.push_back(print_buffer);
    
    sprintf(print_buffer,
	    "       Missing: %8d gates\n", seds->histo_missing_count);
    seds->h_output.push_back(print_buffer);
    
    sprintf(print_buffer,
	    "   Points below:%8d gates\n", seds->low_count);
    seds->h_output.push_back(print_buffer);
    
    sprintf(print_buffer,
	    "   Points above:%8d gates\n", seds->high_count);
    seds->h_output.push_back(print_buffer);
    
    sprintf(print_buffer,
	    "Points between: %8d gates\n", seds->medium_count);
    seds->h_output.push_back(print_buffer);
    
    nn = seds->medium_count;
    f_mean = seds->histo_sum/nn;
    f_sdev = sqrt((((double)seds->histo_sum_sq -nn*f_mean*f_mean)
		   /((double)(nn-1))));
    sprintf(print_buffer,
	    "  Mean: %7.1f\n", f_mean);
    seds->h_output.push_back(print_buffer);
    
    sprintf(print_buffer,
	    "  Sdev: %7.1f\n", f_sdev);
    seds->h_output.push_back(print_buffer);
    
    if(nn <= 1) {
	mark = 0;
	return;
    }

    f_inc = seds->histo_increment;
    f_mode = f_val = seds->histo_low + 0.5 * f_inc;
    f_min = f_max = *seds->areas_array;
    f_sum = 0;

    for(ii=0; ii < seds->histo_num_bins; ii++, f_val+=f_inc) {
	f = *(seds->areas_array+ii);
	f_sum += f;

	if(f > f_max) {
	    f_max = f;
	    f_mode = f_val;
	}
	if(f < f_min) {
	    f_min = f;
	}
    }

    f_sum2 = 0;
    f_val = seds->histo_low + 0.5 * f_inc;
    for(ii=0; ii < seds->histo_num_bins; ii++, f_val+=f_inc) {
	f = *(seds->areas_array+ii);
	if((f_sum2 += f) >= .5*f_sum) {
	    f_median = f_val;
	    break;
	}
    }
    sprintf(print_buffer,
	    "Median: %7.1f\n", f_median);
    seds->h_output.push_back(print_buffer);
    
    if(H_IRREG & seds->histo_key) {
	se_histo_fin_irreg_areas();
	return;
    }
    sprintf(print_buffer,
	"  Mode: %7.1f\n", f_mode);
    seds->h_output.push_back(print_buffer);
    
    sprintf(print_buffer,
	    "      Low area: %8.1f sq.km.\n", seds->low_area * SQM2KM);
    seds->h_output.push_back(print_buffer);
    
    sprintf(print_buffer,
	    "     High area: %8.1f sq.km.\n", seds->high_area * SQM2KM);
    seds->h_output.push_back(print_buffer);
    
    sprintf(print_buffer,
	    "Histogram area: %8.1f sq.km.      Volume: %.6f\n",
	    f_sum * SQM2KM,
	    HistogramMgr::getInstance()->getAreaXValue() * .001 * 1.e-6 );
    seds->h_output.push_back(print_buffer);
    
    f_inc = (f_max -f_min)/50.;
    rcp_inc = 1./f_inc;;
    sprintf(str, "%7.1f ", f_val); e = str +strlen(str);

    memset(str, ' ', sizeof(str));
    for(f_val=f_min,c=e-8; f_val <= f_max; f_val += 10*f_inc, c+=10) {
	sprintf(number, "%9.1f", f_val*SQM2KM);
	strncpy(c, number, strlen(number));
    }
    *c++ = '\n'; *c++ = '\0';
    seds->h_output.push_back(str);
    
    memset(str, ' ', sizeof(str));
    for(f_val=f_min,c=e; f_val <= f_max; f_val += 10*f_inc, c+=10) {
	*c = '|';
    }
    *c++ = '\n'; *c++ = '\0';
    seds->h_output.push_back(str);
    
    f_val = seds->histo_low + 0.5 * seds->histo_increment;

    for(ii=0; ii < seds->histo_num_bins; ii++, f_val+=seds->histo_increment) {
	nn = (int)((*(seds->areas_array+ii) -f_min)/f_inc);
	sprintf(str, "%7.1f ", f_val); c = str +strlen(str);
	if( nn <= 0 && *(seds->areas_array+ii) > 0 )
	    { nn = 1; }
	for(; nn--; *c++ = H);
	*c++ = '\n'; *c++ = '\0';
	seds->h_output.push_back(str);
    }
    return;
}
/* c------------------------------------------------------------------------ */

void 
se_histo_fin_counts (void)
{
    int ii, kk, mm, nn=0, mark, min, max, med;
    float f_val, f_sum, ff_sum, f_inc, rcp_inc, f_mode, f_mean, f_sdev;
    float f_median = 0.0;
    char *a, *b, *c, *e, H='H', str[88], number[16];
    struct solo_edit_stuff *seds;

    seds = return_sed_stuff();	/* solo editing struct */

    seds->h_output.clear();
  
    char print_buffer[1024];
    
    b = (char *)((H_IRREG & seds->histo_key) ? "irregular intervals of " :
	  "regular intervals of ");
    sprintf(print_buffer, "Counts Histogram in %s%s\n",
	    b, seds->histogram_field.c_str());
    seds->h_output.push_back(print_buffer);
    
    a = print_buffer;
    DateTime start_time;
    start_time.setTimeStamp(seds->histo_start_time);
    sprintf(a, "From %s to ", start_time.toString().c_str());
    a += strlen(a);
    DateTime stop_time;
    stop_time.setTimeStamp(seds->histo_stop_time);
    sprintf(a, "%s for %s at %.1f deg.\n",
	    stop_time.toString().c_str(),
	    seds->histo_radar_name.c_str(),
	    seds->histo_fixed_angle);

    seds->h_output.push_back(print_buffer);
    
    sprintf(print_buffer,
	    "       Missing: %8d gates\n", seds->histo_missing_count);
    seds->h_output.push_back(print_buffer);
    
    sprintf(print_buffer,
	    "   Points below:%8d gates\n", seds->low_count);
    seds->h_output.push_back(print_buffer);
    
    sprintf(print_buffer,
	    "   Points above:%8d gates\n", seds->high_count);
    seds->h_output.push_back(print_buffer);
    
    sprintf(print_buffer,
	    "Points between: %8d gates\n", seds->medium_count);
    seds->h_output.push_back(print_buffer);
    
    nn = seds->medium_count;
    f_mean = seds->histo_sum/nn;
    f_sdev = sqrt((((double)seds->histo_sum_sq -nn*f_mean*f_mean)
			   /((double)(nn-1))));
    sprintf(print_buffer,
	    "  Mean: %8.2f\n", f_mean);
    seds->h_output.push_back(print_buffer);
    
    sprintf(print_buffer,
	    "  Sdev: %8.2f\n", f_sdev);
    seds->h_output.push_back(print_buffer);
    
    if(nn <= 1) {
	mark = 0;
	return;
    }

    f_mode = f_val = seds->histo_low + 0.5 * seds->histo_increment;
    min = max = *seds->counts_array;
    nn = seds->medium_count;
    f_sum = ff_sum = kk = mm = med = 0;


    for(ii=0; ii < seds->histo_num_bins; ii++, f_val+=seds->histo_increment) {
	kk = *(seds->counts_array+ii);
	f_sum += f_val*kk;
	ff_sum += f_val*f_val*kk;

	if(kk > max) {
	    max = kk;
	    f_mode = f_val;
	}
	if(kk < min) min = kk;
	mm += kk;
	if(!med && mm >= nn/2) {
	    med = YES;
	    f_median = f_val;
	}
    }
    sprintf(print_buffer,
	    "Median: %8.2f\n", f_median);
    seds->h_output.push_back(print_buffer);
    
    if(H_IRREG & seds->histo_key) {
	se_histo_fin_irreg_counts();
	return;
    }
    sprintf(print_buffer,
	"  Mode: %8.2f\n", f_mode);
    seds->h_output.push_back(print_buffer);
    
    f_val = seds->histo_low + 0.5 * seds->histo_increment;
    f_inc = (max -min)/50.;
    rcp_inc = 1./f_inc;;
    sprintf(str, "%8.2f ", f_val); e = str +strlen(str);

    memset(str, ' ', sizeof(str));
    for(ii=min,c=e-5; ii <= max; ii += (int)(10*f_inc+.5), c+=10) {
	sprintf(number, "%7d", ii);
	strncpy(c, number, strlen(number));
    }
    *c++ = '\n'; *c++ = '\0';
    seds->h_output.push_back(str);
    
    memset(str, ' ', sizeof(str));
    for(ii=min,c=e; ii <= max; ii += (int)(10*f_inc +.5), c+=10) {
	*c = '|';
    }
    *c++ = '\n'; *c++ = '\0';
    seds->h_output.push_back(str);
    
    for(ii=0; ii < seds->histo_num_bins; ii++, f_val+=seds->histo_increment) {
	nn = (int)((*(seds->counts_array+ii) -min)/f_inc);
	sprintf(str, "%8.2f ", f_val); c = str +strlen(str);
	if( nn <= 0 && *(seds->counts_array+ii) > 0 )
	    { nn = 1; }
	for(; nn--; *c++ = H);
	*c++ = '\n'; *c++ = '\0';
	seds->h_output.push_back(str);
    }
}

/* c------------------------------------------------------------------------ */

void 
se_histo_init (struct solo_edit_stuff *seds)
{
    int nb;
    struct se_pairs *pair;

    seds->low_count =
	  seds->medium_count =
		seds->high_count = 0;
	seds->histo_sum =
	  seds->histo_sum_sq = 0;

    if(!(H_AREAS & seds->histo_key || H_BINS & seds->histo_key)) {
	seds->histo_key |= H_BINS;
    }

    if(H_IRREG & seds->histo_key) { /* find min and max */
	nb = H_BIN_MAX;
	pair = seds->h_pairs;
	seds->histo_low = pair->f_low;
	seds->histo_high = pair->f_high;

	for(pair=pair->next; pair; pair=pair->next) {
	    if(pair->f_low < seds->histo_low)
		  seds->histo_low = pair->f_low;
	    if(pair->f_high > seds->histo_high)
		  seds->histo_high = pair->f_high;
	}
	seds->histo_increment = (seds->histo_high -seds->histo_low)/nb;
    }
    else {
	nb = (int)((seds->histo_high -seds->histo_low)/seds->histo_increment);
    }

    if(H_AREAS & seds->histo_key) {	/* set up for areas */
	if(nb != seds->histo_num_bins) {
	    if(seds->areas_array)
		  free(seds->areas_array);
	    seds->areas_array = (float *)malloc((nb+1)*sizeof(float));
	}
	memset(seds->areas_array, 0, (nb+1)*sizeof(float));
	seds->low_area = seds->high_area = 0;
    }
    else {			/* counts! */
	if(nb != seds->histo_num_bins) {
	    if(seds->counts_array)
		  free(seds->counts_array);
	    seds->counts_array = (int *)malloc((nb+1)*sizeof(int));
	}
	memset(seds->counts_array, 0, (nb+1)*sizeof(int));
    }
    seds->histo_num_bins = nb;
    seds->histo_missing_count = 0;
    return;
}

/* c------------------------------------------------------------------------ */
/* This routine does a histogram for this pass on the specified field using  */
/*  the boundary mask                                                        */

#ifdef USE_RADX
#else

int se_histo_ray(const std::vector< UiCommandToken > &cmd_tokens)
{
  /* #do-histogram# */
# define PIOVR360 .00872664626

  // Boundary mask is set to 1 if the corresponding cell satisfies
  // conditions of the boundaries

  struct solo_edit_stuff *seds = return_sed_stuff();
  std::string name = seds->histogram_field;
  if (seds->finish_up)
  {
    if (H_BINS & seds->histo_key) 
      se_histo_fin_counts();
    else
      se_histo_fin_areas();
    se_histo_output();
    return 1;
  }

  static bool first_time = false;
  
  if (seds->process_ray_count == 1)
  {
    first_time = true;
  }

  if (seds->use_boundary && seds->boundary_exists && !seds->num_segments)
  {
    // Ray does not intersect the boundary

    return 1;
  }

  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(seds->se_frame);
  
  int field_num = data_info->getParamIndex(name);
  
  if (field_num < 0)
  {
    printf("Source parameter %s not found for copy\n", name.c_str());
    seds->punt = YES;
    return -1;
  }

  short *data_ptr = (short *)data_info->getParamData(field_num);
  int bad = static_cast<int>(data_info->getParamBadDataValue(field_num));
  float scale = data_info->getParamScale(field_num);
  float rcp_scale = 1.0 / scale;
  float bias = data_info->getParamBias(field_num);
  
  if (first_time)
  {
    first_time = false;
    seds->histo_start_time = data_info->getTime();
    seds->histo_fixed_angle = data_info->getFixedAngle();
    seds->histo_radar_name = data_info->getRadarNameClean();

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
    se_histo_init(seds);
    HistogramMgr::getInstance()->initAreaXValue();
  }

  int scaled_low = (int)DD_SCALE(seds->histo_low, scale, bias);
  int scaled_high = (int)DD_SCALE(seds->histo_high, scale, bias);
  int scaled_inc = (int)DD_SCALE(seds->histo_increment, scale, bias);
  float rcp_inc = 1.0 / scaled_inc;
  int num_cells = data_info->getClipGate() + 1;
  seds->histo_stop_time = data_info->getTime();

  // Loop through the data

  short *bnd = (short *) seds->boundary_mask;
  
  for (int ndx_ss = 0; ndx_ss < num_cells; ndx_ss++)
  {
    if (!(*(bnd+ndx_ss)))
      continue;
    
    short xx = data_ptr[ndx_ss];
    
    if (xx == bad)
    {
      seds->histo_missing_count++;
      continue;
    }

    if (H_BINS & seds->histo_key)
    {
      if (xx < scaled_low)
      {
	seds->low_count++;
      }
      else if (xx >= scaled_high)
      {
	seds->high_count++;
      }
      else
      {
	seds->medium_count++;
	int kk = (int)((xx-scaled_low)*rcp_inc);
	seds->counts_array[kk]++;
	double d = DD_UNSCALE(xx, rcp_scale, bias);
	seds->histo_sum += d;
	seds->histo_sum_sq += d*d;
      }
    }
    else
    {
      // Areas!

      double r1;
      double r2;
      
      if (ndx_ss != 0)
      {
	r1 = data_info->getGateRange(ndx_ss - 1);
	r2 = data_info->getGateRange(ndx_ss);
      }
      else
      {
	r1 = 0;
	r2 = data_info->getStartRange();
      }

      struct dd_ray_sector *ddrc =
	data_info->rayCoverage(seds->sweep_ray_count);
      float area = fabs(ddrc->sector) * PIOVR360 * (SQ(r2) - SQ(r1));
		
      if (xx < scaled_low)
      {
	seds->low_count++;
	seds->low_area += area;
      }
      else if (xx >= scaled_high)
      {
	seds->high_count++;
	seds->high_area += area;
      }
      else
      {
	seds->medium_count++;
	HistogramMgr::getInstance()->addAreaXValue(area *
						   DD_UNSCALE(xx, rcp_scale, bias));
	int kk = (int)((xx - scaled_low) * rcp_inc);
	seds->areas_array[kk] += area;
	double d = DD_UNSCALE(xx, rcp_scale, bias);
	seds->histo_sum += d;
	seds->histo_sum_sq += d * d;
      }
    }
  }

  return 1;
}

#endif

/* c------------------------------------------------------------------------ */

#ifdef USE_RADX
#else

int se_histog_setup(const std::vector< UiCommandToken > &cmd_tokens)
{
  /* #area-histogram# */
  /* #count-histogram# */
  /* #irregular-histogram-bin# */
  /* #regular-histogram-parameters# */
  /* #histogram-comment# */
  /* #histogram-directory# */
  /* #new-histogram-file# */
  /* #append-histogram-to-file# */
  /* #dont-append-histogram-to-file# */
  /* #map-boundary# */
  /* #site-list# */
  /* #show-site-values# */

  // Get a pointer to the command arguments.  In this case, the arguments
  // will be pulled out below based on the actual command

  int token_num = 1;

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  // Process the command based on which command was received

  if (cmd_tokens[0].getCommandText().compare(0, 9, "area-histogram", 0, 9) == 0 ||
      cmd_tokens[0].getCommandText().compare(0, 9, "count-histogram", 0, 9) == 0)
  {
    seds->histogram_field = cmd_tokens[token_num].getCommandText();
    seds->num_irreg_bins = 0;

    for (struct se_pairs *hp = seds->h_pairs; hp != 0; )
    {
      // Free irreg pairs if any

      struct se_pairs *hpn = hp->next;
      free(hp);
      hp = hpn;
    }	      

    seds->h_pairs = NULL;
    seds->histo_key =
      cmd_tokens[0].getCommandText().compare(0, 9, "area-histogram", 0, 9) == 0 ?
      H_AREAS : H_BINS;
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 11, "irregular-histogram-bin", 0, 11) == 0)
  {
    seds->histo_key |= H_IRREG;
    seds->num_irreg_bins++;

    struct se_pairs *hp = (struct se_pairs *)malloc(sizeof(struct se_pairs));
    memset(hp, 0, sizeof(struct se_pairs));

    if (!seds->h_pairs)
    {
      seds->h_pairs = hp;
    }
    else
    {
      seds->h_pairs->last->next = hp;
    }

    seds->h_pairs->last = hp;
    hp->f_low = cmd_tokens[token_num++].getFloatParam();
    hp->f_high = cmd_tokens[token_num++].getFloatParam();
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 11, "regular-histogram-parameters", 0, 11) == 0)
  {
    seds->histo_key |= H_REG;
    seds->histo_low = cmd_tokens[token_num++].getFloatParam();
    seds->histo_high = cmd_tokens[token_num++].getFloatParam();
    seds->histo_increment = cmd_tokens[token_num++].getFloatParam();
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 15, "histogram-comment", 0, 15) == 0)
  {
    seds->histo_comment =
      se_unquote_string(cmd_tokens[token_num].getCommandText());
    seds->histo_output_key = SE_HST_COPY;
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 15, "histogram-directory", 0, 15) == 0 ||
	   cmd_tokens[0].getCommandText().compare(0, 6, "xy-directory", 0, 6) == 0)
  {
    seds->histo_directory =
      se_unquote_string(cmd_tokens[token_num].getCommandText());
    seds->histo_output_key = SE_HST_COPY;
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 15, "histogram-flush", 0, 15) == 0)
  {
    seds->histo_flush = YES;
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 11, "new-histogram-file", 0, 11) == 0)
  {
    if (histo_stream)
    {
      fclose(histo_stream);
      histo_stream = NULL;
    }
    seds->histo_output_key = SE_HST_COPY;
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 11, "append-histogram-to-file", 0, 11) == 0)
  {
    seds->histo_output_key = SE_HST_COPY;
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 11, "dont-append-histogram-to-file", 0, 11) == 0)
  {
    seds->histo_output_key = SE_HST_NOCOPY;
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 7, "map-boundary", 0, 7) == 0)
  {
    int skip = 1;

    std::string cmd_text =
      se_unquote_string(cmd_tokens[token_num].getCommandText());

    if (cmd_text == "test")
    {
      cmd_text = "/scr/hotlips/oye/spol/cases/bnd.square";
      cmd_text = "/scr/hotlips/oye/src/spol/misc/tfb.txt";
    }
    else if (cmd_text == "cases")
    {
      cmd_text = "/scr/hotlips/oye/spol/cases/walnut10";
      skip = 5;
    }

    int size;
    char *buf = absorb_zmap_bnd(cmd_text, skip, size);
    if (buf != 0)
    {
      se_unpack_bnd_buf(buf, size);
      free(buf);
    }
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 9, "site-list", 0, 9) == 0)
  {
    static char gauge_dir[256];
    static char *gauge_file;

    static int site_list_count = 0;
    
    if (!site_list_count++)
    {
      char *gg;
      
      if ((gg = getenv("GAUGES_DIR")) ||
	  (gg = getenv("RAIN_GAUGES_DIR")))
      {
	if (gg[strlen(gg)-1] == '/')
	  strcpy(gauge_dir, gg);
	else
	  sprintf(gauge_dir, "%s/", gg);
      }
      else
      {
	strcpy(gauge_dir, "/scr/hotlips/oye/spol/cases/");
      }
      gauge_file = gauge_dir + strlen(gauge_dir);
    }

    std::string cmd_text =
      se_unquote_string(cmd_tokens[token_num].getCommandText());

    if (cmd_text == "test")
    {
      cmd_text = "/scr/hotlips/oye/spol/cases/walnut_gauges";
      cmd_text = "/scr/hotlips/oye/src/spol/misc/precip98_gauges.txt";
    }
    else if (cmd_text.find("/") == std::string::npos)
    {
      // We do not have a full path name

      strcpy(gauge_file, cmd_text.c_str());
      cmd_text = gauge_dir;
    }

    // code for these routines is in "se_bnd.c"

    absorb_zmap_pts(&seds->top_zmap_list, cmd_text);
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 7, "show-site-values", 0, 7) == 0)
  {
    if (!seds->top_zmap_list)
    {
      printf( "No zmap lists exist!\n" );
      return 1;
    }

    if (!seds->curr_zmap_list)
    {
      seds->curr_zmap_list = seds->top_zmap_list;
    }

    int frame_num;
    std::string field_name = cmd_tokens[token_num++].getCommandText();
    if (cmd_tokens[token_num].getTokenType() == UiCommandToken::UTT_END)
    {
      frame_num = 0;
    }
    else
    {
      frame_num = cmd_tokens[token_num].getIntParam();
      if (frame_num < 0)
      {
	frame_num = 0;
      }
      else if (frame_num > 0 && frame_num <= 6)
      {
	frame_num--;
      }
      else
      {
	frame_num = 0;
      }
    }
    list_zmap_values(field_name, frame_num);
  }
  else if (cmd_tokens[0].getCommandText().compare(0, 7, "select-site-list", 0, 7) == 0)
  {
    std::string field_name = cmd_tokens[token_num++].getCommandText();
    struct zmap_points *zmpc;
    
    for (zmpc = seds->top_zmap_list; zmpc != 0; zmpc = zmpc->next_list)
    {
      if (strcmp(zmpc->list_id, field_name.c_str()) == 0)
	break;
    }

    if (zmpc != 0)
    {
      seds->curr_zmap_list = zmpc;
    }
    else
    {
      printf( "zmap-list: %s cannot be found!\n", field_name.c_str());
      return 1;
    }
  }

  return 1;
}

#endif

/* c------------------------------------------------------------------------ */

#ifdef USE_RADX
#else

int se_xy_stuff(const std::vector< UiCommandToken > &cmd_tokens)
{
  // Pull the arguments out of the command

  std::string src_name = cmd_tokens[1].getCommandText();
  std::string dst_name = cmd_tokens[2].getCommandText();

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();
  
  if (seds->finish_up)
  {
    if (cmd_tokens[0].getCommandText().compare(0, 3, "xy-listing", 0, 3) == 0)
      fclose(x_y_stream);

    return 1;
  }

  // Get the pointer to the general field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(seds->se_frame);
  
  if (seds->process_ray_count == 1)
  {
    if (cmd_tokens[0].getCommandText().compare(0, 3, "xy-listing", 0, 3) == 0)
    { 
      // Open file and write headers

      DD_TIME dts;
      
      seds->histo_start_time = data_info->getTime();
      dts.time_stamp = seds->histo_start_time;

      seds->histo_radar_name = data_info->getRadarNameClean();
      
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
      
      if ((x_y_stream = fopen(file_path.c_str(), "w")) == 0)
      {
	printf("Could not open xy-listing file : %s\n", file_path.c_str());
	seds->punt = YES;
	return 0;
      }
    }
  }

  seds->histo_stop_time = data_info->getTime();

  // Get the field information

  int src_field_num = data_info->getParamIndex(src_name);
  if (src_field_num < 0)
  {
    printf("Source parameter %s not found for xy-listing\n", src_name.c_str());
    seds->punt = YES;
    return -1;
  }

  int dst_field_num = data_info->getParamIndex(dst_name);
  if (dst_field_num < 0)
  {
    printf("Source parameter %s not found for xy-listing\n", dst_name.c_str());
    seds->punt = YES;
    return -1;
  }
  
  int num_cells = data_info->getNumCells();

  short *src_data_ptr = (short *)data_info->getParamData(src_field_num);
  short *end_src_data_ptr = src_data_ptr +num_cells;
  float src_scale = data_info->getParamScale(src_field_num);
  float src_rcp_scale = 1.0 / src_scale;
  float src_bias = data_info->getParamBias(src_field_num);

  short *dst_data_ptr = (short *)data_info->getParamData(dst_field_num);
  float dst_scale = data_info->getParamScale(dst_field_num);
  float dst_rcp_scale = 1.0 / dst_scale;
  float dst_bias = data_info->getParamBias(dst_field_num);
  int dst_bad = static_cast<int>(data_info->getParamBadDataValue(dst_field_num));

  unsigned short *bnd = seds->boundary_mask;

  // Do the work!

  if (cmd_tokens[0].getCommandText().compare(0, 3, "xy-listing", 0, 3) == 0)
  { 
    for (; src_data_ptr < end_src_data_ptr;
	 src_data_ptr++, dst_data_ptr++, bnd++)
    {
      if (*bnd && *src_data_ptr != dst_bad && *dst_data_ptr != dst_bad)
      {
	float x = DD_UNSCALE((float)(*src_data_ptr), src_rcp_scale, src_bias);
	float y = DD_UNSCALE((float)(*dst_data_ptr), dst_rcp_scale, dst_bias);
	fprintf(x_y_stream, "%.2f %.2f\n", x, y);
      }
    }
  }

  return 1;
}  

#endif

/* c------------------------------------------------------------------------ */

