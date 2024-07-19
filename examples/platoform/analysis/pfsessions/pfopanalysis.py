import pandas as pd
import numpy as np
import sys
import multiprocessing
multiprocessing.set_start_method('spawn', True)
import concurrent.futures
from concurrent.futures import ProcessPoolExecutor
from itertools import product
from progressbar import progressbar
import os
import code  # code.interact(local=dict(globals(), **locals()))
from math import isclose

NCORES = os.cpu_count()
# NCORES = 1

INVALID_SESSION = 0
pfops2string = dict(enumerate([
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
pfops2string
PFOPS = {v:k for k,v in pfops2string.items()}

errorval = tuple((float('nan') for i in range(3)))
def processVehSessions(vdf, veh, params):
    vehsessiontable = []
    vr, mps, mad, pr, rep = params
    #All vesh must have reached the BARRIER_SWITCHOFF
    if vdf.iloc[-1].pfoperation != PFOPS["BARRIERSWITCHOFF"]:
        #print(f"Veh({veh}) SWITCHOFF is not the last log barrier.")
        return None
    
    # Removing swithchoff logs
    vehEndTime = vdf.iloc[-1].time
    vdf = vdf[vdf.time < vehEndTime]

    # Now process all sessions of this veh
    for sid in vdf.pfsessionId.unique():
        sdf = vdf[vdf.pfsessionId==sid]
        if len(sdf) != 2:
            print(f"Wrong LEN with session {sid}:\n{sdf}")
            code.interact(local=dict(globals(), **locals()))
            return params + errorval + ("PROC ERROR",)
        if (sdf.pfoperation.iloc[0] != PFOPS["REQUEST_SENT"]):
            print(f"Wrong start REQ with session: {sid}:\n{sdf}")
            return params + errorval + ("SID NOT STARTING WITH REQ",)
        duration = sdf.time.iloc[1] - sdf.time.iloc[0]
        duration2 = sdf.iloc[1].pfsession_endT - sdf.iloc[1].pfsession_startT
        if not isclose(duration, duration2, abs_tol=0.1):
            return params + errorval + ("SID WRONG DURATION",)
        if (duration <= 0):
            #print(f"Wrong DURATION withsession: {sid}:\n{sdf}")
            #exit()
            return params + errorval + ("SID WITH NEGATIVE DURATION",)
        endPos = sdf.pfsession_endX.iloc[1]
        pfop = pfops2string[sdf.pfoperation.iloc[1]]
        endTime = sdf.pfsession_endT.iloc[1]
        record = params + (veh, sid) + (duration, endPos, endTime, pfop)
        vehsessiontable.append(record)
    return vehsessiontable


def process_params(args):
    f, vr, mps, mad, pr, rep = args
    data = pd.read_parquet(f, filters=[('vr', '==', vr), ('mps', '==', mps), (
        'mad', '==', mad), ('pr', '==', pr), ('rep', '==', rep),
         ('pfsessionId', '!=', INVALID_SESSION)])
    if (len(data) == 0):
        print(f"{args} provided 0 records! Was with collision?")
    data['pfsessionId'] = data['pfsessionId'].astype(np.uint32)
    data['pfoperation'] = data['pfoperation'].astype(np.uint32)
    data = data.sort_values('time')
    #print(f"Processing (file, vr, mps, mad, pr, rep) = ({args})...")
    #print(data.columns)

    r = vr, mps, mad, pr, rep
    table = []
    for veh in data.pfvehSessId.unique():
        vehtable = processVehSessions(data[data.pfvehSessId==veh], veh, r)
        if vehtable:
            table += vehtable
    out = pd.DataFrame(table, columns=['vr', 'mps', 'mad', 'pr', 'rep', 'reqid', 'sid', 'duration', 'endPosX', 'time', 'pfop'])
    #print(f"\tprovided {len(out)} results")
    return out


#############################
#  MAIN
#############################

def main():
    # file = sys.argv[1]
    file = '../data/pfoperation.parquet'
    #mps = int(sys.argv[2])
    #pr = float(sys.argv[3])

    # if mps not in [8,25]:
    #    exit("Wrong mps")

    # if pr not in [0.25, 0.5, 0.75, 1.0]:
    #    exit("Wrong pr")

    #vrs = [5, 10, 15, 20, 25, 30]
    #mps = [6, 8, 10, 25]
    #mads = [50, 100, 150, 200]
    #pr = [0.25, 0.5, 0.75, 1.0]

    vrs = [5, 15, 25, 30]
    mps = [8]
    mads = [50, 200]
    pr = [0.25, 1.0]
    rep = list(range(5))

    params = list(product(vrs, mps, mads, pr, rep))
    # params = [(30, 25, 200, 1.0, 1)]
    params = [(file,)+t for t in params]


    if NCORES > 1:
        with ProcessPoolExecutor(max_workers=int(NCORES)) as executor:
            data = list(progressbar(executor.map(process_params, params),
                                redirect_stdout=True, max_value=len(params)))
    else:
        data = []
        for exp in params:
            record = process_params(exp)
            data.append(record)

    print("Creating summary table...")

    df = pd.concat(data)

    outfile = f"pfopsummary.parquet"
    df.to_parquet(outfile, index=False)
    print(f"{outfile} written!")
    #executor.shutdown()

if __name__ == '__main__':
    main()


