/*****************************************************************************
 * Project: RooFit                                                           *
 *                                                                           *
 * Copyright (c) 2000-2007, Regents of the University of California          *
 *                          and Stanford University. All rights reserved.    *
 *                                                                           *
 * Redistribution and use in source and binary forms,                        *
 * with or without modification, are permitted according to the terms        *
 * listed in LICENSE (http://roofit.sourceforge.net/license.txt)             *
 *****************************************************************************/
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

#ifndef ROOJOHNSONSU
#define ROOJOHNSONSU

#include "RooAbsPdf.h"
#include "RooRealProxy.h"
#include "RooCategoryProxy.h"
#include "RooAbsReal.h"
#include "RooAbsCategory.h"

class RooJohnson : public RooAbsPdf {

 public:
    RooJohnson() {} ;
    RooJohnson(const char *name, const char *title,
	       RooAbsReal& _x,
	       RooAbsReal& _mean,
	       RooAbsReal& _width,
	       RooAbsReal& _nu,
	       RooAbsReal& _tau);
    RooJohnson(const RooJohnson& other, const char* name=0) ;
    virtual TObject* clone(const char* newname) const { return new RooJohnson(*this,newname); }
    inline virtual ~RooJohnson() { }

 protected:
    RooRealProxy x ;
    RooRealProxy mean ;
    RooRealProxy width ;
    RooRealProxy nu ;
    RooRealProxy tau ;
    Double_t evaluate() const ;
    // Integrals
    Int_t getAnalyticalIntegral(RooArgSet& allVars, RooArgSet& analVars, const char* rangeName=0) const ;
    Double_t analyticalIntegral(Int_t code, const char* rangeName=0) const ;

 public:
    ClassDef(RooJohnson,1) // Your description goes here...

};

#endif

