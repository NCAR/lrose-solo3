/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

#include "dd_time.hh"
#include <string.h>
#include <sys/types.h>

# ifndef SECONDS_PER_DAY
# define SECONDS_PER_DAY 86400
# define MAX_FLOAT 1.e22
# define BAD_TIME -MAX_FLOAT
# define SECONDS_TO_DAYS 1.157407407e-5
# define SECONDS_TO_HOURS 2.777777778e-4
# define SECONDS_TO_MINUTES .01666667
# endif

# define MAX_YEARS 100
# define WILD_GUESS 1992
# define REFERENCE_YEAR 1970

static double reg_year[13], leap_year[13]; /* num secs in each month */
static double *accumulated_months[MAX_YEARS+1];
static double accumulated_years[MAX_YEARS+1];
static int first_d_time=1;
static int iguess=WILD_GUESS -REFERENCE_YEAR;


/* c------------------------------------------------------------------------ */

void dd_clear_dts (struct d_time_struct *dts)
{
    memset(dts, 0, sizeof(struct d_time_struct));
}
/* c----------------------------------------------------------------------- */

double d_time_stamp (DD_TIME *dts)
{
    int i, mm, dd;
    /*
     * This routine expects year, month, day, and total seconds
     * into the day
     * it will NOT use hour, min, sec, ms info in the struct
     *
     * This routine gives a time similar if not the same as unix time
     * but does not worry about time zones and daylight saving time.
     */
    if(first_d_time) {
	first_d_time = 0;
	init_d_time();
    }

    if(dts->year < REFERENCE_YEAR || dts->year >= REFERENCE_YEAR + MAX_YEARS)
	  return((double)BAD_TIME);

    i = dts->year-REFERENCE_YEAR;
    dts->sub_second = dts->day_seconds-(int)dts->day_seconds;

    mm = dts->month > 0 ? dts->month-1 : 0;
    dd = dts->day > 0 ? dts->day-1 : 0;

    dts->time_stamp = accumulated_years[i]
	  +(*(accumulated_months[i]+mm)) +dd*SECONDS_PER_DAY
		+dts->day_seconds;

    return(dts->time_stamp);
}
/* c----------------------------------------------------------------------- */

DD_TIME *d_unstamp_time (DD_TIME *dts)
{
    int i=iguess;
    int32_t j=(int32_t) dts->time_stamp;
    double *mp, fudge=0;
    double d=dts->time_stamp;

    if(first_d_time) {
	first_d_time = 0;
	init_d_time();
    }

    if(d < 0 || d > accumulated_years[MAX_YEARS]) {
	dts->year= -1;       
	dts->month= -1;      
	dts->day= -1;        
	dts->hour= -1;       
	dts->minute= -1;     
	dts->second= -1;     
	dts->millisecond= -1;
	return(dts);
    }
    /* if you're at exactly midnight
     * the hours will come out as 24...sigh
     */
    if(!(d-j))
	  if(!(j % SECONDS_PER_DAY))
		fudge = 1.;
    d += fudge;
    if(d <= (double)accumulated_years[i])
	  i = 0;

    for(; i <= MAX_YEARS; i++ ) {
	if((double)accumulated_years[i] > d)
	      break;
    }
    mp = accumulated_months[--i];
    dts->year = REFERENCE_YEAR +i;
    d -= accumulated_years[i];

    dts->julian_day = (int)(d*SECONDS_TO_DAYS +1.);

    i = *(mp+5) < d ? 5 : 0;
    for(; i < 13; i++) {
	if(*(mp+i) > d)
	      break;
    }
    dts->month = i;
    d -= *(mp+i-1);
    dts->day = (int)(d*SECONDS_TO_DAYS);
    d -= (double)((dts->day++)*SECONDS_PER_DAY);
    d -= fudge;
    dts->hour = (int)(d*SECONDS_TO_HOURS);
    d -= 3600.*dts->hour;
    dts->minute = (int)(d*SECONDS_TO_MINUTES);
    d -= 60.*dts->minute;
    dts->second = (int)d;
    d -= (double)dts->second;
    dts->millisecond = (int)(1000.*d +.5);
    return(dts);
}
/* c----------------------------------------------------------------------- */

void init_d_time (void)
{
    int i, yy, mark;
    int t28=2419200, t29=2505600, t30=2592000, t31=2678400;
    
    /*
     * Brute force seconds per month initialization
     */
    i = 0;
    reg_year[i] = leap_year[i] = 0;
    i++;			/* jan */
    reg_year[i] = reg_year[i-1] + t31;
    leap_year[i] = leap_year[i-1] + t31;
    i++;			/* feb */
    reg_year[i] = reg_year[i-1] + t28;
    leap_year[i] = leap_year[i-1] + t29;
    i++;			/* mar */
    reg_year[i] = reg_year[i-1] + t31;
    leap_year[i] = leap_year[i-1] + t31;
    i++;			/* apr */
    reg_year[i] = reg_year[i-1] + t30;
    leap_year[i] = leap_year[i-1] + t30;
    i++;			/* may */
    reg_year[i] = reg_year[i-1] + t31;
    leap_year[i] = leap_year[i-1] + t31;
    i++;			/* jun */
    reg_year[i] = reg_year[i-1] + t30;
    leap_year[i] = leap_year[i-1] + t30;
    i++;			/* jul */
    reg_year[i] = reg_year[i-1] + t31;
    leap_year[i] = leap_year[i-1] + t31;
    i++;			/* aug */
    reg_year[i] = reg_year[i-1] + t31;
    leap_year[i] = leap_year[i-1] + t31;
    i++;			/* sep */
    reg_year[i] = reg_year[i-1] + t30;
    leap_year[i] = leap_year[i-1] + t30;
    i++;			/* oct */
    reg_year[i] = reg_year[i-1] + t31;
    leap_year[i] = leap_year[i-1] + t31;
    i++;			/* nov */
    reg_year[i] = reg_year[i-1] + t30;
    leap_year[i] = leap_year[i-1] + t30;
    i++;			/* dec */
    reg_year[i] = reg_year[i-1] + t31;
    leap_year[i] = leap_year[i-1] + t31;
    
    accumulated_years[0] = 0;
    
    for(i=0,yy=REFERENCE_YEAR; i <= MAX_YEARS; i++, yy++ ) {
	accumulated_months[i] = yy % 4 ? reg_year : leap_year;
	if(i) {
	    accumulated_years[i] = accumulated_years[i-1] +
		  *(accumulated_months[i-1] +12);
	}
    }
}
/* c----------------------------------------------------------------------- */
