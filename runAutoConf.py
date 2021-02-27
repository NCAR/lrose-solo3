#!/usr/bin/env python

#===========================================================================
#
# Run autoconf etc on a dir
#
#===========================================================================

from __future__ import print_function
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
        print("Running %s:" % thisScriptName, file=sys.stderr)
        print("  Dir:", options.dir, file=sys.stderr)

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
        print("running cmd:", cmd, " .....", file=sys.stderr)
    
    try:
        retcode = subprocess.check_call(cmd, shell=True)
        if retcode != 0:
            print("Child exited with code: ", retcode, file=sys.stderr)
            sys.exit(1)
        else:
            if (options.verbose == True):
                print("Child returned code: ", retcode, file=sys.stderr)
    except OSError as e:
        print("Execution failed:", e, file=sys.stderr)
        sys.exit(1)

    if (options.debug == True):
        print(".... done", file=sys.stderr)
    
########################################################################
# Run - entry point

if __name__ == "__main__":
   main()
