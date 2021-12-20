#!/usr/bin/env python

#===========================================================================
#
# Create an lrose-solo3 source release
#
#===========================================================================

from __future__ import print_function
import os, os.path
import sys
import shutil
import subprocess
import glob
import time
import hashlib
from datetime import datetime
from datetime import date
from datetime import timedelta
from optparse import OptionParser

def main():

    # globals

    global thisScriptName
    thisScriptName = os.path.basename(__file__)

    global options
    global tmpDir
    global baseDir
    global versionStr
    global releaseName
    global tarName
    global tarDir
    global package
    
    # parse the command line

    usage = "usage: " + thisScriptName + " [options]"
    homeDir = os.environ['HOME']
    package = 'lrose-solo3'
    defaultReleaseDir = os.path.join(homeDir, 'releases')
    defaultReleaseDir = os.path.join(defaultReleaseDir, package)
    parser = OptionParser(usage)
    parser.add_option('--debug',
                      dest='debug', default=True,
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--verbose',
                      dest='verbose', default=False,
                      action="store_true",
                      help='Set verbose debugging on')
    parser.add_option('--releaseDir',
                      dest='releaseDir', default=defaultReleaseDir,
                      help='Release directory')
    parser.add_option('--force',
                      dest='force', default=False,
                      action="store_true",
                      help='force, do not request user to check it is OK to proceed')

    (options, args) = parser.parse_args()
    
    if (options.verbose):
        options.debug = True

    # runtime

    now = time.gmtime()
    nowTime = datetime(now.tm_year, now.tm_mon, now.tm_mday,
                       now.tm_hour, now.tm_min, now.tm_sec)
    versionStr = nowTime.strftime("%Y%m%d")

    # set directories

    tmpDir = os.path.join(options.releaseDir, "tmp")
    baseDir = os.path.join(tmpDir, "lrose-solo3")

    # compute release name and dir name

    releaseName = package + "-" + versionStr + ".src"
    tarName = releaseName + ".tgz"
    tarDir = os.path.join(baseDir, releaseName)

    if (options.debug):
        print("Running %s:" % thisScriptName, file=sys.stderr)
        print("  package: ", package, file=sys.stderr)
        print("  releaseDir: ", options.releaseDir, file=sys.stderr)
        print("  tmpDir: ", tmpDir, file=sys.stderr)
        print("  force: ", options.force, file=sys.stderr)
        print("  versionStr: ", versionStr, file=sys.stderr)
        print("  releaseName: ", releaseName, file=sys.stderr)
        print("  tarName: ", tarName, file=sys.stderr)

    # save previous releases

    savePrevReleases()

    # create tmp dir

    createTmpDir()

    # checkout solo3 into the tmp dir

    os.chdir(tmpDir)
    shellCmd("git clone https://github.com/NCAR/lrose-solo3")

    # go to the base directory

    os.chdir(baseDir)

    # run autoconf

    #shellCmd("./runAutoConf.py")

    # update the datestamp header file

    updateDateStampHeader(now.tm_year, now.tm_mon, now.tm_mday);
    
    # create the release information file
    
    createReleaseInfoFile()

    # create the tar file

    createTarFile()

    # create the brew formula for OSX builds

    createBrewFormula()

    # move the tar file into release dir

    finalTarPath = os.path.join(options.releaseDir, tarName)
    os.chdir(options.releaseDir)
    os.rename(os.path.join(baseDir, tarName), finalTarPath)
    print("=====>> release dir: ", options.releaseDir, file=sys.stderr)
    print("==>> final tar path: ", finalTarPath, file=sys.stderr)
    
    # delete the tmp dir

    shutil.rmtree(tmpDir)

    sys.exit(0)

########################################################################
# move previous releases

def savePrevReleases():

    if (os.path.isdir(options.releaseDir) == False):
        return
    
    os.chdir(options.releaseDir)
    prevDirPath = os.path.join(options.releaseDir, 'previous_releases')

    # remove if file instead of dir

    if (os.path.isfile(prevDirPath)):
        os.remove(prevDirPath)

    # ensure dir exists
    
    if (os.path.isdir(prevDirPath) == False):
        os.makedirs(prevDirPath)

    # get old releases

    pattern = package + "-????????*.tgz"
    oldReleases = glob.glob(pattern)

    for name in oldReleases:
        newName = os.path.join(prevDirPath, name)
        if (options.debug):
            print("saving oldRelease: ", name, file=sys.stderr)
            print("to: ", newName, file=sys.stderr)
        os.rename(name, newName)

########################################################################
# create the tmp dir

