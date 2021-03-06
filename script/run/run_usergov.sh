#!/bin/bash

help() {

    echo "Usage: $0 [gov name] [cnn name]"
}

if [ "$#" -lt 1 ]; then
    help
    exit
fi

echo "$0: \$1=$1"

GOV_NAME=$1
CNN_LIST=( "lenet-batch100" "cifar10-batch100" "alexnet-batch50" "vggnet16-batch25" "vggnet19-batch25" "googlenet-batch50" "resnet50-batch25" "resnet101-batch10" "resnet152-batch10" )


if [ "$#" -lt 2 ]; then
    echo "$0: You did not give target CNN name. Running all CNNs in the list"
    for CNN_NAME in "${CNN_LIST[@]}"; do
        sudo ./power_measurement -g $GOV_NAME -f exp_result/$GOV_NAME/$CNN_NAME-$GOV_NAME.txt ./script/run/run_$CNN_NAME.sh
    done
else
    echo "$0: \$2=$2"

    CNN_NAME="NOT FOUND"
    IFS='-'
    for CNN_NAME_WITH_BATCHSIZE in "${CNN_LIST[@]}"; do
        CNN_NAME_TOKENS=( $CNN_NAME_WITH_BATCHSIZE )
        CNN_SHORT_NAME=${CNN_NAME_TOKENS[0]}
        if [ $CNN_SHORT_NAME == $2 ]; then
            CNN_NAME=$CNN_NAME_WITH_BATCHSIZE
            break
        fi
    done

    IFS=' '
    echo "$0: Target CNN: $CNN_NAME"
    sudo ./power_measurement -g $GOV_NAME -f exp_result/$GOV_NAME/$CNN_NAME-$GOV_NAME.txt ./script/run/run_$CNN_NAME.sh
fi
