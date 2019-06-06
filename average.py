import os

path = '/home/qiuxiaoyue/simulator_scripts_3/annealing_scripts/downlink_500000_delta/'

bwList = []
delayList = []
avgThput = []
avgDelay = []

# gammaList = []
# tList = []
# windowList = []
deltaList = []

tempList = []

for files in os.listdir(path):
	# gammaList.append(float(files.split('-')[-6]))
	# tList.append(float(files.split('-')[-5]))
	# windowList.append(float(files.split('-')[-3]))
	deltaList.append(float(files.split('-')[-2]))
	
	files = os.path.join(path,files)
	for line in open(files):
		odom = line.split()
		bwList.append(float(odom[0]))
		delayList.append(float(odom[1]))
	
	avgThput.append(round(float(sum(bwList)/len(bwList)),2))
	avgDelay.append(round(float(sum(delayList)/len(delayList)),2))

writefile = open('result_delta.txt','a')
tempList = []
# tempList = gammaList
# tempList = tList
# tempList = windowList
tempList = deltaList
for line in range(len(tempList)):
	str_write = '%lf %lf %lf\n'%(tempList[line],avgThput[line],delayList[line])
	writefile.write(str_write)
writefile.close()
