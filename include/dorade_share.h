#ifndef dorade_share_h
#define dorade_share_h

# include <time.h>
# include <sys/time.h>

/*
 * A sweep file consists of a super_SWIB at the beginning
 * the requisite number of parameter descriptors "PARM"
 * and the requisite RYIB, ASIB and RDAT blocks.
 * last record in sweep should be a "NULL" record
 * the file might also contain additional structures to
 * describe the various processing steps
 */

/* c------------------------------------------------------------------------ */
/* c------------------------------------------------------------------------ */

#ifndef RUNNING_AVG_QUE
#define RUNNING_AVG_QUE

struct que_val {
    int val;
    float f_val;
    double d_val;
    struct que_val *last;
    struct que_val *next;
};

struct running_avg_que {
    double sum;
    double rcp_num_vals;
    int in_use;
    int num_vals;
    struct que_val *at;
    struct running_avg_que *last;
    struct running_avg_que *next;
};

#endif

/* c------------------------------------------------------------------------ */

#ifndef SOLO_LIST_MGMT
#define SOLO_LIST_MGMT
#define SLM_CODE

struct solo_list_mgmt {
    int num_entries;
    int sizeof_entries;
    int max_entries;
    char **list;
};

# endif

/* c------------------------------------------------------------------------ */
struct io_stuff {
    int fid;
    int io_type;
    int rlen;
    int status;
    int whats_left;
    char *at;
};

# endif
