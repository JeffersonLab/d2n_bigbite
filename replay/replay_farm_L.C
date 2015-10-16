//////////////////////////////////////////////////////////////////////////
//
//  Left arm detector replay script
//
//////////////////////////////////////////////////////////////////////////
//	
//	Author : Vincent A. Sulkosky & Jin Huang
//
//////////////////////////////////////////////////////////////////////////


#include "def.h"

using namespace std;

void replay_farm_L(Int_t runnumber=0,Int_t filesuffix=0,Int_t all=0,Int_t fstEvt=0,Bool_t QuietRun = kTRUE)
{

  THaApparatus* B = new THaIdealBeam("B","Idea Beam, for Test Only");
  gHaApps->Add( B );
  
  cout<<"replay: Adding L-arm Helicity"<< endl;
  B->AddDetector( new THaADCHelicity("adchel.L","Beam helicity L-arm") );
  B->AddDetector( new THaADCHelicity("adchel2.L","Beam helicity-2 L-arm") );
  B->AddDetector( new THaG0Helicity("g0hel.L","Left arm G0 helicity") );	
  
  cout<<"replay: Adding UnRastered and Rastered Beam ..."<<endl;
  gHaApps->Add(new THaUnRasteredBeam("urb","Unrastered beam"));
  gHaApps->Add(new THaRasteredBeam("rb","Rastered Beam"));
  
  cout<<"replay: Adding Decoder Data ..."<<endl;
  gHaApps->Add(new THaDecData("DL","Misc. Decoder Data"));
  
  cout<<"replay: Adding HRS-L ..."<<endl;
  THaApparatus* HRSL = new THaHRS("L","Left HRS");
  gHaApps->Add( HRSL );
  
  // add detectors that are not in the default config
  HRSL->AddDetector( new THaCherenkov("cer", "Gas Cherenkov counter" ));
//  HRSL->AddDetector( new THaCherenkov("a1", "A1 Cherenkov counter" ));
  HRSL->AddDetector( new THaShower("prl1", "Pre-shower pion rej." ));
  HRSL->AddDetector( new THaShower("prl2", "Show pion rej." ));

/*  cout<<"replay: adding RICH detector ..."<<endl;
  HRSL->AddDetector( new THaRICH("rich","The RICH")); 
*/  
  cout<<"replay: adding Physics modules ..."<<endl;
  
  gHaPhysics->Add( new THaGoldenTrack("L.gold","Golden track for LHRS", "L") );
  
  THaPhysicsModule *Rpt_l = new THaReactionPoint(
						 "ReactPt_L","Reaction vertex for Left","L","B");
  gHaPhysics->Add( Rpt_l );
  
  // Correct for using an Extended target
  // This needs information about the Point of interaction (hence a THaVertexModule)
  THaPhysicsModule* TgC_l = new THaExtTarCor("ExTgtCor_L",
					     "Corrected for extended target, HRS-L",
					     "L","ReactPt_L");
  gHaPhysics->Add( TgC_l );
  
  // add scalers
  THaScalerGroup* LeftScalers = new THaScalerGroup("Left");
  gHaScalers->Add(LeftScalers);
  
  FarmReplayCore(
	    runnumber,            //run #
		filesuffix,			  // suffix of data file (e.g. 0 for #.dat.0)
	    all,                  //-1=replay all;0=ask for a number
	    200000,                   //defaut replay event num
	    "%s/e06014_det_L_%d.root", //output file format
	    "replay_det_L.odef",	//out define
	    "replay_det_L.cdef", 	//empty cut define
	    kTRUE,                 	//replay scalar?
	    fstEvt,					//First Event To Replay
		QuietRun				//whether ask user for inputs
	    );
}
