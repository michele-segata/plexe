#!/bin/bash
#Usage: filter_runs.sh <filters>
USAGE="
ARG1 must be the configurantion Name;
Rest or args must be <filters>, i.e., pairs of itervaname,value
separated by =, e.g. repetition=1.
Passing multiple <iteration_varname=value> as filters, the script
will give you back the runfile with runs for which the chosen
iteration_variable takes the chosen value. 
Example:
filter_runs.sh NoComm repetition=1 rate=0.5
Hint: the script heavily relies on grep, put a comma afeter the value, e.g,
rate=0.5,
to be sure to match only the runs where rate=0.5 but not also those
where it may be rate=0.51 or 0.52 etc."

[ $# -eq 0 ] && { echo "$USAGE"; exit 1; }

CONFIG="$1"
FILTERS="${@:2}"

RUNS=$(./run -c $CONFIG -q runs)
for filter in $FILTERS; do
  RUNS=$(echo "$RUNS" | grep -i "$filter")
done
echo "$RUNS" | cut -d':' -f 1 | sed 's/Run //' | xargs -I {} printf ". ./run -u Cmdenv -c $CONFIG -r {}\n"

