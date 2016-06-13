/* qdap.F -- translated by f2c (version 20090411).
   You must link the resulting object file with libf2c:
	on Microsoft Windows system, link with libf2c.lib;
	on Linux or Unix systems, link with .../path/to/libf2c.a -lm
	or, if you install libf2c.a in a standard place, with -lf2c -lm
	-- in that order, at the end of the command line, as in
		cc *.o -lf2c -lm
	Source for libf2c is in /netlib/f2c/libf2c.zip, e.g.,

		http://www.netlib.org/f2c/libf2c.zip
*/

#include "f2c_local.h"

/* Common Block Declarations */

union {
    struct {
	integer ncs, taptim[6], ccstm1[6], ccstm2[6], lenbuf, lundat, nfiles, 
		intpt, intno, ivol, iscan, irec, krec, ueof;
    } _1;
    struct {
	integer ncs, taptim[6], ccstm1[6], ccstm2[6], lenbuf, lundat, nfiles, 
		intpt, intno, ivol, iscan, irec, krec;
	logical ueof;
    } _2;
} ufapu_;

#define ufapu_1 (ufapu_._1)
#define ufapu_2 (ufapu_._2)

struct {
    char ufcs[2222];
} ufapc_;

#define ufapc_1 ufapc_

/* Table of constant values */

static integer c__6 = 6;
static integer c__1 = 1;
static integer c__0 = 0;
static integer c__2 = 2;
static integer c_n1 = -1;
static integer c__4 = 4;
static integer c__9 = 9;
static integer c__3 = 3;

/* 	$Id: qdap.F 2 2002-07-02 14:57:33Z oye $ */
/* ---------------------------------------------------------------------- */

/*       This file contains the following routines */

/*       givfld */
/*       givrng */
/*       present */
/*       setcat */
/*       setdpr */
/*       setpsn */
/*       setray */

/*       accver */
/*       mountd */






/* ---------------------------------------------------------------------- */
/* Subroutine */ int givfld_(integer *kbuf, char *name__, real *r1, real *r2, 
	integer *m, real *vector, integer *lenout, real *rzero, real *gs, 
	real *badval, ftnlen name_len)
{
    /* System generated locals */
    integer i__1, i__2;

    /* Local variables */
    static char id[2];
    extern /* Subroutine */ int dap_givfld__(integer *, integer *, integer *, 
	    real *, real *);
    static integer ig1, ig2;
    static real r0x;
    static integer idn, igt, lenx;
    extern integer dap_ndx_name__(char *, ftnlen);
    static integer ngts;
    extern /* Subroutine */ int dap_rng_info__(integer *, integer *, real *, 
	    integer *), typstg_(integer *, char *, integer *, integer *, 
	    ftnlen);


/*       return the values for the "name" field converted to scientific units */
/*       between the ranges "r1" and "r2". */



    /* Parameter adjustments */
    --vector;
    --kbuf;

    /* Function Body */
    if (*r1 > *r2) {
	typstg_(&c__6, "r1 is greater than r2 ^", &c__1, &c__0, (ftnlen)23);
    }
    *lenout = 0;
/*       determine the source id of the "name" field */
    idn = dap_ndx_name__(name__, (ftnlen)2);
    if (idn < 0) {
	goto L90;
    }

/*       range to first gate and gate spacing (ask for 2 gates of range info) */
    dap_rng_info__(&c__1, &c__2, &vector[1], &ngts);
    r0x = vector[1];
    *gs = vector[2] - r0x;

/*       exit if zero gate spacing for no data */
    if (*gs == 0.f || ngts == 0) {
	typstg_(&c__6, id, &c__1, &c__2, (ftnlen)2);
	typstg_(&c__6, "data is garbage ^", &c__1, &c__0, (ftnlen)17);
	goto L90;
    }

/* Computing MAX */
    i__1 = 1, i__2 = (integer) ((*r1 - r0x) / *gs + 2.f);
    ig1 = max(i__1,i__2);
/* Computing MIN */
    i__1 = ngts, i__2 = (integer) ((*r2 - r0x) / *gs + 1.f);
    ig2 = min(i__1,i__2);
/*       range to the first gate returned */
    *rzero = r0x + (ig1 - 1) * *gs;
    lenx = ig2 - ig1 + 1;

/*   for now since there can be variable gate spacing just */
/*   return all the gates */
    *rzero = vector[1];
    lenx = ngts;
    if (*m < 1) {
/*       assume this is just a request for range and gate info */
	*lenout = lenx;
	goto L90;
    }
/*       no. gates to unpack */
    *lenout = min(lenx,*m);

    dap_givfld__(&idn, &ig1, lenout, &vector[1], badval);

    i__1 = *m;
    for (igt = *lenout + 1; igt <= i__1; ++igt) {
	vector[igt] = *badval;
/* L33: */
    }


L90:
    return 0;
} /* givfld_ */

