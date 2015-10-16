//////////////////////////////////////////////////////////////////////////
//                                                                           
// THaMWDC                                                                   
//                                                                           
//////////////////////////////////////////////////////////////////////////
//	
//	Author : Copy from AGen Lib
//	Modify History:
//		 Jin Huang <mailto:jinhuang@jlab.org>    July 2007
//
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_THaMWDC
#define ROOT_THaMWDC


//-----------------------------------------------------------------------//
// Current version force shuts down tracking and connection to THaBBe
#define DO_THAMWDC_TRACKING 0

#if DO_THAMWDC_TRACKING
//#define DO_TRACKING_BIGBITE		//change to future tracking algorithm if necessary
#define DO_TRACKING_BBE
#endif
//-----------------------------------------------------------------------//

#include "THaTrackingDetector.h"
#include "TMatrixD.h"
#include "TClonesArray.h"
#include <vector>


#define STRING_LENGTH 200
#define MAX_MATRIX 500000
#define MAX_MODULE_ERRORS 50
#define MIN_DETERMINANT 1e-5
#define MAX_PLANES 20
#define TOPNTRACKS 10
#define TOPNCOARSETRACKS 50

#define SPEED_OF_LIGHT 0.3

// FIXME:  These go in database
#define SCINT_OFFSET 54.5
#define SCINT_TIME_WIDTH 20.0

using namespace std;

class THaMWDCPlane;
class THaMWDCGroup;
class THaMWDCHit;
class THaTrack;
class TClonesArray;
class THaBenchmark;

class THaMWDC : public THaTrackingDetector {

public:
    THaMWDC( const char* name, const char* description = "", THaApparatus* a = NULL );
    virtual ~THaMWDC();

    virtual Int_t   Decode( const THaEvData& );
    virtual EStatus Init( const TDatime& date );
    virtual Int_t   CoarseTrack( TClonesArray& tracks );
    virtual Int_t   FineTrack( TClonesArray& tracks );
    virtual Int_t   DefineVariables( EMode mode = kDefine );
    virtual Int_t   FindVertices(TClonesArray&) {return 0;}
    virtual void    Print(const Option_t* opt) const;
    virtual void    SetDebug( Int_t level );
    void    EnableBenchmarks( Bool_t b = kTRUE );

    // Data retrieval

    // Planes
    UInt_t                 GetNPlanes()      const { return fPlanes->GetEntriesFast(); }
    TClonesArray*          GetPlanes()       const { return fPlanes; }
    THaMWDCPlane*          GetPlane(Int_t i) const { return (THaMWDCPlane*)fPlanes->At(i);}

    // Groups
    UInt_t                 GetNGroups()      const { return fGroups.size();}
    vector<THaMWDCGroup *> GetGroups()             { return fGroups;}
    THaMWDCGroup*          GetGroup(Int_t i) const { return fGroups[i]; }


    // Enumerations

    // Status bits
    enum {
        kStageMask     = BIT(14) | BIT(15),  // Track processing stage bits
        kInvalid       = 0x0000,  // Not processed
        kCoarse        = BIT(14), // Coarse track
        kFine          = BIT(15), // Fine track
        kReassigned    = BIT(16), // Track is a new track in Fine stage
        kMultiTrack    = BIT(17), // Track was generated in the multitrack analysis
        kBadTrack      = BIT(18)  // Track prematurely exits the spectrometer or similar
    };

    enum {
        kOnlyFastest    = BIT(14), // Only use fastest hit for each wire (highest TDC)
        kTDCbits        = BIT(15) | BIT(16),  // Bits for TDC mode bits
        kHardTDCcut     = BIT(15), // Use hard TDC cuts (fMinTime, fMaxTime)
        kSoftTDCcut     = BIT(16), // Use soft TDC cut (reasonable estimated drifts)
        kIgnoreNegDrift = BIT(17), // Completely ignore negative drift times

        kCoarseOnly     = BIT(23) // Do only coarse tracking
    };

    enum {
        kUseShowerCuts    = 1 << 0,
        kValidShowerData  = 1 << 1,
        kValidClusterData = 1 << 2
    };

    enum {
        kRejected = 1 << 0,
        kSoftMax  = 1 << 1,
        kHardMax  = 1 << 2
    };

    enum ETrackingMode { kCoarseTracking, kFineTracking };

protected:

    TClonesArray*           fPlanes; 
    vector <THaMWDCGroup *> fGroups;

