//////////////////////////////////////////////////////////////////////////
//
// Hall A analyzer replay script Example
//
//////////////////////////////////////////////////////////////////////////
//	
//	Author : Jin Huang (jinhuang@jlab.org)   May 2008
//
//////////////////////////////////////////////////////////////////////////


#include "def.h"

using namespace std;

void replay_farm_BB(Int_t runnumber=0,Int_t filesuffix=0,Int_t all=0,Int_t fstEvt=0,Bool_t QuietRun = kTRUE)
{
//*******************************************************************************************************
//Add BigBite 
//*******************************************************************************************************

  THaBigBite* pB=new THaBigBite("BB","BigBite"); 
  pB->AddDetector(new TreeSearch::MWDC("mwdc","MWDC",pB));
  pB->AddDetector( new THaScintPlaneDet( "s", "BB Scintillator",pB ));
  pB->AddDetector( new THaScintPlaneDet( "sum", "BB Total sum",pB));
  pB->AddDetector( new THaScintPlaneDet( "psum", "BB Preshower sum",pB));
  pB->AddDetector( new THaBBTotalShower( "ts", "BB Total shower",pB));
  pB->AddDetector( new THaCherenkov("cer", "BB gas Cerencov counter",pB));

//*******************************************************************************************************
//Add Beams
//*******************************************************************************************************
  THaApparatus* B = new THaIdealBeam("B","Ideal Beam, for Test Only");
  gHaApps->Add( B );
  gHaApps->Add(new THaUnRasteredBeam("BBurb","Unrastered beam"));
  gHaApps->Add(new THaRasteredBeam("BBrb","Rastered Beam"));

//*******************************************************************************************************
//Add Bigbite Helicity
//*******************************************************************************************************
  B->AddDetector(new THaADCHelicity("adchel.BB", "Beam Helicity BB"));
  B->AddDetector(new THaADCHelicity("adchel2.BB", "Beam Helicity BB"));

//*******************************************************************************************************
//Add physics
//*******************************************************************************************************
  pB->MountOptics(new THaOpticsE06010("optics","BigBite Optics Child Class",pB,"BBurb"));
  gHaPhysics->Add( new THaGoldenTrack("BB.gold","Golden track for Bigbite", "BB") );

//*******************************************************************************************************
//Decode miscellaneous things
//*******************************************************************************************************
  THaApparatus* DECDAT = new THaDecData("DBB","Misc. Decoder Data");
  //THaApparatus* DECDAT = new BBDecData("DBB","Misc. Decoder Data");
  gHaApps->Add( DECDAT );
  gHaApps->Add(pB);

  ////////////////////////////////////////
  //   Scalars
  ////////////////////////////////////////
  // add scalers
  cout<<"replay: Adding Scalar ..."<<endl;
  gHaScalers->Add(new THaScalerGroup("bbite"));
  gHaScalers->Add(new THaScalerGroup("evbbite"));
 
  FarmReplayCore(
	    runnumber,            //run #
		filesuffix,			  //suffix of data file (e.g. 0 for #.dat.0)
	    all,                  //-1=replay all;0=ask for a number
	    10000,                   //defaut replay event num
	    "%s/e06014_det_BB_%d_%d.root", //output file format
	    "replay_farm_BB.odef",       //out define
	    "replay_farm_BB.cdef",       //cut define
	    false,//Enable Scalar?
	    fstEvt,					//First Event To Replay
		QuietRun				//whether ask user for inputs
	    );
}
