import os
import sys
import shutil

bw_bandPath = '/root/bw_bank/'
path = '/home/wanwenkai/fixGammaT_adjustWin/'
verifyPath = '/home/wanwenkai/verify_data/'

def makeDir(dispath, dirname):
	print dirname
	dirname = os.path.join(dispath, dirname)
	if os.path.isdir(dirname):
		shutil.rmtree(dirname)
		os.mkdir(dirname)
	else:
		os.mkdir(dirname)
	return dirname

def copyFiles(sfilename, sourcepath, dispath):
	disfile = os.path.join(dispath, sfilename)
	if os.path.isfile(disfile):
		os.remove(disfile)
	else:
		sfilename = os.path.join(sourcepath, sfilename)
		shutil.copy(sfilename, dispath)

def findFiles(dispath, sourcepath, trainpath):
	filename_list = []
	for tfilename in os.listdir(trainpath):
		if tfilename.startswith('proUDP'):
			filename_list.append(tfilename.split('-')[0])
	for sfilename in os.listdir(sourcepath):
		if sfilename.startswith('proUDP'):
			sfile = sfilename.split('.')[0]
			if sfile in filename_list:
				continue
			copyFiles(sfilename, sourcepath, dispath)

if __name__ == '__main__':
	for dirname in os.listdir(path):
		if dirname.startswith('downlink'):
			dispath = makeDir(verifyPath, dirname)
			trainpath = os.path.join(path, dirname)
			sourcepath = os.path.join(bw_bandPath, dirname)
			findFiles(dispath, sourcepath, trainpath)
