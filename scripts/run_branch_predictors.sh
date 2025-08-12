#!/bin/bash

## Execute this script in the workspace directory.
## Example of usage: ./run_branch_predictors.sh blackscholes rtview
## Use the executable file name in place of the benchmark name (rtview instead of raytrace)
## Modify the following paths appropriately
## CAUTION: use only absolute paths below!!!

# CHANGE THIS PATH TO YOUR COMPILED PIN TOOL
PIN_TOOL=/home/panos/architecture2/ex2/pintool/obj-intel64/branch.so

# CHANGE THESE ONLY IF YOU HAVE YOUR OWN INSTALLATION
PARSEC_PATH=/home/panos/architecture2/parsec-3.0
PIN_EXE=/home/panos/architecture2/pin-external-3.31-98869-gfa6f126a8-gcc-linux/pin

# These are relative paths (must be in the workpace dir)
outDir="./branchPredOutputs"
CMDS_FILE=/home/panos/architecture2/ex2/parsec-3.0/cmds_simlarge.txt


export LD_LIBRARY_PATH=/home/panos/architecture2/ex2/parsec-3.0/pkgs/libs/hooks/inst/amd64-linux.gcc/lib

mkdir -p $outDir

for BENCH in $@; do

	cmd=$(cat ${CMDS_FILE} | grep "$BENCH")

	pinOutFile="$outDir/${BENCH}.bp.out"
	pin_cmd="$PIN_EXE -t $PIN_TOOL -o $pinOutFile -- $cmd"
	echo "Running: $pin_cmd"
	/bin/bash -c "time $pin_cmd"
	
done
