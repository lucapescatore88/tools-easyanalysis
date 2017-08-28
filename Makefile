TOOLSDIR  = $(TOOLSSYS)/tools
INCFLAGS  = -I$(TOOLSSYS) -I$(TOOLSDIR)

TOOLS     = $(wildcard $(TOOLSDIR)/*.cpp)
TOOLSHPP  = $(wildcard $(TOOLSDIR)/*.hpp)
TOOLSOBJ  = $(patsubst $(TOOLSDIR)/%.cpp, $(TOOLSDIR)/obj/%.o, $(TOOLS))

ROOFITDIR = $(TOOLSSYS)/roofit
INCFLAGS  += -I$(ROOFITDIR)

GSLDIR    = $(GSLSYS)
INCFLAGS  += -I$(GSLDIR)/include

ROOTCINT  = rootcint
ROOFIT    = $(wildcard $(ROOFITDIR)/*.cpp)
ROOFITDIC = $(patsubst $(ROOFITDIR)/%.cpp,     $(ROOFITDIR)/dic/%.cpp, $(ROOFIT))
ROOFITOBJ = $(patsubst $(ROOFITDIR)/dic/%.cpp, $(ROOFITDIR)/obj/%.o,   $(ROOFITDIC))

PACKAGE = tools
CINTFILE  = $(TOOLSDIR)/$(PACKAGE)_Dict.cc
CINTOBJ   = $(TOOLSDIR)/$(PACKAGE)_Dict.o
LIBFILE   = $(LIBDIR)/lib$(PACKAGE).a
SHLIBFILE = $(LIBDIR)/lib$(PACKAGE).so

ROOTFLAGS = $(shell root-config --cflags --glibs)

CXX       = g++
CXXFLAGS  = -g -fPIC -Wall -O2 -lTMVA -lRooFit -lRooStats -lMathMore $(ROOTFLAGS) $(INCFLAGS)

LIBDIR    = $(TOOLSSYS)/lib
LIBS      = $(LIBDIR)/roofit.a $(LIBDIR)/tools.a

MAKES     = $(ROOFITDIC) $(ROOFITOBJ) $(TOOLSOBJ) $(LIBS) $(SHLIBFILE)


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

$(CINTOBJ): $(TOOLS) $(TOOLHPP) LinkDef.h
	@echo
	@echo "Making Root dictionary $(@) ..."
	rootcling -rootbuild -f $(CINTFILE) -s $(SHLIBFILE) -rmf lib$(PACKAGE).rootmap $(INCFLAGS) -I`root-config --incdir` $(TOOLS) $(TOOLSHPP) tools.hpp LinkDef.h
	@echo "Compiling $(CINTFILE)"
	$(CXX) -c $(CXXFLAGS) -fPIC -o $(CINTOBJ) $(CINTFILE)

$(SHLIBFILE): $(CINTOBJ)
	@echo
	@echo "Making shared library"
	$(CXX) -shared $(CXXFLAGS) $(CINTOBJ) -o $(SHLIBFILE)

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
	@echo
	@echo "Cleaning ..."
	@rm -f $(LIBS) $(CINTFILE) $(CINTOBJ) $(SHLIBFILE)

cleanall: clean
	@rm -f $(LIBS) $(CINTFILE) $(CINTOBJ) $(SHLIBFILE)
	@rm -f $(MAKES)
	@rm -f @$(ROOFITDIR)/dic/*.pcm

veryclean: cleanall

purge:
	@echo
	@echo "Purging ..."
	@rm -f *~ */*~ */*/*~
