#ifndef OPTIMISER_HPP
#define OPTIMISER_HPP

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
#include "TF2.h"
#include "TCut.h"
#include "TLatex.h"
#include "TPaveText.h"
#include "TMultiGraph.h"
#include "TString.h"
#include "TAxis.h"
#include "TLegend.h"
#include "TLorentzVector.h"
#include "TGaxis.h"
#include "TGraph2D.h"

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

#include "generalFunctions.hpp"
#include "treeReader.hpp"
#include "efficiencyFunctions.hpp"

using namespace std;
using namespace RooFit;


TString subStrings(TString tmp, vector <TString> names, vector<double> values);
bool compare_vectors_double( vector<double> v1, vector<double> v2 );
vector <double> getSteps(double min, double max, int nsteps, string opt = "");


void buildComb(const vector<vector<double> > &allVecs, vector<vector<double> > &result, size_t vecIndex = 0, const vector <double> vSoFar = vector <double>(), string opt = "");
void buildComb_fast(const vector<vector<double> > &allVecs, vector<vector<double> > &result, size_t vecIndex = 0, const vector <double> vSoFar = vector <double>());
void buildComb(const vector<vector<TString> > &allVecs, vector<TString > &result, size_t vecIndex = 0, TString strSoFar = "");
void buildComb(const vector< double > &elms, vector<vector<double> > &result, string opt = "");
void buildComb(int min, int max, int k, vector<vector<double> > &result, string opt = "");


double get_signal(TTree * tree, TString plot, TString cut, TString weight, double norm);
double get_background(TTree * tree, TString plot, TString cut, TString weight, double norm);


typedef  double (*FUNC_GETN_PTR)(TTree *, TString, TString, TString, double);


class CutOptimizer {

    TString analysis, vplot;
    TTree *treeSig, * treeBkg;
    TCut baseCut, sigCut, sideBandCut, baseSigCut, baseBkgCut;
    double sigNorm, bkgNorm;
    TString MCweight;
    int nSteps;
    vector<RooRealVar * > vars, orig_vars;
    string cut_to_optimize;
    string fmerit;
    bool print;
    TFile * ofile;
    TTree * points;
    TreeReader * reader;
    vector <vector <double > > pts_to_scan;
    vector <double> optimalW;
    vector <double> step_sizes;
    double maxSig, totS, totB, maxEff, maxBkgRej, pasB, pasS;

    FUNC_GETN_PTR get_sig;
    FUNC_GETN_PTR get_bkg;

    double get_significance(double S, double B);
    void scan_points(string mode = "-optimise");

public :

    CutOptimizer(TString _analysis, TTree *_treeSig, TTree *_treeBkg,
                 vector<RooRealVar *> _vars, TString _mycut, TCut _sigCut, TCut _sideBandCut, TCut _baseCut = "",
                 double _sigNorm = 1., double _bkgNorm = 1.,
                 TString _MCweight = "", int _nSteps = 4,
                 string _fmerit = "significance", bool _print = 0);

    TString GetOptimalCut()
    {
        if (optimalW.empty()) { cout << "You have to run the otimisation first!!" << endl; return "";}
        else
        {
            vector <TString> varnames;
            for (auto v : vars) varnames.push_back(v->GetName());
            return subStrings(cut_to_optimize, varnames, optimalW);
        }
    }

    vector <double> GetOptimalPoint() { return optimalW; }
    vector <double> GetStepSizes() { return step_sizes; }

    void SetControlVariable(TString var) { vplot = var; }
    void SetNSteps(int nsteps) { ResetSteps(nsteps); }
    void SetFoM(TString myfmerit) { fmerit = myfmerit; }
    void SetBkgFunction(FUNC_GETN_PTR my_get_bkg)
    {
        get_bkg = my_get_bkg;
        totB = get_bkg(treeBkg, vplot, (TString) (baseBkgCut), "", bkgNorm);
    }

    void SetSigFunction(FUNC_GETN_PTR my_get_sig)
    {
        get_sig = my_get_sig;
        totS = get_sig(treeSig, vplot, (TString) (baseSigCut), MCweight, sigNorm);
    }

    void ResetSteps(unsigned nsteps);

    vector <double>  optimise(string option = "");
    vector <double>  iterative_optimise(double precision = 0.01, string option = "-noperc");

    void ClosePrintAndSave(string option = "");
};

#endif
