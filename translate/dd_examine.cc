/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */
/* c------------------------------------------------------------------------ */

# define MAX_DESC_LEN 5
# define FBIO_EXCEED 100
# define DDX_MAX_BLOCKS 4

#include <LittleEndian.hh>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "dd_files.hh"
#include "dd_examine.hh"
#include "dorade_share.hh"
#include "dd_io_mgmt.hh"
#include "dd_crackers.hh"
#include "ddb_common.hh"


# ifndef K64
# define K64 65536
# endif
# define UPPER(c) ((c) >= 'A' && (c) <= 'Z')

# ifdef sparc
int Sparc_Arch=YES;
# else
int Sparc_Arch=NO;
# endif

static char dname[8]={0,0,0,0,0,0,0,0};
static int the_fmt = WSR_88D_FMT;
static int view_char=0, view_char_count=200;
static char desc_ids[1024];
static char *pbuf=NULL;
static int debugMode=NO, gottaSwap=NO;
static char *swapBuf=NULL;

static struct solo_list_mgmt *slm;
static char preamble[24], postamble[64];

/* c------------------------------------------------------------------------ */

void 
solo_message (const char *message)
{
    printf("%s", message);
}
/* c------------------------------------------------------------------------ */

int 
main (int argc, char *argv[])
{
    int ii, jj, mark, direction;
    char *aa, *b, *c, *sdev;
    struct input_read_info *iri;
    double d;

    int nn, ival, *iptr;
    float val;
    char str[256];
    DD_TIME dts;

    iptr = (int *)str;
    *iptr = 1;

    if(argc > 1)
	  sdev = argv[1];		/* first arg: dev name */
    else if(!(sdev=getenv("SOURCE_DEV"))) {
	printf("No source dev! Either setenv SOURCE_DEV to the device name ");
        printf("or type ddex dev_name\n");
	exit(0);
    }
    swapBuf = (char *)malloc(K64);
    memset(swapBuf, 0, (int)K64);

    if(aa = getenv("INPUT_FORMAT")) {
	if(strstr(aa, "WSR_88D_FORMAT")) {
	    the_fmt = WSR_88D_FMT; 
	}
	else if(strstr(aa, "UF_FORMAT")) {
	    the_fmt = UF_FMT;
	}
	else if(strstr(aa, "HRD_FORMAT")) {
	    the_fmt = HRD_FMT;
	}
	else if(strstr(aa, "SIGMET_FORMAT")) {
	    the_fmt = SIGMET_FMT;
	}
	else if(strstr(aa, "DORADE_FORMAT")) {
	    the_fmt = DORADE_FMT;
	}
    }
    if(aa = getenv("OPTIONS")) {
	if(strstr(aa, "DEBUG")) {
	    debugMode = YES; 
	}
     }
    strcpy(desc_ids, DD_ID_LIST);
    strcat(desc_ids, ",SPAL,SPTL,SSFI,SPMI,SCPM,SWVI,PISP,SEDS,");
    strcat(desc_ids, ",RKTB,");	/* rotation angle table */
    strcat(desc_ids, ",NDDS,SITU,ISIT,TIME,INDF,MINI,LDAT,LIDR,FLIB,");
    strcat(desc_ids, ",CSFD,XSTF");
    if(aa = getenv("MORE_DESCRIPTORS")) {
	strcat(desc_ids, aa);
    }
    slm = solo_malloc_list_mgmt(87);
    /*
     * set the io utilities
     */
    iri = dd_return_ios(0, the_fmt);

    dd_input_read_open(iri, sdev);
    /*
     * read in a bunch of records so you can figure out what you got
     */
    for(ii=0; ii < iri->que_count; ii++) {
	dd_logical_read(iri, FORWARD);
    }
    dd_skip_recs(iri, BACKWARD, iri->que_count);

    printf("\n\
You need to read at least one record before entering the\n\
Inventory option. You can also give a negative skip\n\
value. Hitting <return> is equivalent to entering a 0\n\
\n"
	   );

 menu1:
    printf("\n\
-1 = exit program (-2 exits from any other prompt)\n\
 0 = continue\n\
Option = "
	   );
    nn = getreply(str, sizeof(str));
    if( cdcode(str, nn, &ival, &val) != 1 || ival < -2 || ival > 1 ) {
	printf( "\nIllegal Option!\n" );
	goto menu1;
    }
   if(ival < 0)
	  exit(0);
    else if(ival == 0) {

    menu2:
	printf("\n\
 0 = Skip records and read a record or 64K bytes for binary\n\
 1 = Inventory\n\
 2 = Skip files \n\
 3 = Rewind\n\
 4 = Unwind\n\
 5 = Move backwards 1 rec. (no read)\n\
Option = "
	       );
	nn = getreply(str, sizeof(str));
	if( cdcode(str, nn, &ival, &val) != 1 || ival < -2 || ival > 8 ) {
	    if(ival == -2) exit(1);
	    printf( "\nIllegal Option!\n" );
	    goto menu2;
	}
	if(ival < 0)
	      exit(0);
	else if(ival == 0) {
	    printf("Type skip or hit <return> to read next rec:");
	    nn = getreply(str, sizeof(str));
	    if(cdcode(str, nn, &ival, &val) != 1 || fabs((double)val) > 64*K64) {
		printf( "\nIllegal Option!\n" );
		goto menu2;
	    }
	    if(ival != 0) {
		direction = ival >= 0 ? FORWARD : BACKWARD;
		dd_skip_recs(iri, direction, ival > 0 ? ival : -ival);
		if(!iri->read_count)
		      dd_phys_read(iri);
	    }
	    else
		  dd_logical_read(iri, (int)FORWARD);
	    nn = iri->top->sizeof_read;
	    printf("\n Read %d bytes\n", nn);
	    nn = nn < 400 ? nn : 400;
	    slm_ezhxdmp(slm, iri->top->buf, 0, nn);
# ifdef obsolete
	    ezascdmp(iri->top->buf, 0, nn);
# endif
	    printf("\n");
	    nn = nn < 200 ? nn : 200;
	    ctypeu16((unsigned char *)iri->top->buf, 0, nn);  //Jul 26, 2011
# ifdef obsolete
	    printf("\n");
	    ezhxdmp(iri->top->buf, 0, nn);
# endif
	}
	else if(ival == 1) {
	    ddx_inventory(iri);
	}
	else if(ival == 2) {
	    printf("Type skip: ");
	    nn = getreply(str, sizeof(str));
	    if(cdcode(str, nn, &ival, &val) != 1 || fabs((double)val) > K64) {
		printf( "\nIllegal Option!\n" );
		goto menu2;
	    }
	    if(ival) {
		direction = ival >= 0 ? FORWARD : BACKWARD;
		dd_skip_files(iri, direction, ival >= 0 ? ival : -ival);
	    }
	}
	else if(ival == 3) {
	    dd_rewind_dev(iri);
	}
	else if(ival == 4) {
	    dd_unwind_dev(iri);
	}
	else if(ival == 5) {
	    dd_really_skip_recs(iri, (int)BACKWARD, (int)1);
	}
	else if(ival == 6) {
	    /* move forward in time
	     */
	}
	else if(ival == 7) {
	}
	else if(ival == 8) {
	}
	goto menu2;
    }
    else {

    menu3:
	printf("\n\
 0 = No format\n\
 1 = DORADE_FMT\n\
 2 = UF_FMT\n\
 3 = HRD_FMT\n\
 4 = WSR_88D_FMT\n\
 5 = SIGMET_FMT\n\
 6 = ELDORA_FMT\n\
Option = "
	       );
	nn = getreply(str, sizeof(str));
	if( cdcode(str, nn, &ival, &val) != 1 || ival < -2 || ival > 6 ) {
	    printf( "\nIllegal Option!\n" );
	    goto menu3;
	}
	if(ival < 0)
	      exit(0);
	else if(ival == 0) {
	    the_fmt = 0;
	}
	else if(ival == 1) {
	    the_fmt = DORADE_FMT;
	}
	else if(ival == 2) {
	    the_fmt = UF_FMT;
	}
	else if(ival == 3) {
	    the_fmt = HRD_FMT;
	}
	else if(ival == 4) {
	    the_fmt = WSR_88D_FMT;
	}
	else if(ival == 5) {
	    the_fmt = SIGMET_FMT;
	}
	else if(ival == 6) {
	    the_fmt = ELDORA_FMT;
	}
	goto menu1;
    }
}
/* c------------------------------------------------------------------------ */

