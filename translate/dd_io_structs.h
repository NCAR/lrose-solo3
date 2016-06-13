/* 	$Id$	 */

# ifndef DD_IO_STRUCTS_H
# define DD_IO_STRUCTS_H

# include <stdio.h>

# define MAX_OPENED_FILES 7
# define MAX_LOG_PACKETS 100

struct input_buf {
    struct input_buf *last;
    struct input_buf *next;
    char *buf;
    char *at;
    int read_state;
    int skip_state;
    int eof_flag;
    int32_t sizeof_read;
    int32_t offset;
    int seq_num;
    int b_num;
    int bytes_left;
};

struct io_packet {
    struct io_packet *last;
    struct io_packet *next;
    struct input_buf *ib;
    char *at;
    int len;
    int seq_num;
    int bytes_left;
};

struct input_read_info {
    struct input_buf *top;
    struct io_packet *packet_que;
    float f_bytes;
    int32_t current_offset;
    int seq_num;
    int fmt;
    int fid;
    int io_type;
    int read_ahead_count;
    int read_count;
    int que_count;
    int packet_count;
    int sizeof_bufs;
    int32_t sizeof_file;
    char dev_name[256];
    char directory_name[128];
    int min_block_size;
    FILE *strm;
};

# endif	/* DD_IO_STRUCTS_H */
