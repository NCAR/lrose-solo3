#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */


/*
 * This file contains the following routines
 * 
 *
 * dd_absorb_header
 * dd_absorb_ray_info
 * dd_absorb_rotang_info
 * dd_absorb_seds
 * dd_return_sweepfile_structs_v3
 * dd_setup_sweepfile_structs_v3
 * dd_sweepfile_list_v3
 * 
 * 
 * dd_ts_start
 * dd_update_ray_info
 * dd_update_sweep_info
 * 
 * ddswp_last_ray
 * ddswp_new_sweep_v3
 * ddswp_next_ray_v3
 * ddswp_stuff_ray
 * ddswp_stuff_sweep
 * 
 */

#include <LittleEndian.hh>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <FieldRadar.h>
#include <Xtra_stuff.h>
#include <CellSpacingFP.h>
#include "dorade_share.hh"
#include "dd_files.hh"
#include "dd_files.h"
#include "input_limits.hh"
#include "dd_stats.h"
#include "dda_common.hh"
#include "ddb_common.hh"
#include "dd_crackers.hh"
#include "swp_file_acc.hh"

using namespace std;

# include <piraqx.h>

# define NEW_MAX_READ MAX_READ/2
# ifndef NULL_RECORD
# define         NULL_RECORD -5
# endif

#define SWAP_CHECK_REC_SIZE 65535

static struct dd_input_sweepfiles_v3 *dis_v3=NULL;

/*
 * static vars for reading dynamic rotation angle table
 * to keep reads separate from normal reads
 */

static int _buf_bytes_left = MAX_READ;
static int _file_byte_count = 0;
static char *_in_next_block = NULL;
static char *_in_buf = NULL;
static int32_t _disk_offset_read_start = 0;
//static int _note_printed = 0;

/* c------------------------------------------------------------------------ */

