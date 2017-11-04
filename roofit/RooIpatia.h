/*****************************************************************************
 * Project: RooFit                                                           *
 *                                                                           *
  * This code was autogenerated by RooClassFactory                            * 
 *****************************************************************************/

#ifndef ROOIPATIA
#define ROOIPATIA

#include "RooAbsPdf.h"
#include "RooRealProxy.h"
#include "RooCategoryProxy.h"
#include "RooAbsReal.h"
#include "RooAbsCategory.h"
 
class RooIpatia : public RooAbsPdf {

 public:
    RooIpatia() {} ; 
    RooIpatia(const char *name, const char *title,
	      RooAbsReal& _x,
	      RooAbsReal& _l,
	      RooAbsReal& _zeta,
	      RooAbsReal& _fb,
	      RooAbsReal& _sigma,
	      RooAbsReal& _mu,
	      RooAbsReal& _a,
	      RooAbsReal& _n);
    RooIpatia(const RooIpatia& other, const char* name=0) ;
    virtual TObject* clone(const char* newname) const { return new RooIpatia(*this,newname); }
    inline virtual ~RooIpatia() { }

 protected:
    RooRealProxy x ;
    RooRealProxy l ;
    RooRealProxy zeta ;
    RooRealProxy fb ;
    RooRealProxy sigma ;
    RooRealProxy mu ;
    RooRealProxy a ;
    RooRealProxy n ;
    Double_t evaluate() const ;

 public:
    ClassDef(RooIpatia,1) // Your description goes here...

};
 
#endif
