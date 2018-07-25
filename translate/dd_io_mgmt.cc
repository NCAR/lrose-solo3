#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

/*
 * This file contains routines:
 * 
 * cdcode
 * ctype16
 * ctypeu16
 * 
 * dd_crack_datime
 * dd_establish_source_dev_names
 * dd_gen_packet_info
 * dd_init_io_structs
 * dd_input_read_close
 * dd_input_read_open
 * dd_io_reset_offset
 * dd_logical_read
 * dd_next_source_dev_name
 * dd_phys_read
 * dd_really_skip_recs
 * dd_relative_time
 * dd_reset_ios
 * dd_return_current_packet
 * dd_return_ios
 * dd_return_next_packet
 * dd_rewind_dev
 * dd_skip_files
 * dd_skip_recs
 * getreply
 * ezascdmp
 * ezhxdmp
 * oketype
 * okint
 * okreal
 * strnlen
 * 
 * 
 * gri_interest
 * gri_max_lines
 * gri_nab_input_filters
 * gri_print_list
 * gri_set_max_lines
 * gri_start_stop_chars
 * 
 */

/* c------------------------------------------------------------------------ */

#include <iostream>

#include <LittleEndian.hh>
#include <sys/time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/file.h>
#include "dorade_share.hh"
#include "dd_crackers.hh"
#include "dd_io_mgmt.hh"
#include "to_fb.hh"

# ifdef HAVE_SYS_MTIO_H
#   include <sys/mtio.h>
# endif /* ifdef HAVE_SYS_MTIO_H */
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <ctype.h>
# include <string.h>
# include <dd_math.h>
# ifdef sun
# include <sys/ioccom.h>
# endif

# if (defined(osx) || defined(__APPLE__))
# include <sys/ioccom.h>
# endif

# ifdef SVR4
# include <fcntl.h>
# endif



/*
# define MTIOCTOP 0
 * from <unistd.h> just to remember what the symbols mean
 */
/* #define	SEEK_SET		0	absolute offset */
/* #define	SEEK_CUR		1	relative to current offset */
/* #define	SEEK_END		2	relative to end of file */
static const int K64 = 65536;

static struct source_devs_mgmt *devs_top=NULL;
static int lines_in_display=26;
static int num_io_structs=0;
static struct input_read_info *read_io_list[MAX_OPENED_FILES];
static char naught='\0';


/* c------------------------------------------------------------------------ */
void Alarm (int v)
{ /* yawn */ }
/* c------------------------------------------------------------------------ */
void QuickSleep (int ms)
//
// Sleep for the given number of milliseconds.
//
{
	struct itimerval iv;
//
// Set up an alarm.
//
	iv.it_interval.tv_sec = iv.it_interval.tv_usec = 0;
	iv.it_value.tv_sec = ms >= 1000 ? ms/1000 : 0;
	iv.it_value.tv_usec = (ms % 1000) * 1000;
	signal (SIGALRM, Alarm);
	setitimer (ITIMER_REAL, &iv, 0);
//
// Wait for it to come through.
//
	pause ();
}
/* c------------------------------------------------------------------------ */

int ffb_skip_one_fwd (FILE *strm)
{
   int ii, jj;
   int32_t nab0, nab, size_read, offset;

   size_read = fread((void *)&nab0, 1, sizeof(nab), strm );
   if (feof (strm) || size_read < sizeof (nab)) {
      return (-1);
   }
   if (ferror (strm)) {
      perror ("Forward skip error");
      return (-1);
   }
   if (hostIsLittleEndian())
//     { swack4 (&nab0, &nab); }  //Jul 26, 2011
     { swack4 ((char *)&nab0, (char *)&nab); }  //Jul 26, 2011
   else
     { nab = nab0; }
   if ((jj = fseek( strm, nab+sizeof(nab), SEEK_CUR )) == 0)
     { return (nab); }

   return (-1);
}

/* c------------------------------------------------------------------------ */

int ffb_skip_one_bkw (FILE *strm)
{
   int ii, jj;
   int32_t nab0, nab, size_read, offset;

   jj = fseek( strm, -sizeof(nab), SEEK_CUR );
   size_read = fread((void *)&nab0, 1, sizeof(nab), strm );
   if (ferror (strm)) {
      perror ("Backward skip error");
      return (-1);
   }
   if (size_read < sizeof (nab)) {
      return (-1);
   }
   if (hostIsLittleEndian())
//     { swack4 (&nab0, &nab); }  //Jul 26, 2011
     { swack4 ((char *)&nab0, (char *)&nab); }  //Jul 26, 2011
   else
     { nab = nab0; }
   if ((jj = fseek( strm, -(nab +2*sizeof(nab)), SEEK_CUR )) == 0)
     { return (nab); }

   return (-1);
}


/* c------------------------------------------------------------------------ */

int ffb_read(FILE *fin, char *buf, int count )
{
    /* fortran-binary read routine
     */
    int32_t size_rec=0, rlen1, rlen2=0, nab;
    size_t len_read, len2;

    /* read the record size */
    rlen1 = fread((void *)&nab, 1, sizeof(nab), fin );

    if( rlen1 < sizeof(nab))
	  return(rlen1);

    if(hostIsLittleEndian()) {
//       swack4(&nab, &size_rec);  //Jul 26, 2011
       swack4((char *)&nab, (char *)&size_rec);  //Jul 26, 2011
    }
    else {
       size_rec = nab;
    }

    if( size_rec > 0 ) {
	/* read the record
	 * (the read may be less than the size of the record)
	 */
	rlen2 = size_rec <= count ? size_rec : count;

	rlen1 = fread( (void *)buf, sizeof(char), rlen2, fin );
	if( rlen1 < 1 )
	      return(rlen1);
	rlen2 = rlen1 < size_rec ?
	      size_rec-rlen1 : 0; /* set up skip to end of record */
    }
    else
	  rlen1 = 0;
    
    rlen2 += sizeof(size_rec);

    /* skip thru the end of record */
    rlen2 = fseek( fin, rlen2, SEEK_CUR );
    rlen2 = ftell( fin );
    return(rlen1);
}
/*c----------------------------------------------------------------------*/

int 
cdcode ( /* try to decode the string */
    char *s,
    int n,
    int *ival,
    float *rval
)
{
    if( okint( s, n, ival ))
	 {
	     *rval = (float)(*ival);
	     return(1);
	 }
    else if( oketype( s, n, rval ))
	 {
	     if( fabs((double)(*rval)) <= 1000000000. )
		  *ival = (int)(*rval +0.5); 
	     else
		  *ival = 0;
	     return(2);
	 }
    return(0);
}
/* c-------------------------------------------------------------------------*/

void 
ctype16 (	/* type as 16 bit integers */
    short *a,
    int m,
    int n
)
{
    int ii=m/2, s=0, nn=(n+1)/2;
    short *us=a+m;
    
    for(; nn-- > 0; ii++, us++) {
	if( s == 0 ) {
	    printf("%5d)", ii);	/* new line label */
	    s = 6;
	}
	printf( "%6d", *us);
	s += 6;
	
	if( s >= 66 ) {
	    printf( "\n" );	/* start a new line */
	    s = 0;
	}
    }
    if( s > 0 ) {
	printf( "\n" );
    }
}
/* c-------------------------------------------------------------------------*/

void 
xctypeu16 (	/* type as 16 bit integers */
    unsigned short *a,
    int m,
    int n
)
{
    int ii=m/2, s=0, nn=(n+1)/2;
    unsigned short *us=a+ii;
    
    for(; nn-- > 0; ii++, us++) {
	if( s == 0 ) {
	    printf("%5d)", ii);	/* new line label */
	    s = 6;
	}
	printf( "%6d", *us);
	s += 6;
	
	if( s >= 66 ) {
	    printf( "\n" );	/* start a new line */
	    s = 0;
	}
    }
    if( s > 0 ) {
	printf( "\n" );
    }
}
/* c-------------------------------------------------------------------------*/

