/***
* class ModelBuilder
*
* author: Luca Pescatore
* email : luca.pescatore@cern.ch
* 
* date  : 01/07/2015
***/


#ifndef MODEL_BUILDER_HPP
#define MODEL_BUILDER_HPP

#ifndef __CINT__
#include "RooGlobalFunc.h"
#endif
#include "RooRealVar.h"
#include "RooStats/SPlot.h"
#include "RooDataSet.h"
#include "RooRealVar.h"
#include "RooGaussian.h"
#include "RooExponential.h"
#include "RooCBShape.h"
#include "RooChebychev.h"
#include "RooVoigtian.h"
#include "RooBreitWigner.h"
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
#include "RooGExpModel.h"
#include "RooArgusBG.h"
#include "RooGamma.h"
#include "RooConstVar.h"
#include "RooNumConvPdf.h"
#include "RooFFTConvPdf.h"
#include "RooKeysPdf.h"
#include "RooNDKeysPdf.h"
#include "RooHistPdf.h"
#include "RooPlot.h"

#include "TLegend.h"
#include "TColor.h"
#include "TCanvas.h"
#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1.h"
#include "TF1.h"
#include "TEntryList.h"
#include "TCanvas.h"
#include "TBranch.h"
#include "TCut.h"
#include "TMath.h"
#include "TIterator.h"
#include "TAxis.h"
#include "TGaxis.h"

#include <vector>
#include <sstream>
#include <iostream>
#include <map>
#include <string>
#include <time.h>
#include <iomanip>

#include "ReadTree_comp.hpp"
#include "general_functions.hpp"

using namespace std;


///\brief This type is supposed to contain a list of pointers to RooRealVars (parameters) collable by name 

typedef std::map <string, RooAbsReal *> Str2VarMap;
typedef std::map <string, RooAbsReal *>::iterator Str2VarMapItr;

/// \brief Given a parameter name return a RooRealVar pointer
RooRealVar * addPar(string par, string parstr, Str2VarMap stval_list, Str2VarMap myvars, string pmode = "v");

/// \brief Given a PDF type it returns the list of needed parameters to built a RooAbsPdf
Str2VarMap getPar(string typepdf_, TString namepdf_, RooRealVar * val, Str2VarMap myvars = Str2VarMap(), string pmode = "v");

/** \brief Given a PDF name returns a RooAbsPdf
 * Available PDFs: Gauss, DGauss, CB, DCB, Argus, Exp, Poly, Cheb
 * @param typepdf: The PDF type (CB,Gauss,etc)
 * @param namepdf: The name of the PDF e.g. Ugly_Bkg. The PDF will be named (type)_(name)
 * @param myvars:  While creating parameters for the RooAbsPdf it will go through this list and if parameters with the required names are found
 * it will clone their properties and use the for RooAbsPdf
 * @param pmode : Print level flag (verbose "v", silent "s")
 * */
RooAbsPdf * stringToPdf(const char * typepdf, const char * namepdf, RooRealVar * var, Str2VarMap myvars = Str2VarMap(), string pmode = "v");

/// \brief Returns the number of free patameters in a RooAbsPdf
int getNFreePars(RooAbsPdf * pdf, RooArgSet vars);

/// \brief Prints all paraeters of a RooAbsPdf 
void printParams(RooAbsPdf * pdf, RooArgSet obs = RooArgSet(), string opt = "");

/** \brief Sets all parameters of a RooAbsPdf to the values of the parameters with the same name in the "set" list.
* Does nothing if the name is not found.
* <br> fix == "fix"  -> sets also the parameters to constants
* <br> fix == "free" -> sets also the parameters to free
* <br> fix == ""     -> keeps the variable as it is in "set" 
*/
void fixParam(RooAbsPdf * pdf, RooRealVar * obs, RooArgSet * set, string fix = "");
void fixParam(RooAbsPdf * pdf, RooRealVar * obs, string fix = "fix");
void fixParam(RooAbsPdf * pdf, RooArgSet * obs, RooArgSet * set, string fix = "");

