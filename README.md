# lrose-solo3

This is version 3 of the NCAR solo polar radar viewer and editor.

This is effectively the the soloii application, converted to C++.
It depends on updated version of the gtk libraries.

1. [prepare](#prepare)
2. [download](#download)
3. [build](#build)
4. [install](#install)
5. [install-for-the-mac](./homebrew_install.mac_osx.md)

## DOI

The DOI for lrose-solo3 is:

* [https://doi.org/10.5065/sv4e-7z49](https://doi.org/10.5065/sv4e-7z49)

The DOI entry information is stored at:

* [https://search.datacite.org/works/10.5065/sv4e-7z49](https://search.datacite.org/works/10.5065/sv4e-7z49)

<a name="prepare"/>

## 1. Prepare

Most good, up-to date LINUX distributions should work.

Recommended distributions are:

  * Debian
  * Ubuntu (based on Debian)
  * RedHat
  * Centos (based on RedHat)
  * Fedora (based on RedHat)

First, you will need to install the required packages.

### RedHat

On a RedHat-based system, run the following:

```
yum install -y epel-release && \
yum install -y tcsh wget git \
    tkcvs emacs rsync python \
    m4 make libtool autoconf automake \
    gcc gcc-c++ gcc-gfortran glibc-devel \
    libX11-devel libXext-devel \
    libpng-devel libtiff-devel zlib-devel \
    expat-devel libcurl-devel \
    flex-devel fftw3-devel \
    gtk+-devel gtkmm24-devel glib-devel glibmm24-devel  \
    bzip2-devel \
    hdf5-devel netcdf-devel \
    xorg-x11-xauth xorg-x11-apps \
    rpm-build redhat-rpm-config \
    rpm-devel rpmdevtools \
    atk-2.28.1-2.el7.x86_64 \
    libXrandr-devel-1.5.1-2.el7.x86_64
```

### Debian / Ubuntu

On a Debian-based system, run the following:

```
apt-get update && \
    apt-get install -y \
    git gcc g++ gfortran cmake rsync mlocate \
    automake make libtool pkg-config python \
    libcurl3-dev curl \
    libfl-dev libbz2-dev libx11-dev libpng-dev \
    libgtk2.0-dev gtkmm-2.4-dev \
    glib-2.0-dev glibmm-2.4-dev \
    libfftw3-dev libexpat1-dev \
    libgeographic-dev libeigen3-dev libzip-dev \
    libnetcdf-dev netcdf-bin libhdf5-dev hdf5-tools
```

On Ubuntu 22.04:

```
apt update && \
apt install -y \
git gcc g++ gfortran cmake rsync mlocate \
automake make libtool pkg-config python3.11 \
libcurl3-dev curl libfl-dev libbz2-dev libx11-dev libpng-dev \
libgtk2.0-dev libgtkmm-2.4-dev \
glibc-source glibc-tools libglibmm-2.4-dev \
libfftw3-dev libexpat1-dev \
libgeographic-dev libeigen3-dev libzip-dev \
libnetcdf-dev netcdf-bin libhdf5-dev hdf5-tools build-essential
```

<a name="download"/>

## 2. Downloading

### Create a working directory for building the distribution:

```
  mkdir ~/solo3_build
  cd ~/solo3_build
```

### Download the source release

```
  git clone https://github.com/NCAR/lrose-solo3 
```

<a name="build"/>

## 3. Build

Run configure to create the makefiles:

```
  cd ~/solo3_build/lrose-solo3
  autoreconf --install
  ./configure --prefix=/usr/local
```

The do the build:

```
  make -j 4
```

<a name="install"/>

## 4. Install

```
  make install
```

The following apps will be installed in /usr/local/bin:

```
  solo3 (previously soloii)
  xltrs3 (previously xltrsii)
  ddex3 (previously ddex)
  nx_reblock3 (previously nx_reblock)
```

## 5. For Mac Users - install using brew

See [Homebrew install](./homebrew_install.mac_osx.md)

<a name="changelog"/>

## 6. Change Log - including soloii versions

This includes the soloii versions, the predecessor of solo3.

### update on 2014/07/28

Increased line buffer size in dor_print_sswb() to keep things from crashing
if SSWB contains absurd d_start_time or d_stop_time, e.g., with date from 
year 200000000.

### update on 2014/07/11

* PIRAQX updates for better handling of Rapid-scan DOW data.
* Homebrew improvements for Mac.
* Fixed bugs in reading color tables from text files.
* Fixed a buffer overflow issue which causes crashes on Ubuntu 14.04 systems.
* Better handling when there are sweepfile header load failures.

### update on 2013/12/08

In persusal/sp_lists.cc removed unneeded include of <bits/localefwd.h>.

### initial version 3.00 (2012/05/14):

* upgraded to C++
* 64-bit native support
* GUI components modified to use gtkmm widgets
  
### version 1.20 (2010/12/6):

* fixed memory overruns which were causing problems when changing display
  configuration, e.g., when loading saved configuration files
  
### version 1.19: 

* actually tell people that we cannot display floating-point DORADE data,
    rather than just silently drawing a bunch of gray

* bump maximum gate count we handle from 1500 to 8192
* disable tape I/O on systems without /usr/include/sys/mtio.h (e.g.,
    Mac Snow Leopard)
* add "-m32" flag to compilation and linking for builds on OS X, to
    force making 32-bit executables
    
### version 1.18

Introduce first-cut support for Mac OS X.  Solo3 has been
tested briefly (on a Leopard x86 system), and will display and perform
basic edits on DORADE sweep files.

### version 1.17

For xltrsii, fixed a problem where the Nyquist velocity 
written into sweep files was retained from sweep to sweep, even when 
PRT changed between sweeps.

### version 1.16

Added directions for building on 64-bit systems.  Added xltrsii
support for PIRAQX rev 2 data (generated by newer ARC HiQ systems).  Added
documentation for PIRAQX options in translate/README.xltrsii.html.

### version 1.15

Cleaned code to make it work with newer compilers without 
generating warnings and made configure more robust.

### version 1.14

Added yum netcdf install paths to configure script. 

### version 1.13

Fixed some bugs for Dorade to netCDF conversion.

### version 1.12

For linux machines compiles with the -m32 flag to 
force 32 bit compiles on 64 bit machines.

### version 1.11

Changed the mode permissions for open/creat calls to 666.

### version 1.10

Updates for Fedora Core 4.

### version 1.09

Found bug in logging code.  String sizes were not large enough.

### version 1.08

Hooked up logging function for examine widget.

### version 1.07

Modified translators to work with new WSR88D archive II format.



 
