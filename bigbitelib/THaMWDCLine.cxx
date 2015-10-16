#include "THaMWDCLine.h" 
#include <iostream>

using namespace std;

THaMWDCLine::THaMWDCLine(Double_t *Z, Double_t *Y, Int_t npoints, Double_t t0):TObject(), fTimeOffset(t0){

  Double_t zsum=0, ysum=0;
  
  Int_t i;

  for(i=0; i<npoints; i++){
 
    zsum += Z[i];    
    ysum += Y[i];
  }
  Double_t zavg = zsum/npoints;
  Double_t yavg = ysum/npoints;
  Double_t d;


  Double_t Szy=0;
  Double_t Szz=0;
  for( i=0; i<npoints; i++){
    Szy += Z[i]*(Y[i] - yavg);
    Szz += Z[i]*(Z[i] - zavg);
  }

  if( Szz != 0 )
    {
      fSlope = Szy/Szz;
    }
  else if( npoints !=  0 )
    {
      clog << "Considered " << npoints << endl;
      clog << "Divide by zero in THaMWDCLine::THaMWDCLine" << endl;
    }

  fIntercept = yavg - GetSlope()*zavg;
  Double_t C = 0;
  for(Int_t i=0; i<npoints; i++){
    d = Y[i] - (GetSlope()*Z[i] + GetIntercept());
    C += d*d;
  }
  fChiSquared = C;
}

ClassImp(THaMWDCLine)

