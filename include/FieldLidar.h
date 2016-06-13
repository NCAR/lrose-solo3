/*
 *      $Id$
 *
 *      Module:          FieldLidar.h
 *      Original Author: Rich Neitzel
 *      Copywrited by the National Center for Atmospheric Research
 *      Date:            $Date$
 *
 * revision history
 * ----------------
 * $Log$
 * Revision 1.1  2002/07/02 14:57:33  oye
 * Initial revision
 *
 * Revision 1.1  1995/08/23 15:39:04  oye
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

#ifndef _FIELDLIDAR_H
#define _FIELDLIDAR_H

#ifdef __cplusplus
extern "C" {
#endif

struct field_lidar_i {
    char field_lidar_info[4];   /* Identifier for a field written */
                                /* lidar information block (four ASCII */
                                /* characters "FLIB"). */
    int32_t field_lidar_info_len;  /* Length of the field written lidar */
                                /* information block . */
    int32_t data_sys_id;           /* Data system identification number. */
    float transmit_beam_div[10]; /* Transmitter beam divergence. Entry */
                                /* [0] is for wavelength #1 etc. */
    float xmit_power[10];       /* Nominal peak transmitted power (by */
                                /* channel). Entry [0] is for */
                                /* wavelength #1 etc. */
    float receiver_fov[10];     /* Receiver field of view. */
    int32_t receiver_type[10];     /* 0=direct detection,no */
                                /* polarization,1=direct detection */
                                /* polarized parallel to transmitted */
                                /* beam,2 = direct detection, */
                                /* polarized perpendicular to */
                                /* transmitted beam,3= photon counting */
                                /* no polarization, 4= photon counting */
                                /* polarized parallel to transmitted */
                                /* beam,5 = photon counting, polarized */
                                /* perpendicular to transmitted beam. */
    float r_noise_floor[10];    /* Receiver noise floor. */
    float receiver_spec_bw[10]; /* Receiver spectral bandwidth */
    float receiver_elec_bw[10]; /* Receiver electronic bandwidth */
    float calibration[10];      /* 0 = linear receiver,  non zero log */
                                /* reciever */
    int32_t range_delay;           /* Delay between indication of */
                                /* transmitted pulse in the data */
                                /* system and the pulse actually */
                                /* leaving the telescope (can be */
                                /* negative). */
    float peak_power_multi[10]; /* When the measured peak transmit */
                                /* power is multiplied by this number */
                                /* it yields the actual peak transmit */
                                /* power. */
    float encoder_mirror_up;    /* Encoder reading minus IRU roll */
                                /* angle when scan mirror is pointing */
                                /* directly vertically up in the roll */
                                /* axes. */
    float pitch_mirror_up;      /* Scan mirror pointing angle in pitch */
                                /* axes, minus IRU pitch angle, when */
                                /* mirror is pointing directly */
                                /* vertically up in the roll axes. */
    int32_t max_digitizer_count;   /* Maximum value (count) out of the */
                                /* digitizer  */
    float max_digitizer_volt;   /* Voltage that causes the maximum */
                                /* count out of the digitizer. */
    float digitizer_rate;       /* Sample rate of the digitizer. */
    int32_t total_num_samples;     /* Total number of A/D samples to */
                                /* take. */
    int32_t samples_per_cell;      /* Number of samples average in range
				   per data cell. */
    int32_t cells_per_ray;         /* Number of data cells averaged
				   per data ray. */
    float pmt_temp;             /* PMT temperature */
    float pmt_gain;             /* D/A setting for PMT power supply */
    float apd_temp;             /* APD temperature */
    float apd_gain;             /* D/A setting for APD power supply */
    int32_t transect;              /* transect number */
    char derived_names[10][12]; /* Derived parameter names */
    char derived_units[10][8];  /* Derived parameter units */
    char temp_names[10][12];    /* Names of the logged temperatures */
};

typedef struct field_lidar_i field_lidar_i;
typedef struct field_lidar_i FIELDLIDAR;
#ifdef __cplusplus
};
#endif

#endif