def createTmpDir():

    # check if exists already

    if (os.path.isdir(tmpDir)):

        if (options.force == False):
            print(("WARNING: you are about to remove all contents in dir: " + tmpDir))
            print("===============================================")
            contents = os.listdir(tmpDir)
            for filename in contents:
                print(("  " + filename))
            print("===============================================")
            answer = "n"
            if (sys.version_info > (3, 0)):
                answer = input("WARNING: do you wish to proceed (y/n)? ")
            else:
                answer = raw_input("WARNING: do you wish to proceed (y/n)? ")
            if (answer != "y"):
                print("  aborting ....")
                sys.exit(1)
                
        # remove it

        shutil.rmtree(tmpDir)

    # make it clean

    os.makedirs(tmpDir)

########################################################################
# write release information file

def createReleaseInfoFile():

    # go to base dir
    os.chdir(baseDir)

    # open info file

    releaseInfoPath = os.path.join(baseDir, "ReleaseInfo.txt")
    info = open(releaseInfoPath, 'w')

    # write release info

    info.write("package:" + package + "\n")
    info.write("version:" + versionStr + "\n")
    info.write("release:" + releaseName + "\n")

    # close

    info.close()

########################################################################
# create the tar file

def createTarFile():

    # go to base dir, make tar dir

    os.chdir(baseDir)
    os.makedirs(tarDir)

    # copy the contents into the tar directory

    shellCmd("rsync -av * " + tarDir)

    # create the tar file

    shellCmd("tar cvfzh " + tarName + " " + releaseName)
    
########################################################################
# update the date stamp header file

def updateDateStampHeader(year, month, day):

    versionStr = '{:04d}'.format(year) + '/' + '{:02d}'.format(month) + '/' + '{:02d}'.format(day)
    header = open('./include/date_stamp.h', 'w')
    header.write('static const char *sii_date_stamp = \"Version ' + versionStr + '\";\n')
    header.close()

#################################################
# Template for homebrew formula file

formulaBody = """

require 'formula'

# Documentation: https://github.com/mxcl/homebrew/wiki/Formula-Cookbook

class LroseSolo3 < Formula

  homepage 'https://github.com/NCAR/lrose-solo3'

  url '{0}'
  version '{1}'
  sha256 '{2}'

  depends_on 'pkg-config'
  depends_on 'gtk+'
  depends_on 'gtkmm'
  depends_on 'libx11'
  depends_on 'libxext'

  def install

    # Build/install solo3
    ENV['LROSE_INSTALL_DIR'] = prefix
    system "cmake", "-DCMAKE_INSTALL_PREFIX=#{{prefix}}", "."
    system "make install"

#   system "./configure", "--disable-dependency-tracking", "--prefix=#{{prefix}}"

  end

  def test
    # Run the test with `brew test solo3`.
     system "#{{bin}}/solo3", "-h"
  end

end
"""

####################################################################
# build a Homebrew forumula file for solo3
#

def buildSolo3Formula(tar_url, tar_name, formula_name):

    print("=======>> tar_name: ", tar_name, file=sys.stderr)
    print("=======>> tar_url: ", tar_url, file=sys.stderr)
    print("=======>> formula_name: ", formula_name, file=sys.stderr)

    """ build a Homebrew forumula file for lrose-solo3 """	
    dash = tar_name.find('-')
    period = tar_name.find('.', dash)
    version = tar_name[dash+1:period]
    result = subprocess.check_output(("sha256sum", tar_name))
    checksum = result.split()[0].decode('ascii')
    formula = formulaBody.format(tar_url, version, checksum)
    outf = open(formula_name, 'w')
    outf.write(formula)
    outf.close()

########################################################################
# create the brew formula for OSX builds

def createBrewFormula():

    # go to base dir

    os.chdir(baseDir)

    tarUrl = "https://github.com/NCAR/lrose-solo3/releases/download/" + \
             package + "-" + versionStr + "/" + tarName
    formulaName = package + ".rb"

    # create the brew formula file

    buildSolo3Formula(tarUrl, tarName, formulaName)

    # move it up into the release dir

    os.rename(os.path.join(baseDir, formulaName),
              os.path.join(options.releaseDir, formulaName))

########################################################################
# Run a command in a shell, wait for it to complete

def shellCmd(cmd):

    print("Running cmd:", cmd, file=sys.stderr)
    
    try:
        retcode = subprocess.check_call(cmd, shell=True)
        if retcode != 0:
            print("Child exited with code: ", retcode, file=sys.stderr)
            sys.exit(1)
        else:
            if (options.verbose):
                print("Child returned code: ", retcode, file=sys.stderr)
    except OSError as e:
        print("Execution failed:", e, file=sys.stderr)
        sys.exit(1)

    print("    done", file=sys.stderr)
    
########################################################################
# Run - entry point

if __name__ == "__main__":
   main()
