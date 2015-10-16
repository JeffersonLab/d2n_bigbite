///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// THaAdcHit                                                                 //
//                                                                           //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "THaAdcHit.h"
#include "THaScintPMT.h"

using namespace std;


//_____________________________________________________________________________
THaAdcHit::THaAdcHit(THaScintPMT* pmt, Int_t rawampl):fPMT(pmt),
						      fRawAmpl(rawampl) 
{
  // Construct and correct the Adc hit for calibrations.
  if (pmt) {
    fBarNum = pmt->GetBarNum();
    fSide = pmt->GetSide();
  } else {
    fBarNum = 0;
    fSide = 0;
  }
  CorrectHit();
}

//_____________________________________________________________________________
void THaAdcHit::CorrectHit() {
  // Apply the associated PMT's pedestal and gain correction factors.

  THaScintPMT* pmt = GetPMT();
  if (pmt) {
    fAmplPedCor = fRawAmpl - pmt->GetPed();
    fAmpl = fAmplPedCor * pmt->GetGain();
  } else {
    fAmplPedCor = 0;
    fAmpl = 0;
  }
}

//_____________________________________________________________________________
Int_t THaAdcHit::Compare(const TObject *obj) const {
  // sort THaAdcHits according to side and bar number.
  //   obj should be a THaAdcHit, and we will assume as much
  //
  // Returns -1 if this < obj, 0 if this==obj, and 1 if this>obj
  const THaAdcHit *h = static_cast<const THaAdcHit*>(obj);

  // Side (Left-right) comparison
  if (fSide < h->fSide) return -1;
  if (fSide > h->fSide) return  1;

  // Bar-number comparison
  if (fBarNum < h->fBarNum) return -1;
  if (fBarNum > h->fBarNum) return  1;

  // finally, Amplitude comparison, highest Amplitude "wins" (first)
  if (fAmpl < h->fAmpl) return  1;
  if (fAmpl > h->fAmpl) return -1;
  
  return 0;
}

//_____________________________________________________________________________
void THaAdcHit::Clear(Option_t *s) {
  // clear the data inside THaAdcHit, so it does not have to be deleted each
  // time

  fPMT = 0;
  fRawAmpl = fAmplPedCor = fSide = 0;
  fBarNum = -1;
  fAmpl = 0;
}

///////////////////////////////////////////////////////////////////////////////

ClassImp(THaAdcHit)