int 
dd_absorb_header_info (DGI_PTR dgi)
{
    int ii, jj, mm, nn, pn, bc=0, loop_count=0, gottaSwap=NO, broken=NO;
    int iret = 0;
    short ns;
    int gotta_cfac = NO, gotta_celv = NO;
    char *aa, *at, *bb, str[256], radar_name[16];
    char *deep6=0, *cfac_id;
    struct generic_descriptor *gd;
    DDS_PTR dds=dgi->dds;
    struct dd_input_sweepfiles_v3 *dis;
    u_int32_t gdsos;
    float *fp, rr, gs;
    CELLSPACINGFP * csfd;
    struct cfac_wrap *cfw;
    XTRA_STUFF *xstf;
    struct field_radar_i *frib = 0;

    /*
     * absorb header info
     */
    dis = dd_return_sweepfile_structs_v3();
    dgi_buf_rewind(dgi);
    dd_reset_comments();

    if((dgi->buf_bytes_left=read(dgi->in_swp_fid, dgi->in_buf, MAX_READ ))
       < 1 ) {
	printf("Bad read or eof state: %d in dd_absorb_header\n"
	       , dgi->buf_bytes_left );
	return(1);
    }

    dis->MB += BYTES_TO_MB(dgi->buf_bytes_left);
    dis->rec_count++;
    dis->sweep_count++;
    dis->file_count++;
    dgi->num_parms = 0;

    for(;;) {
	loop_count++;
	gd = (struct generic_descriptor *)dgi->in_next_block;
	gdsos = (u_int32_t)gd->sizeof_struct;

	if(gdsos > SWAP_CHECK_REC_SIZE) {
	   gottaSwap = YES;
	   swack4((char *)&gd->sizeof_struct, (char *)&gdsos);
	}
	if(gdsos < sizeof(struct generic_descriptor) ||
	   gdsos > SWAP_CHECK_REC_SIZE) {
		 broken = YES;
		 break;
	}
	
	if(strncmp(dgi->in_next_block, "RYIB", 4) == 0 )
	      break;
	
	if(strncmp(dgi->in_next_block, "SSWB", 4) == 0 ) {
	   if(gottaSwap) {
	      if(hostIsLittleEndian()) {
                if (sizeof(struct super_SWIB) > 196) {
                  /* need swap routine which handles 8-byte alignment */
                  ddin_crack_sswbLE_aligned8
                    (dgi->in_next_block, (char *)dds->sswb, (int)0);
                } else {
                  ddin_crack_sswbLE
                    (dgi->in_next_block, (char *)dds->sswb, (int)0);
                }
	      }
	      else {
                if (sizeof(struct super_SWIB) > 196) {
                  /* need swap routine which handles 8-byte alignment */
                  ddin_crack_sswb_aligned8
                    (dgi->in_next_block, (char *)dds->sswb, (int)0);
                } else {
                  ddin_crack_sswb
                    (dgi->in_next_block, (char *)dds->sswb, (int)0);
                }
	      }
	   }
	   else {
	      memcpy((char *)dds->sswb, dgi->in_next_block, gdsos);
	   }
	    dgi->source_num_parms = dgi->num_parms = 0;
	    bc += gdsos;
	}
	else if(strncmp(dgi->in_next_block,"VOLD",4)==0) {
	  
	   if(gottaSwap) {
	      ddin_crack_vold(dgi->in_next_block, (char *)dds->vold, (int)0);
	      
	   }
	   else {
	      memcpy((char *)dds->vold, dgi->in_next_block, gdsos);
	      
	   }
	    if(dgi->source_vol_num != dds->vold->volume_num) {
		dgi->new_vol = YES;
		dis->vol_count++;
	    }
	    dgi->source_vol_num = dds->vold->volume_num;
	    bc += gdsos;
	}
	else if(strncmp(dgi->in_next_block,"RADD",4)==0 ) {
	   if(gottaSwap) {
	      ddin_crack_radd(dgi->in_next_block, (char *)dds->radd, (int)0);
	      if(gdsos < sizeof(struct radar_d))
		{ dds->radd->extension_num = 0; }
	   }
	   else {
	      memcpy((char *)dds->radd, dgi->in_next_block, gdsos);
	   }
	   strcpy(radar_name, str_terminate(str, dds->radd->radar_name, 8));
	   dgi->radar_name = radar_name;
	    for(ii=0; ii < MAX_PARMS; dds->field_present[ii++]=NO);
	    /* always put the lat/lon/alt in the ASIB
	     */
	    dds->asib->longitude = dds->radd->radar_longitude;
	    dds->asib->latitude = dds->radd->radar_latitude;
	    dds->asib->altitude_msl = dds->radd->radar_altitude;
	    dds->asib->heading = 0;
	    bc += gdsos;
	}
	else if(strncmp(dgi->in_next_block,"CSFD",4)==0 ) {
	    /* funky cell spacing info in map condensed files */

	    fp = dds->celv->dist_cells;
	    dds->celv->number_cells = 0;
	    at = dgi->in_next_block;
	    csfd = (CELLSPACINGFP *)at;

	    if(gottaSwap) {
		aa = at + 8;
		swack4( aa, (char *)&mm );
		aa += 4;
		swack4( aa, (char *)&rr );
		aa += 4;
		bb = aa + sizeof( csfd->spacing );

		for(; mm-- ; aa += 4, bb += 2 ) {
		    swack4( aa, (char *)&gs );
		    swack2( bb, (char *)&ns );
		    dds->celv->number_cells += ns;

		    for(; ns-- ; rr += gs ) {
			*fp++ = rr;
		    }
		}
	    }
	    else {
		mm = csfd->num_segments;
		rr = csfd->distToFirst;

		for(ii = 0; ii < mm ; ii++ ) {
		    gs = csfd->spacing[ ii ];
		    ns = csfd->num_cells[ ii ];
		    dds->celv->number_cells += ns;

		    for(; ns-- ; rr += gs ) {
			*fp++ = rr;
		    }
		}
	    }
	    nn = dds->celv->number_cells * sizeof(float) + 12;
	   memcpy((char *)dds->celvc, dds->celv, nn);
	   bc += gdsos;
	   for(ii=0; ii < MAX_PARMS; ii++) {
	      if(dgi->dds->field_present[ii])
		dd_alloc_data_field(dgi, ii);
	   }
	}
	else if(strncmp(dgi->in_next_block,"CELV",4)==0 ) {
	   if(gottaSwap) {
	      ddin_crack_celv(dgi->in_next_block, (char *)dds->celv, (int)0);
	      swack_long(dgi->in_next_block+12, (char*)&dds->celv->dist_cells[0]
			  , (int)dds->celv->number_cells);
	   }
	   else {
	      memcpy((char *)dds->celv, dgi->in_next_block, gdsos);
	   }
	   memcpy((char *)dds->celvc, dds->celv, gdsos);
	   bc += gdsos;
	   for(ii=0; ii < MAX_PARMS; ii++) {
	      if(dgi->dds->field_present[ii])
		dd_alloc_data_field(dgi, ii);
	   }
	}
	else if(strncmp(dgi->in_next_block,"FRIB",4)==0 ) {
	   if (!dds->frib) {
	      dds->frib = (struct field_radar_i *)malloc (sizeof (*frib));
	   }
	   if(gottaSwap) {
	     ddin_crack_frib(dgi->in_next_block, (char *)dds->frib, (int)0);
	   }
	   else {
	     memcpy (dds->frib, dgi->in_next_block, sizeof (*frib));
	   }
	   dds->frib->field_radar_info_len = sizeof (*frib);
	   frib = dds->frib;
	   bc += gdsos;
	}
	else if(strncmp(dgi->in_next_block,"CFAC",4)==0 ) {
	   dgi->ignore_cfacs = NO;
	   /* get CFAC stuff
	    */
	   aa = getenv ("DEFAULT_CFAC_FILES");
	   if (!aa) { aa = getenv ("DEFAULT_CFAC_FILE");}
	   if (aa) {
	     /* external default cfac files (read in each time) */
	     memset (dgi->dds->cfac, 0, sizeof (*dgi->dds->cfac));
	     strncpy( dgi->dds->cfac->correction_des, "CFAC", 4 );
	     dgi->dds->cfac->correction_des_length = sizeof(*dgi->dds->cfac);
	     printf ("Absorbing CFAC file\n");
	     dd_absorb_cfac( aa, radar_name, dgi->dds->cfac);
	     dgi->ignore_cfacs = YES;
	   }
	   else if (cfw = ddswp_nab_cfacs (radar_name)) {
	     /* external cfac files */
	     cfac_id = (char *)"default";
	     if (cfw->ok_frib && frib)
	       { cfac_id = frib->file_name; }
	     
	     for(; cfw ; cfw = cfw->next ) {
		 if( strstr (cfac_id, cfw->frib_file_name)) {
		    memcpy( dgi->dds->cfac, cfw->cfac, sizeof( *cfw->cfac ));
		    dgi->ignore_cfacs = YES;
		    break;
		 }
	      }
	   }

	   if(!dgi->ignore_cfacs) {
	      if(gottaSwap) {
		 ddin_crack_cfac(dgi->in_next_block, (char *)dds->cfac, (int)0);
	      }
	      else {
		 memcpy((char *)dds->cfac, dgi->in_next_block, gdsos);
	      }
	   }

	   dd_set_uniform_cells(dgi->dds);
	   bc += gdsos;
	   for(ii=0; ii < MAX_PARMS; ii++) {
	      if(dgi->dds->field_present[ii])
		dd_alloc_data_field(dgi, ii);
	   }
	}
	else if(strncmp(dgi->in_next_block, "SWIB", 4) == 0 ) {
	   if(gottaSwap) {
	      ddin_crack_swib(dgi->in_next_block, (char *)dds->swib, (int)0);
	   }
	   else {
	      memcpy((char *)dds->swib, dgi->in_next_block, gdsos);
	   }
	    dgi->source_num_rays = dds->swib->num_rays;
	    dgi->source_sweep_num = dds->swib->sweep_num;
	    dgi->source_ray_num = 0;
	    dgi->new_sweep = YES;
	    bc += gdsos;
	}
	else if(strncmp(dgi->in_next_block, "PARM", 4) == 0 ) {
	    pn = dgi->source_num_parms++;
	    dgi->num_parms = dgi->source_num_parms;
	    dds->field_present[pn] = YES;
	    if(gottaSwap) {
	       ddin_crack_parm(dgi->in_next_block, (char *) dds->parm[pn], (int)0);
	       if(gdsos < sizeof(struct parameter_d))
		 { dds->parm[pn]->extension_num = 0; }
	    }
	    else {
	       memcpy((char *)dds->parm[pn], dgi->in_next_block, gdsos);
	    }
	    str_terminate(str, dds->parm[pn]->param_description
			  , sizeof(dds->parm[pn]->param_description));
	    jj = dds->field_id_num[pn] =
		  dd_return_id_num(dds->parm[pn]->parameter_name);
	    dgi->parm_type[pn] = 
		jj == DBZ_ID_NUM || jj == DZ_ID_NUM ? OTHER : VELOCITY;

	    if( dds->parm[pn]->binary_format == DD_8_BITS ) {
		dds->its_8_bit[pn] = YES;
		dds->parm[pn]->binary_format = DD_16_BITS;
	    }
	    else
		{ dds->its_8_bit[pn] = NO; }

	    bc += gdsos;
	}
	else if(strncmp(dgi->in_next_block, "COMM", 4) == 0 ) {
	    dd_next_comment((struct comment_d *)dgi->in_next_block
			    , gottaSwap);
	}
	dgi->buf_bytes_left -= gdsos;
	dgi->file_byte_count += gdsos;
	dgi->in_next_block += gdsos;
    }
    if(broken) {
	  /* oops! This is probably a garbaged file */
	  printf("Current sweep file is garbage!\n");
	  printf("Last time from previous sweep is %s\n"
		    , dts_print(d_unstamp_time(dgi->dds->dts)));
	  if(dd_solo_flag()) {
	    ii = *deep6;
	  }
	  else
	    exit(1);
	  /* This is probably a garbaged file */
    }
    /* apply range correction to the corrected celv
     */
    for(ii=0; ii < dds->celv->number_cells; ii++) {
	dds->celvc->dist_cells[ii] += dds->cfac->range_delay_corr;
    }
    dd_set_uniform_cells(dgi->dds);
    iret = dd_absorb_rotang_info(dgi, gottaSwap);
    dd_absorb_seds(dgi, gottaSwap);
    
    /* if no rotation angle table was found in the file,
     * create one dynamically by reading the rays */

    if (iret != 0) {
      dd_create_dynamic_rotang_info(dgi);
    }

    return(0);
}
/* c------------------------------------------------------------------------ */

