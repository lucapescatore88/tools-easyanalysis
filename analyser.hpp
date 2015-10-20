#ifndef __CINT__
#include "RooGlobalFunc.h"
#endif

#ifndef ANALYSER_HPP
#define ANALYSER_HPP

#include "RooWorkspace.h"
#include "RooRealVar.h"
#include "RooStats/SPlot.h"
#include "RooDataSet.h"
#include "RooRealVar.h"
#include "RooGaussian.h"
#include "RooExponential.h"
#include "RooCBShape.h"
#include "RooChebychev.h"
#include "RooAddPdf.h"
#include "RooDataSet.h"
#include "RooChi2Var.h"
#include "RooDataHist.h"
#include "RooAbsData.h"
#include "RooProdPdf.h"
#include "RooPolynomial.h"
#include "RooAddition.h"
#include "RooProduct.h"
#include "RooAbsPdf.h"
#include "RooFit.h"
#include "RooFitResult.h"
#include "RooWorkspace.h"
#include "RooConstVar.h"
#include "RooNumConvPdf.h"
#include "RooAbsDataStore.h"
#include "RooFFTConvPdf.h"
#include "RooKeysPdf.h"
#include "RooFitResult.h"
#include "RooHistPdf.h"
#include "TRandom3.h"

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

#include <vector>
#include <sstream>
#include <iostream>
#include <string>
#include <time.h>
#include <iomanip>
#include <typeinfo>

#include "ReadTree_comp.hpp"
#include "ModelBuilder.hpp"
#include "general_functions.hpp"

using namespace RooFit;
using namespace RooStats;

Long64_t randomKill(TreeReader * reader, vector< Long64_t > entry);

/** \class Analysis
 *  \brief Allows to handle data starting from TTree or TH1
 *  allows to make cuts and check for multiple candidates.
 *  Then using the ModelBuilder class it allows to create a model
 *  and finally you can Fit it to the dataset with advanced options.
 * */

class Analysis : public ModelBuilder {

	TString title;
	TCut * cuts;
	double chi2[2];
	vector <double> regions;
	vector <string> regStr;
	bool init;
	string unit;
	static string pmode;
	RooAbsReal * weight;
	RooDataSet * data;
	RooFitResult * m_fitRes;
	double fitmin;
	double fitmax;
        RooArgSet * constr;

/** \brief Converts the information in the Analysis object in a RooDataSet which can be fitted
 **/ 
	RooDataSet * CreateDataSet(string opt = "", TCut mycuts = "");
	TH1 * CreateHisto(double min = 0, double max = 0, int nbin = 50, TCut _cuts = "", string _weight = "", string opt = "", TH1 * htemplate = NULL);	
	TH1 * CreateHisto(string opt);

	TreeReader * dataReader;
	TTree * reducedTree;
	TH1 * dataHist;

	double scale;

	public:

	Analysis( TString _name, RooRealVar * _var, RooAbsPdf * pdf = NULL, int ngen = 1000, string opt = "-subtree"):
		ModelBuilder(_name,_var), title(""), cuts(NULL), init(false), unit(""),
		weight(NULL), data(NULL), m_fitRes(NULL), fitmin(0.), fitmax(0.),
		dataReader(NULL), dataHist(NULL), scale(1)
	{
		SetVariable(_var);
		constr = new RooArgSet("constraints_"+name);

		if(pdf)
		{
			SetModel(pdf);
			Generate(ngen,opt);
			init = true;
			ForceValid();
		}
	};	

	Analysis( TString _name, TString _title, TreeReader * reader, TCut * _cuts, RooRealVar * _var = NULL, string _w = ""):
		ModelBuilder(_name,_var), title(_title), cuts(_cuts), init(false), unit(""),
		weight(NULL), data(NULL), m_fitRes(NULL), fitmin(0.), fitmax(0.),
		dataReader(reader), dataHist(NULL), scale(1)
	{
		if(!var) 
        {
            SetVariable(new RooRealVar("x","",0)); 
		    vars.push_back(var);
        }
        constr = new RooArgSet("constraints_"+name);
		if(_w != "") SetWeight( (TString)_w );

		if(!reader->isValid()) reader->Initialize();
		reducedTree = (TTree *)reader->GetChain()->Clone("reduced_"+name);
	};