int 
ddx_options (void)
{
}
/* c------------------------------------------------------------------------ */

void 
ddx_inventory (struct input_read_info *iri)
{
    /* the purpose of this routine is to facilitate a more detailed
     * examination of the data
     */
    int nn, ival;
    float val;
    char str[256];
    double d;

 menu1:
    printf("\n\
 0 = return to previous menu\n\
 1 = loop through DORADE descriptors\n\
 2 = 16-bit integers\n\
 3 = hex display\n\
 4 = dump as characters\n\
 5 = backwards loop through DORADE descriptors\n\
Option = "
	       );
    nn = getreply(str, sizeof(str));
    if( cdcode(str, nn, &ival, &val) != 1 || ival < -2 || ival > 8 ) {
	printf( "\nIllegal Option!\n" );
	goto menu1;
    }
    if(ival <= 0) {
	if(ival < -1)
	      exit(1);
	return;
    }
    else if(ival == 0) {
    }
    else if(ival == 1) {
       ddx_dorade_fwd_desc_loop(iri->top->buf, 0, iri->top->sizeof_read
				, iri->top->offset
				, iri->sizeof_file);
    }
    else if(ival == 2) {
	if(ddx_start_stop_chars() >= 0) {
	    printf("\n");
# ifdef obsolete	    
	    ctypeu16(iri->top->buf, view_char, view_char_count);
# else
	    slm_ctypeu16(slm, (unsigned char*)iri->top->buf, view_char, view_char_count);  //Jul 26, 2011
# endif
	}
    }
    else if(ival == 3) {
	if(ddx_start_stop_chars() >= 0) {
	    printf("\n");
	    slm_ezhxdmp(slm, iri->top->buf, view_char, view_char_count);
	}
    }
    else if(ival == 4) {
	if(ddx_start_stop_chars() >= 0) {
	    printf("\n");
	    slm_ezascdmp(slm, iri->top->buf, view_char, view_char_count);
	}
    }
    else if(ival == 5) {
	ddx_dorade_bkwd_desc_loop(iri->top->buf, 0, iri->top->sizeof_read
			     , iri->top->offset
			     , iri->sizeof_file);
    }
    else if(ival == 6) {
	if(ddx_start_stop_chars() >= 0) {
	    printf("\n");
	    slm_xctypeu16(slm, (unsigned char*)iri->top->buf, view_char, view_char_count);  //Jul 26, 2011
	}
    }
    else if(ival == 7) {
    }
    else if(ival == 0) {
    }
    goto menu1;
}
/* c------------------------------------------------------------------------ */

int 
ddx_dorade_bkwd_desc_loop (char *aa, int ii, int jj, int32_t offset, int32_t sizeof_file)
{
    int kk, mm, mark, bad_vibs, skip=0, loop=1, bonk=NO;

    int nn, ival;
    float val;
    char str[256], *bb;
    double d;
    static int trip=22000;
    u_int32_t gdsos=0;

    struct generic_descriptor {
	char name_struct[4];	/* "????" */
	int32_t sizeof_struct;
    };
    typedef struct generic_descriptor *GDPTR;
    GDPTR gd;


    
    for(;;) {
	for(; loop--;) {
	    kk = ddx_desc_bkwd_sync(aa, ii, jj);

	    if(kk < 0) {
		return(1);
	    }
	    if(kk & 3) {
		printf("Out of sync on %s at %d\n"
		       , aa+kk, kk);
		return(-1);
	    }
	    gd = (GDPTR)(aa+kk);
	    gdsos = gd->sizeof_struct;
	    if(!debugMode && gdsos > MAX_REC_SIZE) {
	       swack4((char *)&gd->sizeof_struct, (char *)&gdsos);
	       gottaSwap = YES;
	    }

	    if(bonk = (gdsos < 8)) {
		printf("Bad size of struct: %d\n", gdsos);
		break;
	    }
	    bb = hostIsLittleEndian() ? dname : aa+kk;
	    bb = dname;

	    bad_vibs = !strstr(desc_ids, bb);
	    bad_vibs |= gdsos < 8;
	    bad_vibs |= gdsos > MAX_REC_SIZE;
	    
	    if(bad_vibs) {
		printf("at offset: %d  size of struct: %d\n"
		       , kk, gdsos);
		ddx_bad_vibs(aa+kk);
		bonk = YES;
		break;
	    }
	    if(gdsos+kk > jj) {
		jj = kk;
		continue;	/* can print the whole descriptor */
	    }
	    printf("At byte: %d  offset: %d  sizeof file: %d\n"
		   , kk, offset, sizeof_file);
	    if(!skip--) {
		ddx_print_desc(aa+kk);
	    }

	    if((jj=kk) <= ii) {
		bonk = YES;
		break;
	    }
	}
	if(bonk)
	      break;
	printf("Hit <return> to continue (-1 = cease): ");
	nn = getreply(str, sizeof(str));
	if(cdcode(str, nn, &ival, &val) != 1 || ival < 0) {
	    if(ival == -2) exit(1);
	    return(0); //Jul 26, 2011 add return value - in fact, the returned value is not used
	}
	skip = ival > 0 ? ival : 0;
	loop = skip > 0 ? skip+1 : 1;
    }
}
/* c------------------------------------------------------------------------ */

int 
ddx_dorade_fwd_desc_loop (char *aa, int ii, int jj, int32_t offset, int32_t sizeof_file)
{
    int kk=0, mm=ii, mark, bad_vibs, skip=0, loop=1, bonk=NO;
    int gdlen=0, prev_gdsos=0, last_kk;

    int nn, ival;
    float val;
    char str[256], *bb;
    double d;
    static int trip=22000;
    u_int32_t gdsos;

    struct generic_descriptor {
	char name_struct[4];	/* "????" */
	int32_t sizeof_struct;
    };
    typedef struct generic_descriptor *GDPTR;
    GDPTR gd;


    for(;;) {
	for(; loop--;) {
	    if(ii >= jj) {
		bonk = YES;
		break;
	    }
	    kk = ddx_desc_fwd_sync(aa, mm, jj);

	    if(kk < 0 || jj-kk < 8) {
		return(1);
	    }
	    if(kk & 3) {
		printf("Out of sync on %s at %d\n"
		       , aa+kk, kk);
		return(-1);
	    }
	    gd = (GDPTR)(aa+kk);
	    gdsos = gd->sizeof_struct;
	    if(!debugMode && gdsos > MAX_REC_SIZE) {
	       swack4((char *)&gd->sizeof_struct, (char *)&gdsos);
	       gottaSwap = YES;
	    }

	    gdlen = kk-ii;
	    if(gdlen && gdlen != prev_gdsos) {
		printf(
 "\nBad size for last struct: %d  at offset: %d  possible size: %d\n\n"
		       , prev_gdsos, offset, gdlen);
	    }
	    prev_gdsos = gdsos;

	    if(bonk = (gdsos < 8)) {
		printf("Bad size of struct: %d  at offset: %d\n"
		       , gdsos, offset);
		break;
	    }

	    bb = hostIsLittleEndian() ? dname : aa+kk;
	    bb = dname;
	    bad_vibs = !strstr(desc_ids, bb);
	    
	    if(bad_vibs) {
		printf("at offset: %d  size of struct: %d\n"
		       , kk, gdsos);
		ddx_bad_vibs(aa+kk);
		bonk = YES;
		break;
	    }
	    printf("At byte: %d  offset: %d  sizeof file: %d\n"
		   , kk, offset, sizeof_file);
	    if(!skip--) {
		ddx_print_desc(aa+kk);
	    }
	    ii = kk;
#ifdef obsolete
	    mm = kk +4;
#else
	    /* avoid searching body of previously processed structures */
	    mm = kk +gdsos;
#endif
	}
	if(bonk)
	      break;
	printf("Hit <return> to continue (-1 = cease): ");
	nn = getreply(str, sizeof(str));
	if(cdcode(str, nn, &ival, &val) != 1 || ival < 0) {
	    if(ival == -2) exit(1);
	    return(0);  //Jul 26, 2011 add return value - in fact, the returned value is not used
	}
	skip = ival > 0 ? ival : 0;
	loop = skip > 0 ? skip+1 : 1;
    }
}
/* c------------------------------------------------------------------------ */

