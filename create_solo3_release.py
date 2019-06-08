#!/usr/bin/env python

#===========================================================================
#
# Create an lrose-solo3 source release
#
#===========================================================================

from __future__ import print_function
import os
import sys
import shutil
import subprocess
import glob
import time
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
    global logPath
    global logFp
    global package
    
    # parse the command line

    usage = "usage: " + thisScriptName + " [options]"
    homeDir = os.environ['HOME']
    package = 'lrose-solo3'
    defaultReleaseDir = os.path.join(homeDir, 'releases')
    defaultReleaseDir = os.path.join(defaultReleaseDir, package)
    logDirDefault = '/tmp/create_solo3_release/logs'
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
    parser.add_option('--logDir',
                      dest='logDir', default=logDirDefault,
                      help='Logging dir')
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

    # initialize logging

    if (os.path.isdir(options.logDir) == False):
        os.makedirs(options.logDir)
    logPath = os.path.join(options.logDir, "master");
    logFp = open(logPath, "w+")

    if (options.debug):
        print("Running %s:" % thisScriptName, file=sys.stderr)
        print("  package: ", package, file=sys.stderr)
        print("  releaseDir: ", options.releaseDir, file=sys.stderr)
        print("  logDir: ", options.logDir, file=sys.stderr)
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

    logPath = prepareLogFile("git-checkout");
    os.chdir(tmpDir)
    shellCmd("git clone https://github.com/NCAR/lrose-solo3")

    # go to the base directory

    os.chdir(baseDir)

    # run autoconf

    #logPath = prepareLogFile("run-autoconf");
    #shellCmd("./runAutoConf.py")

    # set the datestamp file

    logPath = prepareLogFile("set_release_date");
    shellCmd("./set_release_date.pl")
    
    # create the release information file
    
    createReleaseInfoFile()

    # create the brew formula for OSX builds

    logPath = prepareLogFile("create-brew-formula");
    createBrewFormula()

    sys.exit(1)
    
    # create the tar file

    logPath = prepareLogFile("create-tar-file");
    createTarFile()

    # move the tar file into release dir

    os.chdir(options.releaseDir)
    os.rename(os.path.join(baseDir, tarName),
              os.path.join(options.releaseDir, tarName))
              
    # delete the tmp dir

    shutil.rmtree(tmpDir)

    logFp.close()
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
            print("saving oldRelease: ", name, file=logFp)
            print("to: ", newName, file=logFp)
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
# create the brew formula for OSX builds

def createBrewFormula():

    # go to base dir

    os.chdir(baseDir)

    tarUrl = "https://github.com/NCAR/lrose-solo3/releases/download/" + \
             package + "-" + versionStr + "/" + tarName
    formulaName = package + ".rb"
    scriptName = "build_solo3_formula"
    scriptPath = os.path.join(baseDir, scriptName)

    # check if script exists

    if (os.path.isfile(scriptPath) == False):
        print("WARNING - ", thisScriptName, file=logFp)
        print("  No script: ", scriptPath, file=logFp)
        print("  Will not build brew formula for package", file=logFp)
        return

    # create the brew formula file

    shellCmd(scriptPath + " " + tarUrl + " " +
             tarName + " " + formulaName)

    # move it up into the release dir

    os.rename(os.path.join(baseDir, formulaName),
              os.path.join(options.releaseDir, formulaName))

########################################################################
# prepare log file

def prepareLogFile(logFileName):

    global logFp

    logFp.close()
    logPath = os.path.join(options.logDir, logFileName + ".log");
    if (logPath.find('no-logging') >= 0):
        return logPath
    print("========================= " + logFileName + " =========================", file=sys.stderr)
    if (options.verbose):
        print("====>> Creating log file: " + logPath + " <<==", file=sys.stderr)
    logFp = open(logPath, "w+")
    logFp.write("===========================================\n")
    logFp.write("Log file from script: " + thisScriptName + "\n")

    return logPath

########################################################################
# Run a command in a shell, wait for it to complete

def shellCmd(cmd):

    print("Running cmd:", cmd, file=sys.stderr)
    
    if (logPath.find('no-logging') >= 0):
        cmdToRun = cmd
    else:
        print("Log file is:", logPath, file=sys.stderr)
        print("    ....", file=sys.stderr)
        cmdToRun = cmd + " 1>> " + logPath + " 2>&1"

    try:
        retcode = subprocess.check_call(cmdToRun, shell=True)
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
