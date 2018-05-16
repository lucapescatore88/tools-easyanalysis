#ifndef __CINT__
#include "RooGlobalFunc.h"
#endif

#ifndef ANALYSER_HPP
#define ANALYSER_HPP

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <time.h>
#include <typeinfo>
#include <vector>

#include "TBranch.h"
#include "TCanvas.h"
#include "TCut.h"
#include "TEntryList.h"
#include "TFile.h"
#include "TH1.h"
#include "TMath.h"
#include "TRandom3.h"
#include "TString.h"
#include "TTree.h"

#include "RooAbsData.h"
#include "RooAbsDataStore.h"
#include "RooAbsPdf.h"
#include "RooAddPdf.h"
#include "RooAddition.h"
#include "RooCBShape.h"
#include "RooChebychev.h"
#include "RooChi2Var.h"
#include "RooConstVar.h"
#include "RooDataHist.h"
#include "RooDataSet.h"
#include "RooExponential.h"
#include "RooFFTConvPdf.h"
#include "RooFit.h"
#include "RooFitResult.h"
#include "RooGaussian.h"
#include "RooHistPdf.h"
#include "RooKeysPdf.h"
#include "RooMinuit.h"
#include "RooNumConvPdf.h"
#include "RooPolynomial.h"
#include "RooProdPdf.h"
#include "RooProduct.h"
#include "RooRandom.h"
#include "RooRealVar.h"
#include "RooStats/SPlot.h"
#include "RooWorkspace.h"

#include "generalFunctions.hpp"
#include "modelBuilder.hpp"
#include "treeReader.hpp"

using namespace std;
using namespace RooFit;
using namespace RooStats;


typedef  Long64_t (*FUNC_PTR)(TreeReader *, vector<Long64_t>);

Long64_t randomKill(TreeReader * reader, vector< Long64_t > entry);


/** \class Analysis
 *  \brief Allows to handle data starting from TTree or TH1
 *  allows to make cuts and check for multiple candidates.
 *  Then using the ModelBuilder class it allows to create a model
 *  and finally you can Fit it to the dataset with advanced options.
 * */

class Analysis : public ModelBuilder {

    TCut m_cuts;
    double * m_chi2;
    vector <double> m_regions;
    vector <string> m_regStr;
    map <string, vector<double>> m_reg;
    bool m_init;
    string m_unit;
    static string m_pmode;
    RooAbsReal * m_weight = NULL;
    RooDataSet * m_data = NULL;
    RooFitResult * m_fitRes = NULL;
    double m_fitmin;
    double m_fitmax;
    vector<RooRealVar*> m_datavars;

    TreeReader * m_dataReader = NULL;
    TTree * m_reducedTree = NULL;
    TH1 * m_dataHist = NULL;

    double m_scale;

public:

    Analysis(TString _name, TString _title, RooRealVar * _var, string _opt = "", string _w = "", TCut _cuts = ""):
        ModelBuilder(_name, _var, _title), m_cuts(_cuts), m_chi2(new double[2]), m_init(false), m_unit(""),
        m_weight(NULL), m_data(NULL), m_fitRes(NULL), m_fitmin(0.), m_fitmax(0.),
        m_dataReader(NULL), m_reducedTree(NULL), m_dataHist(NULL), m_scale(1)
    {
        if (!m_var) SetVariable(new RooRealVar("x", "", 0));
        m_datavars.push_back(m_var);
        if (_w != "") SetWeight((TString)_w);
    };

    Analysis(TString _name, RooRealVar * _var, RooAbsPdf * _pdf = NULL, int _ngen = 1000, string _opt = ""):
        Analysis(_name, _name, _var, _opt)
    {
        if (_pdf)
        {
            SetModel(_pdf);
            Generate(_ngen, _opt);
            m_init = true;
            ForceValid();
        }
    };

