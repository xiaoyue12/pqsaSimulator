import os
import sys
import parameter

def InitialFunc(PARAMETER):
    if PARAMETER == parameter.FLAG_EXIT:
        copydata.Show("initial failure! because PARAMETER = FLAG_EXIT.")
        return

    #fix T, capacity, delta, adjust gamma
    if PARAMETER == parameter.FLAG_GAMMA:
        RunMulThread(parameter.INIT_GAMMA, parameter.INIT_T, \
                parameter.INIT_CAPACITY, parameter.INIT_DELTA)
        AppendParameter(parameter.INIT_GAMMA, PARAMETER)
        RunMulThread(parameter.SEC_GAMMA, parameter.INIT_T, \
                parameter.INIT_CAPACITY, parameter.INIT_DELTA)
        AppendParameter(parameter.SEC_GAMMA, PARAMETER)
    
    #fix gamma, capacity, delta, adjust T
    elif PARAMETER == parameter.FLAG_T:
        RunMulThread(gammaList[-1], parameter.INIT_T, \
                parameter.INIT_CAPACITY, parameter.INIT_DELTA)
        AppendParameter(parameter.INIT_T, PARAMETER)
        RunMulThread(gammaList[-1], parameter.SEC_T, \
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

def IteraterExit(p_temp):
    return math.isclose(p_temp, 0):