int 
dd_absorb_ray_info (DGI_PTR dgi)
{
    /*
     * Keep sucking up input until all headers (if applicable )
     * and a complete ray have been read in.
     */
    int ii, mm, n, nn, nc, len, ryib_flag=NO, num_rdats=0, mark, *deep6=0;
    int num_cells, final_bad_count, bc=0, pn, ncopy;
    int broken = NO, gottaSwap=NO, ds, clip_cell;
    char *a, *aa, *b;
    short *ss;
    static int count=0, doodah=110, rec_count=0, loop_count = 0;
    static int rec_trip=12, gd_ptr=-1, disk_offset= -1;
    static char *gd_list[22], *last_block=NULL, *last_last_block=NULL;
    struct dd_input_filters *difs;
    struct paramdata_d *rdat;
    struct qparamdata_d *qdat;
    struct ray_i *ryib;
    struct platform_i *asib;
    struct generic_descriptor *gd;
    XTRA_STUFF *xstfo;
    DDS_PTR dds=dgi->dds;
    struct radar_angles *ra=dds->ra;
    DD_TIME *dts=dds->dts;
    struct dd_input_sweepfiles_v3 *dis;
    u_int32_t gdsos;
   PIRAQX *prqx;

    count++;
    if( count >= doodah ) {
	mark = SWAP_CHECK_REC_SIZE;
    }
    difs = dd_return_difs_ptr();
    dis = dd_return_sweepfile_structs_v3();

    while(1) {
       if( ++loop_count >= doodah ) {
	  mark = SWAP_CHECK_REC_SIZE;
       }
	gd = (struct generic_descriptor *)dgi->in_next_block;

	if(dgi->buf_bytes_left >= sizeof(struct generic_descriptor)) {
	   gdsos = (u_int32_t)gd->sizeof_struct;

	   if(gdsos > SWAP_CHECK_REC_SIZE) {
	      gottaSwap = YES;
	      swack4((char *)&gd->sizeof_struct, (char *)&gdsos);
	   }
	}
	else { gdsos = 0; }

	if( num_rdats == dgi->source_num_parms ) {
	    break;
	}
	if(dgi->buf_bytes_left <= 0 ||
	   sizeof(struct generic_descriptor) > dgi->buf_bytes_left ||
	   gdsos > dgi->buf_bytes_left ) {
	    /* move to top of buffer and refill it
	     */
	    rec_count++;
	    if(rec_count >= rec_trip) {
		mark = 0;
	    }
	    if(dgi->buf_bytes_left > 0 )
		  memcpy(dgi->in_buf, dgi->in_next_block
			 , dgi->buf_bytes_left);
	    else if( dgi->buf_bytes_left < 0 ) {
		printf("bytes left < 0 count: %d rec_num: %d\n"
		       , count, rec_count);
		if(dd_solo_flag()) {
		    ii = *deep6;
		}
		else
		      exit(1);
	    }
	    dgi->in_next_block = dgi->in_buf;
	    disk_offset = lseek(dgi->in_swp_fid, 0L, SEEK_CUR); 

	    if((len=read(dgi->in_swp_fid, dgi->in_buf+dgi->buf_bytes_left
			 , MAX_READ -dgi->buf_bytes_left )) < 1 ) {
	       printf("Trying to read past end of data...stat= %d loop: %d\n"
		      , len, loop_count );
	       
	       if(dd_solo_flag()) {
		  ii = *deep6;
	       }
	       else
		 exit(1);
	    }
	    else if((dgi->buf_bytes_left+=len) < gdsos ) {
		printf("File appears to be truncated...stat= %d loop: %d\n"
		       , len, loop_count );
		mark = *deep6;
		exit(1);
	    }
	    dis->MB += BYTES_TO_MB(len);
	    dis->rec_count++;
	    gd = (struct generic_descriptor *)dgi->in_next_block;
	    gdsos = (u_int32_t)gd->sizeof_struct;

	    if(gdsos > SWAP_CHECK_REC_SIZE) {
	       gottaSwap = YES;
	       swack4((char *)&gd->sizeof_struct, (char *)&gdsos);
	    }
	}

	if(gdsos < sizeof(struct generic_descriptor) || gdsos > SWAP_CHECK_REC_SIZE) {
	   /* This is probably a garbaged file */
	   printf("Sweep file containing time %s is garbage! loop: %d\n"
		  , dts_print(d_unstamp_time(dgi->dds->dts)), loop_count);
	   if(dd_solo_flag()) {
	      ii = *deep6;
	   }
	   else
	    exit(1);
	}
	gd_ptr = INC_NDX(gd_ptr, 22);
	gd_list[gd_ptr] = dgi->in_next_block;

	if(strncmp(dgi->in_next_block,"RDAT",4) == 0 ||
	   strncmp(dgi->in_next_block,"QDAT",4) == 0) {
	    qdat = (struct qparamdata_d *)dgi->in_next_block;
	    pn = num_rdats++;
	    final_bad_count = 0;
	    ncopy = *dgi->in_next_block == 'R'
		  ? sizeof(struct paramdata_d) : dds->parm[pn]->offset_to_data;
	    ds = dd_datum_size((int)dds->parm[pn]->binary_format);

	   if(gottaSwap) {
	      ddin_crack_qdat(dgi->in_next_block, (char *)dds->qdat[pn], (int)0);
	   }
	   else {
	      memcpy((char *)dds->qdat[pn], dgi->in_next_block, ncopy);
	   }
	   if( dds->its_8_bit[pn] ) {
	       aa = dgi->in_next_block + ncopy;
	       ss = (short *)dds->qdat_ptrs[pn];
	       mm = gdsos - ncopy;
	       for( ii = 0; ii < mm; ii++, aa++, ss++ ) {
		   *ss = *aa;
	       }
	   }
	   else if(dds->radd->data_compress == NO_COMPRESSION) {
		a = dgi->in_next_block + ncopy;
		mm = gdsos - ncopy;
		if(gottaSwap && ds > 1) {
		   if(ds == sizeof(short)) {
		      swack_short(a, dds->qdat_ptrs[pn], mm/sizeof(short));
		   }
		   else {	/* punt! */
		   }
		}
		else {
		   memcpy(dds->qdat_ptrs[pn], a, mm);
		}
	    }
	    else {
		a = dgi->in_next_block + ncopy;
		b = dds->qdat_ptrs[pn];
		if(gottaSwap) {
		   n = dd_hrd16LE_uncompressx
		     ((unsigned short *)a, (unsigned short *)b, (int)dds->parm[pn]->bad_data
		      , &final_bad_count, dds->celv->number_cells);
		}
		else {
		   n = dd_hrd16_uncompressx
		     ((short *)a, (short *)b, (int)dds->parm[pn]->bad_data
		      , &final_bad_count, dds->celv->number_cells);
		}
		mm = n * dd_datum_size((int)dds->parm[pn]->binary_format);
	    }
	    dds->qdat[pn]->pdata_length = ncopy + dds->sizeof_qdat[pn];

	    /* make the clip gate indicate max last good gate for this ray */
	    if(( nc = num_cells - final_bad_count) > dgi->clip_gate) {
	       dgi->clip_gate = nc -1;
	    }
	    bc += gdsos;
	}
	else if(strncmp(dgi->in_next_block,"RYIB",4)==0 && ryib_flag ) {
	    /* it'll probably never get here since its counting "rdat"s */
	    printf("RDAT count off! loop: %d\n", loop_count);
		if(dd_solo_flag()) {
		    ii = *deep6;
		}
		else
		      exit(1);
	}
	else if(strncmp(dgi->in_next_block,"RYIB",4)==0 ) {
	    dgi->source_ray_num++;
	    dgi->beam_count++;
	    dgi->ray_num++;
	    dis->ray_count++;
	    if(gottaSwap) {
	       ddin_crack_ryib(dgi->in_next_block, (char *)dds->ryib, (int)0);
	    }
	    else {
	       memcpy((char *)dds->ryib, dgi->in_next_block
		      , sizeof(struct ray_i));
	    }
	    ryib_flag = YES;
	    bc += gdsos;
	    num_cells = dds->celv->number_cells;
	    clip_cell = dgi->clip_gate = 0;
	}
	else if(strncmp(dgi->in_next_block,"ASIB",4)==0) {
	   if(gottaSwap) {
	      ddin_crack_asib(dgi->in_next_block, (char *)dds->asib, (int)0);
	   }
	   else {
	      memcpy((char *)dds->asib, dgi->in_next_block
		     , sizeof(struct platform_i));
	   }
	    bc += gdsos;
	}
	else if(strncmp(dgi->in_next_block,"XSTF",4)==0) {

	   if (!dgi->dds->xstf) {
	     dgi->dds->xstf = (XTRA_STUFF *)malloc (gdsos);
	     dgi->dds->xstf->sizeof_struct = gdsos;
	   }
	   if (gdsos != dgi->dds->xstf->sizeof_struct) {
	     free (dgi->dds->xstf);
	     dgi->dds->xstf = (XTRA_STUFF *)malloc (gdsos);
	   }
	   memcpy (dgi->dds->xstf, dgi->in_next_block, gdsos);
	   prqx = (PIRAQX *)(dgi->in_next_block
			     +dgi->dds->xstf->offset_to_first_item);
	   if(gottaSwap) {
	     xstfo = (XTRA_STUFF *)dgi->in_next_block;
	     dgi->dds->xstf->sizeof_struct = gdsos;
	     dgi->dds->xstf->offset_to_first_item =
	       swap4 ((char *)&xstfo->offset_to_first_item);
	     dgi->dds->xstf->source_format =
	       swap4 ((char *)&xstfo->source_format);
	     dgi->dds->xstf->transition_flag =
	       swap4 ((char *)&xstfo->transition_flag);
	   }
	   bc += gdsos;
	}
	else if(strncmp(dgi->in_next_block, "COMM", 4) == 0 ) {
	    dd_next_comment((struct comment_d *)dgi->in_next_block
			    , gottaSwap);
	}
	else if(strncmp(dgi->in_next_block,"NULL",4)==0) {
	    /* it'll probably never get here since its counting rays
	     * and rdats
	     */
	    printf("Read the NULL record...ray count off!\n");
            printf("swib:%d source:%d beams:%d rnum:%d rcount:%d loop: %d\n"
		   , dgi->dds->swib->num_rays
		   , dgi->source_ray_num
		   , dgi->beam_count
		   , dgi->ray_num
		   , dis->ray_count
		   , loop_count
		   );
		if(dd_solo_flag()) {
		    ii = *deep6;
		}
		else
		      exit(NULL_RECORD);

	}

	else {			/* ignore this descriptor */
	    mark = 0;
	    bc += gdsos;
	}

	dgi->buf_bytes_left -= gdsos;
	dgi->file_byte_count += gdsos;
	dgi->in_next_block += gdsos;
    }
    
    dgi->time = dts->time_stamp =
	  dorade_time_stamp(dds->vold, dds->ryib, dts);
    d_unstamp_time(dts);

    if( dgi->dds->radd->scan_mode == DD_SCAN_MODE_AIR ||
       dgi->dds->radd->radar_type == DD_RADAR_TYPE_AIR_LF ||
	dgi->dds->radd->radar_type == DD_RADAR_TYPE_AIR_NOSE )
      {
       
	 dd_radar_angles( dds->asib, dds->cfac, ra, dgi );
      }
    if(difs->altitude_truncations) {
	dgi->clip_gate = dd_clip_gate
	      (dgi, dds->ra->elevation, dds->asib->altitude_msl
	       , difs->altitude_limits->lower
	       , difs->altitude_limits->upper);
    }
    else if(!dgi->clip_gate)
	  dgi->clip_gate = dds->celv->number_cells-1;

    if(!dgi->time0 ) {	/* first time processed */
	dgi->time0 = dgi->time;
    }

    return(bc);
}
/* c------------------------------------------------------------------------ */

