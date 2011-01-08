Instructions for compiling and running the code templates
---------------------------------------------------------

This directory contains code templates and data to get you 
started writing clients for Player simulations.

Inside this directory contains:

    template_cpp/       Client in C++
    template_python/    Client in Python.
    stage_data/         Worlds and robot definitions for simulations.

Albeit we give examples in C++ or Python, your polyglot TAs will
accept work done in any other language that has an interface to Player.
(between you and me, the smart choice is Python)
    
These instructions are guaranteed to work on Tokyo, and should work
with minimal modifications in other Linux systems.  


Now, decide whether you want to use Python or C++ and go read 
the READMEs in those directories. Then, read the next section
for running the examples.


Running the examples
--------------------

Here we assume that you have successfully compiled/installed your
C++/Python client, so that you have an executable "my_cpp_client" 
or "my_python_client".

To run the examples, you have to first start the stage simulator,
then your client. It helps if you use two different terminals.

In the first terminal, go to the "worlds" directory. Now we need
to load up a simulated world in Stage. We can do that by typing
the following command

    $ player simple.cfg -p $MY_PORT_NUMBER
    
You should see a single blue robot in the middle of the world and 
several other robots at the bottom of the screen.

Leave that open, and in another window write:

    $ my_cpp_client -p $MY_PORT_NUMBER

Now you should be seeing the robot going around in circles.
