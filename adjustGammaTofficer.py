import os
import sys
import threading
import time
import avgthputdelay
import copydata
import shutil
import parameter
import update

#period = [2000000]
#delta = 2000
#capacity = 30000
top = '/home/wanwenkai/simulator_scripts/verify_sourceData/'
datapath = '/home/wanwenkai/simulator_scripts/annealing_scripts/'
savePath = '/home/wanwenkai/simulator_scripts/annealingData/'
#initial bandwidth level, rate, window and period
item = ['downlink_500000', '10', '110000', '10000000']
gammaList = []
tList = []
delta = []
capacity = []

thputList = []
delayList = []
globalCount = 0
THRESHOLD = 100000
#when compute T use MS_US
MS_US = 1000

class process_bw_level(threading.Thread):
    def __init__(self, level, rate, window, period, gamma, t, capacity, delta, trace):
        threading.Thread.__init__(self)
        self.__rate = rate
        self.__window = window
        self.__period = period
        self.__gamma = gamma
        self.__t = t
        self.__capacity = capacity
        self.__delta = delta
        self.__level = level
        self.__trace = trace
    def run(self):
        level = self.__level
        rate = self.__rate
        window = self.__window
        period = self.__period
        gamma = self.__gamma
        t = self.__t
        capacity = self.__capacity
        delta = self.__delta
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

def RunMulThread(_gamma, _t, _capacity, _delta):
    threadList = []
    threadCount = 0
    level = os.path.join(top, item[0])
    for files in os.listdir(level):
        if threadCount >= 1:
            break
        if os.path.isdir(level):
            trace = os.path.join(level, files)
            if IsNewFile(files, level, trace, _gamma, _t):
                rate, window, period = GetInitialValue()
                threadCount = threadCount + 1
                thread_id = process_bw_level(level, rate, window, period, _gamma, _t, _capacity, _delta, trace)
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

def GetCurrentPath():
    for item in os.listdir(datapath):
        if item.startswith('downlink_'):
            dirname = os.path.join(datapath, item)
            if os.path.isdir(dirname):
                return dirname
    return 0

def AppendParameter(p_temp, PARAMETER):
    if PARAMETER == parameter.FLAG_EXIT:
        copydata.Show("append parameter failure! because PARAMETER = FLAG_EXIT.")
        return

    path = GetCurrentPath()
    if path == 0:
        copydata.Show("Get current path failure!")
        return

    thput, delay = avgthputdelay.GetThputDelay(path, p_temp)
    #print thput, delay
    thputList.append(round(thput, 2))
    delayList.append(round(delay, 2))
    
    if PARAMETER == parameter.FLAG_GAMMA:
        gammaList.append(round(p_temp, 2))
    elif PARAMETER == parameter.FLAG_T:
        tList.append(round(p_temp, 2))
    elif PARAMETER == parameter.FLAG_CAPACITY:
        capacityList.append(round(p_temp, 2))
    else :
        deltaList.append(round(p_temp, 2))

def InitialFunc(PARAMETER):
    if PARAMETER == parameter.FLAG_EXIT:
        copydata.Show("initial failure! because PARAMETER = FLAG_EXIT.")

    #fix T, capacity, delta, adjust gamma
    if PARAMETER == parameter.FLAG_GAMMA:
        RunMulThread(parameter.INIT_GAMMA, parameter.INIT_T, 
                parameter.INIT_CAPACITY, parameter.INIT_DELTA)
        AppendParameter(parameter.INIT_GAMMA, PARAMETER)
        RunMulThread(parameter.SEC_GAMMA, parameter.INIT_T, 
                parameter.INIT_CAPACITY, parameter.INIT_DELTA)
        AppendParameter(parameter.SEC_GAMMA, PARAMETER)
    
    #fix gamma, capacity, delta, adjust T
    elif PARAMETER == parameter.FLAG_T:
        RunMulThread(gammaList[-1], parameter.INIT_T, 
                parameter.INIT_CAPACITY, parameter.INIT_DELTA)
        AppendParameter(parameter.INIT_T, PARAMETER)
        RunMulThread(gammaList[-1], parameter.SEC_T, 
                parameter.INIT_CAPACITY, parameter.INIT_DELTA)
        AppendParameter(parameter.SEC_T, PARAMETER)

    #fix gamma, T, delta, adjust capacity
    elif PARAMETER == parameter.FLAG_CAPACITY:
        RunMulThread(gammaList[-1], tList[-1], parameter.INIT_CAPACITY, parameter.INIT_DELTA)
        AppendParameter(parameter.INIT_CAPACITY, PARAMETER)
        RunMulThread(gammaList[-1], tList[-1], parameter.SEC_CAPACITY, parameter.INIT_DELTA)
        AppendParameter(parameter.SEC_CAPACITY, PARAMETER)

    #fix gamma, T, capacity, adjust delta
    else :
        RunMulThread(gammaList[-1], tList[-1], capacityList[-1], parameter.INIT_DELTA)
        AppendParameter(parameter.INIT_DELTA, PARAMETER)
        RunMulThread(gammaList[-1], tList[-1], capacityList[-1], parameter.SEC_DELTA)
        AppendParameter(parameter.SEC_DELTA, PARAMETER)

