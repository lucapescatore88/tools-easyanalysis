#ifndef MY_FELDMAN_COUSINS_HPP
#define MY_FELDMAN_COUSINS_HPP

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
#include "TGraph2D.h"
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
#include "TPolyLine.h"
#include "TLine.h"
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
#include "RooCategory.h"
#include "RooSimultaneous.h"
#include "RooFitResult.h"
#include "RooNLLVar.h"
#include "RooMinuit.h"

#include "treeReader.hpp"
#include "modelBuilder.hpp"

using namespace std;
using namespace RooFit;


// typedef bool (*FUNC_PTR)( Str2VarMap );
typedef bool (*ISVALIDF_PTR)( RooAbsPdf * );
typedef RooFitResult * (*FITFUNC_PTR)( RooAbsPdf *, RooDataSet *, Str2VarMap, ISVALIDF_PTR isValid, string opt, int nfree, RooArgSet *, RooAbsReal * );

//vector<double> extractLimits(TGraph * gr, double central, double CL);

/// \brief Default function for fitting
/// You can set your custom one using myFeldmanCousins::SetFitFunc
RooFitResult * defFit(RooAbsPdf * pdf, RooDataSet * data, Str2VarMap p, ISVALIDF_PTR isValid = NULL, string opt = "", int nfree = -1, RooArgSet * cons = NULL, RooAbsReal * mynll = NULL);


/** \class myFeldmanCousins
 *  \brief Implements the extraction of a pvalues grid usig the Feldman-Cousins plug-in method
 *  */

class myFeldmanCousins
{
    TString name;
    vector<RooDataSet *> datas;
    vector<RooAbsPdf *> pdfs;
    vector< TString > categories;
    RooDataSet * combData;
    RooAbsPdf * combPdf;
    RooCategory * samples;

    Str2VarMap PoIs;
    RooRealVar * PoI;
    RooRealVar * PoI2;
    RooArgSet * obs;

    TString weight;
    unsigned npoints;
    unsigned nexp;
    int nfree;
    RooArgSet * cons;
    FITFUNC_PTR myfit;

    RooArgSet * origPars;
    RooAbsReal * nll;

    ISVALIDF_PTR isValid;
    double dataratio;

    /** \brief Generates toy MC events with a PDF given by the internal model and fits them returns the percentage of toys where the minimum Log Likeliood was found to be worse than in data
     * @param testVal, testVal2: Current parameters of interest to be used for generation
     * @param dataratio :        Ratio of min LL with free PoIs / fixed PoIs in data
     * @param h:                 if != NULL it stores the minLL distirbution in the h histogram
     * @param isValid :          a boolean values funcion can be passed that returns true if the current combination of PoIs is allowed (e.g. to implement physical boundaries)
     * */
    double generateToys(double testVal, double testVal2, TH1 * h = NULL, string opt = "");

public:

    /// \brief Builts the combined PDF and model
    void Initialize();
    /// \brief Builts the combined PDF and model and performs the fit
    /// In case you want to perform the by youself use Initialize()
    void InitializeAndFit();

    /// \brief Construtor for multiple samples and PoIs (most general)
    myFeldmanCousins(TString _name, vector<TString> _categories, vector <RooDataSet *> _datas, vector <RooAbsPdf *> _pdfs, RooRealVar * _obs, Str2VarMap _PoIs, TString _weight = ""):
        name(_name), PoIs(_PoIs), weight(_weight), npoints(11), nexp(100), nfree(-1), cons(NULL), origPars(NULL), nll(NULL), isValid(NULL), dataratio(-1)
    {
        obs = new RooArgSet(*_obs);
        if (PoIs.size() < 1) cout << "ATTENTION: You must set at least one PoI!" << endl;
        else
        {
            Str2VarMap::iterator it = PoIs.begin();
            PoI = (RooRealVar *)it->second;
            if (PoIs.size() == 1) PoI2 = NULL;
            if (PoIs.size() == 2) { it++; PoI2 = (RooRealVar *)it->second; }
            else if (PoIs.size() > 2) cout << "Sorry only 2 PoIs are allowed. The others will be ignored." << endl;
        }
        datas.insert(datas.end(), _datas.begin(), _datas.end());
        pdfs.insert(pdfs.end(), _pdfs.begin(), _pdfs.end());
        categories.insert(categories.end(), _categories.begin(), _categories.end());
        myfit = &defFit;
        Initialize();
    };

