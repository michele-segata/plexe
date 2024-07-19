import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.lines import Line2D
from matplotlib.patches import Patch
import sys
import code  # code.interact(local=dict(globals(), **locals()))

plt.rcParams['axes.grid'] = True
plt.rcParams['text.usetex'] = True


df1 = pd.read_csv(sys.argv[1])

# We look only after steady state at 45s
s1mint, s1maxt = 45, 90
df1 = df1[(df1.time >= s1mint) & (df1.time <= s1maxt)]

df1 = df1.sort_values('time')

# Conversion m/s to km/h
df1['speed'] = df1.speed * 3.6

p1 = [0, 1, 2]
p2 = [3, 4]


leaderkwargs = {'ls': '-',  'ms': 1, 'lw': 1, 'marker': '^'}
followerkwargs = {'ls': '--',  'ms': 0.8, 'lw': 0.6, 'marker': '.'}

# S1 analysis
w = 3.28*1.5 #3.28 should be the column-width of an a4paper 2columns paper in inches
fig, axs = plt.subplots(2, figsize=(w, w*9/16 * 1.8), sharex=True)

for veh in p1:
    tmp = df1[df1.nodeId == veh]
    kwargs = leaderkwargs if veh == 0 else followerkwargs
    axs[0].plot(tmp.time, tmp.speed, color='b', **kwargs)
    axs[1].plot(tmp.time, tmp.acceleration,  color='b', **kwargs)

for veh in p2:
    tmp = df1[df1.nodeId == veh]
    kwargs = leaderkwargs if veh == 3 else followerkwargs
    axs[0].plot(tmp.time, tmp.speed, color='r', **kwargs)
    axs[1].plot(tmp.time, tmp.acceleration, color='r', **kwargs)

axs[1].set_xlabel("Time $[s]$")
axs[0].set_ylabel("Speed $[km/h]$")
axs[1].set_ylabel("Acceleration $[m/s^2]$")

axs[0].set_xlim(s1mint, s1maxt)
axs[1].set_xlim(s1mint, s1maxt)


#axs[0].axvline(x = 50, color = 'k', ls='--', lw=0.8) #startPFapp
axs[0].axvline(x = 50.31, color = 'k', ls='--', lw=0.8) #PFrequest sent
axs[0].axvline(x = 50.32, color = 'k', ls='--', lw=0.8) #LaneChange completed
axs[0].axvline(x = 83.02, color = 'k', ls='--', lw=0.8) #PFCompleteAck
axs[0].axvline(x = 88.03, color = 'k', ls='--', lw=0.8) #ManeuverEnd

#axs[1].axvline(x = 50, color = 'k', ls='--', lw=0.8) #startPFapp
axs[1].axvline(x = 50.31, color = 'k', ls='--', lw=0.8) #PFrequest sent
axs[1].axvline(x = 50.32, color = 'k', ls='--', lw=0.8) #LaneChange completed
axs[1].axvline(x = 83.02, color = 'k', ls='--', lw=0.8) #PFCompleteAck
axs[1].axvline(x = 88.03, color = 'k', ls='--', lw=0.8) #ManeuverEnd




legend_elements = [Line2D([0], [0], color='b', label='p1 Leader', **leaderkwargs),
                   Line2D([0], [0], color='r',
                          label='p2 Leader', **leaderkwargs),
                   Line2D([0], [0], color='b',
                          label='p1 Followers', **followerkwargs),
                   Line2D([0], [0], color='r', label='p2 Followers', **followerkwargs), ]

fig.legend(handles=legend_elements, bbox_to_anchor=(0.8, 1), ncol=2)

plt.subplots_adjust(left=0.13, bottom=0.12, right=0.98, top=0.87, wspace=0.1)

plt.savefig("simplecase1.pdf", format='pdf')
