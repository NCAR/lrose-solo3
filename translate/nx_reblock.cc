#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <fcntl.h>
#include <iostream>
//#include <fstream>
#include "nx_reblock.hh"
#include "dorade_share.hh"

/* reblock a NEXRAD Archive 2 file into Fortran Binary I/O, so
   the standard translator can process it

   strip off the header, which contains 'ARCHIVE2.'
   subsequent records are written in Fortran Binary format, where each block
   is proceeded and followed by its length
   */

#define SZ_HEADER 2456

#define SZ_NEXRAD_REC 2432

int
main(int argc, char *argv[])
{
    int i;
    
    if (argc <2) {
	fprintf(stderr, "usage: %s nexrad_a2file . . .\n", argv[0]);
    }
    for (i = 1; i < argc; ++i) {
	reblock(argv[i]);
    }
}

void reblock(char *filename)
{
    int fd = open(filename, O_RDONLY);
    char namebuf[MAXPATHLEN];
    int fout = -1;
    char databuf[SZ_HEADER];
    unsigned char ctm_header[] = {0x00,0x00,0x00,0x00,
			          0x09,0x80,0x00, 0x00};
    int sz;
    unsigned int fpos = 0;

    if (fd == -1) {
	perror("reblock open read");
    	return;
    }
    sprintf(namebuf, "%s.nxfb", filename);

    fout = open(namebuf, O_WRONLY| O_CREAT, 0664);

    if (fout == -1) {
	perror("reblock open output");
    	return;
    }

    sz = read(fd, databuf, SZ_HEADER);
    if (sz != SZ_HEADER) {
	perror("reblock read header");
    	return;
    }
    fpos = SZ_HEADER;
    while ((sz = read(fd, databuf, SZ_NEXRAD_REC)) == SZ_NEXRAD_REC) {
    	/* check CTM message fields and record length */
	if ( (databuf[4] != '\x09') || (databuf[5] != '\x80') ||
	    (databuf[12] != '\x04') || (databuf[13] != '\xB8') ) {
	    fprintf(stderr, "warning - ctm header mismatch at %d\n",
		    fpos);
    	} else {
    	    if (fb_write(fout, databuf, SZ_NEXRAD_REC) != SZ_NEXRAD_REC) {
		perror("write error");
		break;
	    }
    	}
    	fpos += SZ_NEXRAD_REC;
    }
    close(fout);
    close(fd);
    return;
}
/* needed by some access routines in the library */
void 
solo_message (	/* some of the access routines call
				 * this routine
				 */
    const char *message
)
{
    printf("%s", message);
}