void 
ctypeu16 (	/* type as 16 bit integers */
    unsigned char *a,
    int m,
    int n
)
{
    int ii=m/2, s=0, nn=(n+1)/2;
    int jj = m;
    short ival;
    unsigned char *bb = a + m;
    
    for(; nn-- > 0; ii++, bb += 2, jj += 2) {
	if( s == 0 ) {
	    printf("%5d)", jj);	/* new line label */
	    s = 6;
	}
	memcpy(&ival, bb, 2);
	printf( "%6d", ival);
	s += 6;
	
	if( s >= 66 ) {
	    printf( "\n" );	/* start a new line */
	    s = 0;
	}
    }
    if( s > 0 ) {
	printf( "\n" );
    }
}
/* c-------------------------------------------------------------------------*/

void 
slm_ctypeu16 (	/* type as 16 bit integers */
    struct solo_list_mgmt *slm,
    unsigned char *a,
    int m,
    int n
)
{
    int ii=m/2, s=0, nn=(n+1)/2;
    int jj = m;
    short ival;
    unsigned char *bb = a + m;
    char str[1024];
    char *aa = str;
    
    solo_reset_list_entries(slm);

    for(; nn-- > 0; ii++, bb += 2, jj += 2) {
	if( s == 0 ) {
	    sprintf( aa, "%5d)", jj);	/* new line label */
	    aa += strlen(aa);
	    s = 6;
	}
	memcpy(&ival, bb, 2);
	sprintf( aa, "%6d", ival);
	aa += strlen(aa);
	*aa = naught;
	s += 6;
	
	if( s >= 66 ) {
	    aa = str;
	    solo_add_list_entry(slm, aa);
	    s = 0;
	}
    }
    if( s > 0 ) {
	aa = str;
	solo_add_list_entry(slm, aa);
    }
    slm_print_list(slm);
}
/* c-------------------------------------------------------------------------*/

void 
slm_xctypeu16 (	/* type as 16 bit integers */
    struct solo_list_mgmt *slm,
    unsigned char *a,
    int m,
    int n
)
{
    int ii=m/2, s=0, nn=(n+1)/2;
    int jj = m;
    short ival;
    unsigned char *bb = a + m;
    char str[1024];
    char *aa = str;
    
    solo_reset_list_entries(slm);

    for(; nn-- > 0; ii++, bb += 2, jj += 2) {
	if( s == 0 ) {
	    sprintf( aa, "%5d)", jj);	/* new line label */
	    aa += strlen(aa);
	    s = 6;
	}
	swack2((char *)bb, (char *)&ival);  //jul 26, 2011
	sprintf( aa, "%6d", ival);
	aa += strlen(aa);
	*aa = naught;
	s += 6;
	
	if( s >= 66 ) {
	    aa = str;
	    solo_add_list_entry(slm, aa);
	    s = 0;
	}
    }
    if( s > 0 ) {
	aa = str;
	solo_add_list_entry(slm, aa);
    }
    slm_print_list(slm);
}
/* c-------------------------------------------------------------------------*/

void 
Xctypeu16 (	/* type as 16 bit integers */
    unsigned char *a,
    int m,
    int n
)
{
    int ii=m/2, s=0, nn=(n+1)/2;
    int jj = m;
    short ival;
    unsigned char *bb = a + m;
    
    for(; nn-- > 0; ii++, bb += 2, jj += 2) {
	if( s == 0 ) {
	    printf("%5d)", jj);	/* new line label */
	    s = 6;
	}
	swack2((char *)bb, (char *)&ival);  //Jul 26, 2011
	printf( "%6d", ival);
	s += 6;
	
	if( s >= 66 ) {
	    printf( "\n" );	/* start a new line */
	    s = 0;
	}
    }
    if( s > 0 ) {
	printf( "\n" );
    }
}
/* c------------------------------------------------------------------------ */

int 
dd_crack_datime (const char *datime, const int nc, DD_TIME *dts)
{
    int yy, mon, dd, hh=0, mm=0, ss=0, ms=0;
    int ii, nn;
    char str[32], *aa, *bb, *cc;
    double d;


    strncpy(str, datime, nc);
    str[nc] = '\0';
    for(aa=str; *aa && (*aa == ' ' || *aa == '\t'); aa++);
    if((*aa == '+' || *aa == '-')) {
	return(NO);		/* relative */
    }

    /* this string can be of the form mon/dd/yy:hh:mm:ss.ms */

    if (bb = strrchr (aa, '.')) { /* contains milliseconds */
	d = atof(strrchr(aa, '.')); /* capture leading zeroes */
	ms = (int) (d * 1000.);
    }

    if((nn = sscanf(aa, "%d/%d/%d:%d:%d:%d"
	     , &mon, &dd, &yy, &hh, &mm, &ss)) == 6) {
	/* assume this string is of the form mon/dd/yy:hh:mm:ss */
    }
    else if((nn = sscanf(aa, "%d/%d/%d%d:%d:%d"
	     , &mon, &dd, &yy, &hh, &mm, &ss)) == 6) {
	/* assume this string is of the form mon/dd/yy hh:mm:ss */
    }
    else if((nn = sscanf(aa, "%d/%d/%d:%d:%d"
			 , &mon, &dd, &yy, &hh, &mm)) == 5) {
	/* assume this string is of the form mon/dd/yy:hh:mm */
    }
    else if((nn = sscanf(aa, "%d/%d/%d%d:%d"
			 , &mon, &dd, &yy, &hh, &mm)) == 5) {
	/* assume this string is of the form mon/dd/yy hh:mm */
    }
    else if((nn = sscanf(aa, "%d/%d/%d:%d", &mon, &dd, &yy, &hh)) == 4) {
	/* assume this string is of the form mon/dd/yy:hh */
    }
    else if((nn = sscanf(aa, "%d/%d/%d%d", &mon, &dd, &yy, &hh)) == 4) {
	/* assume this string is of the form mon/dd/yy hh */
    }
    else if((nn = sscanf(aa, "%d:%d:%d", &hh, &mm, &ss)) == 3) {
	/* assume this string is of the form hh:mm:ss */
	yy = mon = dd = 0;
    }
    else if((nn = sscanf(aa, "%d:%d", &hh, &mm)) == 2) {
	/* assume this string is of the form hh:mm */
	yy = mon = dd = 0;
    }
    else if((nn = sscanf(aa, "%d", &ii)) == 1) {
	if(!ii)
	      return(NO);
	yy = mon = dd = 0;
	if(!strncmp(aa, "0000", 4)) { /* has 4 leading zeroes */
	    ss = ii;
	    ii = hh = mm = 0;
	}
	else if(!strncmp(aa, "00", 2)) { /* has 2 leading zeroes */
	    hh = 0;
	    mm = ii/100;
	    ss = ii % 100;
	}
	else {
	    hh = ii % 100;
	    ii /= 100;
	    if(ii) {
		mm = hh;
		hh = ii % 100;
		ii /= 100;
	    }
	    if(ii) {
		ss = mm;
		mm = hh;
		hh = ii % 100;
		ii /= 100;
	    }
	}

	if(ii || hh > 23 || mm > 59 || ss > 59) {
	    return(NO);
	}
    }
    else {
	return(NO);
    }
    if(yy > 0)
	  dts->year = yy > 1900 ? yy : yy+1900;
    if(mon > 0)
	  dts->month = mon;
    if(dd > 0)
	  dts->day = dd;

    dts->day_seconds = D_SECS(hh,mm,ss,ms);
    return(YES);
}
/* c------------------------------------------------------------------------ */

