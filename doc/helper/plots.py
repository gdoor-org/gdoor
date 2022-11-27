import math
import numpy
import matplotlib.pyplot as plt
from mpl_toolkits.axes_grid1.inset_locator import inset_axes,mark_inset

x = numpy.arange(0, 300*1/60e3, 0.01*1/60e3)
y = []
for t in x:
    u = 0
    if t < 3*1/60e3:
        u = 26
    elif t < (3+60)*1/60e3:
        u = 26+2*math.sin(2*math.pi*t*60e3)
    elif t < (3+60+30)*1/60e3:
        u = 26
    elif t < (3+60+30+32)*1/60e3:
        u = 26+2*math.sin(2*math.pi*t*60e3)
    elif t < (3+60+30+32+30)*1/60e3:
        u = 26
    elif t < (3+60+30+32+30+12)*1/60e3:
        u = 26+2*math.sin(2*math.pi*t*60e3)
    elif t < (3+60+30+32+30+12+30)*1/60e3:
        u = 26
    elif t < (3+60+30+32+30+12+30+12)*1/60e3:
        u = 26+2*math.sin(2*math.pi*t*60e3)
    else:
        u = 26
    y.append(u)

# Note that even in the OO-style, we use `.pyplot.figure` to create the Figure.
fig, ax = plt.subplots(figsize=(15, 5), layout='constrained')
ax.plot(x/1e-3, y, label='Bus Voltage')  # Plot some data on the axes.
ax.set_xlabel('Time [ms]')  # Add an x-label to the axes.
ax.set_ylabel('Voltage [V]')  # Add a y-label to the axes.
ax.set_title("Bus example")  # Add a title to the axes.
ax.set_ylim(0,35)
ax.set_xlim(0,(3+60+30+32+30+12+30+12)*1/60)

ax.annotate('', xy=((3)*1/60, 32), xytext=((3+60)*1/60, 32), xycoords='data', textcoords='data',
            arrowprops={'arrowstyle': '|-|'})
ax.annotate('Startbit', xy=((3+30)*1/60, 33.5), ha='center', va='center')

ax.annotate('', xy=((3+60+30)*1/60, 32), xytext=((3+60+30+32)*1/60, 32), xycoords='data', textcoords='data',
            arrowprops={'arrowstyle': '|-|'})
ax.annotate('Bitvalue:\n0', xy=((3+60+30+16)*1/60, 33.5), ha='center', va='center')

ax.annotate('', xy=((3+60+30+32+30)*1/60, 32), xytext=((3+60+30+32+30+12)*1/60, 32), xycoords='data', textcoords='data',
            arrowprops={'arrowstyle': '|-|'})
ax.annotate('Bitvalue:\n1', xy=((3+60+30+32+30+6)*1/60, 33.5), ha='center', va='center')

axins = inset_axes(ax,3,2,loc='lower right', borderpad=4)
axins.plot(x/1e-3, y)
axins.set_xlim((3)*1/60,0.2)
axins.set_ylim(23,31)

axins.annotate('', xy=((4)*1/60+1/(60*4), 29), xytext=((5)*1/60+1/(60*4), 29), xycoords='data', textcoords='data',
            arrowprops={'arrowstyle': '|-|'})
axins.annotate('60 kHz', xy=((4.5)*1/60+1/(60*4), 30), ha='center', va='center')
mark_inset(ax,axins,loc1=1,loc2=3)

plt.savefig('../busvoltage.png')