	Analysis( TString _name, TString _title, string treename, string filename, RooRealVar * _var = NULL, TCut * _cuts = NULL, string _w = ""):
		ModelBuilder(_name,_var), title(_title), cuts(_cuts), init(false), unit(""),
		weight(NULL), data(NULL), m_fitRes(NULL), fitmin(0.), fitmax(0.),
		dataHist(NULL), scale(1)
	{
		if(!var) 
        {
            SetVariable(new RooRealVar("x","",0));
		    vars.push_back(var);
        }
        constr = new RooArgSet("constraints_"+name);
        if(_w != "") SetWeight( (TString)_w );

		TreeReader * reader = new TreeReader(treename.c_str());
		reader->AddFile(filename.c_str());
		reader->Initialize();
		dataReader = reader;
		reducedTree = (TTree *)reader->GetChain()->Clone("reduced_"+name);
	};


	/// \brief Special constructor for single quick fit
	template <typename T = RooAbsPdf *, typename D = RooDataSet *> Analysis( TString _name, TString _title, D * dd, RooRealVar * _var, T * _sig = (RooAbsPdf*)NULL, string _w = "", string opt = ""):
		ModelBuilder(_name,_var), title(_title), cuts(NULL), init(false), unit(""),
		weight(NULL), m_fitRes(NULL), fitmin(0.), fitmax(0.),
		dataReader(NULL), reducedTree(NULL), dataHist(NULL), scale(1)
	{
		if(!var) 
        {
            SetVariable(new RooRealVar("x","",0));
		    vars.push_back(var);
        }
        constr = new RooArgSet("constraints_"+name);
		if(_w != "") SetWeight( (TString)_w );		

		string tdata = typeid(D).name();
		if(tdata.find("RooDataSet")!=string::npos) data = (RooDataSet *)dd;
		else if(tdata.find("TTree")!=string::npos) 
        { 
            dataReader = new TreeReader((TTree *)dd);
            dataReader->Initialize();
            reducedTree = (TTree *)dd; 
            CreateDataSet(); 
        }
		else if(tdata.find("TH1")!=string::npos) { dataHist = (TH1 *)dd; CreateDataSet(); }
		else cout << "Type '" << tdata << "' not supported!" << endl;

		if(_sig) { SetSignal(_sig,1.e4,opt); Initialize(""); }
	};

	~Analysis()
	{
		delete dataReader;
		delete cuts;
		delete reducedTree;
		delete dataHist;
	};


        RooFitResult * GetFitRes() {
	        return m_fitRes;
	}

        void SetFitRes(RooFitResult *fitRes = NULL) {
	        m_fitRes = fitRes;
	}

	static void SetPrintLevel(string mode) { pmode = mode; ModelBuilder::SetPrintLevel(mode); TreeReader::SetPrintLevel(mode);  }
	static string GetPrintLevel() { return pmode; }

	TTree * GetReducedTree() { return reducedTree; }
	TreeReader * GetTreeReader() { return dataReader; }
	TH1 * GetHisto() { return dataHist; }

	/** \brief Sets units finding the scale factor too
	 *  @param inUnit: unit of the input data. Based on the inUit-outUnit difference the scale factor is found. <br>Works only with masses in eV-PeV.
	 *  For any other unit use SetUnits(string outUnit, double scalefactor)
	 *	@param outUnit: is a label for the unit"units" is a string with the unit label
	**/
	void SetUnits(string inUnit, string outUnit);

	/** \brief Set units
	 *  @param outUnit: is a label for the unit"units" is a string with the unit label
		@scalefactor: factor to rescale input data if they are given in a unit different than the one you want
	 **/
	void SetUnits(string outUnit, double scalefactor = 1);
	double GetScale() { return scale; }
	string GetUnits() { return unit; }

