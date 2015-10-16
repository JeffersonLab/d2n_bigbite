//////////////////////////////////////////////////////////////////////////
//
// THaBigBite
//
// Base class for general purpose bigbite detector (spectrometer)
//
//////////////////////////////////////////////////////////////////////////
//    
// Author: Jin Huang <mailto:jinhuang@jlab.org>   July 2007
//        Base on work of Olivier Gayou, AGen Lib, and Ole Hansen
//    Modify History:
//      Jan 2008    Jin Huang <mailto:jinhuang@jlab.org> 
//          add support to new MWDC class
//      Jan 2008    Jin Huang <mailto:jinhuang@jlab.org> 
//          add new child class THaOptics
//
//////////////////////////////////////////////////////////////////////////
/*
This is a base class for the Hall A BigBite spectrometer (BB).
Ready for AddDetector() of THaTriggerPlane, THaMWDC, TreeSearch::MWDC, THaBBTotalShower



SPECIAL CONFIGURATIONS of BB (e.g. more detectors, different 
detectors) can be supported in on e of three ways:

1. Use the AddDetector() method to include a new detector
  in this apparatus.  The detector will be decoded properly,
  and its variables will be available for cuts and histograms.

2. Write a derived class that creates the detector in the
  constructor. An example is THaBigBiteTpMwdc specially 
  designed for hardron package.



TECHNICALLY, THaBigBite is a standard THaSpectrometer with support of

1.Optics class THaOptics and its heritage classes. 
  For detailed infomation about THaOptics support, please see documentation of THaOptics

2.Special requirements of old AGen version of MWDC (THaMWDC), including reorder 
  of process and read out of environment variables. 

  Environmental variable USE_AGEN_MWDC controls whether THaMWDC is supported.
  if USE_AGEN_MWDC=1, then SWITCH_RECONSTRUCT_ORDER will be turned on, as well as 
  a collection of supportive functions & variables will be defined.

    In the case of SWITCH_RECONSTRUCT_ORDER=1 :
        Reconstruct process is different from standard THaSpectrometer. That's 
        Loop over all THaNonTrackingDetectors and call their CoarseProcess() methods.
        Loop over all THaTrackingDetectors and call their CoarseTrack() methods.
        Loop over all THaNonTrackingDetectors and call their FineProcess() methods.
        Loop over all THaTrackingDetectors and call their FineTrack() methods.
        In this way, THaMWDC could get hit info from THaTriggerPlane, which will 
        dramatically speed track reconstruction.
        Environmental Variable SWITCH_RECONSTRUCT_ORDER controls 
        whether or not to change order of steps in Reconstruct()
*/

#include "THaTrackingDetector.h"
#include "THaTrack.h"
#include "THaOptics.h"
#include "THaSpectrometer.h"
#include "THaParticleInfo.h"
#include "THaTrackingDetector.h"
#include "THaNonTrackingDetector.h"
#include "THaPidDetector.h"
#include "THaPIDinfo.h"
#include "THaTrack.h"
#include "TClass.h"
#include "TMath.h"
#include "TList.h"
#include "VarDef.h"
#include <cmath>
#include <string>
#include "TMath.h"
#include "TString.h"
#include "THaEvData.h"
#include "THaDetMap.h"
#include "TClonesArray.h"
#include "THaOpticsAnalytical.h"

//put this header file below all other headers
#include "THaBigBite.h"

#if DEBUG_LEVEL>=4//massive info
#define WITH_DEBUG 1
#else
#undef WITH_DEBUG
#endif//#if DEBUG_LEVEL>=4


using namespace std;


ClassImp(THaBigBite);

//___________________________________________________________________________
#if DEBUG_LEVEL>1  

Int_t    THaBigBite::DebugPort(void *p)
{
    static bool IsInit=false;

    //interface function for develop debug

    Info(Here("DebugPort()"),"\tDebugPort called");

    TDatime now;

    if (!IsInit) {
        Bool_t mr=MountOptics(new THaOpticsAnalytical("opticstest","BigBite Optics Child Class",this));
        assert(mr);
        Init(now);IsInit=true;
    }

    //cout<<"GetPcentral()="<<GetPcentral()<<endl;
    TVector3 bp(0,0,0),bd(0,0,1);

    GetOptics()->ApplyOptics(static_cast<THaTrack *>(p),bp,bd);


    return kOK;
}

