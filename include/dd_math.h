# ifndef DDMATHH
# define DDMATHH

# include <math.h>

#define dd_isnanf(x)       (isnan(x))

# define FMOD360(x) (fmod((double)((x)+720.), (double)360.))
# define EXP2(x) (pow((double)2.0, (double)(x)))
# define EXPN(x) (pow((double)2.718281828, (double)(x)))
# define EXP10(x) (pow((double)10.0, (double)(x)))

# define WATTZ(x) (pow((double)10.0, (double)((x) * .1)))
# define W_TO_DBM(x) (10. * log10((double)(x)))

# define LOG10(x) (log10((double)(x)))
# define LOGN(x) (log((double)(x)))
# define SQRT(x) (sqrt((double)(x)))
# define FABS(x) (fabs((double)(x)))
# define SIN(x) (sin((double)(x)))
# define COS(x) (cos((double)(x)))
# define TAN(x) (tan((double)(x)))
# define ATAN2(y,x) (atan2((double)(y), (double)(x)))
# define ASIN(x) (asin((double)(x)))
# define ACOS(x) (acos((double)(x)))
# define ATAN(x) (atan((double)(x)))

# define M_TO_KM(x) ((x) * .001)
# define KM_TO_M(x) ((x) * 1000.)

# define CART_ANGLE(x) ((double)90.-(x))
# define RADIANS(x) ((x)*0.017453292)
# define DEGREES(x) ((x)*57.29577951)

# define PIOVR2 1.570796327
# define PI 3.141592654
# define SQ(x) ((x)*(x))
# define TWOPI 6.283185307

# endif
