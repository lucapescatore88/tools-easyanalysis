/*****************************************************************************
 * Project: RooFit                                                           *
 *                                                                           *
 * Copyright (c) 2000-2005, Regents of the University of California          *
 *                          and Stanford University. All rights reserved.    *
 *                                                                           *
 * Redistribution and use in source and binary forms,                        *
 * with or without modification, are permitted according to the terms        *
 * listed in LICENSE (http://roofit.sourceforge.net/license.txt)             *
 *****************************************************************************/
// -- CLASS DESCRIPTION [PDF] --
//
// file: RooJohnson.h
// author: Maurizio Martinelli (Nikhef)
// email: maurizio.martinelli@cern.ch
//
// description: this distribution can have highly asymmetric tails and is
//   therefore helpful in fitting the mass difference between D* and D0 mass.
//
// n.b.: the tail parameters are better defined within the following ranges:
//   - nu in ]-inf,+inf[
//   - tau in [0,+inf[
//
// reference: Johnson, N. L. (1954). Systems of frequency curves derived from the first law of Laplace., Trabajos de Estadistica, 5, 283-291.
//
#include "Riostream.h"
#include "RooJohnson.h"
#include "RooAbsReal.h"
#include "RooAbsCategory.h"
#include "RooMath.h"
#include "TMath.h"
ClassImp(RooJohnson)
RooJohnson::RooJohnson(const char *name, const char *title,
               RooAbsReal& _x,
               RooAbsReal& _mean,
               RooAbsReal& _width,
               RooAbsReal& _nu,
               RooAbsReal& _tau) :
RooAbsPdf(name,title),
  x("x","x",this,_x),
  mean("mean","mean",this,_mean),
  width("width","width",this,_width),
  nu("nu","nu",this,_nu),
  tau("tau","tau",this,_tau)
{
}
RooJohnson::RooJohnson(const RooJohnson& other, const char* name) :
  RooAbsPdf(other,name),
  x("x",this,other.x),
  mean("mean",this,other.mean),
  width("width",this,other.width),
  nu("nu",this,other.nu),
  tau("tau",this,other.tau)
{
}
Double_t RooJohnson::evaluate() const
{
  // ENTER EXPRESSION IN TERMS OF VARIABLE ARGUMENTS HERE
  // calculate a few variables
  double w = exp( tau * tau);
  double omega = - nu * tau;
  double c = 0.5 * (w-1) * (w * TMath::CosH(2 * omega) + 1);
  c = pow(c, -0.5);
  double z = (x - (mean + c * width * sqrt(w) * TMath::SinH(omega) )) / c / width;
  double r = -nu + TMath::ASinH(z) / tau;
  // the actual value
  double val = 1. / (c * width * 2 * TMath::Pi());
  val *= 1. / (tau * sqrt(z*z+1));
  val *= exp(-0.5 * r * r);
  // return
  return val ;
}
// Integrals
Int_t
RooJohnson::getAnalyticalIntegral(RooArgSet& allVars, RooArgSet& analVars, const char* /*rangeName*/) const
{
  if (matchArgs(allVars,analVars,x)) return 1 ;
  return 0 ;
}
Double_t
RooJohnson::analyticalIntegral(Int_t code, const char* rangeName) const
{
  assert(code==1) ;
  // calculate a few variables
  double w = exp( tau * tau);
  double omega = - nu * tau;
  double c = 0.5 * (w-1) * (w * TMath::CosH(2 * omega) + 1);
  c = pow(c, -0.5);
  double zmax = (- x.max(rangeName) + (mean + c * width * sqrt(w) * TMath::SinH(omega) )) / c / width;
  double zmin = (- x.min(rangeName) + (mean + c * width * sqrt(w) * TMath::SinH(omega) )) / c / width;
  static const Double_t pi = atan2(0.0,-1.0);
  static const Double_t PiBy2 = pi/2.0;
  static const Double_t rootPiBy2 = sqrt(PiBy2);
  // the integral calculation
  Double_t ret = 0;
  if(code==1){
    //    ret = rootPiBy2 * sigx * (RooMath::erf((x.max(rangeName)-mean)/xscale)-RooMath::erf((x.min(rangeName)-mean)/xscale));
    ret =  -0.25/rootPiBy2* ( RooMath::erf( (nu*tau + TMath::ASinH( zmax ) )/(sqrt(2)*tau) )-
                   RooMath::erf( (nu*tau + TMath::ASinH( zmin ) )/(sqrt(2)*tau) ) );
    /*-rootPiBy2 * ( RooMath::erf( PiBy2 * nu - ( PiBy2 * TMath::ASinH( zmax ) / tau ) ) -
             RooMath::erf( PiBy2 * nu - ( PiBy2 * TMath::ASinH( zmin ) / tau ) ) );
    ret *= c * width;
    ret *= 0.5 / c / width / pi;*/
//     if (gDebug>2) {
//       cout << "Int_gauss_dx(mean=" << mean << ",sigma=" << sigma << ", xmin=" << x.min(rangeName) << ", xmax=" << x.max(rangeName) << ")=" << ret << endl ;
//     }
  } else{
    std::cout << "Error in RooJohnson::analyticalIntegral" << std::endl;
  }
  return ret ;
}