int 
dd_absorb_rotang_info (DGI_PTR dgi, int gottaSwap)
{
  int i, jj, n, len;
    struct rot_ang_table *rat;
    char *c, *swap_buf;
    int32_t disk_offset;
    DDS_PTR dds=dgi->dds;

    disk_offset = lseek(dgi->in_swp_fid, 0L, SEEK_CUR); /* remember the disk offset */
    /*
     * now position to the rotation angle table
     * and absorb it
     */
    for(i=0; i < MAX_KEYS; i++) {
	if(dds->sswb->key_table[i].type == KEYED_BY_ROT_ANG)
	      break;
    }
    if (i >= MAX_KEYS) {
      return -1;
    }
    n = lseek(dgi->in_swp_fid, dds->sswb->key_table[i].offset, SEEK_SET);
    len = dds->sswb->key_table[i].size;
    if(dgi->source_rat) {
      free(dgi->source_rat);
    }
    c = (char *)malloc(len);
    rat = dgi->source_rat = (struct rot_ang_table *)c;
    memset(rat, 0, len);
    if(gottaSwap) {
       swap_buf = (char *)malloc(len);
       n = read(dgi->in_swp_fid, swap_buf, len);
       if (n != len) {
         fprintf(stderr, "ERROR - dd_absorb_rotang_info\n");
         fprintf(stderr, "  Cannot read in rotation angle table\n");
         perror("");
         free(swap_buf);
         return -1;
       }
       ddin_crack_rktb(swap_buf, (char *)rat, (int)0);
       jj = rat->angle_table_offset;
       swack_long(swap_buf + jj, (char *)rat + jj, rat->ndx_que_size);
       jj = rat->first_key_offset;
       swack_long(swap_buf + jj, (char *)rat + jj, rat->num_rays * 3);
       free(swap_buf);
    }
    else {
       n = read(dgi->in_swp_fid, c, len);
       if (n != len) {
         fprintf(stderr, "ERROR - dd_absorb_rotang_info\n");
         fprintf(stderr, "  Cannot read in rotation angle table\n");
         perror("");
         free(swap_buf);
         return -1;
       }
    }
    /* now put it back
     */
    disk_offset = lseek(dgi->in_swp_fid, disk_offset, SEEK_SET);
    return(0);
}
/* c------------------------------------------------------------------------ */

int 
dd_absorb_seds (DGI_PTR dgi, int gottaSwap)
{
    /* absorb the editing summary */
    int ii, nn;
    int32_t disk_offset, gdsos;
    struct generic_descriptor *gd;
    DDS_PTR dds=dgi->dds;

    /* remember the disk offset
     */
    disk_offset = lseek(dgi->in_swp_fid, 0L, SEEK_CUR);
    /*
     * now position to the solo edit summary
     * and absorb it
     */
    for(ii=0; ii < dds->sswb->num_key_tables; ii++) {
	if(dds->sswb->key_table[ii].type == SOLO_EDIT_SUMMARY)
	      break;
    }
    if(ii == dds->sswb->num_key_tables) {
	/* no seds */
	return(0);
    }
    if(dgi->seds) free(dgi->seds);

    dgi->sizeof_seds = dds->sswb->key_table[ii].size;

    if((dgi->sizeof_seds = dds->sswb->key_table[ii].size) >= 65536) {
       dgi->seds = NULL;
       dgi->sizeof_seds = 0;
       return(1);
    }
    else if(!(dgi->seds = (char *)malloc(dgi->sizeof_seds))) {
       dgi->seds = NULL;
       dgi->sizeof_seds = 0;
       return(1);
    }
    nn = lseek(dgi->in_swp_fid, dds->sswb->key_table[ii].offset, SEEK_SET);
    nn = read(dgi->in_swp_fid, dgi->seds, dgi->sizeof_seds);

    if(gottaSwap) {
       gd = (struct generic_descriptor *)dgi->seds;
       swack4((char *)&gd->sizeof_struct, (char *)&gdsos);
       gd->sizeof_struct = gdsos;
    }
    /*
     * now move disk back to where it was
     */
    disk_offset = lseek(dgi->in_swp_fid, disk_offset, SEEK_SET);
    return(0);
}

/* c------------------------------------------------------------------------ */

