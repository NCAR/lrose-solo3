#!/usr/bin/env python

#===========================================================================
#
# Run autoconf etc on a dir
#
#===========================================================================

import os
import sys
import subprocess
import shutil
from optparse import OptionParser
from datetime import datetime

def main():

    global options
    global subdirList
    global makefileCreateList

    global thisScriptName
    thisScriptName = "runAutoConf.py"

    # parse the command line

    usage = "usage: %prog [options]"
    homeDir = os.environ['HOME']
    parser = OptionParser(usage)
    parser.add_option('--debug',
                      dest='debug', default='False',
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--verbose',
                      dest='verbose', default='False',
                      action="store_true",
                      help='Set verbose debugging on')
    parser.add_option('--dir',
                      dest='dir', default=".",
                      help='Directory containing configure script')

    (options, args) = parser.parse_args()

    if (options.verbose == True):
        options.debug = True
    
    if (options.debug == True):
        print >>sys.stderr, "Running %s:" % thisScriptName
        print >>sys.stderr, "  Dir:", options.dir

    # go to the dir

    os.chdir(options.dir)

    # clean up any link files from previous configs

    linkNames = [ 'ar-lib', 'config.guess', 'config.sub', 'depcomp', 'missing' ]
    for linkName in linkNames:
        if (os.path.lexists(linkName)):
            os.unlink(linkName)

    # run autoconffix the configure

    runAutoConf()
            
    # turn links into actual files

    for linkName in linkNames:
        tmpName = linkName + ".tmp"
        os.rename(linkName, tmpName)
        shutil.copyfile(tmpName, linkName)
        os.unlink(tmpName)

    sys.exit(0)

########################################################################
# run autoconf commands

def runAutoConf():

    cmd = "aclocal"
    runCommand(cmd)

    cmd = "autoheader"
    runCommand(cmd)

    cmd = "automake --add-missing"
    runCommand(cmd)

    cmd = "autoconf"
    runCommand(cmd)

########################################################################
# Run a command in a shell, wait for it to complete

def runCommand(cmd):

    if (options.debug == True):
        print >>sys.stderr, "running cmd:", cmd, " ....."
    
    try:
        retcode = subprocess.check_call(cmd, shell=True)
        if retcode != 0:
            print >>sys.stderr, "Child exited with code: ", retcode
            sys.exit(1)
        else:
            if (options.verbose == True):
                print >>sys.stderr, "Child returned code: ", retcode
    except OSError, e:
        print >>sys.stderr, "Execution failed:", e
        sys.exit(1)

    if (options.debug == True):
        print >>sys.stderr, ".... done"
    
########################################################################
# Run - entry point

if __name__ == "__main__":
   main()
