import matplotlib.pyplot as plt
import pandas as pd
import code  # code.interact(local=dict(globals(), **locals()))
import numpy as np
import seaborn as sns
import sys
from itertools import product, chain
from matplotlib.lines import Line2D

sns.set_theme(style="ticks", palette="pastel")
plt.rcParams['text.usetex'] = True
plt.rcParams['axes.grid'] = True
plt.rc('font', **{'family': 'serif', 'serif': ['Computer Modern']})

w = 3.28*1.5  # 3.28 should be the column-width of an a4paper 2columns paper in inches
#plt.figure(figsize=(w, w*9/16*2))

colors = ['blue', 'red', 'gray']  # buone, cattive, negate

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


def pfopbarchart(data, exp, ax, ylabel=False, xlabel=False):
    mps, mad, pr = exp
    df = extractExpDf(data, exp)
    ok = df[df.pfop == "PF_COMPLETED"]
    aborted = df[df.pfop.isin(ABORTED)]
    denied = df[df.pfop.isin(DENIED)]

    # create data
    myx = [5, 15, 25, 30]
    x = np.array(range(len(myx)))


    y1, y2, y3 = [],[],[]
    for vr in sorted(myx):
        y1.append(ok[ok.vr==vr].groupby(["vr", "rep"])["sid"].count().mean())
        y2.append(aborted[aborted.vr==vr].groupby(["vr", "rep"])["sid"].count().mean())
        y3.append(denied[denied.vr==vr].groupby(["vr", "rep"])["sid"].count().mean())

    #code.interact(local=dict(globals(), **locals()))
    width = 0.3

    # plot data in grouped manner of bar type
    ax.bar(x-width, y1, width, edgecolor="black", color="blue")
    ax.bar(x, y2, width, edgecolor="black", color="red")
    ax.bar(x+width, y3, width, edgecolor="black", color="gray")
    ax.set_xticks(x, labels=myx)

    if ylabel:
        ax.set_ylabel("$D_{max}="+str(mad)+"$")

    if xlabel:
        ax.set_title(f"$R={pr}$")

    yticks = list(range(0, 2001, 250))
    ylabels = map(lambda x: str(x), yticks)
    ax.set_yticks(yticks, ylabels)


#####################
#  MAIN
#####################
data = pd.read_parquet(sys.argv[1])

vrs = [5, 10, 15, 20, 25, 30]
mps = [8]
mads = [50, 200]
pr = [0.25, 1.0]


# Griglia mps x pr (costanti = mad), ogni elemento della griglia contiene tutti i vrs

for m in mps:
    print(f"Plotting for mps={m}...")
    #exps = list(product(mps, [mad], pr))
    fig, axs = plt.subplots(len(mads), len(pr), figsize=(
        w, w*9/16 * 1.6), sharex=True, sharey=True)
    iaxs = chain(*axs)

    for r, mad in enumerate(mads):
        for c, p in enumerate(pr):
            exp = (m, mad, p)
            print(f"\t exp={exp}")
            pfopbarchart(data, exp, next(iaxs), c == 0, r == 0)

    plt.subplots_adjust(left=0.19, bottom=0.15, right=0.98,
                        top=0.86, wspace=0.15, hspace=0.25)

    fig.supxlabel('$A_r$ [veh/min]')
    fig.supylabel('Number of Sessions')

    custom_lines = [Line2D([0], [0], color="blue", lw=2),
                    Line2D([0], [0], color="red", lw=2),
                    Line2D([0], [0], color="gray", lw=2)]

    #fig.legend(custom_lines, ['Completed', 'Aborted',
    #                          'Denied'], ncol=3, bbox_to_anchor=(0.8, 1.0), fontsize=8)

    plt.savefig(f"pfopbarchart_MPS{m}.pdf", format='pdf',
     bbox_inches='tight')
    plt.clf()
