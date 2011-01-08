# CMake Boilerplate
cmake_minimum_required(VERSION 2.8)
 
 
# Include the Player macros for CMake
# Assumes that the environment variable "player_install" 
# points to a player installation.

SET(CMAKE_MODULE_PATH $ENV{player_install}/share/cmake/Modules)
INCLUDE (UsePlayerC++)


# Create the "my_client" program 
# from basic_client.cc and my_functions.cc

PLAYER_ADD_PLAYERCPP_CLIENT(my_client 
    SOURCES basic_client.cc cmdline_parsing.cc my_functions.cc
    LIBDIRS ""
    LIBFLAGS ""
    CFLAGS  ""
    INCLUDEDIRS ""
    )
