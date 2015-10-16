////////////////////////////////////////////////////////////////////
//
//  THaMWDCLine: a simple line class to facilitate the fitting of 
//  points along a track
// 
////////////////////////////////////////////////////////////////////

#ifndef ROOT_THaMWDCLine_H
#define ROOT_THaMWDCLine_H

#include "TObject.h" 

class THaMWDCLine : public TObject {
 public:
  THaMWDCLine(Double_t *z = NULL, Double_t *y = NULL, Int_t npoints=0, Double_t t0=0.0);
  virtual ~THaMWDCLine(){/*no code needed*/}
  
  Double_t GetSlope() const {return fSlope;}
  Double_t GetIntercept() const {return fIntercept;}
  Double_t GetChiSquared() const {return fChiSquared;}
  Double_t GetTimeOffset() const {return fTimeOffset;}

  Double_t Y(Double_t Z) const {return fSlope*Z + fIntercept;}

 protected:

  Double_t fSlope;
  Double_t fIntercept;
  Double_t fChiSquared;
  Double_t fTimeOffset; 

  ClassDef(THaMWDCLine, 0)
};

#endif
