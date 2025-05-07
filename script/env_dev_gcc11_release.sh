#!/bin/bash

export CC=gcc-11
export CXX=g++-11

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source ${SCRIPT_DIR}/env_dev_rel.sh "$@"
