//////////////////////////////////////////////////////////////////////////
//
//    THaHe3SpinModule
//
//  Basic Spin State Module
//  -1 = Spin AntiParallel to Holding Field;
//	0 = Undetermined ;
//	1 = Spin Parallel to Field.
//
//////////////////////////////////////////////////////////////////////////
//    
// Author: Jin Huang <mailto:jinhuang@jlab.org>    Oct 2008
//    Modify History:
//
//////////////////////////////////////////////////////////////////////////

#include "THaHe3SpinModule.h"

using namespace std;

//_____________________________________________________________________________
THaHe3SpinModule::THaHe3SpinModule()
{
	// Normal constructor.

	ClearEvent();
}

//_____________________________________________________________________________
THaHe3SpinModule::~THaHe3SpinModule()
{
	// Destructor

}

//_____________________________________________________________________________
const RVarDef* THaHe3SpinModule::GetRVarDef()
{
	// Get definition block of global variables for the fBeamIfo object

	static const RVarDef vars[] = {
		{ "Spin",	"Target Spin State Relative to Holding Field (+-1;0)",                    "fSpinState"},
		{ "IsSpinValid",    "Whether the spin state specified is valid",             "fSpinStateValid"},
		{ 0 }
	};

	return vars;
}


//_____________________________________________________________________________
//ClassImp(THaHe3SpinModule)
