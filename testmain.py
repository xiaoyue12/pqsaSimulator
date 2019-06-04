import os

# delta = 3000.0
window = 70000.0
while window <= 90000.0:
	t = 13000.0
	while t <= 14000:
		gamma = 1.42
		while gamma <= 1.50:
    		# print('./main' + ' ' + str(rate) + ' ' + str(window) + ' ' + str(delta) + ' ' + str(capacity) + ' ' + str(period) + ' ' + trace + ' ' + str(gamma) + ' ' + str(t) + ' ' + '> /dev/null 2>&1')
			os.system('./main 10.0'+' '+str(window)+' '+' 3000.0 30000.0 10000000.0 /home/qiuxiaoyue/simulator_scripts/verify_sourceData/downlink_500000/proUDP_386620.txt'+' '+str(gamma)+' '+ str(t)+' '+' > /dev/null 2>&1')
			gamma += 0.02
		t += 250.0
	window += 5000.0
