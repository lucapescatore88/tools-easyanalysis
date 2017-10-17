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

#include "treeReader.hpp"
#include "modelBuilder.hpp"
#include "generalFunctions.hpp"
#include "analysis.hpp"

using namespace RooFit;
using namespace RooStats;


/** \class MultiAnalysis
 *  \brief This class can be used to do simultaneous fits to more than 1 sample (in 1D)
 *  <br>Can also be used fitting different variables for each sample.
 *  <br>Finally nothing forbids to use the class to fit "simultaneously" different variables of the same sample. But just be aware that in this case it works well only if the PDFs of the various 1D fits are uncorrelated (namely PDF in N-d = prod(2D PFDs))
 * */

class MultiAnalysis {

    TString name;
    vector< Analysis * > ana;
    vector< TString > categories;
    RooArgSet * vars;
    RooDataSet * combData;
    RooCategory * samples;
    RooSimultaneous * combModel;
    bool init;
    bool isToy;
    RooFitResult * fitResult;
    RooArgSet * constr;

public:

    MultiAnalysis( TString _name ):
        name(_name), init(false), isToy(false), fitResult(NULL)
    {
        samples = new RooCategory("samples", "samples");
        vars = new RooArgSet("vars");
        constr = new RooArgSet("constraints");
    };

    ~MultiAnalysis()
    {
        delete vars;
        delete combData;
        delete combModel;
        delete samples;
        ana.clear();
    };

    RooAbsPdf * GetCombModel() { return combModel; }
    RooDataSet * GetCombData() { return combData; }
    void SetCombData(RooDataSet * data) { combData = data; };
    void SetCombData(RooSimultaneous * model) { combModel = model; };

    void ImportModel(RooWorkspace * ws);
    void ImportData(RooWorkspace * ws);

    RooDataSet * Generate(int nevts, string option);

    void EnlargeYieldRanges(double factor);

    TString GetName() { return name; }
    void SetName(TString _name) { name = _name; }
    RooFitResult * GetFitResult() { return fitResult; }
    RooAbsReal * CreateLogL()
    {
        if (combModel && combData) return combModel->createNLL(*combData);
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
        c->Print(name + "_LogL_vs_" + (TString)PoI->GetName() + ".pdf");
    }
    const RooAbsReal * ProjectLogL(RooRealVar * PoI) {
        RooArgSet * projectedVars = combModel->getParameters(RooDataSet("v", "", *PoI));
        return combModel->createPlotProjection(*PoI, *projectedVars);
    }

    ///\brief Builds the combined PDF and data set
    bool Initialize(string opt = "");


    RooWorkspace * SaveToRooWorkspace();

    RooRealVar * GetPar(string name) { return getParam(combModel, name); }
    double GetParVal(string name)
    {
        RooRealVar * par = GetPar(name);
        if (par) return par->getVal();
        else { cout << "Parameter " << name << " not found" << endl; return -999999; }
    }
    double GetParErr(string name)
    {
        RooRealVar * par = GetPar(name);
        if (par) return par->getError();
        else { cout << "Parameter " << name << " not found" << endl; return -999999; }
    }

    /** \brief Fits the internal model to the internal dataset
     * @param min,max: Limits the fit range to [min,max] (N.B.: works only if the fitted varible is only one)
     * @param nbins  : Sets the number of bins just for display if the fit is unbinned
     * @param print: Options -> same as Analysis::Fit()
     * \return Returns a map mapping categories names to plots with data and fit model.
     * */
    map<string, RooPlot *> SimultaneousFit(double min = 0, double max = 0., unsigned nbins = 50, string print = "-range-log");
    map<string, RooPlot *> SimultaneousFit(string print, unsigned nbins = 50)
    {
        return SimultaneousFit(0., 0., nbins, print);
    }

    ///\brief Adds one category you must give an Analysis object containing data and model and a name
    void AddCategory(Analysis * _ana, TString nameCat);
    void AddCategory(TString nameCat, RooRealVar * var, string opt = "setana");

    void PlotCategories();

    ///\brief Lists the available categories
    void PrintCategories() { for (unsigned i = 0; i < categories.size(); i++) cout << categories[i] << endl; }
    /// \brief Prints the sum of all datasets and models properly normalised
    RooPlot * PrintSum(string option = "", TString dovar = "", string name = "", int nbins = 50);

    void RandomizeInitialParams(string option = "");
    void SetConstants(vector<RooDataSet *> input, int index = 0);

    void PrintParams() { printParams(combModel); };

    /** \brief Allows to Set an unique signal for all categories
     *  See Analysis::SetSignal()
     *  @param opt: same options as Analysis::SetSignal()
     *  <br> "-i" : The models are set the same but with indipendent parametrs.
     *  By default instead all parameters are kept in common.
     * */
    template <class T> void SetUniqueSignal(T * _sig, string opt = "", Str2VarMap myvars = Str2VarMap(), double _nsig = 1.e4)
    {
        if (myvars.size() != 0)
        {
            for (unsigned i = 0; i < categories.size(); i++) ana[i]->SetSignal(_sig, _nsig, opt + "-namepar", myvars);
        }
        else if (opt.find("-i") != string::npos)
        {
            for (unsigned i = 0; i < categories.size(); i++) ana[i]->SetSignal(_sig, _nsig, opt + "-namepar", Str2VarMap());
        }
        else
        {
            ana[0]->SetSignal(_sig, _nsig, opt + "-namepar", myvars);
            for (unsigned i = 1; i < categories.size(); i++) ana[i]->SetSignal(ana[0]->GetSig());
        }
    }

    /** \brief Allows to Set an unique signal for all categories
     *  See Analysis::AddBkgComponent()
     *  @param opt: same options as Analysis::AddBkgComponent()
     *  <br> "-i" : The models are set the same but with indipendent parametrs.
     *  By default instead all parameters are taken in common.
     * */
    template <class T> void SetUniqueBkgComp(const char * _name, T * _comp, string opt = "", Str2VarMap myvars = Str2VarMap(), double _frac = 0)
    {
        if (myvars.size() != 0)
        {
            for (unsigned i = 0; i < categories.size(); i++) ana[i]->AddBkgComponent(_name, _comp, _frac, opt + "-namepar", myvars);
        }
        else if (opt.find("-i") != string::npos)
        {
            for (unsigned i = 0; i < categories.size(); i++) ana[i]->AddBkgComponent(_name, _comp, _frac, opt + "-namepar", Str2VarMap());
        }
        else
        {
            ana[0]->AddBkgComponent(_name, _comp, _frac, myvars, opt + "-namepar");
            for (unsigned i = 1; i < categories.size(); i++) ana[i]->AddBkgComponent(_name, _comp, _frac, opt + "-namepar", myvars);
        }
    }
};

#endif




