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


#ifndef ROOT_THaHe3SpinModule
#define ROOT_THaHe3SpinModule


#include <TObject.h>
#include "VarDef.h"

class THaHe3SpinModule /*: private TObject*/ {

public:
	THaHe3SpinModule();  // public for ROOT I/O
	virtual ~THaHe3SpinModule();

	static const RVarDef* GetRVarDef();

	virtual void   ClearEvent( Option_t* opt = "" )
	{
		fSpinState=kSpinError;
		fSpinStateValid=kFALSE;
	}


	typedef enum  { kAntiPara = -1, kUnknown=0, kPara=1,kSpinError=1<<30 } EHe3Spin;//-1 = Spin AntiParallel to Holding Field; 0 = Undetermined ; 1 = Spin Parallel to Field.

	virtual EHe3Spin GetSpinState() const 
	{
		/*printf("--->GetSpinState=%d\n",fSpinState);*/
		return fSpinState;
	 }	
	virtual EHe3Spin SetSpinState(EHe3Spin state)
	{
		if (state==kSpinError) fSpinStateValid=kFALSE;
		return fSpinState=state;
	}

	virtual Bool_t GetSpinStateValidity() const 
	{
		/*printf("--->GetSpinStateValidity=%d\n",(Int_t)fSpinStateValid);*/
		return fSpinStateValid;
	}
	virtual Bool_t SetSpinStateValidity(Bool_t IsItValid)
	{
		if (!IsItValid) fSpinState=kSpinError;
		return fSpinStateValid=IsItValid;
	}

protected:

	EHe3Spin fSpinState; //-1 = Spin AntiParallel to Holding Field; 0 = Undetermined ; 1 = Spin Parallel to Field.
	Bool_t fSpinStateValid; //Whether the spin state specified is valid

	//ClassDef(THaHe3SpinModule,1)   // Basic Spin State Module

};

#endif