char *
dd_est_src_dev_names_list (char *id, struct solo_list_mgmt *slm, char *src_dir)
{
    /* this routine sets up a list of dev names or most likely
     * file names associated with this "id" so we can cycle through
     * them using "dd_next_source_dev_name(id)"
     */
    int ii, mm, nn = slm->num_entries, size;
    struct source_devs_mgmt *sdm;
    char *aa, *bb, *dlims=(char *)", \t\n", str[256];

    sdm = (struct source_devs_mgmt *)malloc(sizeof(struct source_devs_mgmt));
    memset(sdm, 0, sizeof(struct source_devs_mgmt));

    if( nn < 1 )
	  return(NULL);


    sdm->dev_ptrs = (char **)malloc((nn+1)*sizeof(char **));
    memset(sdm->dev_ptrs, 0, (nn+1)*sizeof(char **));

    size = slm->sizeof_entries +1;
    mm = nn * sizeof(char) * size;
    aa = sdm->dev_names = (char *)malloc( mm );
    memset( aa, 0, mm );

    for( ii = 0; ii < nn; ii++ ) {
      sdm->dev_ptrs[ii] = aa;
      slash_path( aa, src_dir );
      strcat( aa, solo_list_entry( slm, ii ));
      aa += size;
    }

    sdm->id = (char *)malloc(strlen(id)+1);
    strcpy(sdm->id, id);

    sdm->next = devs_top;
    devs_top = sdm;
    return(*sdm->dev_ptrs);
}
/* c------------------------------------------------------------------------ */

char *
dd_establish_source_dev_names (char *id, char *names)
{
    /* this routine sets up a list of dev names or most likely
     * file names associated with this "id" so we can cycle through
     * them using "dd_next_source_dev_name(id)"
     */
    int ii, nn;
    struct source_devs_mgmt *sdm;
    char *aa, *dlims=(char *)", \t\n";

    sdm = (struct source_devs_mgmt *)malloc(sizeof(struct source_devs_mgmt));
    memset(sdm, 0, sizeof(struct source_devs_mgmt));

    if(!(nn = strlen(names)))
	  return(NULL);

    aa = sdm->dev_names = (char *)malloc((nn+1)*sizeof(char));
    strcpy(aa, names);

    for(aa=strtok(aa, dlims),nn=0;  aa; aa = strtok(NULL, dlims), nn++);

    sdm->dev_ptrs = (char **)malloc((nn+1)*sizeof(char **));
    memset(sdm->dev_ptrs, 0, (nn+1)*sizeof(char **));
    
    aa = sdm->dev_names;
    strcpy(aa, names);

    for(aa=strtok(aa, dlims),ii=0;  aa; aa = strtok(NULL, dlims), ii++) {
	*(sdm->dev_ptrs + ii) = aa;
    }
    sdm->id = (char *)malloc(strlen(id)+1);
    strcpy(sdm->id, id);

    sdm->next = devs_top;
    devs_top = sdm;
    return(*sdm->dev_ptrs);
}
/* c------------------------------------------------------------------------ */

void 
dd_gen_packet_info (struct input_read_info *iri, struct io_packet *dp)
{
    dp->ib = iri->top;
    dp->seq_num = iri->top->seq_num;
    dp->at = iri->top->at;
    dp->bytes_left = iri->top->bytes_left;
    dp->len = 0;
    return;
}
/* c------------------------------------------------------------------------ */

struct input_read_info * dd_init_io_structs(int index, int fmt)
{
    int ii, jj;
    struct input_read_info *iri;
    struct input_buf *next, *last;
    struct io_packet *db, *db_last;
    char *a, *c;

    num_io_structs++;
    iri = (struct input_read_info *)malloc(sizeof(struct input_read_info));
    memset(iri, 0, sizeof(struct input_read_info));
    /* implicit here is that io type starts out as PHYSICAL_TAPE
     */
    if(fmt == SIGMET_FMT || fmt == TOGA_FMT ) {
	iri->que_count = 16;
	iri->sizeof_bufs = K64/16;
    }
    else if(fmt == UF_FMT) {
	iri->que_count = 8;
	iri->sizeof_bufs = K64;
	iri->sizeof_bufs = 128 * 1024;
    }
    else if(fmt == ETL2M_FMT) {
	iri->que_count = 16;
	iri->sizeof_bufs = K64/2;
    }
    else if(fmt == WSR_88D_FMT || fmt == PIRAQX_FMT) {
	iri->que_count = 8;
	iri->sizeof_bufs = 96 * 1024;
    }
    else {
	iri->que_count = 8;
	iri->sizeof_bufs = K64;
    }
    iri->packet_count = MAX_LOG_PACKETS;
    read_io_list[index] = iri;
    for(ii=0; ii < iri->que_count; ii++) {
	next = (struct input_buf *)malloc(sizeof(struct input_buf));
	memset(next, 0, sizeof(struct input_buf));
	next->b_num = ii;

	if(ii) {
	    next->last = last;
	    last->next = next;
	    next->buf = c;
	}
	else {			/* first time through */
	    iri->top = next;
	    /*
	     * nab enough memory for all the physical record buffers
	     */
	    c = next->buf = (char *)malloc(iri->sizeof_bufs * iri->que_count);
	    memset(next->buf, 0, iri->sizeof_bufs * iri->que_count);
	}
	iri->top->last = next;
	next->next = iri->top;
	last = next;
	c += iri->sizeof_bufs;
    }

    for(ii=0; ii < iri->packet_count; ii++) {
	db = (struct io_packet *)malloc(sizeof(struct io_packet));
	memset(db, 0, sizeof(struct io_packet));

	if(ii) {
	    db->last = db_last;
	    db_last->next = db;
	}
	else {
	    iri->packet_que = db;
	}
	iri->packet_que->last = db;
	db->next = iri->packet_que;
	db_last = db;
    }
    return(iri);
}
/* c------------------------------------------------------------------------ */

void 
dd_input_read_close (struct input_read_info *iri)
{
   if (iri->io_type == PHYSICAL_TAPE)
     { close(iri->fid); }
   else
     { fclose (iri->strm); }
}
/* c------------------------------------------------------------------------ */

int 
dd_input_read_open (struct input_read_info *iri, char *dev_name)
{
    int ii;
    int32_t *lp, keep, keep2;
    char *aa=dev_name, *bb, *cc;
    fpos_t fpos;

    dd_reset_ios(iri, 1);
    iri->sizeof_file = 0;

    if(bb = strrchr(aa, '/')) {	/* nab the directory name */
	cc = iri->directory_name;
	for(; aa <= bb; *cc++ = *aa++); *cc++ = '\0';
	strcpy(iri->dev_name, dev_name);
    }
    else {
	if(!strlen(iri->directory_name))
	      strcpy(iri->directory_name, "./");
	strcpy(iri->dev_name, iri->directory_name);
	strcat(iri->dev_name, dev_name);
    }
    if(strstr(iri->dev_name, "/dev/")) {
	iri->io_type = PHYSICAL_TAPE;
    }
    else {
	iri->io_type = BINARY_IO;
     }
    printf( "Input file name: %s\n", iri->dev_name);

    if (iri->io_type == PHYSICAL_TAPE) {
       if((iri->fid = open(iri->dev_name, 0 )) < 0) { 
	  printf( "Could not open input file %s  error=%d\n"
		 , iri->dev_name, iri->fid);
	  exit(1);
       }
    }
    else {
       if(!(iri->strm = fopen(iri->dev_name, "r" ))) { 
	  perror (iri->dev_name);
	  exit(1);
       }
    }

    if (iri->io_type != PHYSICAL_TAPE) {

       ii = fseek(iri->strm, 0L, SEEK_END);
       iri->sizeof_file = ftell(iri->strm);
       ii = fseek(iri->strm, 0L, SEEK_SET); /* postion back to beginning */
       iri->current_offset = ftell(iri->strm);

	if(cc = get_tagged_string("IO_TYPE")) {
	    if(strstr(cc, "FB_IO"))
		  iri->io_type = FB_IO;
	    else if(strstr(cc, "PHYSICAL_TAPE"))
		  iri->io_type = PHYSICAL_TAPE;
	}
	else {
	    dd_phys_read(iri);
	    if(hostIsLittleEndian()) {
	       lp = &keep2;
//	       swack4(iri->top->buf, lp);  //Jul 26, 2011
	       swack4(iri->top->buf, (char *)lp);  //Jul 26, 2011
	    }
	    else {
	       lp = (int32_t *)iri->top->buf;
	    }
	    if(*lp > 0 && *lp <= K64-8 && *lp <= iri->sizeof_file) {
		/* this might be FB_IO */
		aa = iri->top->buf + *lp +sizeof(int32_t);
		if(hostIsLittleEndian()) {
//		   swack4(aa, &keep);  //Jul 26, 2011
		   swack4(aa, (char *)&keep);  //Jul 26, 2011
		}
		else {
		   memcpy(&keep, aa, 4);
		}
		if(keep == *lp) {
		    iri->io_type = FB_IO;
		}
	    }
	    dd_rewind_dev(iri);
	}	
    }
    return((iri->io_type == PHYSICAL_TAPE) ? iri->fid : 1);
}
/* c------------------------------------------------------------------------ */