/// \brief Returns a RooArgSet containing a copy of all the free parameters in a RooAbsPdf
RooArgSet * copyFreePars(RooAbsPdf * pdf, RooArgSet vars);
RooArgSet * gaussianConstraints(RooAbsPdf * pdf, RooArgSet vars);

/** \brief Returns the pointer to the parameter named "named" of PDF.
 * Returns null if it doesn't find any.
 * */

RooRealVar * GetParam(RooAbsPdf * pdf, string name);

/** \brief Gets the list of parameters of a RooAbsPdf in Str2VarMap form
* If any name is specified in pnames these names are used to make a selection of parameters. If "pnames" is empty all parameters are returned.
* opt=="-origNames" keeps the names as they are otherwise keeps only the part before the underscore "(alwayskept)_(optional)"
*/

Str2VarMap GetParams(RooAbsPdf * pdf, RooArgSet obs, vector < string > pnames, string opt = "");
Str2VarMap GetParamList(RooAbsPdf * pdf, RooAbsReal * var, string opt = "");
Str2VarMap GetParamList(RooAbsPdf * pdf, RooArgSet obs = RooArgSet(), string opt = "");

/** \brief Allows to make nice plots of data and models including blinded plots
  @param var: variable to plot
  @param data: data to plot -> If no data use NULL
  @param model: model to plot -> If no model use NULL
  @param opt: options string. Options available are (N.B.: case insensitive):
  <br> "-fillSig"    -> signal is filled with color instead of dashed line
  <br> "-fillBkg"    -> bkg is filled with color instead of dashed line
  <br> "-noParams"   -> no params box produced
  <br> "-noCost"     -> no constant parameters shown in params box
  <br> "-nochi2"     -> no chi2 in params box
  <br> "-chi2ndf"    -> plots the result of chi2/ndf instead of the separate chi2 and ndf
  <br> "-sumW2err"   -> if weighted data errors shown reflect statistics of initial sample
  <br> "-plotSigComp"-> prints signal components and not only total signal function
  <br> "-range"      -> plots only the fitted range, otherwise all available is plot
  <br> "-min(num)"     -> sets the minimum to "num"
  @param bins: number of bins to use for data
  @param regions: number of regions to plot is blinded plot, if regions = 0 plots everything 
  @param Xtitle: X axis label
  @param title: title
  @param leg: A TLegend object to fill
 **/


RooPlot * GetFrame(RooRealVar * var, RooAbsData * data, RooAbsPdf * model = NULL, string opt = "", unsigned bins = 0, double * range = NULL, vector<string> regStr = vector<string>(1,"PlotRange"), TString Xtitle = "", TString Ytitle = "", TLegend * leg = NULL);
RooPlot * GetFrame(RooRealVar * var, RooAbsPdf * model, RooAbsData * data = NULL, string opt = "", unsigned bins = 0, double * range = NULL, vector<string> regStr = vector<string>(1,"PlotRange"), TString Xtitle = "", TString Ytitle = "", TLegend * leg = NULL);
RooPlot * GetFrame(RooRealVar * var, RooAbsData * data, RooAbsPdf * model, string opt, unsigned bins, TString Xtitle, TString Ytitle = "", TLegend * leg = NULL);
RooPlot * GetFrame(RooRealVar * var, RooAbsPdf * model, RooAbsData * data, string opt, unsigned bins, TString Xtitle, TString Ytitle = "", TLegend * leg = NULL);

