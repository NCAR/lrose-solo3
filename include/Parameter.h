/*
 *	$Id$
 *
 *	Module:		 Parameter.h
 *	Original Author: Richard E. K. Neitzel
 *      Copywrited by the National Center for Atmospheric Research
 *	Date:		 $Date$
 *
 * revision history
 * ----------------
 * $Log$
 * Revision 1.1  2002/07/02 14:57:34  oye
 * Initial revision
 *
 * Revision 1.5  1996/01/08 16:41:54  oye
 * Added some comments and cell vector information unique to this parameter.
 *
 * Revision 1.4  1995/12/13  17:10:34  oye
 * Added an eff_unamb_vel (Nyquist velocity) for each parameter.
 *
 * Revision 1.3  1995/10/25  17:00:32  oye
 * New extended parameter descriptor. This header also includes the older
 * parameter descriptor named "struct parameter_d_v00".
 *
 * Revision 1.2  1994/04/05  15:32:47  case
 * moved an ifdef RPC and changed an else if to make HP compiler happy.
 *
 * Revision 1.5  1992/07/28  17:31:33  thor
 * Removed filter_flag.
 *
 * Revision 1.4  1992/04/20  17:18:31  thor
 * Latest Eldora/Asterea revisions included.
 *
 * Revision 1.3  1991/11/25  20:07:45  thor
 * Added filter flag.
 *
 * Revision 1.2  1991/10/15  17:56:03  thor
 * Fixed to meet latest version of tape spec.
 *
 * Revision 1.1  1991/08/30  18:39:32  thor
 * Initial revision
 *
 *
 *
 * description:
 *        
 */
#ifndef INCParameterh
#define INCParameterh

#ifdef OK_RPC

#if defined(UNIX) && defined(sun)
#include <rpc/rpc.h>
#else
#if defined(WRS)
#include "rpc/rpc.h"
#endif
#endif /* UNIX */

#endif /* OK_RPC */

struct parameter_d {
    char  parameter_des[4];	/* Parameter Descriptor identifier */
				/* (ascii characters "PARM"). */
    int32_t parameter_des_length;	/* Parameter Descriptor length in */
				/* bytes.*/
    char  parameter_name[8];	/* Name of parameter being described. */
    char  param_description[40]; /* Detailed description of this parameter. */
    char  param_units[8];	/* Units parameter is written in. */
    short interpulse_time;	/* Inter-pulse periods used. bits 0-1 */
				/* = frequencies 1-2. */
    short xmitted_freq;		/* Frequencies used for this */
				/* parameter. */
    float recvr_bandwidth;	/* Effective receiver bandwidth for */
				/* this parameter in MHz.*/
    short pulse_width;		/* Effective pulse width of parameter */
				/* in m. */
    short polarization;		/* Polarization of the radar beam for */
				/* this parameter (0 Horizontal,1 */
				/* Vertical,2 Circular,3 Elliptical) in na. */
    short num_samples;		/* Number of samples used in estimate */
				/* for this parameter. */
    short binary_format;	/* Binary format of radar data. */
    char  threshold_field[8];	/* Name of parameter upon which this */
				/* parameter is thresholded (ascii */
				/* characters NONE if not */
				/* thresholded). */
    float threshold_value;	/* Value of threshold in ? */
    float parameter_scale;	/* Scale factor for parameter. */
    float parameter_bias;	/* Bias factor for parameter. */
    int32_t  bad_data;		/* Bad data flag. */
    
				/* 1995 extension #1 */

    int32_t extension_num;
    char  config_name[8];	/* used to identify this set of
				 * unique radar characteristics */
    int32_t  config_num;
    int32_t offset_to_data;	/* bytes added to the data struct pointer
				 * to point to the first datum whether it's
				 * an RDAT or a QDAT
				 */
    float mks_conversion;
    int32_t num_qnames;		
    char qdata_names[32];	/* each of 4 names occupies 8 characters
				 * of this space
				 * and is blank filled. Each name identifies
				 * some interesting segment of data in a
				 * particular ray for this parameter.
				 */
    int32_t num_criteria;
    char criteria_names[32];	/* each of 4 names occupies 8 characters
				 * and is blank filled. These names identify
				 * a single interesting floating point value
				 * that is associated with a particular ray
				 * for a this parameter. Examples might
				 * be a brightness temperature or
				 * the percentage of cells above or
				 * below a certain value */
    int32_t number_cells;
    float meters_to_first_cell;	/* center! */
    float meters_between_cells;
    float eff_unamb_vel;	/* Effective unambiguous velocity, m/s. */

}; /* End of Structure */


struct parameter_d_v00 {
    char  parameter_des[4];	/* Parameter Descriptor identifier */
				/* (ascii characters "PARM"). */
    int32_t parameter_des_length;	/* Parameter Descriptor length in */
				/* bytes.*/
    char  parameter_name[8];	/* Name of parameter being described. */
    char  param_description[40]; /* Detailed description of this parameter. */
    char  param_units[8];	/* Units parameter is written in. */
    short interpulse_time;	/* Inter-pulse periods used. bits 0-1 */
				/* = frequencies 1-2. */
    short xmitted_freq;		/* Frequencies used for this */
				/* parameter. */
    float recvr_bandwidth;	/* Effective receiver bandwidth for */
				/* this parameter in MHz.*/
    short pulse_width;		/* Effective pulse width of parameter */
				/* in m. */
    short polarization;		/* Polarization of the radar beam for */
				/* this parameter (0 Horizontal,1 */
				/* Vertical,2 Circular,3 Elliptical) in na. */
    short num_samples;		/* Number of samples used in estimate */
				/* for this parameter. */
    short binary_format;	/* Binary format of radar data. */
    char  threshold_field[8];	/* Name of parameter upon which this */
				/* parameter is thresholded (ascii */
				/* characters NONE if not */
				/* thresholded). */
    float threshold_value;	/* Value of threshold in ? */
    float parameter_scale;	/* Scale factor for parameter. */
    float parameter_bias;	/* Bias factor for parameter. */
    int32_t  bad_data;		/* Bad data flag. */
}; /* End of Structure */


typedef struct parameter_d parameter_d;
typedef struct parameter_d PARAMETER;

#ifdef OK_RPC
#if defined(sun) || defined(WRS)
bool_t xdr_parameter_d(XDR *, PARAMETER *);
#endif

#endif /* OK_RPC */

#endif /* INCParameterh */








