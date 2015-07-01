#include "RooDataSet.h"
#include "RooRealVar.h"
#include "RooArgSet.h"
#include "RooMsgService.h"

#include "ReadTree_comp.hpp"

using namespace RooFit;

class DataSet{

	RooDataSet * data;

	public :
	
	DataSet(TString name, TString treename, TString filename, TString weight = "", TString cuts = "", vector <string> br = vector <string>(), string opt = "names");
	DataSet(TString name, TTree * tree, TString weight = "", TString cuts = "", vector <string> br = vector <string>(), string opt = "names");
	DataSet(TString name, TreeReader * tree, TString weight = "", TString cuts = "", vector <string> br = vector <string>(), string opt = "names");

	RooDataSet * GetDataSet() { return data; }
};
	
