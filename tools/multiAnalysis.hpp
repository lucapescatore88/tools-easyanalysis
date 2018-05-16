#ifndef __CINT__
#include "RooGlobalFunc.h"
#endif

#ifndef MULTIANALYSER_HPP
#define MULTIANALYSER_HPP

#include <vector>
#include <map>
#include <sstream>
#include <iostream>
#include <string>
#include <time.h>
#include <iomanip>
#include <typeinfo>

#include "TCanvas.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1.h"
#include "TEntryList.h"
#include "TCanvas.h"
#include "TBranch.h"
#include "TCut.h"
#include "TMath.h"
#include "TString.h"

#include "RooGenericPdf.h"
#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooGaussian.h"
#include "RooConstVar.h"
#include "RooChebychev.h"
#include "RooAddPdf.h"
#include "RooSimultaneous.h"
#include "RooCategory.h"
#include "RooMsgService.h"

#include "analysis.hpp"
#include "generalFunctions.hpp"
#include "modelBuilder.hpp"
#include "treeReader.hpp"

using namespace RooFit;
using namespace RooStats;


/** \class MultiAnalysis
 *  \brief This class can be used to do simultaneous fits to more than 1 sample (in 1D)
 *  <br>Can also be used fitting different variables for each sample.
 *  <br>Finally nothing forbids to use the class to fit "simultaneously" different variables of the same sample. But just be aware that in this case it works well only if the PDFs of the various 1D fits are uncorrelated (namely PDF in N-d = prod(2D PFDs))
 * */

class MultiAnalysis {

    TString m_name;
    vector< Analysis * > m_ana;
    vector< TString > m_categories;
    RooArgSet * m_vars = NULL;
    RooDataSet * m_combData = NULL;
    RooCategory * m_samples = NULL;
    RooSimultaneous * m_combModel = NULL;
    bool m_init;
    bool m_isToy;
    RooFitResult * m_fitResult = NULL;
    RooArgSet * m_constr = NULL;
    static string m_pmode;

    bool m_unbinned = true;
    int  m_nBins    = 100;

public:

    MultiAnalysis( TString _name ):
        m_name(_name), m_init(false), m_isToy(false), m_fitResult(NULL)
    {
        m_samples = new RooCategory("samples", "samples");
        m_vars    = new RooArgSet("vars");
        m_constr  = new RooArgSet("constraints");
    };

    ~MultiAnalysis()
    {
        delete m_vars;
        delete m_combData;
        delete m_combModel;
        delete m_samples;
        m_ana.clear();
    };

    void SetBinnedFit(int _nBins) {
        m_unbinned = false;
        m_nBins    = _nBins;
    }
    int GetBins() { return m_nBins; }

    RooDataSet * GetCombData() { return m_combData; }
    void SetCombData(RooDataSet * data) { m_combData = data; };
    void ImportData(RooWorkspace * ws);

    RooAbsPdf * GetCombModel() { return m_combModel; }
    void SetCombModel(RooSimultaneous * model) { m_combModel = model; };
    void ImportModel(RooWorkspace * ws);

    RooDataSet * Generate(int nevts, string option);

    void EnlargeYieldRanges(double factor);

    TString GetName() { return m_name; }
    void SetName(TString _name) { m_name = _name; }

    RooFitResult * GetFitResult() { return m_fitResult; }
    RooAbsReal * CreateLogL()
    {
        if (m_combModel && m_combData) return m_combModel->createNLL(*m_combData);
        else return NULL;
    }
    void DrawLogL(RooRealVar * PoI) {
        TCanvas * c = new TCanvas();
        RooAbsReal * nll =  CreateLogL();
        double nll_val = nll->getVal();
        RooPlot * nllPlot = PoI->frame();
        nll->plotOn(nllPlot, Name("LogL"));
        nllPlot->SetTitle("");
        nllPlot->SetMinimum(nll_val * 0.95);
        nllPlot->Draw();
        c->Print(m_name + "_LogL_vs_" + (TString)PoI->GetName() + ".pdf");
    }
    const RooAbsReal * ProjectLogL(RooRealVar * PoI) {
        RooArgSet * projectedVars = m_combModel->getParameters(RooDataSet("v", "", *PoI));
        return m_combModel->createPlotProjection(*PoI, *projectedVars);
    }

