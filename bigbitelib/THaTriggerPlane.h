//////////////////////////////////////////////////////////////////////////
//                                                                           
// THaTriggerPlane                                                           
//                                                                           
// A trigger plane for BigBite, consistinf of a eplane and a deplane.        
//                                                                           
//////////////////////////////////////////////////////////////////////////
//	
// Author: Jin Huang <mailto:jinhuang@jlab.org>    July 2007	
//				base on analyzer for SRC experiment
//	Modify History:
//		
//
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_THaTriggerPlane
#define ROOT_THaTriggerPlane

#include "THaNonTrackingDetector.h"
#include "TClonesArray.h"

//------------------------------------------------------//
//
//	Debug Definitions
//	place this section below any other head files
//
//------------------------------------------------------//
#ifdef DEBUG_LEVEL
#   undef DEBUG_LEVEL
#endif

//	DEBUG_LEVEL;	
//	=0	or not define: no debug, full speed 
//	>=1	enable debug extra warning (suggested setting)
//	>=2	above + enable debug assert
//	>=3	above + enable debug extra info
//	>=4	above + massive info (in a for or while)
//  >=5 above + read database dump
#define DEBUG_LEVEL   2

#include    "DebugDef.h"
//------------------------------------------------------//

//pay more attention to which plane when mathcing hits or vice versa
//
//if MATCHING_HITS_E_PLANE_PRIORITY==1,
//then CoarseMatchingHits() will search de hits for those who match 
//e each hits, so de hits maybe be used for combine 
//THaTriggerPlaneHits for more than once.
//vice versa
#define MATCHING_HITS_E_PLANE_PRIORITY 0

// PROCESS_PARTIAL_HIT is switch to process and store partial hits on 
// trigger plan. "partial hits" is those hits on E or dE plane, which do not
// have correspondent hit on the other plane.
//
// partial hits may be useful in MWDC finding all tracks, because, for example,
// it is possible that a track failed to trigger a hit on one of two planes
// in trigger plane
#define PROCESS_PARTIAL_HIT 1

#if PROCESS_PARTIAL_HIT
#   define EXPORT_PARTIAL_HIT 1
#endif
//------------------------------------------------------//

class THaScintPlane;
class THaScintPMT;
class THaScintHit;
class THaScintBar;

class THaTriggerPlaneHit;

class THaTriggerPlane : public THaNonTrackingDetector {

public:
    virtual ~THaTriggerPlane();
    //THaTriggerPlane( const char* name, const char* description,
    //			  THaApparatus* a = NULL);
    THaTriggerPlane( const char* name, const char* description, THaApparatus* a = NULL  );

    Int_t          Decode( const THaEvData& );
    EStatus        Init( const TDatime& run_time);
    THaScintPlane* GetEPlane() const { return fEplane; }
    THaScintPlane* GetdEPlane() const { return fdEplane; }

    Double_t        GetHitAcceptanceDx() const { return fHitAcceptanceDx; }
    Double_t        GetHitAcceptanceDy() const { return fHitAcceptanceDy; }
    Double_t        GetHitAcceptanceDz() const { return fHitAcceptanceDz; }
    Double_t        GetHitAcceptanceMinDt() const { return fHitAcceptanceMinDt; }
    Double_t        GetHitAcceptanceMaxDt() const { return fHitAcceptanceMaxDt; }

    Int_t           GetNHits()     const { return fHits->GetLast()+1; }
    TClonesArray*   GetHits() const {return fHits;}
    THaTriggerPlaneHit*    GetHit(Int_t i) const {return (THaTriggerPlaneHit*)fHits->At(i);}

    Int_t           GetNPartialHits()     const { return fPartialHits->GetLast()+1; }
    TClonesArray*   GetPartialHits() const {return fPartialHits;}
    THaTriggerPlaneHit*    GetPartialHit(Int_t i) const {return (THaTriggerPlaneHit*)fPartialHits->At(i);}

    const TClonesArray* GetTrackHits() const { return fTrackProj; }
    Int_t GetNTracks() const { return fTrackProj->GetLast()+1; }

    Double_t        GetMatchRatioE() const {return fMatchRatioE;}
    Double_t        GetMatchRatiodE() const {return fMatchRatiodE;}
    Double_t        GetMatchRatioTrack() const {return fMatchRatioTrack;}

    void           SetApparatus( THaApparatus* );

    virtual Int_t      CoarseProcess( TClonesArray& tracks );
    virtual Int_t      FineProcess( TClonesArray& tracks );

    virtual Int_t CoarseMatchingHits(); //match hits of de and E plane and save them to fHits
    //match hits of de and E plane with reference to tracks, modify fHits
    //process golden track if possible
    virtual Int_t FineMatchingHits(TClonesArray& tracks); 

protected:
    TClonesArray* fHits;     // Hits matched from dE and E plane
    TClonesArray* fPartialHits;// Hits other than those matched
    TClonesArray*  fTrackProj;  // projection of track onto scintillator plane
    // and estimated match to TOF paddle
    Double_t fMatchRatioE, fMatchRatiodE, fMatchRatioTrack; //# of matched THaTriggerPlaneHit / # of total THaScintHit on THaScintPlane 

    // Subdetectors
    THaScintPlane* fEplane;     //E plane subdetector
    THaScintPlane* fdEplane;    //dE plane subdetector

    //when x y z distance between 2 hits in E and dE plane is smaller
    //than the number below, they will be combined as hit in Triggerplane
    Double_t fHitAcceptanceDx;
    Double_t fHitAcceptanceDy;
    Double_t fHitAcceptanceDz;

    //accepatble time difference between 2 hits in E and dE plane is smaller
    //time difference=TOF(dE)-TOF(E)
    Double_t fHitAcceptanceMinDt;
    Double_t fHitAcceptanceMaxDt;

    //when x y distance between hit and track projection is smaller
    //than the number below, the hit will be matched to the track
    //(the index of hit will be saved to fTrackProj.THaTrackProj.fChannel)
    //if there are more than 1 hit in this region, then the nearest one wins
    Double_t fTrackAcceptanceDx;
    Double_t fTrackAcceptanceDy;

    THaTriggerPlane& operator=( const THaTriggerPlane& ) {return *this; }

    virtual Int_t  DefineVariables( EMode mode = kDefine );
    virtual Int_t  ReadDatabase( const TDatime& date );
    //function for jump through lines starting with #
    char* ReadNumberSignStartComment( FILE* fp, char *buf, const int len );

private:
    void           Setup( const char* name,  const char* desc, 
        const char* enam, const char* denam,
        bool mode );


    ClassDef(THaTriggerPlane,1)//Trigger plane class, consist of dE and E plane
};

#endif