/** Set the paramteres with names in the "names" list to constants
* opt == "-except" -> set contrants all but the ones with name in the list
*/
Str2VarMap setConstant(Str2VarMap * pars, vector<string> names = vector<string>(), string opt = "");
Str2VarMap setConstant(Str2VarMap * pars, string name, string opt = "");
Str2VarMap setConstant(RooAbsPdf * pdf, RooRealVar * var, vector<string> names = vector<string>(), string opt = "");
Str2VarMap EraseParameter(Str2VarMap * pars, vector<string> names);
Str2VarMap ModifyPars(Str2VarMap * pars, vector<string> names, RooRealVar * c, string opt = "-scale");
Str2VarMap ModifyPars(Str2VarMap * pars, string name, RooRealVar * c, string opt = "-scale");

/** \brief Allows to modify a parameter (or more) in a RooFormulaVar
* <br> default       -> Scales the parameter by "c"
* <br> opt=="-shift" -> Adds a shift by "c"
*/
Str2VarMap ModifyPars(Str2VarMap * pars, vector<string> names, vector<RooRealVar *> c, string opt = "-scale");
void PrintPars(Str2VarMap pars, string opt = "");
TPaveText * createParamBox(RooAbsPdf * pdf, RooArgSet * obs, string opt, RooFitResult * fitRes = NULL);

/** Prints the parameters in a Str2VarMap object
* <br> opt == "-nocost"  ->  doesn't print constants
* <br> opt == "-latex"   ->  prints the Title instead of Name of variables assuming latex format
*/
void PrintPars(Str2VarMap pars, string opt);



/** \class ModelBuilder
 * \brief Implements a class which uses RooFit to handel the creation of fit models
 * <br> - some functions like CB, gaussian, Double CB ecc are already available and callable just by name
 * <br> - the model distringuish between signal and background
 * <br> - bkg components can be added ad libitum
 * <br> - trees and histograms can be used to extract signal shapes
 */

class ModelBuilder {

	bool isvalid;

	protected:

	static string pmode;
	TString name;
	RooRealVar * var;
	RooAbsPdf * model;
	RooAbsPdf * sig;
	RooAbsPdf * bkg;
	RooAbsReal * nsig;
	RooAbsReal * nbkg;
	vector <RooAbsPdf *> bkg_components;
	vector <RooAbsReal *> bkg_fractions;
	RooRealVar * tmpvar;
	bool extended;
	bool totBkgMode;
	TString title;

	void ForceValid() { isvalid = true; }

	/// \brief Wrapper for the external function stringToPdf() which returns a model from a string

	RooAbsPdf * StringToPdf(const char * typepdf, const char * namepdf, Str2VarMap myvars)
	{
		return stringToPdf(typepdf, namepdf, var, myvars, pmode);
	}

