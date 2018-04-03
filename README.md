**Status ·** [![build status](https://gitlab.cern.ch/LHCb-RD/tools-easyanalysis/badges/master/build.svg)](https://gitlab.cern.ch/LHCb-RD/tools-easyanalysis/pipelines)

# Easyanalysis

To checkout the project
```bash
git clone ssh://git@gitlab.cern.ch:7999/LHCb-RD/tools-easyanalysis.git
```

To setup the proper environment
```bash
source scripts/setup.sh (or setup.csh if you use tcsh)
```

To build the static libraries
```bash
make
```

To build the shared libraries to be loaded in python
```bash
make shared
```

A doxygen is available at http://pluca.web.cern.ch/pluca/doxygen/annotated.html

#Easyanalysis cmake
To be able to generate the libraries and load them in python when compiling using CMake:
```bash
source scripts/setup.sh
```
Compile the code and produce the (single) library of easyanalysis (both roofit and tools in one go)
```bash
mkdir build #create a location where to build the code
cd build 
make -j4
```
To be able to load everything in python through ```ROOT.gSystem```, be sure ```LD_LIBRARY_PATH``` includes the directory where the ```.pcm,.rootmap and .so``` files are created, 
By default they are produced in ```build/```.
In principle, you should be able to include the easyanalysis to a cmake project simply by adding 
```add_subdirectories(tools)```
in your main project.

