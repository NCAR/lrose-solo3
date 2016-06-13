/*
 *	$Id$
 *
 *	Module:		 FieldParam.h
 *	Original Author: Richard E. K. Neitzel
 *      Copywrited by the National Center for Atmospheric Research
 *	Date:		 $Date$
 *
 * revision history
 * ----------------
 * $Log$
 * Revision 1.1  2002/07/02 14:57:33  oye
 * Initial revision
 *
 * Revision 1.5  1995/01/18 20:44:39  case
 * Removed the ending comment from the log file section.
 *
 * Revision 1.4  1995/01/18  20:08:37  case
 * Added a missing end of comment 
 *
 * Revision 1.3  1994/04/05  15:24:21  case
 * moved an ifdef RPC and changed else if to else and if on another line
 * >> to keep the HP compiler happy.
 * >> .
 *
 * Revision 1.2  1991/10/15  17:54:55  thor
 * Fixed to meet latest version of tape spec.
 *
 * Revision 1.2  1991/10/15  17:54:55  thor
 * Fixed to meet latest version of tape spec.
 *
 * Revision 1.1  1991/08/30  18:39:21  thor
 * Initial revision
 *
 *
 *
 * description:
 *        
 */
#ifndef INCFieldParamh
#define INCFieldParamh

#ifdef OK_RPC

#if defined(UNIX) && defined(sun)
#include <rpc/rpc.h>
#else
#if defined(WRS)
#include "rpc/rpc.h"
#endif
#endif

#endif

struct field_parameter_data {
    char  field_param_data[4];	/* Field parameter data identifier */
				/* (ascii characters FRAD) */
    int32_t field_param_data_len;	/* Length of the field parameter */
				/* data block in bytes */
    int32_t data_sys_status;	/* Status word, bits will be assigned */
                                /*  particular status when needed */
    char  radar_name[8];	/* Name of radar from which this data ray  */
				/* came from */
    float test_pulse_level;	/* Test pulse power level as measured by the */
                                /*  power meter in dbm */
    float test_pulse_dist;	/* Distance from antenna to middle of */
				/* test pulse in km */
    float test_pulse_width;	/* Test pulse width in m  */
    float test_pulse_freq;	/* Test pulse frequency in Ghz */
    short test_pulse_atten;	/* Test pulse attenuation in db */
    short test_pulse_fnum;	/* Frequency number being calibrated */
				/* with the test pulse (what mux on */
				/* timing module is set to) */
    float noise_power;		/* Total estimated noise power in dbm */
    int32_t  ray_count;		/* Data Ray counter For this */
				/* particular type of data ray */
    short first_rec_gate;	/* First recorded gate number (N) */
    short last_rec_gate;	/* Last recorded gate number (M) */
};				/* End of Structure */

typedef struct field_parameter_data field_parameter_data;
typedef struct field_parameter_data FIELDPARAMDATA;

#ifdef OK_RPC
#if defined(sun) || defined(WRS)
bool_t xdr_field_parameter_data(XDR *, field_parameter_data *);
#endif

#endif

#endif
