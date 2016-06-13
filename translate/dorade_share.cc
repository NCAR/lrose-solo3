/* 	$Id$	 */

#include <iostream>
# include <sys/types.h>
# include <dirent.h>


#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */
/*
 * This file contains the following routines
 * 
 * angdiff
 * cascdmp
 * chexdmp
 * cpy_bfill_str
 * dcdatime
 * dd_blank_fill
 * dd_chop
 * dd_crack_file_name
 * dd_crack_file_name_ms
 * dd_file_name
 * dd_file_name_ms
 * 
 * dd_hrd16
 * dd_hrd16_uncompressx
 * dd_nab_floats
 * dd_nab_ints
 * dd_rdp_uncompress8
 * dd_scan_mode_mne
 * dd_unquote_string
 * dorade_time_stamp
 * dts_print
 * eldora_time_stamp
 * fb_read
 * fb_write
 * fgetsx
 * file_time_stamp
 * fstring
 * get_tagged_string
 * gp_read
 * gp_write
 * in_sector
 * put_tagged_string
 * 
 * 
 * se_free_raqs
 * se_return_ravgs
 * slash_path
 * slm_print_list
 * solo_add_list_entry
 * solo_list_entry
 * solo_list_remove_dups
 * solo_list_remove_entry 
 * solo_list_sort_file_names
 * 
 * solo_malloc_list_mgmt
 * solo_malloc_pisp
 * solo_modify_list_entry
 * solo_reset_list_entries
 * solo_unmalloc_list_mgmt

 * str_terminate
 * time_now
 * todays_date
 * 
 * 
 */

#include <LittleEndian.hh>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "dorade_share.hh"
#include "dap_common.hh"
#include "ddb_common.hh"
#include "dd_io_mgmt.hh"
#include "dd_crackers.hh"
#include "to_fb.hh"

# ifdef HAVE_SYS_MTIO_H
#   include <sys/mtio.h>
# endif /* ifdef HAVE_SYS_MTIO_H */

#include <sys/ioctl.h>

# define MAX_TAGS 512
# define MAX_TAG_SIZE 32
# define MAX_STRING_SIZE 128

static char *Stypes[] = { (char *)"CAL", (char *)"PPI", (char *)"COP",
			  (char *)"RHI", (char *)"VER", (char *)"TAR",
			  (char *)"MAN", (char *)"IDL", (char *)"SUR",
			  (char *)"AIR", (char *)"???" };

static struct running_avg_que *top_raq=NULL;
static char *tags[MAX_TAGS], *tagged_strings[MAX_TAGS];
static int num_tags=0;

/* c------------------------------------------------------------------------ */


/* c------------------------------------------------------------------------ */


/* c------------------------------------------------------------------------ */


/* c------------------------------------------------------------------------ */

int dd_text_to_slm(const char *txt, struct solo_list_mgmt *slm)
{
  char *buf = NULL;
  int ii, nc=0, nn, nt;
  char *aa, *lines[512], str[256], *sptrs[32];

  slm->num_entries = 0;
 
  nn = strlen (txt);
  buf = (char *)malloc (nn+1);
  strcpy (buf, txt);
  nn = dd_tokenz (buf, lines, "\n");
  
  for (ii=0; ii < nn; ii++) {
    if (aa = strchr (lines[ii], '#')) /* comment */
      { *aa = '\0'; }
    
    strcpy (str, lines[ii]);
    nt = dd_tokens (str, sptrs);
    if (nt < 1)		/* no tokens in line */
      { continue; }
    
    solo_add_list_entry(slm, lines[ii]);
    nc++;
  }
  free (buf);
  return nc;
}

/* c------------------------------------------------------------------------ */

int dd_strings_to_slm (const char **lines, struct solo_list_mgmt *slm, int nn)
{
  int ii, nc=0, nt;
  char *aa, str[256], *sptrs[32], str2[256];

  slm->num_entries = 0;
 
  for (ii=0; ii < nn; ii++) {
    strcpy (str, lines[ii]);
    if (aa = strchr (str, '#')) /* comment */
      { *aa = '\0'; }
    strcpy (str2, str);
    nt = dd_tokens (str2, sptrs);
    if (nt < 1)		/* no tokens in line */
      { continue; }
    
    solo_add_list_entry(slm, str);
    nc++;
  }
  return nc;
}

/* c------------------------------------------------------------------------ */

int dd_absorb_strings (char *path_name, struct solo_list_mgmt *slm)
{
    int ii, nn, nt, mark;
    char *aa, *bb, str[256], str2[256], *sptrs[64];
    FILE *stream;


    if(!(stream = fopen(path_name, "r"))) {
	printf("Unable to open %s", path_name);
	return(-1);
    }
    slm->num_entries = 0;

    /* read in the new strings
     */
    for(nn=0;; nn++) {
	if(!(aa = fgets(str, (int)128, stream))) {
	    break;
	}
	if (bb = strchr (str, '#')) /* comment */
	  { *bb = '\0'; }

	strcpy (str2, str);
	if ((nt = dd_tokens (str2, sptrs)) < 1)
	  { continue; }
	solo_add_list_entry(slm, str);
    }
    fclose(stream);
    return(nn);
}
/* c------------------------------------------------------------------------ */

void 
solo_sort_strings (char **sptr, int ns)
{
    int ii, jj;
    char *keep;

    for(ii=0; ii < ns-1; ii++) {
	for(jj=ii+1; jj < ns; jj++) {
	    if(strcmp(*(sptr+jj), *(sptr+ii)) < 0) {
		keep = *(sptr+ii);
		*(sptr+ii) = *(sptr+jj);
		*(sptr+jj) = keep;
	    }
	}
    }
}
/* c------------------------------------------------------------------------ */

