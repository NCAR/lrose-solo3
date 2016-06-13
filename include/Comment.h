/* 	$Id$	 */
#ifndef Comment_h
#define Comment_h

struct comment_d {
    char comment_des[4];	/* Comment descriptor identifier: ASCII */
				/* characters "COMM" stand for Comment */
				/* Descriptor. */
    int32_t  comment_des_length;	/* Comment descriptor length in bytes. */
    char  comment[500];	        /* comment*/

}; /* End of Structure */



typedef struct comment_d comment_d;
typedef struct comment_d COMMENT;

#endif
