# define X2(x) (swap2((char *)&(x)))
# define X4(x) (swap4((char *)&(x)))

#include <union.hh>

/* c------------------------------------------------------------------------ */

int 
main (void) {
    union abc *def;
    int i=8;

    i = swap2((char *)&i);
    i = X2(i);

    def = &fsrp;
    def->fsrpp.rad_const = 1.;
    fsrp.fsrpp.rad_const = 1.;
}
/* c------------------------------------------------------------------------ */

short 
swap2 (		/* swap integer*2 */
    char *ov
)
{
    union {
	short newval;
	char nv[2];
    }u;
    u.nv[1] = *ov++; u.nv[0] = *ov++;
    return(u.newval);
}
/* c------------------------------------------------------------------------ */

int32_t 
swap4 (		/* swap integer*4 */
    char *ov
)
{
    union {
	int32_t newval;
	char nv[4];
    }u;
    u.nv[3] = *ov++; u.nv[2] = *ov++; u.nv[1] = *ov++; u.nv[0] = *ov++;
    return(u.newval);
}
/* c------------------------------------------------------------------------ */