int 
ddx_start_stop_chars (void)
{
    int nn, ival;
    float val;
    char str[256];
    double d;

    printf("Type starting character number (=%d):"
	   , view_char+1);
    nn = getreply(str, sizeof(str));
    if(cdcode(str, nn, &ival, &val) != 1 || ival < 0 ||
       fabs((double)val) > 96 * 1024 ) {
	printf( "\nIllegal Option!\n" );
	return(-1);
    }
    if(ival) {
	view_char = ival-1;
	printf("Type character count (=%d):"
	       , view_char_count);
	nn = getreply(str, sizeof(str));
	if(cdcode(str, nn, &ival, &val) != 1 || ival < 0 ||
	   fabs((double)val) > 96 * 1024) {
	    printf( "\nIllegal Option!\n" );
	    return(-1);
	}
	if(ival) {
	    view_char_count = ival;
	}
    }
    return(1);
}
/* c------------------------------------------------------------------------ */

int 
ddx_desc_fwd_sync (char *aa, int ii, int jj)
{
    /* search for the next descriptor
     * from ii up to jj of aa
     */
    int cuc;

    if(1) {
       dname[4] = 0;
       for(cuc=0; ii < jj; ii++) {
	  if(UPPER((int)(*(aa+ii)))) {
	     if(++cuc == 4) {
		strncpy(dname, aa+ii-3, 4);
		if(strstr(desc_ids, dname)) { return(ii-3); }
		--cuc;
	     }
	  }
	  else { cuc=0; }
       }
    }
    else {
       for(; ii < jj;) {
	  /* find the next null byte counting consecutive UC
	   * characters along the way
	   */
	  for(cuc=0; ii < jj && *(aa+ii); ii++)
	    if(UPPER((int)*(aa+ii)))
	      cuc++;
	    else
	      cuc=0;

	  if(ii >= jj) break;
	  if(cuc > 3) {
	     /* is this a legitimate descriptor? */
	     if(strstr(desc_ids, aa+ii-4))
	       return(ii-4);
	  }
	  ii++;
       }
    }
    return(-1);
}
/* c------------------------------------------------------------------------ */

int 
ddx_desc_bkwd_sync (char *aa, int ii, int jj)
{
    /* search backwards for the next descriptor
     * from ii up to jj of aa
     */
    int kk, cuc;

    if(1) {
       dname[4] = 0;
       /* start in 4 characters since there must b 4 char of desc len
	*/
       for(cuc=0, jj-=5; ii <= jj; jj--) {
	  if(UPPER(*(aa+jj))) {
	     if(++cuc == 4) {
		strncpy(dname, aa+jj, 4);
		if(strstr(desc_ids, dname)) { return(jj); }
		--cuc;
	     }
	  }
	  else { cuc = 0; }
       }
    }
    else {
       for(jj--; ii < jj-3;) {
	  /* find the next null
	   */
	  for(; ii <= jj && *(aa+jj); jj--);
	  if( ii > jj) break;
	  /* count UC chars before the null
	   */
	  for(cuc=0,kk=0,jj--; ii <= jj-kk && kk < 4; kk++)
	    if(UPPER(*(aa+jj-kk)))
	      cuc++;
	    else
	      cuc=0;

	  if(ii > jj-kk) break;
	  if(cuc == 4) {
	     /* is this a legitimate descriptor? */
	     if(strstr(desc_ids, aa+jj-3))
	       return(jj-3);
	  }
       }
    }
    return(-1);
}
/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

char *
dd_print_seds (struct generic_descriptor *gd, char *str)
{
    char *a=str, *b,  c[16];
    int ii, nn;
    unsigned int gdsos = gd->sizeof_struct;

    if(gottaSwap) { swack4((char *)&gd->sizeof_struct, (char *)&gdsos); }

    sprintf(str, "\n\
name_struct %.4s\n\
sizeof_struct %d\n\
"
	    , gd->name_struct
	    , gdsos
	    );
    a += strlen(a);
    b = (char *)gd +sizeof(struct generic_descriptor);
    for(; *b; *a++ = *b++); *a++ = '\0';
}
/* c------------------------------------------------------------------------ */

int 
ddx_print_desc (char *a)
{
    char *bb;
    int ok_print = YES;

    if(!pbuf)
	  pbuf = (char *)malloc(BLOCK_SIZE);
    *pbuf = '\0';
    
    if(strncmp(a, "NULL", 4) == 0) {
	printf( "NULL descriptor!\n");
	return(0);  //Jul 26, 2011 add return value - in fact, the returned value is not used
    }		
    else if(strncmp(a, "ASIB", 4) == 0) {
//	dd_print_asib(a,pbuf);  //Jul 26, 2011
	dd_print_asib((platform_i *)a,pbuf);  //Jul 26, 2011
	ok_print = NO;
    }		
    else if(strncmp(a, "CELV", 4) == 0) {
	dd_print_celv((cell_d *)a,pbuf);
    }		
    else if(strncmp(a, "CFAC", 4) == 0) {
	dd_print_cfac((correction_d *)a,pbuf);
	ok_print = NO;
    }		
    else if(strncmp(a, "COMM", 4) == 0) {
	dd_print_comm((comment_d *)a,pbuf);
    }		
    else if(strncmp(a, "CSPD", 4) == 0) {
	dd_print_cspd((cell_spacing_d *)a,pbuf);
    }		
    else if(strncmp(a, "CSFD", 4) == 0) {
	dd_print_csfd((cell_spacing_fp_d *)a,pbuf);
    }		
    else if(strncmp(a, "FRAD", 4) == 0) {
	dd_print_frad((field_parameter_data *)a,pbuf);
    }
    else if(strncmp(a, "FRIB", 4) == 0) {
	dd_print_frib(a,pbuf);
    }
    else if(strncmp(a, "PARM", 4) == 0) {
	dd_print_parm((parameter_d *)a,pbuf);
	ok_print = NO;
    }		
    else if(strncmp(a, "RADD", 4) == 0) {
	dd_print_radd((radar_d *)a,pbuf);
	ok_print = NO;
    }		
    else if(strncmp(a, "RDAT", 4) == 0) {
	dd_print_rdat((paramdata_d *)a,pbuf);
    }		
    else if(strncmp(a, "RYIB", 4) == 0) {
	dd_print_ryib((ray_i *)a,pbuf);
	ok_print = NO;
    }		
    else if(strncmp(a, "SSWB", 4) == 0) {
	dd_print_sswb((super_SWIB *)a,pbuf);
	ok_print = NO;
    }		
    else if(strncmp(a, "SWIB", 4) == 0) {
	dd_print_swib((sweepinfo_d *)a,pbuf);
	ok_print = NO;
    }		
    else if(strncmp(a, "VOLD", 4) == 0) {
	dd_print_vold((volume_d *)a,pbuf);
	ok_print = NO;
    }		
    else if(strncmp(a, "WAVE", 4) == 0) {
	dd_print_wave((waveform_d *)a,pbuf);
    }		
    else if(strncmp(a, "RKTB", 4) == 0) {
	dd_print_rktb((struct rot_ang_table *)a,pbuf);
    }		
    else if(strncmp(a, "NDDS", 4) == 0) {
	dd_print_ndds((nav_descript *)a,pbuf);
    }		
    else if(strncmp(a, "SITU", 4) == 0) {
	dd_print_situ((insitu_descript *)a,pbuf);
    }		
    else if(strncmp(a, "ISIT", 4) == 0) {
	dd_print_isit((insitu_data *)a,pbuf);
    }		
    else if(strncmp(a, "TIME", 4) == 0) {
	dd_print_time((time_series *)a,pbuf);
	ok_print = NO;
    }		
    else if(strncmp(a, "SEDS", 4) == 0) {
	dd_print_seds((struct generic_descriptor *)a,pbuf);
    }		
    else {
	dd_print_gnrc((struct generic_descriptor *)a,pbuf);
	ok_print = NO;
    }		

    if(ok_print)
	  printf("%s\n", pbuf);
}
/* c------------------------------------------------------------------------ */

char *
dd_print_gnrc (struct generic_descriptor *gd, char *str)
{
    char *a=str, *b,  c[16];
    int ii, nn;
    unsigned int gdsos = gd->sizeof_struct;

    if(gottaSwap) { swack4((char *)&gd->sizeof_struct, (char *)&gdsos); }

    printf("\n\
name_struct %.4s\n\
sizeof_struct %d\n\
"
	    , gd->name_struct
	    , gdsos
	    );
    nn = gdsos > 480 ? 480 : gdsos;
    ezascdmp(gd->name_struct, 0, nn);
    printf("\n");
    ctypeu16((unsigned char *)gd->name_struct, 0, nn);  //Jul 26, 2011
    printf("\n");
    ezhxdmp(gd->name_struct, 0, nn);
}
/* c------------------------------------------------------------------------ */

