import matplotlib.pyplot as plt
# modify filename

filename_1 = 'result_gamma.txt'
X1,Y1 = [],[]
Y11 = []
with open(filename_1,'r') as f1:
	lines = f1.readlines() 
	for line in lines:
		value_1 = [float(s) for s in line.split()]
		X1.append(value_1[0])
	X1.sort()
	for x in X1:
		for line in lines:
			value=[float(s) for s in line.split()]
			if x == value[0]:
				Y1.append(value[1])
				Y11.append(value[2])
				break
'''
filename_2 = 'result_60000000_t.txt'
X2,Y2 = [],[]
with open(filename_2,'r') as f2:
	lines = f2.readlines()
	for line in lines:
		value_2 = [float(s) for s in line.split()]
		X2.append(value_2[0]/10000.0)
	X2.sort()
	for x in X2:
		for line in lines:
			value = [float(s) for s in line.split()]
			if x == (value[0]/10000.0):
				Y2.append(value[1])
				break

filename_3 = 'result_window.txt'
X3,Y3 = [],[]
with open(filename_3,'r') as f3:
	lines = f3.readlines()
	for line in lines:
		value_3 = [float(s) for s in line.split()]
		X3.append(value_3[0]/100000.0)
	X3.sort()
	for x in X3:
		for line in lines:
			value = [float(s) for s in line.split()]
			if x == (value[0]/100000.0):
				Y3.append(value[1])
				break

filename_4 = 'result_delta.txt'
X4,Y4 = [],[] 
with open(filename_4,'r') as f4:
	lines = f4.readlines()
	for line in lines:
		value_4 = [float(s) for s in line.split()]
		X4.append(value_4[0]/4000.0)
	X4.sort()
	for x in X4:
		for line in lines:
			value = [float(s) for s in line.split()]
			if x == (value[0]/4000.0):
				Y4.append(value[1])
				break

plt.xlabel('parameters')
plt.ylabel('throughput performance')
# plt.ylabel('delay performance')
plt.xlim(0.6,1.5)

# plt.plot(X1,Y1,color = 'blue',label = 'G (gamma=G)',linewidth=1)
# plt.plot(X2,Y2,color = 'yellow',label = 'T (t=T*10000)',linewidth=1)
plt.plot(X3,Y3,color = 'red',label = 'W (window=W*100000)',linewidth=1)
plt.plot(X4,Y4,color = 'cyan',label = 'D (delta=D*4000)',linewidth=1)

plt.legend(loc=8,fontsize='small',bbox_to_anchor=(1.17,0),title='session length 1min')
out_png='/home/qiuxiaoyue/simulator_scripts/annealing_scripts/figure_2.png'
plt.savefig(out_png,dpi=200)
#plt.show()
'''

plt.plot(X1,Y1,color = 'blue',label = 'throughput',linewidth = 1)
plt.twinx()
plt.plot(X1,Y11,color = 'red',label = 'delay',linewidth = 1)
plt.legend(loc=0,fontsize='small',title='parameter is gamma')

plt.show()
