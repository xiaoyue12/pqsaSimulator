import os
import sys
import parameter
import copydata

def UpdateT(newThput, delay, bdLevel, oldThput, old_T, new_T):
    
    if delay > parameter.THRESHOLD:
        new_T = float((old_T + new_T)/2)
        copydata.Show("delay is more than THRESHOLD!")
        return round(new_T, 2)
    elif newThput - oldThput > 0:
        deltaThput = newThput - oldThput
        old_T = new_T
        #because T's incremental is large, such as deltaT = 5000 or deltaT = 15000, 
        #but gamma's incremental is small, such as deltagamma = 0.1 or deltagamma = 0.2,
        #MS_US = 1000 and UNIT = 100
        step = round(float(deltaThput)/((bdLevel + 1)*100)*100*parameter.MS_US, 2)
        if abs(step) < 1000.0:
            return parameter.FLAG_WINDOW
        copydata.Show("t step =", step)
        new_T = float(old_T + step)
        return round(new_T, 2)
    elif newThput - oldThput < 0:
        new_T = float((old_T + new_T)/2)
        step = round(float(old_T - new_T), 2)
        copydata.Show ("new throughput is less than old throughput!")
        if abs(step) < 1000.0:
            return parameter.FLAG_WINDOW
        copydata.Show("t step =", step)
        return round(new_T, 2)
    else:
        copydata.Show ("Iteration is done!")
        return parameter.FLAG_WINDOW

def UpdateGamma(newThput, delay, bdLevel, oldThput, oldGamma, newGamma):
       
    #print oldGamma, newGamma
    if delay > parameter.THRESHOLD:
        newGamma = (oldGamma + newGamma)/2
        copydata.Show("delay is more than THRESHOLD!")
        return round(newGamma, 2)
    elif newThput > oldThput:
        deltaThput = newThput - oldThput
        oldGamma = newGamma
        step = round(float(deltaThput)/((bdLevel + 1)*100), 2)
        if abs(step) < 0.001:
            return parameter.FLAG_T
        newGamma = oldGamma + step
        copydata.Show("gamma step =", step)
        return round(newGamma, 2)
    elif newThput < oldThput:
        newGamma = (oldGamma + newGamma)/2
        step = round(float(oldGamma - newGamma), 2)
        if abs(step) < 0.001:
            return parameter.FLAG_T
        copydata.Show("step =", step)
        copydata.Show ("new throughput is less than old throughput!")
        return round(newGamma, 2)
    else:
        copydata.Show ("gamma's iteration is done!")
        return parameter.FLAG_T

def UpdateWindow(newThput, delay, bdLevel, oldThput, oldWindow, newWindow):
     
    if delay > parameter.THRESHOLD:
        newWindow = float((oldWindow + newWindow)/2)
        copydata.Show("delay is more than THRESHOLD!")
        return round(newWindow, 2)
    elif newThput - oldThput > 0:
        deltaThput = newThput - oldThput
        oldWindow = newWindow
        #because T's incremental is large, such as deltaT = 5000 or deltaT = 15000, 
        #but gamma's incremental is small, such as deltagamma = 0.1 or deltagamma = 0.2,
        #MS_US = 1000 and UNIT = 100
        step = round(float(deltaThput)/((bdLevel + 1)*100)*parameter.MS_US*parameter.MS_US, 2)
        if abs(step) < 10000.0:
            return parameter.FLAG_DELTA
        copydata.Show("window step =", step)
        newWindow = float(oldWindow + step)
        return round(newWindow, 2)
    elif newThput - oldThput < 0:
        newWindow = float((oldWindow + newWindow)/2)
        step = round(float(oldWindow - newWindow), 2)
        copydata.Show ("new throughput is less than old throughput!")
        if abs(step) < 10000.0:
            return parameter.FLAG_DELTA
        copydata.Show("window step =", step)
        return round(newWindow, 2)
    else:
        copydata.Show ("Iteration is done!")
        return parameter.FLAG_DELTA

def UpdateDelta(newThput, delay, bdLevel, oldThput, oldDelta, newDelta):
     
    if delay > parameter.THRESHOLD:
        newDelta = float((oldDelta + newDelta)/2)
        copydata.Show("delay is more than THRESHOLD!")
        return round(newDelta, 2)
    elif newThput - oldThput > 0:
        deltaThput = newThput - oldThput
        oldDelta = newDelta
        #because T's incremental is large, such as deltaT = 5000 or deltaT = 15000, 
        #but gamma's incremental is small, such as deltagamma = 0.1 or deltagamma = 0.2,
        #MS_US = 1000 and UNIT = 100
        step = round(float(deltaThput)/((bdLevel + 1)*100)*parameter.MS_US*parameter.UNIT, 2)
        if abs(step) < 1000.0:
            return parameter.FLAG_EXIT
        copydata.Show("delta step =", step)
        newDelta = float(oldDelta + step)
        return round(newDelta, 2)
    elif newThput - oldThput < 0:
        newDelta = float((oldDelta + newDelta)/2)
        step = round(float(oldDelta - newDelta), 2)
        copydata.Show ("new throughput is less than old throughput!")
        if abs(step) < 1000.0:
            return parameter.FLAG_EXIT
        copydata.Show("delta step =", step)
        return round(newDelta, 2)
    else:
        copydata.Show ("Iteration is done!")
        return parameter.FLAG_EXIT
