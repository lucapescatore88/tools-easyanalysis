TOOLSDIR  = $(TOOLSSYS)/tools
INCFLAGS  = -I$(TOOLSSYS) -I$(TOOLSDIR)

TOOLS     = $(wildcard $(TOOLSDIR)/*.cpp)
TOOLSOBJ  = $(patsubst $(TOOLSDIR)/%.cpp, $(TOOLSDIR)/obj/%.o, $(TOOLS))

ROOFITDIR = $(TOOLSSYS)/roofit
INCFLAGS  += -I$(ROOFITDIR)

GSLDIR    = $(GSLSYS)
INCFLAGS  += -I$(GSLDIR)/include

ROOTCINT  = rootcint
ROOFIT    = $(wildcard $(ROOFITDIR)/*.cpp)
ROOFITDIC = $(patsubst $(ROOFITDIR)/%.cpp,     $(ROOFITDIR)/dic/%.cpp, $(ROOFIT))
ROOFITOBJ = $(patsubst $(ROOFITDIR)/dic/%.cpp, $(ROOFITDIR)/obj/%.o,   $(ROOFITDIC))

ROOTFLAGS = $(shell root-config --cflags --glibs)

CXX       = g++
CXXFLAGS  = -g -fPIC -Wall -O2 -lTMVA -lRooFit -lRooStats -lMathMore $(ROOTFLAGS) $(INCFLAGS)

LIBDIR    = $(TOOLSSYS)/lib
LIBS      = $(LIBDIR)/roofit.a $(LIBDIR)/tools.a

MAKES     = $(ROOFITDIC) $(ROOFITOBJ) $(TOOLSOBJ) $(LIBS)


all: $(MAKES)

$(TOOLSDIR)/obj/%.o: $(TOOLSDIR)/%.cpp
	@echo
	@echo "Making $(@) ..."
	$(CXX) -c $< -o $@ $(CXXFLAGS)

$(LIBDIR)/tools.a: $(TOOLSOBJ)
	@echo
	@echo "Archiving $(@) ..."
	ar rcs $@ $^;

$(ROOFITDIR)/dic/%.cpp: $(ROOFITDIR)/%.cpp
	@echo
	@echo "Generating dictionary $(@) ..."
	$(ROOTCINT) -l -f $@ -c -p -I$(GSLDIR)/include $^

$(ROOFITDIR)/obj/%.o: $(ROOFITDIR)/dic/%.cpp
	@echo
	@echo "Making $(@) ..."
	$(CXX) -c $< -o $@ $(CXXFLAGS)

$(LIBDIR)/roofit.a: $(ROOFITOBJ)
	@echo
	@echo "Archiving $(@) ..."
	ar rcs $@ $^;

print:
	@echo
	@echo "Sources"
	@echo $(TOOLS)
	@echo $(ROOFIT)
	@echo
	@echo "Dictionaries"
	@echo $(ROOFITDIC)
	@echo
	@echo "Libraries"
	@echo $(TOOLSOBJ)
	@echo $(ROOFITOBJ)
	@echo
	@echo "Flags"
	@echo $(CXXFLAGS)
	@echo
	@echo "Outputs"
	@echo $(LIBS)
	@echo

clean:
	@echo "Cleaning ..."
	@rm -f $(LIBS)

cleanall: clean
	@rm -f $(MAKES)
	@rm -f @$(ROOFITDIR)/dic/*.pcm

veryclean: cleanall

purge:
	@echo "Purging ..."
	@rm -f *~ */*~