#endif//DEBUG_LEVEL>1


//__________________________________________________________________________

THaBigBite::~THaBigBite()
{
    if (fOptics.IsValid()) delete (THaOptics *)(fOptics.GetObject());
}

#include "ha_compiledata.h" //include for analyzer version check

THaBigBite::THaBigBite( const char* name, const char* description ) 
:   THaSpectrometer( name, description )
{

    DEBUG_LEVEL_RELATED_PERFORMACE_CHECKER;

    DEBUG_HALL_A_ANALYZER_DEBUGER_INIT;

#if DEBUG_LEVEL>=1 && SWITCH_RECONSTRUCT_ORDER//start show warning 
    string sanaver(HA_VERSION);
    if (
        sanaver!="1.4.6" and
        sanaver!="1.4.7"
        )
    {
        Warning(Here("Constructor"),
            "\tReconstruction order changed to CoarseProcess->CoarseTrack->FineProcess->FineTrack. This code has been confirmed to be working properly under analyzer v%s. Please check compability with your version (v%s)",
            SWITCH_RECONSTRUCT_ORDER_COMPATIBLE_ANALYZER_VERSION,
            HA_VERSION
            );
    }
#endif

#if SWITCH_RECONSTRUCT_ORDER
    fListInit=kFALSE;
#endif//#if SWITCH_RECONSTRUCT_ORDER

	fTriggerTimingOffset=0;
} 

Bool_t THaBigBite::MountOptics(THaOptics *Optics)
{
    //mount a optics class to the Spectrometer. 
    //return true if successful.
    //Notice: This function have to be called before Init() to make sure readdatebase
    //        is called for the optics class.

    if (fOptics.IsValid()) {
        DEBUG_WARNING("MountOptics","The Optics has already been mounted. Deleting the old one and mounting new.");
        delete fOptics.GetObject();
    }

    fOptics=Optics;

    return fOptics.IsValid();
}

//_____________________________________________________________________________
Int_t THaBigBite::DefineVariables( EMode mode )
{
  // Define/delete standard variables for a spectrometer (tracks etc.)
  // Can be overridden or extended by derived (actual) apparatuses


	RVarDef vars[] = {
		{ "TriggerTimingOffset",    "Real_TDC = TDC(common-stop) - BB.TriggerTimingOffset",             "GetTriggerTimingOffset()" },
  { 0 }
	};

	return DefineVarsFromList( vars, mode )+THaSpectrometer::DefineVariables( mode );
}

//_____________________________________________________________________________
Int_t THaBigBite::FindVertices(TClonesArray& tracks )
{
    // Reconstruct target coordinates for all tracks found in the focal plane.

    if (fOptics.IsValid()) 
    {
        Int_t n_exist = tracks.GetLast()+1;
	
	((THaOptics *)(fOptics.GetObject()))->Clear("C");

        for( Int_t t = 0; t < n_exist; t++ ) {
            THaTrack* theTrack = static_cast<THaTrack*>( tracks.At(t) );
            assert(fOptics.IsValid());
            ((THaOptics *)(fOptics.GetObject()))-> ApplyOptics (theTrack);
        }
    }
    else
    {
        DEBUG_INFO(Here("FindVertices"),"\t No Optics Module found" );
    }



    // Find the "Golden Track". 
    if( GetNTracks() > 0 ) {
        //FIXME: quick and dirty hack to get started ...
        fGoldenTrack = static_cast<THaTrack*>( fTracks->At(0) );
        fTrkIfo      = *fGoldenTrack;
        fTrk         = fGoldenTrack;
    } else
        fGoldenTrack = NULL;

    return 0;
}

//__________________________________________________________________________

THaAnalysisObject::EStatus THaBigBite::Init( const TDatime& run_time )
{
    THaAnalysisObject::EStatus stat=THaSpectrometer::Init(run_time);
    if (stat!=kOK) return stat;

    //Optics is the last one to be initialized. So it could use any variable in 
    //THaBigBite class
    if (fOptics.IsValid()) stat=((THaOptics *)(fOptics.GetObject()))->Init(run_time);
    else
    {
        DEBUG_WARNING(Here("Init"),"\t No Optics Module found" );
    }

	fTriggerTimingOffset=0;
    return stat;
}