char *
dd_print_ndds (struct nav_descript *ndds, char *str)
{
   if(gottaSwap) {
      
   }

    sprintf(str, "Contents of the nav descriptor  len: %d\n"
	    , sizeof(struct nav_descript));
    sprintf(str+strlen(str), "\n\
nav_descript_id[4] %.4s \n\
nav_descript_len %d\n\
ins_flag %d\n\
gps_flag %d\n\
minirims_flag %d\n\
kalman_flag %d\n\
"
	    , ndds->nav_descript_id
	    , ndds->nav_descript_len
	    , ndds->ins_flag
	    , ndds->gps_flag
	    , ndds->minirims_flag
	    , ndds->kalman_flag
	    );
    return(str);
}
/* c------------------------------------------------------------------------ */

char *
dd_print_situ (struct insitu_descript *situ, char *str)
{
    int ii, nn;
    char name[16], units[16];
    struct insitu_parameter *isp;

    sprintf(str, "Contents of the insitu descriptor  len: %d\n"
	    , sizeof(struct insitu_descript));
    sprintf(str+strlen(str), "\n\
insitu_descript_id[4] %.4s\n\
insitu_descript_len %d\n\
number_params %d\n\
"
	    , situ->insitu_descript_id
	    , situ->insitu_descript_len
	    , situ->number_params
	    );
    return(str);

# ifdef notyet
    isp = (struct insitu_parameter *)
	  ((char *)situ + sizeof(struct insitu_descript));

    nn = situ->number_params > 0 && situ->number_params <= 256
	  ? situ->number_params : 0;
    for(ii=0; ii < nn; ii++,isp++) {
	printf(" %s %s", str_terminate(name, isp->name, 8)
	       , str_terminate(units, isp->units, 8));
	if(!((ii+1) % 4))
	      printf("\n");
	
    }
    if(ii % 4)
	  printf("\n");
# endif
}
/* c------------------------------------------------------------------------ */

char *
dd_print_isit (struct insitu_data *isit, char *str)
{
    int ii, nn;
    float *fptr;
    struct insitu_parameter *isp;

    sprintf(str, "Contents of insitu data  len: %d\n"
	    , sizeof(struct insitu_data));
    sprintf(str+strlen(str), "\n\
insitu_data_id %.4s\n\
insitu_data_len %d\n\
julian_day %d\n\
hours %d\n\
minutes %d\n\
seconds %d\n\
"
	    , isit->insitu_data_id
	    , isit->insitu_data_len
	    , isit->julian_day
	    , isit->hours
	    , isit->minutes
	    , isit->seconds
	    );
    return(str);

# ifdef notyet
    fptr = (float *)((char *)isit + sizeof(struct insitu_data));
    nn = (isit->insitu_data_len -sizeof(struct insitu_data))/sizeof(float);
    nn = nn > 256 ? 256 : nn;

    for(ii=0; ii < nn; ii++,fptr++) {
	if(!(ii % 8))
	   printf(" %3d", ii);
	printf(" %8.2f", *fptr);
	if(!((ii+1) % 8))
	      printf("\n");
    }
    if(ii % 8)
	  printf("\n");
# endif
}
/* c------------------------------------------------------------------------ */

char *
dd_print_time (struct time_series *times, char *str)
{
    int ii, nn;
    float *fptr;

    sprintf(str, "Contents of time series data  len: %d\n"
	    , sizeof(struct time_series));
    sprintf(str+strlen(str), "\n\
time_series_id[4] %.4s\n\
time_series_len %d \n\
"
	    , times->time_series_id
	    , times->time_series_len
	    );
    printf( "%s", str);

    fptr = (float *)((char *)times + sizeof(struct time_series));
    nn = (times->time_series_len -sizeof(struct time_series))/sizeof(float);
    nn = nn > 400 ? 400 : nn;

    for(ii=0; ii < nn; ii++,fptr++) {
	if(!(ii % 8))
	   printf(" %3d", ii);
	printf(" %8.2f", *fptr);
	if(!((ii+1) % 8))
	      printf("\n");
    }
    if(ii % 8)
	  printf("\n");
}
/* c------------------------------------------------------------------------ */

char *
dd_print_rktb (struct rot_ang_table *rktb, char *str)
{
    char *a=str, *b,  c[16];
    int ii;
    struct rot_table_entry *rte;

   if(gottaSwap) {
//      ddin_crack_rktb (rktb, swapBuf, (int)0);  //Jul 26, 2011
      ddin_crack_rktb ((char *)rktb, swapBuf, (int)0);  //Jul 26, 2011
      rktb = (struct rot_ang_table *)swapBuf;
   }
    
    sprintf(str, "Contents of the super rotation angle descriptor  len: %d\n"
	    , sizeof(struct rot_ang_table));
    sprintf(str+strlen(str), "\n\
name_struct %.4s\n\
sizeof_struct %d\n\
angle2ndx %.3f\n\
ndx_que_size %d\n\
first_key_offset %d\n\
angle_table_offset %d\n\
num_rays %d\n\
"
	    , rktb->name_struct
	    , rktb->sizeof_struct
	    , rktb->angle2ndx
	    , rktb->ndx_que_size
	    , rktb->first_key_offset
	    , rktb->angle_table_offset
	    , rktb->num_rays
	    );  
    
    printf("%s", str);
    b = (char *)rktb + rktb->first_key_offset;

    for(ii=0; ii < rktb->num_rays; ii++) {
	rte = (struct rot_table_entry *)b;
	printf("%4d %7.2f %8d %7d\n"
	       , ii
	       , rte->rotation_angle
	       , rte->offset
	       , rte->size
	       );
	b += sizeof(struct rot_table_entry);
    }
    *str = '\0';
    return(str);
}

/* c------------------------------------------------------------------------ */

char *
dd_print_comm (struct comment_d *comm, char *str)
{
    unsigned int gdsos = comm->comment_des_length;

    if(gottaSwap) { swack4((char *)&comm->comment_des_length, (char *)&gdsos); }

    /* routine to print the contents of the descriptor */

    sprintf(str, "Contents of the comment descriptor  len: %d\n"
	    , sizeof(struct comment_d));
    sprintf(str+strlen(str), "\n\
 comment_des %.4s\n\
 comment_des_length %d\n\
 comment %s\n\
"
	    , comm->comment_des
	    , gdsos
	    , comm->comment
	    );
    return(str);
}
/* c------------------------------------------------------------------------ */

char *
dd_print_asib (struct platform_i *asib, char *str)
{
   if(gottaSwap) {
//      ddin_crack_asib (asib, swapBuf, (int)0);  //Jul 26, 2011
      ddin_crack_asib ((char *)asib, swapBuf, (int)0);  //Jul 26, 2011
      asib = (struct platform_i *)swapBuf;
   }
    
# ifdef obsolete
    /* routine to print the contents of the descriptor */

    sprintf(str, "Contents of the platform descriptor  len: %d\n"
	    , sizeof(struct platform_i));
    sprintf(str+strlen(str), "\n\
 platform_info %s\n\
 platform_info_length %d\n\
 longitude %f\n\
 latitude %f\n\
 altitude_msl %f\n\
 altitude_agl %f\n\
 ew_velocity %f\n\
 ns_velocity %f\n\
 vert_velocity %f\n\
 heading %f\n\
 roll %f\n\
 pitch %f\n\
 drift_angle %f\n\
 rotation_angle %f\n\
 tilt %f\n\
 ew_horiz_wind %f\n\
 ns_horiz_wind %f\n\
 vert_wind %f\n\
 heading_change %f\n\
 pitch_change %f\n\
"
	   , asib->platform_info
	   , asib->platform_info_length
	   , asib->longitude            
	   , asib->latitude             
	   , asib->altitude_msl         
	   , asib->altitude_agl         
	   , asib->ew_velocity          
	   , asib->ns_velocity          
	   , asib->vert_velocity        
	   , asib->heading              
	   , asib->roll                 
	   , asib->pitch                
	   , asib->drift_angle          
	   , asib->rotation_angle       
	   , asib->tilt                 
	   , asib->ew_horiz_wind        
	   , asib->ns_horiz_wind        
	   , asib->vert_wind            
	   , asib->heading_change       
	   , asib->pitch_change         
	   );
    return(str);
# else
    solo_reset_list_entries(slm);
    dor_print_asib(asib, slm);
    slm_dump_list(slm);
# endif
}
/* c------------------------------------------------------------------------ */

