#ifndef DD_TYPES_H
#define DD_TYPES_H

typedef   unsigned int   uint4;
typedef   int   int4;
typedef float float4;

#ifndef _TMS320C6X

#ifdef WIN32
typedef __int64 uint8;    /* for Windows Visual C  */
#else
typedef unsigned long long uint8;
#endif
#endif /* _TMS320C6X */

#endif /* DD_TYPES_H */