    Analysis(TString _name, TString _title, TreeReader * reader, TCut _cuts = "", RooRealVar * _var = NULL, string _w = ""):
        Analysis(_name, _title, _var, "", _w, _cuts)
    {
        m_dataReader = reader;
        if (!m_dataReader) { cout << m_name << ": *** WARNING Analysis *** Your TreeReader is NULL, this is going to break!" << endl; return; }
        if (!m_dataReader->isValid()) m_dataReader->Initialize();
        m_reducedTree = (TTree *) m_dataReader->GetChain();
    };

    Analysis(TString _name, TString _title, TTree * tree, TCut _cuts = "", RooRealVar * _var = NULL, string _w = ""):
        Analysis(_name, _title, new TreeReader(tree), _cuts, _var, _w)
    {};

    Analysis(TString _name, TString _title, TChain * tchain, TCut _cuts = "", RooRealVar * _var = NULL, string _w = ""):
        Analysis(_name, _title, new TreeReader(tchain), _cuts, _var, _w)
    {};

    Analysis(TString _name, TString _title, string treename, string filename, TCut _cuts = "", RooRealVar * _var = NULL, string _w = ""):
        Analysis(_name, _title, new TreeReader(treename.c_str(), filename.c_str()), _cuts, _var, _w)
    {};

    Analysis(TString _name, TString _title, TH1D * histo, RooRealVar * _var = NULL):
        Analysis(_name, _title, _var)
    {
        m_dataHist = (TH1 *) histo;
    };

    Analysis(TString _name, TString _title, RooDataSet * dd, RooRealVar * _var = NULL):
        Analysis(_name, _title, _var)
    {
        m_data = (RooDataSet *) dd;
    };

    /// \brief Special constructor for single quick fit
    Analysis(TString _name, TString _title, RooDataSet * dd, RooRealVar * _var, RooAbsPdf * _sig, string _w = "", string _opt = ""):
        Analysis(_name, _title, _var, _opt, _w)
    {
        m_data = (RooDataSet *) dd;
        if (_sig) { SetSignal(_sig); Initialize(""); }
    };

    /// \brief Special constructor for single quick fit
    Analysis(TString _name, TString _title, TTree * dd, RooRealVar * _var, RooAbsPdf * _sig, string _w = "", string _opt = ""):
        Analysis(_name, _title, _var, _opt, _w)
    {
        m_dataReader = new TreeReader((TTree *) dd);
        m_dataReader->Initialize();
        m_reducedTree = (TTree *) dd;
        CreateDataSet();

        if (_sig) { SetSignal(_sig); Initialize(""); }
    };

    ~Analysis()
    {
        //if(m_dataReader) delete m_dataReader;
        //if(m_reducedTree) delete m_reducedTree;
        //if(m_dataHist) delete m_dataHist;
    };


    static string GetPrintLevel() { return m_pmode; }
    static void SetPrintLevel(string mode) { m_pmode = mode; ModelBuilder::SetPrintLevel(mode); TreeReader::SetPrintLevel(mode);  }

    void CreateReducedTree(string option = "", TCut cuts = "", double frac = -1.);
    TTree * GetReducedTree() { return m_reducedTree; }

    TreeReader * GetTreeReader() { return m_dataReader; }

    void CreateDataSet(string opt = "", TCut cuts = "");
    RooDataSet * GetDataSet(string opt = "")
    {
        if (opt.find("-recalc") != string::npos || !m_data) CreateDataSet(opt);
        return m_data;
    }
    void SetDataSet( RooDataSet * d ) { m_data = d; }

    void CreateDataHisto(double min = 0, double max = 0, int nbin = 50, TCut cuts = "", string weight = "", string opt = "", TH1 * htemplate = NULL);
    TH1 * GetDataHisto(string opt = "")
    {
        if (opt.find("-recalc") != string::npos || !m_dataHist) CreateDataHisto();
        return m_dataHist;
    }
    void SetDataHisto( TH1 * h ) { m_dataHist = h; }