char *
dd_print_celv (struct cell_d *celv, char *str)
{
   u_int32_t numCells;
   float f[4];

   if(gottaSwap) {
      swack4((char*)&celv->dist_cells[0], (char*)f);   //Jul 26,2011
      swack4((char*)&celv->dist_cells[1], (char*)f+1); //Jul 26,2011
      swack4((char*)&celv->dist_cells[2], (char*)f+2); //Jul 26,2011

      /* we must swap "number_cells" before we use it */
      swack4((char*)&celv->number_cells, (char*)&numCells);  //Jul 26,2011

      swack4((char*)&celv->dist_cells[numCells-1], (char*)f+3);  //Jul 26,2011
//      ddin_crack_celv (celv, swapBuf, (int)0);  //Jul 26, 2011
      ddin_crack_celv ((char *)celv, swapBuf, (int)0);  //Jul 26, 2011
      celv = (struct cell_d *)swapBuf;
   }
   else {
      f[0] = celv->dist_cells[0];
      f[1] = celv->dist_cells[1];
      f[2] = celv->dist_cells[2];
      f[3] = celv->dist_cells[celv->number_cells -1];
   }

    sprintf(str, "Contents of the cell vector descriptor  len: %d\n"
	    , sizeof(struct cell_d));
    sprintf(str+strlen(str), "\n\
cell_spacing_des %.4s  \n\
cell_des_len %d\n\
number_cells %d\n\
dist_cells[0] %f\n\
dist_cells[1] %f\n\
dist_cells[2] %f\n\
dist_cells[%d] %f\n\
"
	    , celv->cell_spacing_des
	    , celv->cell_des_len
	    , celv->number_cells
	    , f[0]
	    , f[1]
	    , f[2]
	    , celv->number_cells -1
	    , f[3]
	    );
    return(str);
}
/* c------------------------------------------------------------------------ */

char *
dd_print_cfac (struct correction_d *cfac, char *str)
{
   if(gottaSwap) {
//      ddin_crack_cfac (cfac, swapBuf, (int)0);  //Jul 26, 2011
      ddin_crack_cfac ((char *)cfac, swapBuf, (int)0);  //Jul 26, 2011
      cfac = (struct correction_d *)swapBuf;
   }
    
    /* routine to print the contents of the descriptor */

# ifdef obsolete
    sprintf(str, "Contents of the correction descriptor  len: %d\n"
	    , sizeof(struct correction_d));
    sprintf(str+strlen(str), "\n\
 correction_des %s     \n\
 correction_des_length %d\n\
 azimuth_corr %f         \n\
 elevation_corr %f       \n\
 range_delay_corr %f     \n\
 longitude_corr %f       \n\
 latitude_corr %f        \n\
 pressure_alt_corr %f    \n\
 radar_alt_corr %f       \n\
 ew_gndspd_corr %f       \n\
 ns_gndspd_corr %f       \n\
 vert_vel_corr %f        \n\
 heading_corr %f         \n\
 roll_corr %f            \n\
 pitch_corr %f           \n\
 drift_corr %f           \n\
 rot_angle_corr %f       \n\
 tilt_corr %f            \n\
"
	   , cfac->correction_des     
	   , cfac->correction_des_length
	   , cfac->azimuth_corr         
	   , cfac->elevation_corr       
	   , cfac->range_delay_corr     
	   , cfac->longitude_corr       
	   , cfac->latitude_corr        
	   , cfac->pressure_alt_corr    
	   , cfac->radar_alt_corr       
	   , cfac->ew_gndspd_corr       
	   , cfac->ns_gndspd_corr       
	   , cfac->vert_vel_corr        
	   , cfac->heading_corr         
	   , cfac->roll_corr            
	   , cfac->pitch_corr           
	   , cfac->drift_corr           
	   , cfac->rot_angle_corr       
	   , cfac->tilt_corr            
	   );
    return(str);
# else
    solo_reset_list_entries(slm);
    dor_print_cfac(cfac, slm);
    slm_dump_list(slm);
# endif
}
/* c------------------------------------------------------------------------ */

char *
dd_print_cspd (struct cell_spacing_d *cspd, char *str)
{
    int i;
    char *a=str;

   if(gottaSwap) {
//      eld_crack_cspd (cspd, swapBuf, (int)0);  //Jul 26, 2011
      eld_crack_cspd ((char *)cspd, swapBuf, (int)0);  //Jul 26, 2011
      cspd = (struct cell_spacing_d *)swapBuf;
   }
    
    sprintf(a, "Contents of the cell_spacing_d descriptor  len: %d\n"
	    , sizeof(struct cell_spacing_d));
    a += strlen(a);
    sprintf(a, "\n\
cell_spacing_des %.4s\n\
cell_spacing_des_len %d\n\
num_segments %d\n\
distToFirst %d\n\
"
	    , cspd->cell_spacing_des
	    , cspd->cell_spacing_des_len
	    , cspd->num_segments   
	    , cspd->distToFirst
	    );
    strcat(a, "spacing[6]");
    a += strlen(a);
    for(i=0; i < 6; i++) {
	sprintf(a, " %d", cspd->spacing[i]);
	a += strlen(a);
    }
    strcat(a, "\nnum_cells[6]");
    a += strlen(a);
    for(i=0; i < 6; i++) {
	sprintf(a, " %d", cspd->num_cells[i]);
	a += strlen(a);
    }
    sprintf(a, "\n");
    a += strlen(a);
    return(str);
}
/* c------------------------------------------------------------------------ */

char *
dd_print_csfd (struct cell_spacing_fp_d *csfd, char *str)
{
    int i;
    char *a=str;

   if(gottaSwap) {
# ifdef notyet
      eld_crack_csfd (csfd, swapBuf, (int)0);
      csfd = (struct cell_spacing_fp_d *)swapBuf;
# endif
   }
    
    sprintf(a, "Contents of the cell_spacing_fp_d descriptor  len: %d\n"
	    , sizeof(struct cell_spacing_fp_d));
    a += strlen(a);
    sprintf(a, "\n\
cell_spacing_des %.4s\n\
cell_spacing_des_len %d\n\
num_segments %d\n\
distToFirst %f\n\
"
	    , csfd->cell_spacing_des
	    , csfd->cell_spacing_des_len
	    , csfd->num_segments   
	    , csfd->distToFirst
	    );
    strcat(a, "spacing[8]");
    a += strlen(a);
    for(i=0; i < 8; i++) {
	sprintf(a, " %f", csfd->spacing[i]);
	a += strlen(a);
    }
    strcat(a, "\nnum_cells[8]");
    a += strlen(a);
    for(i=0; i < 8; i++) {
	sprintf(a, " %d", csfd->num_cells[i]);
	a += strlen(a);
    }
    sprintf(a, "\n");
    a += strlen(a);
    return(str);
}
/* c------------------------------------------------------------------------ */

char *
dd_print_frad (struct field_parameter_data *frad, char *str)
{
    int i;
    short *ss;
    char *a=str, *c;

   if(gottaSwap) {
//      eld_crack_frad (frad, swapBuf, (int)0);  //Jul 26, 2011
      eld_crack_frad ((char *)frad, swapBuf, (int)0);  //Jul 26, 2011
      frad = (struct field_parameter_data *)swapBuf;
   }
    
    sprintf(str, "Contents of the field parameter descriptor  len: %d\n"
	    , sizeof(struct field_parameter_data));
    sprintf(str+strlen(str), "\n\
field_param_data %.4s\n\
field_param_data_len %d  \n\
data_sys_status %d       \n\
radar_name %.8s\n\
test_pulse_level %f     \n\
test_pulse_dist %f      \n\
test_pulse_width %f     \n\
test_pulse_freq %f      \n\
test_pulse_atten %d     \n\
test_pulse_fnum %d      \n\
noise_power %f          \n\
ray_count %d            \n\
first_rec_gate %d       \n\
last_rec_gate %d        \n\
"
	    , frad->field_param_data
	    , frad->field_param_data_len  
	    , frad->data_sys_status       
	    , frad->radar_name
	    , frad->test_pulse_level     
	    , frad->test_pulse_dist      
	    , frad->test_pulse_width     
	    , frad->test_pulse_freq      
	    , frad->test_pulse_atten     
	    , frad->test_pulse_fnum      
	    , frad->noise_power          
	    , frad->ray_count            
	    , frad->first_rec_gate       
	    , frad->last_rec_gate        
	    );
    a += strlen(a);
    ss = (short *)((char *)frad + sizeof(struct field_parameter_data));
    for(i=0; i < 96; ) {
	sprintf(a, " %6d", *ss++);
	if(!(++i % 4))
	      strcat(a, "\n");
	a += strlen(a);
    }
    strcat(a, "\n");
    return(str);
}
/* c------------------------------------------------------------------------ */