//__________________________________________________________________________
Int_t THaBigBite::ReadDatabase(const TDatime& date)
{



    // Read BigBite Database
#if USE_AGEN_MWDC
    static const Double_t degrad = TMath::Pi()/180.0;
#endif

#if DEBUG_LEVEL>=3 or USE_AGEN_MWDC//start show info
    static const char* const here = "ReadDatabase()";
#endif//#if DEBUG_LEVEL>=3

    FILE* file = OpenFile( date );
    if( !file ) return kFileError;
#if DEBUG_LEVEL>=3//start show info
    Info(Here(here),"\tTHaBigBite::ReadDatabase; DB File open OK\n" );
#endif//#if DEBUG_LEVEL>=3

#if USE_AGEN_MWDC
    //detector package orientation defined as rotations about Target Frame
    //  x, y, and z axes  (in that order)
    Double_t rot_x=0.0,rot_y = 0.0, rot_z = 0.0;
    Double_t det_x=0.0, det_y=0.0, det_z=0.0;
    Double_t tar_x=0.0, tar_y=0.0, tar_z=0.0;
    Double_t mag_x=0.0, mag_y=0.0, mag_z=0.0;

    Double_t bendz = 0.0, bendy = 0.0 , bendx = 0.0;

    fBendTheta = 0.0;
    Double_t bendthetadeg = 0.0;

    fShowerXExtension = 0.0;
    fShowerYExtension = 0.0;
    fTargetXExtension = 0.0;
    fTargetYExtension = 0.0;

    const TagDef tags[]={
        {"det_rot_x", &rot_x },
        {"det_rot_y", &rot_y },
        {"det_rot_z", &rot_z },
        {"det_x", &det_x },
        {"det_y", &det_y },
        {"det_z", &det_z },
        {"target_x", &tar_x },
        {"target_y", &tar_y },
        {"target_z", &tar_z },
        {"target_length", &fTargetLength },
        {"magnet_x", &mag_x },
        {"magnet_y", &mag_y },
        {"magnet_z", &mag_z },
        {"effbendz", &bendz},
        {"effbendy", &bendy},
        {"effbendx", &bendx},
        {"effbendplanetheta", &bendthetadeg},
        {"showerxext", &fShowerXExtension},
        {"showeryext", &fShowerYExtension},
        {"targetxext", &fTargetXExtension},
        {"targetyext", &fTargetYExtension},
        {"targetwindowxoffset", &fTargetWindowXOffset },  
        { 0 }
    };

    Int_t err =LoadDB( file, date,tags, fPrefix );

    if( err ){
        clog<<Here(here)<<" returned from LoadDB with err="<<err<<endl;
        Error(Here(here), "Required tag %s%s missing in the "
            "run databse. \nBigBite initialization failed.",
            fPrefix, tags[err-1].name );
        fclose(file);
        return kInitError;
    }
    else
    {
#if DEBUG_LEVEL>=3//start show info
        TString sDebugOutput;
        sDebugOutput=GetName();
        sDebugOutput+=" Database read in successfully with:";
        sDebugOutput+="\n \tdet_rot_x\t= ";    sDebugOutput+=rot_x;
        sDebugOutput+="\n \tdet_rot_y\t= ";    sDebugOutput+=rot_y;
        sDebugOutput+="\n \tdet_rot_z\t= ";    sDebugOutput+=rot_z;
        sDebugOutput+="\n \tdet_x\t= ";    sDebugOutput+=det_x;
        sDebugOutput+="\n \tdet_y\t= ";    sDebugOutput+=det_y;
        sDebugOutput+="\n \tdet_z\t= ";    sDebugOutput+=det_z;
        sDebugOutput+="\n \ttarget_x\t= ";    sDebugOutput+=tar_x;
        sDebugOutput+="\n \ttarget_y\t= ";    sDebugOutput+=tar_y;
        sDebugOutput+="\n \ttarget_z\t= ";    sDebugOutput+=tar_z;
        sDebugOutput+="\n \ttarget_length\t= ";    sDebugOutput+=fTargetLength;
        sDebugOutput+="\n \tmagnet_x\t= ";    sDebugOutput+=mag_x;
        sDebugOutput+="\n \tmagnet_y\t= ";    sDebugOutput+=mag_y;
        sDebugOutput+="\n \tmagnet_z\t= ";    sDebugOutput+=mag_z;
        sDebugOutput+="\n \teffbendz\t= ";    sDebugOutput+=bendz;
        sDebugOutput+="\n \teffbendy\t= ";    sDebugOutput+=bendy;
        sDebugOutput+="\n \teffbendx\t= ";    sDebugOutput+=bendx;
        sDebugOutput+="\n \teffbendplanetheta\t= ";    sDebugOutput+=bendthetadeg;
        sDebugOutput+="\n \tshowerxext\t= ";    sDebugOutput+=fShowerXExtension;
        sDebugOutput+="\n \tshoweryext\t= ";    sDebugOutput+=fShowerYExtension;
        sDebugOutput+="\n \ttargetxext\t= ";    sDebugOutput+=fTargetXExtension;
        sDebugOutput+="\n \ttargetyext\t= ";    sDebugOutput+=fTargetYExtension;
        sDebugOutput+="\n \ttargetwindowxoffset\t= ";    sDebugOutput+=fTargetWindowXOffset;  
        sDebugOutput+="\n";
        Info(Here(here),sDebugOutput.Data());
#endif    
    }

    //Compute rotation matricies between detector frame and "target" frame

    //if( useshowercuts > 0 )
    //  fUseShowerCuts = 1;
    //else
    //  fUseShowerCuts = 0;

    //fHardMaxGroups = (Int_t) hmaxg;
    //fMaxGroups = (Int_t) maxg;
    //fMinimumPlanes = (Int_t) minp;
    //fMaxCallThreshold = (Int_t) hthresh;
    fBendPlaneCenter.SetXYZ( bendx, bendy, bendz );

    fTargOffset.SetXYZ(tar_x,tar_y,tar_z);
    fDetOffset.SetXYZ(det_x,det_y,det_z);
    fMagnetCenter.SetXYZ(mag_x,mag_y,mag_z);

    fdet_rot_x=degrad*rot_x;
    fdet_rot_y=degrad*rot_y;
    fdet_rot_z=degrad*rot_z;

    // fBendTheta is given in target coords, but we want detector coords
    fBendTheta = degrad * ( bendthetadeg + rot_y );

    fDetToTarg.SetToIdentity();
    fDetToTarg.RotateX(fdet_rot_x).RotateY(fdet_rot_y).RotateZ(fdet_rot_z);
    fTargToDet=fDetToTarg.Inverse();
    //Compute rotation matricies between lab frame and "target" frame
    //input is from B.theta read in from db_run.dat
    //FIXME: redundant?  fToLabRot and fToTraRot already do this, even
    //for the more general case of a possible out-of-plane angle
    fLabToTarg.SetToIdentity();
    fLabToTarg.RotateZ(90.0*degrad); //Targ x becomes Lab -y

#if DEBUG_LEVEL>=3//start show info
    clog << "\tBigBite Central Angle: "  << fThetaGeo*180.0/TMath::Pi() << endl;
#endif//#if DEBUG_LEVEL>=3

    fLabToTarg.RotateX(fThetaGeo);     //rotate about verticle so z goes down BigBite
    fTargToLab=fLabToTarg.Inverse();


#endif //#if USE_AGEN_MWDC

    fclose(file);

    //if( fBeamName )
    //  {
    //    fBeam = dynamic_cast<THaBeam *>( gHaApps->FindObject(fBeamName) );
    //    clog << GetName() << " using beam " << fBeamName << endl;
    //  }
    //else 
    //  {
    //    fBeam = NULL;
    //    clog << GetName() << " not using beam data" << endl;
    //  }

    return kOK;
}

