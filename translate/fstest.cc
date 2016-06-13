
#include <stdio.h>
#include <stdlib.h>
#include <sys/statfs.h>
#include <fstest.hh>

main (void)
{
    struct statfs *fs;
    int i;
    
    fs = (struct statfs *)malloc(sizeof(struct statfs));

    i = statfs( "/steam/dt", fs );
    i = statfs( "/steam/dt/oye", fs );
    perror("doit");
    
    printf( "bs=%d tb=%d ff=%d ba=%d\n"
	   , fs->f_bsize, fs->f_blocks, fs-> f_bfree, fs->f_bavail );

    printf( " %.1f percent of the disk is free\n"
	   , 100.*(float)fs->f_bavail/fs->f_blocks );
}
