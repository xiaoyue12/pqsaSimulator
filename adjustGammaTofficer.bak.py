import os
import sys
import threading
import time
import avgthputdelay
import copydata

#period = [2000000]
delta = 2000
capacity = 30000
top = '/root/bw_bank/'
datapath = '/home/wanwenkai/pqsa_simulator/annealing_scripts/'
savePath = '/home/wanwenkai/pqsa_simulator/annealingData/'
#initial bandwidth level, rate, window and period
item = ['downlink_400000', '10', '110000', '10000000']
gammaList = []
thputList = []
delayList = []
globalCount = 0
THRESHOLD = 5000
class process_bw_level(threading.Thread):
	def __init__(self, level, rate, window, period, gamma, t, trace):
		threading.Thread.__init__(self)
		self.__rate = rate
		self.__window = window
		self.__period = period
		self.__gamma = gamma
		self.__t = t
		self.__level = level
		self.__trace = trace
	def run(self):
		level = self.__level
		rate = self.__rate
		window = self.__window
		period = self.__period
		gamma = self.__gamma
		t = self.__t
		trace = self.__trace
	
		os.system('./main' + ' ' + str(rate) + ' ' + str(window) + ' ' + \
		str(delta) + ' ' + str(capacity) + ' ' + str(period) + ' ' + trace + \
		' ' + str(gamma) + ' ' + str(t) + ' ' + '> /dev/null 2>&1')

def GetInitialValue():
	rate = int(item[1])
	window = int(item[2])
	period = int(item[3])
	
	return rate, window, period

def IsNewFile(files, level, trace, gamma, t):
	if os.path.isfile(trace):
		rate, window, period = GetInitialValue()
		output_path = level.split('/')[-1]
		output_path = os.path.join(output_path, files.split('.')[0] + '-' + str(gamma)\
		 + '-' + str(t) + '-' + str(rate) + '-' + str(window) + '-' + str(period) + '.sum')
		# print output_path
		if os.path.isfile(output_path):
			return False
		else :
			return True

def RunMulThread(_gamma, _t):
	threadList = []
	threadCount = 0
	level = os.path.join(top, item[0])
	for files in os.listdir(level):
		if threadCount >= 10:
			break
		if os.path.isdir(level):
			trace = os.path.join(level, files)
			if IsNewFile(files, level, trace, _gamma, _t):
				rate, window, period = GetInitialValue()
				threadCount = threadCount + 1
				thread_id = process_bw_level(level, rate, window, period, _gamma, _t, trace)
				thread_id.start()
				threadList.append(thread_id)
				time.sleep(1)

	for thread in threadList:
		thread.join()    

def stopIteration():
	global globalCount
	if globalCount < 5:
		globalCount = globalCount + 1
		return False	
	else :
		globalCount = 0
		return True	

def UpdateGammaAndT(newThput, delay, bdLevel):
	oldThput = thputList[-1]
	oldGamma = gammaList[-2]
	newGamma = gammaList[-1]
	t = 5000

	if delay > THRESHOLD:
		newGamma = (oldGamma + newGamma)/2
	elif newThput - oldThput > 0:
		deltaThput = newThput - oldThput
		oldGamma = newGamma
		step = deltaThput/((bdLevel + 1)*100)
		newGamma = oldGamma + step
	else:
		newGamma = (oldGamma + newGamma)/2
	
	return newGamma, t

def GetCurrentPath():
	for item in os.listdir(datapath):
		if item.startswith('downlink_'):
			dirname = os.path.join(datapath, item)
			if os.path.isdir(dirname):
				return dirname		
	return 0

def AppendParameter(_gamma, _t):
	path = GetCurrentPath()
	if path == 0:
		print "Can't get current directon's path!"
	thput, delay = avgthputdelay.GetThputDelay(path, _gamma, _t)
	thputList.append(thput)
	delayList.append(delay)
	gammaList.append(_gamma)

def InitialFunc():
	RunMulThread(1.0, 5000)
	AppendParameter(1.0, 5000)
	RunMulThread(1.1, 5000)
	AppendParameter(1.1, 5000)

def WriteResult():
	writefile = open('result.txt', 'w')
	for line in range(len(gammaList)):
		str_write = '%lf %d %d\n'%(gammaList[line], thputList[line], delayList[line])
		writefile.write(str_write)
	writefile.close()

def UpdateData():
	sPath = GetCurrentPath()
	dirname = sPath.split('/')[-1]
	dPath = copydata.MakeDir(savePath, dirName)
	copydata.FindFiles(dPath, sPath)
	shutil.retree(sPath)

if __name__ == "__main__":
	InitialFunc()
	while not stopIteration():
		path = GetCurrentPath()
		thput, delay = avgthputdelay.GetThputDelay(path, gammaList[-1], 5000)
		bdLevel = int(item[0].split('_')[-1])
		gamma, t = UpdateGammaAndT(thput, delay, bdLevel)
		AppendParameter(gamma, t)
		RunMulThread(gamma, t)
	WriteResult()
	UpdateData()
