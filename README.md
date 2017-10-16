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

## This is the folder where you have the tools
## If not set it will assume they are in the same folder.

TOOLSDIR   = $(PWD)
ifneq ($(TOOLSSYS),)
    TOOLSDIR   = $(TOOLSSYS) 
endif

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


Include the package as a cmake project:
In the top dir of your "new" CMakeProject you want to git clone first the easyanalysis project

You have to add these lines to the CMakeLists.txt in the top directory.
```cmake
cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
project(YOURPROJECT_NAME)

#Load the root configs to compile 
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
#find the root package and add the libraries you need
find_package(ROOT REQUIRED Cling TreePlayer Tree Rint MathMore Postscript Matrix RIO Core Foam RooStats RooFit RooFitCore Gpad Graf3d Graf Hist Net TMVA  XMLIO MLP)
include(${ROOT_USE_FILE})
#Define the PROJECT_SYS folder for the easytools 
set(PROJECT_SYS ${CMAKE_CURRENT_SOURCE_DIR}/tools-easyanalysis)
#Where to look for the ROOT linkers for tools and roofit package in tools-easyanalysis
set(LINKDEF_TOOLS ${PROJECT_SYS}/LinkDef.h)
set(LINKDEF_ROOFIT ${PROJECT_SYS}/Roofit_LinkDef.h)
set(EASYTOOLS_ROOFITPATH ${PROJECT_SYS}/roofit)
set(EASYTOOLS_TOOLSPATH ${PROJECT_SYS}/tools)

#In which path you want to produce the libraries 
set(LIBRARY_OUTPUT_PATH ${PROJECT_SYS}/lib)

message("EasyTools PROJECT_SYS is set to" ${PROJECT_SYS})
include_directories(${PROJECT_SYS} ${ROOT_INCLUDE_DIRS})
add_definitions(${ROOT_CXX_FLAGS})

#name of the tools librariaries , the compiler adds lib prefix automatically 
set(TOOLS_LIB_NAME tools)
set(ROOFIT_LIB_NAME roofittools)

add_subdirectory(${EASYTOOLS_ROOFITPATH})
add_subdirectory(${EASYTOOLS_TOOLSPATH})

add_subdirectory(OTHERMODULES YOU WRITE)

#In the OTHERMODULES path if you need to use the tools and roofit you simply need for a given target to add the following line:
add_executable(YOUREXECUTABLE Targets/YOUREXECUTABLE.cxx)
target_link_libraries(YOUREXECUTABLE PUBLIC ${TOOLS_LIB_NAME})
#If the OTHERMODULES is a library to be produced:
file(GLOB SOURCESOFLIBRARY) "path_tosourcesnewlib/*.cpp path_toheadersnewlib/*.hpp"
add_library(YOURNEWLIBRARIESNAME SHARED ${SOURCESOFLIBRARY})
target_include_directories(YOURNEWLIBRARIESNAME PUBLIC path_toheadersnewlib)
target_link_libraries(YOURNEWLIBRARIESNAME PUBLIC ${TOOLS_LIB_NAME})
```
