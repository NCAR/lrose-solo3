/* 	$Id$	 */

# ifndef RUN_SUM_H
# define RUN_SUM_H
/* c------------------------------------------------------------------------ */

struct run_sum {
    double sum;
    double short_sum;
    double *vals;
    int count;
    int index_next;
    int index_lim;
    int run_size;
    int short_size;
};
/* c------------------------------------------------------------------------ */
# endif
