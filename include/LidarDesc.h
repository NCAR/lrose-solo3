/*
 *      $Id$
 *
 *      Module:          LidarDesc.h
 *      Original Author: Rich Neitzel
 *      Copywrited by the National Center for Atmospheric Research
 *      Date:            $Date$
 *
 * revision history
 * ----------------
 * $Log$
 * Revision 1.1  2002/07/02 14:57:34  oye
 * Initial revision
 *
 * Revision 1.1  1995/08/23 15:39:48  oye
 * New.
 *
 * Revision 1.1  1995/06/16  16:32:02  thor
 * Initial revision
 *
 *
 *
 * description:
 *        
 */

#ifndef _LIDARDESC_H
#define _LIDARDESC_H

#ifdef __cplusplus
extern "C" {
#endif

struct lidar_d {    
    char lidar_des[4];          /* Identifier  a lidar descriptor */
                                /* block (four ASCII characters */
                                /* "LIDR"). */
    int32_t lidar_des_length;      /* Length of a lidar descriptor block. */
    char lidar_name[8];         /* Eight character lidar */
                                /* name. (Characters SABL) */
    float lidar_const;          /* Lidar constant. */
    float pulse_energy;         /* Typical pulse energy of the lidar. */
    float peak_power;           /* Typical peak power of the lidar. */
    float pulsewidth;           /* Typical pulse width. */
    float aperature_size;       /* Diameter of the lidar aperature. */
    float field_of_view;        /* Field of view of the receiver. mra; */
    float aperature_eff;        /* Aperature efficiency. */
    float beam_divergence;      /* Beam divergence. */
    short lidar_type;           /* Lidar type: 0) Ground,  1) Airborne */
                                /* fore,  2) Airborne aft,  3) */
                                /* Airborne tail,  4) Airborne lower */
                                /* fuselage,  5) Shipborne. 6) */
                                /* Airborne Fixed */
    short scan_mode;            /* Scan mode:  0) Calibration,  1) PPI */
                                /* (constant elevation),  2) Co-plane, */
                                /* 3) RHI (Constant azimuth),  4) */
                                /* Vertical pointing up,  5) Target */
                                /* (stationary),  6) Manual,  7) Idle */
                                /* (out of control), 8) Surveillance, */
                                /* 9) Vertical sweep, 10) Vertical */
                                /* scan. 11) Vertical pointing down, */
                                /* 12 Horizontal pointing right, 13) */
                                /* Horizontal pointing left */
    float req_rotat_vel;        /* Requested rotational velocity of */
                                /* the scan mirror. */
    float scan_mode_pram0;      /* Scan mode specific parameter #0 */
                                /* (Has different meanings for */
                                /* different scan modes) (Start angle */
                                /* for vertical scanning). */
    float scan_mode_pram1;      /* Scan mode specific parameter #1 */
                                /* (Has different meaning for */
                                /* different scan modes) (Stop angle */
                                /* for vertical scanning). */
    short num_parameter_des;    /* Total number of parameter */
                                /* descriptor blocks for this lidar. */
    short total_number_des;     /* Total number of all descriptor */
                                /* blocks for this lidar. */
    short data_compress;        /* Data compression scheme in use:  0) */
                                /* no data compression, 1) using HRD */
                                /* compression scheme. */
    short data_reduction;       /* Data reduction algorithm in use: */
                                /* 0) None, 1) Between two angles, 2) */
                                /* Between concentric circles. 3) */
                                /* Above and below certain altitudes. */
    float data_red_parm0;       /* Data reduction algorithm specific */
                                /* parameter  #0:  0) Unused, 1) */
                                /* Smallest positive angle in degrees, */
                                /* 2) Inner circle diameter in km,  3) */
                                /* Minimum altitude in km. */
    float data_red_parm1;       /* Data reduction algorithm specific */
                                /* parameter  #1 0) unused, 1) Largest */
                                /* positive angle in degrees, 2) Outer */
                                /* circle diameter in km,  3) Maximum */
                                /* altitude in km. */
    float lidar_longitude;      /* Longitude of airport from which */
                                /* aircraft took off  northern */
                                /* hemisphere is positive, southern */
                                /* negative. */
    float lidar_latitude;       /* Latitude of airport from which */
                                /* aircraft took off eastern */
                                /* hemisphere is positive, western */
                                /* negative. */
    float lidar_altitude;       /* Altitude of airport from which */
                                /* aircraft took off up is positive, */
                                /* above mean sea level. */
    float eff_unamb_vel;        /* Effective unambiguous velocity.. */
    float eff_unamb_range;      /* Effective unambiguous range. */
    int num_wvlen_trans;        /* Number of different wave lengths */
                                /* transmitted. */
    float prf;                  /* Pulse repetition frequency. */
    float wavelength[10];       /* Wavelengths of all the different */
                                /* transmitted light. */
};

typedef struct lidar_d lidar_d;
typedef struct lidar_d LIDARDESC;

#ifdef __cplusplus
};
#endif

#endif /* _LIDARDESC_H */
