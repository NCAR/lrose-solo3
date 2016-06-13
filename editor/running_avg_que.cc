/* 	$Id: dorade_share.cc 487 2011-11-16 14:22:57Z rehak $	 */

#include <iostream>
#include <stdlib.h>
#include <string.h>

#include <running_avg_que.hh>

static struct running_avg_que *top_raq=NULL;

/* c------------------------------------------------------------------------ */

int RAQ_free(void)
{
    int ii;
    struct running_avg_que *raq=top_raq;

    if(!top_raq)
	  return(0);
    for(ii=0; raq; ii++,raq->in_use=false,raq=raq->next);
    return(ii);
}

/* c------------------------------------------------------------------------ */

struct running_avg_que *RAQ_return_queue(int nvals)
{
    /* routine to return a pointer to the next free
     * running average struct/queue set up to average "nvals"
     */
    struct que_val *qv, *qv_next, *qv_last = 0;
    struct running_avg_que *raq, *last_raq;
    int jj;

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
    raq->in_use = true;

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