    /** \brief Sets units finding the scale factor too
     *  @param inUnit: unit of the input data. Based on the inUit-outUnit difference the scale factor is found. <br>Works only with masses in eV-PeV.
     *  For any other unit use SetUnits(string outUnit, double scalefactor)
     *  @param outUnit: is a label for the unit"units" is a string with the unit label
    **/
    void SetUnits(string inUnit, string outUnit);

    /** \brief Set units
     *  @param outUnit: is a label for the unit"units" is a string with the unit label
        @scalefactor: factor to rescale input data if they are given in a unit different than the one you want
     **/
    void SetUnits(string outUnit);//, double scalefactor = 1);
    string GetUnits() { return m_unit; }

    double GetScale() { return m_scale; }

    /** \brief Adds a variable to the internal DataSet
     * An Analysis object can also be used to create RooDataSet objects to use then somewhere else. Therefore it comes usefull to have in the RooDataSet more than one variable. This can me added simpy by name (if it exists in the input tree).
     * */
    void AddVariable(RooRealVar * v)
    {
        for (auto vv : m_datavars) if (vv->GetName() == v->GetName()) return;
        m_datavars.push_back(v);
    }
    void AddVariable(TString vname)
    {
        AddVariable(new RooRealVar(vname, vname, 0.));
    }
    void AddAllVariables();

    /** Function to unitialize the Analysis object before fitting
     *  <br> Runs ModelBuilder::Initialize() to initialize the model
     *  <br> Runs CreatedataSet() to initialize the data
     **/
    bool Initialize(string _usesig = "-exp", double frac = -1.);

    bool isValid() { return m_init; }

    RooAbsReal * CreateLogL(RooCmdArg extended)
    {
        if (m_model && m_data) return m_model->createNLL(*m_data, extended);
        else return NULL;
    }

    void SetWeight( TString w ) { m_weight = new RooRealVar(w, w, 1.); return; }
    string GetWeight() { if (!m_weight) return ""; else return m_weight->GetName(); }

    void SetCuts( TCut cuts ) { m_cuts = cuts; }
    void SetCuts( TString cuts ) { m_cuts = TCut(cuts); }

    RooWorkspace * SaveToRooWorkspace(string option = "");

    void ImportModel(RooWorkspace * ws);
    void ImportModel(RooWorkspace * wsSig, RooWorkspace * wsBkg);
    void ImportData(RooWorkspace * ws);

    /** \brief Adds a blinded region.
     If you have to add more than one region you must add them in order from low to high and the must not overlap. If you set one or more regions parameters will be hidden and model and data will not be plotted in those regions.
     **/
    void SetBlindRegion(double min, double max);
    void Reset()
    {
        m_regions.clear(); ClearBkgList();
        ResetVariable(); m_datavars.clear(); m_datavars.push_back(m_var);
        m_chi2[0] = m_chi2[1] = -1; m_sig = NULL, m_bkg = NULL; m_init = false;
    }

    /** \brief Generates events using the model internally set and a specific number of total events
     * @param nevt: Number of events to generate (N.B.: this is Nsig+Nbkg and the fraction between is supposed to be right in the model)
     * @param opt: Options:
     * <br> "-seed(n)":  To set a specific seed
     * <br> "-smear(r)": "r" is a double interpreted as resolution. The generated events will be gaussian smeared by this resolution.
     **/
    TTree * Generate(int nevt, string opt = "");

    /** \brief Generate events using the model inetrnally set and a specific number of signal and background events
         * @param nsigevt: Number of signal events to generate
         * @param nbkgevt: Number of background events to generate
         * @param opt: Options:
         * <br> "-seed(n)":  To set a specific seed
         * <br> "-smear(r)": "r" is a double interpreted as resolution. The generated events will be gaussian smeared by this resolution.
         **/
    TTree * Generate(double nsigevt, double nbkgevt, string opt = "");