int 
generic_sweepfiles (char *dir, struct solo_list_mgmt *lm, char *prefix, char *suffix, int not_this_suffix)
{
    /* tries to create a list of files in a directory
     */
    DIR *dir_ptr;
    struct dirent *dp;
    char mess[256];
    char *aa, *bb, *cc, *pfx=prefix, *sfx=(char *)"";
    int pfx_len = strlen( prefix );
    int sfx_len = strlen( suffix );
    int mark;

    if( sfx_len )
      { sfx = suffix; }

    lm->num_entries = 0;

    if(!(dir_ptr = opendir(dir))) {
	sprintf(mess, "Cannot open directory %s\n", dir);
	printf( "%s", mess );
	return(-1);
    }

    for(;;) {
	dp=readdir(dir_ptr);
	if(dp == NULL ) {
	    break;
	}
	aa = dp->d_name;
	if( *aa == '.' )
	   { continue; }

	if( pfx_len && strncmp(aa, pfx, pfx_len))
	   { continue; }	/* does not compare! */

	if( sfx_len ) {
	  bb = aa + strlen(aa) - sfx_len;
	  mark = strncmp( bb, sfx, sfx_len );

	  if( not_this_suffix && mark == 0 )
	    { continue; }	/* don't want files with this suffix */
	  else if( mark != 0 )
	    { continue; }	/* wrong suffix */
	}
	/* passes all tests */
	solo_add_list_entry(lm, dp->d_name);
    }
    closedir(dir_ptr);
    if(lm->num_entries > 1)
	  solo_sort_strings(lm->list, lm->num_entries);

    return(lm->num_entries);
}
/* c------------------------------------------------------------------------ */

int 
dd_return_interpulse_periods (struct dd_general_info *dgi, int field_num, double *ipps)
{
  int ii, nn = 0, mask = dgi->dds->parm[field_num]->interpulse_time;
  int probe = 1;
  float * fptr = &dgi->dds->radd->interpulse_per1;

  if( field_num < 0 || field_num >= dgi->dds->radd->num_parameter_des )
    { return 0; }
				/*
  // interpulse periods returned in milliseconds
				 */
# ifdef notyet
  for( ii = 0; ii < 5; ii++, probe <<= 1 ) {
    if( probe & mask ) {
      /*
      // in the header as milliseconds
       */
      *( ipps + nn ) = *( fptr + ii ) * .001; 
      nn++;
    }
  }
# endif

  nn = dgi->dds->radd->num_ipps_trans;
  for (ii=0; ii < nn; ii++) {
     ipps[ii] = fptr[ii];
  }
  return nn;
}

/* c------------------------------------------------------------------------ */

int 
dd_return_frequencies (struct dd_general_info *dgi, int field_num, double *freqs)
{
  int ii, nn = 0, mask = dgi->dds->parm[field_num]->xmitted_freq;
  int probe = 1;
  float * fptr = &dgi->dds->radd->freq1;

  if( field_num < 0 || field_num >= dgi->dds->radd->num_parameter_des )
    { return 0; }
				/*
  // frequencies returned in GHz
				 */

  for( ii = 0; ii < 5; ii++, probe <<= 1 ) {
    if( probe & mask )
      { *( freqs + nn++ ) = *( fptr + ii ); }
  }
  return nn;
}
/* c------------------------------------------------------------------------ */

int 
dd_alias_index_num (struct dd_general_info *dgi, char *name)
{
  static char * known_aliases [] =
  {
      (char *)"DZ DB DBZ",
      (char *)"VE VR",
      (char *)"RH RX RHO RHOHV",
      (char *)"PH DP PHI PHIDP",
      (char *)"ZD DR ZDR",
      (char *)"LD LC LDR",
      (char *)"NC NCP",
      (char *)"LV LVDR LDRV"
  };
  char str[256], * sptrs[64], *aa, *bb, *cc;
  int ii, jj, kk, mm, nn, nt, ndx;
  int num_alias_sets = sizeof( known_aliases )/sizeof( char *);

  if(( mm = strlen( name )) < 1 )
    { return -1; }

  for( ii = 0; ii < num_alias_sets; ii++ ) { /* for each set of aliases */
    if( !strstr( known_aliases[ii], name ))
      { continue; }		/* not even close */

    strcpy( str, known_aliases[ii] );
    nt = dd_tokens( str, sptrs );

    for( jj = 0; jj < nt; jj++ ) {
      nn = strlen( sptrs[jj] );
      if( mm != nn )
	{ continue; }		/* lengths of possible match not the same */
      if( !strcmp( name, sptrs[jj] )) {
	/* 
	 * we have a match; now see if this mapper responds to
	 * one of these aliases
	 */
	for( kk = 0; kk < nt; kk++ ) {
	  if(( ndx = dd_find_field( dgi, sptrs[kk] )) >= 0 )
	    { return ndx; }
	}
	return -1;
      }
    }
  }
  return -1;
}
/* c------------------------------------------------------------------------ */

double 
angdiff (double a1, double a2)
{
    double d=a2-a1;

    if( d < -270. )
	  return(d+360.);
    if( d > 270. )
	  return(d-360.);
    return(d);
}
/* c------------------------------------------------------------------------ */

double 
d_angdiff (double a1, double a2)
{
    double d=a2-a1;

    if( d < -180. )
	  return(d+360.);
    if( d > 180. )
	  return(d-360.);
    return(d);
}
/*c----------------------------------------------------------------------*/

void 
cascdmp (		/* ascii dump of n bytes */
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
	    s = 7;
	}
	if( *c > 31 && *c < 127 )
	     putchar(*c);	/* print actual char */
	else
	     putchar('.');
	s++;
	
	if( s > 76 ) {
	    putchar('\n');	/* start a new line */
	    s = 0;
	}
    }
    if( s > 0 )
	 printf( "\n" );
}
/*c----------------------------------------------------------------------*/

void 
chexdmp (		/* hexdump of n bytes */
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
	    printf("%5d) ", m/4);	/* new line label */
	    s = 7;
	}
	/* get upper and lower nibble */
	c = ((int32_t) *(b+m) >>4) & 0xf;
	putchar(*(h+c));	/* print actual hex char */
	c = (int32_t) *(b+m) & 0xf; 
	putchar(*(h+c));
	s += 2;
	
	if( s > 76 ) {
	    putchar('\n');	/* start a new line */
	    s = 0;
	}
	else if( i%4 == 0 ) {
	    putchar(' ');	/* space every 8 char */
	    s++;
	}
    }
    if( s > 0 ) printf( "\n" );
}
/* c------------------------------------------------------------------------ */