    /// \brief Construtor for multiple samples and just one PoI
    myFeldmanCousins(TString _name, vector<TString> _categories, vector <RooDataSet *> _datas, vector <RooAbsPdf *> _pdfs, RooRealVar * _obs, RooRealVar * _PoI, TString _weight = ""):
        name(_name), PoI(_PoI), PoI2(NULL), weight(_weight), npoints(11), nexp(100), nfree(-1), cons(NULL), origPars(NULL), nll(NULL), isValid(NULL), dataratio(-1)
    {
        obs = new RooArgSet(*_obs);
        PoIs[PoI->GetName()] = PoI;
        datas.insert(datas.end(), _datas.begin(), _datas.end());
        pdfs.insert(pdfs.end(), _pdfs.begin(), _pdfs.end());
        categories.insert(categories.end(), _categories.begin(), _categories.end());
        myfit = &defFit;
        Initialize();
    };

    /// \brief Construtor for just one sample and one PoI
    myFeldmanCousins(TString _name, RooDataSet * _data, RooAbsPdf * _pdf, RooRealVar * _obs, RooRealVar * _PoI, TString _weight = ""):
        name(_name), PoI(_PoI), PoI2(NULL), weight(_weight), npoints(11), nexp(100), nfree(-1), cons(NULL), origPars(NULL), nll(NULL), isValid(NULL), dataratio(-1)
    {
        obs = new RooArgSet(*_obs);
        PoIs[PoI->GetName()] = PoI;
        datas.push_back(_data);
        pdfs.push_back(_pdf);
        categories.push_back("data");
        myfit = &defFit;
        Initialize();
    };


    ///\brief Sets the number of points to scan along the PoIs
    void SetNPointsToScan( int _npoints ) { npoints = _npoints; }
    /**\brief Sets the number o toy experiments
    * More they are and more precision your pvalue has but takes longer
    * With 1000 toys (default) the precision is 1./sqrt(1000) ~ 3%
    * */
    void SetNExp( double _nexp ) { nexp = _nexp; }
    ///\brief Variable in the dataset to be interpreted as event-by-event weight
    void SetWeight( TString w ) { weight = w; }
    /// \brief To set a custom fit function
    void SetFitFunc( FITFUNC_PTR afitfunc ) { myfit = afitfunc; }
    /// \brief Adds a set of external constraints to the LogL
    void AddConstraints( RooArgSet * _cons ) { cons = _cons; }
    /// \brief Needed if the PDF has more than one observable
    /// The constructors only fill in one observable
    void AddObservable( RooRealVar * newobs ) { obs->add(*newobs); }
    /// \brief returns the number of free parameters in the current combined PDF
    int GetNFreePars() { nfree = getNFreePars(combPdf, *obs); return nfree; }

    /// \brief Draws the LogL vs observables in 1D (for each variable)
    void DrawLogL();

    /** \brief Main function to extract the pvalues grid after cunstructing and initializing the object.
     * @param points:   You can give a set of custom points to scan. If this is empty a regular grid is generated.
     * @param origPars: Original parameters. This is needed in the case you did some fitting before and the current parameters of the PDF are not/ the initial ones. This is important because each fit to data and toys has to begin with the same initial parameters to be compared. If this is empty it copied the current parameters of the PDF at the initialization moment.
     * @param point: Returns the point of best fit
     * @param opt  : Options
     * @param isValid:
     * @param nll_val: Allows to set an external value for the min LL of data. Can reduce execution time. mostly for internal use.
     * */
    vector< vector < double > > ExtractLimits( vector< vector < double > > points, RooArgSet * origPars = NULL, TGraph ** point = NULL, string opt = "", ISVALIDF_PTR isValid = NULL, double nll_val = -1 );
    vector< vector < double > > ExtractLimits(TGraph ** point = NULL, string opt = "", ISVALIDF_PTR isValid = NULL)
    {
        return ExtractLimits( vector< vector < double > >(), NULL, point, opt, isValid, -1 );
    }


};


#endif
