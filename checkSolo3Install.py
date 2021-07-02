#!/usr/bin/env python

#===========================================================================
#
# Check that all solo3 apps are installed
# Checks against a list of expected apps.
#
#===========================================================================

from __future__ import print_function
import os
import time
import sys
import shutil
from optparse import OptionParser
from datetime import datetime

def main():

    global options
    global missingApps
    global oldApps

    global thisScriptName
    thisScriptName = os.path.basename(__file__)

    global thisScriptDir
    thisScriptDir = os.path.dirname(__file__)
 
    global requiredApps
    requiredApps = [ 'solo3', 'ddex3', 'nx_reblock3', 'xltrs3' ]

    # parse the command line

    usage = "usage: %prog [options]"
    homeDir = os.environ['HOME']
    prefixDirDefault = os.path.join(homeDir, 'lrose')
    parser = OptionParser(usage)
    parser.add_option('--debug',
                      dest='debug', default=False,
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--prefix',
                      dest='prefix', default=prefixDirDefault,
                      help='Install directory, default is ~/lrose')
    parser.add_option('--maxAge',
                      dest='maxAge', default=-1,
                      help='Max file age in secs. Check not done if negative (default).')

    (options, args) = parser.parse_args()
    
    # compute dirs and paths

    global buildDir, checklistPath

    buildDir = os.path.join(thisScriptDir, "..")
    os.chdir(buildDir)
    buildDir = os.getcwd()

    # print status

    if (options.debug == True):
        print("Running %s: " % thisScriptName, file=sys.stderr)
        print("  prefix: ", options.prefix, file=sys.stderr)
        print("  maxAge: ", options.maxAge, file=sys.stderr)
        print("  buildDir: ", buildDir, file=sys.stderr)

    # check required files exist

    checkForApps()

    if (len(missingApps) > 0):
        print("==================>> ERROR <<====================", file=sys.stderr)
        print("=====>> INCOMPLETE solo3 APPS INSTALLATION <<====", file=sys.stderr)
        print("  n applications missing: " + str(len(missingApps)), file=sys.stderr)
        for app in missingApps:
            print("    missing app: " + app, file=sys.stderr)
        print("=================================================", file=sys.stderr)
    else:
        print("================>> SUCCESS <<==================", file=sys.stderr)
        print("=========>> ALL solo3 APPS INSTALLED <<========", file=sys.stderr)
        print("===============================================", file=sys.stderr)

    if (len(oldApps) > 0):
        print("==================>> WARNING <<====================", file=sys.stderr)
        print("=====>> SOME solo3 APPS ARE OLD <<====", file=sys.stderr)
        print("  n old apps: " + str(len(oldApps)), file=sys.stderr)
        if (options.debug):
            for app in oldApps:
                print("    old app: " + app, file=sys.stderr)
        print("=================================================", file=sys.stderr)

    sys.exit(0)

########################################################################
# check that the app list is installed

def checkForApps():

    global missingApps
    global oldApps
    missingApps = []
    oldApps = []
    
    installBinDir = os.path.join(options.prefix, "bin")
    maxAge = float(options.maxAge)

    for name in requiredApps:

        path = os.path.join(installBinDir, name)

        if (options.debug == True):
            print("Checking for installed app: ", path, file=sys.stderr)

        if (os.path.isfile(path) == False):
            if (options.debug == True):
                print("   .... missing", file=sys.stderr)
            missingApps.append(path)
        else:
            if (options.debug == True):
                print("   .... found", file=sys.stderr)
            age = getFileAge(path)
            if ((maxAge > 0) and (age > maxAge)):
                oldApps.append(path)
                if (options.debug == True):
                    print("   file is old, age: ", age, file=sys.stderr)
                    print("   maxAge: ", maxAge, file=sys.stderr)


########################################################################
# get file age

def getFileAge(path):

    stats = os.stat(path)
    age = float(time.time() - stats.st_mtime)
    return age

########################################################################
# Run - entry point

if __name__ == "__main__":
   main()
