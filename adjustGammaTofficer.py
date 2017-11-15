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
#initial bandwidth level, rate, capacity and period
item = ['downlink_800000', '10', '30000', '2000000']
gammaList = []
tList = []
deltaList = []
windowList = []

thputList = []
delayList = []
globalCount = 0
THRESHOLD = 100000
#when compute T use MS_US
#MS_US = 1000
PARAMETER = parameter.FLAG_GAMMA

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
    capacity = int(item[2])
    period = int(item[3])
    
    return rate, capacity, period

def IsNewFile(files, level, trace, gamma, t, window, delta):
    if os.path.isfile(trace):
        rate, capacity, period = GetInitialValue()
        output_path = level.split('/')[-1]
        output_path = os.path.join(output_path, files.split('.')[0] + '-' + str(gamma)\
         + '-' + str(t) + '-' + str(rate) + '-' + str(window) + '-' + str(delta) +'-' + str(period) + '.sum')
        # print output_path
        if os.path.isfile(output_path):
            return False
        else :
            return True

def RunMulThread(_gamma, _t, _window, _delta):
    threadList = []
    threadCount = 0
    level = os.path.join(top, item[0])
    for files in os.listdir(level):
        if threadCount >= 1:
            break
        if os.path.isdir(level):
            trace = os.path.join(level, files)
            if IsNewFile(files, level, trace, _gamma, _t, _window, _delta):
                rate, capacity, period = GetInitialValue()
                threadCount = threadCount + 1
                thread_id = process_bw_level(level, rate, _window, period, _gamma, _t, capacity, _delta, trace)
                thread_id.start()
                threadList.append(thread_id)
                time.sleep(1)

    for thread in threadList:
        thread.join()    

def stopIteration():
    global globalCount
    if globalCount < 10:
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

def GetIndexToMaxThput():
    maxValueIndex = thputList.index(max(thputList))
    if delayList[maxValueIndex] > THRESHOLD:
        del thputList[maxValueIndex]
        del delayList[maxValueIndex]
        del gammaList[maxValueIndex]
        maxValueIndex = GetIndexToMaxThput()

    return maxValueIndex

def AppendParameter(PARAMETER, p_temp):
    if PARAMETER == parameter.FLAG_EXIT:
        copydata.Show("append parameter failure! because PARAMETER = FLAG_EXIT.")
        sys.exit(0)

    path = GetCurrentPath()
    if path == 0:
        copydata.Show("Get current path failure!")
        return
    
    if PARAMETER == parameter.FLAG_GAMMA:
        gammaList.append(round(p_temp, 2))
        thput, delay = avgthputdelay.GetThputDelay(path, p_temp, parameter.INIT_T, parameter.INIT_WINDOW, parameter.INIT_DELTA)
    elif PARAMETER == parameter.FLAG_T:
        tList.append(round(p_temp, 2))
        copydata.Show("t =", p_temp)
        thput, delay = avgthputdelay.GetThputDelay(path, gammaList[-1], p_temp, parameter.INIT_WINDOW, parameter.INIT_DELTA)
    elif PARAMETER == parameter.FLAG_WINDOW:
        windowList.append(round(p_temp, 2))
        copydata.Show("window =", p_temp)
        thput, delay = avgthputdelay.GetThputDelay(path, gammaList[-1], tList[-1], p_temp, parameter.INIT_DELTA)
    else:
        deltaList.append(round(p_temp, 2))
        copydata.Show("delta =", p_temp)
        thput, delay = avgthputdelay.GetThputDelay(path, gammaList[-1], tList[-1], windowList[-1], p_temp)
        
    thputList.append(round(thput, 2))
    delayList.append(round(delay, 2))

def ClearList(pList):
    global thputList, delayList
    copydata.Show(pList)
    copydata.Show(thputList)
    pList.append(round(pList[GetIndexToMaxThput()], 2))
    thputList = []
    delayList = []

def Initial(PARAMETER, aparameter, gamma = parameter.INIT_GAMMA, t = parameter.INIT_T, 
        window = parameter.INIT_WINDOW, delta = parameter.INIT_DELTA):
    RunMulThread(gamma, t, window, delta)
    AppendParameter(PARAMETER, aparameter)

