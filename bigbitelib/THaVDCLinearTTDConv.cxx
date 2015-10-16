///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// THaVDCLinearTTDConv                                                        //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>
#include "TMath.h"
#include "THaVDCLinearTTDConv.h"

using namespace std;

THaVDCLinearTTDConv::THaVDCLinearTTDConv()
{
}

THaVDCLinearTTDConv::THaVDCLinearTTDConv( Double_t driftvelocity )
{
  fDriftVelocity = driftvelocity;
}


Double_t THaVDCLinearTTDConv::ConvertTimeToDist( Double_t time, Double_t tanTheta, Double_t *drift_dist )
{
  // clog << "Called conversion" << endl;
  if( drift_dist )
    {
      //      clog << "Assigning to drift dist!" << endl;
          *drift_dist =  fDriftVelocity * time / TMath::Cos( TMath::ATan(TMath::Abs(tanTheta)));
      //*drift_dist = fDriftVelocity * time;
    }
  
  return fDriftVelocity * time / TMath::Cos( TMath::ATan(TMath::Abs(tanTheta)));
  //clog << "Returning a value of " << fDriftVelocity * time << endl;
  //return fDriftVelocity * time;
}


Double_t THaVDCLinearTTDConv::ConvertDistToTime( Double_t plane_intercept, Double_t tanTheta )
{
  return plane_intercept / fDriftVelocity;
}

THaVDCLinearTTDConv::~THaVDCLinearTTDConv()
{
}

ClassImp(THaVDCLinearTTDConv)