void 
dd_io_reset_offset (struct input_read_info *iri, int32_t offset)
{
    int32_t nn;
    /* reset the current offset
     * and forget read ahead records
     */
    iri->read_count -= iri->read_ahead_count;
    nn = fseek(iri->strm, offset, SEEK_SET);
    iri->current_offset = ftell (iri->strm);
}
/* c------------------------------------------------------------------------ */

int 
dd_logical_read (struct input_read_info *iri, int direction)
{
    int ii;

    if(direction == FORWARD) {
	if(iri->read_ahead_count > 0) {
	    iri->read_ahead_count--;
	    iri->top = iri->top->next;
	}
	else {			/* we have to do it physically */
	    iri->top = iri->top->next;
	    return(dd_phys_read(iri));
	}
    }
    else if(direction == BACKWARD) {
	if(iri->read_count > 1 &&
	   iri->read_ahead_count < iri->read_count-1) {
	    /* we can go back to the previous record
	     * without any physical io
	     */
	    iri->read_ahead_count++;
	    iri->top = iri->top->last;
	}
	else {			/* we have to do it physically */
	    dd_really_skip_recs(iri, direction, 2);
	    return(dd_phys_read(iri));
	}
    }
    iri->top->at = iri->top->buf;
    iri->top->bytes_left = iri->top->sizeof_read;
    return(iri->top->read_state);
}
/* c------------------------------------------------------------------------ */

char *
dd_next_source_dev_name (char *id)
{
    struct source_devs_mgmt *sdm=devs_top;

    for(; sdm; sdm = sdm->next) {
	if(!strcmp(sdm->id, id))
	      break;
    }
    if(!sdm)
	  return(NULL);

    if(*(sdm->dev_ptrs + sdm->dev_count))
	  return(*(sdm->dev_ptrs + sdm->dev_count++));

    return(NULL);
}
/* c------------------------------------------------------------------------ */

int 
dd_phys_read (struct input_read_info *iri)
{
    int ii, mark;

    iri->seq_num++;
    iri->top->seq_num = iri->seq_num;
    iri->top->at = iri->top->buf;
    iri->read_ahead_count = 0;
    memset(iri->top->buf, 0, 8);

    if(iri->io_type == PHYSICAL_TAPE) {
	iri->top->read_state = read(iri->fid, iri->top->buf, iri->sizeof_bufs);
	if(iri->top->read_state > 0)
	      iri->f_bytes += iri->top->read_state;
	if(iri->f_bytes > 2.e9) {
	    printf("Reset input dev: %d at %.3f MB\n"
		   , iri->fid, iri->f_bytes*1.e-6);
	    mark = lseek(iri->fid, 0L, SEEK_SET); /* reset klooge for 2GB limit */
	    iri->f_bytes = 0;
	}
    }
    else if(iri->io_type == FB_IO) {
	iri->top->offset = ftell(iri->strm);
	if(iri->current_offset >= iri->sizeof_file) 
	      iri->top->read_state = -1;
	else 
	      iri->top->read_state = ffb_read(iri->strm, iri->top->buf
					     , iri->sizeof_bufs);
	iri->current_offset = ftell(iri->strm);
    }
    else if(iri->io_type == BINARY_IO) {
	iri->top->offset = ftell(iri->strm);
	if(iri->current_offset >= iri->sizeof_file) 
	      iri->top->read_state = -1;
	else { 
	   iri->top->read_state = fread((void *)iri->top->buf, 1
					, iri->sizeof_bufs, iri->strm);
	   iri->current_offset = ftell(iri->strm);

	   if (feof(iri->strm)) {
	      iri->top->read_state = 0;
	   }
	   else if (ferror (iri->strm)) {
	      iri->top->read_state = -1;
	      perror ("Input dev error");
	   }
	}
    }    
    else {
	printf("iri->io_type: %d has gone awry\n", iri->io_type);
	exit(0);
    }
    if(iri->read_count < iri->que_count) {
	iri->read_count++;
    }
    iri->top->eof_flag = iri->top->read_state == 0;
    iri->top->bytes_left = iri->top->sizeof_read =
	  iri->top->read_state > 0 ? iri->top->read_state : 0;

    return(iri->top->read_state);
}
/* c------------------------------------------------------------------------ */

int 
dd_really_skip_recs (struct input_read_info *iri, int direction, int skip_count)
{
    int ii, nn, mark;
    int32_t ll, rlen;
#ifdef HAVE_SYS_MTIO_H
    struct mtop op;
#endif /* ifdef HAVE_SYS_MTIO_H */
    char mess[256];

    dd_reset_ios(iri, 0);
    iri->top->skip_state = skip_count;

#ifdef HAVE_SYS_MTIO_H
    if(iri->io_type == PHYSICAL_TAPE) {
	op.mt_count = skip_count;
	op.mt_op = direction == FORWARD ? MTFSR : MTBSR;
	if((ii = ioctl(iri->fid, MTIOCTOP, &op)) < 0 ) {
	    sprintf(mess,
"Skip records error on %s\n count: %d dir: %d fid: %d state: %d\n"
		    , iri->dev_name, skip_count, direction
		    , iri->fid, iri->top->skip_state);
	    perror(mess);
	    iri->top->skip_state = ii;
	}
    }
    else 
#endif /* ifdef HAVE_SYS_MTIO_H */
    if(iri->io_type == FB_IO) {
	/* record headers and trailers should be 4 bytes
	 */
       for (ii=0; ii < skip_count; ii++) {
	  mark = (direction == FORWARD) ? ffb_skip_one_fwd (iri->strm) :
	    ffb_skip_one_bkw (iri->strm);  
	  iri->current_offset = ftell (iri->strm);
	  if (mark < 0)
	    { break; }
       }
       iri->top->skip_state = ii;
	if(skip_count > 0) {
	    mark = 0;
	}
    }
    else {
	/* assume binary io */
	iri->current_offset = ftell(iri->strm);

	if(direction == FORWARD) {
	    if(iri->current_offset +skip_count*iri->sizeof_bufs <
	       iri->sizeof_file)
	      {
		 ii = fseek (iri->strm, skip_count*iri->sizeof_bufs
			     , SEEK_CUR);
		 iri->current_offset = ftell (iri->strm);
		 iri->top->skip_state = skip_count;
	      }
	    else {
	       iri->top->skip_state = -1;
	    }
	 }
	else {			/* backward */
	    if(iri->current_offset -skip_count*iri->sizeof_bufs >= 0) {
		ii = fseek(iri->strm, -skip_count*iri->sizeof_bufs
			   , SEEK_CUR);
		iri->current_offset = ftell (iri->strm);
		iri->top->skip_state = skip_count;
	    }
	    else {
		iri->top->skip_state = -1;
	    }
	}
    }
    iri->top->at = iri->top->buf;
    iri->top->bytes_left = 0;
    return(iri->top->skip_state);
}
/* c------------------------------------------------------------------------ */