char *
cpy_bfill_str (CHAR_PTR dst, CHAR_PTR srs, int n)
{
    int i, j;

    if((j=strlen(srs)) >= n) {
	strncpy(dst,srs,n);
	return(dst);
    }
    strcpy(dst,srs);

    for(; j < n; j++) {
	*(dst+j) = ' ';		/* blank fill */
    }
    return(dst);
}
/*c----------------------------------------------------------------------*/

int 
dcdatime (const char *str, int n, short *yy, short *mon, short *dd, short *hh, short *mm, short *ss, short *ms)
{
    /*
     * this routine decodes a time string hh:mm:ss.ms
     */
    char ls[88], *s, *t;
    
    *yy = *mon = *dd = *hh = *mm = *ss = *ms = 0;
    strncpy( ls, str, n );
    ls[n] = '\0';
    s = ls;
    
    if( t = strchr( s, '/' )) {
	/*
	 * assume this string contains date and time
	 * of the form mm/dd/yy:hh:mm:ss.ms
	 */
	*t = '\0';
	*mon = atoi(s);
	s = ++t;
	if( !( t = strchr( s, '/' )))
	      return(NO);
	*t = '\0';
	*dd = atoi(s);
	s = ++t;
	if(( t = strchr( s, ':' ))) {
	    *t = '\0';
	    *yy = atoi(s);
	    s = ++t;
	}
	else if( strlen(s)) {
	    *yy = atoi(s);
	    return(YES);
	}
	else
	      return(NO);
    }
    /* now get hh, mm, ss, ms */
    if(( t = strchr( s, ':' ))) {
	*t = '\0';
	*hh = atoi(s);
    }
    else {
	if(strlen(s))
	      *hh = atoi(s);
	return(YES);
    }    
    s = ++t;
    if(( t = strchr( s, ':' ))) {
	*t = '\0';
	*mm = atoi(s);
    }
    else {
	if(strlen(s))
	      *mm = atoi(s);
	return(YES);
    }    
    s = ++t;
    if(( t = strchr( s, '.' ))) {
	*t = '\0';
	*ss = atoi(s);
    }
    else {
	if(strlen(s))
	      *ss = atoi(s);
	return(YES);
    }
    s = ++t;
    if(strlen(s))
	  *ms = atoi(s);
    return(YES);
}
/* c------------------------------------------------------------------------ */

char *
dd_blank_fill (CHAR_PTR srs, int n, CHAR_PTR dst)
{
    int i;

    for(i=0; i < n && *srs; i++)
	  *(dst+i) = *srs++;

    for(; i < n; i++ )
	  *(dst+i) = ' ';

    return(dst);
}
/* c------------------------------------------------------------------------ */

char *
dd_chop (char *aa)
{
    /* remove the line feed from the end of the string
     * if there is one
     */
    int nn;

    if(!aa || !(nn=strlen(aa)))
	  return(NULL);

    if(*(aa + (--nn)) == '\n')
	  *(aa +nn) = '\0';
    return(aa);
}
/* c------------------------------------------------------------------------ */

int 
dd_crack_file_name (char *type, int32_t *time_stamp, char *radar, int *version, const char *fn)
{
    char *delim=(char *)DD_NAME_DELIMITER;
    double d;
    DD_TIME dts;
    char string_space[256], *str_ptrs[32];
    int nt, mon, day, hrs, min, secs, ival;
    char *aa, *bb, *cc;

    *version = *time_stamp = 0;
    *type = *radar = '\0';

    if(!fn || !strlen(fn))
	  return(NO);

    strcpy( string_space, fn );
    nt = dd_tokenz( string_space, str_ptrs, DD_NAME_DELIMITER );

    strcpy( type, str_ptrs[0] );
    strcpy( radar, str_ptrs[2] );

    /* time */
    dd_clear_dts( &dts );
    aa = str_ptrs[1];
    bb = aa + strlen( aa ) - 10; /*  ready to suck off all but year  */
    sscanf( bb, "%2d%2d%2d%2d%2d", &mon, &day, &hrs, &min, &secs );
    *bb = '\0';

    ival = atoi( aa );
    dts.year = ival > 1900 ? ival : ival +1900;
    dts.month = mon;
    dts.day = day;
    dts.day_seconds = D_SECS(hrs, min, secs, 0);
    *time_stamp = (int32_t)d_time_stamp(&dts);

    *version = atoi( str_ptrs[3] ); /* milliseconds and version num */

    return(YES);
}
/* c------------------------------------------------------------------------ */

int 
dd_crack_file_name_ms (char *type, double *time_stamp, char *radar, int *version, char *fn, int *ms)
{
    char *delim=(char *)DD_NAME_DELIMITER;
    double d;
    DD_TIME dts;
    char string_space[256], *str_ptrs[32];
    int nt, mon, day, hrs, min, secs, ival;
    char *aa, *bb, *cc;

//    *version = *time_stamp = 0;  //Jul 26, 2011
    *version = 0;
    *time_stamp = 0;
    *type = *radar = '\0';

    if(!fn || !strlen(fn))
	  return(NO);

    strcpy( string_space, fn );
    nt = dd_tokenz( string_space, str_ptrs, DD_NAME_DELIMITER );

    strcpy( type, str_ptrs[0] );
    strcpy( radar, str_ptrs[2] );

    /* time */
    dd_clear_dts( &dts );
    aa = str_ptrs[1];
    bb = aa + strlen( aa ) - 10; /*  ready to suck off all but year  */
    sscanf( bb, "%2d%2d%2d%2d%2d", &mon, &day, &hrs, &min, &secs );
    *bb = '\0';

    ival = atoi( aa );
    dts.year = ival > 1900 ? ival : ival + 1900;
    dts.month = mon;
    dts.day = day;

    *version = atoi( str_ptrs[3] ); /* milliseconds and version num */

    *ms = *version % 1000;
    *version /= 1000;
    dts.day_seconds = D_SECS(hrs, min, secs, *ms);

    *time_stamp = d_time_stamp(&dts);

    return(YES);
}
/* c------------------------------------------------------------------------ */

void 
dd_file_name (const char *type, const int32_t time_stamp,
	      const char *radar, const int version, char *fn)
{
    DD_TIME dts;

    dts.time_stamp = time_stamp;
    d_unstamp_time(&dts);

    sprintf( fn
	    , "%s%s%02d%02d%02d%02d%02d%02d%s%s%s%d"
	    , type
	    , DD_NAME_DELIMITER
	    , dts.year -1900
	    , dts.month
	    , dts.day
	    , dts.hour
	    , dts.minute
	    , dts.second
	    , DD_NAME_DELIMITER, radar
	    , DD_NAME_DELIMITER, version );
}
/* c------------------------------------------------------------------------ */

