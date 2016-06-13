/* created using cproto */
/* Tue Jun 21 22:05:29 UTC 2011*/

#ifndef running_avg_que_hh
#define running_avg_que_hh

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
    bool in_use;
    int num_vals;
    struct que_val *at;
    struct running_avg_que *last;
    struct running_avg_que *next;
};

#endif

extern int RAQ_free(void);
extern struct running_avg_que *RAQ_return_queue(int nvals);

#endif
