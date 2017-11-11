// Definitions for Cint and PYTHON interface
// For info have a look at https://root.cern.ch/selecting-dictionary-entries-linkdefh

#ifdef __CLING__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;
#pragma link C++ nestedtypedef;


// ls *.h | awk '{ print "#pragma link C++ defined_in \""$1"\";" }'

#pragma link C++ class RooAmorosoPdf+;
#pragma link C++ defined_in "RooAmorosoPdf.h";
#pragma link C++ class RooApollonios+;
#pragma link C++ defined_in "RooApollonios.h";
#pragma link C++ class RooExpAndGauss+;
#pragma link C++ defined_in "RooExpAndGauss.h";
#pragma link C++ class RooGeneralizedHyperbolic+;
#pragma link C++ defined_in "RooGeneralizedHyperbolic.h";
#pragma link C++ class RooIpatia2+;
#pragma link C++ defined_in "RooIpatia2.h";
#pragma link C++ class RooIpatia+;
#pragma link C++ defined_in "RooIpatia.h";
#pragma link C++ class RooJohnson+;
#pragma link C++ defined_in "RooJohnson.h";
//#pragma link C++ class RooLogGamma+;
//#pragma link C++ defined_in "RooLogGamma.h";
#pragma link C++ class RooMisIDGaussian+;
#pragma link C++ defined_in "RooMisIDGaussian.h";
#pragma link C++ class RooMomentumFractionPdf+;
#pragma link C++ defined_in "RooMomentumFractionPdf.h";
#pragma link C++ class RooPowerLawApproxVar+;
#pragma link C++ defined_in "RooPowerLawApproxVar.h";

#endif