def InitialFunc(PARAMETER):
    if PARAMETER == parameter.FLAG_EXIT:
        copydata.Show("initial failure! because PARAMETER = FLAG_EXIT.")

    #fix T, WINDOW, delta, adjust gamma
    if PARAMETER == parameter.FLAG_GAMMA:
        Initial(PARAMETER, parameter.INIT_GAMMA)
        Initial(PARAMETER, parameter.SEC_GAMMA, parameter.SEC_GAMMA)
            
    #fix gamma, WINDOW, delta, adjust T
    elif PARAMETER == parameter.FLAG_T:
        ClearList(gammaList)
        AppendParameter(PARAMETER, parameter.INIT_T)
        Initial(PARAMETER, parameter.SEC_T, gammaList[-1], parameter.SEC_T)

    #fix gamma, T, delta, adjust WINDOW
    elif PARAMETER == parameter.FLAG_WINDOW:
        ClearList(tList)
        AppendParameter(PARAMETER, parameter.INIT_WINDOW)
        Initial(PARAMETER, parameter.SEC_WINDOW, gammaList[-1], tList[-1], parameter.SEC_WINDOW)

    #fix gamma, T, WINDOW, adjust delta
    elif PARAMETER == parameter.FLAG_DELTA:
        ClearList(windowList)
        AppendParameter(PARAMETER, parameter.INIT_DELTA)
        Initial(PARAMETER, parameter.SEC_DELTA, gammaList[-1], 
                tList[-1], windowList[-1], parameter.SEC_DELTA)

def RunFunction(p_temp, PARAMETER):
    if PARAMETER == parameter.FLAG_EXIT:
        copydata.Show("run failure! because PARAMETER = parameter.FLAG_EXIT.")
        sys.exit(0)
    if PARAMETER == parameter.FLAG_GAMMA:
        RunMulThread(p_temp, parameter.INIT_T, parameter.INIT_WINDOW, parameter.INIT_DELTA)
    elif PARAMETER == parameter.FLAG_T:
        RunMulThread(gammaList[-1], p_temp, parameter.INIT_WINDOW, parameter.INIT_DELTA)
    elif PARAMETER == parameter.FLAG_WINDOW:
        RunMulThread(gammaList[-1], tList[-1], p_temp, parameter.INIT_DELTA)
    else :
        RunMulThread(gammaList[-1], tList[-1], windowList[-1], p_temp)

def UpdateParameter(newThput, delay, bdLevel, PARAMETER):
    if PARAMETER == parameter.FLAG_GAMMA:
        copydata.Show("gamma history", gammaList)
        return update.UpdateGamma(newThput, delay, bdLevel, thputList[-2], gammaList[-2], gammaList[-1])
    elif PARAMETER == parameter.FLAG_T:
        copydata.Show("tList history", tList)
        return update.UpdateT(newThput, delay, bdLevel, thputList[-2], tList[-2], tList[-1])
    elif PARAMETER == parameter.FLAG_WINDOW:
        copydata.Show("windowList history", windowList)
        return update.UpdateWindow(newThput, delay, bdLevel, thputList[-2], windowList[-2], windowList[-1])
    else:
        copydata.Show("deltaList history", deltaList)
        return update.UpdateDelta(newThput, delay, bdLevel, thputList[-2], deltaList[-2], deltaList[-1])

def ChangeParameter(p_temp, PARAMETER):
    if isinstance(p_temp, float):
        copydata.Show("p_temp is a float number!")
        return False
    if p_temp != PARAMETER:
        return True
    else:
        return False

def IteraterExit(PARAMETER):
    if PARAMETER == parameter.FLAG_EXIT:
        return True
    else:
        return False

def WriteResult(temp):
    writefile = open('result.txt', 'a')
    tempList = []
    if temp == parameter.FLAG_T:
        tempList = gammaList
    elif temp == parameter.FLAG_WINDOW:
        tempList = tList
    elif temp == parameter.FLAG_DELTA:
        tempList = windowList
    else:
        tempList = deltaList

    for line in range(len(tempList)):
        str_write = '%lf %lf %lf\n'%(tempList[line], thputList[line], delayList[line])
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
        InitialFunc(PARAMETER)

        while not stopIteration():
            path = GetCurrentPath()
            if path == 0:
                copydata.Show("Get current path failure!")
            bdLevel = int(item[0].split('_')[-1])
            #temp can be gamma, T, window, delta
            temp = UpdateParameter(thputList[-1], delayList[-1], bdLevel/100000, PARAMETER)
            if ChangeParameter(temp, PARAMETER):
                WriteResult(temp)
                copydata.Show ("temp = ", temp, "PARAMETER = ", PARAMETER)
                PARAMETER = temp
                break
            RunFunction(temp, PARAMETER)
            #copydata.Show("gamma =", p_temp)
            AppendParameter(PARAMETER, temp)

        if IteraterExit(PARAMETER):
            copydata.Show("iterater done!")
            break
    #WriteResult()
    UpdateData()

