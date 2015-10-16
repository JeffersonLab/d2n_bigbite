#ifndef ROOT_THaTdcHit
#define ROOT_THaTdcHit

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// THaTdcHit                                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "TObject.h"
#include "TRef.h"
#include "THaScintPMT.h"
#include <cstdio>

class THaTdcHit : public TObject {
  
 public:
  THaTdcHit(const THaScintPMT* pmt=NULL, Int_t rawtime=0, Double_t ext_offset=0);
  virtual ~THaTdcHit() {}

  THaScintPMT* GetPMT() const { return (THaScintPMT*)fPMT.GetObject(); }

  Int_t    GetRawTime() const {return fRawTime;}
  Double_t GetTime() const {return fTime;}
  Int_t    GetBarNum() const {return fBarNum;}
  Int_t    GetSide() const {return fSide;}

  void UpdateTime(Double_t ext_offset=0.);
  void SetRawTime(Int_t rawtime) { fRawTime = rawtime;}

  Bool_t IsSortable() const { return kTRUE; }

  Int_t  Compare(const TObject* obj) const;

  void   Clear(Option_t *s="");
  
 protected:

  TRef fPMT;  // reference to "real" PMT, kept in detector arrays
  Int_t fRawTime;
  Double_t fTime;
  Int_t fBarNum;
  Int_t fSide;

 public:
  ClassDef(THaTdcHit,1)  // TDC and real-time for a Hit. Per PMT
};
/////////////////////////////////////////////////////////////////
#endif
