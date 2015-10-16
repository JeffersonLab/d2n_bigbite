//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   THaMWDCHit                                                         //
//                                                                      // 
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_THaMWDCHit_H
#define ROOT_THaMWDCHit_H

#include "THaVDCHit.h"

class THaMWDCPlane;
class THaMWDCGroup;

class THaMWDCHit : public THaVDCHit {
 public:
  THaMWDCHit(THaMWDCPlane* plane = NULL, THaVDCWire* wire = NULL, Int_t rawtime = 0, 
	     Double_t time = 0.0, Double_t offset = 0.0, Double_t reftime = 0.0, Bool_t valid = true ); 

  enum ELR        { kL          = 1 << 0, 
		    kR          = 1 << 1, 
		    kUnDef      = 1 << 2, 
		    kNULL       = 1 << 3 };

  enum EHitStatus { kOwned      = 1 << 0, 
		    kLRFixed    = 1 << 1, 
		    kVerShower  = 1 << 2, 
		    kValShower  = 1 << 3, 
		    kDataValid  = 1 << 4, 
		    kCoordValid = 1 << 5 };

  THaMWDCPlane* GetPlane() const { return fPlane; }
  THaMWDCGroup* GetGroup() {return fGroup;}
 
  Double_t GetCoord( Double_t slope = 0.0, ELR LR = kNULL, Double_t t0 = 0.0  );

  void     SetTrackNumber( Int_t tracknum ) { fTrackNum = tracknum; }
  Int_t    GetTrackNumber() { return fTrackNum; }

  Bool_t   VerifiedByShower()       const { return            (fHitStatus&kVerShower ? true: false); }
  Bool_t   VerifiedByShowerDouble() const { return (Double_t) (fHitStatus&kVerShower ? true: false); }
  Bool_t   IsValidByShower()        const { return            (fHitStatus&kValShower ? true: false); }
  Bool_t   IsValidByShowerDouble()  const { return (Double_t) (fHitStatus&kValShower ? true: false); }

  void     SetVerifiedByShower( Bool_t isverified ) {fHitStatus = (fHitStatus& ~kVerShower)|(kVerShower*(isverified?1:0));}
  void     SetIsValidByShower ( Bool_t isvalid    ) {fHitStatus = (fHitStatus& ~kValShower)|(kValShower*(isvalid   ?1:0));}

  void     SetOwnedStatus( Bool_t owned ) {fHitStatus = (fHitStatus& ~kOwned)|(kOwned*(owned?1:0));}

  void     SetScintOffset(Double_t scint_off) { fScintOffset = scint_off; }

  void     SetLR( ELR LR ) {fLR = LR;}
  void     FixLR( ELR LR ) {fLR = LR; fHitStatus |= kLRFixed; }
  void     UnfixLR()       { fLR = kUnDef; fHitStatus &= ~kLRFixed; }
  ELR      GetLR()         { return fLR; }

  void     SetChi2Cont( Double_t chi2cont ) { fChi2Cont = chi2cont; }
  void     SetCoarseChi2Cont( Double_t chi2cont ) { fCoarseChi2Cont = chi2cont; }

  Double_t GetChi2Cont() { return fChi2Cont; }
  Double_t GetCoarseChi2Cont() { return fCoarseChi2Cont; }

  Bool_t   IsDataValid() { return (fHitStatus&kDataValid  ? true : false );}
  Bool_t   IsOwned()     { return (fHitStatus&kOwned      ? true : false );}
  Bool_t   IsLRFixed()   { return (fHitStatus&kLRFixed    ? true : false );}
  Bool_t   IsCoordValid(){ return (fHitStatus&kCoordValid ? true : false );}

 protected:
  THaMWDCGroup* fGroup;
  THaMWDCPlane* fPlane;

  Double_t fOffset;
  Double_t fRefTime;
  Double_t fCoord;

  Double_t fScintOffset;
  
  Int_t    fTrackNum;

  ELR      fLR;
  UInt_t   fHitStatus;

  Double_t fChi2Cont;
  Double_t fCoarseChi2Cont;

  ClassDef(THaMWDCHit, 0)
};
#endif
