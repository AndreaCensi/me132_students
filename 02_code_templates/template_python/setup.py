from setuptools import setup, find_packages

setup(
    name='me132_template',
    author="The ME132 TAs",
    author_email="me132-tas@caltech.edu",
    url='www.its.caltech.edu/~me132',

    description="A minimal Player client",
      
    version="0.1",
    
    package_dir={'':'src'},
    packages=find_packages(),
    
    entry_points={
     'console_scripts': [
        # List executables in the format '<name> = <module>:<function>'
       'my_python_client = me132_template.basic_client:main'
      ]
    }
)

