/* 	$Id$	 */
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/uio.h>


#include <Volume.h>
#include <CellSpacing.h>
#include <RadarDesc.h>
#include <Parameter.h>
#include <Platform.h>
#include <Ray.h>
#include <Waveform.h>
#include <FieldRadar.h>
#include <FieldParam.h>
#include <CellVector.h>
#include <Correction.h>
#include <Comment.h>
#include <Sweep.h>
#include <Pdata.h>

struct volume_header{
       VOLUME   volume;
       COMMENT  comment;
       WAVEFORM waveform;
   };

struct sensorDes{
       RADARDESC    radar;
       FIELDRADAR   fradar;
       CELLVECTOR   cell;
       CORRECTION   correction;
       PARAMETER    parameter[10];
   };

struct sweep_information{
       SWEEPINFO       sweep;
       RAY             ray;
       PLATFORM        air;
       PARAMDATA       parameterdata[10];
       FIELDPARAMDATA  fp;
   };
       

/*declare external structures*/
  struct volume_d volume;
  struct comment_d comment;
  struct radar_d  radar;
  struct parameter_d parameter[10];
  struct waveform_d waveform;
  struct correction_d correction;
  struct platform_i  air;
  struct cell_d cell;
  struct ray_i ray;
  struct volume_header vheader;
  struct sensorDes     sensordescriptor;  
  struct sweep_information sweeprec;
  struct sweepinfo_d   sweep;
  struct paramdata_d   parameterdata[10];
  struct field_radar_i fradar;
  struct field_parameter_data  fp;
  
  int sweepnumber;
  char *volheadpointer;