/* ---------------------------------------------------------------------- */
/* Subroutine */ int givgts_(integer *kbuf, char *name__, integer *ig1, 
	integer *m, real *vector, integer *lenout, real *badval, ftnlen 
	name_len)
{
    /* System generated locals */
    integer i__1;

    /* Local variables */
    static real x;
    static integer ng;
    extern /* Subroutine */ int dap_givfld__(integer *, integer *, integer *, 
	    real *, real *);
    static integer idn, igt;
    extern integer dap_ndx_name__(char *, ftnlen);
    extern /* Subroutine */ int dap_rng_info__(integer *, integer *, real *, 
	    integer *);


/*       return the values for the "name" field converted to scientific units */
/*       between the gates ig1, ig2 */



    /* Parameter adjustments */
    --vector;
    --kbuf;

    /* Function Body */
    *lenout = 0;
/*       determine the source id of the "name" field */
    idn = dap_ndx_name__(name__, (ftnlen)2);
    if (idn < 0) {
	goto L90;
    }

/*       no. gates to unpack */
    dap_rng_info__(&c__1, &c__1, &x, &ng);

    if (*ig1 - 1 + *m > ng) {
	*lenout = ng - (*ig1 - 1);
    } else {
	*lenout = *m;
    }

    dap_givfld__(&idn, ig1, lenout, &vector[1], badval);

    i__1 = *m;
    for (igt = *lenout + 1; igt <= i__1; ++igt) {
	vector[igt] = *badval;
/* L33: */
    }

L90:
    return 0;
} /* givgts_ */

/* ---------------------------------------------------------------------- */
/* Subroutine */ int givrng_(integer *kbuf, integer *name__, real *r1, real *
	r2, integer *m, real *vector, integer *lenout, real *rzero, real *gs, 
	logical *changed, integer *ig1)
{
    /* System generated locals */
    integer i__1;

    /* Local variables */
    static real rg[2];
    static integer ig2;
    static real rr1, rr2;
    static integer ngts;
    extern /* Subroutine */ int dap_rng_info__(integer *, integer *, real *, 
	    integer *), dap_range_gate__(real *, integer *, real *);


/*   the purpose of givrng is to return the number of the first gate */
/*   and the number of gates following to cover the ranges "r1" and "r2"; */
/*   (this info can be used with "givgts") */
/*   to fill vector with the range values corresponding to these */
/*   particular gates; */
/*   and to return a flag indicating that the range information will */
/*   be different from the information passed in in vector */

/*   It is done this way to accommodate variable gate spacing */



    /* Parameter adjustments */
    --vector;
    --kbuf;

    /* Function Body */
    dap_range_gate__(r1, ig1, &rr1);
    dap_range_gate__(r2, &ig2, &rr2);
    *lenout = ig2 - *ig1 + 1;
    dap_rng_info__(ig1, &c__2, rg, &ngts);
    *rzero = rg[0];
    *gs = rg[1] - rg[0];
    *lenout = min(*m,*lenout);
    i__1 = *ig1 - 1 + *lenout;
    dap_rng_info__(&i__1, &c__1, &rr2, &ngts);
    *changed = vector[1] != rr1 || vector[*lenout] != rr2;

    if (*changed && *lenout > 0) {
	dap_rng_info__(ig1, lenout, &vector[1], &ngts);
    }

/* L90: */
    return 0;
} /* givrng_ */

