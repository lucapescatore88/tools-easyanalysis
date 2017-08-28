// Definitions for Cint and PYTHON interface
// For info have a look at https://root.cern.ch/selecting-dictionary-entries-linkdefh

#ifdef __CINT__
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;
#pragma link C++ nestedtypedef;

// cuts.cpp
//#pragma link C++ defined_in "tools/analyserscuts.cpp";
// Must define a random function inside the file, then if I call it in python also the other things become available
//#pragma link C++ function getSampleID;

// TreeReader
#pragma link C++ class TypeDB+;
#pragma link C++ class variable+;
#pragma link C++ class varEq+;
#pragma link C++ class TreeReader+;

// optimiser
#pragma link C++ class CutOptimizer+;

// ModelBuilder
#pragma link C++ class ModelBuilder+;

// optimiser
#pragma link C++ class CutOptimizer+;

// CutFlow
#pragma link C++ class EffComp+;
#pragma link C++ class CutFlow+;

// analyser
#pragma link C++ class Scaler+;
#pragma link C++ class Analysis+;

// multi_analyser
#pragma link C++ class MultiAnalysis+;

// general_function
#pragma link C++ defined_in "tools/general_function.hpp";
#pragma link C++ function genRndm;
#pragma link C++ function luminosity;
#pragma link C++ function luminosita;
#pragma link C++ function calcChi2;
#pragma link C++ function residual;
#pragma link C++ function pull;
#pragma link C++ function computeAverage;
#pragma link C++ function computeAverage2D;


#endif
