///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// TimeToDistConv                                                            //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "TimeToDistConv.h"
#include "TMath.h"

ClassImp(TreeSearch::TimeToDistConv)
ClassImp(TreeSearch::LinearTTD)
ClassImp(TreeSearch::TanhFitTTD)
ClassImp(TreeSearch::Pol2FitTTD)

namespace TreeSearch {

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// LinearTTD                                                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

//_____________________________________________________________________________
LinearTTD::LinearTTD() : TimeToDistConv(1), fDriftVel(kBig)
{
  // Constructor.
}

//_____________________________________________________________________________
Double_t LinearTTD::GetParameter( UInt_t i ) const
{
  // Get i-th parameter

  return (i==0) ? GetDriftVel() : kBig;
}

//_____________________________________________________________________________
Int_t LinearTTD::SetParameters( const vector<double>& parameters )
{
  // Set parameters. The first element of parameter array is interpreted as
  // the drift velocity in m/s. Further elements are ignored.

  if( parameters.empty() )
    return -1;

  fDriftVel = parameters[0];
  return 0;
}

//_____________________________________________________________________________
Double_t LinearTTD::ConvertTimeToDist( Double_t time, Double_t slope ) const
{
  // Time in s. Return distance in m. slope is used to project the distance
  // of closest approach onto the wire plane (1/cos correction).
  
  if( time <= 0.0 )
    return 0.0;

  // Don't bother with very small slopes
  if( TMath::Abs(slope) < 1e-2 )
    return fDriftVel * time;
  
  // NB: 1/cos = sqrt(1+tan^2)
  return fDriftVel * time *  TMath::Sqrt( 1.0 + slope*slope );
}


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// TanhFitTTD                                                                //
//                                                                           //
// Fit from S. Riordan's Ph.D. thesis, CMU, 2008                             //
// d = c0 * tanh( (v*(t-t0) + c2*(t-t0)^2)/c0 )                              //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

//_____________________________________________________________________________
TanhFitTTD::TanhFitTTD()
  : TimeToDistConv(4), 
    fDriftVel(kBig), fC0(kBig), fC2(kBig), fT0(kBig), fInvC0(0)
			   
{
  // Constructor.
}

//_____________________________________________________________________________
Double_t TanhFitTTD::GetParameter( UInt_t i ) const
{
  // Get i-th parameter

  switch(i) {
  case 0:
    return fDriftVel;
  case 1:
    return fC0;
  case 2:
    return fC2;
  case 3:
    return fT0;
  }
  return kBig;
}

//_____________________________________________________________________________
Int_t TanhFitTTD::SetParameters( const vector<double>& parameters )
{
  // Set parameters of the tanh fit:
  // 0: drift velocity (m/s)
  // 1: c0 (m)
  // 2: c2 (m/s^2)
  // 3: t0 (s)

  if( (UInt_t)parameters.size() < fNparam )
    return -1;

  fDriftVel = parameters[0];
  fC0       = parameters[1];
  fC2       = parameters[2];
  fT0       = parameters[3];
  fInvC0    = 1.0/fC0;
  return 0;
}

//_____________________________________________________________________________
Double_t TanhFitTTD::ConvertTimeToDist( Double_t time, Double_t slope ) const
{
  // Convert time (s) to distance (m) using fit to tanh function.
  // Slope is used to project the distance of closest approach onto the wire
  // plane (1/cos correction).

  Double_t t = time - fT0;
  if( t <= 0.0 )
    return 0.0;
  Double_t d = fC0 * TMath::TanH( (fDriftVel*t +  fC2*t*t) * fInvC0 );
  // NB: 1/cos = sqrt(1+tan^2)
  return d * TMath::Sqrt( 1.0 + slope*slope );
}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Pol2FitTTD                                                                //
//                                                                           //
// Fit From Xin Qian                              //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

//_____________________________________________________________________________
Pol2FitTTD::Pol2FitTTD()
  : TimeToDistConv(14)
			   
{
  // Constructor.
  for (Int_t i=0;i!=14;i++){
    f1[i] = kBig;
  }
}

//_____________________________________________________________________________
Double_t Pol2FitTTD::GetParameter( UInt_t i ) const
{
  // Get i-th parameter

  if (i>=0&&i<14) return f1[i];
  return kBig;
}

//_____________________________________________________________________________
Int_t Pol2FitTTD::SetParameters( const vector<double>& parameters )
{

  if( (UInt_t)parameters.size() < fNparam )
    return -1;
  for (Int_t i=0;i!=14;i++){
    f1[i] = parameters[i];
  }
  return 0;
}

//_____________________________________________________________________________
Double_t Pol2FitTTD::ConvertTimeToDist( Double_t time, Double_t slope ) const
{
  Double_t t = time;
  if( t <= 0.0 )
    return 0.0;
  Double_t d;
  Double_t t1 = 20e-9;
  const Double_t *par = &f1[0];
  if (t<15e-9){
    d = *(par) + *(par+1)*t + *(par+2)*t*t ;
  }else if (t>=15e-9&&t<20e-9){
    d = (*(par) + *(par+1)*t + *(par+2)*t*t)*(20e-9-t) + (*(par+3) + *(par+4)*t + *(par+5)*t*t)*(t-15e-9);
    d = d/5e-9;
  }else if (t>=20e-9&&t<60e-9){
    d = *(par+3) + *(par+4)*t + *(par+5)*t*t;
  }else if (t>=60e-9&&t<70e-9){
    d = (*(par+3) + *(par+4)*t + *(par+5)*t*t)*(70e-9-t) + (*(par+6) + *(par+7)*t + *(par+8)*t*t)*(t-60e-9);
    d = d/10e-9;
  }else if (t>=70e-9&&t<100e-9){
    d = *(par+6) + *(par+7)*t + *(par+8)*t*t;
  }else if (t>=100e-9&&t<110e-9){
    d = (*(par+6) + *(par+7)*t + *(par+8)*t*t)*(110e-9-t)+(*(par+9) + *(par+10)*t + *(par+11)*t*t)*(t-100e-9);
    d = d/10e-9;
  }else if (t>=110e-9&&t<=150e-9){
    d = *(par+9) + *(par+10)*t + *(par+11)*t*t;
  }else if (t>=150e-9&&t<160e-9){
    d = (*(par+9) + *(par+10)*t + *(par+11)*t*t)*(160e-9-t) + (*(par+12)+*(par+13)*t)*(t-150e-9);
    d = d/10e-9;
  }else{
    d = *(par+12)+*(par+13)*t;
  }

  return d * TMath::Sqrt( 1.0 + slope*slope );
}

///////////////////////////////////////////////////////////////////////////////





}  // end namespace TreeSearch
