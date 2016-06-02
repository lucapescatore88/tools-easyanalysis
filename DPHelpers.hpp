#ifndef DPHELPERS_HH
#define DPHELPERS_HH

#include "TLorentzVector.h"
#include "pkmumu_functions.hpp"
namespace DPHelpers
{
  double daughterMomentum(double mR, double m1, double m2);
  void LbPsiRAngles(TLorentzVector initialProton, TLorentzVector pB,
                     TLorentzVector pJpsi, TLorentzVector pLambda, TLorentzVector pmp,
                     TLorentzVector pmm, TLorentzVector pp, TLorentzVector ppi, int pCharge, 
                     float& cosTheta, float& cosThetaL, float& cosThetaB,
                     float& phiL, float& phiB, float& dphi);
};

#endif