double
dd_relative_time(const char *rtm)
{
    /* routine to detect and extract relative time in seconds
     */
    const char *aa;
    char *bb, *cc, str[64], hms;
    int ii, nn, relative;
    double d=0, dsign=1;
    float f_val;
    DD_TIME dts;

    if(!strlen(rtm))
	  return(d);

    if(strchr(rtm, '/'))	/* it's probably a date */
	  return(d);

# ifdef obsolete
    if((ii = sscanf(rtm, "%f", &f_val)) == 1) {
	if(FABS(f_val) < 100) {
	    return((double)f_val);
	}
    }
    if(dd_crack_datime(bb, strlen(bb), &dts)) {
	return(d);
    }
# endif
    /*
     * relative implies a number preceeded by a + or - and
     * optionally followed by "h", "m", "s", or not implying seconds
     *
     * first skip over leading blanks or tabs
     */
    for(aa=rtm; *aa && (*aa == ' ' || *aa == '\t'); aa++);

    if(!(*aa == '+' || *aa == '-')) {
	return(d);		/* must be preceeded by "+" or "-". */
    }
    dsign = *aa == '-' ? -1. : 1.;

    /* remove other blanks and tabs from the string */
    for(++aa,bb=cc=str; *aa; aa++) { 
	if(!(*aa == ' ' || *aa == '\t'))
	      *cc++ = *aa;
    }
    *cc = '\0';
    if(!strlen(bb))
	  return(d);

    hms = (unsigned char)(*(cc-1)); /* last character */

    if(isalpha(hms)) {
	if(isupper(hms)) hms = tolower(hms);
	hms = (hms == 'h' || hms == 'm' || hms == 's') ? hms : 0;
	if(!hms)
	      return(d);
	*(cc-1) = '\0';
    }
    else
	  hms = 0;

    if(!strlen(bb))
	  return(d);

# ifdef obsolete
    relative = hms ? YES : NO;

    if(!isdigit((int)(*bb))) {	/* get the sign */
	dsign = *bb == '-' ? -1. : 1.;
	bb++;
	relative = YES;
    }
    if(!relative || strchr(bb, '/'))
	  return(d);
# endif

    if(strchr(bb, ':')) {
	/* try and decode it as hh:mm:ss.ms
	 */
	if(dd_crack_datime(bb, strlen(bb), &dts))
	      d = dsign * dts.day_seconds;
    }
    else if((nn = sscanf(bb, "%f", &f_val)) == 1) {
	if(hms == 'h') {
	    d = dsign * f_val * 3600.;
	}
	else if(hms == 'm') {
	    d = dsign * f_val * 60.;
	}
	else {			/* absence of a character
				 * also implies seconds */
	    d = dsign * f_val;
	}
    }
    return(d);
}
/* c------------------------------------------------------------------------ */

void 
dd_reset_ios (struct input_read_info *iri, int level)
{
    int ii;
    struct input_buf *ib;

    iri->read_ahead_count = iri->read_count = 0;

    if(level == 1) {
	iri->f_bytes =
	      iri->current_offset =
		    iri->top->skip_state = 0;
    }

    for(ii=0; ii < iri->que_count; ii++) {
	ib = iri->top = iri->top->next;
	ib->seq_num = 0;
	ib->bytes_left = 0;
    }
}
/* c------------------------------------------------------------------------ */

struct io_packet *
dd_return_current_packet (struct input_read_info *iri)
{
    return(iri->packet_que);
}
/* c------------------------------------------------------------------------ */

struct input_read_info *
dd_return_ios(int index, int fmt)
{
    int ii;

    if(index < 0 || index >= MAX_OPENED_FILES) {
	return NULL;
    }

    if(!num_io_structs) {
	for(ii=0; ii < MAX_OPENED_FILES; ii++) {
	    read_io_list[ii] = NULL;
	}
    }
    if(read_io_list[index])
	  return(read_io_list[index]);

    return(dd_init_io_structs(index, fmt));
}
/* c------------------------------------------------------------------------ */

struct io_packet *
dd_return_next_packet (struct input_read_info *iri)
{
    struct io_packet *dp;

    dp = iri->packet_que = iri->packet_que->next;
    dd_gen_packet_info(iri, dp);
    return(dp);
}
/* c------------------------------------------------------------------------ */

void 
dd_rewind_dev (struct input_read_info *iri)
{
#ifdef HAVE_SYS_MTIO_H
    struct mtop op;
#endif /* ifdef HAVE_SYS_MTIO_H */
    char mess[256];
    int mark;

    dd_reset_ios(iri, 1);

#ifdef HAVE_SYS_MTIO_H
    if(iri->io_type == PHYSICAL_TAPE) {
	op.mt_op = MTREW;
	op.mt_count = 1;	/* Only rewind once */
	if((iri->top->skip_state = ioctl(iri->fid, MTIOCTOP, &op)) < 0 ) {
	    sprintf(mess, "Rewind error on %s fid: %d state: %d\n"
		    , iri->dev_name, iri->fid, iri->top->skip_state);
	    perror(mess);
	}
    }
    else
#endif /* ifdef HAVE_SYS_MTIO_H */
    {
	mark = fseek(iri->strm, 0L, SEEK_SET);
	iri->current_offset = ftell (iri->strm);
    }
}
/* c------------------------------------------------------------------------ */

void 
dd_skip_files (struct input_read_info *iri, int direction, int skip_count)
{
    int ii, jj, mark, ok=YES;
    int32_t ll, rlen;
#ifdef HAVE_SYS_MTIO_H
    struct mtop op;
#endif /* ifdef HAVE_SYS_MTIO_H */
    char mess[256];

    dd_reset_ios(iri, 0);
    iri->top->skip_state = skip_count;

#ifdef HAVE_SYS_MTIO_H
    if(iri->io_type == PHYSICAL_TAPE) {
	op.mt_count = skip_count;
	op.mt_op = direction == FORWARD ? MTFSF : MTBSF;
	if((iri->top->skip_state = ioctl(iri->fid, MTIOCTOP, &op)) < 0 ) {
	    sprintf(mess,
"Skip files error on %s\n count: %d dir: %d fid: %d state: %d\n"
		    , iri->dev_name, skip_count, direction
		    , iri->fid, iri->top->skip_state);
	    perror(mess);
	}
     }
    else 
#endif /* ifdef HAVE_SYS_MTIO_H */ 
    if(iri->io_type == FB_IO) {
	/* record headers and trailers should be 4 bytes
	 */
	if(direction == FORWARD) {
	    for(; skip_count > 0; skip_count--) {
		for(ok=YES;;) {
		   mark = (direction == FORWARD) ? ffb_skip_one_fwd (iri->strm)
		     : ffb_skip_one_bkw (iri->strm);
                   if (mark <= 0) {
		      if (mark < 0)
			{ ok = NO; }
		      break;
		   }
		}
		if(!ok) {
		    break;
		}
	    }
	    iri->top->skip_state = (ok) ?  0: -1;
	 }
     }
    else {
	/* assume binary io */
	sprintf(mess, "NO file skipping capability in binary io\n");
	perror(mess);
     }
 }