//===============================================================================//
//
//  Forget any code below if you are not using the old AGen THaMWDC class
//
//===============================================================================//

//_____________________________________________________________________________
Int_t THaBigBite::TrackCalc()
{
    return 0;
}

#if USE_AGEN_MWDC
//_____________________________________________________________________________
TVector3 THaBigBite::GetTargetCenterInDet()
{
    TVector3 TargetCenter;
    TargetCenter.SetXYZ( 0.0, 0.0, 0.0 );

    TargetCenter = TargetCenter - fDetOffset;
    TargetCenter.Transform(fTargToDet);

    return TargetCenter;
}
TVector3 THaBigBite::GetTargetStartInDet()
{
    TVector3 TargetStart;
    TVector3 TargetCenter = fTargOffset;

    TargetStart.SetZ( TargetCenter.Z() - fTargetLength/2.0 );
    TargetStart.SetY(TargetCenter.Y());
    TargetStart.SetX(TargetCenter.X());

    TargetStart = TargetStart - fTargOffset;
    TargetStart.Transform(fLabToTarg);
    TargetStart = TargetStart - fDetOffset; 
    TargetStart.Transform(fTargToDet);

    return TargetStart;
}

TVector3 THaBigBite::GetTargetEndInDet()
{
    TVector3 TargetEnd;
    TVector3 TargetCenter = fTargOffset;

    TargetEnd.SetZ( TargetCenter.Z() + fTargetLength/2.0 );
    TargetEnd.SetY(TargetCenter.Y());
    TargetEnd.SetX(TargetCenter.X());

    TargetEnd = TargetEnd - fTargOffset;
    TargetEnd.Transform(fLabToTarg);
    TargetEnd = TargetEnd - fDetOffset;
    TargetEnd.Transform(fTargToDet);

    return TargetEnd;
}
#endif//#if USE_AGEN_MWDC

