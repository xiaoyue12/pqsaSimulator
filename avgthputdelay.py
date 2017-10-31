#!/usr/bin/python
# coding:utf-8

import os
import sys
import copydata

#path = '/home/wanwenkai/simulator_scripts/annealing_scripts/downlink_500000/'

def GetThputDelay(path, _gamma, _t, _window, _delta):
    bwList = []
    delayList = []
    copydata.Show("_gamma =", _gamma, "_t =", _t, "_window =", _window, 
            "_delta =", _delta)
    for files in os.listdir(path):
        if files.endswith('.sum'):
            gamma = float(files.split('-')[-6])
            t = float(files.split('-')[-5])
            window = float(files.split('-')[-3])
            delta = float(files.split('-')[-2])
            #t = int(files.split('-')[-4])
            if gamma == _gamma and t == _t and _window == window and _delta == delta:
                files = os.path.join(path, files)
                #print files
                for line in open(files):
                    odom = line.split()
                    #if int(odom[1]) > 2000000:
                    #	continue
                    bwList.append(float(odom[0]))
                    delayList.append(float(odom[1]))
                copydata.Show ("sum bandwidth =", sum(bwList), "sum file length =", len(bwList))
    
    if sum(bwList) == 0 or sum(delayList) == 0:
        copydata.Show ("File is NULL!")
        sys.exit(0)
    avgThput = round(float(sum(bwList)/len(bwList)), 2)
    avgDelay = round(float(sum(delayList)/len(delayList)), 2)			
    copydata.Show ("avg throughput =", avgThput, "avg delay =", avgDelay)
    
    return avgThput, avgDelay

#if __name__ == "__main__":
#    GetThputDelay(path, 1.29)
