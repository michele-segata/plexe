import matplotlib.pyplot as plt
from matplotlib.lines import Line2D
from matplotlib.patches import Patch
import sys
from collections import defaultdict
from tqdm import tqdm
from pfutil import *

plt.rcParams['axes.grid'] = True
plt.rcParams['text.usetex'] = True
plt.rc('font', **{'family': 'serif', 'serif': ['Computer Modern']})

# S1 analysis
w = 3.28*1.5  # 3.28 should be the column-width of an a4paper 2columns paper in inches
plt.figure(figsize=(w, w*9/16))


def extractExpDf(data, exp):
    mps, mad, vr = exp
    query = f"mps=={mps} and mad=={mad} and vr=={vr}"
    df = data.query(query)
    return df


###################
#  MAIN
###################

markers = iter(['o', 's', '^', 'X', '+', '>', '<'])

lss = iter([':', '--', '-.', '-'])

greens = iter(['#4dff4d', '#00cc00', '#008000', '#004d00'])

file = sys.argv[1]
mps = int(sys.argv[2])
mad = int(sys.argv[3])
vr = int(sys.argv[4])

vrs = [5, 10, 15, 20, 25, 30]
mpss = [6, 8, 10, 25]
mads = [50, 100, 150, 200]
prs = [0.25, 0.5, 0.75, 1.0]


if mps not in mpss:
    exit("Wrong mps")

if mad not in mads:
    exit("Wrong mad")

if vr not in vrs:
    exit("Wrong vr")

exp = mps, mad, vr  # 8, 200m, 1

data = pd.read_parquet(file)
df = extractExpDf(data, exp)

print(f"Processing exp={exp}...")
for pr in sorted(df.pr.unique()):
    print(f"\tpr={pr}...")
    replist = []
    dfpr = df[df.pr == pr]
    for rep in sorted(dfpr.rep.unique()):
        print(f"\t\trep={rep}...")
        tmp = dfpr[dfpr.rep == rep]
        etagates = process_exp(tmp, gates, exp+(pr, rep))
        replist.append(etagates)
    expgatesvr = pd.DataFrame(replist)
    plt.plot(gates, expgatesvr.mean(axis=0).to_list(), label=f"$R = {pr}$", 
        ls = next(lss), color='#2ca02c', lw=1.25) #  next(greens)) #marker=next(markers)

xticksgates = np.array(range(START, START+ROADLENGTH+1, 1000))

plt.xticks(xticksgates, labels=map(int, xticksgates/1000))

plt.yticks(list(np.arange(0, 1.01, 0.2)))
plt.xlabel("Observation Points [km]")
plt.ylabel("$\eta$")

plt.ylim(-0.01, 1.01)
plt.xlim(-1, 10100)

plt.legend(fontsize=7, loc='lower right')

plt.subplots_adjust(left=0.13, bottom=0.18, right=0.98, top=0.87, wspace=0.1)
plt.savefig(f"pfgates_mps{mps}_mad{mad}_vrConst{vr}.pdf", format='pdf', bbox_inches='tight')
