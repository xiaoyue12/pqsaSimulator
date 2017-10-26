import os
import sys
import parameter
import copydata
import adjustGammaTofficer

def UpdateT(newThput, delay, bdLevel, oldThput, old_T, new_T):
    
    if delay > adjustGammaTofficer.THRESHOLD:
        new_T = float((old_T + new_T)/2)
        copydata.Show("delay is more than THRESHOLD!")
        return round(new_T, 2)
    elif newThput - oldThput > 0:
        deltaThput = newThput - oldThput
        old_T = new_T
        #because T's incremental is large, such as deltaT = 5000 or deltaT = 15000, 
        #but gamma's incremental is small, such as deltagamma = 0.1 or deltagamma = 0.2,
        #MS_US = 1000 and UNIT = 100
        step = round(float(deltaThput)/((bdLevel + 1)*100), 2)*100
        copydata.Show("step =", step)
        if abs(step) < 1.0:
            return parameter.FLAG_EXIT
        new_T = float(old_T + step*parameter.MS_US)
        return round(new_T, 2)
    elif newThput - oldThput < 0:
        new_T = float((old_T + new_T)/2)
        step = round(float(old_T - new_T), 2)
        copydata.Show("step =", step)
        copydata.Show ("new throughput is less than old throughput!")
        if abs(step) < 1.0:
            return parameter.FLAG_EXIT
        return round(new_T, 2)
    else:
        copydata.Show ("Iteration is done!")
        return parameter.FLAG_EXIT

def UpdateGamma(newThput, delay, bdLevel, oldThput, oldGamma, newGamma):
       
    #print oldGamma, newGamma
    if delay > adjustGammaTofficer.THRESHOLD:
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
        copydata.Show("step =", step)
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

#def UpdateParameter(PARAMETER, newThput, delay, bdLevel):
#    if PARAMETER == parameter.FLAG_GAMMA:
#        return UpdateGamma(newThput, delay, bdLevel)
#    elif PARAMETER == parameter.FLAG_T:
#        return UpdateT(newThput, delay, bdLevel)