#ifndef notyet
char *
dd_print_frib (char *a, char *str)
{
    struct generic_descriptor *gd;
    char *b;
    
    gd = (struct generic_descriptor *)a;
    if(gd->sizeof_struct == sizeof(struct field_radar_i_v0)) {
	b = dd_print_frib_v0((struct field_radar_i_v0 *)a, str);
    }
    else {
	b = dd_print_frib_v1((field_radar_i *)a, str);
    }
}
/* c------------------------------------------------------------------------ */

char *
dd_print_frib_v1 (struct field_radar_i *frib, char *str)
{
    int i, nn;
    char *a=str, c[128];

    sprintf(a, "Contents of the field radar descriptor  len: %d\n"
	    , sizeof(struct field_radar_i));
    a += strlen(a);
    sprintf(a, "\n\
field_radar_info[4] %.4s\n\
field_radar_info_len %d\n\
data_sys_id %d\n\
loss_out %f		\n\
loss_in %f		\n\
loss_rjoint %f		\n\
ant_v_dim %f		\n\
ant_h_dim %f		\n\
ant_noise_temp %f	\n\
r_noise_figure %f	\n\
"
	    , frib->field_radar_info
	    , frib->field_radar_info_len
	    , frib->data_sys_id
	    , frib->loss_out		
	    , frib->loss_in		
	    , frib->loss_rjoint		
	    , frib->ant_v_dim		
	    , frib->ant_h_dim		
	    , frib->ant_noise_temp	
	    , frib->r_noise_figure	
	    );
    a += strlen(a);
    sprintf(a, "xmit_power[5]");
    for(i=0; i < 5; i++) {
	sprintf(a+strlen(a), " %f", frib->xmit_power[i]);
    }
    a += strlen(a);
    sprintf(a, "\n\
x_band_gain %f          \n\
"
	    , frib->x_band_gain          
	    );
    a += strlen(a);
    sprintf(a, "receiver_gain[5]");
    for(i=0; i < 5; i++) {
	sprintf(a+strlen(a), " %f", frib->receiver_gain[i]);
    }
    a += strlen(a);
    sprintf(a, "\nif_gain[5]");
    for(i=0; i < 5; i++) {
	sprintf(a+strlen(a), " %f", frib->if_gain[i]);
    }
    a += strlen(a);
    sprintf(a, "\n\
conversion_gain %f      \n\
"
	    , frib->conversion_gain
	    );
    a += strlen(a);
    sprintf(a, "scale_factor[5]");
    for(i=0; i < 5; i++) {
	sprintf(a+strlen(a), " %f", frib->scale_factor[i]);
    }
    nn = strlen( frib->file_name ) < 128 ? strlen( frib->file_name ) : 127;
    strncpy( c, frib->file_name, nn );
    c[nn] = '\0';
    a += strlen(a);

    sprintf(a, "\n\
processor_const %f      \n\
dly_tube_antenna %d	\n\
dly_rndtrip_chip_atod %d\n\
dly_timmod_testpulse %d\n\
dly_modulator_on %d	\n\
dly_modulator_off %d	\n\
peak_power_offset %f    \n\
test_pulse_offset %f    \n\
E_plane_angle %f        \n\
H_plane_angle %f        \n\
encoder_antenna_up %f   \n\
pitch_antenna_up %f     \n\
indepf_times_flg %d	\n\
indep_freq_gate %d	\n\
time_series_gate %d	\n\
file_name %s\n\
"
	    , frib->processor_const      
	    , frib->dly_tube_antenna	
	    , frib->dly_rndtrip_chip_atod
	    , frib->dly_timmod_testpulse
	    , frib->dly_modulator_on	
	    , frib->dly_modulator_off	
	    , frib->peak_power_offset    
	    , frib->test_pulse_offset    
	    , frib->E_plane_angle        
	    , frib->H_plane_angle        
	    , frib->encoder_antenna_up   
	    , frib->pitch_antenna_up     
	    , frib->indepf_times_flg	
	    , frib->indep_freq_gate	
	    , frib->time_series_gate	
	    , c
	    );
    a += strlen(a);
    return(str);
}
/* c------------------------------------------------------------------------ */

char *
dd_print_frib_v0 (struct field_radar_i_v0 *frib, char *str)
{
    int i;
    char *a=str, c[128];

    sprintf(a, "Contents of the field radar descriptor  len: %d\n"
	    , sizeof(struct field_radar_i_v0));
    a += strlen(a);
    sprintf(a, "\n\
field_radar_info %.4s\n\
field_radar_info_len %d  \n\
data_sys_id %d          \n\
signal_source %d        \n\
loss_out %f             \n\
loss_in %f              \n\
ant_loss %f             \n\
sys_loss_0 %f           \n\
sys_loss_1 %f           \n\
sys_loss_2 %f           \n\
sys_loss_3 %f           \n\
ant_v_dim %f            \n\
ant_h_dim %f            \n\
aperture_eff %f         \n\
filter_num %s\n\
bessel_correct %f       \n\
ant_noise_temp %f       \n\
r_noise_figure %f       \n\
dly_tube_antenna %d     \n\
dly_antenna_dircplr %d  \n\
"
	    , frib->field_radar_info
	    , frib->field_radar_info_len  
	    , frib->data_sys_id          
	    , frib->signal_source        
	    , frib->loss_out             
	    , frib->loss_in              
	    , frib->ant_loss             
	    , frib->sys_loss_0           
	    , frib->sys_loss_1           
	    , frib->sys_loss_2           
	    , frib->sys_loss_3           
	    , frib->ant_v_dim            
	    , frib->ant_h_dim            
	    , frib->aperture_eff         
	    , str_terminate(c, frib->filter_num, sizeof(frib->filter_num))
	    , frib->bessel_correct       
	    , frib->ant_noise_temp       
	    , frib->r_noise_figure       
	    , frib->dly_tube_antenna     
	    , frib->dly_antenna_dircplr
	    );
    a += strlen(a);
    sprintf(a, "dly_dircplr_ad[6]");
    for(i=0; i < 6; i++) {
	sprintf(a, " %d", frib->dly_dircplr_ad[i]);
	a += strlen(a);
    }
    sprintf(a, "\n\
dly_timmod_testpulse %d \n\
dly_modulator_on %d     \n\
dly_modulator_off %d    \n\
dly_rf_twt_on %d        \n\
dly_rf_twt_off %d       \n\
indepf_times_flg %d     \n\
indep_freq_gate %d      \n\
times_series_gate %d    \n\
"
	    , frib->dly_timmod_testpulse 
	    , frib->dly_modulator_on     
	    , frib->dly_modulator_off    
	    , frib->dly_rf_twt_on        
	    , frib->dly_rf_twt_off       
	    , frib->indepf_times_flg     
	    , frib->indep_freq_gate      
	    , frib->times_series_gate    
	    );
    a += strlen(a);
    return(str);
}
#endif
/* c------------------------------------------------------------------------ */

char *
dd_print_parm (struct parameter_d *parm, char *str)
{
    char *a=str, c[128];
    char name[16], long_name[48], units[16], thr_name[16];

   if(gottaSwap) {
//      ddin_crack_parm (parm, swapBuf, (int)0);  //jul 26, 2011
      ddin_crack_parm ((char *)parm, swapBuf, (int)0);  //jul 26, 2011
      parm = (struct parameter_d *)swapBuf;
   }
    
    /* routine to print the contents of the parameter descriptor */

# ifdef obsolete
    sprintf(str, "Contents of the parameter descriptor  len: %d\n"
	    , sizeof(struct parameter_d));
    sprintf(str+strlen(str), "\n\
parameter_des %s     \n\
parameter_des_length %d  \n\
parameter_name %s    \n\
param_description %s\n\
param_units %s       \n\
interpulse_time %d      \n\
xmitted_freq %d         \n\
recvr_bandwidth %f      \n\
pulse_width %d          \n\
polarization %d         \n\
num_samples %d          \n\
binary_format %d        \n\
threshold_field %s   \n\
threshold_value %f      \n\
parameter_scale %f      \n\
parameter_bias %f       \n\
bad_data %d             \n\
"
	    , parm->parameter_des     
	    , parm->parameter_des_length  
	    , str_terminate(c, parm->parameter_name
			    , sizeof(parm->parameter_name))
	    , str_terminate(c+16, parm->param_description
			    , sizeof(parm->param_description))
	    , str_terminate(c+64, parm->param_units
			    , sizeof(parm->param_units))
	   , parm->interpulse_time      
	   , parm->xmitted_freq         
	   , parm->recvr_bandwidth      
	   , parm->pulse_width          
	   , parm->polarization         
	   , parm->num_samples          
	   , parm->binary_format        
	   , str_terminate(c+80, parm->threshold_field
			   , sizeof(parm->threshold_field))
	   , parm->threshold_value      
	   , parm->parameter_scale      
	   , parm->parameter_bias       
	   , parm->bad_data             
	   );
    return(str);
# else
    solo_reset_list_entries(slm);
    dor_print_parm(parm, slm);
    slm_dump_list(slm);
# endif
}
/* c------------------------------------------------------------------------ */

