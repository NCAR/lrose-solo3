#!/usr/bin/env python

# ========================================================================== #
#
# Find the unique set of dynamic lib files for all of the binaries
# in a given directory.
# Copy those dynamic libraries to a select location, normally:
#   $ORIGIN/../rel_origin/lib
#
# ========================================================================== #

import string
import os
from os.path import join, getsize
import sys
import subprocess
from optparse import OptionParser
import shutil

def main():

    global progName
    global options
    global debug
    global libDict
    global ignoreList

    progName = os.path.basename(sys.argv[0])
    
    # parse the command line

    usage = "usage: %prog [options]: installs $ORIGIN lib files"
    ignoreDefault = 'libc.so,libpthread.so,libdl.so'

    parser = OptionParser(usage)
    parser.add_option('--debug',
                      dest='debug', default='False',
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--binDir',
                      dest='binDir', default='.',
                      help='Path to installed binaries')
    parser.add_option('--relDir',
                      dest='relDir', default='../rel_origin/lib',
                      help='Path of installed libs relative to bins')
    parser.add_option('--ignore',
                      dest='ignore',
                      default=ignoreDefault,
                      help='Comma-delimited list of libs to ignore. ' +
                      'Any lib containing these strings will not be ' +
                      'included in the install. Default is: ' +
                      ignoreDefault)

    (options, args) = parser.parse_args()

    ignoreList = options.ignore.split(',')

    if (options.debug == True):
        print >>sys.stderr, "Options:"
        print >>sys.stderr, "  Debug: ", options.debug
        print >>sys.stderr, "  binDir: ", options.binDir
        print >>sys.stderr, "  relDir: ", options.relDir
        print >>sys.stderr, "  ignoreList: ", ignoreList
        
    # compile the list of dependent libs

    libDict = {}
    compileLibList()

    # copy each file

    for libName in libDict.keys():
        libPath = libDict[libName]
        copyLib(libName, libPath)

    sys.exit(0)

########################################################################
# Get the list of libs

def compileLibList():

    # read in list of files in binDir

    fileList = os.listdir(options.binDir)
    for fileName in fileList:
        binPath = os.path.join(options.binDir, fileName)
        if (os.path.isfile(binPath)):
            getLibsForBin(binPath)

########################################################################
# Get the libs for a binary

def getLibsForBin(binPath):

    global libDict

    if (options.debug == True):
        print >>sys.stderr, "  Checking bin file: ", binPath

    # check if this is an elf file - will not work on Mac
    
    pipe = subprocess.Popen('file ' + binPath, shell=True,
                            stdout=subprocess.PIPE).stdout
    lines = pipe.readlines()
    isElfFile = False
    for line in lines:
        if (line.find('ELF') >= 0):
            isElfFile = True
    if (isElfFile == False):
        return

    # get list of dynamic libs using ldd
    
    pipe = subprocess.Popen('ldd ' + binPath, shell=True,
                            stdout=subprocess.PIPE).stdout
    lines = pipe.readlines()
    for line in lines:
        line.strip()
        toks = line.split()
        libName = ''
        libPath = ''
        libFound = False
        for ii in range(1,len(toks)-1):
            if (toks[ii] == '=>'):
                libName = toks[ii-1]
                libPath = toks[ii+1]
                if (libPath.find(libName) >= 0):
                    libFound = True
                break
        if (libFound):
            useThisLib = True
            for ignoreStr in ignoreList:
                if (libName.find(ignoreStr) >= 0):
                    useThisLib = False
            if (useThisLib == True):
                libDict[libName] = libPath

########################################################################
# Copy the lib to the rel dir

def copyLib(libName, libPath):

    destDir = os.path.join(options.binDir, options.relDir)
    
    if (options.debug == True):
        print >>sys.stderr, "===>>> Copying lib: ", libPath
        print >>sys.stderr, "===>>>   to:: ", destDir

    # ensure directory exists
    
    if not os.path.exists(destDir):
        os.makedirs(destDir)

    # copy in lib file
    
    try:
        shutil.copy2(libPath, destDir)
    except shutil.Error, err:
        print >>sys.stderr, "===>>> WARNING: ", err
 
########################################################################
# Run - entry point

if __name__ == "__main__":
   main()