/* c------------------------------------------------------------------------ */

int 
dd_skip_recs (struct input_read_info *iri, int direction, int skip_count)
{
    /* move in the direction indicated and read the next record
     */
    int ii, mark;
    char mess[256];

    if(skip_count <= 0)
	  return(iri->top->read_state);

    iri->top->bytes_left = 0;
    iri->top->at = iri->top->buf;


    if(direction == FORWARD &&
       skip_count < iri->read_ahead_count) {
	for(ii=0; ii <= skip_count; ii++) {
	    iri->top = iri->top->next;
	    iri->top->at = iri->top->buf;
	    iri->top->bytes_left = 0;
	    iri->read_ahead_count--;
	}
	return(iri->top->read_state);
    }
    if(direction == BACKWARD &&
       skip_count <= (iri->read_count - iri->read_ahead_count)) {
	for(ii=0; ii < skip_count; ii++) {
	    iri->top = iri->top->last;
	    iri->top->at = iri->top->buf;
	    iri->top->bytes_left = 0;
	    iri->read_ahead_count++;
	}
	return(iri->top->read_state);
    }
    /* to get where we want to go there must be some physical io
     */
    if(direction == FORWARD)
	skip_count -= iri->read_ahead_count;
    else if(iri->read_count > 0)
	skip_count += iri->read_ahead_count;

    if(direction == FORWARD && skip_count < 2*iri->que_count) {
	/* just read all the records into the que so they'll be there
	 * if we want to back up a few
	 */
	for(ii=skip_count; ii > 0; ii--) {
	    iri->top = iri->top->next;
	    if(dd_phys_read(iri) < 0)
		  break;
	}
	if(ii) {
	    sprintf(mess,
		    "Trouble skipping %d blocks on %s\n fid: %d state: %d\n"
		    , skip_count, iri->dev_name
		    , iri->fid, iri->top->read_state);
	    perror(mess);
	}
	return(iri->top->read_state);
    }
    /* backwards!
     */
    dd_really_skip_recs(iri, direction, skip_count);
    return(iri->top->skip_state);
}
/* c------------------------------------------------------------------------ */

void 
dd_unwind_dev (struct input_read_info *iri)
{
    char mess[256];
    int ii;

    dd_reset_ios(iri, 0);

    if(iri->io_type == PHYSICAL_TAPE) {
	    sprintf(mess, "Unwind undefined for PHYSICAL TAPE\n");
	    perror(mess);
    }
    else {
       fseek(iri->strm, 0L, SEEK_END);
	iri->current_offset = iri->sizeof_file = ftell(iri->strm);
    }
}
/*c----------------------------------------------------------------------*/

void 
ezascdmp (		/* ascii dump of n bytes */
    char *b,
    int m,
    int n
)
{
    int s=0;
    char *c=b+m;
    
    for(; 0 < n--; c++,m++) {
	if( s == 0 ) {
	    printf("%5d) ", m );	/* new line label */
	}
	if( *c > 31 && *c < 127 )
	     putchar(*c);	/* print actual char */
	else
	     putchar('.');
	s++;
	
	if( s >= 40 ) {
	    putchar('\n');	/* start a new line */
	    s = 0;
	}
    }
    if( s > 0 )
	 printf( "\n" );
}
/*c----------------------------------------------------------------------*/

void 
slm_ezascdmp (		/* ascii dump of n bytes */
    struct solo_list_mgmt *slm,
    char *b,
    int m,
    int n
)
{
    int s=0;
    char *c=b+m;
    char str[1024];
    char *aa = str;

    solo_reset_list_entries(slm);
    
    for(; 0 < n--; c++,m++) {
	if( s == 0 ) {
	    sprintf( aa, "%5d) ", m );	/* new line label */
	    aa += strlen(aa);
	}
	if( *c > 31 && *c < 127 ) {
	    *aa++ = *c;		/* print actual char */
	}	
	else {
	    *aa++ = '.';
	}
	*aa = naught;
	s++;
	
	if( s >= 40 ) {
	    aa = str;
	    solo_add_list_entry(slm, aa);
	    s = 0;
	}
    }
    if( s > 0 ) {
	aa = str;
	solo_add_list_entry(slm, aa);
    }
    slm_print_list(slm);
}
/*c----------------------------------------------------------------------*/

void 
ezhxdmp (		/* hexdump of n bytes */
    char *b,
    int m,
    int n
)
{
    int c, i, s;
    char h[20];
    sprintf( h, "0123456789abcdef" );   /* adds a '\0'to the string */
    
    for(i=1,s=0; 0 < n--; i++,m++) {
	if( s == 0 ) {
	    printf("%5d) ", m);	/* new line label */
	}
	/* get upper and lower nibble */
	c = ((int32_t) *(b+m) >>4) & 0xf;
	putchar(*(h+c));	/* print actual hex char */
	c = (int32_t) *(b+m) & 0xf; 
	putchar(*(h+c));
	s += 2;
	
	if( s > 39 ) {
	    putchar('\n');	/* start a new line */
	    s = 0;
	}
	else if(s%4 == 0 ) {
	    putchar(' ');	/* space every 4 char */
	}
    }
    if( s > 0 ) printf( "\n" );
}
/*c----------------------------------------------------------------------*/

void 
slm_ezhxdmp (		/* hexdump of n bytes */
    struct solo_list_mgmt *slm,
    char *b,
    int m,
    int n
)
{
    int c, s;
    char ascii[16], *as;
    char h[20];
    char str[1024];
    char *aa = str, *bb = b+m;
    strcpy (ascii, " ");

    solo_reset_list_entries(slm);
    sprintf( h, "0123456789abcdef" );   /* adds a '\0'to the string */
    
    for(s=0; 0 < n--; bb++,m++) {
	if( s == 0 ) {
	    sprintf( aa, "%5d) ", m); /* new line label */
	    aa += strlen(aa);
	    as = ascii+1;
	}
	/* get upper and lower nibble */
	c = ((int32_t) *bb >>4) & 0xf;
	*aa++ = *(h+c);	/* actual hex char */
	c = (int32_t) *bb & 0xf; 
	*aa++ = *(h+c);	/* actual hex char */
	s += 2;
	*aa = naught;

	if( *bb > 31 && *bb < 127 ) {
	    *as++ = *bb;		/* print actual char */
	}	
	else {
	    *as++ = '.';
	}
	*as = naught;
	
	if( s > 39 ) {
	    aa = str;
	    strcat (aa, ascii);
	    solo_add_list_entry(slm, aa);
	    s = 0;
	}
	else if(s%4 == 0 ) {
	    *aa++ = ' ';	/* space every 4 char */
	}
	*aa = naught;
    }
    if( s > 0 ) {
	aa = str;
	strcat (aa, ascii);
	solo_add_list_entry(slm, aa);
    }
    slm_print_list(slm);
}
/* c---------------------------------------------------------------------- */

int 
getreply (char *s, int lim)
{
    int c, ii=0, nn, mark=EOF;
    char *a=s;

    *a = '\0';

    for(;;) {
	if(lim-- <= 0)
	      break;

	c = getc(stdin);

	if(c == '\n')
	      break;

	if(c == EOF)
	      break;

	*a++ = c;
	ii++;
    }

    *a = '\0';
    return(ii);
}
 /* c---------------------------------------------------------------------- */

