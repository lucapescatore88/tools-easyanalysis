#ifndef EFF_FUNCTIONS_HPP
#define EFF_FUNCTIONS_HPP

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>

#include "TFile.h"
#include "TTree.h"
#include "TH1.h"
#include "TH2.h"
#include "TMath.h"
#include "TEntryList.h"
#include "TGraph.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TLine.h"
#include "TRandom.h"
#include "TRandom3.h"
#include "TF1.h"
#include "TCut.h"
#include "TLatex.h"
#include "TPaveText.h"
#include "TMultiGraph.h"
#include "TString.h"
#include "TAxis.h"
#include "TLorentzVector.h"

#include "RooAbsArg.h"
#include "RooArgSet.h"
#include "RooDataSet.h"
#include "RooDataHist.h"
#include "RooAbsPdf.h"
#include "RooArgList.h"
#include "RooRealVar.h"
#include "RooPlot.h"
#include "RooCurve.h"
#include "RooHist.h"

#include "treeReader.hpp"

using namespace std;


TString buildSelectStr(TCut cuts, TString weight);

double getEvtN(TString name, TString plot, TTree * tree, TCut cuts = "", TString weight = "", double * entries = NULL);
double getEff(TString name, TString plot, TTree * tree, TCut cuts, TTree * downTree = NULL, TCut downCuts = "", TString weight = "", double  * efferr = NULL, TTree * relTree = NULL, TCut relCuts = "", TTree * relDownTree = NULL, TCut relDownCuts = "", TString relWeight = "");
vector< vector< double > > getEvtsAfterCuts(TString name, TString plot, TTree * tree, TCut genCut = "", vector<TCut> cuts = vector<TCut>(), TString weight = "");
vector< vector < double > > getEff(TString name, TString part, TTree * tree, TCut genCut, vector<TCut> cuts, TString weight = "", TTree * relTree = NULL, TCut relCuts = "", TString relWeight = "");

TH2F * getEff(TString name, TString plot,  TString xvar, TString yvar, int xnbins, double * xbins, int ynbins, double * ybins, TTree * tree, TCut cuts, TTree * downTree, TCut downCuts, TString weight = "", TTree * relTree = NULL, TCut relCuts = "", TTree * relDownTree = NULL, TCut relDownCuts = "", TString relWeight = "", string opt = "");
TH2F * getEff(TString name, TString plot, TString xvar, TString yvar, int xnbins, double * xbins, int ynbins, double * ybins, TTree * tree, TCut cuts, TCut downCuts, TString weight = "", TTree * relTree = NULL, TCut relCuts = "", TCut relDownCuts = "", TString relWeight = "", string opt = "");

TH1F * getEff(TString name, TString xvar, int xnbins, double * xbins, TTree * tree, TCut cuts, TTree * downTree, TCut downCuts, TString weight = "", TTree * relTree = NULL, TCut relCuts = "", TTree * relDownTree = NULL, TCut relDownCuts = "", TString relWeight = "", string opt = "");
TH1F * getEff(TString name,  TString xvar, int xnbins, double * xbins, TTree * tree, TCut cuts, TCut downCuts, TString weight = "", TTree * relTree = NULL, TCut relCuts = "", TCut relDownCuts = "", TString relWeight = "", string opt = "");

#endif
