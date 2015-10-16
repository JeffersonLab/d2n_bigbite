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

void replay_target(Int_t runnumber=0,Int_t all=0)
{


  THaHe3Target* pT=new THaHe3Target("he3","Hall A Polarized he3 target"); 
  //THaBBTotalShower* ts=new THaBBTotalShower("ts","BigBite total shower");
  //pB->AddDetector(ts);
  //pT->AddDetector( new THaADCHe3Spin( "la1", "Left HRS Target Spin Signal",pT ));
  //pT->AddDetector( new THaADCHe3Spin( "bb", "BigBite Target Spin Signal",pT ));
  gHaApps->Add(pT);


  ReplayCore(
	    runnumber,            //run #
	    all,                  //-1=replay all;0=ask for a number
	    -1,                   //defaut replay event num
	    "%s/e06010_target_%d.root", //output file format
	    "replay_test.odef",       //out define
	    "replay_test.cdef"        //cut define
	    );
}
