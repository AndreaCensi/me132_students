# CMake Boilerplate
cmake_minimum_required(VERSION 2.8)
 
 
# Include the Player macros for CMake
# Assumes that the environment variable "player_install" 
# points to a player installation.

set(player_install $ENV{player_install})
if(player_install)
    message(STATUS "Using player_install = ${player_install} from environment variable.")
else(player_install)
    set(player_install /usr/local)    
    message(STATUS "Env. var. 'player_install' not found, defaulting to ${player_install}.")
endif(player_install)

# Include the Player macros for CMake
set(CMAKE_MODULE_PATH ${player_install}/share/cmake/Modules)
INCLUDE (UsePlayerC++)


# Create the "my_client" program 
# from basic_client.cc and my_functions.cc

PLAYER_ADD_PLAYERCPP_CLIENT(my_cpp_client 
    SOURCES src/basic_client.cc src/cmdline_parsing.cc src/my_functions.cc
    LIBDIRS ""
    LIBFLAGS ""
    CFLAGS  ""
    INCLUDEDIRS ""
    )