//_____________________________________________________________________________

#if SWITCH_RECONSTRUCT_ORDER
//__________________________________________________________________________

// now function Reconstruct() Track() CoarseReconstruct() CoarseTrack() just play a role of 
// gate way to steps of reconstruction. Actually action of each function is defined in stepN()
//
// ListInit should be direct copy from analyzer THaSpectrometer source code

void THaBigBite::ListInit()
{
    // Initialize lists of specialized detectors. 
    // Private function called by Reconstruct().
#if DEBUG_LEVEL>=1//start show warning 
    if (fListInit)
        Warning(Here("ListInit()"),
        "\t Performance warning: This functon is called more than once." );
#endif//#if DEBUG_LEVEL>=1

#ifdef WITH_DEBUG
    clog<<"ListInit() called"<<endl;
#endif

    fTrackingDetectors->Clear();
    fNonTrackingDetectors->Clear();
    fPidDetectors->Clear();

    TIter next(fDetectors);
    while( THaSpectrometerDetector* theDetector = 
        static_cast<THaSpectrometerDetector*>( next() )) {

            if( theDetector->IsTracking() )
                fTrackingDetectors->Add( theDetector );
            else
                fNonTrackingDetectors->Add( theDetector );

            if( theDetector->IsPid() )
                fPidDetectors->Add( theDetector );
    }

    // Set up PIDinfo and vertex objects that can be associated with tracks

    UInt_t ndet  = GetNpidDetectors();
    UInt_t npart = GetNpidParticles();
    TClonesArray& pid  = *fTrackPID;

    for( int i = 0; i < kInitTrackMultiplicity; i++ ) {
        new( pid[i] )  THaPIDinfo( ndet, npart );
    }

    fListInit = kTRUE;
}

//__________________________________________________________________________

Int_t THaBigBite::CoarseTrack()
{
    // CoarseTrack function is the 1st step of spectrometer analysis

#if DEBUG_LEVEL>=1//start show warning 
    if (IsDone(kCoarseTrack))
        Warning(Here("CoarseTrack"),
        "\t Performance warning: This functon is called more than once." );
#endif//#if DEBUG_LEVEL>=1


#ifdef WITH_DEBUG
    clog<<"CoarseTrack() l1 called"<<endl;
#endif

    if( !fListInit )
        ListInit();


#ifdef WITH_DEBUG
    clog<<"CoarseTrack() called"<<endl;
#endif

    Step1();

    fStagesDone |= kCoarseTrack;
    return 0;
}

//_____________________________________________________________________________
Int_t THaBigBite::CoarseReconstruct()
{


    //assert(!IsDone(kCoarseRecon));
#if DEBUG_LEVEL>=1//start show warning 
    if (IsDone(kCoarseRecon))
        Warning(Here("CoarseReconstruct"),
        "\t Performance warning: This functon is called more than once." );
#endif//#if DEBUG_LEVEL>=1

#ifdef WITH_DEBUG
    clog<<"CoarseReconstruct() l1 called"<<endl;
#endif

    if( !IsDone(kCoarseTrack))
        CoarseTrack();

#ifdef WITH_DEBUG
    clog<<"CoarseReconstruct() called"<<endl;
#endif

    Step2();

    fStagesDone |= kCoarseRecon;
    return 0;
}

