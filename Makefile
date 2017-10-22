NAME      = tools

TOOLSDIR  = $(TOOLSSYS)/$(NAME)
INCFLAGS  = -I$(TOOLSSYS) -I$(TOOLSDIR)

TOOLS     = $(wildcard $(TOOLSDIR)/*.cpp)
TOOLSINC  = $(wildcard $(TOOLSDIR)/*.hpp)
TOOLSOBJ  = $(patsubst $(TOOLSDIR)/%.cpp, $(TOOLSDIR)/obj/%.o, $(TOOLS))

ROOFITDIR = $(TOOLSSYS)/roofit
INCFLAGS  += -I$(ROOFITDIR)

GSLDIR    = $(GSLSYS)
INCFLAGS  += -I$(GSLDIR)/include

ROOTCINT  = rootcint
ROOFIT    = $(wildcard $(ROOFITDIR)/*.cpp)
ROOFITINC = $(wildcard $(ROOFITDIR)/*.hpp)
ROOFITDIC = $(patsubst $(ROOFITDIR)/%.cpp,     $(ROOFITDIR)/dic/%.cpp, $(ROOFIT))
ROOFITOBJ = $(patsubst $(ROOFITDIR)/dic/%.cpp, $(ROOFITDIR)/obj/%.o,   $(ROOFITDIC))


ROOTFLAGS = $(shell root-config --cflags --glibs)

CXX       = g++
CXXFLAGS  = -g -fPIC -Wall -O2 -lTMVA -lRooFit -lRooStats -lMathMore $(ROOTFLAGS) $(INCFLAGS)

LIBDIR    = $(TOOLSSYS)/lib
LIBS      = $(LIBDIR)/libroofit.a $(LIBDIR)/lib$(NAME).a

ROOTCLING = rootcling
CINTFILE  = $(TOOLSDIR)/$(NAME)_Dict.cc
CINTOBJ   = $(TOOLSDIR)/$(NAME)_Dict.o
SHLIB     = $(LIBDIR)/lib$(NAME).so

MAKES     = $(ROOFITDIC) $(ROOFITOBJ) $(TOOLSOBJ) $(LIBS) $(SHLIB)


all: $(MAKES)

$(TOOLSDIR)/obj/%.o: $(TOOLSDIR)/%.cpp
	@echo
	@echo "Making $(@) ..."
	$(CXX) -c $< -o $@ $(CXXFLAGS)

$(LIBDIR)/libtools.a: $(TOOLSOBJ)
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

$(LIBDIR)/libroofit.a: $(ROOFITOBJ)
	@echo
	@echo "Archiving $(@) ..."
	ar rcs $@ $^;

$(CINTOBJ): $(TOOLS) $(TOOLINC) LinkDef_$(NAME).h
	@echo
	@echo "Making Root dictionary $(@) ..."
	$(ROOTCLING) -rootbuild -f $(CINTFILE) -s $(SHLIB) -rmf lib$(PACKAGE).rootmap $(INCFLAGS) -I`root-config --incdir` $(TOOLS) $(TOOLSINC) LinkDef_$(NAME).h
	@echo
	@echo "Compiling $(CINTFILE) ..."
	$(CXX) -c $(CXXFLAGS) -fPIC -o $(CINTOBJ) $(CINTFILE)

$(SHLIB): $(CINTOBJ)
	@echo
	@echo "Making shared library $(CINTOBJ) ..."
	$(CXX) -shared $(CXXFLAGS) $(CINTOBJ) -o $(SHLIB)

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
	@echo $(SHLIB)
	@echo

clean:
	@echo "Cleaning ..."
	@rm -f $(LIBS) $(CINTFILE) $(CINTOBJ) $(SHLIB)

cleanall: clean
	@rm -f $(MAKES)
	@rm -f $(ROOFITDIR)/dic/*.pcm
	@rm -f $(LIBDIR)/*.pcm

veryclean: cleanall

purge:
	@echo "Purging ..."
	@rm -f *~ */*~ */*/*~
