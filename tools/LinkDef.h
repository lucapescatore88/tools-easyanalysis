// Definitions for Cint and PYTHON interface
// For info have a look at https://root.cern.ch/selecting-dictionary-entries-linkdefh

#ifdef __CLING__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;
#pragma link C++ nestedtypedef;


// ls *.hpp | awk '{ print "#pragma link C++ defined_in \""$1"\";" }'

#pragma link C++ class Scaler+;
#pragma link C++ class Analysis+;
#pragma link C++ defined_in "analysis.hpp";

#pragma link C++ class EffComp+;
#pragma link C++ class CutFlow+;
#pragma link C++ defined_in "cutFlow.hpp";

#pragma link C++ defined_in "efficiencyFunctions.hpp";

#pragma link C++ defined_in "feldmanCousins.hpp";

#pragma link C++ defined_in "generalFunctions.hpp";

#pragma link C++ class ModelBuilder+;
#pragma link C++ defined_in "modelBuilderFunctions.hpp";
#pragma link C++ defined_in "modelBuilder.hpp";

#pragma link C++ class MultiAnalysis+;
#pragma link C++ defined_in "multiAnalysis.hpp";

#pragma link C++ class CutOptimizer+;
#pragma link C++ defined_in "optimiser.hpp";

#pragma link C++ defined_in "stringToPdf.hpp";

#pragma link C++ class TypeDB+;
#pragma link C++ class variable+;
#pragma link C++ class varEq+;
#pragma link C++ class TreeReader+;
#pragma link C++ defined_in "treeReader.hpp";

#endif