//_____________________________________________________________________________
Int_t THaBigBite::Track()
{
    // Fine tracking.  Compute the tracks with high precision.
    // If coarse tracking was done, this step should simply "refine" the
    // tracks found earlier, not add new tracks to fTrack.

    //assert(!IsDone(kTracking));
#if DEBUG_LEVEL>=1//start show warning 
    if (IsDone(kTracking))
        Warning(Here("Track")
        ,"\t Performance warning: This functon is called more than once." );
#endif//#if DEBUG_LEVEL>=1

#ifdef WITH_DEBUG
    clog<<"Track() l1 called"<<endl;
#endif

    if( !IsDone(kCoarseRecon))
        CoarseReconstruct();

#ifdef WITH_DEBUG
    clog<<"Track() called"<<endl;
#endif

    Step3();

    fStagesDone |= kTracking;
    return 0;
}

//_____________________________________________________________________________
//Int_t THaBigBite::FineReconstruct()
//{
//    //assert(!IsDone(kFineReconstruct));
//#if DEBUG_LEVEL>=1//start show warning 
//    if (IsDone(kFineReconstruct))
//        Warning(Here("FineReconstruct"),
//        "\t Performance warning: This functon is called more than once." );
//#endif//#if DEBUG_LEVEL>=1
//
//#ifdef WITH_DEBUG
//    clog<<"FineReconstruct() l1 called"<<endl;
//#endif
//    if( !IsDone(kCoarseTrack))
//        CoarseTrack();
//
//#ifdef WITH_DEBUG
//    clog<<"FineReconstruct() called"<<endl;
//#endif
//
//    Step3(void)
//
//    fStagesDone |= kFineReconstruct;
//    return 0;
//}


//_____________________________________________________________________________
Int_t THaBigBite::Reconstruct()
{
    // This method implements a fairly generic algorithm for spectrometer
    // reconstruction which should be useful for most situations.
    // Additional, equipment-specific processing can be done in
    // a derived class that calls this method first.
    //
    // The algorithm is as follows:
    // excute step 1-4
    // Compute additional attributes of tracks (e.g. momentum, beta)
    // Find "Golden Track"
    // Combine all PID detectors to get overall PID for each track
    //

    //assert(!IsDone(kReconstruct));
#if DEBUG_LEVEL>=1//start show warning 
    if (IsDone(kReconstruct))
        Warning(Here("Reconstruct"),
        "\t Performance warning: This functon is called more than once." );
#endif//#if DEBUG_LEVEL>=1

#ifdef WITH_DEBUG
    clog<<"Reconstruct() l1 called"<<endl;
#endif

    if( !IsDone(kTracking))
        Track();

#ifdef WITH_DEBUG
    clog<<"Reconstruct() called"<<endl;
#endif

    Step4();

    // Compute additional track properties (e.g. beta)
    // Find "Golden Track" if appropriate.

    TrackCalc();

    // Compute combined PID

    if( fPID ) CalcPID();

    fStagesDone |= kReconstruct;

    return 0;
}
//__________________________________________________________________________

Int_t THaBigBite::Step1(void)
{
    // current configuration
    // For all non-tracking detectors:
    //   CoarseProcess()
    // For all tracking detectors:
    //   CoarseTrack()
    // For all non-tracking detectors:
    //   FineProcess()
    // For all tracking detectors:
    //   FineTrack()
    // Reconstruct tracks to target

    // 1st step: Coarse processing.  Pass the coarse tracks to the remaining
    // detectors for any processing that can be done at this stage.
    // This may include clustering and preliminary PID.
    // PID information is tacked onto the tracks as a THaPIDinfo object.
#ifdef WITH_DEBUG
    clog<<"Actual Step1: CoarseProcess"<<endl;
#endif

    TIter next( fNonTrackingDetectors );
    while( THaNonTrackingDetector* theNonTrackDetector =
        static_cast<THaNonTrackingDetector*>( next() )) {
#ifdef WITH_DEBUG
            if( WITH_DEBUG ) clog << "Call CoarseProcess() for " 
                << theNonTrackDetector->GetName() << "... ";
#endif
            theNonTrackDetector->CoarseProcess( *fTracks );
#ifdef WITH_DEBUG
            if( WITH_DEBUG ) clog << "done.\n";
#endif
    } 

    return kOK;
}
//__________________________________________________________________________

