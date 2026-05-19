#!/bin/bash

CONFIG_FILE="config.env"
if [[ -f "$CONFIG_FILE" ]]; then
    echo "Load values from $CONFIG_FILE"
    source "$CONFIG_FILE"
else
    echo "No $CONFIG_FILE found"
    exit 1
fi

MPI_ARGS=(
    -n $NODES
)

EXECUTION_ARGS=(
    -i $DATASET
    -s $SOLUTION
    -o $OUTPUT_FILE
    -k $NUM_CLUSTERS
    -l $MAX_LOAD
    -p $PENALIZATION
)

mpiexec.hydra "${MPI_ARGS[@]}" ./LogisticDelivery "${EXECUTION_ARGS[@]}"