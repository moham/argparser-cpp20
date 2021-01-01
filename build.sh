#!/bin/bash

COMPILER=clang++
BASE_DIR=$(dirname $0)
BUILD_DIR=$BASE_DIR/build
SRC_DIR=$BASE_DIR/libargparser

if [ ! -d $BUILD_DIR ]; then
    mkdir -p $BUILD_DIR &>/dev/null
    if [ $? -ne 0 ]; then
        echo "Could not create build directory" >&2
        exit 1
    fi
fi

$COMPILER \
    -std=c++2a \
    -fmodules-ts \
    --precompile \
    -o $BUILD_DIR/libargparser.pcm \
    $SRC_DIR/libargparser.cpp

