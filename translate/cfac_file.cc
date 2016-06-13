# define PMODE 0666

#include <cfac_file.hh>
#include <string.h>
#include <Correction.h>
#include <fcntl.h>

int 
main (void)
{
    struct correction_d cfac;
    int n, fid;
    /* We assume the numbers are to be added to effect the correction
     * whereas Testud expects the numbers to be subtracted
     * The numbers in parentheses are Testud's numbers.
     */
    strncpy(cfac.correction_des, "CFAC", 4);
    cfac.correction_des_length = sizeof(struct correction_d);

    fid = creat("/home/steam/oye/radar/dorade/cfac/TA-ELDR_feb10", PMODE);
    /* AFT corrections! */
    cfac.azimuth_corr          = 0;
    cfac.elevation_corr        = 0;
    cfac.range_delay_corr      = -450. +(0);
    cfac.longitude_corr        = 0;
    cfac.latitude_corr         = 0;
    cfac.pressure_alt_corr     = -(-240*.001); /* in km. */
    cfac.radar_alt_corr        = 0;
    cfac.ew_gndspd_corr        = -(0.2);
    cfac.ns_gndspd_corr        = 0;
    cfac.vert_vel_corr         = 0;
    cfac.heading_corr          = 0;
    cfac.roll_corr             = 0;
    cfac.pitch_corr            = -(1.5);
    cfac.drift_corr            = -(1.0);
    cfac.rot_angle_corr        = -(-2.3);
    cfac.tilt_corr             = 0;

    n = write(fid, (char *)&cfac, sizeof(struct correction_d));
    close(fid);

    fid = creat("/home/steam/oye/radar/dorade/cfac/TF-ELDR_feb10", PMODE);
    /* FORE corrections! */
    cfac.azimuth_corr          = 0;
    cfac.elevation_corr        = 0;
    cfac.range_delay_corr      = -450. +(0);
    cfac.longitude_corr        = 0;
    cfac.latitude_corr         = 0;
    cfac.pressure_alt_corr     = -(-240*.001); /* in km. */
    cfac.radar_alt_corr        = 0;
    cfac.ew_gndspd_corr        = -(0.2);
    cfac.ns_gndspd_corr        = 0;
    cfac.vert_vel_corr         = 0;
    cfac.heading_corr          = 0;
    cfac.roll_corr             = 0;
    cfac.pitch_corr            = -(1.5);
    cfac.drift_corr            = -(1.0);
    cfac.rot_angle_corr        = -(-2.3);
    cfac.tilt_corr             = 0;

    n = write(fid, (char *)&cfac, sizeof(struct correction_d));
    close(fid);
}
