NAME       = tools

LIBDIR     = $(TOOLSSYS)/lib

TOOLSDIR   = $(TOOLSSYS)/$(NAME)
INCFLAGS   = -I$(TOOLSSYS) -I$(TOOLSDIR)
TOOLSSRC   = $(wildcard $(TOOLSDIR)/*.cpp)
TOOLSINC   = $(wildcard $(TOOLSDIR)/*.hpp)
TOOLSOBJ   = $(patsubst $(TOOLSDIR)/%.cpp, $(TOOLSDIR)/obj/%.o, $(TOOLSSRC))
TOOLSLD    = $(TOOLSDIR)/LinkDef.h
TOOLSLIB   = $(LIBDIR)/lib$(NAME).a

ROOTCLING  = rootcling
TOOLSDIC   = $(LIBDIR)/Dict.cc
TOOLSDICO  = $(LIBDIR)/Dict.o
TOOLSLIBSO = $(LIBDIR)/lib$(NAME).so
TOOLSLIBRM = $(LIBDIR)/lib$(NAME).rootmap

ROOFIT     = roofit
ROOFITDIR  = $(TOOLSSYS)/$(ROOFIT)
INCFLAGS   += -I$(ROOFITDIR)
ROOFITLIB  = $(LIBDIR)/lib$(ROOFIT).a

ROOTCINT   = rootcint
ROOFITSRC  = $(wildcard $(ROOFITDIR)/*.cpp)
ROOFITINC  = $(wildcard $(ROOFITDIR)/*.hpp)
ROOFITDIC  = $(patsubst $(ROOFITDIR)/%.cpp,     $(ROOFITDIR)/dic/%.cpp, $(ROOFITSRC))
ROOFITDICO = $(patsubst $(ROOFITDIR)/dic/%.cpp, $(ROOFITDIR)/obj/%.o,   $(ROOFITDIC))
ROOFITDF   = $(ROOFITDIR)/LinkDef.h

GSLDIR     = $(GSLSYS)
INCFLAGS  += -I$(GSLDIR)/include

ROOTFLAGS  = $(shell root-config --cflags --glibs)
ROOTINC    = $(shell root-config --incdir)

CXX        = g++
CXXFLAGS   = -g -fPIC -Wall -O2 -lTMVA -lRooFit -lRooStats -lMathMore $(ROOTFLAGS) $(INCFLAGS)

MAKES      = $(ROOFITDIC) $(ROOFITDICO) $(ROOFITLIB) $(TOOLSOBJ) $(TOOLSLIB)
#MAKES      = $(ROOFITDIC) $(ROOFITDICO) $(ROOFITLIB) $(TOOLSOBJ) $(TOOLSLIB) $(TOOLSLIBSO)



all: $(MAKES)

$(TOOLSDIR)/obj/%.o: $(TOOLSDIR)/%.cpp
	@echo
	@echo "Making object $(@) ..."
	$(CXX) -c $< -o $@ $(CXXFLAGS)

$(TOOLSDIR)/obj/%.o: $(TOOLSDIR)/%.cpp $(TOOLSDIR)/%.hpp
	@echo
	@echo "Making object $(@) ..."
	$(CXX) -c $< -o $@ $(CXXFLAGS)

$(TOOLSLIB): $(TOOLSOBJ)
	@echo
	@echo "Making static library $(@) ..."
	ar rcs $@ $^;

$(ROOFITDIR)/dic/%.cpp: $(ROOFITDIR)/%.cpp
	@echo
	@echo "Making dictionary $(@) ..."
	$(ROOTCINT) -l -f $@ -c -p -I$(GSLDIR)/include $^

$(ROOFITDIR)/dic/%.cpp: $(ROOFITDIR)/%.cpp $(ROOFITDIR)/%.hpp
	@echo
	@echo "Making dictionary $(@) ..."
	$(ROOTCINT) -l -f $@ -c -p -I$(GSLDIR)/include $^

$(ROOFITDIR)/obj/%.o: $(ROOFITDIR)/dic/%.cpp
	@echo
	@echo "Making dictionary object $(@) ..."
	$(CXX) -c $< -o $@ $(CXXFLAGS)

$(ROOFITLIB): $(ROOFITDICO)
	@echo
	@echo "Making static library $(@) ..."
	ar rcs $@ $^;

$(TOOLSDIC):
	@echo
	@echo "Making dictionary $(@) ..."
	cd $(NAME) ; $(ROOTCLING) -rootbuild -f $(TOOLSDIC) -s $(TOOLSLIBSO) -rmf $(TOOLSLIBRM) -I$(ROOTINC) $(INCFLAGS) $(TOOLSSRC) $(TOOLSINC) $(TOOLSLD)

$(TOOLSDICO): $(TOOLSDIC)
	@echo
	@echo "Making dictionary object $(@) ..."
	$(CXX) -c $(TOOLSDIC) -o $(TOOLSDICO) $(CXXFLAGS)

shared: all $(TOOLSDICO)
#$(TOOLSLIBSO): all $(TOOLSDICO)
	@echo
	@echo "Making shared library $(@) ..."
	$(CXX) -shared $(TOOLSDICO) -o $(TOOLSLIBSO) $(CXXFLAGS)

print:
	@echo
	@echo "Sources"
	@echo $(TOOLSSRC)
	@echo $(ROOFITSRC)
	@echo
	@echo "Includes"
	@echo $(TOOLSINC)
	@echo $(ROOFITINC)
	@echo
	@echo "Dictionaries"
	@echo $(TOOLSDIC)
	@echo $(ROOFITDIC)
	@echo
	@echo "Flags"
	@echo $(INCFLAGS)
	@echo $(CXXFLAGS)
	@echo
	@echo "Libraries"
	@echo $(TOOLSLIB)
	@echo $(TOOLSLIBSO)
	@echo $(ROOFITLIB)
	@echo

clean:
	@echo "Cleaning ..."
	@rm -f $(TOOLSLIB) $(TOOLSLIBSO) $(ROOFITLIB) $(TOOLSDIC) $(TOOLSDICO) $(TOOLSLIBSO) $(TOOLSLIBRM)

cleanall: clean
	@rm -f $(MAKES)
	@rm -f $(LIBDIR)/*
	@rm -f $(ROOFITDIR)/dic/*

veryclean: cleanall

purge:
	@echo "Purging ..."
	@rm -f *~ */*~ */*/*~
