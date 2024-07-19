# Plots for VNC 2024 paper

## Plots for the distribution of platoon sizes

To obtain the input file after running the simulation run
```commandline
make platoform.csv
```
Move the file `platoform.csv` into the `results` folder or change the variable `inputfolder` in `platoon-size.R`.
If the input file is a parquet, convert it to csv running `python scripts/parquet-to-csv.py <filename>`.
To generate an intermediate data file, run
```commandline
Rscript platoon-size.R
```
This script keeps bins platoon size data into observation points of 1 km, (0 to 1 km, 1 to 2 km, ..., 10 to 11 km).
To change the size/number of bins, edit the `bins` variable within the script.
This creates a file `platoon-size-distr.csv` into the same folder, which is then used by the plotting script.
In addition, it also computes the ratio of vehicles in a platoon, also binned into observation points.

To plot the discrete violin plots of the distributions, simply run `python plot-platoon-size.py`.
This will generate tons of plots.


## Simple Cases (2 Elected platoform demo-sessions)

Within the `simplecases` folder run the bash script `autoRunAndPlot.sh`

```
cd simplecases
./autoRunAndPlot.sh
```

The script will

1. Run two demo simulations
2. Extract data
3. Plot extracted data, generating 2 pdf files inside the `simplecases` folder

## Analysis of PlatoonRation (\eta) at gates across space

Within the `platoatgates` folder run the script `/figetagates.sh
`

```
cd platoatgates
./figetagates.sh
```

The script will load some data from `../data/platoform.parquet` and generate various plots showing the evolution of the PlatoonRatio (eta) over the 10km space where vehicles are allowed to run the Platoform protocol.


## Analysis of Platoforom Sessions

Raw Data about platoform sessions across various different simulations have been collected
in the file `../data/pfoperation.parquet`.

To process this data to compute, e.g., the duration of such sessions, run the `pfopanalysis.py` script:
```
python3 pfopanalysis.py ../data/pfoperation.parquet
```

This will generate `pfopsummary.parquet` that should be provided as input to plotting scripts. For example, run the followings:

```
python3 plotpfop.py pfopsummary.parquet
python3 plotpfopDURATION.py pfopsummary.parquet
```

to generate boxplots about the number and the duration of successful/aborted platoform sessions.