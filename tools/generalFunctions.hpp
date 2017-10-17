#ifndef GENERAL_FUNCTIONS_HPP
#define GENERAL_FUNCTIONS_HPP

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
#include "RooAddPdf.h"

#include "treeReader.hpp"
#include "efficiencyFunctions.hpp"

using namespace std;
using namespace RooFit;


bool genRndm(RooRealVar * var, RooCurve * curve, double * xval, double ymax, TRandom3 * rdm_x = new TRandom3(0), double smear = -1);

TTree * generate(RooArgSet * set, RooAbsPdf * pdf, int nevt, string opt = "");
TTree * generate(RooArgSet * set, RooAbsPdf * pdfSig, float nsig, RooAbsPdf * pdfBkg, float nbkg, string opt = "");
RooDataSet * generateDataSet(TString name, RooArgSet * set, RooAbsPdf * pdfSig, int nsig, RooAbsPdf * pdfBkg, int nbkg, string opt = "");
RooDataSet * generateDataSet(TString name, RooArgSet * set, RooAbsPdf * pdf, int nevt, string opt = "");

double luminosity( TString namefile, string doerr = "lumi" );
double luminosita( vector< TString > namefile, string doerr = "lumi" );

double * calcChi2(RooPlot * frame, unsigned npar = 0, double * range = NULL, bool extended = true);
double residual( double datum, double pdf );
double pull( double datum, double pdf );
TH1D* residualHist( const RooHist* rhist, const RooCurve* curve, float * range = NULL, string opt = "");
TH1 * getPulls(RooPlot * pl, float * range = NULL, string opt = "", string data = "data", string model = "model");

vector<float> computeAverage(TH1* hist);
vector<float> computeAverage2D(TH2* hist2D);

vector<TH1*> createRandomFluctuations(TH2 * input, int nRandom = 1000, char limit = 'E');

TH1 * getSliceX(TH2 *hHisto, double slice);
TH1 * getSliceX(TH2 *hHisto, int bin);

vector <double> scalarProd(vector<double> v, double c);
float computeEta(float P, float PT);

void cleanHistos(TH1 &histo, float cut, float min, float max);
void shiftHistos(TH1 &histo, float shift);
void uniformBins(TH1 *histo);
void addSystematicError(TH1 &histo, float sysError);
void divideForBinCenter(TH1 &histo, float cost = 0.);

float computeMCError(bool mean, char* formula, float best, Double_t *param, Double_t *err, int ntry, bool gauss = false, bool valAss = false);

TH1F * rebinHisto(TH1 &histo, float init, vector<float > sizes, char* title = 0, bool media = false);
TH1F * rebinHisto(TH1 &histo, TH1 &tamplateHisto, char* title = 0, bool media = false);
TH1F * rebinHisto(TH1 &histo, float error, char* title = 0, bool media = false);

TH1* setLabels(TH1* histo, vector<float > &labels, int nDiv, float dNmax);

vector<string> getFilesNames(string filename);

double * decodeBinning(string str, int * _nbins = NULL, string opt = "unif");

/**
  This functions extracts from a root file the objects with names in the "nameHistos" vector
  */

template <class T> vector<T *> getHistosFromFile(vector<string> nameHistos, string nameFile, string path)
{

    TFile * file = new TFile(nameFile.c_str());
    vector<T *> histos;

    for (size_t i = 0; i < nameHistos.size(); i++)
    {
        if (path != "") {file->cd(path.c_str()); cout << path.c_str() << endl;}

        TH1 * h = (T *)gDirectory->Get(nameHistos[i].c_str());
        if (h == 0) {cout << "ERROR at " << nameHistos[i].c_str() << endl; continue;}
        else histos.push_back(h);
    }

    return histos;
}



/**
  Shows the percentage of work done, usefull in loops
start: is an object of time_t type marking the beginning of the program
ntimes: sets how often to print the percentage. If not set prints every entry
dobar: allows to desable the progress bar
doentry: allow to desable the dispay of the current entry
*/

inline double showPercentage(int ientry, int nentries, time_t start = 0, int ntimes = 2000, bool dobar = true, bool doentry = true, string title = "")
{
    static string normalcolor = "\033[0;0m";
    static string barcolor = "\033[2;42m\033[1;33m";
    static string timecolor = "\033[1;31m";
    static int first_entry = ientry;

    static int div = (nentries - first_entry) / ntimes;
    if ( div < 1. ) div = 1;

    int myentry = ientry - first_entry;
    if ( ((int)(myentry) % div) == 0 || myentry == 0 )
    {
        //static int i = 0;
        //if(i > 3) i = 0;
        //char c[] = {'|','/','-','\\'};
        //cout << "\r" << c[++i];

        double perc = (double)ientry / (nentries - 1.);

        cout << "\r| Working... " << title << " | " << fixed << setprecision(1) << perc * 100. << "% ";

        if (dobar)
        {
            cout << barcolor << "[";
            for (int p = 0; p < perc * 20; p++) cout << ">";
            for (int p = 0; p < (1 - perc) * 20; p++) cout << "_";
            cout << "]" << normalcolor << "  ";
        }


        if (doentry) cout << "Entry #" << ientry + 1;
        if (start != 0)
        {
            time_t end = time(NULL);
            double dt = difftime(end, start);
            cout << "  (";

            double t_left = ((double)nentries / ientry - 1.) * dt;

            cout << "~" << timecolor;
            if (t_left > 60.) cout << t_left / 60. << normalcolor << " min to the end)";
            else cout << t_left << normalcolor << " s to the end)";
            cout << flush;
        }

        if ( ientry == (nentries - 1) ) cout << endl;
        return perc;
    }

    if ( ientry == (nentries - 1) ) cout << endl;
    return 100.;
}

#endif
