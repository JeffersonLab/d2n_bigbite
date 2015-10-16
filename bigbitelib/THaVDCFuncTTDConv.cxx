///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// THaVDCFuncTTDConv                                                        //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>
#include "TMath.h"
#include "THaVDCFuncTTDConv.h"

using namespace std;

THaVDCFuncTTDConv::THaVDCFuncTTDConv()
{
}

THaVDCFuncTTDConv::THaVDCFuncTTDConv( Double_t driftvelocity, Double_t c1, Double_t c2, Double_t c3 )
{
  fDriftVelocity = driftvelocity;
  fC1 = c1;
  fC2 = c2;
  fC3 = c3;
}


Double_t THaVDCFuncTTDConv::ConvertTimeToDist( Double_t time, Double_t tanTheta, Double_t *drift_dist )
{
   Double_t drift_d = (fC1*TMath::TanH(((time-fC3)*fDriftVelocity+fC2*(time-fC3)*(time-fC3))/fC1))/TMath::Cos(TMath::ATan(TMath::Abs(tanTheta)));
  //  Double_t drift_d = (fC1*TMath::TanH((time)*fDriftVelocity+fC2*(time)*(time))/fC1)/TMath::Cos(TMath::ATan(TMath::Abs(tanTheta)));

  if( drift_d < 0 ) drift_d = 0.0;

  // clog << "Called conversion" << endl;
  if( drift_dist )
    {
      *drift_dist = drift_d;
    }
  
  return drift_d;
}


Double_t THaVDCFuncTTDConv::ConvertDistToTime( Double_t plane_intercept, Double_t tanTheta )
{
  return TMath::ATanH(plane_intercept*(TMath::Cos(TMath::ATan(TMath::Abs(tanTheta))))/fC1)*fC1/fDriftVelocity ;
}

THaVDCFuncTTDConv::~THaVDCFuncTTDConv()
{
}

ClassImp(THaVDCFuncTTDConv)
