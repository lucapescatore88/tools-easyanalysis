Easy analysis
=============

To setup on lxplus you can go into the main forlder and do

```bash
source setup.sh
make
```

The setup.sh script launches a ``SetupProject root" 
and defines the TOOLSSYS environment variable pointing to the
folder containing the tools.

To use the tools you can add the headers (mainly analyser.hpp) 
to your cpp files.

Below is attached a `Makefile` I use for my analysis.
To use it, the tools can be anywhere, but the TOOLSSYS 
variable needs to be defined with the full path to the tools folder.

Structure:
myanalysis
-- src

The `src` subfolder contains independent `cpp` files for each executable;
each with a main. If you defined the TOOLSSYS variable the makefile 
will find the tools and make them automatically.
If you don't define it, the Makefile it will assume
that the tools are in the same folder.

```makefile
ROOTCFLAGS = $(shell root-config --cflags --glibs)

TOOLSDIR   = $(PWD)
ifneq ($(TOOLSSYS),)
    TOOLSDIR   = $(TOOLSSYS) 
endif

## This is the folder where you have the tools
## If not set it will assume they are in the same folder.

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
	echo "$(CXX) $(CXXFLAGS) $^ -o $@"; \
	$(CXX) $(CXXFLAGS) $^ -o $@; \

tmp:
	@rm -rf tmp.root
	@rm -rf /tmp/$(USER)/tmp.root
	@ln -sf /tmp/$(USER)/tmp.root tmp.root

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
```
