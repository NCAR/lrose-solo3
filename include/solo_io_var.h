/* 	$Id$	 */
/******************************************************************************
*  INCLUDE:             solo_io_var.h        10/92
*  Description:         Structure definitions and Global variables for
*                       solo_io.c
******************************************************************************/

#ifndef SOLO_GLOBAL
#define SOLO_GLOBAL extern
#endif

#include "solo_defs.h"

/*       FILE I/O Structures                  */


typedef struct{
    char filename[80];
    char vol_header[80];
    int32_t current_time;
    int32_t unix_time;
    int  cell_v_flag;
    int  old_num_params;
}IO_STUFF;

typedef struct{
    struct ray_i rray;
    struct platform_i rplat;
    struct field_parameter_data rdata;
  }BEAM;

SOLO_GLOBAL BEAM *beam;

/******* I think I can remove this since I am using a different method
         to read this stuff in -- remove 11/93                        ****/
/************************************************************
  Pointers for DORADE format Super SWIB, SWIB, RYIB,
  Platform data and Parameter Data block
 ************************************************************/
SOLO_GLOBAL struct super_SWIB *SSWIBptr;
SOLO_GLOBAL SWEEPINFO         *SWIBptr;
SOLO_GLOBAL RAY               *RAYptr;
SOLO_GLOBAL PLATFORM          *PLATFptr;
SOLO_GLOBAL PARAMDATA         *PDATAptr;
SOLO_GLOBAL PARAMETER         *PARMptr;
SOLO_GLOBAL CELLVECTOR        *CELLptr;
SOLO_GLOBAL FILE              *forefptr, *aftfptr;

/***Keep this **/
/***************************************************************
 Pointers for DORADE Volume header information unique to DORADE
 ***************************************************************/
SOLO_GLOBAL  struct volume_d         *VOLptr;
SOLO_GLOBAL  struct radar_d          *RADARptr;
SOLO_GLOBAL  struct parameter_d      *VPARMptr;
SOLO_GLOBAL  struct cell_d           *VCELLptr;
SOLO_GLOBAL  struct correction_d     *CRRCTNptr;


typedef struct{
    struct super_SWIB      SSWIBs;
    SWEEPINFO              SWIBs;
    RAY                    RYIBs;
    PLATFORM               ASIBs;
    PARAMDATA              RDATs;
    PARAMETER              PARMs[15];
    CELLVECTOR             CELLs;
    struct radar_d         RADARs;
    struct volume_d        VOLs;
    struct radar_d         VRADARs;
    struct parameter_d     VPARMs[4];
    struct cell_d          VCELLs;
    struct correction_d    CRRCTNs;
    IO_STUFF               IOs;
}ANTENNA_HDRS;

SOLO_GLOBAL ANTENNA_HDRS *FORE;
SOLO_GLOBAL ANTENNA_HDRS *AFT;
SOLO_GLOBAL ANTENNA_HDRS *WhichAntenna;

SOLO_GLOBAL char WhichField[8];

typedef struct{
     char tag[4];
     int32_t structsize;
     char structdata[1];          /* 'C' trick makes space for variable size */
}STRUCT_D;

SOLO_GLOBAL STRUCT_D *SDptr, *HeaderDesc[HEADER_BLOCKS], *RayDesc[RAY_BLOCKS];
SOLO_GLOBAL STRUCT_D RayData;
