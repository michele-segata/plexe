import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.lines import Line2D
from matplotlib.patches import Patch
import sys
import numpy as np
from collections import defaultdict 
from tqdm import tqdm
from pfutil import *

plt.rcParams['axes.grid'] = True
plt.rcParams['text.usetex'] = True
plt.rc('font', **{'family': 'serif', 'serif': ['Computer Modern']})

# S1 analysis
w = 3.28*1.5 #3.28 should be the column-width of an a4paper 2columns paper in inches
plt.figure(figsize=(w, w*9/16))


def extractExpDf(data, exp):
    mps, mad, pr = exp
    query = f"mps=={mps} and mad=={mad} and pr=={pr}"
    df = data.query(query)
    return df


###################
#  MAIN
###################

file = sys.argv[1]
mps = int(sys.argv[2])
mad = int(sys.argv[3])
pr = float(sys.argv[4])

vrs = [5, 10, 15, 20, 25, 30]
mpss = [6, 8, 10, 25]
mads = [50, 100, 150, 200]
prs = [0.25, 0.5, 0.75, 1.0]


if mps not in mpss:
    exit("Wrong mps")

if mad not in mads:
    exit("Wrong mad")

if pr not in prs:
    exit("Wrong pr")

exp = mps, mad, pr #8, 200m, 1

data = pd.read_parquet(file)
df = extractExpDf(data, exp)

print(f"Processing exp={exp}...")
for vr in sorted(df.vr.unique()):
    print(f"\tvr={vr}...")
    replist = []
    dfvr = df[df.vr == vr]
    for rep in sorted(dfvr.rep.unique()):
        print(f"\t\trep={rep}...")
        tmp = dfvr[dfvr.rep == rep]
        etagates = process_exp(tmp, gates, exp + (vr,))
        replist.append(etagates)
    expgatesvr = pd.DataFrame(replist)
    plt.plot(gates, expgatesvr.mean(axis=0).to_list(), label=f"$A_r = {vr}$")

xticksgates = np.array(range(START, START+ROADLENGTH+1, 1000))
lbls = map(int, xticksgates/1000.0)
#lbls = [l.replace(".0", "") if l.endswith(".0") else "" for l in lbls]
plt.xticks(xticksgates, labels = lbls)

plt.yticks(list(np.arange(0, 1.01, 0.2)))
plt.xlabel("Observation Points [km]")
plt.ylabel("$\eta$")


plt.ylim(-0.01, 1.01)
#plt.xlim(-1, 10100)

plt.legend(fontsize=7, loc ='lower right')

plt.subplots_adjust(left=0.13, bottom=0.18, right=0.98, top=0.87, wspace=0.1)
plt.savefig(f"pfgates_mps{mps}_mad{mad}_pr{int(pr)}.pdf", format='pdf', bbox_inches='tight')