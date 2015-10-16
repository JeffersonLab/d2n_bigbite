//////////////////////////////////////////////////////////////////////////
//
// d2n Physics Replay Script for Coincidence Mode
//
//////////////////////////////////////////////////////////////////////////
//	
// Nov 2008 : Jin Huang : Init the script with ref to detector replays
//                        For Transversity
//
// Oct 2009 : David Flay : Edited to reflect proper configuration for 
//			   d2n coincidence runs
//
//////////////////////////////////////////////////////////////////////////



#include "def.h"

using namespace std;


void replay_phys_coin(Int_t runnumber=0,Int_t all=0,Int_t fstEvt=0,Bool_t QuietRun = kTRUE)
{


  ////////////////////////////////////////
  //   Beams
  ////////////////////////////////////////

  cout<<"replay: Adding Idea Beam"<< endl;

  THaApparatus* B = new THaIdealBeam("B","Idea Beam, for Test Only");
  gHaApps->Add( B );

  cout<<"replay: Adding BigBite Helicity"<< endl;
  B->AddDetector( new THaADCHelicity("adchel.BB","Beam helicity BB") );
  B->AddDetector( new THaADCHelicity("adchel2.BB","Beam helicity BB") );
  cout<<"replay: Adding L-arm Helicity"<< endl;
  B->AddDetector( new THaADCHelicity("adchel.L","Beam helicity L-arm") );
  B->AddDetector( new THaADCHelicity("adchel2.L","Beam helicity-2 L-arm") );
  B->AddDetector( new THaG0Helicity("g0hel.L","Left arm G0 helicity") );	

  
  //BB Rastered/Unrastered beam
  gHaApps->Add(new THaUnRasteredBeam("BBurb","BB Unrastered beam"));
  gHaApps->Add(new THaRasteredBeam("BBrb","BB Rastered Beam"));
  //HRS Rastered/Unrastered beam
  gHaApps->Add(new THaUnRasteredBeam("urb","HRS Unrastered beam"));
  gHaApps->Add(new THaRasteredBeam("rb","HRS Rastered Beam"));
  
  //======================================
  //   LHRS
  //======================================

  cout<<"replay: Adding HRS-L ..."<<endl;
  THaApparatus* HRSL = new THaHRS("L","Left HRS");
  gHaApps->Add( HRSL );
  
  // add L detectors that are not in the default config
  HRSL->AddDetector( new THaCherenkov("cer", "Gas Cherenkov counter" ));
//  HRSL->AddDetector( new THaCherenkov("a1", "A1 Cherenkov counter" ));
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

  ////////////////////////////////////////
  //   Bigbite
  ////////////////////////////////////////

  cout<<"replay: adding BigBite ..."<<endl;
  THaBigBite* pB=new THaBigBite("BB","BigBite"); 
  pB->AddDetector(new TreeSearch::MWDC("mwdc","MWDC",pB));
  pB->AddDetector( new THaScintPlaneDet( "s", "BB Scintillator",pB ));
  pB->AddDetector( new THaScintPlaneDet( "sum", "BB Total sum",pB));
  pB->AddDetector( new THaScintPlaneDet( "psum", "BB Preshower sum",pB));
  pB->AddDetector( new THaBBTotalShower( "ts", "BB Total shower",pB));
  pB->AddDetector( new THaCherenkov("cer", "gas Cerenkov counter", pB ));

  ////////////////////////////////////////
  //   Other Modules
  ////////////////////////////////////////
  
  //tracking modules
// pB->MountOptics(new THaOpticsAnalytical("optics","Analytical Bigbite Optics model",pB,"BBurb"));
  pB->MountOptics(new THaOpticsE06010("optics","BigBite Optics Child Class",pB,"BBurb"));
  gHaPhysics->Add( new THaGoldenTrack("BB.gold","Golden track for Bigbite", "BB") );

  //target 
/*  cout<<"replay: Adding Target ..."<<endl;
  THaHe3Target* pT=new THaHe3Target("he3","Hall A Polarized he3 target");
  gHaApps->Add(pT);
*/
  //decoder data
  cout<<"replay: Adding Decoder Data ..."<<endl;
//  gHaApps->Add(new THaDecData("DL","Misc. Decoder Data"));

  THaApparatus* DECDAT = new THaDecData("DL","Misc. Decoder Data");
//  THaApparatus* DDECDAT = new THaDecData("DL","Misc. Decoder Data");
  gHaApps->Add( DECDAT );
//  gHaApps->Add( DDECDAT );
  gHaApps->Add(pB);

  cout<<"replay: Adding A Quick Hack Code to Extract t3[0] ..."<<endl;
  THaApparatus* DL1 = new THaIdealBeam("DL1","Base Apparatus for extract t3[0]");
  gHaApps->Add( DL1 );
  DL1->AddDetector(new THaADCHe3Spin("t3","copy of DL.t3[0], for .odef file",DL1));

  ////////////////////////////////////////
  //   Physics
  ////////////////////////////////////////

  cout<<"replay: adding Physics modules (BB) ..."<<endl;
  gHaPhysics->Add(new THaPrimaryKine("PriKineBB","kinematics of scattering of electron to BB","BB","BBurb",.939565));
  gHaPhysics->Add(new THaPrimaryKine("PriKineBBHe3","kinematics of scattering of electron to BB","BB","BBurb",3.016*0.931494));

/*  cout<<"replay: adding Physics modules (LHRS) ..."<<endl;
  gHaPhysics->Add(new THaPrimaryKine("PriKineL","kinematics of scattering of electron to HRS","L","urb",.939565));
  gHaPhysics->Add(new THaPrimaryKine("PriKineLHe3","kinematics of scattering of electron to HRS","L","urb",3.016*0.931494));
*/
/*  THaSecondaryKine *SecKinePion=new THaSecondaryKine(
                "SecKinePion","secondary kinematics of scattering pi into HRS",
                "ExTgtCor_L","PriKine", .13957018);
  gHaPhysics->Add(SecKinePion);
*/

  //for elastics, we detect the scattered proton using the LHRS
  THaSecondaryKine *SecKineP=new THaSecondaryKine(
                "SecKineP","secondary kinematics of scattering proton into LHRS",
                "ExTgtCor_L","PriKineBB", .938);
  gHaPhysics->Add(SecKineP); 
  
  ////////////////////////////////////////
  //   Scalars
  ////////////////////////////////////////
  // add scalers
  
  gHaScalers->Add(new THaScalerGroup("Left"));
  gHaScalers->Add(new THaScalerGroup("bbite"));
  gHaScalers->Add(new THaScalerGroup("evLeft"));
  gHaScalers->Add(new THaScalerGroup("evbbite"));

  ////////////////////////////////////////
  //   Do replay
  ////////////////////////////////////////

  
  ReplayCore(
	    runnumber,                      //run #
	    all,                            //-1=replay all;0=ask for a number
	    50000,                         //defaut replay event num
	    "%s/e06014_phys_coin_%d.root",  //output file format
	    "replay_phys_coin.odef",        //out define
	    "replay_test.cdef",    	    //empty cut define
	    kTRUE,                 	    //replay scalar?
	    fstEvt,			    //First Event To Replay
	    QuietRun		            //whether ask user for inputs
	    );

}
