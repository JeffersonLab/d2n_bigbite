//========================================================================
//
// d2n Physics Replay Script
//
//========================================================================
//	
// Nov 2008 : Jin Huang -- Init the script with ref to detector replays
//                         for Transversity
//
// Sept 2009 : David Flay -- Edited for d2n
// 
//========================================================================



#include "def.h"

using namespace std;


void replay_phys_L(Int_t runnumber=0,Int_t all=0,Int_t fstEvt=0)
{


  //======================================
  //   Beams
  //======================================

  cout<<"replay: Adding Idea Beam"<< endl;
  THaApparatus* B = new THaIdealBeam("B","Idea Beam, for Test Only");
  gHaApps->Add( B );
  
  cout<<"replay: Adding L-arm Helicity"<< endl;
  B->AddDetector( new THaADCHelicity("adchel.L","Beam helicity L-arm") );
  B->AddDetector( new THaADCHelicity("adchel2.L","Beam helicity-2 L-arm") );
  B->AddDetector( new THaG0Helicity("g0hel.L","Left arm G0 helicity") );	
   
  
  cout<<"replay: Adding UnRastered and Rastered Beam ..."<<endl;
  gHaApps->Add(new THaUnRasteredBeam("urb","Unrastered beam"));
  gHaApps->Add(new THaRasteredBeam("rb","Rastered Beam"));

  //======================================
  //   LHRS
  //======================================

  cout<<"replay: Adding HRS-L ..."<<endl;
  THaApparatus* HRSL = new THaHRS("L","Left HRS");
  gHaApps->Add( HRSL );
  
  // add L detectors that are not in the default config
  HRSL->AddDetector( new THaCherenkov("cer", "Gas Cherenkov counter" ));
//   HRSL->AddDetector( new THaCherenkov("a1", "A1 Cherenkov counter" ));
  HRSL->AddDetector( new THaShower("prl1", "Pre-shower pion rej." ));
  HRSL->AddDetector( new THaShower("prl2", "Show pion rej." ));

  //cout<<"replay: adding RICH detector ..."<<endl;
  //HRSL->AddDetector( new THaRICH("rich","The RICH")); 
  
  cout<<"replay: adding LHRS track modules ..."<<endl;
  gHaPhysics->Add( new THaGoldenTrack("L.gold","Golden track for LHRS", "L") );

  THaPhysicsModule *Rpt_l = new THaReactionPoint("ReactPt_L","Reaction vertex for Left","L","B");
  gHaPhysics->Add( Rpt_l );

  // Correct for using an Extended target
  // This needs information about the Point of interaction (hence a THaVertexModule)
  THaPhysicsModule* TgC_l = new THaExtTarCor("ExTgtCor_L","Corrected for extended target, HRS-L","L","ReactPt_L");
  gHaPhysics->Add( TgC_l );

  //======================================
  //   Other Modules
  //======================================

//   cout<<"replay: Adding Target ..."<<endl;
//   THaHe3Target* pT=new THaHe3Target("he3","Hall A Polarized he3 target");
//   gHaApps->Add(pT);

  cout<<"replay: Adding Decoder Data ..."<<endl;
  gHaApps->Add(new THaDecData("DL","Misc. Decoder Data"));

//   cout<<"replay: Adding A Quick Hack Code to Extract t3[0] ..."<<endl;
//   THaApparatus* DL1 = new THaIdealBeam("DL1","Base Apparatus for extract t3[0]");
//   gHaApps->Add( DL1 );
//   DL1->AddDetector(new THaADCHe3Spin("t3","copy of DL.t3[0], for .odef file",DL1));

  //======================================
  //   Physics
  //======================================

  cout<<"replay: adding Physics modules ..."<<endl;
  gHaPhysics->Add(new THaPrimaryKine("PriKineL","kinematics of scattering of electron to HRS","L","urb",.939565));
  gHaPhysics->Add(new THaPrimaryKine("PriKineLHe3","kinematics of scattering of electron to HRS","L","urb",3.016*0.931494));
  
//  THaSecondaryKine *SecKineP=new THaSecondaryKine(
//                "SecKineP","secondary kinematics of scattering proton into LHRS",
//                "ExTgtCor_L","PriKineBB", .938);
//  gHaPhysics->Add(SecKineP);
  
  //======================================
  //   Scalars
  //======================================
  
   gHaScalers->Add(new THaScalerGroup("Left"));
   gHaScalers->Add(new THaScalerGroup("bbite"));
   gHaScalers->Add(new THaScalerGroup("evLeft"));
   gHaScalers->Add(new THaScalerGroup("evbbite"));

  //======================================
  //   Do replay
  //======================================

  
  ReplayCore(
	    runnumber,            //run #
	    all,                  //-1=replay all;0=ask for a number
	    100000,                   //defaut replay event num
	    "%s/e06014_phys_L_%d.root", //output file format
	    "replay_phys_L.odef",    	//out define
	    "replay_test.cdef",    	//empty cut define
	    kTRUE,                 	//replay scalar?
	    fstEvt					//First Event To Replay
	    );

}
