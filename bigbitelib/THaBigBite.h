//////////////////////////////////////////////////////////////////////////
//
// THaBigBite
//
// Base class for general purpose bigbite detector
//
//////////////////////////////////////////////////////////////////////////
//    
// Author: Jin Huang <mailto:jinhuang@jlab.org>    July 2007
//        Base on work of Olivier Gayou, AGen Lib, and Ole Hansen
//    Modify History:
//      Jan 2008    Jin Huang <mailto:jinhuang@jlab.org> 
//          add support to new MWDC class
//      Jan 2008    Jin Huang <mailto:jinhuang@jlab.org> 
//          add new child class THaOptics
//      Mar 2009    Jin Huang <mailto:jinhuang@jlab.org>
//          add GetTriggerTimingOffset(){return fTriggerTimingOffset;} portal
//
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_THaBigBite
#define ROOT_THaBigBite

#include "THaSpectrometer.h"
#include "THaEvData.h"
//#include "THaRunParameters.h"
#include "THaDetector.h"
#include "VarDef.h"
#include "TVector3.h"
#include "TRotation.h"
#include "TMatrixD.h"

class THaOptics;
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
//  >=5 Decode dump  
#define DEBUG_LEVEL   2

#include    "DebugDef.h"
//------------------------------------------------------//

// whether use ThaMWDC class developed by AGen
// USE_AGEN_MWDC=1: USE AGen Version of MWDC class, SWITCH_RECONSTRUCT_ORDER=1
// USE_AGEN_MWDC=0: USE TreeSearch Version of MWDC class, SWITCH_RECONSTRUCT_ORDER=0
#define USE_AGEN_MWDC 0

// Environmental Variable SWITCH_RECONSTRUCT_ORDER controls 
//   whether or not to change order of steps in Reconstruct()
// Warning: this might make this code non-compatible for future analyzer,
//   check campability before upgrading analyzer!!
#if USE_AGEN_MWDC
#   define SWITCH_RECONSTRUCT_ORDER 1
#else
#   define SWITCH_RECONSTRUCT_ORDER 0
#endif

// Since function Reconstruct() Track() CoarseReconstruct() CoarseTrack() ListInit()
// are overloaded from THaSpectrometer. So one need to test codes compability
// for each analyzer version that may be used
//
// following is a list of analyzer version number, which has been confirmed to be
// compatible. This information will be posted onto replay screen (if warning is enabled)
// also add this list to constructor manually
#define SWITCH_RECONSTRUCT_ORDER_COMPATIBLE_ANALYZER_VERSION "1.4.6&7"



//#define MAX_GRID 50
//#define N_VERTEX_COEFFS   5
//#define N_MOMENTUM_COEFFS 5
//#define _BBOPTICS_BUFFER_LEN 200


class THaBigBite : public THaSpectrometer {


public:
	//////////////////////////////////////////////////////////////////
	// Constructor desctructor
	//////////////////////////////////////////////////////////////////
    virtual ~THaBigBite();
    THaBigBite( const char* name, const char* description );//constructor
	
	//////////////////////////////////////////////////////////////////
	//Trigger Correction
	//////////////////////////////////////////////////////////////////
public:
	virtual Double_t GetTriggerTimingOffset(){return fTriggerTimingOffset;}
	//returning time off sets between a physical event - trigger edge
	//So, Real_TDC = TDC(common-stop) - BB.GetTriggerTimingOffset()
	//each experiment should write a derived class from THaBigBite to implement Offset calculation according to their trigger setup
	
protected:
	Double_t fTriggerTimingOffset;//Real_TDC = TDC(common-stop) - fTriggerTimingOffset. It's up to derived class to clear and update according to its trigger setup.
			
	//////////////////////////////////////////////////////////////////
	// Optics Module
	//////////////////////////////////////////////////////////////////
public:

    virtual Int_t   FindVertices(TClonesArray& tracks);
    virtual Int_t   TrackCalc();

    virtual Bool_t   MountOptics(THaOptics *);
protected:
    TRef            fOptics;  //pointer to optics calculation class
public:
    inline THaOptics *  GetOptics() const
    {return (THaOptics *)(fOptics.GetObject());}