def RunFunction(p_temp, PARAMETER):
    if PARAMETER == parameter.FLAG_EXIT:
        copydata.Show("run failure! because PARAMETER = parameter.FLAG_EXIT.")
        sys.exit(0)

    if PARAMETER == parameter.FLAG_GAMMA:
        RunMulThread(p_temp, parameter.INIT_T, parameter.INIT_CAPACITY, parameter.INIT_DELTA)
    elif PARAMETER == parameter.FLAG_T:
        RunMulThread(gammaList[-1], p_temp, parameter.INIT_CAPACITY, parameter.INIT_DELTA)
    elif PARAMETER == parameter.FLAG_CAPACITY:
        RunMulThread(gammaList[-1], tList[-1], p_temp, parameter.INIT_DELTA)
    else :
        RunMulThread(gammaList[-1], tList[-1], capacityList[-1], p_temp)

def UpdateParameter(newThput, delay, bdLevel, PARAMETER):
    if PARAMETER == parameter.FLAG_GAMMA:
        copydata.Show("gamma history", gammaList)
        return update.UpdateGamma(newThput, delay, bdLevel, thputList[-2], gammaList[-2], gammaList[-1], PARAMETER)
    elif PARAMETER == parameter.FLAG_T:
        copydata.Show("tList history", tList)
        return update.UpdateT(newThput, delay, bdLevel, thputList[-2], tList[-2], tList[-1], PARAMETER)

def IteraterExit(PARAMETER):
    if PARAMETER == parameter.FLAG_EXIT:
        return True
    else:
        return False

def WriteResult():
    writefile = open('result.txt', 'w')
    for line in range(len(gammaList)):
        str_write = '%lf %lf %lf\n'%(gammaList[line], thputList[line], delayList[line])
        writefile.write(str_write)
    writefile.close()

def UpdateData():
    sPath = GetCurrentPath()
    if sPath == 0:
        copydata.Show("Get current path failure!")
    dirName = sPath.split('/')[-1]
    dPath = copydata.MakeDir(savePath, dirName)
    copydata.FindFiles(dPath, sPath)
    shutil.rmtree(sPath)

if __name__ == "__main__":

    while 1:

        PARAMETER = parameter.FLAG_GAMMA
        InitialFunc(PARAMETER)

        while not stopIteration():
            path = GetCurrentPath()
            #print path
            if path == 0:
                copydata.Show("Get current path failure!")
            bdLevel = int(item[0].split('_')[-1])
            #p_temp can be gamma, T, capacity, delta
            p_temp = UpdateParameter(thputList[-1], delayList[-1], bdLevel/100000, PARAMETER)
            if IteraterExit(PARAMETER):
                break
            RunFunction(p_temp, PARAMETER)
            #RunMulThread(gamma, t)
            copydata.Show("gamma =", gamma, "t =", t)
            AppendParameter(p_temp, PARAMETER)
        WriteResult()
        UpdateData()
        if IteraterExit(PARAMETER):
            sys.exit(0)

