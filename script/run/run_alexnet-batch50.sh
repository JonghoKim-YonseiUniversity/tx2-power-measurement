#!/bin/bash
export CAFFE_ROOT=$HOME/caffe
cd $CAFFE_ROOT

NETWORK_PATH=$CAFFE_ROOT/models/bvlc_alexnet
PROTO=$NETWORK_PATH/batch50_train_val.prototxt
WEIGHTS=$NETWORK_PATH/bvlc_alexnet.caffemodel

COMMAND="$CAFFE_ROOT/build/tools/caffe test -model $PROTO  -weights $WEIGHTS -gpu all"

echo "$0: $COMMAND"
$COMMAND
