import pandas as pd
import numpy as np

START=0
ROADLENGTH = 11000
GATES_STEP = 100
MINOBSERVEDVEHS = 1000
GRACEPERIOD = 1
MAXSIMTIME = 24000

gates = list(range(START+100, START+ROADLENGTH+101, GATES_STEP))

def process_exp(df, gates, exp):
    v2boolmap = {}
    valid = 0
    maxtime = df.time.max()

    for v in df.pfvehId.unique():
        vdf = df[df.pfvehId == v]

        boolmap = []

        maxReachedDistance = vdf.platoVarX.max()
        endTimeVeh = vdf.time.max()

        if maxReachedDistance < START+ROADLENGTH:
            continue  # dont consider vehs taht did not travel for at least ROADLENGTH
        else:
            valid += 1

        # discard data after CoolDown dist = ROADLENGHT
        vdf = vdf[vdf.platoVarX <= START+ROADLENGTH]
        startPlatooningPos = vdf[vdf.platoonSize > 1].platoVarX.min()

        for x in gates:
            if pd.isna(startPlatooningPos):
                boolmap.append(0)
            else:
                boolmap.append(0 if x < startPlatooningPos else 1)
        v2boolmap[v] = boolmap

    if (valid < 0.98*MINOBSERVEDVEHS):
        print(f"{exp} did not provide enough valid records... was a crash? (valid={valid})")
        return np.array([float("nan") for v in gates])
    # else:
    #    print(f"\t {valid} num of veh that reached posx={ROADLENGTH}")

    dfr = pd.DataFrame(v2boolmap)
    numPlatoVehAtEachVarco = dfr.sum(axis=1).to_numpy()
    ratioPlatoVehAtEachVarco = numPlatoVehAtEachVarco / valid
    return ratioPlatoVehAtEachVarco
