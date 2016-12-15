ROOTCFLAGS  = $(shell root-config --cflags --glibs)
ROOTCINT    = rootcint

TOOLSDIR    = $(TOOLSSYS)

CXX         = g++
CXXFLAGS    = -std=c++0x -g -fPIC -Wall -O2 $(ROOTCFLAGS) -lTMVA -lRooFit -lRooStats -lMathMore -I$(TOOLSDIR) -I$(TOOLSDIR)/analysis -I$(TOOLSDIR)/roofit -L$(TOOLSDIR)/lib

ANALYSIS    = $(wildcard $(TOOLSDIR)/analysis/*.cpp)
ROOFIT      = $(wildcard $(TOOLSDIR)/roofit/*.cpp)
TOOLS       = $(wildcard $(TOOLSDIR)/*.cpp)

ANALYSISOBJ = $(patsubst $(TOOLSDIR)/analysis/%.cpp,   $(TOOLSDIR)/analysis/lib/%.o, $(ANALYSIS))
ROOFITDIC   = $(patsubst $(TOOLSDIR)/roofit/%.cpp,     $(TOOLSDIR)/roofit/dic/%.cpp, $(ROOFIT))
ROOFITOBJ   = $(patsubst $(TOOLSDIR)/roofit/dic/%.cpp, $(TOOLSDIR)/roofit/lib/%.o,   $(ROOFITDIC))
LIBS        = $(patsubst $(TOOLSDIR)/%.cpp,            $(TOOLSDIR)/lib/%.a,          $(TOOLS))
EXE         = $(ROOFITDIC) $(ROOFITOBJ) $(ANALYSISOBJ) $(TOOLSDIR)/lib/roofit.a $(TOOLSDIR)/lib/analysis.a $(LIBS)
#EXE         = $(ANALYSISOBJ) $(TOOLSDIR)/lib/analysis.a $(LIBS)

all: $(EXE)

$(TOOLSDIR)/roofit/dic/%.cpp: $(TOOLSDIR)/roofit/%.cpp
	@echo
	@echo "Generating dictionary $(@) ..."
	$(ROOTCINT) -l -f $@ -c -p $^

$(TOOLSDIR)/roofit/lib/%.o: $(TOOLSDIR)/roofit/dic/%.cpp
	@echo
	@echo "Making $(@) ... "
	$(CXX) $(CXXFLAGS) -c $< -o $@
	@rm -f $(TOOLSDIR)/lib/roofit.a

$(TOOLSDIR)/analysis/lib/%.o: $(TOOLSDIR)/analysis/%.cpp $(TOOLS) $(ANALYSIS)
	@echo
	@echo "Making $(@) ... "
	$(CXX) $(CXXFLAGS) -c $< -o $@
	@rm -f $(TOOLSDIR)/lib/analysis.a

$(TOOLSDIR)/lib/roofit.a: $(ROOFITOBJ)
	@echo
	ar rcs $@ $^;

$(TOOLSDIR)/lib/analysis.a: $(ANALYSISOBJ)
	@echo
	ar rcs $@ $^;

$(TOOLSDIR)/lib/%.a: $(TOOLSDIR)/%.cpp $(TOOLS)
	@echo
	@echo "Making $(@) ... "
	$(CXX) $(CXXFLAGS) -c $< -o $@

print:
	@echo
	@echo "Sources"
	@echo $(ANALYSIS)
	@echo $(ROOFIT)
	@echo $(TOOLS)
	@echo
	@echo "Dictionaries"
	@echo $(ROOFITDIC)
	@echo
	@echo "Libraries"
	@echo $(ANALYSISOBJ)
	@echo $(ROOFITOBJ)
	@echo $(LIBS)
	@echo $(EXE)
	@echo
	@echo "Flags"
	@echo "CXX:  " $(CXXFLAGS)
	@echo

clean:
	@echo "Cleaning ..."
	@rm -f $(TOOLSDIR)/lib/*.a

cleanall:
	@echo "Cleaning ..."
	@rm -f $(TOOLSDIR)/analysis/lib/*.o
	@rm -f $(TOOLSDIR)/roofit/dic/*.*
	@rm -f $(TOOLSDIR)/roofit/lib/*.o
	@rm -f $(TOOLSDIR)/lib/*.a

veryclean:
	@echo "Cleaning ..."
	@rm -f $(TOOLSDIR)/analysis/lib/*.o
	@rm -f $(TOOLSDIR)/roofit/dic/*.*
	@rm -f $(TOOLSDIR)/roofit/lib/*.o
	@rm -f $(TOOLSDIR)/lib/*.a

purge:
	@echo "Purging ..."
	@rm -f *~ */*~