    /** \brief Allows to apply cuts on "dataReader" and returnes a tree containing only events which pass the cut.
      @param substtree: if you want to set the three obtained as "reducedTree" (default = true)
      @param addFunc: you may define a function getting a TreeReader and a TTree which is called in the loop and adds variables to the new tree combining information from the old tree
      <br> N.B.: addFunc is called once before the loop and here you should set static addresses.
     @param frac: uses only the fraction "frac" of the available entries
     **/
    TTree * ApplyCuts(TCut _cuts = NULL, bool substtree = true, void (*addFunc)(TreeReader *, TTree *, bool) = NULL, double frac = 1);
    TTree * ApplyFunc(void (*addFunc)(TreeReader *, TTree *, bool), double frac = 1);


    /** \brief Checks for multiple candidate in the same event
     * */
    /** Also creates a plot of number of candidates and a new tree with a variable "isSingle" added. This will be 1 for the best candidate and 0 for the others.
    N.B.: if you just need to apply cuts ad not to check for multiples USE "ApplyCuts" it's much more efficient.

    randomKill() is a standard function for random killing of multiple candidates.
    However you can make your own function and pass it to the method.
    Function template should be:

    Long64_t choose(TreeReader * reader, vector< Long64_t > entry )
    {
    //Typycally loop on entry and do something with reader
    return chosen_entry;
    }
    **/
    TTree * GetSingleTree(FUNC_PTR choose = NULL, TString namevar = "", bool reset = false);

    TTree * GetSingles(vector<FUNC_PTR> choose, vector <TString> namevars)
    {
        if (choose.size() != namevars.size()) { cout << m_name << ": *** WARNING GetSingles *** The vector of choose functions and their names must have the same size" << endl; return NULL; }
        for (size_t cf = 0; cf < choose.size(); cf++)
            GetSingleTree(choose[cf], namevars[cf], true);
        return m_reducedTree;
    }


    /** \brief Fits the "reducedTree" with the "model"
     * Data and model must be previourly set and initialized.
      <br>One can set the fit range and number of bins for chi2. If not set the variable range is used.
      @param min, max: fitting interval, if min => max all available is used
      @param nbins: n of bins to use (if unbinned this is only for display)
      @param unbinned: true for unbinned fit
      @param print: Print options (includes also all options of ModelBuilder::Print() and GetFrame())
      <br>"-quiet"      -> shell output minimized
      <br>"-sumW2err"   -> if weighted data errors shown reflect statistics of initial sample
      <br>"-log"        -> logarithmic plot
      <br>"-pulls" or <br>"-ANDpulls" -> if data is inserted these add a pull histogram -pulls in other plot -ANDpulls under fit plot
      <br>"-range"      -> plots only the fitted range, otherwise all available is plot
      <br>"-noPlot"     -> doesn't print and only returns the frame
      <br>"-minos"      -> Enables MINOS for asymmetric errors
      @param cuts: cuts to make before fitting
     **/
    RooPlot * Fit(unsigned nbins = 50, bool unbinned = false, string print = "-range-log", TCut mycuts = "");
    RooPlot * Fit(string option, TCut extracuts = TCut(""));

    RooFitResult * GetFitRes() { return m_fitRes; }
    void SetFitRes(RooFitResult *fitRes = NULL) { m_fitRes = fitRes; }

    /** \brief Makes nice plots of data and models including blinded plots
      @param do_model: if true plots model on data
      @param opt: options string. Options available are:
      <br>"-fillSig"  -> signal is filled with color instead of dashed line
      <br>"-fillBkg"  -> bkg is filled with color instead of dashed line
      <br>"-log"      -> logarithmic plot
      <br>"-pulls" or "-ANDpulls" -> if data is inserted these add a pull histogram -pulls in other plot -ANDpulls under fit plot
      <br>"-none"     -> doesn't print and only returns the frame
      <br>"-LHCb"     -> Prints LHCb name on the plot
      @param data: data to plot -> If no data use NULL
      @param bins: number of bins to use for data
      @param Xtitle: X axis label
      @param title: title
     **/
    RooPlot * Print(bool dom_model,  RooAbsData * _data,  string opt = "-range-keepname", unsigned bins = 50, TString Xtitle = "", TString title = "", RooRealVar * myvar = NULL);
    RooPlot * Print(string opt = "-range-keepname", unsigned bins = 50, TString Xtitle = "", TString title = "", RooRealVar * myvar = NULL);
    RooPlot * Print(RooRealVar * myvar, string option = "", unsigned bins = 50, TString Xtitle = "", TString title = "");
    RooPlot * PrintAndCalcChi2(int nbins, string print, RooAbsData * mydata = NULL );