void dd_file_name_ms(const char *type, const int32_t time_stamp,
		     const char *radar, const int version, char *fn,
		     const int ms)
{
  int vzn = ms + (version * 1000);
  DD_TIME dts;

  dts.time_stamp = time_stamp;
  d_unstamp_time(&dts);

  sprintf(fn,
	  "%s%s%02d%02d%02d%02d%02d%02d%s%s%s%d",
	  type,
	  DD_NAME_DELIMITER,
	  dts.year -1900,
	  dts.month,
	    dts.day,
	  dts.hour,
	  dts.minute,
	  dts.second,
	  DD_NAME_DELIMITER, radar,
	  DD_NAME_DELIMITER, vzn);

}

void dd_file_name_ms(const char *type, const int32_t time_stamp,
		     const char *radar, const int version, std::string &fn,
		     const int ms)
{
  int vzn = ms + (version * 1000);
  DD_TIME dts;

  dts.time_stamp = time_stamp;
  d_unstamp_time(&dts);

  char buffer[1024];
  
  sprintf(buffer,
	  "%s%s%02d%02d%02d%02d%02d%02d%s%s%s%d",
	  type,
	  DD_NAME_DELIMITER,
	  dts.year -1900,
	  dts.month,
	    dts.day,
	  dts.hour,
	  dts.minute,
	  dts.second,
	  DD_NAME_DELIMITER, radar,
	  DD_NAME_DELIMITER, vzn);

  fn = buffer;
}

/* c------------------------------------------------------------------------ */

int 
dd_hrd16 (char *buf, short *dd, int flag, int *empty_run)
{
    /*
     * routine to unpacks actual data assuming MIT/HRD compression
     */
    short *ss=(short *)buf;
    int n, mark, wcount=0;

    while(*ss != 1) {
	n = *ss & MASK15;
	wcount += n;
	if( *ss & SIGN16 ) {	/* data! */
	    *empty_run = 0;
	    ss++;
	    for(; n--;) {
		*dd++ = *ss++;
	    }
	}	
	else {			/* fill with flags */
	    *empty_run = n;
	    ss++;
	    for(; n--;) {
		*dd++ = flag;
	    }
	}
    }
    return(wcount);
}
/* c------------------------------------------------------------------------ */

int 
dd_hrd16_uncompressx (short *ss, short *dd, int flag, int *empty_run, int wmax)
{
    /*
     * routine to unpacks actual data assuming MIT/HRD compression where:
     * ss points to the first 16-bit run-length code for the compressed data
     * dd points to the destination for the unpacked data
     * flag is the missing data flag for this dataset that is inserted
     *     to fill runs of missing data.
     * empty_run pointer into which the number of missing 16-bit words
     *    can be stored. This will be 0 if the last run contained data.
     # wmax indicate the maximum number of 16-bit words the routine can
     *    unpack. This should stop runaways.
     */
    int i, j, k, n, mark, wcount=0;

    while(*ss != 1) {		/* 1 is an end of compression flag */
	n = *ss & 0x7fff;	/* nab the 16-bit word count */
	if(wcount+n > wmax) {
	    printf("Uncompress failure %d %d %d\n"
		   , wcount, n, wmax);
	    mark = 0;
	    break;
	}
	else {
	    wcount += n;		/* keep a running tally */
	}
	if( *ss & 0x8000 ) {	/* high order bit set implies data! */
	    *empty_run = 0;
	    ss++;
	    for(; n--;) {
		*dd++ = *ss++;
	    }
	}	
	else {			/* otherwise fill with flags */
	    *empty_run = n;
	    ss++;
	    for(; n--;) {
		*dd++ = flag;
	    }
	}
    }
    return(wcount);
}
/* c------------------------------------------------------------------------ */

int 
dd_hrd16LE_uncompressx (unsigned short *ss, unsigned short *dd, int flag, int *empty_run, int wmax)
{
    /*
     * routine to unpacks actual data assuming MIT/HRD compression where:
     * ss points to the first 16-bit run-length code for the compressed data
     * dd points to the destination for the unpacked data
     * flag is the missing data flag for this dataset that is inserted
     *     to fill runs of missing data.
     * empty_run pointer into which the number of missing 16-bit words
     *    can be stored. This will be 0 if the last run contained data.
     # wmax indicate the maximum number of 16-bit words the routine can
     *    unpack. This should stop runaways.
     */
    int i, j, k, n, mark, wcount=0;
    unsigned short rlcw;
    unsigned char *aa, *bb;
    
    aa = (unsigned char *)&rlcw;

    for(;;) {	
       bb = (unsigned char *)ss;
       *aa = *(bb+1);
       *(aa+1) = *bb;		/* set run length code word "rlcw" */
       if(rlcw == 1) { break; }	/* 1 is the end of compression flag */

       n = rlcw & 0x7fff;	/* nab the 16-bit word count */
       if(wcount+n > wmax) {
	  printf("Uncompress failure %d %d %d\n"
		 , wcount, n, wmax);
	  mark = 0;
	  break;
       }
       else {
	  wcount += n;		/* keep a running tally */
       }
       if( rlcw & 0x8000 ) {	/* high order bit set implies data! */
	  *empty_run = 0;
	  ss++;
	  swack_short((char *)ss, (char *)dd, n);
	  ss += n;
	  dd += n;
	  
       }	
       else {			/* otherwise fill with flags */
	  *empty_run = n;
	  ss++;
	  for(; n--;) {
	     *dd++ = flag;
	  }
       }
    }
    return(wcount);
}
/* c------------------------------------------------------------------------ */

int 
dd_nab_floats (char *str, float *vals)
{
    int ii, jj, nt;
    char string_space[256], *str_ptrs[32];

    strcpy(string_space, str);
    nt = dd_tokens(string_space, str_ptrs);

    for(ii=0; ii < nt; ii++) {
	jj = sscanf(str_ptrs[ii], "%f", vals+ii);
    }
    return(0);
}
/* c------------------------------------------------------------------------ */