/* --------------------------------------------------------------------- */
logical present_(integer *kbuf, char *mne, ftnlen mne_len)
{
    /* System generated locals */
    logical ret_val;

    /* Local variables */
    extern integer dap_parm_present__(char *, ftnlen);
    static integer i__;

/*   this routine determines if the source field for this mnemonic is present */
/*   in the input data. */



    /* Parameter adjustments */
    --kbuf;

    /* Function Body */
    i__ = dap_parm_present__(mne, mne_len);
    if (i__ == -1) {
	ret_val = FALSE_;
    } else {
	ret_val = TRUE_;
    }

    return ret_val;
} /* present_ */

/* --------------------------------------------------------------------- */
logical prznt_(integer *kbuf, char *mne, ftnlen mne_len)
{
    /* System generated locals */
    logical ret_val;

    /* Local variables */
    extern integer dap_parm_present__(char *, ftnlen);
    static integer i__;

/*   this routine determines if the source field for this mnemonic is present */
/*   in the input data. */



    /* Parameter adjustments */
    --kbuf;

    /* Function Body */
    i__ = dap_parm_present__(mne, mne_len);
    if (i__ == -1) {
	ret_val = FALSE_;
    } else {
	ret_val = TRUE_;
    }

    return ret_val;
} /* prznt_ */

/* ---------------------------------------------------------------------- */
/* Subroutine */ int setcat_(integer *buffer, integer *lenbf, integer *lundx, 
	integer *lucat, integer *ludat, char *uics, integer *lens, integer *
	ierr, ftnlen uics_len)
{

    /* System generated locals */
    integer i__1;

    /* Local variables */
    static integer i__, ia, na, ix, ia1;
    static char str[77];
    static integer iarg;
    extern /* Subroutine */ int copyc_(char *, integer *, integer *, char *, 
	    integer *, ftnlen, ftnlen);
    extern integer nwds16_(integer *);
    extern /* Subroutine */ int putfx_(integer *, integer *), accver_(void);
    extern integer indexa_(char *, integer *, integer *, integer *, char *, 
	    ftnlen, ftnlen), namefx_(char *, integer *, ftnlen);
    extern /* Subroutine */ int attarg_(char *, integer *, integer *, integer 
	    *, integer *, integer *, ftnlen), cdatmz_(char *, integer *, 
	    integer *, integer *, integer *, ftnlen), mountd_(integer *), 
	    typstg_(integer *, char *, integer *, integer *, ftnlen);


/*  this routine scans an input control string for attributes */
/*  that specify which tapes to mount and where to start and stop */
/*  reading data. */
/*  if all necessary information is found, then on return from this */
/*  routine the first useable record should be in the record buffer. */





    /* Parameter adjustments */
    --buffer;

    /* Function Body */


    *ierr = 0;
/*   print out the access version */
    accver_();
    ufapu_1.nfiles = 0;
    ufapu_1.ivol = 0;
    ufapu_1.iscan = 0;
    ufapu_1.lenbuf = *lenbf;
    ix = namefx_("REC_LEN", &i__, (ftnlen)7);
    i__1 = nwds16_(&ufapu_1.lenbuf);
    putfx_(&i__, &i__1);
    ufapu_1.lundat = *ludat;

    if (*lens < 1) {
	typstg_(&c__6, "no ufap input control string  ^", &c__1, &c_n1, (
		ftnlen)31);
	*ierr = 999;
	goto L90;
    }

/*  copy and compress the input control string */
    ufapu_1.ncs = 1;
    *(unsigned char *)ufapc_1.ufcs = ';';
    i__1 = *lens;
    for (i__ = 1; i__ <= i__1; ++i__) {
	if (*(unsigned char *)&uics[i__ - 1] != ' ') {
	    ++ufapu_1.ncs;
	    *(unsigned char *)&ufapc_1.ufcs[ufapu_1.ncs - 1] = *(unsigned 
		    char *)&uics[i__ - 1];
	}
/* L11: */
    }
    ++ufapu_1.ncs;
    *(unsigned char *)&ufapc_1.ufcs[ufapu_1.ncs - 1] = ';';


    ia = indexa_(ufapc_1.ufcs, &c__1, &ufapu_1.ncs, &c__4, "SHOW", (ftnlen)
	    2222, (ftnlen)4);
    if (ia > 0) {
	iarg = 1;
L22:
	attarg_(ufapc_1.ufcs, &ia, &ufapu_1.ncs, &iarg, &ia1, &na, (ftnlen)
		2222);
	if (na > 0) {
	    copyc_(ufapc_1.ufcs, &ia1, &na, str, &c__1, (ftnlen)2222, (ftnlen)
		    77);
	    ix = namefx_(str, &i__, na);
	    putfx_(&i__, &c__1);
	    ++iarg;
	    goto L22;
	}
    }

/*  extract the start and stop times */
    cdatmz_(ufapc_1.ufcs, &ufapu_1.ncs, ufapu_1.ccstm1, ufapu_1.ccstm2, ierr, 
	    (ftnlen)2222);
    if (*ierr != 0) {
	goto L90;
    }

/*  get set to access the data and read the first record */
    mountd_(ierr);
    if (*ierr != 0) {
	goto L90;
    }

/*  read in the first record */
/*        call setray( buffer, 1, ierr ) */
    if (*ierr != 0) {
	goto L90;
    }


L90:
    return 0;
} /* setcat_ */

