Easy analysis
=============

To make the tools on lxplus you can use

Setup on lxplus :
```bash
SetupLHCb v37r1 //(you can try the latest but is a gamble)
make
```

All here, simple. Then to use the tools you can
add the headers (mainly analyser.hpp) to your cpp files.

Here below is attached the `Makefile` I use for my analysis.
This is in a folder with a `src` subfolder containing `cpp` and `hpp` files.
You also need to set a `TOOLDIR` ambient variables pointing to
this tools directory. The makefile with make the tools automatically
and all the `.cpp` executables in the src folder.


```makefile
ROOTCFLAGS = $(shell root-config --cflags --glibs)

TOOLSDIR   = $(TOOLDIR) 
## This is the folder where you have the tools
## You need to set the ambient variable by yourself

#NBDIR      = $(NEUROBAYES)
#NBLIBS     = $(NBDIR)/lib
#NBFLAGS    = -I$(NBDIR)/include -L$(NBLIBS) -lNeuroBayesTeacherCPP -lNeuroBayes

#TMVADIR    = $(ROOTSYS)/tmva/test
#TMVAFLAGS  = -I$(TMVADIR)

BDIR       = /afs/cern.ch/sw/lcg/external/Boost/1.55.0_python2.7/x86_64-slc6-gcc47-opt
BFLAGS     = -I/usr/include/python2.6 -I$(BDIR)/inlude/boost-1_55/boost -L$(BDIR)/lib

CXX        = g++
CXXFLAGS   = -g -fPIC -Wall -Wno-write-strings -Wunused-variable -O2 $(ROOTCFLAGS) -lTMVA -lRooFit -lRooStats -I$(TOOLSDIR) -I$(TOOLSDIR)/analysis -I$(DBDIR) -I/usr/include/boost/property_tree/detail -L$(TOOLSDIR)/lib

LIBS       = $(patsubst $(TOOLSDIR)/%.hpp, $(TOOLSDIR)/lib/%.a, $(wildcard $(TOOLSDIR)/*.hpp))

SOURCES    = $(wildcard src/*.cpp)
EXE        = $(patsubst src/%.cpp, %.out, $(SOURCES))

all: libraries exe tmp

exe: $(EXE)

libraries:
	$(MAKE) -C $(TOOLSDIR)

%.out: src/%.cpp $(LIBS)
	@echo
	@echo "Making $(@) ... "
	@if [ "$(@)" == "trainNB.out" ]; then \
		echo "$(CXX) $(CXXFLAGS) $(NBFLAGS) $^ -o $@"; \
		$(CXX) $(CXXFLAGS) $(NBFLAGS) $^ -o $@; \
	elif [ "$(@)" == "trainTMVA.out" ]; then \
		echo "$(CXX) $(CXXFLAGS) $(TMVAFLAGS) $^ -o $@"; \
		$(CXX) $(CXXFLAGS) $(TMVAFLAGS) $^ -o $@; \
	elif [ "$(@)" == "apply.out" ]; then \
		echo "$(CXX) $(CXXFLAGS) $(NBFLAGS) $(TMVAFLAGS) $^ -o $@"; \
		$(CXX) $(CXXFLAGS) $(NBFLAGS) $(TMVAFLAGS) $^ -o $@; \
	else \
		echo "$(CXX) $(CXXFLAGS) $^ -o $@"; \
		$(CXX) $(CXXFLAGS) $^ -o $@; \
	fi

tmp:
	@rm -rf tmp.root
	@rm -rf /tmp/$(USER)/tmp.root
	@ln -sf /tmp/$(USER)/tmp.root tmp.root

weights:
	@echo "tmva"
	@rm -rf weights
	@mkdir -p /tmp/$(USER)/weights
	@ln -sf /tmp/$(USER)/weights weights

shared: 
	@echo
	@echo "Making $(@) ... "
	$(CXX) $(CXXFLAGS) $(BFLAGS) -c -Isrc src/shared/shared_functions.cpp -o src/shared/shared_functions.o
	$(CXX) -shared -Wl,-soname,shared_functions.so -L/usr/local/lib -lboost_python -fpic src/shared/shared_functions.o -o src/shared/shared_functions.so
	@ln -sf ../src/shared/shared_functions.so python/.

print:
	@echo
	@echo "Sources"
	@echo $(SOURCES)
	@echo
	@echo "Libraries"
	@echo $(LIBS)
	@echo
	@echo "Flags"
	@echo "CXX:  " $(CXXFLAGS)
	@echo "NB:   " $(NBFLAGS)
	@echo

clean:
	@echo "Cleaning ..."
	@rm -f *.out tmp.root weights  
	@rm -rf src/shared/*.o src/shared/*.so python/*.so

cleanall: clean
	@$(MAKE) -C $(TOOLSDIR) veryclean

purge:
	@echo "Purging ..."
	@rm -f *~ */*~ */*/*~ *.pdf *.root *.nb *.txt *.html
	@rm -f tables/*.txt tables/*.aux tables/*.log

```
