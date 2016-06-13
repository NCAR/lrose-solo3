/* 	$Id$	 */

# ifndef DDFILESH
# define DDFILESH

/* file type id */
# define SWP_FILE 1
# define VOL_FILE 2
# define CAT_FILE 3
# define DOR_FILE 4
# define UFD_FILE 5
# define GDE_FILE 6
# define BND_FILE 7
# define XXX_FILE 8
# define PPT_FILE 9
# define PIC_FILE 10
# define CPI_FILE 11

# define DD_FILTER_TIME_BEFORE  4
# define        DD_TIME_BEFORE  2
# define       DD_TIME_NEAREST  0
# define         DD_TIME_AFTER  1
# define  DD_FILTER_TIME_AFTER  3

# define    DD_LATEST_VERSION -1
# define  DD_EARLIEST_VERSION -2
# define   DD_EXHAUSTIVE_LIST -3

struct dd_file_name {
    int32_t time_stamp;
    short file_type;
    short radar_num;
    short version;
    short num_versions;
    char radar_name[12];
};

struct dd_file_name_v2 {
    int32_t time_stamp;
    short file_type;
    short radar_num;
    short version;
    short num_versions;
    char radar_name[12];
    short milliseconds;
};

struct dd_file_name_v3 {
    double time;
    struct dd_file_name_v3 *last;
    struct dd_file_name_v3 *next;

    struct dd_file_name_v3 *parent;
    struct dd_file_name_v3 *right;
    struct dd_file_name_v3 *left;

    struct dd_file_name_v3 *ver_right;
    struct dd_file_name_v3 *ver_left;

    double time_stamp;
    char radar_name[12];
    unsigned char version;
    unsigned char red_link;
    short milliseconds;
    char comment[88];
};

# endif




