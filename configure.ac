# $Id$
#

dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.50])
AC_INIT([solo3], [2.0.0], [http://www.ral.ucar.edu/projects/titan/docs])

AC_CONFIG_MACRO_DIR([m4])

# Explicit archiver needed for later automake
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

# Create the VERSION file, which contains the package version from
# AC_INIT.
echo -n AC_PACKAGE_VERSION>VERSION
AC_SUBST(PACKAGE_VERSION)
AC_MSG_NOTICE([netCDF AC_PACKAGE_VERSION])

# Keep libtool macros in an m4 directory.
AC_CONFIG_MACRO_DIR([m4])

# Find out about the host we're building on.
AC_CANONICAL_HOST

# We will output a config.h for the C library, and a fortran inc file,
# nfconfig.inc.
AC_CONFIG_HEADERS([config.h])

# initialize automake
AM_INIT_AUTOMAKE([-Wall foreign subdir-objects])

# Without this line, automake 1.13 quits with a complaint about
# "using a non-POSIX archiver requires 'AM_PROG_AR'"...
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

# Check for the existance of this file before proceeding.
AC_CONFIG_SRCDIR([perusal/soloii.hh])

AC_MSG_NOTICE([checking user options])

dnl When we're making binary releases, it's often nice to just statically link
dnl the final executables so we don't worry about what's installed, or which
dnl version it is.
#AC_ARG_ENABLE(allstatic,
#  AC_HELP_STRING([--enable-allstatic],[Enable using static libraries when possible for dependencies]),
#[case "${enableval}" in
#  yes) allstatic=yes ;;
#  no)  allstatic=no ;;
#  *)   AC_MSG_ERROR([bad value ${enableval} for --enable-allstatic option]) ;;
#esac],allstatic=no)
#AM_CONDITIONAL(ALLSTATIC, test x$allstatic = xyes)

# Does the user want a 32-bit build?
#AC_ARG_ENABLE([build-32bit], [AS_HELP_STRING([--build-32bit],
#              [build in 32-bit mode])])

# Did the user specify a location for the HDF5 library?
AC_MSG_CHECKING([whether a location for the HDF5 library was specified])
AC_ARG_WITH([hdf5],
              [AS_HELP_STRING([--with-hdf5=<directory>],
                              [Specify location of HDF5 library. Configure will \
                              expect to find subdirs include and lib.])],
            [HDF5DIR=$with_hdf5])
AC_MSG_RESULT([$HDF5DIR])
AC_SUBST(HDF5DIR, [$HDF5DIR])

# Did the user specify a location for the NETCDF library?
AC_MSG_CHECKING([whether a location for the NETCDF library was specified])
AC_ARG_WITH([netcdf],
              [AS_HELP_STRING([--with-netcdf=<directory>],
                              [Specify location of NETCDF library. Configure will \
                              expect to find subdirs include and lib.])],
            [NETCDFDIR=$with_netcdf])
AC_MSG_RESULT([$NETCDFDIR])
AC_SUBST(NETCDFDIR, [$NETCDFDIR])

# Add the HDF5 directory to includes and link flags
if test ! "x$HDF5DIR" = x; then
   LDFLAGS="${LDFLAGS} -L${HDF5DIR}/lib"
   CPPFLAGS="${CPPFLAGS} -I${HDF5DIR}/include"
fi    

# Add the NETCDF directory to includes and link flags
if test ! "x$NETCDFDIR" = x; then
   LDFLAGS="${LDFLAGS} -L${NETCDFDIR}/lib"
   CPPFLAGS="${CPPFLAGS} -I${NETCDFDIR}/include"
fi    

#
# Use our parent directory rather than /usr/local as the default prefix
#
ac_default_prefix=`pwd`/..

dnl Check for sys/mtio.h
AC_CHECK_HEADERS([sys/time.h])
AC_CHECK_HEADERS(sys/mtio.h)

dnl Checks for programs.
#c-----------------------------------------------------
AC_PROG_CC
AC_PROG_CXX
AC_PROG_F77
AC_PROG_RANLIB
#c-----------------------------------------------------
AC_PROG_CPP
#c-----------------------------------------------------
AC_PROG_INSTALL
#c-----------------------------------------------------
AC_PROG_LN_S
#c-----------------------------------------------------
AC_PROG_MAKE_SET
AC_CHECK_LIB(m, cos)

dnl We need to enable C++11 compilation, required for libsigc++ >= 2.5.1
AX_CXX_COMPILE_STDCXX_11

dnl Checks for library functions.
AC_PROG_GCC_TRADITIONAL

AC_C_BIGENDIAN
LOCAL_DEFS="-DUNIX"
if test $ac_cv_c_bigendian != yes; then
   LOCAL_DEFS="$LOCAL_DEFS -DLITTLENDIAN"
fi
#c---- AC_PATH_XTRA -----------------------------------
AC_PATH_XTRA
#c-----------------------------------------------------

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

# Checks for library functions.
AC_HEADER_STDC
AC_FUNC_STAT
AC_FUNC_UTIME_NULL
AC_CHECK_FUNCS([floor gettimeofday memset modf pow strerror])

# get GTK includes for compiling, and lib dependencies for linking

GTKP_vv="2.0"
PKG_CHECK_MODULES([GTKP], [gtk+-${GTKP_vv}], [HAVE_GTKP="yes"], [HAVE_GTKP="no"])
if test "$HAVE_GTKP" = "no" ; then
AC_MSG_ERROR([
*** ERROR - Cannot find up to date gtk+ package ***
*** ERROR - you need to install gtk+ version $GTKP_vv ***
])
fi

GTKMM_vv="2.4"
PKG_CHECK_MODULES([GTKMM], [gtkmm-${GTKMM_vv}], [HAVE_GTKMM="yes"], [HAVE_GTKMM="no"])
if test "$HAVE_GTKMM" = "no" ; then
AC_MSG_ERROR([
*** ERROR - Cannot find up to date gtkmm package ***
*** ERROR - you need to install gtkmm version $GTKMM_vv ***
])
fi

AC_ARG_ENABLE(centos,
  AS_HELP_STRING([--enable-centos],
                 [Enable special compile for CENTOS because of gtk issues]),
  [case "${enableval}" in
     yes) centos=yes ;;
     no)  centos=no ;;
     *)   AC_MSG_ERROR([bad value ${enableval} for --enable-centos option]) ;;
  esac],
  centos=no)
AM_CONDITIONAL(OS_IS_CENTOS, test x$centos = xyes)

AC_CONFIG_FILES([
        editor/makefile
        perusal/makefile
        translate/makefile
        makefile])

AC_OUTPUT

