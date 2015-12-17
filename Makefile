ROOTCFLAGS  = $(shell root-config --cflags --glibs)

TOOLSDIR    = $(TOOLSSYS)
ifeq ($(TOOLSSYS),)
	TOOLSDIR    = $(PWD)
endif

CXX         = g++
CXXFLAGS    = -g -fPIC -Wall -O2 $(ROOTCFLAGS) -lTMVA -lRooFit -lRooStats -I$(TOOLSDIR) -I$(TOOLSDIR)/analysis -L$(TOOLSDIR)/lib

ANALYSIS    = $(wildcard $(TOOLSDIR)/analysis/*.cpp)
TOOLS       = $(wildcard $(TOOLSDIR)/*.cpp)

ANALYSISOBJ = $(patsubst $(TOOLSDIR)/analysis/%.cpp, $(TOOLSDIR)/analysis/lib/%.o, $(ANALYSIS))
LIBS        = $(patsubst $(TOOLSDIR)/%.cpp, $(TOOLSDIR)/lib/%.a, $(TOOLS))
EXE         = $(ANALYSISOBJ) $(TOOLSDIR)/lib/analysis.a $(LIBS)



all: $(EXE)

print:
	$(EXE)

#Compiling libraries
$(TOOLSDIR)/analysis/lib/%.o: $(TOOLSDIR)/analysis/%.cpp $(TOOLS) $(ANALYSIS)
	@echo
	@echo "Making $(@) ... "
	$(CXX) $(CXXFLAGS) -c $< -o $@
	@rm -f $(TOOLSDIR)/lib/analysis.a

$(TOOLSDIR)/lib/analysis.a: $(ANALYSISOBJ)
	@echo
	ar rcs $@ $^;

$(TOOLSDIR)/lib/%.a: $(TOOLSDIR)/%.cpp $(TOOLS)
	@echo
	@echo "Making $(@) ... "
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "Cleaning ..."
	@rm -f $(TOOLSDIR)/lib/*.a

cleanall:
	@echo "Cleaning ..."
	@rm -f $(TOOLSDIR)/analysis/lib/*.o
	@rm -f $(TOOLSDIR)/lib/*.a

purge:
	@echo "Purging ..."
	@rm -f *~ */*~