	/** \brief Returns a PDF starting from different kinds of objects
	 * This is one of the core function of the ModelBuilder.
	 * The template type "T" can be:
	 * <br> - RooAbsPdf -> in this case it simply returns itself
	 * <br> - string    -> it used stringToPdf() to get a PDF from the string
	 * <br> - TTree     -> it uses the RooKeysPdf class to extract a smooth pdf from the tree events distribution
	 * <br> - TH1F      -> it used the RoohistPdf class to extract a PDF
	 *
	 * @param _base:  Object to start with to create the PDF
	 * @param _name:  Name to give to the newly created PDF
	 * @param myvars: This works only if the given type is a string. While creating the model it looks if its parameters are already in "myvars". And if it finds them it uses them to create the new PDF. Does nothing if myvars contains no parameters or wrong parameters or if the given type is not "string".
	 * @param weight: In case the starting object is a TTree distributions are created considering the variable "weight" as an event-by-event weight.
	 * @param opt:    Options:
	 *                <br> "-namepar" : put the ModelBuiler object's name et the end of each parameter name.
	 *                This is important because every parameter name must be unique in RooFit.
	 *                <br> "-c[]-v[]" (TTree only) : Using TTrees you can apply cuts on them before extracting shapes.
	 *                The cuts go inside the "c" braces and in the v braces you must list comma separated all variables on which you cut.
	 *                "-c" and "-v" always go together!
	 *                e.g.: "-c[B0_PT > 50 && Kst_P > 200]-v[B0_PT,Kst_P]"
	 *                <br> "-s(n)" (TTree only): n = 1 or n = 2 (default) defines how fine is the resolution of the smoothing
	 *	*/
	template <class T> RooAbsPdf * getPdf(T * _base, const char * _name, Str2VarMap myvars = Str2VarMap(), string weight = "", string opt = "")
	{
		string t = typeid(T).name();
		RooAbsPdf * res = NULL;

		if((t.find("c")!=string::npos && t.length()==1) || t.find("TString")!=string::npos)
		{
			res = (RooAbsPdf*)StringToPdf((const char *)_base, _name, myvars);
		}
		else if(t.find("RooAbsPdf")!=string::npos)
		{
			res = (RooAbsPdf*)_base;

			if(t.find("vector")!=string::npos)
			{
				RooArgList * pdfList = new RooArgList("sigPdfList_"+name);
				RooArgList * fracList = new RooArgList("fracSigPdfList_"+name);

				vector<RooAbsPdf *> * myv = (vector<RooAbsPdf *> *)_base;
				for(unsigned e = 0; e < myv->size(); e++)
				{
					RooRealVar * f  = new RooRealVar(("f_"+(string)((*myv)[e]->GetName())).c_str(),"f",0.1,0.,1.);
					pdfList->add(*(*myv)[e]);
					if(e>0)fracList->add(*f);
				}

				res = new RooAddPdf("totsigpdf_"+name,"totsigpdf",*pdfList,*fracList);
			}

			res->SetName(_name);
		}
		else if(t.find("TTree")!=string::npos)
		{	
			RooArgSet * vars = new RooArgSet(*var);
			if(opt.find("-v[")!=string::npos)
			{
				size_t pos = opt.find("-v[") + 2; 
				size_t posend = opt.find("]",pos);
				
				while(true)
				{
					size_t pos2 = opt.find(",",pos+1);
					TString vname = (TString)opt.substr(pos+1,pos2-pos-1);
					if(pos2 >= posend) vname.ReplaceAll("]","");
					vars->add(*(new RooRealVar(vname,vname,0.)));
					if(pos2 < posend) pos = pos2;
					else break;
				}
			}

			RooDataSet * sigDataSet = NULL;
			if(weight!="")
			{
				vars->add(*(new RooRealVar(weight.c_str(),weight.c_str(),0.)));
				sigDataSet = new RooDataSet((TString)_name+"_DataSet_"+name,"",(TTree*)_base,*vars,0,weight.c_str());
			}
			else sigDataSet = new RooDataSet((TString)_name+"_DataSet_"+name,"",(TTree*)_base,*vars);
			
			if(opt.find("-c[")!=string::npos)
			{
				size_t pos = opt.find("-c[") +3; 
				size_t posend = opt.find("]",pos);
				TString treecut = (TString)(opt.substr( pos, posend - pos));
				sigDataSet->Print();
				sigDataSet->reduce(treecut);
			}
			
			if(opt.find("-rho")!=string::npos) 
			{
				int pos = opt.find("-rho");
				string rhostr = opt.substr(pos+4,20);
				double rho = ((TString)rhostr).Atof();
				res = new RooKeysPdf((TString)_name,_name,*var,*sigDataSet,RooKeysPdf::MirrorBoth,rho);
			}
			else res = new RooKeysPdf((TString)_name,_name,*var,*sigDataSet,RooKeysPdf::MirrorBoth,2);
		}
		else if(t.find("TH1")!=string::npos)
		{
			RooDataHist * sigDataHist = new RooDataHist((TString)_name+"_DataHist"+name,"",*var,(TH1*)_base,6);
			res = new RooHistPdf((TString)_name,_name,*var,*sigDataHist);
		}
		else
		{
			if(((string)_name).find("_noprint")!=string::npos) cout << name << ": ATTANTION:_noprint option in background name: component won't be added to background list" << endl;
			else cout << name << ": ***** ATTANTION: Wrong type (" << t << ") given to getPdf. Only string, RooAbsPdf, vector<RooAbsPdf *>, TTree and TH1 are allowed! *****" << endl;
		}

		return res;
	}


