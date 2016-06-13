/* created using cproto */
/* Tue Jun 21 22:05:27 UTC 2011*/

#ifndef dorade_ncdf_hh
#define dorade_ncdf_hh

#include <dd_general_info.h>
/* dorade_ncdf.c */

struct ncdf_production {
  time_t vol_start_time;
  time_t base_time;
  int radar_num;
  int sweep_count;
  int vol_count;
  int new_sweep;
  int new_vol;
  int source_format;
  char filename[256];
  char vol_stt[32];

  int ncid;
  int num_cells;
  int ray_count;

  int id_lat;
  int id_lon;
  int id_alt;
  int id_agl;
  int id_az;
  int id_el;
  int id_g1;
  int id_gs;
  int id_nyq;
  int id_rconst;
  int id_ra;
  int id_fa;
  int id_ut;
  int id_tmoffs;
  int id_rotang;
  int id_tilt;
  int id_hdg;
  int id_drift;
  int id_roll;
  int id_pitch;
  int id_track;
  int id_nsv;
  int id_ewv;
  int id_nsw;
  int id_eww;
  int id_gspd;
  int id_wspd;
  int id_aspd;
  int id_vspd;
  int id_vwspd;
  int id_wdir;
  int id_hchng;
  int id_rchng;
  int id_pchng;
  int id_tsrt;
  int id_azc;
  int id_elc;
  int id_rngc;
  int id_lonc;
  int id_latc;
  int id_paltc;
  int id_raltc;
  int id_ewc;
  int id_nsc;
  int id_vrtc;
  int id_hdgc;
  int id_rollc;
  int id_pitchc;
  int id_driftc;
  int id_rotc;
  int id_tiltc;
  int id_vst;
  int id_acvc;
  int id_rcvrg;
  int id_xmtrg;
  int id_antnag;
  int id_sysg;
  int id_bmwdt;
  int id_plsw;
  int id_bndw;
  int id_pkpwr;
  int id_xmtpwr;
  int id_noispwr;
  int id_tppwr;
  int id_tprng0;
  int id_tprng1;
  int id_prf;
  int id_wvlen;
  int id_clip;

};

enum field_types
{
    DBZ_FLD
  , DM_FLD
  , VEL_FLD
  , RHO_FLD
  , PHI_FLD

  , ZDR_FLD
  , LDR_FLD
  , LVDR_FLD
  , NCP_FLD
  , SW_FLD

  , CH_FLD
  , CV_FLD
  , AH_FLD
  , AV_FLD
  , NIQ_FLD

  , AIQ_FLD
  , UNK_FLD  
  , KDP_FLD
  , RAC_FLD
  , PID_FLD

  , SDPHI_FLD
  , SDVR_FLD
  , SDZDR_FLD
  , HDR_FLD
  , TEMP_FLD
};

typedef int t_ident;

struct rnc_info {
  enum field_types field_type;
  char    units[32];
  char    long_name[80];
  float   valid_range[2];
  /*
  // min - max
   */
  float   ncdf_scale_factor;
  float   ncdf_add_offset;
  float   missing_value;
  float   fill_value;
};


extern struct rnc_info *return_rnc_info(char *name);
extern void produce_ncdf(struct dd_general_info *dgi, int finish);
extern void ncdf_finish(struct ncdf_production *ncp);
extern void ncdf_init_sweep(struct dd_general_info *dgi, struct ncdf_production *ncp);
extern void ncp_stuff_ray(struct dd_general_info *dgi, struct ncdf_production *ncp);

#endif
