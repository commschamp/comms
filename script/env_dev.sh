#!/bin/bash

if [ -z "${CC}" -o -z "${CXX}" ]; then
    echo "ERROR: Compilers are not provided"
    exit 1
fi

if [ -z "${BUILD_TYPE}" ]; then
    export BUILD_TYPE=Debug
fi

if [ -z "${SANITIZERS}" ]; then
    export SANITIZERS="ON"
fi

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR=$( dirname ${SCRIPT_DIR} )
BUILD_DIR="${ROOT_DIR}/build.${CC}.${BUILD_TYPE}"
mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

cmake .. -DCMAKE_INSTALL_PREFIX=install -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DCC_COMMS_USE_CCACHE=ON \
    -DCC_COMMS_BUILD_UNIT_TESTS=ON  -DCC_COMMS_UNIT_TESTS_USE_SANITIZERS=${SANITIZERS} "$@"
