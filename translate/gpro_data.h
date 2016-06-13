/*      $Id$        */
#ifndef gpro_data_h
#define gpro_data_h

# define GOOD 1
# define AMBIVALENT 0
# ifndef BAD
# define BAD -1
# endif
# define UGLY -2
# define MAX_VARS 555
# define LINE_LEN 80
# define VAR_LEN 10
# define GP_BITS 32
# define GP_SKIP 0
# define GP_CONKEY 1
# define GP_SCLKEY 1
# define GP_TERM 1000.
# define GP_FACTOR 1000.
# define GP_RCP_FACTOR (1./GP_FACTOR)
# define GP_UNSCALE(n) ((float)(n)*GP_RCP_FACTOR -GP_TERM)
# define GP_DUNSCALE(n) ((double)(n)*GP_RCP_FACTOR -GP_TERM)
# define GP_SCALE(f) ((int)(((f)+GP_TERM)*GP_FACTOR +.5))
# define TTITLE 1
# define TNAME 3
# define UTITLE 1
# define SAMPLE 2
# define RATE 3
# define BITS 4
# define FSTBIT 5
# define SKIP 6
# define UNAME 8
# define CONKEY 1
# define SCLKEY 2
# define TERM 3
# define FACTOR 4
# define CNAME 6

# define         GPRO_ID_HR 1
# define        GPRO_ID_MIN 2
# define        GPRO_ID_SEC 3
# define      GPRO_ID_PITCH 4
# define       GPRO_ID_ROLL 5
# define        GPRO_ID_THI 6
# define       GPRO_ID_ALAT 7
# define       GPRO_ID_ALON 8
# define      GPRO_ID_GMOD1 9
# define      GPRO_ID_GSTAT 10
# define       GPRO_ID_GLAT 11
# define       GPRO_ID_GLON 12
# define       GPRO_ID_GALT 13
# define      GPRO_ID_ALAT2 14
# define      GPRO_ID_ALON2 15
# define        GPRO_ID_VNS 16
# define        GPRO_ID_VEW 17
# define       GPRO_ID_GVNS 18
# define       GPRO_ID_GVEW 19
# define         GPRO_ID_UI 20
# define         GPRO_ID_VI 21
# define       GPRO_ID_SSRD 22
# define       GPRO_ID_PALT 23
# define       GPRO_ID_HGME 24
# define      GPRO_ID_IVSPD 25
# define         GPRO_ID_WI 26
# define      GPRO_ID_PTIME 27


# define  GPRO_UNSIGNED_INT 1
# define    GPRO_SIGNED_INT 2
# define       GPRO_IEEE_FP 11

# define ALMOST_ONE_DAY 82800
# define PIRAD   9.54e-7*360    /* converted from radians to degrees */
# define LINE_LEN 80
# define FSTBIT_TO_OFFSET(x) ((x) >> 3)
# define GPRO_UNSCALE(x,rcp_scale,offset) ((float)(x)*(rcp_scale)-(offset))
# define MAX_RDQ 3
# define MAX_VBLS 88
# define ELD_ROLL_LIMIT 5.      /* degrees */
# define MAX_POS_AVG 12

# define PALT_CORR_1600 .123
# define PALT_CORR_4600 .264

struct letvar_info {
    struct letvar_info *last;
    struct letvar_info *next;
    float term;
    float rcp_factor;

    int id_num;
    int offset;
    int binc;
    int sinc;
    int linc;

    int fstbit;
    int bits;
    int skip;
    int sample;
    int conkey;

    char *letvar;
    char name[VAR_LEN+2];
};
struct ads_data {
    double time;
    float pitch;
    float roll;
    float thi;
    float alat;
    float alon;
    float palt;
    float hgme;
    float ivspd;
    float alat2;
    float alon2;
    float glat;
    float glon;
    float galt;
    float vns;
    float vew;
    float gvns;
    float gvew;
    float ui;
    float vi;
    float wi;
    float drift;
    int itime;
    int ptime;
};

struct ads_raw_data_que {
    double time;
    struct ads_raw_data_que *last;
    struct ads_raw_data_que *next;
    int32_t *at[MAX_VBLS];
    char *raw_data_buf;
};


struct requested_variables_list {
    struct requested_variables_list *last;
    struct requested_variables_list *next;
    int id_num;
    char *name;
};

struct running_avg {
    struct running_avg *last;
    struct running_avg *next;
    float val;
};

struct ins_errors {
    float alat_err;
    float alon_err;
    float sum_alat_err;
    float sum_alon_err;
    float rcp_num;
    struct running_avg *top_alat_err;
    struct running_avg *top_alon_err;
    int num;
};

#endif
