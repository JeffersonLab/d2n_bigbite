//////////////////////////////////////////////////////////////////////////
//
// Transversity Semi-inclusive Asymetry Replay Script
//
//////////////////////////////////////////////////////////////////////////
//	
// Nov 2008 : Jin Huang : Init the script with ref to detector replays
//
//////////////////////////////////////////////////////////////////////////



#include "def.h"

using namespace std;


void replay_asym(Int_t runnumber=0,Int_t all=0,Int_t fstEvt=0,Bool_t QuietRun = kTRUE)
{


  ////////////////////////////////////////
  //   Beams
  ////////////////////////////////////////

  cout<<"replay: Adding Idea Beam"<< endl;

  THaApparatus* B = new THaIdealBeam("B","Idea Beam, for Test Only");
  gHaApps->Add( B );
  
  cout<<"replay: Adding L-arm Helicity"<< endl;
  B->AddDetector( new THaADCHelicity("adchel.L","Beam helicity L-arm") );
  B->AddDetector( new THaADCHelicity("adchel2.L","Beam helicity-2 L-arm") );
  B->AddDetector( new THaG0Helicity("g0hel.L","Left arm G0 helicity") );	
    cout<<"replay: Adding UnRastered and Rastered Beam ..."<<endl;
  gHaApps->Add(new THaUnRasteredBeam("urb","Unrastered beam"));
//   gHaApps->Add(new THaRasteredBeam("rb","Rastered Beam"));
  
  

  ////////////////////////////////////////
  //   LHRS
  ////////////////////////////////////////

  cout<<"replay: Adding HRS-L ..."<<endl;
  THaApparatus* HRSL = new THaHRS("L","Left HRS");
  gHaApps->Add( HRSL );
  
  // add L detectors that are not in the default config
  HRSL->AddDetector( new THaCherenkov("cer", "Gas Cherenkov counter" ));
  HRSL->AddDetector( new THaCherenkov("a1", "A1 Cherenkov counter" ));
  HRSL->AddDetector( new THaShower("prl1", "Pre-shower pion rej." ));
  HRSL->AddDetector( new THaShower("prl2", "Show pion rej." ));

  //cout<<"replay: adding RICH detector ..."<<endl;
  //HRSL->AddDetector( new THaRICH("rich","The RICH")); 
  

  cout<<"replay: adding LHRS track modules ..."<<endl;

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
  //Add BigBite 
  THaBigBite* pB=new THaBigBite("BB","BigBite"); 
  gHaApps->Add(pB);
  //THaBBTotalShower* ts=new THaBBTotalShower("ts","BigBite total shower");
  //pB->AddDetector(ts);
  pB->AddDetector(new TreeSearch::MWDC("mwdc","MWDC",pB));
  pB->AddDetector( new THaScintPlaneDet( "s", "BB Scintillator",pB ));
  pB->AddDetector( new THaScintPlaneDet( "sum", "BB Total sum",pB));
  pB->AddDetector( new THaScintPlaneDet( "psum", "BB Preshower sum",pB));
  pB->AddDetector( new THaBBTotalShower( "ts", "BB Total shower",pB));

  cout<<"replay: adding LHRS track modules ..."<<endl;
  pB->MountOptics(new THaOpticsE06010("optics","BigBite Optics Child Class",pB,"urb"));


  ////////////////////////////////////////
  //   Physics
  ////////////////////////////////////////

  cout<<"replay: adding Physics modules ..."<<endl;
  THaPrimaryKine *PriKine=new THaPrimaryKine("PriKine","kinematics of scattering of electron to BB","BB","urb",.939565);
  gHaPhysics->Add(PriKine);

  THaSecondaryKine *SecKinePion=new THaSecondaryKine(
                "SecKinePion","secondary kinematics of scattering pi into HRS",
                "ExTgtCor_L","PriKine", .13957018);
  gHaPhysics->Add(SecKinePion);

  
  ////////////////////////////////////////
  //   Other Modules
  ////////////////////////////////////////

  
  cout<<"replay: Adding Target ..."<<endl;
  THaHe3Target* pT=new THaHe3Target("he3","Hall A Polarized he3 target");
  gHaApps->Add(pT);
  THaSecondaryKine *SecKineHe3=new THaSecondaryKine(
		  "SecKineHe3","Calculate angles of polarized he3 VS electron scattering",
	"he3","PriKine", 0);
  gHaPhysics->Add(SecKineHe3);

  cout<<"replay: Adding Decoder Data ..."<<endl;
  gHaApps->Add(new THaDecData("DL","Misc. Decoder Data"));

  
  
  ////////////////////////////////////////
  //   Scalars
  ////////////////////////////////////////
  // add scalers
  
  gHaScalers->Add(new THaScalerGroup("Left"));
  gHaScalers->Add(new THaScalerGroup("bbite"));
  gHaScalers->Add(new THaScalerGroup("evLeft"));
  gHaScalers->Add(new THaScalerGroup("evbbite"));

  ////////////////////////////////////////
  //   BB Norm Ana
  ////////////////////////////////////////
  char buff[1000]="";
  sprintf(buff,"./summaryfiles/NormAna%d.log",runnumber);
  
  cout<<"replay: Adding BBNormAna & saving result to "<<buff<<endl;
  BBNormAna* norm = new BBNormAna("N","Normalization Analysis",buff,8); 
  gHaPhysics->Add(norm);
  
  ////////////////////////////////////////
  //   Do replay
  ////////////////////////////////////////
  THaAnalyzer* analyzer = THaAnalyzer::GetInstance();
  if( !analyzer ) {
  analyzer = new THaAnalyzer;
  }
  analyzer->EnableBenchmarks();
  analyzer->EnableHelicity();
  analyzer->EnableScalers();
  //analyzer->EnableSlowControl();
  analyzer->SetMarkInterval(20000);

  
  ReplayCore(
	    runnumber,            //run #
	    all,                  //-1=replay all;0=ask for a number
	    -1,                   //defaut replay event num
	    "%s/e06010_asym_%d.root", //output file format
	    "replay_asym.odef",    	//out define
	    "replay_asym.cdef",    	//empty cut define
	    kTRUE,                 	//replay scalar?
	    fstEvt,					//First Event To Replay
		QuietRun				//whether ask user for inputs
	    );
  analyzer->SetMarkInterval(1000);

}
