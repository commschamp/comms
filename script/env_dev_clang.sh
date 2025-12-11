#!/bin/bash

if [ -z "${CC}" ]; then
    export CC=clang
fi

if [ -z "${CXX}" ]; then
    export CXX=clang++
fi

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source ${SCRIPT_DIR}/env_dev.sh "$@"