	//////////////////////////////////////////////////////////////////
	// USE_AGEN_MWDC stuff
	//////////////////////////////////////////////////////////////////
#if USE_AGEN_MWDC
public:
    TVector3        GetTargetCenterInDet();
    TVector3        GetTargetStartInDet();
    TVector3        GetTargetEndInDet();

    Double_t        GetShowerXExtension() {return fShowerXExtension;}
    Double_t        GetShowerYExtension() {return fShowerYExtension;}
    Double_t        GetTargetXExtension() {return fTargetXExtension;}
    Double_t        GetTargetYExtension() {return fTargetYExtension;}
    Double_t        GetTargetWindowXOffset() { return fTargetWindowXOffset;}

    TRotation       GetDetectorToTarg() { return fDetToTarg; }
    TVector3        GetDetectorOffset() { return fDetOffset; }
#endif

#if DEBUG_LEVEL>1 
    virtual Int_t   DebugPort(void *p);
#endif

#if DEBUG_LEVEL>=5 
    virtual Int_t      Decode( const THaEvData& );
#endif

public:
    virtual THaAnalysisObject::EStatus Init(const TDatime& run_time);
	virtual Int_t DefineVariables( EMode mode );
protected:
    virtual Int_t ReadDatabase( const TDatime& date );
    //Int_t DefineVariables( EMode mode );

	//////////////////////////////////////////////////////////////////
	// SWITCH_RECONSTRUCT_ORDER stuff
	//////////////////////////////////////////////////////////////////
#if SWITCH_RECONSTRUCT_ORDER
    // now function Reconstruct() Track() CoarseReconstruct() CoarseTrack() just play a role of 
    // gate way to steps of reconstruction. Actually action of each function is defined in stepN()
    //
    // ListInit should be direct copy from analyzer THaSpectrometer source code

    virtual Int_t    CoarseTrack();
    virtual Int_t    CoarseReconstruct();
    virtual Int_t    Track();
    //virtual Int_t    FineReconstruct();
    virtual Int_t    Reconstruct();

    virtual Int_t   Step1(void);
    virtual Int_t   Step2(void);
    virtual Int_t   Step3(void);
    virtual Int_t   Step4(void);

    //enum EExtraStagesDone {
    //    //kCoarseTrack = BIT(0),
    //    //kCoarseRecon = BIT(1),
    //    //kTracking    = BIT(2),
    //    //kReconstruct = BIT(3)
    //    kFineReconstruct= BIT(7)
    //};

    Bool_t    fListInit;      //Detector lists initialized
    void    ListInit();     //Initializes lists of specialized detectors

#endif//#if SWITCH_RECONSTRUCT_ORDER

	//////////////////////////////////////////////////////////////////
	// USE_AGEN_MWDC stuff
	//////////////////////////////////////////////////////////////////
#if USE_AGEN_MWDC
    //Frame transform rotation matricies and offset
    //  "Target Frame" is the frame with z-axis pointing down BigBite  
    Double_t        fdet_rot_x;       //rotation about x axis
    Double_t        fdet_rot_y;       //rotation about y axis
    Double_t        fdet_rot_z;      //rotation about z axis
    TRotation       fTargToDet;     //Target to Detctor Frame Rotation
    TRotation       fDetToTarg;     //Detector Target Frame Rotation
    TVector3        fDetOffset;     //Detector Frame Origin in Target Frame

    TVector3        fBendPlaneCenter;
    Double_t        fBendTheta;
    Double_t        fTargetWindowXOffset;

    TRotation       fLabToTarg;     //Lab to Target Rotation
    TRotation       fTargToLab;     //Target to Lab Rotation
    TVector3        fTargOffset;    //Target Frame origin specified in Lab Frame

    Double_t        fTargetLength;


    Double_t        fShowerXExtension;
    Double_t        fShowerYExtension;
    Double_t        fTargetXExtension;
    Double_t        fTargetYExtension;

    TVector3        fMagnetCenter;   //point at center of BigBite Magnet
#endif



    ///////////////////////////////////////////////////////////////////

private:
    ClassDef(THaBigBite,0) //ABC for Hall A BigBite
};

#endif