Int_t THaBigBite::Step2(void)
{
    // 2nd step: Coarse tracking.  This should be quick and dirty.
    // Tracking detector could use hit data of Nontracking ones.
    // Any tracks found are put in the fTrack array.
#ifdef WITH_DEBUG
    clog<<"Actual Step2: CoarseTrack"<<endl;
#endif

    TIter next( fTrackingDetectors );
    while( THaTrackingDetector* theTrackDetector =
        static_cast<THaTrackingDetector*>( next() )) {
#ifdef WITH_DEBUG
            if( WITH_DEBUG ) clog << "Call CoarseTrack() for " 
                << theTrackDetector->GetName() << "... ";
#endif
            theTrackDetector->CoarseTrack( *fTracks );
#ifdef WITH_DEBUG
            if( WITH_DEBUG ) clog << "done.\n";
#endif
    }    

    return kOK;
}
//__________________________________________________________________________

Int_t THaBigBite::Step3(void)
{
    // Fine processing.  Pass the precise tracks to the
    // remaining detectors for any precision processing.
    // PID likelihoods should be calculated here.

#ifdef WITH_DEBUG
    clog<<"Actual Step3: FineProcess"<<endl;
#endif

    TIter next( fNonTrackingDetectors );
    while( THaNonTrackingDetector* theNonTrackDetector =
        static_cast<THaNonTrackingDetector*>( next() )) 
    {
#ifdef WITH_DEBUG
        if( WITH_DEBUG ) clog << "Call FineProcess() for " 
            << theNonTrackDetector->GetName() << "... ";
#endif
        theNonTrackDetector->FineProcess( *fTracks );
#ifdef WITH_DEBUG
        if( WITH_DEBUG ) clog << "done.\n";
#endif
    }   

    return kOK;
}
//__________________________________________________________________________

Int_t THaBigBite::Step4(void)
{
#ifdef WITH_DEBUG
    clog<<"Actual Step4: FineTrack"<<endl;
#endif

    TIter next( fTrackingDetectors );
    while( THaTrackingDetector* theTrackDetector =
        static_cast<THaTrackingDetector*>( next() )) {
#ifdef WITH_DEBUG
            if( WITH_DEBUG ) clog << "Call FineTrack() for " 
                << theTrackDetector->GetName() << "... ";
#endif
            theTrackDetector->FineTrack( *fTracks );
#ifdef WITH_DEBUG
            if( WITH_DEBUG ) clog << "done.\n";
#endif
    }    

    // Reconstruct tracks to target/vertex
    // This usually also determines the track momentum
    FindVertices( *fTracks );

    return kOK;
}
//__________________________________________________________________________


#endif //#if SWITCH_RECONSTRUCT_ORDER

//__________________________________________________________________________
#if DEBUG_LEVEL>=5 
Int_t    THaBigBite::Decode( const THaEvData& evdata)
{
    clog <<"-- THaBigBite (name: "<<GetName()<<") Search for hit in THaEvData of Run#"<<evdata.GetRunNum()<<" --"<<endl;
    clog<<"EvType="<<evdata.GetEvType()<<"; EvLength="<<evdata.GetEvLength()<<"; EvNum="<<evdata.GetEvNum()<<endl;

    //evdata.Dump();
    //char buf[10240];
    //evdata.hexdump(buf,10240);
    //clog<<">>>>>Hex dump:"<<endl<<buf<<endl;

    {
        int crate,slot,chan;
        for (crate=0;crate<=100;crate++)
        {
            for (slot=0;slot<=50;slot++)
            {
                if (evdata.GetNumChan(crate,slot)) 
                {
                    clog<<"Get Channel\n";
                    evdata.PrintSlotData(crate,slot);
                }
                for (chan=0;chan<=100;chan++)
                {
                    Int_t nHits = evdata.GetNumHits(crate, slot, chan);
                    if (nHits)
                        clog<<"found "<<nHits<<" hit(s) @ crate="<<crate
                        <<", slot="<<slot<<", chan="<<chan<<endl;    
                }
            }
        }
    }
    clog <<"-- End search --"<<endl;

    return THaSpectrometer::Decode(evdata);
}
#endif