	public:

	//Constructors

	ModelBuilder():
		isvalid(false), var(NULL), sig(NULL), bkg(NULL), extended(true), totBkgMode(false)
	{
		nsig = new RooRealVar("nsig","nsig",0.,0.,1.e10);
		nbkg = new RooRealVar("nbkg","nbkg",0.,0.,1.e10);
		name = "model";
		title = name;
	}

	ModelBuilder(TString _name, RooRealVar * _var):
		isvalid(false), name(_name), var(_var), sig(NULL), bkg(NULL), extended(true), totBkgMode(false)
	{
		if(var) tmpvar = new RooRealVar(*var); 
		nsig = new RooRealVar("nsig"+name,"nsig",0.,0.,1.e10);
		nbkg = new RooRealVar("nbkg"+name,"nbkg",0.,0.,1.e10);
		title = name;
	}

	~ModelBuilder()
	{
		delete var;
		delete model;
		delete sig;
		delete bkg;
		delete nsig;
		delete nbkg;

		for(unsigned i = 0; i < bkg_components.size(); i++)
		{
			delete bkg_components[i];
			delete bkg_fractions[i];
		}
	}



	//Methods
	
	/** \brief Builds the model:
	 * Using RooAddPdf builds a model using all components in sig and background_compnents.
	 * Options: 
	 * <br> "-exp":     Automatically adds an exponential coponent to the background.
	 * <br> "-namepar": Appends the ModelBuilder's name to to model's name
	 * <br> "-noBkg" -> ignores bkg
	 * */
	RooAbsPdf * Initialize(string optstr = "-exp-namepar");

	/** \brief Adds a bkg component.
	 * Adds a bkg component named "_name" based on the object "_comp".
	 * @param _frac: This can ve a RooRealVar/RooFormulaVar or a double. The Roo object are set as yields of the current component.
	 * <br> In case it's a double a Roo object is created in the following way:
	 * 	<br> - _frac > 1    creates a RooRealVar with "_frac" value (in the fit the yield will start from "_frac" and float)
	 * 	<br> - |_frac| <= 1 created a RooFormulaVar connecting this background yield as a fraction "_frac" of the singla yield
	 * 	<br> - _frac < -1   same as _frac > 1 but the number is fixed. Meaning that the yield of this bacgkround will not float in the fit
	 *
	 * @param opt: Options:
	 * -> It is passed as options to getPdf() 
	 * <br> - "-namepar": See getPdf() 
	 * <br> - "-ibegin":  Adds the bkg component in front and not at the back of the backgrounds list
	 *
	 * @param myvars:
	 * -> It is passed as argument of getPdf()
	 *  */
	template <class T> RooAbsPdf * addBkgComponent(const char * _name, T * _comp, RooAbsReal * _frac, string opt = "-namepar", Str2VarMap myvars = Str2VarMap(), string weight = "")
	{
		TString nstr = "bkg_"+(TString)_name;
		string lowopt = opt;
		transform(lowopt.begin(), lowopt.end(), lowopt.begin(), ::tolower);
		RooAbsReal * frac = NULL;
		
		//if(lowopt.find("-namepar")!=string::npos)
		nstr+=("_"+name);

		if(opt.find("-frac")!=string::npos) {

			TString ss( (TString)nsig->GetName() + " * " + _frac->GetName() );
			frac = new RooFormulaVar("n"+nstr,"f_{"+(TString)_name+"}^{wrtsig}",ss,RooArgSet(*_frac,*nsig));
		}
		else { frac = _frac; }
		
		//if(lowopt.find("-namepar")!=string::npos)
		frac->SetName((TString)frac->GetName()+"_"+name);

		RooAbsPdf * comp = getPdf(_comp,nstr,myvars,weight,opt);
		

		if(comp!=NULL && _frac != NULL && lowopt.find("-nofit")==string::npos)
		{
			if(lowopt.find("-ibegin")==string::npos)
			{
				bkg_components.push_back(comp);
				bkg_fractions.push_back(frac);
			}
			else
			{
				bkg_components.insert(bkg_components.begin(),comp);
				bkg_fractions.insert(bkg_fractions.begin(),frac);
			}
		}
		//else cout << "***** ATTENTION: Something was set wrong! Bkg " << _name << " not added. *****" << endl;

		return comp;
	}

