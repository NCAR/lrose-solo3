Build instructions using the solo3 brew Formula:
=========================================================
Install Homebrew from http://brew.sh/

$ brew tap ncareol/lrose-solo3
$ brew install solo3


Build instructions from source for solo3 using Homebrew:
=========================================================

Install the most recent XQuartz from http://xquartz.macosforge.org/landing/
brew install gtk+ gtkmm 
brew link --force gettext  # needed for -lintl
./configure PKG_CONFIG_PATH=/opt/X11/lib/pkgconfig 
make

For Mac OS X, HomeBrew, http://brew.sh/ is the supported environment for 
building solo3.  Install Homebrew from http://brew.sh/
To build and install solo3:
brew install ftp://ftp.eol.ucar.edu/pub/archive/rdpdist/solo3.rb

Formula maintainers instructions:
=========================================================

To create a new Homebrew formula for solo3:
1) Find the current solo3 source tar archive from the solo3 software distribution 
   page, https://www.eol.ucar.edu/software/solo3, e.g., 
   https://www.eol.ucar.edu/system/files/software/solo3/all-oss/solo3-20160613.src_.tgz

2) Run the formula creation script:

   $ ./build_solo3_formula \
     https://www.eol.ucar.edu/system/files/software/solo3/all-oss/solo3-20160613.src_.tgz \
     solo3.rb

4) Clone https://github.com/ncareol/homebrew-lrose-solo3.git to a local
   temporary repository, then copy the new solo3.rb there:

   $ git clone https://github.com/ncareol/homebrew-lrose-solo3.git \
     /tmp/homebrew-lrose-solo3
   $ cp solo3.rb /tmp/homebrew-lrose-solo3
   
5) Commit the new solo3.rb and push it up to GitHub, so that it's available
   for homebrew access:

   $ cd /tmp/homebrew-lrose-solo3
   $ git commit solo3.rb -m "Update formula for version 20160613"
   $ git push    # puts the new version at GitHub
   
6) Remove the temporary git repository:
   $ rm -rf /tmp/homebrew-lrose-solo3
