#ifndef STRINGTOPDF_HPP
#define STRINGTOPDF_HPP

#ifndef __CINT__
#include "RooGlobalFunc.h"
#endif

#include <vector>
#include <sstream>
#include <iostream>
#include <map>
#include <string>
#include <time.h>
#include <iomanip>
#include <algorithm>

#include "TAxis.h"
#include "TBranch.h"
#include "TCanvas.h"
#include "TColor.h"
#include "TCut.h"
#include "TEntryList.h"
#include "TF1.h"
#include "TFile.h"
#include "TGaxis.h"
#include "TH1.h"
#include "TIterator.h"
#include "TLegend.h"
#include "TMath.h"
#include "TString.h"
#include "TTree.h"

#include "RooAbsData.h"
#include "RooAbsPdf.h"
#include "RooAddPdf.h"
#include "RooAddition.h"
#include "RooArgusBG.h"
#include "RooBifurGauss.h"
#include "RooBreitWigner.h"
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
#include "RooGExpModel.h"
#include "RooGamma.h"
#include "RooGaussian.h"
#include "RooHistPdf.h"
#include "RooKeysPdf.h"
#include "RooNDKeysPdf.h"
#include "RooNumConvPdf.h"
#include "RooPlot.h"
#include "RooPolynomial.h"
#include "RooProdPdf.h"
#include "RooProduct.h"
#include "RooRealVar.h"
#include "RooStats/SPlot.h"
#include "RooVoigtian.h"
#include "RooWorkspace.h"

#include "generalFunctions.hpp"
#include "treeReader.hpp"
#include "roofit.hpp"
#include "modelBuilderFunctions.hpp"

using namespace std;
using namespace RooFit;


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
RooAbsPdf * stringToPdf(const char * typepdf1, const char * typepdf2, const char * namepdf, RooRealVar * var1, RooRealVar * var2, Str2VarMap myvars = Str2VarMap(), string opt = "", TString title = "");
RooAbsPdf * stringToPdf(RooAbsPdf * pdf1, const char * typepdf, const char * namepdf, RooRealVar * var, Str2VarMap myvars = Str2VarMap(), string opt = "", TString title = "");

RooAbsPdf * get2DRooKeys(string name, TTree * tree, RooRealVar * v1, RooRealVar * v2, string opt = "");
RooAbsPdf * get2DRooKeys(TTree * tree, RooRealVar * v1, RooRealVar * v2, string opt = "");

#endif
