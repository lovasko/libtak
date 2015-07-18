#!/bin/sh

INC_DIR=/usr/include
LIB_DIR=/usr/lib

cp -v bin/libtak.so "${LIB_DIR}"
cp -v src/tak.h "${INC_DIR}"