	/** \brief Adds a variable to the internal DataSet
	 * An Analysis object can also be used to create RooDataSet objects to use then somewhere else. Therefore it comes usefull to have in the RooDataSet more than one variable. This can me added simpy by name (if it exists in the input tree).
	 * */
	void AddVariable(RooRealVar * v) 
    { 
        if( ((string)v->GetTitle()).find("__var__")==string::npos )
                v->SetTitle((TString)v->GetTitle()+"__var__"); 
        vars.push_back(v); }
	void AddVariable(TString vname) { RooRealVar * v = new RooRealVar(vname,vname+"__var__",0.); vars.push_back(v); }
	bool isValid() { return init; }

	/** Function to unitialize the Analysis object before fitting
	 *	<br> Runs ModelBuilder::Initialize() to initialize the model
	 *	<br> Runs CreatedataSet() to initialize the data
	 **/
	bool Initialize(string _usesig = "-exp", double frac = -1.);
	void CreateReducedTree(string option = "", double frac = -1., TCut cuts = "");
	RooAbsReal * CreateLogL()
	{ 
		if(model && data) return model->createNLL(*data);
		else return NULL;
	}
	void SetWeight( TString w ) { weight = new RooRealVar(w,w,1.); return; }
    string GetWeight() { if (!weight) return ""; else return weight->GetName(); }
	void SetDataSet( RooDataSet * d ) { data = d; }
	void SetCuts( TCut _cuts ) { cuts = &_cuts; }
	void SetCuts( TCut * _cuts ) { cuts = _cuts; }
	void SetCuts( TString _cuts ) { TCut tmpcut = (TCut)_cuts; cuts = &tmpcut; }
	
    RooWorkspace * SaveToRooWorkspace();
    void ImportModel(RooWorkspace * ws);
    void ImportData(RooWorkspace * ws);

   
    RooDataSet * GetDataSet( string opt = "" )
	{
		if(opt.find("-recalc")!=string::npos || !data) return CreateDataSet(opt);
		else return data;
	}
	TH1 * GetHisto(double min = 0, double max = 0, int nbin = 50, TCut _cuts = "", string _weight = "", TH1 * htemplate = NULL)
	{ return CreateHisto(min,max,nbin,_cuts,_weight); }

    void AddConstraint(RooAbsReal * pdfconst) { constr->add(*pdfconst); }
    void AddGaussConstraint(TString name, double mean, double sigma);
    void AddGaussConstraint(RooRealVar * par, double mean = -1e9, double sigma = -1e9);
    RooArgSet * GetConstraints() { return constr; }

/** \brief Adds a blinded region.
 If you have to add more than one region you must add them in order from low to high and the must not overlap. If you set one or more regions parameters will be hidden and model and data will not be plotted in those regions.
 **/
	void SetBlindRegion(double min, double max);
	void Reset() { regions.clear(); ClearBkgList(); ResetVariable(); vars.clear(); vars.push_back(var); chi2[0] = chi2[1] = -1; sig = NULL, bkg = NULL; init = false; }

	/** \brief Generates events using the model internally set and a specific number of total events
	 * @param nevt: Number of events to generate (N.B.: this is Nsig+Nbkg and the fraction between is supposed to be right in the model)
	 * @param opt: Options:
	 * <br> "-seed(n)":  To set a specific seed
	 * <br> "-smear(r)": "r" is a double interpreted as resolution. The generated events will be gaussian smeared by this resolution.
	 **/
	TTree * Generate(int nevt, string opt = "-subtree");

/** \brief Generate events using the model inetrnally set and a specific number of signal and background events
	 * @param nsigevt: Number of signal events to generate
	 * @param nbkgevt: Number of background events to generate      
 	 * @param opt: Options:
	 * <br> "-seed(n)":  To set a specific seed
	 * <br> "-smear(r)": "r" is a double interpreted as resolution. The generated events will be gaussian smeared by this resolution.
	 **/
	TTree * Generate(int nsigevt, int nbkgevt, string opt = "-subtree");

/** \brief Allows to apply cuts on "dataReader" and returnes a tree containing only events which pass the cut.
  @param substtree: if you want to set the three obtained as "reducedTree" (default = true)
  @param addFunc: you may define a function getting a TreeReader and a TTree which is called in the loop and adds variables to the new tree combining information from the old tree
  <br> N.B.: addFunc is called once before the loop and here you should set static addresses.
 @param frac: uses only the fraction "frac" of the available entries  
 **/
	TTree * applyCuts(TCut _cuts = NULL, bool substtree = true, void (*addFunc)(TreeReader *, TTree *, bool) = NULL, double frac = 1);
	TTree * applyFunc(void (*addFunc)(TreeReader *, TTree *, bool), double frac = 1);


