#ifndef DD_CRACKERS_H
#define DD_CRACKERS_H


#ifdef __cplusplus
extern "C" {
#endif
extern void piraq_crack_header (char *srs, char *dst, int limit, int sparc_alignment);
extern void piraq_crack_radar (char *srs, char *dst, int limit, int sparc_alignment);

#ifdef __cplusplus
}
#endif


#endif /* DD_CRACKERS_H */
