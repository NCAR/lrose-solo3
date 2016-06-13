#!/usr/bin/perl

($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)
    = localtime (time);
#print "$sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst\n";

$tstamp = sprintf 'static const char *sii_date_stamp = "Version %04d/%02d/%02d-%02d:%02d:%02d";'
    , ($year+1900,$mon+1,$mday,$hour,$min,$sec);

print "$tstamp\n";

$ii = `echo '$tstamp' > ./include/date_stamp.h`;
