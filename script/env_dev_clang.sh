#!/bin/bash

export CC=clang
export CXX=clang++

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source ${SCRIPT_DIR}/env_dev.sh "$@"
