#ifndef ROOT_THaMWDCPlane
#define ROOT_THaMWDCPlane



///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// THaMWDCPlane                                                              //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "THaSubDetector.h"
#include "TClonesArray.h"
#include "THaVar.h"
#include "THaVarList.h"
#include "TMath.h"
#include <iostream>
using namespace std;

class THaEvData;
class THaVDCWire;
class THaMWDCHit;
class THaVDCTimeToDistConv;

class THaMWDCPlane : public THaSubDetector {

public:

  THaMWDCPlane( const char* name="", const char* description="",
	       THaDetectorBase* parent = NULL );
  virtual ~THaMWDCPlane();

  virtual Int_t   Decode( const THaEvData& ); // Raw data -> hits
  virtual EStatus Init( const TDatime& date );
 
  enum EPlaneType { kXchamber = 0, 
		    kVchamber = 1, 
		    kUchamber = 2, 
		    kYchamber = 3, 
                    kOchamber = 4 } ;


  //Get and Set functions
  Int_t          GetNWires()    const { return fWires->GetEntries(); }
  TClonesArray*  GetWires()     const { return fWires; }
  THaVDCWire*    GetWire(Int_t i) const { return (THaVDCWire*)fWires->At(i);}

  UInt_t          GetNHits()     const { return (UInt_t) fHits->GetEntries(); }
  TClonesArray*  GetHits()      const { return fHits; }
  THaMWDCHit*    GetHit(Int_t i) const  
    {  return (THaMWDCHit*)fHits->At(i); }

  Int_t    GetNWiresHit() const  { return fNWiresHit; } 

  Int_t          GetNRefCh()    const { return fRefCh->GetEntries(); }
  TClonesArray*  GetRefCh()     const { return fRefCh; }
  THaVDCWire*    GetRefCh(Int_t i) const
  { return (THaVDCWire*)fRefCh->At(i);}

  Int_t          GetNRefHits()     const { return fRefHits->GetEntries(); }
  TClonesArray*  GetRefHits()      const { return fRefHits; }
  THaMWDCHit*     GetRefHit(Int_t i) const
  { return (THaMWDCHit*)fRefHits->At(i); }

  Int_t    GetNRefHit() const  { return fNRefHits; } 

  Double_t GetZ()        const   { return fZ; }
  Double_t GetXWidth()   const   { return fXWidth; }
  Double_t GetYWidth()   const   { return fYWidth; }
  Double_t GetWBeg()     const   { return fWBeg; }
  Double_t GetWSpac()    const   { return fWSpac; }
  Double_t GetCellHeight() const { return fCellHeight; }
  Double_t GetWAngle()   const   { return fWAngle; }
  Double_t GetTDCRes()   const   { return fTDCRes; }
  Double_t GetPosRes()   const   { return fPosRes; }
  Double_t GetDriftVel() const   { return fDriftVel; }
  Double_t GetActAreaMaxX() const  { return fXMax ; }
  Double_t GetActAreaMaxY() const  { return fYMax ; }
  Double_t GetActAreaMinX() const  { return fXMin ; }
  Double_t GetActAreaMinY() const  { return fYMin ; }
  Double_t GetActAreaMaxTH() const  { return fTHMax ; }
  Double_t GetActAreaMaxPH() const  { return fPHMax ; }
  Double_t GetActAreaMinTH() const  { return fTHMin ; }
  Double_t GetActAreaMinPH() const  { return fPHMin ; }
  Double_t GetXCorr() const {return fXCorr; }
  Double_t GetYCorr() const {return fYCorr; }
  Double_t GetThetaCorr() const {return fThetaCorr; }
  Double_t GetPhiCorr() const {return fPhiCorr; }
  Bool_t IsInReconstruction() const {return fUseInReconstruct;}


  EPlaneType GetPlaneType() const { return fPlaneType ; }
  Bool_t IsXPlane() const { return (fPlaneType==kXchamber); } 
  Bool_t IsUPlane() const { return (fPlaneType==kUchamber); } 
  Bool_t IsVPlane() const { return (fPlaneType==kVchamber); } 
  Bool_t IsYPlane() const { return (fPlaneType==kYchamber); } 
  Int_t AreRefChOkay() const { return ( fRefOkay ? -1 : 0 ); } 

