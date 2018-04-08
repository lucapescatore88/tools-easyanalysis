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

A doxygen is available at http://pluca.web.cern.ch/pluca/doxygen/annotated.html

n.b. In order to access `easyanalysis` in python via `ROOT.gSystem`, `LD_LIBRARY_PATH` must include the directory where the `.pcm`, `.rootmap` and `.so` files are located (as done with `source scripts/setup`)

## Make

To build the static libraries
```bash
make
```

To build the shared libraries to be loaded in python
```bash
make shared
```

## CMake

To build the static and shared libraries
```bash
mkdir build
cd build
cmake ..
make -j4
```

One should be able to include `easyanalysis` in a cmake project by simply adding 
```bash
add_subdirectories(tools)
```
to the main project
