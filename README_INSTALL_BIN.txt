UCAR/EOL/RAL solo3 software for radial data for radar/lidar
==========================================================

README for INSTALLING a BINARY distribution.

Web documentation
-----------------

For full details see:

  TODO - fill in here

LINUX setup
-----------

solo3 is primarily intended to run on LINUX and OSX.

Most good, up-to date LINUX distributions should work.

Recommended OSs are:

  Ubuntu
  Debian
  Fedora
  Scientific Linux
  Mac OSX

Required packages for compiling:

  tcsh shell
  perl shell
  python shell

Downloading
-----------

For 64-bit systems, the solo3 binary release will be named:

  solo3-yyyymmdd.x86_64.tgz

For 32-bit systems, the solo3 binary release will be named:

  solo3-yyyymmdd.i686.tgz

where yyyymmdd is the date of the distribution.

Download this file from:

  ftp.rap.ucar.edu/pub/titan/solo3

Previous releases can be found in:

  ftp.rap.ucar.edu/pub/titan/solo3/previous_releases

Download the file into a tmp area. For example:

  /tmp/solo3

and install from there.

However, any suitable directory can be used for this purpose.

Un-tarring the file
-------------------

Use the command

  tar xvfz solo3-yyyymmdd.x86_64.tgz (64-bit systems)

or

  tar xvfz solo3-yyyymmdd.i686.tgz (32-bit systems)

to un-tar the distribution into the current directory.

yyyymmdd should be substituded with the actual date on the file.

Then, cd into the directory:

  cd solo3-yyyymmdd.x86_64

or

  cd solo3-yyyymmdd.i686

Installing
----------

You will probably need to be root for this step, unless you install
in your user area.

By default, the binaries will be installed in /usr/local/bin. You can specify an
alternative.

  install_bin_release 

will install in /usr/local/bin.

  install_bin_release /opt/local

will install in /opt/local/bin.

The support dynamic libraries will be found in:

  ..../bin/solo3_runtime_libs

i.e. in a subdirectory of the bin directory.

