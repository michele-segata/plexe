#!/usr/bin/env bash
RESDIR=../../results
PFDIR=$PLEXE_DIR/examples/platoform
ANALYSISDIR=$PFDIR/analysis/simplecases

CWD=$(pwd)

echo "Running simple simulations, compile PLEXE if they don't run!!"

cd $PLEXE_DIR/examples/platoform

./run -u Cmdenv -c simple1 -r 0 
./run -u Cmdenv -c simple2 -r 0

echo "Analysis: Extracting data..."
cd $ANALYSISDIR
genmakefile.py parse-config > Makefile

make $RESDIR/simple1.csv
make $RESDIR/simple2.csv

echo "Plotting simplecases related speed/acc plots..."
python3 simplecases1.py ../../results/s.simple1_0.csv
python3 simplecases2.py ../../results/s.simple2_0.csv

cd $CWD
