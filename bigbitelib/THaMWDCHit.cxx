#include "THaMWDCHit.h" 
#include "THaMWDCPlane.h"
#include "TMath.h"

#include <iostream>

using namespace std;

#define MAXIMUM_DRIFT 0.01

//-----------------------------------------------------------------------------
THaMWDCHit::THaMWDCHit(THaMWDCPlane* plane, THaVDCWire* wire, Int_t rawtime,
                       Double_t time, Double_t offset, Double_t reftime, Bool_t valid) : 
THaVDCHit(wire, rawtime, time), fPlane(plane), fOffset(offset), fRefTime(reftime), 
fCoord(0.0), fTrackNum(-1)
{
    fLR             = kUnDef;
    fHitStatus      = valid ? kDataValid : 0;
    fScintOffset    = 0.0;
    fChi2Cont       = 0.0;
    fCoarseChi2Cont = 0.0;
}

Double_t THaMWDCHit::GetCoord(Double_t slope, ELR LR, Double_t t0)
{
    // If LR is undefined just return the wire position if LR isn't
    // flagged as fixed.  If it is fixed then we should have LR information

    if( LR == kNULL )
    {
        LR = fLR;
    }

    if( LR == kUnDef )
    {
        fHitStatus |= kCoordValid;
        return GetPos();
    }

    Double_t effTime  = GetTime() - t0 - fScintOffset;
    Double_t absSlope = TMath::Abs(slope);
    Double_t angle    = TMath::ATan(absSlope);
    Double_t cosAngle = TMath::Cos(angle);

    SetDist( TMath::Abs(fPlane->ConvertDriftTimeToDistance(effTime, slope)) );  
    SetdDist( TMath::Abs(GetDist()*cosAngle) );

    //Coordinate is valid if the drift distance isn't outrageous
    if( GetdDist() > MAXIMUM_DRIFT )
    {
        fHitStatus &= ~kCoordValid;    
    }
    else
    {
        fHitStatus |= kCoordValid;
    }


    switch(LR){
  case kL: //left
      return GetPos() - GetDist();
      break;
  case kR: //right
      return GetPos() + GetDist();
      break;
  case kUnDef:
      return GetPos();
      break;
  default:
      clog << "Fed bad LR coordinate " << LR << endl;
      fHitStatus &= ~kCoordValid;
      return GetPos();
      break;
    }
}

ClassImp(THaMWDCHit)