int 
oketype ( /* determine if string is real number */
    char s[],
    int n,
    float *val
)
{
    int d, i, ival;
    float tenx=1.0;

    *val = 0;

    for( d=0; d < n && s[d] != 'e' && s[d] != 'E'; d++ )
	 ;
    i = okreal( s, d, val ); /* get the real part */
    
    if( d == n ) {
	if( !i )
	     return(0);
	return(1);
    }
    if( !okint( s+(d+1), n-(d+1), &ival)) /* get the power part */
	 return(0);

    if( ival < 0 )
	 while( ival++ < 0 )
	      tenx *= 0.1;
    else
	 while( ival-- > 0 )
	      tenx *= 10.0;

    *val *= tenx;

 /* this doesn't work!
    *val = (double)(*val) * rpow( 10, ival ); 
  */
    return(1);
}
/* c---------------------------------------------------------------------- */

int 
okint ( /* see if string is an integer */
    char s[],
    int n,
    int *ival
)
{
    int sign, i;
    
    *ival = 0;

    for( i=0; i < n && s[i] == ' '; i++ )  /* skip leading blanks */
    if( i == n )
	 return(1);

    for( ; --n >= i && s[n] == ' ';); /* skip trailing blanks */
    if( i > n )
	 return(1);
    
    sign = 1;
    if( s[i] == '+' )
	 i++;
    else if( s[i] == '-' ){
	sign = -1;
	i++;
    }
    for(; i <= n; i++ )
	 if( isdigit( s[i] ))
	      *ival = *ival*10 + s[i]-'0';
	 else {
	     *ival = 0;
	     return(0);
	 }
    *ival *= sign;
    return(1);
}
 /* c---------------------------------------------------------------------- */

int 
okreal ( /* determine if string is real number */
    char s[],
    int n,
    float *val
)
{
    int d, i, ival;
    float frak=1.0;

    *val = 0;
    for( d=0; d < n && s[d] != '.'; d++ )
	 ;
    i = okint( s, d, &ival ); /* get the whole part */
    *val = (float) ival;
    
    if( d == n ) {
	if( !i )
	     return(0);
	return(1);
    }
    i = n-d-1;
    if( !okint( s+d+1, i, &ival)) /* get the fraction */
	 return(0);

    while( i-- > 0 )
	 frak *= 0.1;

    for( i=0; i < n && s[i] != '-'; i++ )
	 ;
    if( i == n ) /* no minus sign */
	 *val += (float) ival * frak;
    else
	 *val -= (float) ival * frak;

    return(1);
}
# ifdef obsolete
/* c------------------------------------------------------------------------ */

int 
strnlen (char *str, int n)
{
    /* routine to return the length of string "s" but
     * don't look past the "n" th character for a null
     */
    int nc=0;
    for(; nc < n && *str++; nc++); /* non-zero char value */
    if(nc == n)
	  return(0);
    else
	  return(nc);
}
# endif
/* c------------------------------------------------------------------------ */

void 
gri_interest (struct generic_radar_info *gri, int verbose, char *preamble, char *postamble)
{
    /* print interesting info from the gri struct such as:
     * time
     * radar
     * fixed angle
     * prf
     * sweep num
     * azimuth
     * elevation
     * aircraft positioning info if this is aircraft data
     * lat/lon/alt
     * fields present
     * field data
     * c...mark
     */
    int ii, jj, nn;
    int ac_mode = gri->scan_mode == DD_SCAN_MODE_AIR ||
	gri->radar_type == DD_RADAR_TYPE_AIR_NOSE ||
      gri->radar_type == DD_RADAR_TYPE_AIR_LF;
    char str[256], *a;
    char scan_mode[8];

    if(verbose)
	  printf("\n");
    printf(" %s ", preamble);
    printf("%s ", dts_print(d_unstamp_time(gri->dts)));
    printf("%s ", gri->radar_name);
    if( ac_mode ) {
	printf("t:%5.1f ", gri->tilt);
	printf("rt:%6.1f ", gri->rotation_angle);
	printf("rl:%5.1f ", gri->roll);
	printf("h:%6.1f ", gri->heading);
	printf("al:%6.3f ", .001*gri->altitude);
    }
    else {
	printf("%s ", dd_scan_mode_mne(gri->scan_mode, scan_mode));
	printf("fx:%6.1f ", gri->fixed);
	printf("az:%6.1f ", gri->azimuth);
	printf("el:%6.2f ", gri->elevation);
	printf("prf: %6.1f ", gri->PRF);
    }
    printf(" %03d ", gri->source_sweep_num);
    printf("%s ", postamble);
    printf("\n");

    if(verbose) {
	printf(" ");
	printf("la:%9.4f ", gri->latitude);
	printf("lo:%9.4f ", gri->longitude);
	if( ac_mode ) {
	    printf("ag:%6.3f ", .001*gri->altitude_agl);
	    printf("p:%5.1f ", gri->pitch);
	    printf("d:%5.1f ", gri->drift);
	    printf("\n");
	    printf(" ");
	    printf("ve:%6.1f ", gri->vew);
	    printf("vn:%6.1f ", gri->vns);
	    printf("vv:%6.1f ", gri->vud);
	    printf("we:%6.1f ", gri->ui);
	    printf("wn:%6.1f ", gri->vi);
	    printf("wv:%6.1f ", gri->wi);
	}
	else {
	    printf("al:%6.3f ", gri->altitude);
	}
	printf("\n");

	printf(" Fields: ");
	for(ii=0; ii < gri->num_fields_present; ii++) {
	    printf("%s ", str_terminate(str, gri->field_name[ii], 8));
	}
	printf("\n");
	if(strlen(gri->source_field_mnemonics))
	      printf(" Raw fields: %s\n", gri->source_field_mnemonics);
    }

}
/* c------------------------------------------------------------------------ */

int 
gri_max_lines (void)
{
    static int count=0;
    char *a;
    int ii=0;

    if(!count++) {
	if(a=get_tagged_string("LINES_IN_WINDOW")) {
	    if((ii = atoi(a)) > 0)
		  lines_in_display = ii;
	}
    }

    return(lines_in_display);
}
/* c------------------------------------------------------------------------ */

