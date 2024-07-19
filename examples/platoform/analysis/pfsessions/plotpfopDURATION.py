import matplotlib.pyplot as plt
import pandas as pd
import code  # code.interact(local=dict(globals(), **locals()))
import numpy as np
import seaborn as sns
import sys
from itertools import product, chain
from matplotlib.lines import Line2D

sns.set_theme(style="ticks")  # , palette="pastel")
plt.rcParams['text.usetex'] = True
plt.rcParams['axes.grid'] = True
plt.rc('font', **{'family': 'serif', 'serif': ['Computer Modern']})


w = 3.28*1.5  # 3.28 should be the column-width of an a4paper 2columns paper in inches
#plt.figure(figsize=(w, w*9/16*2))

vrs = [5, 10, 15, 20, 25, 30]
mps = [8, 25]
mads = [100, 200]
pr = [0.25, 0.5, 0.75, 1.0]

PFOPS = dict(enumerate([
    "REQUEST_SENT",
    "ABORT_COORD_TIMEOUT",
    "COMPLETE_COORD",
    "ABORT_INTRUDERDETECTED",
    "ABORT_TOOLONGLANECHANGE",
    "ABORT_REQUESTER_TIMEOUT",
    "RESP_DENIED",
    "AUTH_DENIED",
    "PF_COMPLETED",
    "ABORTMSGRECEIVED",
    "TX_FAILURE",
    "PFREQUEST_TX_FAILURE",
    "END_SIMULATION",
    "BARRIERCOOLDOWN",
    "BARRIERSWITCHOFF",
    "ABORT_CLOSE_TO_END_ROUTE",
]))

DENIED = ['RESP_DENIED', 'AUTH_DENIED']
ABORTED = ['ABORT_COORD_TIMEOUT', 'ABORT_INTRUDERDETECTED',
           'ABORT_TOOLONGLANECHANGE', 'ABORT_REQUESTER_TIMEOUT',
           'ABORTMSGRECEIVED', 'TX_FAILURE', 'PFREQUEST_TX_FAILURE']
    


def extractExpDf(data, exp):
    mps, mad, pr = exp
    query = f"mps=={mps} and mad=={mad} and pr=={pr}"
    df = data.query(query)
    return df


def catOKorAborted(r):
    if r == "PF_COMPLETED":
        return "PF_COMPLETED"
    elif r in ABORTED:
        return "ABORTED"
    elif r in DENIED:
        return "DENIED"
    else:
        return r


def pfopbarchartDURATA(data, exp, ax, ylabel=False, xlabel=False):
    mps, mad, pr = exp
    df = extractExpDf(data, exp)

    tmp = df[["vr", "duration", "pfcat"]]

    #print(df[df.pfcat=="ABORTED"].groupby("pfop").count())
    #code.interact(local=dict(globals(), **locals()))

    sp = sns.boxplot(x="vr", y="duration", hue="pfcat", hue_order=["PF_COMPLETED", "ABORTED"],
                     palette=["blue", "red"], data=tmp, saturation=1, linewidth = 0.5,
                     ax=ax, fliersize=0.8, whis=(0, 100), medianprops=dict(color="none", alpha=0.7),)  # , legend=False)

    ax.legend([],[], frameon=False)
    ax.set_ylabel("")
    ax.set_xlabel("")
    #code.interact(local=dict(globals(), **locals()))


    if ylabel:
        ax.set_ylabel("$D_{max}="+str(mad)+"$")
        ax.set_yticks(range(0, 101, 20))

    if xlabel:
        ax.set_title(f"$R={pr}$")


#####################
#  MAIN
#####################
data = pd.read_parquet(sys.argv[1])

vrs = [5, 15, 25, 30]
mps = [8]
mads = [50, 200]
pr = [0.25, 1.0]

data = data[data.vr.isin(vrs)]
data = data[data.duration > 0]
data['pfcat'] = data['pfop'].apply(lambda x: catOKorAborted(x))
data = data[data.pfcat.isin(["PF_COMPLETED", "ABORTED"])]


# Griglia mps x pr (costanti = mad), ogni elemento della griglia
# contiene tutti i vrs

for m in mps:
    print(f"Plotting for mps={m}...")
    fig, axs = plt.subplots(len(mads), len(pr), figsize=(
        w, w*9/16 * 1.6), sharex=True, sharey=True)
    iaxs = chain(*axs)

    for r, mad in enumerate(mads):
        for c, p in enumerate(pr):
            exp = (m, mad, p)
            print(f"\t exp={exp}")
            pfopbarchartDURATA(data, exp, next(iaxs), c == 0, r == 0)

    plt.subplots_adjust(left=0.19, bottom=0.15, right=0.98,
                        top=0.86, wspace=0.15, hspace=0.25)

    fig.supxlabel('$A_r$ [veh/min]')
    fig.supylabel('Duration of Sessions [s]')

    custom_lines = [Line2D([0], [0], color="blue", lw=2),
                    Line2D([0], [0], color="red", lw=2),
                    Line2D([0], [0], color="gray", lw=2)]

    # fig.legend(custom_lines, ['Completed', 'Aborted'],
    # ncol=2, bbox_to_anchor=(0.8, 1.0), fontsize=8)
    plt.savefig(f"pfopbarchartDURATA_MPS{m}.pdf",
                format='pdf', bbox_inches='tight')
    plt.clf()


'''


# plot data in grouped manner of bar type
ax.bar(x-width, y1, width, edgecolor="black", color="blue")
ax.bar(x, y2, width, edgecolor="black", color="red")
ax.bar(x+width, y3, width, edgecolor="black", color="gray")
ax.set_xticks(x, labels=myx)

if ylabel:
    ax.set_ylabel("$D_{max}="+str(mad)+"$")

if xlabel:
    ax.set_title(f"$R={pr}$")

yticks = list(range(0, 16*10**3, 2*10**3))
ylabels = map(lambda x: str(int(x/1000))+'k', yticks)
ax.set_yticks(yticks, ylabels)
'''

'''
bpred = dict(linestyle='-', linewidth=1, facecolor="red")
bpblue = dict(linestyle='-', linewidth=1, facecolor="blue")
flierprops = dict(marker='o', markerfacecolor='green', markersize=12,
                  markeredgecolor='none')
medianprops = dict(linestyle='-.', linewidth=2.5, color='firebrick')
meanpointprops = dict(marker='D', markeredgecolor='black',
                      markerfacecolor='firebrick')
meanlineprops = dict(linestyle='--', linewidth=2.5, color='purple')




'''
