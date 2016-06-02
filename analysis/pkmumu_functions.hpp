#ifndef PKMUMU_FUNCTIONS_HPP
#define PKMUMU_FUNCTIONS_HPP

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
#include "TLorentzVector.h"
#include "TVector.h"
#include "TVector3.h"
#include "general_functions.hpp"
#include "ReadTree_comp.hpp"
#include "DPHelpers.hpp" 

using namespace std;

//////////////////////////////////////////
//			Pete's Functions			//
//////////////////////////////////////////


void infoprint(string message, bool sep=0, string type="INFO");
TCut Str_conv(string input);
TCut Anglecut(string particle);
TCut Triggers(int level,string mother ="Lb");
TCut pKmmSelectionCut(int diMuOpts,int PIDOpts,string bStr="Lb");
bool kinvar_nanchecker(vector<double> v_anglemass, int entry, TLorentzVector proton,TLorentzVector kaon,TLorentzVector mupp,TLorentzVector mump,TLorentzVector JPsip);
double EtaCalc (TreeReader* reader, string pStr);
bool pKmmPreSelection(TreeReader* reader, int diMuOpts, int PIDOpts,string bStr = "Lb");
bool L0triggers(TreeReader* reader, string mother = "Lb");
bool Hlt1triggers(TreeReader* reader, string mother = "Lb");
bool Hlt2triggers(TreeReader* reader, string mother = "Lb");
double fourBodyMass(TLorentzVector part1,TLorentzVector part2,TLorentzVector part3,TLorentzVector part4);
bool qbinCheck(double q2,int qbin);
int binCheck(vector<double>bins,double val);
bool rangeCheck(double val,double min,double max);
bool accCheck(double theta);
bool motherIDmuCheck (TreeReader* reader,string mother);
bool pKPIDTrue(TreeReader* reader,string bStr="Lb");
bool bkgPIDTrue(TreeReader* reader,string bStr, bool recotrue = false);
bool RKstPIDTrue(TreeReader* reader, int type);
TLorentzVector particleBuilder(TreeReader* reader, string part, bool truth = 1);
TVector3 pVecBuilder(TreeReader* reader, string part, bool truth = 1);
TLorentzVector partSwap(TreeReader* reader, string recoID, string swapID);
vector <vector <int> > q2BinStat(TreeReader* reader,vector <vector <double> > bins,int checkType = 0, bool recoMu = true,string mother="Lb",string bStr = "Lb",string weightType = "none");
vector<vector<vector<int> > > q2EffStats(TreeReader* reader,vector<vector<double> > bins,string mother="Lb",string bStr="Lb");
void effCalc(vector<int> q2Stats1, vector<int> q2Stats2, string qbinStr[], stringstream * stream, TH1F* hist, double scaleFactor = 1);
void asciiPrint(string prog,string auth);


#endif


