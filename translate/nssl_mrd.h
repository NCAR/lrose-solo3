/* 	$Id$	 */
# ifndef NSSL_MRD_H
# define NSSL_MRD_H

# include <input_limits.hh>
# include <dd_stats.h>
# include <time.h>
# include <sys/time.h>

# define  FIND_VEL 1
# define FIND_REFL 2
# define USE_PALT 0x1

# define MRD_SWEEP_MARK -1
# define MAX_MRD_GATES 2048
# define ALTITUDE_RA 0
# define ALTITUDE_PA 1

# define MASK10 0x3ff
# define  MASK6 0x3f

# define NAV_INE1 1
# define NAV_INE2 2
# define  NAV_GPS 3

# ifndef PMODE
# define PMODE 0666
# endif

# ifndef S10
# define S10(x) ((x)*10.+.5)
# endif

# ifndef S1000
# define S1000(x) ((x)*1000.+.5)
# endif

# ifndef S64
# define S64(x) ((x)*64.+.5)
# endif

struct mrd_head {
    short word_1;
    short word_2;
    short word_3;
    short raw_rot_ang_x10;	/* deg. */
    short lat_deg;
    short lat_min;
    short lat_sec_x10;
    short lon_deg;
    short lon_min;
    short lon_sec_x10;
    short altitude;		/* meters */
    short roll_x10;		/* deg. */
    short heading_x10;		/* deg. */
    short drift_x10;		/* deg. */
    short pitch_x10;		/* deg. */
    short raw_tilt_x10;
    short nyq_vel_x10;		/* m/s */
    short julian_date;
    short azimuth_samples;
    short gate_length;		/* m */
    short range_delay;		/* m */
    short ground_speed_x64;	/* m/s */
    short vert_airspeed_x64;	/* m/s */
    char flight_number[8];
    char storm_name[12];
    short wind_dir_x10;		/* deg. */
    short nav_flag;
    short wind_speed_x10;	/* m/s */
    short noise_threshold;
    short corrected_tilt_x10;	/* deg. */
    short num_good_gates;
    short gspd_vel_corr_x10;	/* m/s */
    short sweep_num;
    short max_gates;
    short tilt_corr_flag;
    short altitude_flag;
    char aircraft_id[2];
    short place_holder[80];	/* second header has to be copied in here */
				/* to avoid alignment problems */
};

struct mrd_head2 {
    int32_t year;
    int32_t month;
    int32_t day;
    int32_t hour;
    int32_t minute;
    int32_t second;
    int32_t word_7;
    int32_t word_8;
    int32_t word_9;
    int32_t word_10;
    int32_t word_11;
    int32_t word_12;
    int32_t word_13;
    int32_t word_14;
    int32_t word_15;
    int32_t word_16;
    int32_t word_17;
    int32_t word_18;
    int32_t word_19;
    int32_t word_20;
    int32_t word_21;
    int32_t word_22;
    int32_t word_23;
    int32_t word_24;
    int32_t word_25;
    int32_t word_26;
    int32_t word_27;
    int32_t word_28;
    int32_t word_29;
    int32_t word_30;
    int32_t word_31;
    int32_t word_32;
    int32_t word_33;
    int32_t word_34;
    int32_t word_35;
    int32_t word_36;
    int32_t word_37;
    int32_t word_38;
    int32_t word_39;
    int32_t word_40;
};

struct mrd_record {
    char data[8192];
    struct mrd_head *mrdh;
    struct mrd_head2 *mrdh2;
    struct mrd_record *last;
    struct mrd_record *next;
};

struct mrd_production {
    float r1;
    float gs;
    int fid;
    int file_size;
    int prev_vol_num;
    int prev_sweep_num;
    int rec_count;
    int vol_count;
    int sweep_num;
    int max_gates;
    int vpn;
    int rpn;
    int dd_gate_lut[MAX_MRD_GATES];
    struct mrd_record *rec_que;
    char file_name[88];
    char radar_name[12];
};

#endif

