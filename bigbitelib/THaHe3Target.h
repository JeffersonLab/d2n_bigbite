//////////////////////////////////////////////////////////////////////////
//
//    THaHe3Target
//
//  ABC for an apparatus providing target information
//
//////////////////////////////////////////////////////////////////////////
//    
// Author: Jin Huang <mailto:jinhuang@jlab.org>    Oct 2008
//    Modify History:
//
//////////////////////////////////////////////////////////////////////////


#ifndef ROOT_THaHe3Target
#define ROOT_THaHe3Target

#include "TTree.h"
#include "THaApparatus.h"
#include "THaHe3SpinModule.h"
#include "THaTrackingModule.h"
#include "VarDef.h"
#include <vector>

using namespace std;
//------------------------------------------------------//
//
//    Debug Definitions
//    place this section below any other head files
//
//------------------------------------------------------//
#ifdef DEBUG_LEVEL
#   undef DEBUG_LEVEL
#endif

//    DEBUG_LEVEL;    
//    =0    or not define: no debug, full speed 
//    >=1    enable debug extra warning (suggested setting)
//    >=2    above + enable debug assert
//    >=3    above + enable debug extra info
//    >=4    above + massive info (in a for or while)
#define DEBUG_LEVEL   2

#include    "DebugDef.h"
//------------------------------------------------------//

class THaHe3Target :
					public THaApparatus,
	 				public THaHe3SpinModule,
  					public THaTrackingModule //to mimic a spectrometer, so THaSecondaryKine could be used to calculate angles
{

public:
	THaHe3Target( const char* name, const char* description ) ;
	virtual ~THaHe3Target();

	virtual EStatus	Init( const TDatime& run_time );
	virtual Int_t	End( THaRunBase* run );

	Double_t GetPolarization() {return fPolarization;}
	Double_t GetTheta() {return fTheta;}
	Double_t GetPhi() {return fPhi;}

protected:
	virtual Int_t	ReadDatabase( const TDatime& date );
	virtual Int_t	Reconstruct();
	virtual Int_t	DefineVariables( EMode mode = kDefine );

	vector<THaHe3SpinModule *>	fSpinModule;
	//TTree *fTree;

	Double_t fPolarization;	//He3 Polarizaion in percent
	Double_t fTheta;		//Polarizaion Direction in rad - Theta: Diviation of spin + vector from vertical up
	Double_t fPhi;			//Polarizaion Direction in rad - Phi: Angle of spin + vector projected to horizonal plane; Downstream is 0, Trans+ = 90
	Double_t fFieldTheta;		//Field Direction in rad 
	Double_t fFieldPhi;			//Field Direction in rad 

	ClassDef(THaHe3Target,1)    // ABC for hall A polarized he3 target
};

#endif

