#!/bin/bash

export CC=clang-12
export CXX=clang++-12

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source ${SCRIPT_DIR}/env_dev.sh "$@"
