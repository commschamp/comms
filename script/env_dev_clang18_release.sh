#!/bin/bash

export CC=clang-18
export CXX=clang++-18

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source ${SCRIPT_DIR}/env_dev_rel.sh "$@"
