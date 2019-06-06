import os

# gamma = 1.50
# t = 13250.0
# window = 70000.0
# delta = 2000.0
delta = 1000.0
while delta <= 3000.0:
    # print('./main' + ' ' + str(rate) + ' ' + str(window) + ' ' + str(delta) + ' ' + str(capacity) + ' ' + str(period) + ' ' + trace + ' ' + str(gamma) + ' ' + str(t) + ' ' + '> /dev/null 2>&1')
	os.system('./main 10.0 70000.0'+' '+str(delta)+' '+'30000.0 10000000.0 /home/qiuxiaoyue/simulator_scripts_3/verify_sourceData/downlink_500000/proUDP_386620.txt 1.50 13250.0 > /dev/null 2>&1')
	delta += 500.0
