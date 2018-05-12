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

The doxygen documentation of the project can be found at this [link](http://pluca.web.cern.ch/pluca/doxygen/annotated.html)

n.b. In order to access `easyanalysis` in python via `ROOT.gSystem`, `LD_LIBRARY_PATH` must include the directory where the `.pcm`, `.rootmap` and `.so` files are located (as done with `source scripts/setup`)

## CMake

To build the static and shared libraries
```bash
mkdir build
cd build
cmake ..
make -j8
```

One should be able to include `easyanalysis` in a cmake project by simply adding 
```bash
add_subdirectories(tools)
```
to the main project

## Make (DEPRECATED, ONLY WORKING ON SLC6-GCC49)

To setup the proper environment
```bash
source scripts/setup.sh old (or setup.csh if you use tcsh)
```

To build the static libraries
```bash
make
```

To build the shared libraries to be loaded in python
```bash
make shared
```
