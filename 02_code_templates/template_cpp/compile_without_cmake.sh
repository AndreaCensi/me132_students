#!/bin/bash
set -e # stop on errors (always include this in your scripts)
set -x # echo commands 


sources="src/my_functions.cc src/cmdline_parsing.cc src/basic_client.cc"

# Note this assumes that PKG_CONFIG_PATH is setup correctly
libraries=`pkg-config --cflags --libs playerc++`

g++  ${libraries} ${sources} -o my_cpp_client
