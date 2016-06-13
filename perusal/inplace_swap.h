/*
 * Inline functions to swap two-byte, three-byte, or four-byte values in place.
 */
# ifndef INPLACE_SWAP_H
# define INPLACE_SWAP_H

static inline void inplace_swap2 (unsigned short *loc);
static inline void inplace_swap3 (unsigned char *loc);
static inline void inplace_swap4 (u_int32_t *loc);


inline void
inplace_swap2 (unsigned short* loc)
{
    char* cloc = (char*) loc;
    char tmp = cloc[0];
    cloc[0] = cloc[1];
    cloc[1] = tmp;
}


inline void
inplace_swap3 (unsigned char* loc)
{
    unsigned char tmp = loc[0]; 
    loc[0] = loc[2];
    loc[2] = tmp;
}


inline void
inplace_swap4 (u_int32_t* loc)
{
    char* cloc = (char*) loc;
    char tmp;

    tmp = cloc[0];
    cloc[0] = cloc[3];
    cloc[3] = tmp;

    tmp = cloc[2];
    cloc[2] = cloc[1];
    cloc[1] = tmp;
}

#endif