    ///\brief Builds the combined PDF and data set
    bool Initialize(string opt = "");

    RooWorkspace * SaveToRooWorkspace();

    RooRealVar * GetPar(string name) { return getParam(m_combModel, name); }
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

    /** \brief Fits the internal model to the internal dataset
     * @param min,max: Limits the fit range to [min,max] (N.B.: works only if the fitted varible is only one)
     * @param nbins  : Sets the number of bins just for display if the fit is unbinned
     * @param print: Options -> same as Analysis::Fit()
     * \return Returns a map mapping categories names to plots with data and fit model.
     * */
    map<string, RooPlot *> Fit(unsigned nbins = 50, string opt = "-range-log", double min = 0, double max = 0.);

    ///\brief Adds one category you must give an Analysis object containing data and model and a name
    void AddCategory(Analysis * ana, TString nameCat);
    void AddCategory(TString nameCat, RooRealVar * var, string opt = "setana");

    void PlotCategories();

    ///\brief Lists the available categories
    void PrintCategories() { for (unsigned i = 0; i < m_categories.size(); i++) cout << m_categories[i] << endl; }
    /// \brief Prints the sum of all datasets and models properly normalised
    RooPlot * PrintSum(string option = "", TString dovar = "", string name = "", int nbins = 50);

    void RandomizeInitialParams(string option = "");
    void SetConstants(vector<RooDataSet *> input, int index = 0);

    void PrintParams() { printParams(m_combModel); };

    /** \brief Allows to Set an unique signal for all categories
     *  See Analysis::SetSignal()
     *  @param opt: same options as Analysis::SetSignal()
     *  <br> "-i" : The models are set the same but with indipendent parametrs.
     *  By default instead all parameters are kept in common.
     * */
    template <class T> void SetUniqueSignal(T * _sig, string _opt = "", Str2VarMap _myvars = Str2VarMap(), double _nsig = 1.e4)
    {
        //_opt += "-namepar";
        if (_myvars.size() != 0)
        {
            for (unsigned i = 0; i < m_categories.size(); i++) m_ana[i]->SetSignal(_sig, _nsig, _opt, _myvars);
        }
        else if (_opt.find("-i") != string::npos)
        {
            for (unsigned i = 0; i < m_categories.size(); i++) m_ana[i]->SetSignal(_sig, _nsig, _opt, Str2VarMap());
        }
        else
        {
            m_ana[0]->SetSignal(_sig, _nsig, _opt, _myvars);
            for (unsigned i = 1; i < m_categories.size(); i++) m_ana[i]->SetSignal(m_ana[0]->GetSig());
        }
    }

    /** \brief Allows to Set an unique signal for all categories
     *  See Analysis::AddBkgComponent()
     *  @param opt: same options as Analysis::AddBkgComponent()
     *  <br> "-i" : The models are set the same but with indipendent parametrs.
     *  By default instead all parameters are taken in common.
     * */
    template <class T> void SetUniqueBkgComp(const char * _name, T * _comp, string _opt = "", Str2VarMap _myvars = Str2VarMap(), double _frac = 0)
    {
        //_opt += "-namepar";
        if (_myvars.size() != 0)
        {
            for (unsigned i = 0; i < m_categories.size(); i++) m_ana[i]->AddBkgComponent(_name, _comp, _frac, _opt, _myvars);
        }
        else if (_opt.find("-i") != string::npos)
        {
            for (unsigned i = 0; i < m_categories.size(); i++) m_ana[i]->AddBkgComponent(_name, _comp, _frac, _opt, Str2VarMap());
        }
        else
        {
            m_ana[0]->AddBkgComponent(_name, _comp, _frac, _myvars, _opt);
            for (unsigned i = 1; i < m_categories.size(); i++) m_ana[i]->AddBkgComponent(_name, _comp, _frac, _opt, _myvars);
        }
    }
};

#endif