  void SetZ(Double_t z)            { fZ = z; }
  void SetXWidth()       { fXWidth=fXMax-fXMin; }
  void SetYWidth()       { fYWidth=fYMax-fYMin; }
  void SetWBeg(Double_t b)         { fWBeg = b; }
  void SetWSpac(Double_t s)        { fWSpac = s; }
  void SetWAngle(Double_t a);      
  void SetWAngleDeg(Double_t a);   
  void SetTDCRes(Double_t r)       { fTDCRes = r; }
  void SetPosRes(Double_t r)       { fPosRes = r; }
  void SetDriftVel(Double_t v)     { fDriftVel = v; }
  void SetActAreaMaxX(Double_t x)    { fXMax = x; }
  void SetActAreaMaxY(Double_t y)    { fYMax = y; }
  void SetActAreaMinX(Double_t x)    { fXMin = x; }
  void SetActAreaMinY(Double_t y)    { fYMin = y; }
  void SetActAreaMaxTH(Double_t t)   { fTHMax = t; }
  void SetActAreaMaxPH(Double_t p)   { fPHMax = p; }
  void SetActAreaMinTH(Double_t t)   { fTHMin = t; }
  void SetActAreaMinPH(Double_t p)   { fPHMin = p; }
  void SetXCorr( Double_t x )        { fXCorr = x; }
  void SetYCorr( Double_t y )        { fYCorr = y; }
  void SetThetaDegCorr( Double_t theta )        { fThetaCorr = ( TMath::Pi() * theta )/180.0; }
  void SetPhiDegCorr( Double_t phi )        { fPhiCorr = (TMath::Pi()*phi )/180.0; }

  void SetInReconstruct( Bool_t i ) { fUseInReconstruct = i; }

  void SetHits( TClonesArray *hits ) {/* fHits->Clear(); delete fHits;*/ fHits = hits; }
  THaVDCTimeToDistConv* GetTTDConv() { return fTTDConv; }
  Double_t ConvertDriftTimeToDistance( Double_t, Double_t = 0.0 );

  Double_t XYtoPlaneCoord( Double_t x, Double_t y);
  Double_t GetXYCorr();

  void LoadMCHitAt( Double_t x, Double_t y, Double_t xp, Double_t yp );
  void PrintDriftCuts();

protected:

  Int_t GetParameter( FILE* file, const TString tag, Double_t* value  );
  Int_t GetOptionalParameter( FILE* file, const TString tag, Double_t* value  );
  Int_t GetParameter( FILE* file, const TString tag, char* string  );
  //Use TClonesArray::GetLast()+1 to get the number of wires, hits, & clusters 
  TClonesArray*  fWires;     // Wires
  TClonesArray*  fHits;      // Fired wires 
  Int_t fNWiresHit;  // Number of wires that were hit
  Int_t fNWires; // Number of wires in that plane

  THaVar *fEventType;
  THaVar *fBit3;

  // same for reference channels 
  TClonesArray*  fRefCh;        // reference channels for pipeline tdc
  TClonesArray*  fRefHits;      // hits on reference channels (should be one for each trigger on each line)
  Int_t fNRefHits;  // Number of refch that were hit
  Int_t fNRefCh; // number of reference channels 
  Bool_t fRefOkay; 

  // The following parameters are read from database.
 
  Double_t fMinTime, fMaxTime;  // Min and Max limits of TDC times for clusters
  Int_t fFlags;              // Analysis control flags

  Double_t fZ;            // Z coordinate of planes 
  Double_t fXWidth;       // Width of active area in dispersive direction(m)
  Double_t fYWidth;       // Width of active area in out-of-plane directoin(m)
  Double_t fPosRes;       // Estimate for the position resolution along xp (m)

  Double_t fXMax, fXMin, fYMax, fYMin ;  // active area of the plane
  Double_t fTHMax, fTHMin, fPHMax, fPHMin ; // reasonable range of intersection
                                            // angles, theta between x and z in rad
                                            // phi the angle between y and z in rad


  Double_t fWBeg;         // Position of 1-st wire (m)
  Double_t fWSpac;        // Wires spacing and direction (m)
  Double_t fCellHeight;   // Height of a cell in a plane (m)
  Double_t fWAngle;       // Angle between dispersive direction and direction of 
  EPlaneType fPlaneType;  // x,u,v,y or other type of plane
                          // decreasing wire number (rad)
  Double_t fTDCRes;       // TDC Resolution ( s / channel)
  Double_t fDriftVel;     // Drift velocity in the wire plane (m/s)
  Double_t fDriftC1;      // Parameter to use for TanH TTD conversion
  Double_t fDriftC2;      // Parameter to use for TanH TTD conversion
  Double_t fDriftC3;      // Parameter to use for TanH TTD conversion

  Double_t fXCorr;
  Double_t fYCorr;
  Double_t fThetaCorr;
  Double_t fPhiCorr;

  Bool_t fUseInReconstruct;

  // Lookup table parameters

  THaVDCTimeToDistConv* fTTDConv;  // Time-to-distance converter for this plane's wires

  THaDetectorBase* fMWDC;  // MWDC detector to which this plane belongs

  Int_t fTotalHits;
  Int_t fRejectedHits;

  virtual void  Clear( Option_t* opt="" );

  virtual Int_t ReadDatabase( const TDatime& date );
  virtual Int_t DefineVariables( EMode mode = kDefine );

  // for debugging: hits on each wire

  Double_t* fRawT; // [fNWires]
  Double_t* fCorT; // [fNWires]
  Int_t* fTcounter; // [fNWires]
  Double_t* fRefT; // [fNRefCh]


  ClassDef(THaMWDCPlane,0)             // MWDCPlane class
};

///////////////////////////////////////////////////////////////////////////////

#endif
