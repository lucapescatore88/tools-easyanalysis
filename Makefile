NAME       = tools

LIBDIR     = $(TOOLSSYS)/lib

TOOLSDIR   = $(TOOLSSYS)/$(NAME)
INCFLAGS   = -I$(TOOLSSYS) -I$(TOOLSDIR)
TOOLSSRC   = $(wildcard $(TOOLSDIR)/*.cpp)
TOOLSINC   = $(wildcard $(TOOLSDIR)/*.hpp)
TOOLSOBJ   = $(patsubst $(TOOLSDIR)/%.cpp, $(TOOLSDIR)/obj/%.o, $(TOOLSSRC))
TOOLSLD    = $(TOOLSDIR)/LinkDef.h
TOOLSLIB   = $(LIBDIR)/lib$(NAME).a
TOOLSDIC   = $(LIBDIR)/Dict_$(NAME).cc
TOOLSDICO  = $(LIBDIR)/Dict_$(NAME).o
TOOLSLIBSO = $(LIBDIR)/lib$(NAME).so
TOOLSLIBRM = $(LIBDIR)/lib$(NAME).rootmap

ROOFIT     = roofit
ROOFITDIR  = $(TOOLSSYS)/$(ROOFIT)
INCFLAGS   += -I$(ROOFITDIR)
ROOFITSRC  = $(wildcard $(ROOFITDIR)/*.cpp)
ROOFITINC  = $(wildcard $(ROOFITDIR)/*.h)
ROOFITDIC  = $(patsubst $(ROOFITDIR)/%.cpp,     $(ROOFITDIR)/dic/%.cpp, $(ROOFITSRC))
ROOFITDICO = $(patsubst $(ROOFITDIR)/dic/%.cpp, $(ROOFITDIR)/obj/%.o,   $(ROOFITDIC))
ROOFITLD   = $(ROOFITDIR)/LinkDef.h
ROOFITINC  := $(filter-out $(ROOFITLD), $(ROOFITINC))
ROOFITLIB  = $(LIBDIR)/lib$(ROOFIT).a
RFDIC      = $(LIBDIR)/Dict_$(ROOFIT).cc
RFDICO     = $(LIBDIR)/Dict_$(ROOFIT).o
ROOFITLIBSO = $(LIBDIR)/lib$(ROOFIT).so
ROOFITLIBRM = $(LIBDIR)/lib$(ROOFIT).rootmap

ROOTCINT   = rootcint
ROOTCLING  = rootcling

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

$(TOOLSLIBSO): $(TOOLSDICO)
	@echo
	@echo "Making shared library $(@) ..."
	$(CXX) -shared $(TOOLSDICO) -o $(TOOLSLIBSO) $(CXXFLAGS)

$(RFDIC):
	@echo
	@echo "Making dictionary $(@) ..."
	cd $(ROOFIT) ; $(ROOTCLING) -rootbuild -f $(RFDIC) -s $(ROOFITLIBSO) -rmf $(ROOFITLIBRM) -I$(ROOTINC) $(INCFLAGS) $(ROOFITSRC) $(ROOFITINC) $(ROOFITLD)

$(RFDICO): $(RFDIC)
	@echo
	@echo "Making dictionary object $(@) ..."
	$(CXX) -c $(RFDIC) -o $(RFDICO) $(CXXFLAGS)

$(ROOFITLIBSO): $(RFDICO)
	@echo
	@echo "Making shared library $(@) ..."
	$(CXX) -shared $(RFDICO) -o $(ROOFITLIBSO) $(CXXFLAGS)

shared: $(ROOFITLIBSO) $(TOOLSLIBSO)

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
	@echo $(RFDIC)
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
	@echo $(ROOFITLIBSO)
	@echo

cleanso:
	@echo "Cleaning shared ..."
	@rm -f $(TOOLSLIBSO) $(TOOLSDIC) $(TOOLSDICO) $(TOOLSLIBRM)
	@rm -f $(ROOFITLIBSO) $(RFDIC) $(RFDICO) $(ROOFITLIBRM)

clean:
	@echo "Cleaning ..."
	@rm -f $(TOOLSLIB) $(TOOLSLIBSO) $(TOOLSDIC) $(TOOLSDICO) $(TOOLSLIBRM)
	@rm -f $(ROOFITLIB) $(ROOFITLIBSO) $(RFDIC) $(RFDICO) $(ROOFITLIBRM)

cleanall: clean
	@rm -f $(MAKES)
	@rm -f $(LIBDIR)/*
	@rm -f $(ROOFITDIR)/dic/*

veryclean: cleanall

purge:
	@echo "Purging ..."
	@rm -f *~ */*~ */*/*~
