#!/bin/bash

if [ -z "${CC}" ]; then
    export CC=gcc
fi

if [ -z "${CXX}" ]; then
    export CXX=g++
fi

export BUILD_TYPE=Release

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source ${SCRIPT_DIR}/env_dev.sh "$@"