int 
dd_nab_ints (char *str, int *vals)
{
    int ii, jj, nt;
    char string_space[256], *str_ptrs[32];

    strcpy(string_space, str);
    nt = dd_tokens(string_space, str_ptrs);

    for(ii=0; ii < nt; ii++) {
	jj = sscanf(str_ptrs[ii], "%d", vals+ii);
    }
    return(0);
}
/* c------------------------------------------------------------------------ */

int 
dd_rdp_uncompress8 (unsigned char *buf, unsigned char *dd, int bad_val)
{
    /*
     * routine to unpacks actual data assuming MIT/HRD compression
     */
    unsigned char *uc=buf;
    int count=0, n;

    while (*uc != 0 || *uc != 1 ) {
	n = *uc & MASK7;
	if( *uc & SIGN8 ) {	/* data! */
	    for(; n--;) {
		count++;
		*dd++ = *uc++;
	    }
	}	
	else {			/* fill with nulls */
	    uc++;
	    for(; n--;) {
		count++;
		*dd++ = bad_val;
	    }
	}
    }
    return(count);
}
/* c------------------------------------------------------------------------ */

int 
dd_scan_mode (char *str)
{
    char uc_scan_mode[16];
    int max_scan_mode = 10, ii, nn;
    char *aa;

    if( !str )
      { return(-1); }
    if( !(nn = strlen(str)))
      { return(-1); }
    
    aa = uc_scan_mode;
    strcpy( aa, str );
    for(; !*aa ; aa++ )
      {	*aa = (char)toupper((int)(*aa)); }

    for( ii = 0; ii < max_scan_mode; ii++ )
      {
	if( !strcmp( uc_scan_mode, Stypes[ii] ))
	  { return( ii ); }
      }
    return( -1 );
}
/* c------------------------------------------------------------------------ */

char *
dd_scan_mode_mne (int scan_mode, char *str)
{
    int max_scan_mode = 10;

    scan_mode = scan_mode < 0 || scan_mode > max_scan_mode
	  ? max_scan_mode : scan_mode;

    strcpy(str, Stypes[scan_mode]);
    return(str);
}
/* c------------------------------------------------------------------------ */

char *
dd_radar_type_ascii (int radar_type, char *str)
{
    int max_radar_types = 10;
    static char *Rtypes[] = { (char *)"GROUND", (char *)"AIR_FORE",
			      (char *)"AIR_AFT", (char *)"AIR_TAIL",
			      (char *)"AIR_LF", (char *)"SHIP",
			      (char *)"AIR_NOSE", (char *)"SATELLITE",
			      (char *)"LIDAR_MOVING", (char *)"LIDAR_FIXED",
			      (char *)"UNKNOWN"
			      };

    radar_type = radar_type < 0 || radar_type > max_radar_types
	  ? max_radar_types : radar_type;

    strcpy(str, Rtypes[radar_type]);
    return(str);
}
/* c------------------------------------------------------------------------ */

char *
dd_unquote_string (char *uqs, const char *qs)
{
    char *a=uqs;
    /*
     * remove the double quotes from either end of the string
     */
    for(; *qs; qs++)
	  if(*qs != '"') *a++ = *qs;
    *a = '\0';
    return(uqs);
}
/* c------------------------------------------------------------------------ */

double 
dorade_time_stamp (struct volume_d *vold, struct ray_i *ryib, DD_TIME *dts)
{
    double d;

    dts->year = vold->year > 1900 ? vold->year : vold->year +1900;
    dts->month = dts->day = 0;
    dts->day_seconds = (ryib->julian_day-1)*SECONDS_IN_A_DAY
	  +D_SECS(ryib->hour, ryib->minute, ryib->second
		  , ryib->millisecond);
    d = d_time_stamp(dts);
    
    return(d);
 }
/* c----------------------------------------------------------------------- */

char *
dts_print (DD_TIME *dts)
{
    static char str[32];

    sprintf(str, "%02d/%02d/%02d %02d:%02d:%02d.%03d"
	   , dts->month
	   , dts->day
	   , dts->year
	   , dts->hour
	   , dts->minute
	   , dts->second
	   , dts->millisecond
	   );
    return(str);
}
/* c------------------------------------------------------------------------ */

double 
eldora_time_stamp (struct volume_d *vold, struct ray_i *ryib)
{
    double d;
    DD_TIME dts;

    d = dorade_time_stamp(vold, ryib, &dts);
	  
    return(d);
}
/* c------------------------------------------------------------------------ */

int 
fb_read (int fin, char *buf, int count)
{
    /* fortran-binary read routine
     */
    int32_t size_rec=0, rlen1, rlen2=0, nab;

    /* read the record size */
    rlen1 = read (fin, &nab, sizeof(nab));

    if( rlen1 < sizeof(nab))
	  return(rlen1);

    if(hostIsLittleEndian()) {
       swack4((char *)&nab, (char *)&size_rec);  //Jul 26, 2011 need to confirm
    }
    else {
       size_rec = nab;
    }

    if( size_rec > 0 ) {
	/* read the record
	 * (the read may be less than the size of the record)
	 */
	rlen2 = size_rec <= count ? size_rec : count;

	rlen1 = read (fin, buf, rlen2);	/* read it! */
	if( rlen1 < 1 )
	      return(rlen1);
	rlen2 = rlen1 < size_rec ?
	      size_rec-rlen1 : 0; /* set up skip to end of record */
    }
    else
	  rlen1 = 0;
    
    rlen2 += sizeof(size_rec);

    /* skip thru the end of record */
    rlen2 = lseek( fin, rlen2, SEEK_CUR );
    return(rlen1);
}
/* c------------------------------------------------------------------------ */

int 
fb_write (int fout, char *buf, int count)
{
    /* fortran-binary write routine
     */
    int32_t size_rec=count, rlen1, rlen2=0, blip;

    if(hostIsLittleEndian()) {
//    swack4(&size_rec, &blip);  //Jul 26, 2011
      swack4((char *)&size_rec, (char *)&blip);
    }
    else {
       blip = size_rec;
    }
    /* write the record length */
    rlen1 = write(fout, &blip, sizeof(blip));
    if( rlen1 < sizeof(size_rec))
	  return(rlen1);

    if( size_rec > 0 ) {
	/* write the record */
	rlen1 = write (fout, buf, size_rec);
	if( rlen1 < 1 )
	      return(rlen1);
    }
    /* write the record length */
    rlen2 = write (fout, &blip, sizeof(blip));
    if( rlen2 < sizeof(blip))
	  return(rlen2);

    else if(size_rec == 0)
	  return(0);
    else
	  return(rlen1);
}
/* c------------------------------------------------------------------------ */