	typedef  Long64_t (*FUNC_PTR)(TreeReader *, vector< Long64_t >);
	
	/** \brief Checks for multiple candidate in the same event
	 * */
	/** Also creates a plot of number of candidates and a new tree with a variable "isSingle" added. This will be 1 for the best candidate and 0 for the others.
  N.B.: if you just need to apply cuts ad not to check for multiples USE "applyCuts" it's much more efficient.

  randomKill() is a standard function for random killing of multiple candidates.
  However you can make your own function and pass it to the method.
  Function template should be:

  Long64_t choose(TreeReader * reader, vector< Long64_t > entry )
  {
  	//Typycally loop on entry and do something with reader
	return chosen_entry;
  }	
**/
	TTree * GetSingleTree(FUNC_PTR choose = NULL);
	

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
	RooPlot* Fit(double min = 0, double max = 0., unsigned nbins = 50, bool unbinned = false, string print = "-range-log", TCut mycuts = "");


/** \brief Makes nice plots of data and models including blinded plots
  @param model: if true plots model on data
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
	RooPlot * Print(bool domodel,  RooAbsData * data,  string opt = "-range-keepname", unsigned bins = 50, double * range = NULL, TString Xtitle = "", TString title = "", RooRealVar * myvar = NULL);
	RooPlot * Print(string opt = "-range-keepname", unsigned bins = 50, double * range = NULL, TString Xtitle = "", TString title = "", RooRealVar * myvar = NULL);
	RooPlot * PrintAndCalcChi2(int nbins, double * range, string print, RooAbsData * mydata = NULL );
	RooPlot * PrintVar(RooRealVar * myvar, int nbins = 50, string option = ""); 
    void PrintComposition(float min, float max)
	{
		if(m_fitRes) ModelBuilder::PrintComposition(min,max,m_fitRes);
		else cout << "Fit didn't happen yet, composition unkown" << endl;
	}

	double GetNBkgVal(double min = 0, double max = 0, double * valerr = NULL)
	{
		if(!init || !m_fitRes) return -1;
		return ModelBuilder::GetNBkgVal(min,max,valerr,m_fitRes);
	}
	double GetNBkgErr()
	{
		double valerr = 0;
		GetNBkgVal(0, 0, &valerr);
		return valerr;
	}
	double GetNSigVal(double min = 0, double max = 0, double * valerr = NULL)
	{
		if(!init || !m_fitRes) return -1;
		return ModelBuilder::GetNSigVal(min,max,valerr,m_fitRes,fitmin,fitmax);
	}
	double GetNSigErr()
	{
		double valerr = 0;
		GetNSigVal(0, 0, &valerr);
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

	///\brief Returns chi2/NDF
	double GetChi2();
	///\brief Returns NDF
	double GetNDF();
	void PrintChi2();
	//Returns chi2 probability
	double GetProb();

	/** \brief Calculates S-weight for the data and model set
  		The function will perform a fit but it could help to Fit first anyway.
	 	@param min,max: Calculates S-weight only in [min,max]
		@param nbins:   Number of bins (just for display if unbinned fit)
		@param unbinned: If true performs unbinned fit
		@pram options: Options:
		"-nofit" doesn't perform the fit
	 **/
	RooDataSet * CalcSWeight(double min = 0, double max = 0., unsigned nbins = 50, bool unbinned = false, string option = "");
	RooDataSet * CalcReducedSWeight(double min = 0, double max = 0., unsigned nbins = 50, bool unbinned = false, string option = "");
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
		if(reset) tree->Branch((TString)_var->GetName()+"_unscaled",&unscaled,(TString)_var->GetName()+"_unscaled/F");
		else
		{
			float value = reader->GetValue(_var->GetName());
			unscaled = value;
			reader->SetValue(_var->GetName(),value*(float)_scale);
		}
	}
};

#endif