struct dd_input_sweepfiles_v3 *
dd_return_sweepfile_structs_v3 (void)
{
    if(!dis_v3) {
	dis_v3 = (struct dd_input_sweepfiles_v3 *)
	      malloc(sizeof(struct dd_input_sweepfiles_v3));
	memset(dis_v3, 0, sizeof(struct dd_input_sweepfiles_v3));
	dis_v3->usi_top = new struct unique_sweepfile_info_v3;

	dis_v3->usi_top->last = 0;
	dis_v3->usi_top->next = 0;
	dis_v3->usi_top->swp_list = 0;
	dis_v3->usi_top->ddir_num_swps = 0;
	dis_v3->usi_top->ddir_radar_num = 0;
	dis_v3->usi_top->dir_num = 0;
	dis_v3->usi_top->forget_it = 0;
	dis_v3->usi_top->max_num_sweeps = 0;
	dis_v3->usi_top->new_version_flag = 0;
	dis_v3->usi_top->num_rays = 0;
	dis_v3->usi_top->num_swps = 0;
	dis_v3->usi_top->radar_num = 0;
	dis_v3->usi_top->ray_num = 0;
	dis_v3->usi_top->swp_count = 0;
	dis_v3->usi_top->version = 0;
	dis_v3->usi_top->vol_num = 0;
	dis_v3->usi_top->final_swp_list = 0;
	dis_v3->usi_top->first_cfac = 0;
    }
    return(dis_v3);
}
/* c------------------------------------------------------------------------ */

struct dd_input_sweepfiles_v3 *
dd_setup_sweepfile_structs_v3 (int *ok)
{
    int ii, jj, kk, nn, nt, mark, drn, rn;
    int old_num_radars;
    char *a, *radar_names, *str_ptrs[32], name_list[256];
    char *b, *c;
    struct dd_input_filters *difs;
    struct dd_input_sweepfiles_v3 *dis_v3;
    struct unique_sweepfile_info_v3 *usi, *usi_last, *usix;
    double upper, lower;
    struct cfac_wrap *cfw;


    *ok = YES;
    difs = dd_return_difs_ptr();

    if(!(radar_names=get_tagged_string("SELECT_RADARS"))) {
	printf( "Could not find any radar names!\n" );
	*ok = NO;
	return(NULL);
    }
    strcpy(name_list, radar_names);
    nt = dd_tokens(name_list, str_ptrs);
    dis_v3 = dd_return_sweepfile_structs_v3();
    old_num_radars = dis_v3->num_radars;
    dis_v3->num_radars = 0;
    drn = 12;			/* radar numbers are fudged so as not to
				 * interfere with structs for the display
				 */
    /*
     * generate structs for each radar requested
     */
    for(rn=0,usi=dis_v3->usi_top,a=radar_names; rn < nt; rn++) {
	/* for each radar
	 */
	ii = dis_v3->num_radars++;
	/* set up a struct to create a list of the desired sweepfiles
	 * for this radar in this directory
	 */
	if(dis_v3->num_radars > old_num_radars) {
	    if(ii) {
	      usi = new struct unique_sweepfile_info_v3;
	      usi->last = 0;
	      usi->next = 0;
	      usi->swp_list = 0;
	      usi->ddir_num_swps = 0;
	      usi->ddir_radar_num = 0;
	      usi->dir_num = 0;
	      usi->forget_it = 0;
	      usi->max_num_sweeps = 0;
	      usi->new_version_flag = 0;
	      usi->num_rays = 0;
	      usi->num_swps = 0;
	      usi->radar_num = 0;
	      usi->ray_num = 0;
	      usi->swp_count = 0;
	      usi->version = 0;
	      usi->vol_num = 0;
	      usi->final_swp_list = 0;
	      usi->first_cfac = 0;
	      
	      usi->last = usi_last;
	      usi_last->next = usi;
	    }
	    else {
		/* the first one is created automatically */
		usi = dis_v3->usi_top;
	    }
	    dis_v3->usi_top->last = usi;
	    usi->next = dis_v3->usi_top;
	    usi->radar_num = usi->dir_num = drn+ii; /* radar numbers are
						     * fudged so as not to
						     * interfere with structs
						     * for the display 
						     */
	}
	usi_last = usi;
	usi->ray_num = 99999;
	usi->version = 99999; /* latest version */

	char *dir_string = new char[usi->directory.size()+1];
	strcpy(dir_string, usi->directory.c_str());
	slash_path(dir_string, get_tagged_string("DORADE_DIR"));
	usi->directory = dir_string;
	delete [] dir_string;
	
	/*
	 * create a database of sweepfiles for this directory
	 * if it hasn't already been done
	 */
	usix = dis_v3->usi_top;
	for(kk=0; kk < dis_v3->num_radars-1; kk++,usix=usix->next) {
	  if(usi->directory.compare(usix->directory) != 0) {
		break;
	    }
	}
	if(kk != dis_v3->num_radars-1) {
	    /* found this directory somewhere else */
	    usi->dir_num = usix->dir_num;
	    usi->ddir_num_swps = usix->ddir_num_swps;
	}
	else  {
	  usi->ddir_num_swps = mddir_file_list_v3(drn, usi->directory.c_str());
	}
	if(usi->ddir_num_swps < 1) {
	    /* no sweeps in this directory
	     */
	  printf("No sweepfiles in directory: %s\n", usi->directory.c_str());
	    *ok = NO;
	}
	/*
	 * see if we can construct a radar name select string
	 */
	usi->radar_name = str_ptrs[rn];
	if(difs->num_time_lims < 1) {
	    if(!difs->times[0]) difs->times[0] = 
		  (struct d_limits *)malloc(sizeof(struct d_limits));
	    difs->num_time_lims = 1;
	    difs->times[0]->lower = 0;
	    difs->times[0]->upper = ETERNITY;
	}
	/*
	 * see if this radar is in the directory
	 */
	usi->ddir_radar_num = mddir_radar_num_v3(usi->dir_num,
						 usi->radar_name.c_str());
	if(usi->ddir_radar_num >= 0 ) {
	  usi->radar_name = mddir_radar_name_v3(drn, usi->ddir_radar_num);
	    /*
	     * loop throught the time periods requested
	     */
	    for(jj=0; jj < difs->num_time_lims; jj++) {
		lower = difs->times[jj]->lower;
		upper = difs->times[jj]->upper;
		/*
		 * assemple a list of sweep file times for this radar
		 */
		dd_sweepfile_list_v3(usi, lower, upper);
	    }
	    mark = 0;
	}
	else {
	    *ok = NO;
	    printf("Radar name: %s not found in directory: %s\n",
		   usi->radar_name.c_str(), usi->directory.c_str());
	}
    }
    return(dis_v3);
}
/* c------------------------------------------------------------------------ */

int 
dd_sweepfile_list_v3 (struct unique_sweepfile_info_v3 *usi, double start_time, double stop_time)
{
    /*
     * create a list of time stamps of sweep files
     * for the requested time intervals
     */
    struct dd_file_name_v3 *fn, *last, **swps;
    double d, t1;
    int ii, jj, kk, num_swps;
    struct dd_input_sweepfiles_v3 *dis;

    usi->swp_list = swps = 
	  mddir_return_swp_list_v3(usi->dir_num, usi->ddir_radar_num
				   , &num_swps);
    if(num_swps <= 0) {
	return(0);
    }
    usi->vol_num = 
	  usi->swp_count = usi->num_swps = 
		usi->num_rays = usi->ray_num = 0;
    /*
     * assemble a list of unique sweep times for the time span requested
     */
    for(ii=0; ii < num_swps; ii++, swps++) { 
	fn = *swps;
	t1 = ddfn_time(fn);

	if(t1 +.001 < start_time)
	      continue;
	else if(t1 > stop_time && usi->num_swps) /* must have nabbed at least
						  * one scan */
	      break;
	/*
	 * add this one to the list
	 */
	if(usi->num_swps >= usi->max_num_sweeps) {
	    /*
	     * need to create more space
	     */
	    usi->max_num_sweeps += 100;
	    if(usi->final_swp_list) { /* not the first time */
		usi->final_swp_list = (struct dd_file_name_v3 **)
		      realloc(usi->final_swp_list
			      , usi->max_num_sweeps *
			      sizeof(struct dd_file_name_v3 *));
	    }
	    else {
		usi->final_swp_list = (struct dd_file_name_v3 **)
		      malloc(usi->max_num_sweeps *
			     sizeof(struct dd_file_name_v3 *));
	    }
	}
	/*
	 * locate the proper version
	 */
	for(; fn;) {
	    last = fn;
	    if(usi->version < (int)fn->version) {
		fn = fn->ver_left;
	    }
	    else {
		fn = fn->ver_right;
	    }
	}
	*(usi->final_swp_list + usi->num_swps) = last;
	usi->num_swps++;
	last->time = t1;
    }
    return(usi->num_swps); 
}
/* c------------------------------------------------------------------------ */