/* ---------------------------------------------------------------------- */
/* Subroutine */ int setray_(integer *kbuf, integer *kdir, integer *ierr)
{
    extern integer dap_next_ray__(integer *, integer *);
    static integer state;
    extern integer kvoln_(integer *);
    static integer kount;
    extern integer kprnv_(integer *), kswepn_(integer *);


/*       move to the next ray */
/*       ignore kdir --- always go forward */






    /* Parameter adjustments */
    --kbuf;

    /* Function Body */
    *ierr = 0;
    kount = 0;
    ufapu_2.ueof = FALSE_;

/* L11: */
    state = dap_next_ray__(&ufapu_2.lundat, &kbuf[1]);

    if (state == -9) {
	*ierr = 999;
	goto L90;
    }

    ufapu_2.iscan = kswepn_(&kbuf[1]);
    ufapu_2.ivol = kvoln_(&kbuf[1]);
    ufapu_2.irec = kprnv_(&kbuf[1]);

L90:
    return 0;
} /* setray_ */

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/*           Unix DEPENDENT CODE */
/* ----------------------------------------------------------------------- */
/* Subroutine */ int accver_(void)
{
    extern /* Subroutine */ int dmplin_(char *, integer *, ftnlen);

/*   dump out the current access version date */
    dmplin_("qDAP unix VERSION Novermber 1997", &c__0, (ftnlen)32);
    return 0;
} /* accver_ */

