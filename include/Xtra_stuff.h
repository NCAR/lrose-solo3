/* 	$Id$	 */

# ifndef XTRA_STUFF_H
# define XTRA_STUFF_H

struct dd_extra_stuff {		/* extra container for non DORADE structs */
  char name_struct[4];		/* "XSTF" */
  int32_t sizeof_struct;

  int32_t one;			/* always set to one (endian flag) */
  int32_t source_format;		/* as per ../include/dd_defines.h */

  int32_t offset_to_first_item;	/* bytes from start of struct */
  int32_t transition_flag;
};

typedef struct dd_extra_stuff XTRA_STUFF;

# endif