int 
gri_nab_input_filters (double current_time, struct dd_input_filters *difs, int less)
{
    int ii, kk, mark, longer_menu = !less;
    int nn, ival;
    float val;
    char str[256];
    char *a;
    DD_TIME dts, dtx;
    float fp;
    double d;
    double t1, t2;

    /* c...mark */

 menu2:

    printf("\n");
    printf(" 0 = Cease\n");
    printf(" 1 = Enter time span\n");
    printf(" 2 = Enter scan count\n");
    printf(" 3 = Enter volume count\n");
    printf(" 4 = Enter ray count\n");
    if(longer_menu) {
	printf(" 5 = Enter fixed angle limits\n");
	printf(" 6 = Enter PRF limits\n");
	ii = 7;
    }
    else
	  ii = 5;
    printf("%2d = Enter num lines in browser display\n", ii);


    printf("Option = ");
    nn = getreply(str, sizeof(str));
    if(cdcode(str, nn, &ival, &val) != 1 || ival < -2 || ival > 11) {
	printf( "\nIllegal Option!\n" );
	goto menu2;
    }
    if(ival < 1)
	  return(ival);

    else if(ival == 1) {	/* time limits
				 */
	t1 = t2 = 0;
	dd_clear_dts(&dts);
	dd_clear_dts(&dtx);
	dts.time_stamp = current_time;
	d_unstamp_time(&dts);

	printf("Type start time (e.g. 14:22:55 or +2h, +66m, +222s): ");
	nn = getreply(str, sizeof(str));

	if(cdcode(str, nn, &ival, &val) == 1 && ival == 0) {
	    t1 = current_time;
	}
	else if(nn < 2) {
	    printf("\nIllegal time spec: %s\n", str);
	    goto menu2;
	}
	else if(d = dd_relative_time(str)) {
	    t1 = current_time +d;
	}
	else if(!(kk = dd_crack_datime(str, nn, &dtx))) {
	    printf("\nIllegal time spec: %s\n", str);
	    goto menu2;
	}
	else {
	    if(!dtx.year) dtx.year = dts.year;
	    if(!dtx.month) dtx.month = dts.month;
	    if(!dtx.day) dtx.day = dts.day;
	    t1 = d_time_stamp(&dtx);
	}

	printf("Type stop time (e.g. 14:22:55 or +2h, +66m, +222s): ");
	nn = getreply(str, sizeof(str));
	if(nn < 2)
	      goto menu2;
	if(d = dd_relative_time(str)) {
	    t2 = t1 +d;
	}
	else if(!(kk = dd_crack_datime(str, nn, &dtx))) {
	    printf("\nIllegal time spec: %s\n", str);
	    goto menu2;
	}
	else {
	    if(!dtx.year) dtx.year = dts.year;
	    if(!dtx.month) dtx.month = dts.month;
	    if(!dtx.day) dtx.day = dts.day;
	    t2 = d_time_stamp(&dtx);
	}
	ii = 0;
	difs->num_time_lims = ii+1;
	if(!difs->times[ii]) difs->times[ii] = 
	      (struct d_limits *)malloc(sizeof(struct d_limits));
	difs->times[ii]->lower = t1;
	difs->final_stop_time = 
	      difs->times[ii]->upper = t2;
	difs->stop_flag = NO;
	dtx.time_stamp = t1;
	printf("Start time: %s  ", dts_print(d_unstamp_time(&dtx)));
	dtx.time_stamp = t2;
	printf("Stop time: %s \n", dts_print(d_unstamp_time(&dtx)));
    }
    else if(ival == 2) {
	printf("Type number of sweeps to be read in: ");
	nn = getreply(str, sizeof(str));
	if(cdcode(str, nn, &ival, &val) != 1 || ival < 0) {
	    printf( "\nIllegal Option!\n" );
	    goto menu2;
	}
	difs->max_sweeps = ival;
	printf("\nMax sweep count: %d\n", difs->max_sweeps);
    }
    else if(ival == 3) {
	printf("Type number of volumes to be read in: ");
	nn = getreply(str, sizeof(str));
	if(cdcode(str, nn, &ival, &val) != 1 || ival < 0) {
	    printf( "\nIllegal Option!\n" );
	    goto menu2;
	}
	difs->max_vols = ival;
	printf("\nMax volume count: %d\n", difs->max_vols);
    }
    else if(ival == 4) {
	printf("Type number of rays to be read in: ");
	nn = getreply(str, sizeof(str));
	if(cdcode(str, nn, &ival, &val) != 1 || ival < 0) {
	    printf( "\nIllegal Option!\n" );
	    goto menu2;
	}
	difs->max_beams = ival;
	printf("\nMax ray count: %d\n", difs->max_beams);
    }
    else if(ival == 5 && longer_menu) {
	printf("Type lower fixed angle limit: ");
	nn = getreply(str, sizeof(str));
	if(cdcode(str, nn, &ival, &val) < 1 || ival < 0) {
	    printf( "\nIllegal Option!\n" );
	    goto menu2;
	}
	fp = val;
	printf("Type upper fixed angle limit: ");
	nn = getreply(str, sizeof(str));
	if(cdcode(str, nn, &ival, &val) < 1 || ival < 0) {
	    printf( "\nIllegal Option!\n" );
	    goto menu2;
	}
	if((d=angdiff(fp, val) < 0)) {
	    printf("Illegal fixed angle limits: %.2f %.2f\n", fp, val);
	}
	ii = 0;
	difs->num_fixed_lims = ii+1;
	if(!difs->fxd_angs[ii])
	      difs->fxd_angs[ii] = (struct d_limits *)
		    malloc(sizeof(struct d_limits));
	difs->fxd_angs[ii]->lower = fp;
	difs->fxd_angs[ii]->upper = val;
	printf("\nFixed angle limits: %.2f %.2f\n", fp, val);
    }
    else if(ival == 6 && longer_menu) {
	printf("Type lower PRF limit: ");
	nn = getreply(str, sizeof(str));
	if(cdcode(str, nn, &ival, &val) < 1 || ival < 0) {
	    printf( "\nIllegal Option!\n" );
	    goto menu2;
	}
	fp = val;
	printf("Type upper PRF limit: ");
	nn = getreply(str, sizeof(str));
	if(cdcode(str, nn, &ival, &val) < 1 || ival < 0) {
	    printf( "\nIllegal Option!\n" );
	    goto menu2;
	}
	if(fp > val) {
	    printf("Illegal PRF limits: %.2f %.2f\n", fp, val);
	}
	ii = 0;
	difs->num_prf_lims = ii+1;
	if(!difs->prfs[ii])
	      difs->prfs[ii] = (struct d_limits *)
		    malloc(sizeof(struct d_limits));
	difs->prfs[ii]->lower = fp;
	difs->prfs[ii]->upper = val;
	printf("\nPRF limits: %.2f %.2f\n", fp, val);
    }
    else if(ival == 5 || ival == 7) {
	printf("Type lines in display: ");
	nn = getreply(str, sizeof(str));
	if(cdcode(str, nn, &ival, &val) < 1 || ival < 0) {
	    printf( "\nIllegal Option!\n" );
	    goto menu2;
	}
	gri_set_max_lines((int)ival);
    }
    goto menu2;
}
/* c------------------------------------------------------------------------ */

void 
gri_print_list (char **ptrs)
{
    char **pt=ptrs;
    int ii, jj, kk, ll=0, mm, nl, max=gri_max_lines(), last_mm;
    int nn, ival;
    float val;
    char str[128];

    for(nl=0; *pt; pt++, nl++);	/* count number of lines to print */

    if(nl < max) max = nl;

    last_mm = mm = max;

    for(;;) {
	for(pt=ptrs+ll; mm--; pt++) {
	    printf("%s\n", *pt);
	}
	printf("Hit <return> to continue ");
	nn = getreply(str, sizeof(str));
	if(cdcode(str, nn, &ival, &val) != 1 || ival == 1) {
	    return;
	}
	mm = max;
	if(ival > 0) {
	    if(ll +ival +mm > nl) {
		ll = nl -mm;
	    }
	    else 
		  ll += ival;
	}
	else if( ival < 0) {
	    ll = ll +ival < 0 ? 0 : ll + ival;
	}
	else {
	    if(ll+last_mm == nl)
		  return;
	    ll += last_mm;
	    if(ll +mm > nl) {
		mm = nl -ll;
	    }
	}
	last_mm = mm;
    }
}
/* c------------------------------------------------------------------------ */

void 
gri_set_max_lines (int nlines)
{
    if(nlines > 0)
	  lines_in_display = nlines;
}
/* c------------------------------------------------------------------------ */

int 
gri_start_stop_chars (int *view_char, int *view_char_count)
{
    int nn, ival;
    float val;
    char str[256];
    double d;

    printf("Type starting character number (=%d): "
	   , *view_char+1);
    nn = getreply(str, sizeof(str));
# ifdef obsolete
    if(cdcode(str, nn, &ival, &val) != 1 || ival < 0 ||
       fabs((double)val) > K64) {
# else
    if(cdcode(str, nn, &ival, &val) != 1 || 
       fabs((double)val) > K64) {
# endif
	printf( "\nIllegal Option!\n" );
	return(-1);
    }
    if(ival) {
	*view_char = ival-1;
	printf("Type character count (=%d): "
	       , *view_char_count);
	nn = getreply(str, sizeof(str));
	if(cdcode(str, nn, &ival, &val) != 1 || ival < 0 ||
	   fabs((double)val) > K64) {
	    printf( "\nIllegal Option!\n" );
	    return(-1);
	}
	if(ival) {
	    *view_char_count = ival;
	}
    }
    return(1);
}
/* c------------------------------------------------------------------------ */

