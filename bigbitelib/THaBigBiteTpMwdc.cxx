//////////////////////////////////////////////////////////////////////////
//
//	THaBigBiteTpMwdc
//
//  A bundle deal of detectors and bigbite spectrometer specially designed
//  for BigBite Hadron Package.
//
//  Here is the setup:
//
//      Module/Detector     Class               Name    Database
//      ----------------------------------------------------------------
//      Trigger Plane       THaTriggerPlane     tp      db_BB.tp.dat
//      MWDC                TreeSearch::MWDC    dc      db_BB.mwdc.dat
//      Optics              THaOpticsAnalytical optics  db_BB.optics.dat
//	
//////////////////////////////////////////////////////////////////////////
//	
// Author: Jin Huang <mailto:jinhuang@jlab.org>    July 2007
//		reference to AGenLib::class THaBBe
//	Modify History:
//
//////////////////////////////////////////////////////////////////////////

#include "THaMWDC.h"
#include "MWDC.h"
#include "THaTriggerPlane.h"

#include "THaOpticsAnalytical.h"

//put this header file below all other headers
#include "THaBigBiteTpMwdc.h"

ClassImp(THaBigBiteTpMwdc);
//___________________________________________________________________________

THaBigBiteTpMwdc::THaBigBiteTpMwdc(const char* name, const char* description)
: THaBigBite( name, description )
{

    DEBUG_LEVEL_RELATED_PERFORMACE_CHECKER;

    DEBUG_HALL_A_ANALYZER_DEBUGER_INIT

# if USE_AGEN_MWDC
    AddDetector( new THaMWDC("dc", "BigBite Multi Wire Drift Chamber", this));
    fMWDC=(THaMWDC*)GetDetector("dc");  
    assert(fMWDC);
#else
    AddDetector( new TreeSearch::MWDC("mwdc", "BigBite Multi Wire Drift Chamber", this));
    fMWDC=(TreeSearch::MWDC*)GetDetector("mwdc");  
#endif //#if USE_AGEN_MWDC

    AddDetector( new THaTriggerPlane("tp", "BigBite trigger plane", this));
    fTriggerPlane=(THaTriggerPlane*)GetDetector("tp");		
    assert(fTriggerPlane);


    //choose the type of optics here
    Bool_t mr=MountOptics(new THaOpticsAnalytical("optics","BigBite Optics Child Class",this));
    assert(mr);
}

//___________________________________________________________________________
THaBigBiteTpMwdc::~THaBigBiteTpMwdc(void)
{
	
}

//___________________________________________________________________________
#if DEBUG_LEVEL>1 

#include "THaScintPlane.h"
Int_t	THaBigBiteTpMwdc::DebugPort(void *p)
{
	Info(Here("DebugPort()"),"\tDebugPort called");

	TDatime now;

	//MakePrefix();

	
	//fTriggerPlane-> GetEPlane()->ReadDatabase(now) ;
	Init(now);
	//fTriggerPlane->GetEPlane()->Dump();

    cout<<"GetPcentral()="<<GetPcentral()<<endl;

	return kOK;
}
#endif//DEBUG_LEVEL>1