/*
      Descriptor for NSSL/MRD disc file format airborne Doppler Data

  
c  One sweep consists of up to 400 radials of data with each radial
c  containing up to 1024 gates.
c  Radial Velocity and Reflectivity data are stored in a "compressed
c  sequential" format such that only the first N_Gates of data are stored.
C  Hence the disc records are variable length
c  End of sweeps (usually when the antenna passes through
c  the vertical) are denoted by sweep marks (Ihead(41) = -1)
  
c  Each beam consists of two physical records.
c  The first record consists of a short header (45 integer*2 words) followed
c  by a second header (40 integer*4 words).
c  The data record then follows which consists of N_Gates of data.  The data
c  is thresholded prior to writing the file by marching inward from the 
c  end of the beam and finding the first good gate, calling that point 
c  N_Gates.  Data is thresholded on board the aircraft by either returned
c  power (dBm) or by spectral width.  This
c  methodology seems to work well for an airborne radar since the beam usually
c  spends most of its time pointing into outer space or below the surface.
  
c  The format of the disc header record is as follows:
C   Ihead(1) = unused
C   Ihead(2) = unused
C   Ihead(3) = unused
C   Ihead(4) = Raw Rotation Angle [deg*10] from straight up
C              i.e., not roll corrected, relative to the fuselage)
C   Ihead(5) = Latitude [deg]
C   Ihead(6) = Latitude [min]
C   Ihead(7) = Latitude [sec*10.0]
C   Ihead(8) = Longitude [deg]
C   Ihead(9) = Longitude [min]
C   Ihead(10) = Longitude [sec*10.0]
C   Ihead(11) = Altitude [m] defined at load time as AGL (i.e., radar
C               altitude) or MSL (i.e., pressure altitude appropriate
C               for work over highly variable terrain)
C   Ihead(12) = Roll [deg*10.0]
C   Ihead(13) = Head [deg*10.0]
C   Ihead(14) = Drift [deg*10.0]
C   Ihead(15) = Pitch [deg*10.0]
C   Ihead(16) = Raw "Tilt" angle [deg*10] fore/aft pointing angle relative
C               to a plane normal to the longitudinal axis of the aircraft
C   Ihead(17) = Nyquist velocity [m/s*10]
C   Ihead(18) = Julian date
C   Ihead(19) = # of azimuth samples
C   Ihead(20) = Gate length [m]
C   Ihead(21) = Range delay [m]
C   Ihead(22) = Ground speed [m/s*64.0]
C   Ihead(23) = Vertical airspeed [m/s*64.0]
C   Ihead(24-27) = Flight number (8 ASCII characters)
C   Ihead(28-33) = Storm name (12 ASCII characters)
C   Ihead(34) = Wind dir [deg*10]
C   Ihead(35) = flag for navigation system used to derive position and winds
C               1=INE1; 2=INE2; 3=GPS
C   Ihead(36) = Wind speed [m/s*10]
C   Ihead(37) = Threshold [dBm] used to identify "noise" -999=spectral width
C   Ihead(38) = "corrected" tilt angle for pitch, roll, and drift
C   Ihead(39) = # of good gates 
C   Ihead(40) = radial velocity correction for the ray due to ground speed
c   Ihead(41) = Sweep Number
c   Ihead(42) = Maximum number of gates
c   Ihead(43) = Tilt correction flag (0=no; 1=yes) if data has been changed
c   Ihead(44) = flag for altitude 0 for RA, 1 for PA
c   Ihead(45) = Not Used

c   The second header currently uses only the first 6 words:
c     Iheadr(1) = year (00 - 99)
c     Iheadr(2) = month (00 - 12)
c     Iheadr(3) = day (00 - 31)
c     Iheadr(4) = hour (00 - 23)
c     Iheadr(5) = minute (00 - 59)
c     Iheadr(6) = second (00 - 59)
c
c   The velocity and reflectivity data are scaled for each gate as
c   a single 16 bit integer (to save disc space) as follows:
c
c                                 bit #
c                             1111110000000000
c                             5432109876543210
c                             zzzzzzvvvvvvvvvv
c    where:
c          z: scaled reflectivity [6 bits] 0 - 63 dBZ
c          v: velocity data [10 bits] +-68 m/s 0.15 m/s resolution
c       If signal <noise then word is =-1

  
      Character*8 Flid
      Character*16 Project

      Dimension Hed(20), Vel(1024), Ref(1024), Itime(6)
  
      Integer*2 Ihead(45), Ibufr(1024), Ival
      Integer*4 Iheadr(40)

c  First read the headers
  
    1 Read (75,End=1000,Err=999,Iostat=Ierr) (Ihead(i),i=1,45),
     #     (Iheadr(i),i=1,40)

      If (Ihead(41) .eq. -1) Go To 1    !   psuedo end of sweep mark
  
      N_Gates = Ihead(39)
      Max_Gates = Ihead(42)

      If (N_Gates .gt. 1024 .or. Max_Gates .gt. 1024 .or. N_Gates
     #    .gt. Max_Gates) Then
         Write (60,'("Probem with n_gates or max_gates",2i14)')
     #           N_Gates, Max_Gates
         Stop
      End If

      Itime(1) = Iheadr(1)   ! year without 1900
      Itime(2) = Iheadr(2)   ! month
      Itime(3) = Iheadr(3)   ! day
      Itime(4) = Iheadr(4)   ! hour
      Itime(5) = Iheadr(5)   ! minute
      Itime(6) = Iheadr(6)   ! second
  
c  Next read the data record which is of variable length and could be 
c  as large as 1024 16 bit integer words

c  Note that only the first N_Gates out of Max_Gates are actually stored,
c  since thresholding is performed prior to writing the file.
 
      Read (75,End=1000,Err=999,Iostat=Ierr) (Ibufr(i),i=1,N_Gates)
  
c  Load the header information needed by the calling routine

c  Latitude

      Hed(2) = Float(Ihead(5)) + Float(Ihead(6))/60.0 +
     #         Float(Ihead(7))/36000.0

c  Longitude

      Hed(3) = Float(Ihead(8)) + Float(Ihead(9))/60.0 +
     #         Float(Ihead(10))/36000.0

c  Gate Length

      Hed(4) = Float(Ihead(20))/1000.0

c  Number of good gates in the ray

      Hed(5) = Float(Ihead(39))

c  Sweep Number
      
      Hed(7) = Float(Ihead(41))

c  Altitude (m)

      Hed(10) = Float(Ihead(11))

c  Range delay (km) "RDEL"

      Hed(11) = Float(Ihead(21))/1000.0

c  Which altitude flag

      Hed(12) = Float(Ihead(44))  ! =0 for RA, = 1 for PA

c  Project and flight identifiers

      Write (Flid,'(4a2)') (Ihead(kk),kk=24,27)
      Write (Project,'(6a2,4H- - )') (Ihead(kk),kk=28,33)

c  Nyquist interval (m/s)
  
      Xniq = Float(Ihead(17)) / 10.0
  
c  Load up the data
  
      Do i = 1, Max_Gates
         If (i .le. N_Gates) Then
            Ival = Ibufr(i)
         Else
            Ival = -1
         End If

         If (Ival .eq. -1) Then
            Vel(i) = -999.0
            Ref(i) = -999.0
         Else
            Ivel = Ibits(Ival,0,10)
            Vel(i) = Float(Ivel - 511) / 7.5
            Ref(i) = Ibits (Ival,10,6)
         End If
      End Do
  
c  Correct the ray for bad antenna behavior and compute angles
c  relative to the track
  
      Call Correct_Ray (Vel, Ref, Ihead, Tilt, Rot, CompAz, Azm_RC)

c  Azimuth roll corrected

      Hed(6) = Azm_RC

c  Rotation angle from zenith

      Hed(8) = Rot

c  Track relative tilt angle (fore/aft direction)

      Hed(9) = Tilt

c  Compass azimuth from North

      Hed(1) = CompAz

      Return
  
  999 Write (60,'("Error:",i5)') Ierr
      Return
  
 1000 Write (60,'(/" End of file on input disc file",i5)') Ierr
      Return
 
      End

--

Dave Jorgensen
NOAA/NSSL/Mesoscale Research Division
davej@mrd3.mmm.ucar.edu or davej@ncar.ucar.edu

Phone: (303) 497-6246  Fax: (303) 497-6930
Mailing Address: Mail Code: N/C/MRD, 325 Broadway, Boulder, CO  80303
Express Mail Address: 3450 Michell Lane, Bldg. 3, Room 2035, Boulder, CO  80301



 */

