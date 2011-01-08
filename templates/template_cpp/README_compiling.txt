Instructions for compiling the C++ client code template
--------------------------------------------------------

We provide two options:

* Using CMake. This is the preferred way. It might seem overkill
  for a simple program like this, but eventually, for any nontrivial 
  programming, you *really* need to use this.

* Using a shell script. I know, the homework is due tomorrow morning,
  it's late night, you haven't slept in 28 hours, and you can't
  install CMake for some reason.
 

Note that both options require that your environment is correctly setup.
In particular, you need the environment variable "player_install" to 
point to the player installation. (On Tokyo it should be "/usr/local")


Using CMake
^^^^^^^^^^^

The sequence of commands is as follows:

1) Read the CMakeLists.txt and create a Makefile:

       $ cmake .    

2) Compile the program.

       $ make
    


Using a script
^^^^^^^^^^^^^^

The commands are in the file "compile_using_cmake.sh". 
Use it like this:

    $ source compile_using_cmake.sh
    
(see further comments inside that file)





