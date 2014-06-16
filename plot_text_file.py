import numpy as NP
from matplotlib import pyplot as PLT
import time



with open('generated_logs/out_deltas.txt') as f:
	v = f.readlines()
	for element in v: element.strip().toint()
 #	v = NP.loadtxt(f, delimiter=",", dtype='int', comments="#", skiprows=0, usecols=None)
with open('generated_logs/drawn_deltas.txt') as f2:
#	v2 = NP.loadtxt(f2, delimiter=",", dtype='int', comments="#", skiprows=0, usecols=None)
	v2=f2.readlines()
	for element in v2: element.strip().toint()
print "pure lines"
print v2
print v


v_hist= NP.ravel(v)
print(v_hist)
v_hist.sort()
print(len(v_hist))
print(v_hist)

v2_hist=NP.ravel(v2)
print(v2_hist)
v2_hist.sort()
print(len(v2_hist))
print(v2_hist)

fig = PLT.figure(num=None, figsize=(16, 9), dpi=80, facecolor='w', edgecolor='k')


ax = fig.add_subplot(111, axisbg='w')
ax1 = fig.add_subplot(211)	#subplotgrid, 1 x 1
ax2 = fig.add_subplot(212)	#subplotgrid, 1 x 1

# Turn off axis lines and ticks of the big subplot
ax.spines['top'].set_color('none')
ax.spines['bottom'].set_color('none')
ax.spines['left'].set_color('none')
ax.spines['right'].set_color('none')
ax.tick_params(labelcolor='w', top='off', bottom='off', left='off', right='off')


ax1.set_yscale('log')
ax2.set_yscale('log')

fig.suptitle('Simulated MRd-CplD latency distribution ', fontsize=20)

ax1.set_title('Latency distribution experienced by EP')
ax2.set_title('Latency distribution modelled by RC')

ax.set_xlabel('Delay (ns)', fontsize=18)
ax.set_ylabel('Log scaled sample-number', fontsize=16)

n, bins, patches = ax1.hist(v_hist, bins = range(0,10000,10), facecolor='green')
n, bins, patches = ax2.hist(v2_hist, bins = range(0,10000,10), facecolor='green')

fig.savefig('generated_logs/out_deltas.jpg', dpi=fig.dpi)
#PLT.draw()

#time.sleep(3)
#PLT.close()
