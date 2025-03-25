#!/bin/bash

export CC=clang-19
export CXX=clang++-19

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source ${SCRIPT_DIR}/env_dev_rel.sh "$@"
