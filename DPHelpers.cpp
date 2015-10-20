#include "DPHelpers.hpp"
#include "TMath.h"
#include "TLorentzVector.h"
#include <iostream>

double DPHelpers::daughterMomentum(double m, double m1, double m2)
{
	double momentum;

	momentum=(m*m-(m1+m2)*(m1+m2))*(m*m-(m1-m2)*(m1-m2));
	momentum=TMath::Sqrt(momentum);
	momentum/=2*m;

	return momentum;
}


/*
 * Calculation of all relevant angles for Lb decays
 */
void DPHelpers::LbPsiRAngles(TLorentzVector initialProton, TLorentzVector pB,
                     TLorentzVector pJpsi, TLorentzVector pLambda, TLorentzVector pmp,
                     TLorentzVector pmm, TLorentzVector pp, TLorentzVector ppi, int pCharge, 
                     float& cosTheta, float& cosThetaL, float& cosThetaB,
                     float& phiL, float& phiB, float& dphi)
{

  bool decayLambda=true;
  if ( pp.M()<1e-5 )  // Should be zero exactly, but allow for numerical treatment
  {
    decayLambda=false;
  }
  TVector3 analyzer;
  analyzer=initialProton.Vect().Cross(pB.Vect());
  analyzer*=1.0/analyzer.Mag();

  // Move everything to Lambda_b rest frame
  if ( pB.Vect().Mag() > 1e-6 )
  {
    TVector3 boost=-1.0*pB.BoostVector();
    pB.Boost(boost);
    pJpsi.Boost(boost);
    pLambda.Boost(boost);
    pmp.Boost(boost);
    pmm.Boost(boost);
    if ( decayLambda )
    {
      pp.Boost(boost);
      ppi.Boost(boost);
    }
  }
  // First level
  cosTheta=analyzer.Dot(pLambda.Vect())/analyzer.Mag()/pLambda.Vect().Mag();

  // Frame 1
  TVector3 z1=pLambda.Vect()*(1./pLambda.Vect().Mag());
  TVector3 y1=analyzer.Cross(z1);
  y1=y1*(1./y1.Mag());
  TVector3 x1=z1.Cross(y1); x1=x1*(1./x1.Mag());
  // Proton in Lambda rest frame
  TLorentzVector ppLRest=pp;
  TVector3 h1;
  if ( decayLambda )
  {
    ppLRest.Boost(-1.0*pLambda.BoostVector());
    cosThetaB=z1.Dot(ppLRest.Vect())/ppLRest.Vect().Mag();
    TVector3 pperp=ppLRest.Vect()-1.0*ppLRest.Vect().Mag()*z1*cosThetaB;
    double cosPhi=pperp.Dot(x1)/pperp.Mag();
    double sinPhi=pperp.Dot(y1)/pperp.Mag();
    phiB=TMath::ACos(cosPhi);
    if ( sinPhi<0 )
      phiB=-phiB;
//   phiB=2*TMath::Pi()-phiB;

    h1=( (ppLRest.Vect().Cross(pLambda.Vect())));
  }
  // Frame 2
  TVector3 z2=pJpsi.Vect()*(1./pJpsi.Vect().Mag());
  TVector3 y2=analyzer.Cross(z2);
  y2=y2*(1./y2.Mag());
  TVector3 x2=z2.Cross(y2); x2=x2*(1./x2.Mag());
  // Proton in Lambda rest frame
  ppLRest=pmp;
  ppLRest.Boost(-1.0*pJpsi.BoostVector());
  cosThetaL=z2.Dot(ppLRest.Vect())/ppLRest.Vect().Mag();
  TVector3 pperp=ppLRest.Vect()-1.0*ppLRest.Vect().Mag()*z2*cosThetaL;
  double cosPhi=pperp.Dot(x2)/pperp.Mag();
  double sinPhi=pperp.Dot(y2)/pperp.Mag();
  phiL=TMath::ACos(cosPhi);
  if ( sinPhi<0 )
   phiL=-phiL;

  if ( decayLambda )
  {
    TVector3 h2( (ppLRest.Vect().Cross(pJpsi.Vect())));

  // Calculate delta phi
//  TVector3 h1((pmp.Vect()).Cross(pmm.Vect()));
//  TVector3 h2((pp.Vect()).Cross(ppi.Vect()));
    TVector3 dir(h1.Cross(h2));
    float a1=dir.Dot(pJpsi.Vect())/dir.Mag()/pJpsi.Vect().Mag();
    a1=TMath::ACos(a1);
    dphi=h1.Dot(h2)/h1.Mag()/h2.Mag();
//  infoToStore.dphi=TMath::Pi()+TMath::ACos(infoToStore.dphi);
    dphi=TMath::ACos(dphi);
    if ( a1 > TMath::Pi() -0.001 )
    {
      dphi=-dphi;
    }
  }

  dphi=-999;
  if (decayLambda)
  {
    //
    TVector3 p3KPi = pLambda.Vect();
    TVector3 p3K = pp.Vect();
    TVector3 p3Psi = pJpsi.Vect();
    TVector3 p3MuPlus = pmp.Vect();

    TVector3 aK = p3K - p3KPi * (p3K.Dot(p3KPi) / p3KPi.Mag2());
    TVector3 aMuPlus = p3MuPlus - p3Psi * (p3MuPlus.Dot(p3Psi) / p3Psi.Mag2());

    // angle between K* and Psi decay planes in B0 rest frame
    dphi = atan2((p3Psi.Cross(aK)).Dot(aMuPlus) /
                     (p3Psi.Mag() * aK.Mag() * aMuPlus.Mag()),
                 aK.Dot(aMuPlus) / (aK.Mag() * aMuPlus.Mag()));

    if (std::isnan(dphi))
    {
      // std::cout << "phi is nan" << std::endl;
      dphi = 0.;
    }
  }

// Now we handle antiparticles (charge of the proton being negative)

  if ( pCharge < 0 )
  {
    cosThetaL*=-1;
    if ( phiB > 0 )
    {
      phiB=TMath::Pi()-phiB;
    }
    else
    {
      phiB=-TMath::Pi()-phiB;
    }
    if ( phiL > 0 )
    {
      phiL=TMath::Pi()-phiL;
    }
    else
    {
      phiL=-TMath::Pi()-phiL;
    }
    if (dphi > -999)
    {
      if (dphi > 0)
      {
        dphi = TMath::Pi() - dphi;
      }
      else
      {
        dphi = -TMath::Pi() - dphi;
      }
    }
  }
  
}


