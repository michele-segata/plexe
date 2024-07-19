#!/usr/bin/env bash

echo "PF GATES MPS=8 MAD=50 PR=1"
python3 pfgates.py ../data/platoform.parquet 8 50 1

echo "PF GATES MPS=8 MAD=200 PR=1"
python3 pfgates.py ../data/platoform.parquet 8 200 1

echo "PF GATES MPS=8 MAD=200 VR=15"
python3 pfgates_4vr.py ../data/platoform.parquet 8 200 15