char *fgetsx(unsigned char *aa, int nn, FILE *stream)
{
    /* similar to fgets()
     */
    int ii, jj=0;
    unsigned char *c=aa;

    for(; nn--; c++,jj++) {
	ii = fgetc(stream);
	if(ii == EOF) {
	    if(jj) {
		*c = '\0';
		return( (char *) aa);
	    }
	    return(NULL);
	}
	*c = ii;
	if(ii == '\n') {
	    *(++c) = '\0';
	    return( (char *) aa);
	}
    }
    return(NULL);
}
/* c------------------------------------------------------------------------ */

int32_t file_time_stamp (const char *fn)
{
    int32_t time_stamp;
    int version;
    char type[8], radar[12];

    dd_crack_file_name( type, &time_stamp, radar, &version, fn );

    return(time_stamp);
}
/* c------------------------------------------------------------------------ */

char *fstring(char *line, int nn, FILE *stream)
{
    /* read in lines from a file and remove leading blanks
     * trailing blanks and line feeds
     */
    int ii, jj;
    unsigned char *aa, *cc;

    for(aa=cc=(unsigned char*)line ;; aa=cc=(unsigned char*)line) {
	for(jj=1 ;;) {
	    ii = fgetc(stream);
	    if(ii == EOF || ii == '\n') {
		*cc = '\0';
		break;
	    }
	    if(jj < nn) {
		*cc++ = ii;
		jj++;
	    }
	}
	for(; aa < cc && *aa == ' '; aa++); /* leading blanks */
	for(; aa < cc && *(cc-1) == ' '; *(--cc) = '\0'); /* trailing blanks */
	if(aa != cc)
	      return( (char *) aa);
	if(ii == EOF)
	      return(NULL);
    }
}
/* c------------------------------------------------------------------------ */

char *
get_tagged_string (const char *tag)
{
    int i;

    for(i=0; i < num_tags; i++ ) {
	if(!strcmp(tag,tags[i])) {
	    return(tagged_strings[i]);
	}
    }
    return(0);
}
/* c------------------------------------------------------------------------ */

int 
gp_read (int fid, char *buf, int size, int io_type)
{
    /* general purpose write
     */
    int n=size;

    if(io_type == FB_IO ) { /* fortran binary */
	n = fb_read( fid, buf, size );
    }
    else if(size > 0 ) {
	n = read(fid, buf, size);
    }
    return(n);
}
/* c------------------------------------------------------------------------ */

int 
gp_write (int fid, char *buf, int size, int io_type)
{
    /* general purpose write
     */
    int n;
#ifdef HAVE_SYS_MTIO_H
    struct mtop op;
#endif /* HAVE_SYS_MTIO_H */

    if(io_type == FB_IO ) { /* fortran binary */
	n = fb_write( fid, buf, size );
    }
    else if(size > 0 ) {
	n = write(fid, buf, size);
    }
    else if(io_type == BINARY_IO) { /* pure binary output */
	/* since size == 0 this is a noop */
	return(size);
    }
#ifdef HAVE_SYS_MTIO_H
    else {			/* assume tape io */
	if(size != 0)
	      return(size);
	/* need to write a physical eof */
	op.mt_op = MTWEOF;
	op.mt_count = 1;
	if (ioctl(fid, MTIOCTOP, &op) < 0) {
	    perror ("Mag tape WEOF");
	}
	n = 0;
    }
#endif /* ifdef HAVE_SYS_MTIO_H */
    return(n);
}
/* c------------------------------------------------------------------------ */

int 
in_sector (double ang, double ang1, double ang2)
{
    if(ang1 > ang2) return(ang >= ang1 || ang < ang2);

    return(ang >= ang1 && ang < ang2);
}
/* c------------------------------------------------------------------------ */

char *
put_tagged_string (char *tag, char *string)
{
    int i, j, m=strlen(tag), n=strlen(string);

    if(!m)
	  return(0);
    
    /* see if this tag is there already */
    for(i=0,j= -1; i < num_tags; i++ ) {
	if(!strcmp(tag,tags[i])) {
	    free(tagged_strings[i]);
	    break;
	}
    }    
    if(i >= num_tags) {
	num_tags++;		/* new entry */
	tags[i] = (char *)malloc(m+1);
    }
    tagged_strings[i] = (char *)malloc(n+1);
    strcpy(tags[i], tag);
    strcpy(tagged_strings[i], string);
    return(tagged_strings[i]);
}
/* c------------------------------------------------------------------------ */

int 
se_free_raqs (void)
{
    int ii;
    struct running_avg_que *raq=top_raq;

    if(!top_raq)
	  return(0);
    for(ii=0; raq; ii++,raq->in_use=NO,raq=raq->next);
    return(ii);
}
/* c------------------------------------------------------------------------ */

