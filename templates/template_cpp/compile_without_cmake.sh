#!/bin/bash
set -e # stop on errors (always include this in your scripts)
set -x # echo commands 


sources="my_functions.cc cmdline_parsing.cc basic_client.cc"

# Note this assumes that PKG_CONFIG_PATH is setup correctly
libraries=`pkg-config --cflags --libs playerc++`

g++  ${libraries} ${sources} -o my_client