	template <class T> RooAbsPdf * addBkgComponent(const char * _name, T * _comp, double _frac = 0, string opt = "-namepar", Str2VarMap myvars = Str2VarMap(), string weight = "")
	{
		TString nstr = "bkg_"+(TString)_name;
		RooAbsReal * frac = NULL;
		double val = TMath::Abs(_frac);
		if(val == 0) val = 1e3;
		double min = 0;
		double max = 1.e7;
		
		if(_frac < 0) { min = val; max = val; }

		if(totBkgMode) 
		{
			if(val > 1) { cout << "Attention in 'totBkgMode' the nevt must be between 0 and 1" << endl; return NULL; }
			frac = new RooRealVar("f"+nstr,"f_{"+(TString)_name+"}",val,0,1);
		}
		else if((TMath::Abs(_frac) > 0 && TMath::Abs(_frac) <= 1) || opt.find("-frac")!=string::npos)
		{
			TString ss( (TString)nsig->GetName() + Form(" * %e", val) );
			if(!nsig) { cout << "Attention if you use this option abs(nevt) < 1 you must set the signal first." << endl; return NULL; }
			frac = new RooFormulaVar("n"+nstr,"f_{"+(TString)_name+"}^{wrtsig}",ss,*nsig);
		}
		else frac = new RooRealVar("n"+nstr,"N_{"+(TString)_name+"}",val,min,max);

		return addBkgComponent(_name, _comp, frac, opt, myvars, weight);
	}

	template <class T> RooAbsPdf * addBkgComponent(const char * _name, T * _comp, RooAbsReal * _frac, string opt, string weight, Str2VarMap myvars = Str2VarMap())
	{
		return addBkgComponent(_name, _comp, _frac, opt, myvars, weight);
	}

	template <class T> RooAbsPdf * addBkgComponent(const char * _name, T * _comp, double _frac, string opt, string weight, Str2VarMap myvars = Str2VarMap())
	{
		return addBkgComponent(_name, _comp, _frac, opt, myvars, weight);
	}



	/** \brief Adds a blacgound component
	 * The behaviour it's similar to addBkgComponent().
	 * The only difference is the nsig, is set as signal yield. If given as a double it can be only > 1 (starting value)
	 * or < -1 (starting value fixed in the fit).
	 * */

	template <class T> RooAbsPdf * SetSignal(T * _sig, RooAbsReal * _nsig, string opt = "-namepar", Str2VarMap myvars = Str2VarMap(), string weight = "")
	{
		string lowopt = opt;
		transform(lowopt.begin(), lowopt.end(), lowopt.begin(), ::tolower);
		
		ResetVariable();
		TString myname = "_"+name;
		nsig = _nsig;
		//if(opt.find("-namepar")!=string::npos)
		nsig->SetName((TString)nsig->GetName()+myname);
		if(_sig) 
		{
			sig = getPdf(_sig,"sig"+myname, myvars, weight, opt);
			sig->SetName("totsig"+myname);
		}
		return sig;
	}