struct running_avg_que *
se_return_ravgs (int nvals)
{
    /* routine to return a pointer to the next free
     * running average struct/queue set up to average "nvals"
     */
    struct que_val *qv, *qv_next, *qv_last;
    struct running_avg_que *raq, *last_raq;
    int ii, jj, kk, mark;

    /* look for a free raq
     */
    for(last_raq=raq=top_raq; raq; raq=raq->next) {
	last_raq = raq;
	if(!raq->in_use)
	      break;
    }
    if(!raq) {			/* did not find a free que so
				 * make one! */
	raq = (struct running_avg_que *)
	      malloc(sizeof(struct running_avg_que));
	memset(raq, 0, sizeof(struct running_avg_que));
	if(last_raq) {
	    last_raq->next = raq;
	    raq->last = last_raq;
	}
	else {
	    top_raq = raq;
	}
	/* raq->next is deliberately left at 0 or NULL */
	top_raq->last = raq;
    }
    raq->sum = 0;
    raq->in_use = YES;

    if(raq->num_vals == nvals) {
	for(qv=raq->at,jj=0; jj < raq->num_vals; jj++,qv=qv->next) {
	    qv->d_val = qv->f_val = qv->val = 0;
	}
	return(raq);
    }
    if(raq->num_vals) {	/* free existing values */
	for(qv=raq->at,jj=0; jj < raq->num_vals; jj++) {
	    qv_next = qv->next;
	    free(qv);
	    qv = qv_next;
	}
    }
    /* now create a new que of the correct size
     */
    raq->num_vals = nvals;
    raq->rcp_num_vals = 1./nvals;
    for(jj=0; jj < raq->num_vals; jj++) {
	qv = (struct que_val *)
	      malloc(sizeof(struct que_val));
	memset(qv, 0, sizeof(struct que_val));
	if(!jj)
	      raq->at = qv;
	else {
	    qv->last = qv_last;
	    qv_last->next = qv;
	}
	qv->next = raq->at;
	raq->at->last = qv;
	qv_last = qv;
    }
    return(raq);
}
/* c------------------------------------------------------------------------ */
/* Add a slash to the end of the given string, which is assumed to be a path */

char *
slash_path (char *dst, const char *srs)
{
    int n;

    if(srs && (n=strlen(srs))) {
	strcpy(dst,srs);
	if(dst[n-1] != '/' ) {
	    dst[n] = '/';
	    dst[n+1] = '\0';
	}
    }
    else
	  *dst = '\0';
    return(dst);
}
/* c------------------------------------------------------------------------ */

void 
slm_dump_list (struct solo_list_mgmt *slm)
{
    char **ptr;
    int nl;

    if(!slm)
	  return;

    if(!(ptr = slm->list))
	  return;

    nl = slm->num_entries;

    for(; nl-- > 0;) {
       solo_message(*ptr++);
       solo_message("\n");
    }
}
/* c------------------------------------------------------------------------ */