/* ----------------------------------------------------------------------- */
/* Subroutine */ int mountd_(integer *ierr)
{
    /* System generated locals */
    integer i__1;

    /* Builtin functions */
    integer s_wsle(cilist *), do_lio(integer *, integer *, char *, ftnlen), 
	    e_wsle(void);

    /* Local variables */
    extern /* Subroutine */ int time2unix_(integer *, integer *);
    extern integer dap_mnt_f__(integer *, integer *, integer *, integer *, 
	    integer *, integer *, integer *), dap_mnt_s__(integer *, integer *
	    , integer *, integer *, integer *, integer *, integer *);
    static integer i1, directory[40], in, ni, radar_name__[4], input_file__[
	    40], tm1, tm2, dir;
    extern /* Subroutine */ int put8_(integer *, integer *, integer *);
    static integer kbuf;
    extern integer kvoln_(integer *), kprnv_(integer *), indexa_(char *, 
	    integer *, integer *, integer *, char *, ftnlen, ftnlen);
    static integer buffer;
    extern /* Subroutine */ int attarg_(char *, integer *, integer *, integer 
	    *, integer *, integer *, ftnlen), copycn_(char *, integer *, 
	    integer *, integer *, integer *, ftnlen), setray_(integer *, 
	    integer *, integer *);
    extern integer kswepn_(integer *);

    /* Fortran I/O blocks */
    static cilist io___38 = { 0, 6, 0, 0, 0 };
    static cilist io___41 = { 0, 6, 0, 0, 0 };
    static cilist io___42 = { 0, 6, 0, 0, 0 };


/*   make sure that the tape with name as identifier is mounted */
/*   on unit lun */






    *ierr = 0;

/*   convert times to unix time */
    time2unix_(ufapu_2.ccstm1, &tm1);
    time2unix_(ufapu_2.ccstm2, &tm2);

    in = indexa_(ufapc_1.ufcs, &c__1, &ufapu_2.ncs, &c__4, "INSTRUMENT", (
	    ftnlen)2222, (ftnlen)10);
    if (in > 0) {
	attarg_(ufapc_1.ufcs, &in, &ufapu_2.ncs, &c__1, &i1, &ni, (ftnlen)
		2222);
    }
    if (in > 0 && ni > 0) {
	copycn_(ufapc_1.ufcs, &i1, &ni, radar_name__, &c__1, (ftnlen)2222);
    } else {
	ni = 4;
	copycn_("NONE", &c__1, &ni, radar_name__, &c__1, (ftnlen)4);
    }
    i__1 = ni + 1;
    put8_(radar_name__, &i__1, &c__0);

/*   open the file and read to the first usable ray */

    in = indexa_(ufapc_1.ufcs, &c__1, &ufapu_2.ncs, &c__4, "INPUT", (ftnlen)
	    2222, (ftnlen)5);
    dir = indexa_(ufapc_1.ufcs, &c__1, &ufapu_2.ncs, &c__4, "DIRECTORY", (
	    ftnlen)2222, (ftnlen)9);

    if (in > 0) {
	attarg_(ufapc_1.ufcs, &in, &ufapu_2.ncs, &c__1, &i1, &ni, (ftnlen)
		2222);
	copycn_(ufapc_1.ufcs, &i1, &ni, input_file__, &c__1, (ftnlen)2222);
	i__1 = ni + 1;
	put8_(input_file__, &i__1, &c__0);
	*ierr = dap_mnt_f__(input_file__, radar_name__, &tm1, &tm2, &
		ufapu_2.ivol, &ufapu_2.iscan, &ufapu_2.irec);
	if (*ierr != 0) {
	    s_wsle(&io___38);
	    do_lio(&c__9, &c__1, "Error mountd first read...err: ", (ftnlen)
		    31);
	    do_lio(&c__3, &c__1, (char *)&(*ierr), (ftnlen)sizeof(integer));
	    e_wsle();
	    goto L90;
	}
    } else if (dir > 0) {
	attarg_(ufapc_1.ufcs, &dir, &ufapu_2.ncs, &c__1, &i1, &ni, (ftnlen)
		2222);
	if (ni > 0) {
	    copycn_(ufapc_1.ufcs, &i1, &ni, directory, &c__1, (ftnlen)2222);
	    i__1 = ni + 1;
	    put8_(directory, &i__1, &c__0);
	    *ierr = dap_mnt_s__(directory, radar_name__, &tm1, &tm2, &
		    ufapu_2.ivol, &ufapu_2.iscan, &ufapu_2.irec);
	    if (*ierr == 0) {
/*  read in the first record */
		setray_(&buffer, &c__1, ierr);
		if (*ierr != 0) {
		    s_wsle(&io___41);
		    do_lio(&c__9, &c__1, "Error in mountd on first read...er"
			    "r: ", (ftnlen)37);
		    do_lio(&c__3, &c__1, (char *)&(*ierr), (ftnlen)sizeof(
			    integer));
		    e_wsle();
		    goto L90;
		}
	    }
	} else {
	    *ierr = 999;
	}
    } else {
	*ierr = 999;
    }

    if (*ierr != 0) {
	s_wsle(&io___42);
	do_lio(&c__9, &c__1, "Missing directory or input attribute!", (ftnlen)
		37);
	e_wsle();
	return 0;
    }


    ufapu_2.iscan = kswepn_(&kbuf);
    ufapu_2.ivol = kvoln_(&kbuf);
    ufapu_2.irec = kprnv_(&kbuf);
    ufapu_2.nfiles = 0;
    ufapu_2.krec = 0;

L90:
    return 0;
} /* mountd_ */