	template <class T> RooAbsPdf * SetSignal(T * _sig, double _nsig = 0, string opt = "-namepar", Str2VarMap myvars = Str2VarMap(), string weight = "")
	{
		RooAbsReal * tmpnsig = NULL;
		double val = TMath::Abs(_nsig);
		if(val == 0) val = 1e3;
		double min = 0;
		double max = 1.e7;
		if(_nsig < 0) { min = val; max = val; }

		if(TMath::Abs(_nsig) > 0 && TMath::Abs(_nsig) <= 1) tmpnsig = new RooRealVar("nsig","N_{sig}",val*max,min,max);
		else tmpnsig = new RooRealVar("nsig","N_{sig}",val,min,max);

		return SetSignal(_sig,tmpnsig,opt,myvars, weight);
	}

	template <class T> RooAbsPdf * SetSignal(T * _sig, RooAbsReal * _nsig, string opt, string weight, Str2VarMap myvars = Str2VarMap())
	{
		return SetSignal(_sig,_nsig,opt,myvars, weight);
	}

	template <class T> RooAbsPdf * SetSignal(T * _sig, double _nsig, string opt, string weight, Str2VarMap myvars = Str2VarMap())
	{
		return SetSignal(_sig,_nsig,opt,myvars, weight);
	}

	void SetTitle(TString _title) { title = _title; }
	
	/** \brief Sets the variable to fit "var".
	 * It also creates a copy of the initial variable "tmpvar". In fact when fitting the varible is modified and
	 * if you want to fit again or change the model and refit you need to reset the variable as it was.
	 * */
	void SetVariable(RooRealVar * _var) { var = _var; tmpvar = new RooRealVar(*var); }
	
	/** \brief Sets the background mode
	 * Normally the model is built as model = nsig*sig + nbkg1*bkg1 + nbkg2*bkg2 * ...
	 * If the totBkgModel is set to "tot" the model is built as model = nsig*sig + nTotbkg*(fracBkg1*bkg1 + fracBkg2*bkg2 + ...)
	 * */
	void SetBkgMode( bool mode ) { totBkgMode = mode; }
	void SetSig(RooAbsPdf * _sig) { sig = _sig; }
	///\brief Forces a model
	void SetModel(RooAbsPdf * _model);
	void SetNSig(RooAbsReal * _nsig) { nsig = _nsig; }
	void SetBkg(RooAbsPdf * _bkg) { bkg = _bkg; }
	void SetBkg(vector<RooAbsPdf *> _bkg_comp) { bkg_components = _bkg_comp; }
	void SetName(const char * newname) { name = newname; }
	void ClearBkgList() { bkg_components.clear(); bkg_fractions.clear(); }
    void ResetVariable() { var->setVal(tmpvar->getVal()); var->setRange(tmpvar->getMin(),tmpvar->getMax()); };

	////\brief Return true if the model was correctly built and initialized
	bool isValid() { return isvalid; }
	bool isExtended() { return extended; }
	TString GetName() { return name; }
	///\brief Return the variable to fit
	RooRealVar * GetVariable() { return var; }
	///\brief Returns the model pdf
	RooAbsPdf * GetModel() { return model; }
	///\brief Returns the signal pdf
	RooAbsPdf * GetSig() { return sig; }
	///\brief Returns a pdf corresponding to the sum of all bkg PDFs
	RooAbsPdf * GetTotBkg() { return bkg; }
	///\brief Returns the number of bkg events integrating the bkg pdf in [min,max] (Returns nbkg * (int [min,max] of bkg) )
	/// If "fitRes" is passed stores the error in "valerr"
	double GetNBkgVal(double min = 0, double max = 0, double * valerr = NULL, RooFitResult * fitRes = NULL);
	///\brief Returns the number of sig events integrating the sig pdf in [min,max] (Returns nsig * (int [min,max] of sig) )
	/// If "fitRes" is passed stores the error in "valerr"
	double GetNSigVal(double min = 0, double max = 0, double * valerr = NULL, RooFitResult * fitRes = NULL, double fmin = 0, double fmax = 0);
	///\brief Return S/B integrating sig and bkg in [min,max]
	double GetSOverB(float min, float max, double * valerr = NULL, RooFitResult * fitRes = NULL);
	///\brief Return S/(S+B) integrating sig and bkg in [min,max]
	double GetSigFraction(float min, float max, double * valerr = NULL, RooFitResult * fitRes = NULL);
	///\brief Returns the full list of bkg PDFs
	vector<RooAbsPdf *> GetBkgComponents() { return bkg_components; }
	///\brief Prints to screen the composition e.g signal = NSIG / NTOT, bkg1 = NBKG1/NTOT, etc
	void PrintComposition(float min = 0, float max = 0, RooFitResult * fitRes = NULL);
	///\brief Returns the full list of bkg yields variables
	vector<RooAbsReal *> GetBkgFractions() { return bkg_fractions; }
	///\brief Returns the number of sig events in the full range. Same as GetNSigVal(0,0)
	double GetSigVal(double * valerr = NULL, RooFitResult * fitRes = NULL);
	///\brief Returns the number of sig events in the full range. And can also return its asymmetric error
	double GetSigVal(double * errHi, double * errLo); 

