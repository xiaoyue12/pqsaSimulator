import os
import sys
import parameter
import copydata
import adjustGammaTofficer

def UndateT(newThput, delay, bdLevel, oldThput, old_T, new_T, PARAMETER):
    
    if delay > adjustGammaTofficer.THRESHOLD:
        new_T = (old_T + new_T)/2
        copydata.Show("delay is more than THRESHOLD!")
        return round(new_T, 2)
    elif newThput - oldThput > 0:
        deltaThput = newThput - oldThput
        old_T = new_T
        #because T's incremental is large, such as deltaT = 5000 or deltaT = 15000, 
        #but gamma's incremental is small, such as deltagamma = 0.1 or deltagamma = 0.2,
        #MS_US = 1000 and UNIT = 100
        step = round(float(deltaThput)/((bdLevel + 1)*100), 2)*UNIT
        new_T = old_T + step*MS_US
        copydata.Show("step =", step)
        return round(new_T, 2)
    elif newThput - oldThput < 0:
        new_T = (old_T + new_T)/2
        step = round(float(old_T - new_T), 2)
        copydata.Show("step =", step)
        copydata.Show ("new throughput is less than old throughput!")
        return round(new_T, 2)
    else:
        copydata.Show ("Iteration is done!")
        PARAMETER = parameter.FLAG_EXIT
        return PARAMETER

def UpdateGamma(newThput, delay, bdLevel, oldThput, oldGamma, newGamma, PARAMETER):
       
    #print oldGamma, newGamma
    if delay > adjustGammaTofficer.THRESHOLD:
        newGamma = (oldGamma + newGamma)/2
        copydata.Show("delay is more than THRESHOLD!")
        return round(newGamma, 2)
    elif newThput > oldThput:
        deltaThput = newThput - oldThput
        oldGamma = newGamma
        step = round(float(deltaThput)/((bdLevel + 1)*100), 2)
        newGamma = oldGamma + step
        copydata.Show("step =", step)
        return round(newGamma, 2)
    elif newThput < oldThput:
        newGamma = (oldGamma + newGamma)/2
        step = round(float(oldGamma - newGamma), 2)
        copydata.Show("step =", step)
        copydata.Show ("new throughput is less than old throughput!")
        return round(newGamma, 2)
    else:
        copydata.Show ("gamma's iteration is done!")
        PARAMETER = parameter.FLAG_EXIT
        return PARAMETER

#def UpdateParameter(PARAMETER, newThput, delay, bdLevel):
#    if PARAMETER == parameter.FLAG_GAMMA:
#        return UpdateGamma(newThput, delay, bdLevel)
#    elif PARAMETER == parameter.FLAG_T:
#        return UpdateT(newThput, delay, bdLevel)