/* -------------------------------------------------------------------------- */
/* Subroutine */ int setdpr_(logical *flag__)
{
    extern /* Subroutine */ int dap_setdpr__(integer *);

    if (*flag__) {
	dap_setdpr__(&c__1);
    } else {
	dap_setdpr__(&c__0);
    }
    return 0;
} /* setdpr_ */

/* -------------------------------------------------------------------------- */
doublereal uazim_(void)
{
    /* System generated locals */
    real ret_val;

    /* Local variables */
    static real x;
    extern /* Subroutine */ int cuazim_(real *);

/*   this bunch of crap is necessary because C returns a double precision */
/*   for a real function call */
    cuazim_(&x);
    ret_val = x;
    return ret_val;
} /* uazim_ */

/* -------------------------------------------------------------------------- */
doublereal udmrc_(void)
{
    /* System generated locals */
    real ret_val;

    /* Local variables */
    static real x;
    extern /* Subroutine */ int cudmrc_(real *);

    cudmrc_(&x);
    ret_val = x;
    return ret_val;
} /* udmrc_ */

/* -------------------------------------------------------------------------- */
doublereal udmprf_(real *buf, integer *name__)
{
    /* System generated locals */
    real ret_val;

    /* Local variables */
    static real x;
    extern /* Subroutine */ int cudmprf_(integer *, real *);

    cudmprf_(name__, &x);
    ret_val = x;
    return ret_val;
} /* udmprf_ */

/* -------------------------------------------------------------------------- */
doublereal uelev_(void)
{
    /* System generated locals */
    real ret_val;

    /* Local variables */
    static real x;
    extern /* Subroutine */ int cuelev_(real *);

    cuelev_(&x);
    ret_val = x;
    return ret_val;
} /* uelev_ */

/* -------------------------------------------------------------------------- */
doublereal ufixed_(void)
{
    /* System generated locals */
    real ret_val;

    /* Local variables */
    static real x;
    extern /* Subroutine */ int cufixed_(real *);

    cufixed_(&x);
    ret_val = x;
    return ret_val;
} /* ufixed_ */

/* -------------------------------------------------------------------------- */
doublereal ugealt_(void)
{
    /* System generated locals */
    real ret_val;

    /* Local variables */
    static real x;
    extern /* Subroutine */ int cugealt_(real *);

    cugealt_(&x);
    ret_val = x;
    return ret_val;
} /* ugealt_ */

/* -------------------------------------------------------------------------- */
doublereal uhbwid_(void)
{
    /* System generated locals */
    real ret_val;

    /* Local variables */
    static real x;
    extern /* Subroutine */ int cuhbwid_(real *);

    cuhbwid_(&x);
    ret_val = x;
    return ret_val;
} /* uhbwid_ */

/* -------------------------------------------------------------------------- */
doublereal uhight_(void)
{
    /* System generated locals */
    real ret_val;

    /* Local variables */
    static real x;
    extern /* Subroutine */ int cuhight_(real *);

    cuhight_(&x);
    ret_val = x;
    return ret_val;
} /* uhight_ */

/* -------------------------------------------------------------------------- */
doublereal ulatit_(void)
{
    /* System generated locals */
    real ret_val;

    /* Local variables */
    static real x;
    extern /* Subroutine */ int culatit_(real *);

    culatit_(&x);
    ret_val = x;
    return ret_val;
} /* ulatit_ */

/* -------------------------------------------------------------------------- */
doublereal ulongt_(void)
{
    /* System generated locals */
    real ret_val;

    /* Local variables */
    static real x;
    extern /* Subroutine */ int culongt_(real *);

    culongt_(&x);
    ret_val = x;
    return ret_val;
} /* ulongt_ */

/* -------------------------------------------------------------------------- */
doublereal urotat_(void)
{
    /* System generated locals */
    real ret_val;

    /* Local variables */
    static real x;
    extern /* Subroutine */ int curotat_(real *);

    curotat_(&x);
    ret_val = x;
    return ret_val;
} /* urotat_ */

/* -------------------------------------------------------------------------- */
doublereal uvenyq_(void)
{
    /* System generated locals */
    real ret_val;

    /* Local variables */
    static real x;
    extern /* Subroutine */ int cuvenyq_(real *);

    cuvenyq_(&x);
    ret_val = x;
    return ret_val;
} /* uvenyq_ */