	/** \brief Returns the number of total bkg evens
	 * Returns a RooAbsReal variable which is a RooFormulaVar built as the sum of all bkg yields.
	 */
	RooAbsReal * GetTotNBkg();
	RooAbsReal * GetNSigPtr() { return nsig; }
	///\brief Returns a Str2VarMap object containing all paramters only of the signal PDF 
	Str2VarMap GetSigParams(string opt = "");
	///\brief Returns a Str2VarMap object containing all paramters of the entire model PDF, inclusing yields
	Str2VarMap GetParams(string opt = "");

	static void SetPrintLevel(string mode) { pmode = mode; }

	/** \brief Prints a plots with the model on it using the GetFrame() function.
	 * @param title:  Title for the plot
	 * @param Xtitle: Title for the X axis (can be latex)
	 * @param data:   In case you want to print some data on it (from some external source)
	 * @param bins:   How many bins
	 * @param regStr: This vector containg a list of ranges to print. See GetFrame()
	 * @param range:  Range to print
	 * @param opt:    All options available from GetFrame() plus:
	 * <br> - "-LHCb"/"-LHCbDX" -> Draws the LHCb name on the plot by default on the top left corner (if DX top right)
	 * <br> - "-noleg"          -> Doesn't draw the legend
	 * <br> - "-log"            -> Sets Y axis to logarithmic scale
	 * <br> - "-quality"        -> Draws a box containing fir quality parameters (EDM and covQual)
	 * <br> - "-H"              -> If "data != NULL" produces a separate histogram with pulls distribution
	 * <br> - "-pulls"          -> Produces a separate hitogram with pulls vs variable
	 * <br> - "-andpulls"       -> Produced an histogram with pulls and puts it below the main plot
	 * */
	RooPlot* Print(TString title = "", TString Xtitle = "", string opt = "", RooAbsData* data = NULL, int bins = 0, vector<string> regStr = vector<string>(), double * range = NULL, RooFitResult * fitRes = NULL, TString Ytitle = "");
	
	///\brief Prints all the paramters to screen in RooFit format (opt="-nocost" skips constants)
	void PrintParams(string opt = "") { PrintPars(GetParams(), opt); }
	///\brief Prints the sgnal pdf paramters to screen in RooFit format (opt="-nocost" skips constants)
	void PrintSigParams(string opt = "") { PrintPars(GetSigParams(), opt); }
	///\brief Prints all the paramters to screen in latex table format (opt="-nocost" skips constants)
	void PrintParamsTable(string opt = "") { PrintPars(GetParams(),"-latex"+opt); }
	///\brief Prints all sigal PDF paramters to screen in latex table format (opt="-nocost" skips constants)
	void PrintSigParamsTable(string opt = "") { PrintPars(GetSigParams(),"-latex"+opt); }

	///\brief Returns the value of S(x)/(S(x)+B(x)) for x = value. It corresponds to a naive S-weight.
    float GetReducedSWeight(float value);

};


#endif
