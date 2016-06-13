/* 	$Id$	 */

# ifndef D_TIME_STRUCT
# define D_TIME_STRUCT

typedef double RTIME;

struct d_time_struct {
    RTIME time_stamp;
    RTIME day_seconds;
    double sub_second;
    int julian_day;
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    int millisecond;
};

typedef struct d_time_struct DD_TIME;
# endif
