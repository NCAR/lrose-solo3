/* 	$Id$	 */

# ifndef INCtime_dependent_fixesh
# define INCtime_dependent_fixesh


struct time_dependent_fixes {
    struct time_dependent_fixes *last;
    struct time_dependent_fixes *next;
    double start_time;
    double stop_time;
    float pointing_angle;
    float cell_spacing;
    float baseline;
    float counts_per_db;
    float end_range;
    int num_cells;

    int typeof_fix;
    int use_it;
    int node_num;
};

typedef struct time_dependent_fixes *TDF_PTR;

# define      FIX_CELL_SPACING 1
# define    FIX_POINTING_ANGLE 2
# define          RED_BASELINE 3
# define        GREEN_BASELINE 4
# define    RED_COUNTS_PER_DB  5
# define  GREEN_COUNTS_PER_DB  6
# define         RED_END_RANGE 7
# define       GREEN_END_RANGE 8
# define         FIX_NUM_CELLS 9

# endif /* INCtime_dependent_fixesh */
