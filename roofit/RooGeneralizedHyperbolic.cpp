/***************************************************************************** 
 * Project: RooFit                                                           * 
 *                                                                           * 
 * This code was autogenerated by RooClassFactory                            * 
 *****************************************************************************/ 

// Your description goes here... 

#include "Riostream.h" 

#include "RooGeneralizedHyperbolic.h" 
#include "RooAbsReal.h" 
#include "RooAbsCategory.h" 
#include <math.h> 
#include <cmath>
#include "TMath.h" 
#include "Math/SpecFunc.h"
#include "Math/IFunction.h"
#include "gsl/gsl_sf_bessel.h"

Double_t RooGeneralizedHyperbolic__low_x_BK(Double_t nu,Double_t x){
    return TMath::Gamma(nu)*pow(2,nu-1)*pow(x,-nu);
}

Double_t RooGeneralizedHyperbolic__low_x_LnBK(Double_t nu, Double_t x){
    return log(TMath::Gamma(nu)) + (nu-1)*log(2) - nu * log(x);
}

Double_t RooGeneralizedHyperbolic__BK(Double_t ni, Double_t x) {
    Double_t nu = abs(ni);
    if ( x < 1e-06 && nu > 0) return RooGeneralizedHyperbolic__low_x_BK(nu,x);
    if ( x < 1e-04 && nu > 0 && nu < 55) return RooGeneralizedHyperbolic__low_x_BK(nu,x);
    if ( x < 0.1 && nu >= 55) return RooGeneralizedHyperbolic__low_x_BK(nu,x);
 
    return gsl_sf_bessel_Knu(nu, x);
    //return ROOT::Math::cyl_bessel_k(nu, x);
}

Double_t RooGeneralizedHyperbolic__LnBK(double ni, double x) {
    Double_t nu = abs(ni);
    if ( x < 1e-06 && nu > 0) return RooGeneralizedHyperbolic__low_x_LnBK(nu,x);
    if ( x < 1e-04 && nu > 0 && nu < 55) return RooGeneralizedHyperbolic__low_x_LnBK(nu,x);
    if ( x < 0.1 && nu >= 55) return RooGeneralizedHyperbolic__low_x_LnBK(nu,x);
  
    return gsl_sf_bessel_lnKnu(nu, x);
    //return ROOT::Math::cyl_bessel_k(nu, x);
}

Double_t RooGeneralizedHyperbolic__eval(Double_t x, Double_t l, Double_t alpha, Double_t beta, Double_t delta, Double_t mu) {
    Double_t sq2pi = sqrt(2*acos(-1));
    Double_t cons1 = 1./sq2pi;
    Double_t gamma = sqrt(alpha*alpha-beta*beta);
    Double_t dg = delta*gamma;
    //Double_t mu_ = mu;// - delta*beta*RooGeneralizedHyperbolic__BK(l+1,dg)/(gamma*RooGeneralizedHyperbolic__BK(l,dg));
    Double_t d = x-mu;
    Double_t thing = sqrt(delta*delta + d*d);
    Double_t no = pow(gamma/delta,l)/RooGeneralizedHyperbolic__BK(l,dg)*cons1;
    Double_t num = no*RooGeneralizedHyperbolic__BK(l-0.5,thing*alpha);
    Double_t den = pow(thing/alpha,0.5-l);
    return  exp(beta*d)*num/den ;
}

Double_t RooGeneralizedHyperbolic__LogEval(Double_t x, Double_t l, Double_t alpha, Double_t beta, Double_t delta, Double_t mu) {
    Double_t d = x-mu;
    Double_t sq2pi = sqrt(2*acos(-1));
    Double_t gamma = sqrt(alpha*alpha-beta*beta);
    Double_t dg = delta*gamma;
    Double_t thing = delta*delta + d*d;
    Double_t logno = l*log(gamma/delta) - log(sq2pi) -RooGeneralizedHyperbolic__LnBK(l, dg);
    return exp(logno + beta*d +(0.5-l)*(log(alpha)-0.5*log(thing)) + RooGeneralizedHyperbolic__LnBK(l-0.5,alpha*sqrt(thing)) );

}



ClassImp(RooGeneralizedHyperbolic) 

RooGeneralizedHyperbolic::RooGeneralizedHyperbolic(const char *name, const char *title, 
						   RooAbsReal& _x,
						   RooAbsReal& _l,
						   RooAbsReal& _alpha,
						   RooAbsReal& _beta,
						   RooAbsReal& _delta,
						   RooAbsReal& _mu) :
RooAbsPdf(name,title), 
    x("x","x",this,_x),
    l("l","l",this,_l),
    alpha("alpha","alpha",this,_alpha),
    beta("beta","beta",this,_beta),
    delta("delta","delta",this,_delta),
    mu("mu","mu",this,_mu)
{ 
} 

RooGeneralizedHyperbolic::RooGeneralizedHyperbolic(const RooGeneralizedHyperbolic& other, const char* name) :  
    RooAbsPdf(other,name), 
    x("x",this,other.x),
    l("l",this,other.l),
    alpha("alpha",this,other.alpha),
    beta("beta",this,other.beta),
    delta("delta",this,other.delta),
    mu("mu",this,other.mu)
{ 
} 

Double_t RooGeneralizedHyperbolic::evaluate() const 
{ 
    Double_t d = x-mu;
    Double_t thing = 0;
    Double_t out = 0.;
  
    if (alpha!= 0.) {
	//out = eval(x,l,alpha,beta,delta, mu);
	out = RooGeneralizedHyperbolic__LogEval(x,l,alpha,beta,delta, mu);
    }
    else if (l < 0) {
	thing = 1 + d*d/(delta*delta);
	out = exp(beta*d)*pow(thing,l-0.5);
    }
    return out;   
}
