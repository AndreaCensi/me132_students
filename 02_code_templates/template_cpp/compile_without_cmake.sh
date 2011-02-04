#!/bin/bash
set -e # stop on errors (always include this in your scripts)
set -x # echo commands 


# Note this assumes that PKG_CONFIG_PATH is setup correctly
libraries=`pkg-config --cflags --libs playerc++`

common="src/common_functions.cc src/cmdline_parsing.cc"

sources="${common} src/me132_tutorial_0.cc"
g++  ${libraries} ${sources} -o me132_tutorial_0


sources="${common} src/me132_tutorial_1.cc"
g++  ${libraries} ${sources} -o me132_tutorial_1
