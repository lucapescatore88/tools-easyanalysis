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