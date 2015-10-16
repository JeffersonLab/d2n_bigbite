//////////////////////////////////////////////////////////////////////////
//
// Test Bigbite repaly
//   For shower detector test only !
//
//////////////////////////////////////////////////////////////////////////
//	
//	Author : Jin Huang (jinhuang@jlab.org)   May 2008
//
//////////////////////////////////////////////////////////////////////////


#include "def.h"

using namespace std;

void replay_BB(Int_t runnumber=0,Int_t all=0)
{

  cout<<"/nThis script only replay shower part of bigbite data"<<endl
      <<"To do full bigbite detector replay, use replay_det_BB()"<<endl<<endl;

  //Add BigBite 
  THaBigBite* pB=new THaBigBite("BB","BigBite"); 
  //THaBBTotalShower* ts=new THaBBTotalShower("ts","BigBite total shower");
  //pB->AddDetector(ts);
  pB->AddDetector( new THaScintPlaneDet( "s", "BB Scintillator",pB ));
  pB->AddDetector( new THaScintPlaneDet( "sum", "BB Total sum",pB));
  pB->AddDetector( new THaScintPlaneDet( "psum", "BB Preshower sum",pB));
  pB->AddDetector( new THaBBTotalShower( "ts", "BB Total shower",pB));

  //Decode miscellaneous things
  THaApparatus* DECDAT = new THaDecData("DL","Misc. Decoder Data");
  gHaApps->Add( DECDAT );

  gHaApps->Add(pB);


  ReplayCore(
	    runnumber,            //run #
	    all,                  //-1=replay all;0=ask for a number
	    -1,                   //defaut replay event num
	    "%s/e06010_BB_%d.root", //output file format
	    "replay_BB.odef",       //out define
	    "replay_test.cdef"        //cut define
	    );
}
