Instructions for using the Python client code template
--------------------------------------------------------

As usual, there are several options.


Zero-installation
-----------------

If you are in rush, it should be possible to run every python
script directly as an argument to the "python" command, like so:


    $ python src/me132_template/basic_client.py -p 6666


However, this might not work if you want to organize your
Python projects as a hierarchy of packages.


Simple installation
-------------------

If you are working on your own machine, the best way to install
your Python program is to use a "setup.py" file, which calls
a library called "setuptools", which makes all the hard work for you.

The included setup.py is completely functional and can be used as follows:

    $ sudo python setup.py develop

This should install a binary called "my_python_client".

("develop" instead of "install" allows you to edit the sources, and the
changes to be instantaneously propagated to the installed version. 
If you use "install", you have to run "python setup.py install" after
every change. )


Installation on a machine where you don't have permissions
----------------------------------------------------------

The above command only works if you have super user permissions
on the machine, because it installs your program globally.

One way to get around that problem is to create a "virtual environment"
which is like a local installation of python, just for you, where
you can install your program.


One sequence of commands could be:

    $ virtualenv  my_environment
    $ source my_environment/bin/activate
    $ python setup.py develop             # installs in my_environment

The first command creates the virtual environment, the other
sets up PATH and other environment variables such that when
you write "python" you will use your local copy.
    
    