void 
slm_print_list (struct solo_list_mgmt *slm)
{
    char **pt, **ptrs=slm->list;
    int ii, jj, kk, ll=0, mm, nl, last_mm;
    int nn, ival;
    float val;
    char str[128];
    int max = 256;

    if(!slm)
	  return;
    nl = slm->num_entries;

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
solo_add_list_entry (struct solo_list_mgmt *which, const char *entry)
{
    int ii;
    char *a, *c;

    if(!which)
	  return;

    int len = strlen(entry);

    if(which->num_entries >= which->max_entries) {
	which->max_entries += 30;
	if(which->list) {
	    which->list = (char **)realloc
		  (which->list, which->max_entries*sizeof(char *));
	}
	else {
	    which->list = (char **)malloc(which->max_entries*sizeof(char *));
	}
	for(ii=which->num_entries; ii < which->max_entries; ii++) {
	    *(which->list+ii) = a = (char *)malloc(which->sizeof_entries+1);
	    *a = '\0';
	}
    }
    len = len < which->sizeof_entries ? len : which->sizeof_entries;
    c = *(which->list+which->num_entries++);

    // NOTE: This should do the same thing, but allows entry to be a const
    // char*.  Also is more readable, so more maintainable.
//    if(a=entry) {
//	for(; *a && len--; *c++ = *a++);
//    }

    if (entry != 0)
    {
      for (int i = 0; i < len; ++i, ++c)
	*c = entry[i];
    }
    
    *c = '\0';
}
/* c------------------------------------------------------------------------ */

char *
solo_list_entry (struct solo_list_mgmt *which, int entry_num)
{
    char *c;

    if(!which || entry_num >= which->num_entries || entry_num < 0)
	  return(NULL);

    c = *(which->list+entry_num);
    return(c);
}
/* c------------------------------------------------------------------------ */

void 
solo_list_remove_dups (struct solo_list_mgmt *slm)
{
    /* remove duplicate strings from the list
     */
    int ii, jj, nd;

    if(!slm || slm->num_entries <= 0)
	  return;

    nd = slm->num_entries-1;

    for(ii=0; ii < slm->num_entries-1; ii++) {
	for(; ii < slm->num_entries-1 &&
	    !strcmp(*(slm->list +ii), *(slm->list +ii +1));) {
	    solo_list_remove_entry(slm, ii+1, ii+1);
	}
    }
    return;
}
/* c------------------------------------------------------------------------ */

void 
solo_list_remove_entry (struct solo_list_mgmt *slm, int ii, int jj)
{
    /* remove a span of entries where ii and jj are entry numbers
     */
    int kk, nn, nd;
    char *keep;

    if(!slm || slm->num_entries <= 0 || ii < 0 || ii >= slm->num_entries)
	  return;
    nd = slm->num_entries-1;

    if(jj >= slm->num_entries)
	  jj = nd;
    else if(jj < ii)
	  jj = ii;

    nn = jj - ii + 1;

    for(; nn--; ) {		/* keep shifting the entries up */
	keep = *(slm->list +ii);
	for(kk=ii; kk < nd; kk++) {
	    *(slm->list +kk) = *(slm->list +kk +1);
	}
	strcpy(keep, " ");
	*(slm->list +(nd--)) = keep;
    }
    slm->num_entries = nd +1;
    return;
}
/* c------------------------------------------------------------------------ */

void 
solo_list_sort_file_names (struct solo_list_mgmt *slm)
{
    int ii, jj;
    char *keep;

    if(!slm || slm->num_entries <= 0)
	  return;

    for(ii=0; ii < slm->num_entries-1; ii++) {
	for(jj=ii+1; jj < slm->num_entries; jj++) {
	    if(file_time_stamp(*(slm->list+jj))
	       < file_time_stamp(*(slm->list+ii))) {
		keep = *(slm->list+ii);
		*(slm->list+ii) = *(slm->list+jj);
		*(slm->list+jj) = keep;
	    }
	}
    }
    return;
}
/* c------------------------------------------------------------------------ */

struct solo_list_mgmt *
solo_malloc_list_mgmt (int sizeof_entries)
{
    struct solo_list_mgmt *slm;

    slm = (struct solo_list_mgmt *)malloc(sizeof(struct solo_list_mgmt));
    memset(slm, 0, sizeof(struct solo_list_mgmt));
    slm->sizeof_entries = sizeof_entries;
    return(slm);
}
/* c------------------------------------------------------------------------ */

struct point_in_space *
solo_malloc_pisp (void)
{
    struct point_in_space *next;

    next = (struct point_in_space *)
	  malloc(sizeof(struct point_in_space));
    memset(next, 0, sizeof(struct point_in_space));

    strcpy(next->name_struct, "PISP");
    next->sizeof_struct = sizeof(struct point_in_space);
    return(next);
}
/* c------------------------------------------------------------------------ */

char *
solo_modify_list_entry (struct solo_list_mgmt *which, char *entry, int len, int entry_num)
{
    int ii;
    char *a, *c;

    if(!which || entry_num > which->num_entries || entry_num < 0)
	  return(NULL);

    if(!len) len = strlen(entry);

    if(entry_num == which->num_entries) { /* this entry doesn't exist
					   * but it can be added */
	solo_add_list_entry(which, entry);
	a = *(which->list+entry_num);
	return(a);
    }
    len = len < which->sizeof_entries ? len : which->sizeof_entries;
    a = c = *(which->list+entry_num);

    for(a=entry; *a && len--; *c++ = *a++);
    *c = '\0';
    return(a);
}
/* c------------------------------------------------------------------------ */

void 
solo_reset_list_entries (struct solo_list_mgmt *which)
{
    int ii;

    if(!which || !which->num_entries)
	  return;

    for(ii=0; ii < which->num_entries; ii++) {
	strcpy(*(which->list +ii), " ");
    }
    which->num_entries = 0;
}
/* c------------------------------------------------------------------------ */

void 
solo_unmalloc_list_mgmt (struct solo_list_mgmt *slm)
{
    int ii;

    if(!slm)
	  return;

    for(ii=0; ii < slm->max_entries; ii++) {
	free(*(slm->list +ii));
    }
    free(slm);
}
/* c------------------------------------------------------------------------ */

void 
solo_sort_slm_entries (struct solo_list_mgmt *slm)
{
   char *aa, *bb;
   int ii, jj, nn = slm->num_entries;
   char str[256];
   
   for(ii=0; ii < nn-1; ii++) {
      aa = solo_list_entry(slm, ii);
      for(jj=ii+1; jj < nn; jj++) {
	 bb = solo_list_entry(slm, jj);
	 if(strcmp(bb, aa) < 0) {
	    strcpy (str, aa);
	    solo_modify_list_entry(slm, bb, strlen (bb), ii);
	    aa = solo_list_entry(slm, ii);
	    solo_modify_list_entry(slm, str, strlen (str), jj);
	 }
      }
   }
}
  
/* c------------------------------------------------------------------------ */

char *
str_terminate (char *dst, const char *srs, int n)
{
    int m=n;
    const char *a=srs;
    char *c=dst;

    *dst = '\0';

    if(n < 1)
	  return(dst);
    /*
     * remove leading blanks
     */
    for(; m && *a == ' '; m--,a++);
    /*
     * copy m char or to the first null
     */
    for(; m-- && *a; *c++ = *a++);
    /*
     * remove trailing blanks
     */
    for(*c='\0'; dst < c && *(c-1) == ' '; *(--c)='\0');

    return(dst);
}
/* c------------------------------------------------------------------------ */

int32_t 
time_now (void)
{
    int i;
    struct timeval tp;
    struct timezone tzp;

    i = gettimeofday(&tp,&tzp);
    return((int32_t)tp.tv_sec);
}
/* c------------------------------------------------------------------------ */

time_t todays_date( short date_time[])
{
    struct tm *tm;
    int i;
    struct timeval tp;
    struct timezone tzp;

    i = gettimeofday(&tp,&tzp);
    tm = localtime((time_t *) &tp.tv_sec);
    date_time[0] = tm->tm_year;
    date_time[1] = tm->tm_mon+1;;
    date_time[2] = tm->tm_mday;
    date_time[3] = tm->tm_hour;
    date_time[4] = tm->tm_min;
    date_time[5] = tm->tm_sec;

    return((int32_t)tp.tv_sec);
}
/* c------------------------------------------------------------------------ */

struct run_sum *
init_run_sum (int length, int short_len)
{
    struct run_sum * rs;
    rs = (struct run_sum *)malloc( sizeof( struct run_sum ));
    memset( rs, 0, sizeof( struct run_sum ));
    rs->vals = (double *)malloc( length * sizeof(double));
    memset( rs->vals, 0, length * sizeof(double));
    rs->run_size = length;
    rs->index_lim = length -1;
    if( short_len > 0 ) {
        rs->short_size = short_len < length ? short_len : length;
    }
    return( rs );
}
/* c------------------------------------------------------------------------ */

void 
reset_running_sum (struct run_sum *rs)
{
    rs->count = 0;
    memset( rs->vals, 0, rs->run_size * sizeof(double));
    rs->sum = 0;
    rs->short_sum = 0;
}
/* c------------------------------------------------------------------------ */

double 
short_running_sum (struct run_sum *rs)
{
    return( rs->short_sum );
}
/* c------------------------------------------------------------------------ */

double 
running_sum (struct run_sum *rs, double val)
{
    int ii, nn = ++rs->count, length = rs->run_size;
    int ndx = rs->index_next;

    rs->sum -= *( rs->vals + ndx );
    rs->sum += val;
    *(rs->vals + ndx ) = val;
    if( rs->short_size ) {
	if( nn > rs->short_size ) {
	    ii = (ndx - rs->short_size +length) % length;
	    rs->short_sum -= *( rs->vals + ii );
	}
	rs->short_sum += val;
    }
    rs->index_next = ++ndx % length;
    return( rs->sum );
}

/* c------------------------------------------------------------------------ */

double 
avg_running_sum (struct run_sum *rs)
{
    double d;

    if( rs->count > 0 ) {
	d = rs->count > rs->run_size ? rs->run_size : rs->count;
	return( rs->sum/d );
    }
    else {
	return(0);
    }
}

/* c------------------------------------------------------------------------ */

double 
short_avg_running_sum (struct run_sum *rs)
{
    double d;
    int nn = rs->count;

    if( nn > 0 ) {
	d = nn > rs->short_size ? rs->short_size : nn;
	return( rs->short_sum/d );
    }
    else {
	return(0);
    }
}

/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

