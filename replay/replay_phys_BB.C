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

void replay_phys_BB(Int_t runnumber=0,Int_t all=0,Int_t fstEvt=0,Bool_t QuietRun = kTRUE)
{

//****************************************************************************************************
//Add Beams/Helicity
//****************************************************************************************************
	THaApparatus* B = new THaIdealBeam("B","Idea Beam, for Test Only");
	gHaApps->Add( B );
        cout<<"replay: Adding BigBite Helicity"<< endl;
        B->AddDetector( new THaADCHelicity("adchel.BB","Beam helicity BB") );
	B->AddDetector( new THaADCHelicity("adchel2.BB","Beam helicity BB") );

	gHaApps->Add(new THaUnRasteredBeam("BBurb","BB Unrastered beam"));
	gHaApps->Add(new THaRasteredBeam("BBrb","BB Rastered Beam"));
        gHaApps->Add(new THaUnRasteredBeam("urb","HRS Unrastered beam"));
        gHaApps->Add(new THaRasteredBeam("rb","HRS Rastered Beam"));
  
//	pB->MountOptics(new THaOpticsAnalytical("optics","Analytical Bigbite Optics model",pB,"BBurb"));


//************************************************************************************************
//Add BigBite 
//************************************************************************************************
	THaBigBite* pB=new THaBigBite("BB","BigBite"); 
	pB->AddDetector(new TreeSearch::MWDC("mwdc","MWDC",pB));
	pB->AddDetector( new THaScintPlaneDet( "s", "BB Scintillator",pB ));
	pB->AddDetector( new THaScintPlaneDet( "sum", "BB Total sum",pB));
	pB->AddDetector( new THaScintPlaneDet( "psum", "BB Preshower sum",pB));
	pB->AddDetector( new THaBBTotalShower( "ts", "BB Total shower",pB));
	pB->AddDetector( new THaCherenkov("cer", "gas Cerenkov counter", pB ));
        pB->MountOptics(new THaOpticsE06010("optics","BigBite Optics Child Class",pB,"BBurb"));
	gHaPhysics->Add( new THaGoldenTrack("BB.gold","Golden track for Bigbite", "BB") );


//************************************************************************************************
//Decode miscellaneous things 
//*************************************************************************************************

	THaApparatus* DECDAT = new THaDecData("DBB","Misc. Decoder Data");
        //THaApparatus* DECDAT = new BBDecData("DBB","Misc. Decoder Data");
	gHaApps->Add( DECDAT );
	gHaApps->Add(pB);
  
  
////////////////////////////////////////
//   Physics
////////////////////////////////////////

		cout<<"replay: adding Physics modules ..."<<endl;
          THaPrimaryKine *PriKineBB=new THaPrimaryKine("PriKineBB","kinematics of scattering electron of BB","BB","BBurb");
          gHaPhysics->Add(PriKineBB);
//		gHaPhysics->Add(new THaPrimaryKine("PriKineBB","kinematics of scattering of electron to BB","BB","BBurb",2.0*.939565));
//		gHaPhysics->Add(new THaPrimaryKine("PriKineBBHe3","kinematics of scattering of electron to BB","BB","BBurb",3.016*0.931494));
    
////////////////////////////////////////
//   Scalars
////////////////////////////////////////

		cout<<"replay: Adding Scalars..."<<endl;
		gHaScalers->Add(new THaScalerGroup("Left"));
        	gHaScalers->Add(new THaScalerGroup("bbite"));
		gHaScalers->Add(new THaScalerGroup("evLeft"));
		gHaScalers->Add(new THaScalerGroup("evbbite"));

////////////////////////////////////////
//   BB Norm Ana
////////////////////////////////////////
//	char buff[1000]="";
//	sprintf(buff,"./summaryfiles/BBNormAna%d.log",runnumber);

//	cout<<"replay: Adding BBNormAna & saving result to "<<buff<<endl;
//	BBNormAna* norm = new BBNormAna("N","Normalization Analysis",buff,8);
//	gHaPhysics->Add(norm);
 
////////////////////////////////////////
//   Do replay
////////////////////////////////////////
//	THaAnalyzer* analyzer = THaAnalyzer::GetInstance();
//	if( !analyzer ) {
//		analyzer = new THaAnalyzer;
//	}
//	analyzer->EnableBenchmarks();
//	analyzer->EnableHelicity();
//	analyzer->EnableScalers();
//      analyzer->EnableSlowControl();
//        analyzer->SetMarkInterval(20000);


  
	ReplayCore(
			runnumber,            //run #
	all,                  //-1=replay all;0=ask for a number
	50000,                   //defaut replay event num
	"%s/e06014_phys_BB_trig_%d.root", //output file format
	"replay_phys_BB.odef",       //out define
	"replay_phys_BB.cdef",       //cut define
	kTRUE,//Enable Scalar?
	fstEvt,					//First Event To Replay
	QuietRun				//whether ask user for inputs
			  );
}
