#ifndef ROOT_THaMWDCGroup
#define ROOT_THaMWDCGroup

#include "TObject.h"
#include "THaMWDCPlane.h"
#include "THaTrack.h"
#include <vector>
#include "TMatrixD.h"

class THaMWDCHit;

using namespace std;

#define MAX_PLANES 20
#define N_TRACK_ELEMENTS 4

// Maximum number of planes that have the same track intercept
// for a track to be considered unique
#define MAX_SIMILAR_PLANES_FOR_UNIQUE_TRACK 6

//Max chi^2/degrees of freedom
#define MAX_CHI2NDOF 50

//Max individual chi^2 contribution
#define MAX_CHI2CONT 5
 
class THaMWDCGroup : public TObject {
 public: 
  // Constructors
  THaMWDCGroup( vector<THaMWDCHit*>   H,           UInt_t planebits, TMatrixD *matrix );
  THaMWDCGroup( THaMWDCHit          **H, Int_t  n, UInt_t planebits, TMatrixD *matrix );
  THaMWDCGroup( THaMWDCGroup& G );

  // Destructor
  virtual ~THaMWDCGroup();

  // Enumerations

  // Status bits
  enum EGroupStatus { kFresh      = 1 << 0, 
		      kCoarse     = 1 << 1, 
		      kFine       = 1 << 2, 
		      kValidTrack = 1 << 3, 
		      kOK         = 1 << 4, 
		      kL1         = 1 << 5, 
		      kL2         = 1 << 6 };

  // Tracking modes
  enum ETrackingMode { kCoarseTracking, kFineTracking, kTrialFineTracking };

  // Basic data retrival

  // Track information
  THaTrack GetTrack()    { return fTrack;}
  Double_t GetX()        { return fGroupStatus & (kCoarse|kFine) ? fTrack.GetX()     : 1e9;}
  Double_t GetY()        { return fGroupStatus & (kCoarse|kFine) ? fTrack.GetY()     : 1e9;}
  Double_t GetTheta()    { return fGroupStatus & (kCoarse|kFine) ? fTrack.GetTheta() : 1e9;}
  Double_t GetPhi()      { return fGroupStatus & (kCoarse|kFine) ? fTrack.GetPhi()   : 1e9;}
  Double_t GetChi2()     { return fGroupStatus & (kCoarse|kFine) ? fTrack.GetChi2()  : 1e9;}
  Double_t GetNDoF()     { return fGroupStatus & (kCoarse|kFine) ? fTrack.GetNDoF()  : 1e9;}
  Double_t GetChi2NDoF() { return fGroupStatus & (kCoarse|kFine) ? fChi2NDoF         : 1e9;}

  // Group information
  UInt_t               GetNHits()      { return fHits.size();}
  vector<THaMWDCHit*>  GetHits()       { return fHits;       }
  THaMWDCHit*          GetHit(Int_t i) { return fHits[i];    }

  UInt_t GetPlaneBits() { return fPlaneBits;}
  UInt_t GetLRbits()    { return fLRbits;   }

  TMatrixD* GetConstructMatrix()     { return fConstructMatrix;  }
  Double_t  GetChi2Cont(int i)       { return fChi2Cont[i];      }
  Double_t  GetCoarseChi2Cont(int i) { return fCoarseChi2Cont[i];}

  // Group status
  UInt_t GetStatusBits(){ return fGroupStatus;}

  Bool_t IsOK()         { return (fGroupStatus & kOK         ? true : false); }
  Bool_t IsL1Group()    { return (fGroupStatus & kL1         ? true : false); }
  Bool_t IsL2Group()    { return (fGroupStatus & kL2         ? true : false); }
  Bool_t IsFresh()      { return (fGroupStatus & kFresh      ? true : false); }
  Bool_t IsCoarse()     { return (fGroupStatus & kCoarse     ? true : false); }
  Bool_t IsFine()       { return (fGroupStatus & kFine       ? true : false); }
  Bool_t IsValidTrack() { return (fGroupStatus & kValidTrack ? true : false); }

  // External set functions for THaMWDC
  void SetTrackNumber(Int_t track_number);
  void SetChi2Cont();
  void SetL1Group(Bool_t b) {if(b) fGroupStatus |= kL1; else fGroupStatus &= ~kL1; } 
  void SetL2Group(Bool_t b) {if(b) fGroupStatus |= kL2; else fGroupStatus &= ~kL2; } 

  // Tracking functions to be called by THaMWDC

  Int_t  DoCoarseTracking();
  Int_t  DoFineTracking( Double_t toffset = 0.0 );
  Bool_t IsSimilarTrack( THaTrack track );
  
 protected:
  
  vector<THaMWDCHit*> fHits;

  UInt_t fLRbits;
  UInt_t fPlaneBits;
  UInt_t fGroupStatus;

  TMatrixD *fConstructMatrix;

  THaTrack fTrack;
  Double_t fChi2Cont[MAX_PLANES];
  Double_t fCoarseChi2Cont[MAX_PLANES];
  Double_t fChi2NDoF;

  Int_t fPlaneIndex[MAX_PLANES];

  // Tracking functions
  THaTrack FindTrack( ETrackingMode mode, Double_t chi2cont[], Double_t toffset = 0.0);
  Int_t    DoLRDiff();
  Int_t    DoLRDiffForType( THaMWDCPlane::EPlaneType type );

  // Tracking calculations
  void     TransformCoordToTrack( Double_t coord[], Double_t track[] );
  Double_t CalcChiSquared( ETrackingMode mode, Double_t coords[], THaTrack track, Double_t chi2cont[]  );

  // Track Check functions
  void     DetermineValidTrack();
  Bool_t   FitsChamber();
  void     CheckHitData();

  // Auxillary functions
  void                Reset();
  vector<THaMWDCHit*> GetHitsOfType( THaMWDCPlane::EPlaneType type );
  void                GeneratePlaneIndex();
  Int_t               GetPlaneIndexForHit(Int_t hit) { return fPlaneIndex[hit]; }

  ClassDef(THaMWDCGroup, 0)
};

#endif
