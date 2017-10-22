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
RooAbsPdf * stringToPdf(const char * typepdf1, const char * typepdf2, const char * namepdf, RooRealVar * var1, RooRealVar * var2, Str2VarMap myvars = Str2VarMap(), string opt = "-vn", TString title = "");
RooAbsPdf * stringToPdf(RooAbsPdf * pdf1, const char * typepdf, const char * namepdf, RooRealVar * var, Str2VarMap myvars = Str2VarMap(), string opt = "-vn", TString title = "");

RooAbsPdf * get2DRooKeys(string name, TTree * tree, RooRealVar * v1, RooRealVar * v2, string opt = "");
RooAbsPdf * get2DRooKeys(TTree * tree, RooRealVar * v1, RooRealVar * v2, string opt = "");

#endif
