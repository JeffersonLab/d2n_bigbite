#ifndef ROOT_THaAdcHit
#define ROOT_THaAdcHit

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// THaAdcHit                                                               //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "TObject.h"
#include "TRef.h"
#include "THaScintPMT.h"

class THaAdcHit : public TObject {
  
 public:
  THaAdcHit(THaScintPMT* pmt=NULL, Int_t rawampl=0); 
  virtual ~THaAdcHit() {}

  THaScintPMT* GetPMT() const { return (THaScintPMT*)fPMT.GetObject(); }
  Int_t GetRawAmpl() const {return fRawAmpl;}
  Int_t GetAmplPedCor() const {return fAmplPedCor;}
  Double_t GetAmpl() const {return fAmpl;}
  Int_t GetBarNum() const {return fBarNum;}
  Int_t GetSide() const {return fSide;}

  void CorrectHit();
  void SetRawAmpl(Int_t rawampl) { fRawAmpl = rawampl;}
  void SetBarNum(Int_t barnum) {fBarNum=barnum;}
  void SetSide(Int_t side) {fSide=side;}

  Bool_t IsSortable() const { return kTRUE; }

  Int_t  Compare(const TObject* obj) const;
  void Clear(Option_t *s="");

 protected:

  TRef fPMT;  // reference to "real" PMT, kept in detector arrays
  Int_t fRawAmpl;
  Int_t fAmplPedCor;
  Double_t fAmpl;
  Int_t fBarNum;
  Int_t fSide;

  ClassDef(THaAdcHit,1)  // ADC and calibrated amplitude hit
};
/////////////////////////////////////////////////////////////////
#endif