char *
dd_print_radd (struct radar_d *radd, char *str)
{
    char *a=str, c[128];

   if(gottaSwap) {
//      ddin_crack_radd (radd, swapBuf, (int)0);  //Jul 26, 2011
      ddin_crack_radd ((char *)radd, swapBuf, (int)0);  //Jul 26, 2011
      radd = (struct radar_d *)swapBuf;
   }
    
    /* routine to print the contents of the descriptor */

# ifdef obsolete
    sprintf(str, "Contents of the radar descriptor  len: %d\n"
	    , sizeof(struct radar_d));
    sprintf(str+strlen(str), "\n\
 radar_des %s         \n\
 radar_des_length %d      \n\
 radar_name %s        \n\
 radar_const %f          \n\
 peak_power %f           \n\
 noise_power %f          \n\
 receiver_gain %f        \n\
 antenna_gain %f         \n\
 system_gain %f          \n\
 horz_beam_width %f      \n\
 vert_beam_width %f      \n\
 radar_type %d          \n\
 scan_mode %d           \n\
 req_rotat_vel %f        \n\
 scan_mode_pram0 %f      \n\
 scan_mode_pram1 %f      \n\
 num_parameter_des %d   \n\
 total_num_des %d       \n\
 data_compress %d        \n\
 data_reduction %d       \n\
 data_red_parm0 %f       \n\
 data_red_parm1 %f       \n\
 radar_longitude %f      \n\
 radar_latitude %f       \n\
 radar_altitude %f       \n\
 eff_unamb_vel %f        \n\
 eff_unamb_range %f      \n\
 num_freq_trans %d       \n\
 num_ipps_trans %d       \n\
 freq1 %f                \n\
 freq2 %f                \n\
 freq3 %f                \n\
 freq4 %f                \n\
 freq5 %f                \n\
 interpulse_per1 %f      \n\
 interpulse_per2 %f      \n\
 interpulse_per3 %f      \n\
 interpulse_per4 %f      \n\
 interpulse_per5 %f      \n\
"
	   , radd->radar_des          
	   , radd->radar_des_length       
	   , str_terminate(c, radd->radar_name, sizeof(radd->radar_name))
	   , radd->radar_const           
	   , radd->peak_power            
	   , radd->noise_power           
	   , radd->receiver_gain         
	   , radd->antenna_gain          
	   , radd->system_gain           
	   , radd->horz_beam_width       
	   , radd->vert_beam_width       
	   , radd->radar_type           
	   , radd->scan_mode            
	   , radd->req_rotat_vel         
	   , radd->scan_mode_pram0       
	   , radd->scan_mode_pram1       
	   , radd->num_parameter_des    
	   , radd->total_num_des        
	   , radd->data_compress         
	   , radd->data_reduction        
	   , radd->data_red_parm0        
	   , radd->data_red_parm1        
	   , radd->radar_longitude       
	   , radd->radar_latitude        
	   , radd->radar_altitude        
	   , radd->eff_unamb_vel         
	   , radd->eff_unamb_range       
	   , radd->num_freq_trans        
	   , radd->num_ipps_trans        
	   , radd->freq1                 
	   , radd->freq2                 
	   , radd->freq3                 
	   , radd->freq4                 
	   , radd->freq5                 
	   , radd->interpulse_per1       
	   , radd->interpulse_per2       
	   , radd->interpulse_per3       
	   , radd->interpulse_per4       
	   , radd->interpulse_per5       
	   );
    return(str);
# else
    solo_reset_list_entries(slm);
    dor_print_radd(radd, slm);
    slm_dump_list(slm);
# endif
}
/* c------------------------------------------------------------------------ */

char *
dd_print_rdat (struct paramdata_d *rdat, char *str)
{
    int i;
    char *a=str, c[128];
    short *ss=(short *)((char *)rdat +sizeof(struct paramdata_d)), *tt, sval;

   if(gottaSwap) {
//      ddin_crack_qdat (rdat, swapBuf, (int)0);  //Jul 26, 2011
      ddin_crack_qdat ((char *)rdat, swapBuf, (int)0);  //Jul 26, 2011
      rdat = (struct paramdata_d *)swapBuf;
   }

    /* routine to print the contents of the pdata descriptor */

    sprintf(str, "Contents of the pdata descriptor  len: %d\n"
	    , sizeof(struct paramdata_d));
    sprintf(str+strlen(str), "\n\
pdata_desc %.4s\n\
pdata_length %d\n\
pdata_name %s\n\
"
	    , rdat->pdata_desc
	    , rdat->pdata_length
	    , str_terminate(c, rdat->pdata_name, sizeof(rdat->pdata_name))
	    );	    
    a += strlen(a);
    for(i=0; i < 96; ss++) {
       if(gottaSwap) {
	  tt = &sval;
	  swack2((char*)ss, (char*)tt);  //Jul 26, 2011
       }
       else { tt = ss; }
	sprintf(a, "%6d", *tt);
	if(!(++i % 12))
	      strcat(a, "\n");
	a += strlen(a);
    }
    strcat(a, "\n");
    return(str);
}
/* c------------------------------------------------------------------------ */

char *
dd_print_ryib (struct ray_i *ryib, char *str)
{
   if(gottaSwap) {
//      ddin_crack_ryib (ryib, swapBuf, (int)0);  //Jul 26, 2011
      ddin_crack_ryib ((char *)ryib, swapBuf, (int)0);  //Jul 26, 2011
      ryib = (struct ray_i *)swapBuf;
   }
    
    /* routine to print the contents of the descriptor */

# ifdef obsolete
    sprintf(str, "Contents of the ray descriptor  len: %d\n"
	    , sizeof(struct ray_i));
    sprintf(str+strlen(str), "\n\
 ray_info %s\n\
 ray_info_length %d\n\
 sweep_num %d            \n\
 julian_day %d           \n\
 hour %d                 \n\
 minute %d               \n\
 second %d               \n\
 millisecond %d          \n\
 azimuth %f              \n\
 elevation %f            \n\
 peak_power %f           \n\
 true_scan_rate %f       \n\
 ray_status %d           \n\
"
	   , ryib->ray_info
	   , ryib->ray_info_length
	   , ryib->sweep_num            
	   , ryib->julian_day           
	   , ryib->hour                 
	   , ryib->minute               
	   , ryib->second               
	   , ryib->millisecond          
	   , ryib->azimuth              
	   , ryib->elevation            
	   , ryib->peak_power           
	   , ryib->true_scan_rate       
	   , ryib->ray_status           
	   );
    return(str);
# else
    solo_reset_list_entries(slm);
    dor_print_ryib(ryib, slm);
    slm_dump_list(slm);
# endif
}
/* c------------------------------------------------------------------------ */

