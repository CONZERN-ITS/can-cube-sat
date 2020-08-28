#!/usr/bin/env bash

cmake -G "Unix Makefiles" -B build/ -S src/lib-strela-ms-rpi/ -DCMAKE_INSTALL_PREFIX=/home/agnus/git/can-cube-sat/src/ground/rpi/antenna-control-system/native/stage
cmake --build build/ --target install -- -j4
