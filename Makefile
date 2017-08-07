TOOLSDIR    = $(TOOLSSYS)
INCFLAGS    = -I$(TOOLSDIR) -I$(TOOLSDIR)/analysis -I$(TOOLSDIR)/roofit

GSLDIR      = $(GSLSYS)
INCFLAGS    += -I$(GSLDIR)/include

ROOTCINT    = rootcint
ROOTFLAGS   = $(shell root-config --cflags --glibs)

CXX         = g++
CXXFLAGS    = -g -fPIC -Wall -O2 -lTMVA -lRooFit -lRooStats -lMathMore $(ROOTFLAGS) $(INCFLAGS)

ANALYSIS    = $(wildcard $(TOOLSDIR)/analysis/*.cpp)
ROOFIT      = $(wildcard $(TOOLSDIR)/roofit/*.cpp)
TOOLS       = $(wildcard $(TOOLSDIR)/*.cpp)

ANALYSISOBJ = $(patsubst $(TOOLSDIR)/analysis/%.cpp,   $(TOOLSDIR)/analysis/lib/%.o, $(ANALYSIS))
ROOFITDIC   = $(patsubst $(TOOLSDIR)/roofit/%.cpp,     $(TOOLSDIR)/roofit/dic/%.cpp, $(ROOFIT))
ROOFITOBJ   = $(patsubst $(TOOLSDIR)/roofit/dic/%.cpp, $(TOOLSDIR)/roofit/lib/%.o,   $(ROOFITDIC))
LIBS        = $(patsubst $(TOOLSDIR)/%.cpp,            $(TOOLSDIR)/lib/%.a,          $(TOOLS))

MAKES       = $(ROOFITDIC) $(ROOFITOBJ) $(ANALYSISOBJ) $(TOOLSDIR)/lib/roofit.a $(TOOLSDIR)/lib/analysis.a $(LIBS)


all: $(MAKES)

$(TOOLSDIR)/roofit/dic/%.cpp: $(TOOLSDIR)/roofit/%.cpp
	@echo
	@echo "Generating dictionary $(@) ..."
	$(ROOTCINT) -l -f $@ -c -p -I$(GSLDIR)/include $^

$(TOOLSDIR)/roofit/lib/%.o: $(TOOLSDIR)/roofit/dic/%.cpp
	@echo
	@echo "Making $(@) ..."
	$(CXX) -c $< -o $@ $(CXXFLAGS)

$(TOOLSDIR)/analysis/lib/%.o: $(TOOLSDIR)/analysis/%.cpp
	@echo
	@echo "Making $(@) ..."
	$(CXX) -c $< -o $@ $(CXXFLAGS)

$(TOOLSDIR)/lib/roofit.a: $(ROOFITOBJ)
	@echo
	@echo "Archiving $(@) ..."
	ar rcs $@ $^;

$(TOOLSDIR)/lib/analysis.a: $(ANALYSISOBJ)
	@echo
	@echo "Archiving $(@) ..."
	ar rcs $@ $^;

$(TOOLSDIR)/lib/%.a: $(TOOLSDIR)/%.cpp $(TOOLS)
	@echo
	@echo "Making $(@) ..."
	$(CXX) -c $< -o $@ $(CXXFLAGS)

print:
	@echo
	@echo "Sources"
	@echo $(ROOFIT)
	@echo $(ANALYSIS)
	@echo $(TOOLS)
	@echo
	@echo "Dictionaries"
	@echo $(ROOFITDIC)
	@echo
	@echo "Libraries"
	@echo $(ROOFITOBJ)
	@echo $(ANALYSISOBJ)
	@echo $(LIBS)
	@echo $(EXE)
	@echo
	@echo "Flags"
	@echo $(CXXFLAGS)
	@echo

clean:
	@echo "Cleaning ..."
	@rm -f $(TOOLSDIR)/lib/*.a

cleanall: clean
	@rm -f $(MAKES)

veryclean: cleanall

purge:
	@echo "Purging ..."
	@rm -f *~ */*~