double 
dd_ts_start (int frme, int radar_num, double start, int *version, char *info, char *name)
{
  double d_time = d_mddir_file_info_v3(frme, radar_num, start,
				       (int)DD_TIME_NEAREST,
				       (int)DD_LATEST_VERSION,
				       version, info, name);

  if (start < d_time)
  {
    // Starts after the requested start time

    d_time = d_mddir_file_info_v3(frme, radar_num, d_time,
				  (int)DD_TIME_BEFORE, (int)DD_LATEST_VERSION,
				  version, info, name);
  }

  return d_time;
}

/* c------------------------------------------------------------------------ */

void 
dd_update_ray_info (DGI_PTR dgi)
{
    struct prev_rays *prs;
    struct sweepinfo_d *swib=dgi->dds->swib;
    struct ray_i *ryib=dgi->dds->ryib;
    struct platform_i *asib=dgi->dds->asib;
    double d;

    dgi->swp_que->num_rays++;
    prs = dgi->ray_que = dgi->ray_que->next;

    prs->clip_gate = dgi->clip_gate;
    dgi->swp_que->end_time = prs->time = dgi->time;
    prs->rotation_angle = swib->stop_angle = dd_rotation_angle(dgi);
    prs->source_sweep_num = ryib->sweep_num;
}
/* c------------------------------------------------------------------------ */

void 
dd_update_sweep_info (DGI_PTR dgi)
{
    struct prev_swps *pss;
    DD_TIME *dts=dgi->dds->dts;

    dgi->sweep_time_stamp = (int32_t) dgi->time;
    dgi->sweep_count++;
    d_unstamp_time(dts);
    pss = dgi->swp_que = dgi->swp_que->next;
    pss->source_vol_num = dgi->source_vol_num;
    pss->source_sweep_num = dgi->source_sweep_num;
    pss->segment_num = dts->millisecond;
    pss->listed = NO;
    pss->new_vol = dgi->new_vol;
    pss->start_time = dgi->time;
    pss->sweep_time_stamp = 
      dgi->sweep_time_stamp = (int32_t) dgi->time;
    
    pss->num_rays = 0;
    pss->sweep_file_size = sizeof(struct sweepinfo_d);
    pss->volume_time_stamp = dgi->volume_time_stamp;
    pss->num_parms = dgi->source_num_parms;
    pss->scan_mode = dgi->dds->radd->scan_mode;
}
/* c------------------------------------------------------------------------ */

int 
ddswp_last_ray (struct unique_sweepfile_info_v3 *usi)
{
    return(usi->ray_num >= usi->num_rays);
}
/* c------------------------------------------------------------------------ */

int 
ddswp_new_sweep_v3 (struct unique_sweepfile_info_v3 *usi)
{
    int ii, nn, ierr=0;
    char str[256];
    struct dd_general_info *dgi;
    struct dd_file_name_v3 *ddfn;
    struct dd_input_sweepfiles_v3 *dis;



    if(usi->swp_count >= usi->num_swps) {
	usi->forget_it = YES;
	return(END_OF_TIME_SPAN);
    }
    /* open sweep file and read in all the headers
     */
    ddfn = *(usi->final_swp_list +usi->swp_count++);
    char *char_filename = new char[usi->filename.size()+100];
    strcpy(char_filename, usi->filename.c_str());
    ddfn_file_name(ddfn, char_filename);
    usi->filename = char_filename;
    delete [] char_filename;
    
    dgi = dd_window_dgi(usi->radar_num);

    if(*ddfn->comment){		/* save the qualifier */
      dgi->file_qualifier = ddfn->comment;
    }
    else
    { dgi->file_qualifier = ""; }

    dis = dd_return_sweepfile_structs_v3();
    if(dis->editing) {
	if(usi->new_version_flag) {
	    dgi->version = ddfn->version+1;
	}
	else {
	    dgi->version = ddfn->version;
	}
    }

    if(dgi->in_swp_fid > 0)
	  close(dgi->in_swp_fid);

    slash_path(str, usi->directory.c_str());
    strcat(str, usi->filename.c_str());
    dgi->orig_sweep_file_name = usi->filename;

    if((dgi->in_swp_fid = open(str, 0)) < 1 ) {
	ierr = YES;
	printf("Could not open file %s in dap_new_swp status: %d\n"
	       , str, dgi->in_swp_fid);
	usi->forget_it = YES;
	return(END_OF_TIME_SPAN);
    }
    if (dd_absorb_header_info(dgi) != 0) {
        printf("Could not read header from sweep file '%s'\n"
               , dgi->sweep_file_name.c_str());
        dgi->source_num_rays = 0;
    }
    usi->num_rays = dgi->source_num_rays;
    dgi->swp_que->segment_num = ddfn->milliseconds;

    return(0);
}
/* c------------------------------------------------------------------------ */

struct cfac_wrap *
ddswp_nab_cfacs (const char *radar_name)
{
   struct correction_d cfacx, *cfac;
   struct cfac_wrap *cfw = 0, *first_cfac = 0;
   char *aa, *bb, *cc=0, *ee = 0;
   int kk, mark;
   struct dd_input_sweepfiles_v3 *dis;
  
   dis = dd_return_sweepfile_structs_v3();
   
   for (kk=0; kk < dis->num_cfac_sets; kk++) {
      cfw = dis->cfac_wrap[kk];
      if (strcmp (radar_name, cfw->radar_name) == 0) {
	 first_cfac = cfw;
	 return (first_cfac);
      }
   }
   
   cfac = &cfacx;
   memset (cfac, 0, sizeof (*cfac));
   strncpy( cfac->correction_des, "CFAC", 4 );
   cfac->correction_des_length = sizeof(*cfac);

   aa = getenv ("CFAC_FILES");
   bb = getenv ("CFAC_FILE");

   if (!(aa || bb)) {
     if (aa=getenv("ELD_CFAC_FILES")) {
       first_cfac = dd_absorb_cfacs (aa, radar_name);
       dis->cfac_wrap[dis->num_cfac_sets++] = first_cfac;
       return (first_cfac);
     }
     return (first_cfac);
   }

   aa = (aa) ? aa : bb;

   dd_absorb_cfac(aa, radar_name, cfac);
   cfw = first_cfac = (struct cfac_wrap *)malloc(sizeof(*cfw));
   cfw->ok_frib = NO;
   cfw->next = 0;
   cfw->cfac = (struct correction_d *)malloc( sizeof( *cfac ));
   memcpy( cfw->cfac, cfac, sizeof( *cfac ));
   strcpy (cfw->frib_file_name, "default");
   strcpy (cfw->radar_name, radar_name);
   dis->cfac_wrap[dis->num_cfac_sets++] = first_cfac;

   return (first_cfac);
}

/* c------------------------------------------------------------------------ */