char *
dd_print_sswb (struct super_SWIB *sswb, char *str)
{
    char *a=str,  c[16];
    int ii;

   if(gottaSwap) {
//      if(hostIsLittleEndian()) { ddin_crack_sswbLE (sswb, swapBuf, (int)0); }  //Jul 26, 2011
//      else { ddin_crack_sswb (sswb, swapBuf, (int)0); }  //Jul 26, 2011
      if(hostIsLittleEndian()) { ddin_crack_sswbLE ((char *)sswb, swapBuf, (int)0); }  //Jul 26, 2011
      else { ddin_crack_sswb ((char *)sswb, swapBuf, (int)0); }  //Jul 26, 2011
      sswb = (struct super_SWIB *)swapBuf;
   }
    
# ifdef obsolete
    sprintf(str, "Contents of the super sweepinfo descriptor  len: %d\n"
	    , sizeof(struct super_SWIB));
    sprintf(str+strlen(str), "\n\
name_struct %s\n\
sizeof_struct %d\n\
last_used %d        \n\
start_time %d\n\
stop_time %d\n\
sizeof_file %d\n\
compression_flag %d\n\
volume_time_stamp %d\n\
num_params %d          \n\
radar_name %s\n\
d_start_time %.3f\n\
d_stop_time %.3f\n\
version_num %d\n\
num_key_tables %d\n\
status %d\n\
"
	    , sswb->name_struct
	    , sswb->sizeof_struct
	    , sswb->last_used        
	    , sswb->start_time
	    , sswb->stop_time
	    , sswb->sizeof_file
	    , sswb->compression_flag
	    , sswb->volume_time_stamp
	    , sswb->num_params          
	    , str_terminate(c, sswb->radar_name, sizeof(sswb->radar_name))
	    , sswb->d_start_time
	    , sswb->d_stop_time
	    , sswb->version_num
	    , sswb->num_key_tables
	    , sswb->status
	    );
    a += strlen(a);
    for(ii=0; ii < 7; ii++) {
	sprintf(a, "place_holder[%d] %d\n", ii, sswb->place_holder[ii]);
	a += strlen(a);
    }
    for(ii=0; ii < sswb->num_key_tables; ii++) {
	sprintf(a, "key_table[%d]->  offset: %d  size: %d  type: %d\n"
		, ii
		, sswb->key_table[ii].offset
		, sswb->key_table[ii].size
		, sswb->key_table[ii].type
		);
    }
    return(str);
# else
    solo_reset_list_entries(slm);
    dor_print_sswb(sswb, slm);
    slm_dump_list(slm);
# endif
}

/* c------------------------------------------------------------------------ */

char *
dd_print_swib (struct sweepinfo_d *swib, char *str)
{
    char *a=str, c[128];

   if(gottaSwap) {
//      ddin_crack_swib (swib, swapBuf, (int)0);  //Jul 26, 2011   	
      ddin_crack_swib ((char *)swib, swapBuf, (int)0);  //Jul 26, 2011
      swib = (struct sweepinfo_d *)swapBuf;
   }
    
# ifdef obsolete
    sprintf(str, "Contents of the sweepinfo descriptor  len: %d\n"
	    , sizeof(struct sweepinfo_d));
    sprintf(str+strlen(str), "\n\
sweep_des %s\n\
sweep_des_length %d   \n\
radar_name %s\n\
sweep_num %d          \n\
num_rays %d           \n\
start_angle %f        \n\
stop_angle %f         \n\
fixed_angle %f\n\
filter_flag %d\n\
"
	    , swib->sweep_des
	    , swib->sweep_des_length   
	    , str_terminate(c, swib->radar_name, sizeof(swib->radar_name))
	    , swib->sweep_num          
	    , swib->num_rays           
	    , swib->start_angle        
	    , swib->stop_angle         
	    , swib->fixed_angle
	    , swib->filter_flag
	    );
    return(str);
# else
    solo_reset_list_entries(slm);
    dor_print_swib(swib, slm);
    slm_dump_list(slm);
# endif
}
/* c------------------------------------------------------------------------ */

char *
dd_print_vold (struct volume_d *vold, char *str)
{
    char *a=str, c[64], d[12], e[12];

   if(gottaSwap) {
//      ddin_crack_vold (vold, swapBuf, (int)0);  //Jul 26, 2011
      ddin_crack_vold ((char *)vold, swapBuf, (int)0);  //Jul 26, 2011
      vold = (struct volume_d *)swapBuf;
   }
    
# ifdef obsolete
    /* routine to print the contents of the volume descriptor */

    sprintf(str, "Contents of the volume descriptor  len: %d\n"
	    , sizeof(struct volume_d));
    sprintf(str+strlen(str), "\n\
 volume_des %s\n\
 volume_des_length %d\n\
 format_version %d\n\
 volume_num %d\n\
 maximum_bytes %d\n\
 proj_name %s\n\
 year %d\n\
 month %d\n\
 day %d\n\
 data_set_hour %d\n\
 data_set_minute %d\n\
 data_set_second %d\n\
 flight_num %s\n\
 gen_facility %s\n\
 gen_year %d\n\
 gen_month %d\n\
 gen_day %d\n\
 number_sensor_des %d\n\
"
	   , vold->volume_des
	   , vold->volume_des_length
	   , vold->format_version     
	   , vold->volume_num           
	   , vold->maximum_bytes        
	   , str_terminate(c, vold->proj_name, sizeof(vold->proj_name))
	   , vold->year                 
	   , vold->month                
	   , vold->day                  
	   , vold->data_set_hour        
	   , vold->data_set_minute      
	   , vold->data_set_second      
	   , str_terminate(d, vold->flight_num, sizeof(vold->flight_num))
	   , str_terminate(e, vold->gen_facility, sizeof(vold->gen_facility))
	   , vold->gen_year             
	   , vold->gen_month            
	   , vold->gen_day              
	   , vold->number_sensor_des    
	   );
    return(str);
# else
    solo_reset_list_entries(slm);
    dor_print_vold(vold, slm);
    slm_dump_list(slm);
# endif
}
/* c------------------------------------------------------------------------ */

char *
dd_print_wave (struct waveform_d *wave, char *str)
{
    int i;
    char *a=str, c[128], d[128];

    sprintf(a, "Contents of the waveform descriptor  len: %d\n"
	    , sizeof(struct waveform_d));
    a += strlen(a);
    sprintf(a, "\n\
waveform_des %.4s\n\
waveform_des_length %d\n\
ps_file_name %s\n\
"
	    , wave->waveform_des
	    , wave->waveform_des_length
	    , str_terminate(c, wave->ps_file_name, sizeof(wave->ps_file_name))
	    , wave->ps_file_name
	    );
    strcat(a, "\nnum_chips[6]");
    a += strlen(a);
    for(i=0; i < 6; i++) {
	sprintf(a, " %d", wave->num_chips[i]);
	a += strlen(a);
    }
    sprintf(a, "\n\
blank_chip %s\n\
repeat_seq %f\n\
repeat_seq_dwel %d\n\
total_pcp %d\n\
"
	    , str_terminate(d, wave->blank_chip, sizeof(wave->blank_chip))
	    , wave->repeat_seq
	    , wave->repeat_seq_dwel
	    , wave->total_pcp
	    );
    strcat(a, "chip_offset[6]");
    a += strlen(a);
    for(i=0; i < 6; i++) {
	sprintf(a, " %d", wave->chip_offset[i]);
	a += strlen(a);
    }
    strcat(a, "\nchip_width[6]");
    a += strlen(a);
    for(i=0; i < 6; i++) {
	sprintf(a, " %d", wave->chip_width[i]);
	a += strlen(a);
    }
# ifndef obsolete
    sprintf(a, "\n\
ur_pcp %f\n\
uv_pcp %f\n\
"
	    , wave->ur_pcp
	    , wave->uv_pcp
	    );
# endif
    strcat(a, "num_gates[6]");
    a += strlen(a);
    for(i=0; i < 6; i++) {
	sprintf(a, " %d", wave->num_gates[i]);
	a += strlen(a);
    }
    sprintf(a, "\n");
    a += strlen(a);
    sprintf(a, "gate_dist1[2] %d %d\n"
	    , wave->gate_dist1[0]
	    , wave->gate_dist1[1]
	    );
    a += strlen(a);
    sprintf(a, "gate_dist2[2] %d %d\n"
	    , wave->gate_dist2[0]
	    , wave->gate_dist2[1]
	    );
    a += strlen(a);
    sprintf(a, "gate_dist3[2] %d %d\n"
	    , wave->gate_dist3[0]
	    , wave->gate_dist3[1]
	    );
    a += strlen(a);
    sprintf(a, "gate_dist4[2] %d %d\n"
	    , wave->gate_dist4[0]
	    , wave->gate_dist4[1]
	    );
    a += strlen(a);
    sprintf(a, "gate_dist5[2] %d %d\n"
	    , wave->gate_dist5[0]
	    , wave->gate_dist5[1]
	    );
    a += strlen(a);
    return(str);
}
/* c------------------------------------------------------------------------ */

int 
ddx_bad_vibs (char *a)
{
    printf("Unrecognized descriptor name or bad length\n");
    printf("ASCII\n");
    ezascdmp(a, 0, 480);
    printf("HEX\n");
    ezhxdmp(a, 0, 480);
}
/* c------------------------------------------------------------------------ */

