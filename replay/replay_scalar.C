//////////////////////////////////////////////////////////////////////////
//
// Transversity Scalar Replay Script
//
//////////////////////////////////////////////////////////////////////////
//  
// Nov 2008 : Jin Huang : Init the script with ref to detector replays
// Jan 2010 : David Flay : Edited for d2n
//
//////////////////////////////////////////////////////////////////////////



#include "def.h"

using namespace std;


void replay_scalar(Int_t runnumber=0,Int_t all=0,Int_t fstEvt=0,Bool_t QuietRun = kTRUE)
{


  ////////////////////////////////////////
  //   Beams
  ////////////////////////////////////////

  cout<<"replay: Adding Ideal Beam..."<< endl;
  THaApparatus* B = new THaIdealBeam("B","Idea Beam, for Test Only");
  gHaApps->Add( B );
  
  cout<<"replay: Adding L-arm Helicity..."<< endl;
  B->AddDetector( new THaADCHelicity("adchel.L","Beam helicity L-arm") );
  B->AddDetector( new THaADCHelicity("adchel2.L","Beam helicity-2 L-arm") );
  B->AddDetector( new THaG0Helicity("g0hel.L","Left arm G0 helicity") );  

  cout<<"replay: Adding Unrastered and Rastered Beam ..."<<endl;
  gHaApps->Add(new THaUnRasteredBeam("urb","Unrastered beam"));
  gHaApps->Add(new THaRasteredBeam("rb","Rastered Beam"));

  ////////////////////////////////////////
  //   Target
  //////////////////////////////////////// 
   
  cout<<"replay: Adding Target..."<<endl;
  THaHe3Target* pT=new THaHe3Target("he3","Hall A Polarized he3 target");
  gHaApps->Add(pT);

  ////////////////////////////////////////
  //   Decoder Data
  //////////////////////////////////////// 
  
  // Note: 'DL' tag applies to LHRS.
  cout<<"replay: Adding Decoder Data..."<<endl;
  gHaApps->Add(new THaDecData("DL","Misc. Decoder Data"));

  ////////////////////////////////////////
  //   Scalars
  ////////////////////////////////////////
  // add scalers
  
  cout<<"replay: Adding Scalar ..."<<endl;
  gHaScalers->Add(new THaScalerGroup("Left"));
  gHaScalers->Add(new THaScalerGroup("bbite"));
  
  gHaScalers->Add(new THaScalerGroup("evLeft"));
  gHaScalers->Add(new THaScalerGroup("evbbite"));

  ////////////////////////////////////////
  //   BB Norm Ana
  ////////////////////////////////////////

/*  char buff[1000]="";
  sprintf(buff,"./summaryfiles/NormAna%d.log",runnumber);
  
  cout<<"replay: Adding BBNormAna & saving result to "<<buff<<endl;
  BBNormAna* norm = new BBNormAna("N","Normalization Analysis",buff,8); 
  gHaPhysics->Add(norm);

  cout<<"replay: Adding a Quick Hack Code to Extract t3[0] ..."<<endl;
  THaApparatus* DL1 = new THaIdealBeam("DL1","Base Apparatus for extract t3[0]");
  gHaApps->Add( DL1 );
  DL1->AddDetector(new THaADCHe3Spin("t3","copy of DL.t3[0], for .odef file",DL1));
*/

  ////////////////////////////////////////
  //   Do replay
  ////////////////////////////////////////
  THaAnalyzer* analyzer = THaAnalyzer::GetInstance();
  if( !analyzer ) {
  analyzer = new THaAnalyzer;
  }
  //analyzer->EnableBenchmarks();
  analyzer->EnableHelicity();
  analyzer->EnableScalers();
  analyzer->EnableSlowControl();
  analyzer->SetMarkInterval(20000);

  
  ReplayCore(
      runnumber,                          //run #
      all,                                //-1=replay all;0=ask for a number
      -1,                                 //defaut replay event num
      "%s/e06014_scalar_%d.root",         //output file format
      "replay_scalar.odef",               //out define
      "replay_test.cdef",                 //empty cut define
      kTRUE,                              //replay scalar?
      fstEvt,                             //First Event To Replay
	  QuietRun
      );

  analyzer->SetMarkInterval(1000);
}