    TMatrixD* fConstructMatrix[MAX_MATRIX];
    Int_t     fValidBits[MAX_MATRIX];
    Int_t     fPlaneNumbersForBits[MAX_MATRIX][MAX_PLANES+1];
    UInt_t    fReconstructBits;

    Int_t     fNtracks;
    Int_t     fNmatrices;
    UInt_t    fNPlanes;

    UInt_t    fShowerStatus;
    TVector3  fShowerOrigin;


    // Tracking Accounting 
    Int_t    fTotalConsideredEvents;
    Int_t    fTotalTrackingEvents;
    Int_t    fRejectedTrackingEvents;
    Int_t    fHardMaxTrackingEvents;
    Int_t    fSoftMaxTrackingEvents;

#ifdef DO_TRACKING_BBE
    // Info from database
    UInt_t   fMaxGroups;               // Soft maximum to stop at when finding groups
    UInt_t   fHardMaxGroups;           // Hard maximum to stop at when finding groups
    UInt_t   fMaxCallThreshold;        // Maximum number of estimated calls before tracking is abandoned
    UInt_t   fMinimumPlanes;
#endif

    Double_t fTargetXhigh;
    Double_t fTargetXlow;
    Double_t fTargetYhigh;
    Double_t fTargetYlow;
    Double_t fTargetZ;

    Double_t fTargetXExt;
    Double_t fTargetYExt;
    Double_t fShowerXExt;
    Double_t fShowerYExt;
    Double_t fTargetWindowXOffset;

    // Benchmark
    THaBenchmark* fBench;  // Performance statistics timers
    Bool_t fDoBench;       // if true, collect timing statistics

    // Tracking info to be written out
    UInt_t   fTooBusy;
    Double_t fCoarseProcTime;  
    Double_t fFineProcTime;
    Int_t    fEstNGroups;
    Int_t    fEstNCalls;
    Int_t    fNGroups;
    Int_t    fNCalls;

    // Inherited
    Int_t  ReadDatabase( const TDatime& date );
    void   Clear( Option_t* opt="" );
    Int_t  End(THaRunBase* run);

    // Group Finding/Tracking functions
    Int_t  ConstructTracks( TClonesArray * tracks = NULL, ETrackingMode mode = kCoarseTracking );

#ifdef DO_TRACKING_BBE
    void   FindGroups();
    void   AllCombos( Int_t planebits, vector<THaMWDCGroup*> &groups, vector<THaMWDCHit*> hits, Int_t depth );


    // Shower functions
    Bool_t HitValidByShower( THaMWDCHit *hit );
    void   GetShowerData();
#endif //#ifdef DO_TRACKING_BBE

    Bool_t UseShowerCuts()                  { return (fShowerStatus & kUseShowerCuts)    ? true : false; }
    Bool_t HaveValidShowerData()            { return (fShowerStatus & kValidShowerData)  ? true : false; }
    Bool_t HaveValidClusterData()           { return (fShowerStatus & kValidClusterData) ? true : false; }
    void   SetUseShowerCuts( Bool_t b )     { if(b) fShowerStatus |=  kUseShowerCuts; 
    else  fShowerStatus &= ~kUseShowerCuts; }
    void   SetValidShowerData( Bool_t b )   { if(b) fShowerStatus |=  kValidShowerData; 
    else  fShowerStatus &= ~kValidShowerData; }
    void   SetValidClusterData( Bool_t b )  { if(b) fShowerStatus |=  kValidClusterData; 
    else  fShowerStatus &= ~kValidClusterData;}
    // Auxillary functions
    void     LoadTargetData();
    void     PrintCutInfo();

#ifdef DO_TRACKING_BBE
    void     GenerateConstructMatrices();
    void     GenerateConstructMatrixForBits( Int_t trialbits );
    Bool_t   ValidPlaneBits( UInt_t bits );

    void     HeapSortGroupsByChi2(vector <THaMWDCGroup*> &v);
    void     HeapSortSiftDown( vector <THaMWDCGroup*> &v, UInt_t n );
    void     HeapSortBuildHeap( vector <THaMWDCGroup*> &v );

    Double_t GetScintillatorTime();
    Double_t GetScintillatorZ();
    Double_t GetScintTimeCorrection( THaMWDCPlane* );
#endif //#ifdef DO_TRACKING_BBE
    void     SetScintCorrection();

    ClassDef(THaMWDC,0)             // AGen MWDC class with old version of tracking code
};

////////////////////////////////////////////////////////////////////////////////

#endif
