/* 	$Id$	 */

# ifndef super_SWIB_hh
# define super_SWIB_hh

# define   MAX_KEYS 8

# define      KEYED_BY_TIME 1
# define   KEYED_BY_ROT_ANG 2
# define  SOLO_EDIT_SUMMARY 3

# define        NDX_ROT_ANG 0
# define           NDX_SEDS 1

# define SWIB_NEW_VOL 0x1

struct key_table_info {
    int32_t offset;
    int32_t size;
    int32_t type;
};

struct super_SWIB {
    char name_struct[4];	/* "SSWB" */
    int32_t sizeof_struct;
    /* parameters from the first version */
    int32_t last_used;		/* Unix time */
    int32_t start_time;
    int32_t stop_time;
    int32_t sizeof_file;
    int32_t compression_flag;
    int32_t volume_time_stamp;	/* to reference current volume */
    int32_t num_params;		/* number of parameters */

    /* end of first version parameters */

    char radar_name[8];

    double d_start_time;
    double d_stop_time;
    /*
     * "last_used" is an age off indicator where > 0 implies Unix time
     * of the last access and
     * 0 implies this sweep should not be aged off
     */
    int32_t version_num;
    int32_t num_key_tables;
    int32_t status;
    int32_t place_holder[7];
    struct key_table_info key_table[MAX_KEYS];
    /*
     * offset and key info to a table containing key value such as
     * the rot. angle and the offset to the corresponding ray
     * in the disk file
     */
};

struct super_SWIB_v0 {
    char name_struct[4];	/* "SSWB" */
    int sizeof_struct;

    int32_t last_used;		/* Unix time */
    /*
     * "last_used" is an age off indicator where > 0 implies Unix time
     * of the last access and
     * 0 implies this sweep should not be aged off
     */
    int32_t start_time;
    int32_t stop_time;
    int sizeof_file;
    int compression_flag;
    int32_t volume_time_stamp;	/* to reference current volume */
    int num_params;		/* number of parameters */
};

typedef struct super_SWIB super_SWIB;
typedef struct super_SWIB SUPERSWIB;

# endif  /* INCsuper_SWIBh */

