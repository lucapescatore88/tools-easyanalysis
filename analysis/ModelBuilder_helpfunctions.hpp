#ifndef MODEL_BUILDER_FUNCTIONS_HPP
#define MODEL_BUILDER_FUNCTIONS_HPP

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
#include "RooGExpModel.h"
#include "RooArgusBG.h"
#include "RooGamma.h"
#include "RooKeysPdf.h"
#include "RooNDKeysPdf.h"
#include "RooHistPdf.h"
#include "RooProdPdf.h"
#include "RooPolynomial.h"

#include "RooAddPdf.h"
#include "RooDataSet.h"
#include "RooChi2Var.h"
#include "RooDataHist.h"
#include "RooAbsData.h"
#include "RooAddition.h"
#include "RooProduct.h"
#include "RooAbsPdf.h"
#include "RooFit.h"
#include "RooFitResult.h"
#include "RooWorkspace.h"
#include "RooConstVar.h"
#include "RooNumConvPdf.h"
#include "RooFFTConvPdf.h"
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
#include <algorithm>

#include "general_functions.hpp"
#include "ReadTree_comp.hpp"
#include "roofit.hpp"

using namespace std;


///\brief This type is supposed to contain a list of pointers to RooRealVars (parameters) collable by name 

typedef std::map <string, RooAbsReal *> Str2VarMap;
typedef std::map <string, RooAbsReal *>::iterator Str2VarMapItr;

vector <Color_t> GetDefaultColors();

/// \brief Given a parameter name return a RooRealVar pointer
RooRealVar * addPar(string par, string parstr, Str2VarMap stval_list, Str2VarMap myvars, string pmode = "v");

/// \brief Given a PDF type it returns the list of needed parameters to built a RooAbsPdf
Str2VarMap getPar(string typepdf_, TString namepdf_, RooRealVar * val, Str2VarMap myvars = Str2VarMap(), string pmode = "v", TString title = "");

/** \brief Given a PDF name returns a RooAbsPdf
 * Available PDFs: Gauss, DGauss, CB, DCB, Argus, Exp, Poly, Cheb
 * @param typepdf: The PDF type (CB,Gauss,etc)
 * @param namepdf: The name of the PDF e.g. Ugly_Bkg. The PDF will be named (type)_(name)
 * @param myvars:  While creating parameters for the RooAbsPdf it will go through this list and if parameters with the required names are found
 * it will clone their properties and use the for RooAbsPdf
 * @param pmode : Print level flag (verbose "v", silent "s")
 * */
RooAbsPdf * stringToPdf(const char * typepdf, const char * namepdf, RooRealVar * var, Str2VarMap myvars = Str2VarMap(), string opt = "", TString title = "");
RooAbsPdf * stringToPdf(const char * typepdf1, const char * typepdf2, const char * namepdf, RooRealVar * var1, RooRealVar * var2, Str2VarMap myvars = Str2VarMap(), string opt = "-vn", TString title = "");
RooAbsPdf * stringToPdf(RooAbsPdf * pdf1, const char * typepdf, const char * namepdf, RooRealVar * var, Str2VarMap myvars = Str2VarMap(), string opt = "-vn", TString title = "");

RooAbsPdf * get2DRooKeys(string name, TTree * tree, RooRealVar * v1, RooRealVar * v2, string opt = "");
RooAbsPdf * get2DRooKeys(TTree * tree, RooRealVar * v1, RooRealVar * v2, string opt = "");


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
//RooAbsPdf * corrGaussConstraints(RooAbsPdf * pdf, RooArgSet vars);

string isParInMap( string par, Str2VarMap myvars, string option = "");

/** \brief Returns the pointer to the parameter named "named" of PDF.
 * Returns null if it doesn't find any.
 * */

RooRealVar * GetParam(RooAbsPdf * pdf, string name, string opt = "");

void GetParam(RooFitResult *fRes, string name, double &par, double &parE, string type = "f");
double GetParVal(RooFitResult *fRes, string name, string type = "f");
double GetParErr(RooFitResult *fRes, string name, string type = "f");



/** \brief Gets the list of parameters of a RooAbsPdf in Str2VarMap form
 * If any name is specified in pnames these names are used to make a selection of parameters. If "pnames" is empty all parameters are returned.
 * opt=="-origNames" keeps the names as they are otherwise keeps only the part before the underscore "(alwayskept)_(optional)"
 */

Str2VarMap GetParams(RooAbsPdf * pdf, RooArgSet obs, vector < string > pnames, string opt = "");
Str2VarMap GetParamList(RooAbsPdf * pdf, RooAbsReal * var, string opt = "");
Str2VarMap GetParamList(RooAbsPdf * pdf, RooArgSet obs = RooArgSet(), string opt = "");

bool checkModel(RooAbsPdf * model);


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


RooPlot * GetFrame(RooRealVar * var, RooAbsData * data, RooAbsPdf * model = NULL, string opt = "", 
        unsigned bins = 50, double * range = NULL, vector<string> regStr = vector<string>(1,"PlotRange"),
        TString Xtitle = "", TString Ytitle = "", TLegend * leg = NULL, vector <Color_t> custom_colors = vector <Color_t>());
RooPlot * GetFrame(RooRealVar * var, RooAbsPdf * model, RooAbsData * data = NULL, string opt = "", 
        unsigned bins = 50, double * range = NULL, vector<string> regStr = vector<string>(1,"PlotRange"), 
        TString Xtitle = "", TString Ytitle = "", TLegend * leg = NULL, vector <Color_t> custom_colors = vector <Color_t>());
RooPlot * GetFrame(RooRealVar * var, RooAbsData * data, RooAbsPdf * model, string opt, 
        unsigned bins, TString Xtitle, TString Ytitle = "", TLegend * leg = NULL, vector <Color_t> custom_colors = vector <Color_t>());
RooPlot * GetFrame(RooRealVar * var, RooAbsPdf * model, RooAbsData * data, string opt,
        unsigned bins, TString Xtitle, TString Ytitle = "", TLegend * leg = NULL, vector <Color_t> custom_colors = vector <Color_t>());

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
TPaveText * createParamBox(RooAbsPdf * pdf, RooRealVar * obs, string opt, RooFitResult * fitRes = NULL);

/** Prints the parameters in a Str2VarMap object
 * <br> opt == "-nocost"  ->  doesn't print constants
 * <br> opt == "-latex"   ->  prints the Title instead of Name of variables assuming latex format
 */
void PrintPars(Str2VarMap pars, string opt);

TString getLegendLabel( TString title, string opt = "" );


#endif
