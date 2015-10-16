///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// THaTdcHit                                                                 //
//                                                                           //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "THaTdcHit.h"
#include "THaScintPMT.h"
using namespace std;

//_____________________________________________________________________________
THaTdcHit::THaTdcHit(const THaScintPMT* pmt, Int_t rawtime, Double_t ext_offset):
  fPMT(const_cast<THaScintPMT*>(pmt)), fRawTime(rawtime) {

  if (pmt) {
    fBarNum = pmt->GetBarNum();
    fSide   = pmt->GetSide();
  } else {
    fBarNum = 0;
    fSide   = 0;
  }
  UpdateTime(ext_offset);
}

//_____________________________________________________________________________
void THaTdcHit::UpdateTime(Double_t ext_offset) {
  const THaScintPMT *pmt = GetPMT();
  
  if (pmt) {
    fTime = fRawTime * pmt->GetTDCRes() - pmt->GetTOffset() - ext_offset;
  } else {
    fTime = 0;
  }
}

//_____________________________________________________________________________
Int_t THaTdcHit::Compare(const TObject *obj) const {
  // sort THaTdcHits according to side, bar number, and time
  //   obj should be a THaTdcHit, and we will assume as much
  //
  // Returns -1 if this < obj, 0 if this==obj, and 1 if this>obj
  const THaTdcHit *h = static_cast<const THaTdcHit*>(obj);

  // Side (Left-right) comparison
  if (fSide < h->fSide) return -1;
  if (fSide > h->fSide) return  1;

  // Bar-number comparison
  if (fBarNum < h->fBarNum) return -1;
  if (fBarNum > h->fBarNum) return  1;
  
  // finally, time comparison, earlier wins
  if (fTime < h->fTime) return -1;
  if (fTime > h->fTime) return  1;

  return 0;
}

//_____________________________________________________________________________
void THaTdcHit::Clear(Option_t *) {
  fPMT = 0;
  fBarNum = fSide = 0;
  fRawTime = 0;
  fTime = 0;
}

///////////////////////////////////////////////////////////////////////////////
ClassImp(THaTdcHit)