int 
ddswp_next_ray_v3 (struct unique_sweepfile_info_v3 *usi)
{
    /*
     * Keep sucking up input until all headers (if applicable )
     * and a complete ray have been read in.
     */
    int ii, nn, mark;
    static int count=0, doodah=33;
    struct dd_general_info *dgi;
    struct dd_stats *dd_stats;

    /*
     * absorb the next ray
     */
    count++;
    if( count > doodah ) {
	mark = 0;
    }
    dd_stats = dd_return_stats_ptr();
    dgi = dd_window_dgi(usi->radar_num);
    dgi->dds->first_cfac = usi->first_cfac;

    if(usi->ray_num >= usi->num_rays) {	/* get the next file name
					 * and open it */

	if(ddswp_new_sweep_v3(usi) == END_OF_TIME_SPAN) {
	    return(END_OF_TIME_SPAN);
	}

	usi->ray_num = 0;
	if(usi->vol_num != dgi->source_vol_num) {
	    usi->vol_num = dgi->source_vol_num;
	    /*
	    printf("New_volume!\n");
	     */
	}
    }	
    if((nn = dd_absorb_ray_info(dgi)) <= 0) {
	return(END_OF_TIME_SPAN);
    }
    usi->ray_num++;
    ddswp_stuff_ray(dgi);
    return(1);
}
/* c------------------------------------------------------------------------ */

void 
ddswp_stuff_ray (DGI_PTR dgi)
{
    /*
     * this routine assumes that there is a ray of data to be
     * dealt with
     *
     */
    int mark;
    static int count=0, trip=123;
    double d;

    struct dd_input_filters *difs;
    DDS_PTR dds=dgi->dds;
    struct radar_d *radd=dds->radd;
    struct sweepinfo_d *swib=dds->swib;
    struct ray_i *ryib=dds->ryib;
    struct platform_i *asib=dds->asib;

    count++;
    difs = dd_return_difs_ptr();
    if(count >= trip) {
	mark=0;
    }

    if(dgi->new_sweep) {
	ddswp_stuff_sweep(dgi);
	if(dgi->swp_que->scan_mode != dgi->swp_que->last->scan_mode) {
	   dgi->new_vol = YES;
	}
    }
    if(dgi->new_vol) {
	if( dgi->num_parms != radd->num_parameter_des ) {
	    /* Complain! */
	}
	dgi->vol_count++;
    }
    ryib->sweep_num = swib->sweep_num;

    dd_update_ray_info(dgi);
}
/* c------------------------------------------------------------------------ */

void 
ddswp_stuff_sweep (DGI_PTR dgi)
{
    DDS_PTR dds=dgi->dds;
    struct sweepinfo_d *swib=dds->swib;
    

    dd_update_sweep_info(dgi);
    /* sweep info block */
    swib->num_rays = 0;
    swib->start_angle = dd_rotation_angle(dgi);
    swib->stop_angle = (float)EMPTY_FLAG;
    swib->filter_flag = NO_FILTERING;
    strncpy((char *)swib->radar_name, (char *)dds->radd->radar_name, 8);
}
/* c------------------------------------------------------------------------ */


/* c------------------------------------------------------------------------ */

/*
 * create the dynamic angle table by reading once through the rays
 */

int dd_create_dynamic_rotang_info(DGI_PTR dgi)
{
  
  int nrays;
  int angle_ndx_size = 480;
  int ii, mm, size;
  int a_offset;
  int32_t *ang_index;
  struct rot_ang_table *rat;
  struct rot_table_entry *entry;

  int32_t start_offset;
  
  /* any rays? */

  nrays = dgi->source_num_rays;
  if (nrays == 0) {
    return -1;
  }

//  if (!_note_printed) {
//    fprintf(stderr, "NOTE - dd_create_dynamic_rotang_info\n");
//    fprintf(stderr, "  No rotation table found\n");
//    fprintf(stderr, "  Creating dynamic table on-the-fly instead\n");
//    fprintf(stderr, "  This warning will only be printed once\n");
//    _note_printed = 1;
//  }

  /* save the current values for disk offset etc */
  
  start_offset = lseek(dgi->in_swp_fid, 0L, SEEK_CUR);

  /* allocate read buffer, set up related variables */
  
  if (!_in_buf) {
    _in_buf = (char *) malloc(MAX_READ);
  }
  memset(_in_buf, 0, MAX_READ);
  _buf_bytes_left = 0;
  _file_byte_count = 0;
  _in_next_block = _in_buf;
  _disk_offset_read_start = start_offset;

  /* rewind to start of file */

  lseek(dgi->in_swp_fid, 0L, SEEK_SET);
  
  /* free up rotation angle table */
  
  if(dgi->source_rat) {
    free(dgi->source_rat);
  }
  
  /* compute size of rat */
  
  size = sizeof(struct rot_ang_table);
  size = ((size -1)/8 +1) * 8; /* start tables on an 8 byte boundary */
  mm = size;
  size += (angle_ndx_size * sizeof(int32_t) +
           nrays * sizeof(struct rot_table_entry));

  /* alloc rat */
  
  dgi->source_rat = (struct rot_ang_table *) malloc(size);
  rat = dgi->source_rat;
  if (rat == NULL) {
    fprintf(stderr, "Malloc error - quitting\n");
    exit(1);
  }
  memset(rat, 0, size);
  
  /* fill out main table */
  
  memcpy(rat->name_struct, "RKTB", 4);
  rat->sizeof_struct = size;
  rat->angle2ndx = (float)angle_ndx_size/360.0;
  rat->ndx_que_size = angle_ndx_size;
  rat->first_key_offset = mm + angle_ndx_size * sizeof(int32_t);
  rat->angle_table_offset = mm;
  rat->num_rays = nrays;

  /* read through rays, filling in the rotation angle entries */
  
  for (ii = 0; ii < nrays; ii++) {
    if (dd_read_ray_for_dynamic_angle_table(dgi, ii) < 0) {
      /* restore to starting state */
      lseek(dgi->in_swp_fid, start_offset, SEEK_SET);
      return -1;
    }
  }
  
  /* now compute the angle indexes */

  a_offset = rat->angle_table_offset;
  ang_index = (int32_t *) ((char *) rat + a_offset);
  for (ii = 0; ii < angle_ndx_size; ii++) {
    ang_index[ii] = -1;
  }
  entry = (struct rot_table_entry *)
    ((char *) rat + rat->first_key_offset);
  for (ii = 0; ii < nrays; ii++, entry++) {
    double rotAngle = entry->rotation_angle;
    int jj = (int) (rotAngle * rat->angle2ndx);
    if (jj < 0 || jj > angle_ndx_size - 1) {
      jj = 0;
    }
    ang_index[jj] = ii;
  }
  
#ifdef DEBUG_VERBOSE
  fprintf(stderr, "######## Rotation table computed on the fly ######\n");
  dd_print_angle_table(rat, stderr);
  fprintf(stderr, "##################################################\n");
#endif

  /* restore to starting state */

  lseek(dgi->in_swp_fid, start_offset, SEEK_SET);

  return 0;

}

/* c------------------------------------------------------------------------ */

int dd_read_ray_for_dynamic_angle_table(DGI_PTR dgi, int rayNum)
                                        
