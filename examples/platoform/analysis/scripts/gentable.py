import sys
from itertools import product
from tabulate import tabulate
import code  # code.interact(local=dict(globals(), **locals()))
from pfutil import *


file = sys.argv[1]

varco = varchi.index(9900)  # al varco 9900m


# Tabella per vr=5
print("Tabella per Ar = 5")
mps = 8
prs = [0.25, 0.5, 0.75, 1.0]
mads = [50, 100, 150, 200]
vr = 5

table = []
for pr in prs:
    for mad in mads:
        exp = (mps, vr, mad, pr)
        print(f"Evaluating exp (mps, vr, mad, pr) = {exp}")
        df = pd.read_parquet(file, filters=[
                             ('mps', '==', mps), ('vr', '==', vr), ('mad', '==', mad), ('pr', '==', pr)])
        etalist = []
        for rep in sorted(df.rep.unique()):
            etavarchi = process_exp(df[df.rep == rep], varchi, exp+(rep,))
            etalist.append(etavarchi[varco])
        record = exp + (np.nanmean(etalist),)
        table.append(record)

tab = pd.DataFrame(table, columns=["mps", "vr", "mad", "pr", "eta"])
print(tabulate(tab,  headers="keys", showindex=False))

print("\n--------------------------\n")


# Tabella per vr=30
print("Tabella per Ar = 30")
mps = 8
prs = [0.25, 0.5, 0.75, 1.0]
mads = [50, 100, 150, 200]
vr = 30

table = []
for pr in prs:
    for mad in mads:
        exp = (mps, vr, mad, pr)
        print(f"Evaluating exp (mps, vr, mad, pr) = {exp}")
        df = pd.read_parquet(file, filters=[
                             ('mps', '==', mps), ('vr', '==', vr), ('mad', '==', mad), ('pr', '==', pr)])
        etalist = []
        for rep in sorted(df.rep.unique()):
            etavarchi = process_exp(df[df.rep==rep], varchi, exp)
            etalist.append(etavarchi[varco])
        record = exp + (np.nanmean(etalist),)
        table.append(record)

tab = pd.DataFrame(table, columns=["mps", "vr", "mad", "pr", "eta"])
print(tabulate(tab,  headers="keys", showindex=False))