    void PrintComposition(float min, float max)
    {
        if (m_fitRes) ModelBuilder::PrintComposition(min, max, m_fitRes);
        else cout << m_name << ": *** WARNING PrintComposition *** Fit didn't happen yet, composition unkown!" << endl;
    }
    double GetNSigVal(double min = 0, double max = 0, double * valerr = NULL)
    {
        if (!m_init || !m_fitRes) return -1;
        return ModelBuilder::GetNSigVal(min, max, valerr, m_fitRes);
    }
    double GetNSigErr(double min = 0, double max = 0)
    {
        double valerr = 0;
        GetNSigVal(min, max, &valerr);
        return valerr;
    }
    double GetSOverB(float min, float max, double * valerr = NULL)
    {
        return ModelBuilder::GetSOverB(min, max, valerr, m_fitRes);
    }
    double GetSigFraction(float min, float max, double * valerr = NULL)
    {
        return ModelBuilder::GetSigFraction(min, max, valerr, m_fitRes);
    }

    double GetNBkgVal(double min = 0, double max = 0, double * valerr = NULL)
    {
        if (!m_init || !m_fitRes) return -1;
        return ModelBuilder::GetNBkgVal(min, max, valerr, m_fitRes);
    }
    double GetNBkgErr(double min = 0, double max = 0)
    {
        double valerr = 0;
        GetNBkgVal(min, max, &valerr);
        return valerr;
    }

    RooRealVar * GetPar(string name) { return getParam(m_model, name); }
    double GetParVal(string name)
    {
        RooRealVar * par = GetPar(name);
        if (par) return par->getVal();
        else { cout << m_name << ": *** WARNING GetParVal *** Parameter " << name << " not found!" << endl; return -999999; }
    }
    double GetParErr(string name)
    {
        RooRealVar * par = GetPar(name);
        if (par) return par->getError();
        else { cout << m_name << ": *** WARNING GetParErr *** Parameter " << name << " not found!" << endl; return -999999; }
    }

    ///\brief Returns chi2/NDF
    double GetChi2();
    ///\brief Returns NDF
    double GetNDF();
    void PrintChi2();
    //Returns chi2 probability
    double GetProb();

    RooDataSet * GetParamsVariations(int nvariations = 10000)
    {
        return ModelBuilder::GetParamsVariations(nvariations, m_fitRes);
    }

    /** \brief Calculates S-weight for the data and model set
        The function will perform a fit but it could help to Fit first anyway.
        @param min,max: Calculates S-weight only in [min,max]
        @param nbins:   Number of bins (just for display if unbinned fit)
        @param unbinned: If true performs unbinned fit
        @pram options: Options:
        "-nofit" doesn't perform the fit
     **/

    RooDataSet * CalcSWeight(unsigned nbins = 50, bool unbinned = false, string option = "");

};


class Scaler {

    static double _scale;
    static RooRealVar * _var;

public:

    static void Set( double scale, RooRealVar * var )
    {
        _scale = scale;
        _var = var;
    }
    static void Scale(TreeReader * reader, TTree * tree, bool reset)
    {
        static float unscaled;
        if (reset) tree->Branch((TString)_var->GetName() + "_unscaled", &unscaled, (TString)_var->GetName() + "_unscaled/F");
        else
        {
            float value = reader->GetValue(_var->GetName());
            unscaled = value;
            reader->SetValue(_var->GetName(), value * (float)_scale);
        }
    }
};

#endif