{

  /*
   * Read in rays for computing the dynamic angle table on the fly
   */
  
  int len, ryib_flag=NO, num_rdats=0, mark;
  int num_cells, bc=0, pn, ncopy, stored_len;
  int gottaSwap=NO, ds;
  static int count=0, doodah=110, rec_count=0, loop_count = 0;
  static int rec_trip=12, disk_offset= -1;
  struct qparamdata_d *qdat;
  struct generic_descriptor *gd;
  DDS_PTR dds=dgi->dds;
  u_int32_t gdsos;

  // get rotation table entry for this ray num

  struct rot_ang_table *drat = dgi->source_rat;
  int doffset = drat->first_key_offset +
    rayNum * sizeof(struct rot_table_entry);
  struct rot_table_entry *rotEntry = (struct rot_table_entry *)
    ((char *) drat + doffset);

  count++;
  if( count >= doodah ) {
    mark = SWAP_CHECK_REC_SIZE;
  }

  while(1) {

    if( ++loop_count >= doodah ) {
      mark = SWAP_CHECK_REC_SIZE;
    }
    gd = (struct generic_descriptor *)_in_next_block;
    
    if(_buf_bytes_left >= sizeof(struct generic_descriptor)) {
      gdsos = (u_int32_t)gd->sizeof_struct;
      if(gdsos > SWAP_CHECK_REC_SIZE) {
        gottaSwap = YES;
        swack4((char *)&gd->sizeof_struct, (char *)&gdsos);
      }
    }
    else { gdsos = 0; }
    
    if( num_rdats == dgi->source_num_parms ) {
      break;
    }

    /* refresh the read buffer as needed */

    if(_buf_bytes_left <= 0 ||
       sizeof(struct generic_descriptor) > _buf_bytes_left ||
       gdsos > _buf_bytes_left ) {

      /* move to top of buffer and refill it */

      rec_count++;
      if(rec_count >= rec_trip) {
        mark = 0;
      }
      if(_buf_bytes_left > 0 )
        memcpy(_in_buf, _in_next_block
               , _buf_bytes_left);
      else if( _buf_bytes_left < 0 ) {
        if(!dd_solo_flag()) { return -1; }
      }

      _in_next_block = _in_buf;
      disk_offset = lseek(dgi->in_swp_fid, 0L, SEEK_CUR); 
      _disk_offset_read_start = disk_offset - _buf_bytes_left;
      
      if((len=read(dgi->in_swp_fid, _in_buf+_buf_bytes_left
                   , MAX_READ -_buf_bytes_left )) < 1 ) {
        printf("Trying to read past end of data...stat= %d loop: %d\n"
               , len, loop_count );
        if(!dd_solo_flag()) { return -1; }
      } else if((_buf_bytes_left+=len) < gdsos ) {
        printf("File appears to be truncated...stat= %d loop: %d\n"
               , len, loop_count );
        return -1;
      }
      
      gd = (struct generic_descriptor *)_in_next_block;
      gdsos = (u_int32_t)gd->sizeof_struct;
      
      if(gdsos > SWAP_CHECK_REC_SIZE) {
        gottaSwap = YES;
        swack4((char *)&gd->sizeof_struct, (char *)&gdsos);
      }

    } /* done refreshing read buffer */

    if(gdsos < sizeof(struct generic_descriptor) || gdsos > SWAP_CHECK_REC_SIZE) {
      /* This is probably a garbaged file */
      fprintf(stderr,
              "Sweep file containing time %s is garbage! loop: %d\n"
              , dts_print(d_unstamp_time(dgi->dds->dts)), loop_count);
      return -1;
    }
    
    if(strncmp(_in_next_block,"NULL",4) == 0 ||
       strncmp(_in_next_block,"RKTB",4) == 0) {
      break;
    }
      
    if(strncmp(_in_next_block,"RDAT",4) == 0 ||
       strncmp(_in_next_block,"QDAT",4) == 0) {
      
      /* field data block */

      qdat = (struct qparamdata_d *)_in_next_block;
      pn = num_rdats++;
      ncopy = *_in_next_block == 'R'
        ? sizeof(struct paramdata_d) : dds->parm[pn]->offset_to_data;
      ds = dd_datum_size((int)dds->parm[pn]->binary_format);
      if(gottaSwap) {
        ddin_crack_qdat(_in_next_block, (char *)dds->qdat[pn], (int)0);
      }
      else {
        memcpy((char *)dds->qdat[pn], _in_next_block, ncopy);
      }
      stored_len = dds->qdat[pn]->pdata_length;
      
      /* add to length of ray data in rotation table */

      // struct rot_ang_table *drat = dgi->source_rat;
      // int doffset = drat->first_key_offset +
      //   rayNum * sizeof(struct rot_table_entry);
      // struct rot_table_entry *entry = (struct rot_table_entry *)
      //   ((char *) drat + doffset);
      rotEntry->size += stored_len;

      if (ryib_flag) {
        // done
        // return 0;
      }

      bc += gdsos;

    } else if (strncmp(_in_next_block,"RYIB",4)==0 && !ryib_flag) {

      /* ray header block */
      
      if(gottaSwap) {
        ddin_crack_ryib(_in_next_block, (char *)dds->ryib, (int)0);
      }
      else {
        memcpy((char *)dds->ryib, _in_next_block
               , sizeof(struct ray_i));
      }
      ryib_flag = YES;
      bc += gdsos;
      num_cells = dds->celv->number_cells;

      /* fill out rotation entry */

      char *ray_offset = _in_next_block;
      // struct rot_ang_table *drat = dgi->source_rat;
      // int doffset = drat->first_key_offset +
      //   rayNum * sizeof(struct rot_table_entry);
      // struct rot_table_entry *entry = (struct rot_table_entry *)
      //   ((char *) drat + doffset);
      rotEntry->rotation_angle = dd_rotation_angle(dgi);
      rotEntry->offset = (ray_offset - _in_buf) +
        _disk_offset_read_start;
      rotEntry->size = dds->ryib->ray_info_length;

    } else if(strncmp(_in_next_block,"ASIB",4)==0) {

      /* platform block */

      if(gottaSwap) {
        ddin_crack_asib(_in_next_block, (char *)dds->asib, (int)0);
      } else {
        memcpy((char *)dds->asib, _in_next_block
               , sizeof(struct platform_i));
      }
      bc += gdsos;

      /* add to length of ray data in rotation table
       * also update rotation angle for moving platforms */

      // struct rot_ang_table *drat = dgi->source_rat;
      // int doffset = drat->first_key_offset +
      //   rayNum * sizeof(struct rot_table_entry);
      // struct rot_table_entry *entry = (struct rot_table_entry *)
      //   ((char *) drat + doffset);
      rotEntry->rotation_angle = dd_rotation_angle(dgi);
      rotEntry->size += dds->asib->platform_info_length;
      
    } else {
      /* ignore this bock */
      mark = 0;
      bc += gdsos;
    }
    
    _buf_bytes_left -= gdsos;
    _file_byte_count += gdsos;
    _in_next_block += gdsos;

  } /* while */

  return bc;

}

/* c------------------------------------------------------------------------ */

void dd_print_angle_table(const struct rot_ang_table *rat, FILE *out)
{

  int ii;

  fprintf(out, "============== Rotation angle table ===================\n");
  fprintf(out, "\n");

  fprintf(out,
          "  name_struct: %.4s\n"
          "  sizeof_struct: %d\n"
          "  angle2ndx: %.3f\n"
          "  ndx_que_size: %d\n"
          "  first_key_offset: %d\n"
          "  angle_table_offset: %d\n"
          "  num_rays: %d\n",
          rat->name_struct,
          rat->sizeof_struct,
          rat->angle2ndx,
          rat->ndx_que_size,
          rat->first_key_offset,
          rat->angle_table_offset,
          rat->num_rays);
      
  const struct rot_table_entry *rte = (const struct rot_table_entry *)
    ((const char *)rat + rat->first_key_offset);

  fprintf(out, "\n");
  fprintf(out, "--- Ray angle table ---\n");
  for(ii=0; ii < rat->num_rays; ii++, rte++) {
    fprintf(out, "    RayNum, angle, diskOffset, nBytes: %4d %7.2f %8d %7d\n",
            ii, rte->rotation_angle,
            rte->offset, rte->size);
  }
  
  fprintf(out, "--- Angle index table ---\n");
  const int32_t *indexes = (int32_t *)
    ((const char *)rat + rat->angle_table_offset);
  for (ii = 0; ii < rat->ndx_que_size; ii++) {
    fprintf(out, "    Index[%d]: %d\n", ii, indexes[ii]);
  }
  
  fprintf(out, "\n");
  fprintf(out, "=======================================================\n");

}

