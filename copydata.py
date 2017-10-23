#!/usr/bin/python

import os
import sys
import shutil

#sPath = '/home/wanwenkai/simulator_scripts/annealing_scripts/'
#dPath = '/home/wanwenkai/simulator_scripts/annealingData/'

def Show(*parameter):
    for tmp in parameter:
        print(tmp, end=' ')
    print("\n")

# If the fold name of annealingData/downlink_*** is exit, remove 
# it and then build a new fold.
def MakeDir(dispath, dirname):
    isExists = os.path.exists(dispath)
    if isExists:
        dirname = os.path.join(dispath, dirname)
        if os.path.isdir(dirname):
            shutil.rmtree(dirname)
            os.makedirs(dirname)
        else:
            os.makedirs(dirname)
        return dirname
    else:
        Show(dispath, " is not exist!")
        sys.exit(0)

# Copy the data name of sfilename that belonging in sourcepath to 
# dispath
def CopyFiles(sfilename, sourcepath, dispath):
    disfile = os.path.join(dispath, sfilename)
    if os.path.isfile(disfile):
        os.remove(disfile)
    else:
        sfilename = os.path.join(sourcepath, sfilename)
        shutil.copy(sfilename, dispath)

# Find the file's name which we want to copy
def FindFiles(dispath, sourcepath):
    for sfilename in os.listdir(sourcepath):
        if sfilename.startswith('proUDP'):
            CopyFiles(sfilename, sourcepath, dispath)

#if __name__ == '__main__':
#	dispath = MakeDir(dPath, "downlink_400000/")
#	FindFiles(dispath, sPath)
