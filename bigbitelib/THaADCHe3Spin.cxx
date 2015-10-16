//////////////////////////////////////////////////////////////////////////
//
//    THaADCHe3Spin
//
//  Read target state from a ADC channel 
//  1 channel as spin + another as spin -
//
//////////////////////////////////////////////////////////////////////////
//    
// Author: Jin Huang <mailto:jinhuang@jlab.org>    Oct 2008
//    Modify History:
//
//////////////////////////////////////////////////////////////////////////




#include "THaEvData.h"
#include "THaDetMap.h"
#include <string.h>
#include <iostream>
#include <vector>
#include "VarDef.h"

#include "THaADCHe3Spin.h"

using namespace std;

//____________________________________________________________________
THaADCHe3Spin::THaADCHe3Spin( const char* name, const char* description,
							 THaApparatus* app ):
THaDetector(name,description,app)
{
	// Constructor

	fLogic1Window.UpBound=fLogic1Window.LowBound=-1;
	fLogic0Window.UpBound=fLogic0Window.LowBound=-1;
	fPermInvalidADC=DEFAULT_PERM_INVALID_ADC;
	fMissingADCCount=0;

	Clear("C");

}

//____________________________________________________________________
THaADCHe3Spin::~THaADCHe3Spin() 
{
	DefineVariables( kDelete );
}


//_____________________________________________________________________________
Int_t THaADCHe3Spin::DefineVariables( EMode mode )
{
	// Initialize global variables

	if( mode == kDefine && fIsSetup ) return kOK;
	THaDetector::DefineVariables( mode );

	const RVarDef var[] = {
		{ "UpADC",		"ADC raw data for spin up channel",		"fADCRawUp" },
		{ "DownADC",	"ADC raw data for spin down channel",	"fADCRawDown" },
		{ 0 }
	};

	return DefineVarsFromList( var, mode )
		+DefineVarsFromList(THaHe3SpinModule::GetRVarDef(), mode );
}


//____________________________________________________________________
Int_t THaADCHe3Spin::ReadDatabase( const TDatime& date )
{

	static const char* const here = "ReadDatabase";

	Int_t err = THaDetector::ReadDatabase( date );
	if( err )
		return err;

	FILE* file = OpenFile( date );
	if( !file ) return kFileError;

	vector<Int_t> spinupchan, spindownchan;
	vector<Double_t> logic1window, logic0window;
	 
	const  DBRequest request[] = {
		{ "SpinUpChan",      &spinupchan,		kIntV,		0, 0, -1 },
		{ "SpinDownChan",    &spindownchan,		kIntV,		0, 0, -1 },
		{ "ADCLogic1Window",    &logic1window,	kDoubleV,	0, 0, -1 },
		{ "ADCLogic0Window",  &logic0window,	kDoubleV,	0, 0, -1 },
		{ "PermInvalidEvt",  &fPermInvalidADC,	kInt,		0, 1, -1 },
		{ 0 }
	};
	// Database keys are prefixed with this detector's name, not apparatus.name
	err = LoadDB( file, date, request, fPrefix );
	fclose(file);
	if( err )
		return kInitError;

	if( spinupchan.size() != 3 ) {
		Error( Here(here), "Incorrect defintion of SpinUpChan. Must be "
			"exactly 3 numbers (roc,slot,chan), found %d. Fix database.", 
			spinupchan.size() );
		return kInitError;
	}
	if( spindownchan.size() != 3 ) {
		Error( Here(here), "Incorrect defintion of SpinDownChan. Must be "
			"exactly 3 numbers (roc,slot,chan), found %d. Fix database.", 
			spindownchan.size() );
		return kInitError;
	}
	if( logic1window.size() != 2 ) {
		Error( Here(here), "Incorrect defintion of ADCLogic1Window. Must be "
			"exactly 3 numbers (roc,slot,chan), found %d. Fix database.", 
			logic1window.size() );
		return kInitError;
	}
	if( logic0window.size() != 2 ) {
		Error( Here(here), "Incorrect defintion of ADCLogic0Window. Must be "
			"exactly 3 numbers (roc,slot,chan), found %d. Fix database.", 
			logic0window.size() );
		return kInitError;
	}
	//// Missing gate channel implies ignoring gate unless explicitly set
	//if( gatedef.empty() ) {
	//	if( ignore_gate < 0 )
	//		fIgnoreGate = kTRUE;
	//	else {
	//		Error( Here(here), "Missing gate data channel definition gatechan. "
	//			"Fix database." );
	//		return kInitError;
	//	}
	//}
	//if( !gatedef.empty() && gatedef.size() != 3 ) {
	//	Error( Here(here), "Incorrect defintion of gate data channel. Must be "
	//		"exactly 3 numbers (roc,slot,chan), found %d. Fix database.", 
	//		gatedef.size() );
	//	return kInitError;
	//}


	memcpy( &fSpinUpChan, &spinupchan[0], sizeof(ChanDef_t) );
	memcpy( &fSpinDownChan, &spindownchan[0], sizeof(ChanDef_t) );
	memcpy( &fLogic1Window, &logic1window[0], sizeof(ADCStateBound) );
	memcpy( &fLogic0Window, &logic0window[0], sizeof(ADCStateBound) );

	if (fLogic1Window.UpBound< fLogic1Window.LowBound )
	{
		Double_t tmp=fLogic1Window.UpBound;
		fLogic1Window.UpBound=fLogic1Window.LowBound;
		fLogic1Window.LowBound=tmp;
	}
	if (fLogic0Window.UpBound< fLogic0Window.LowBound )
	{
		Double_t tmp=fLogic0Window.UpBound;
		fLogic0Window.UpBound=fLogic0Window.LowBound;
		fLogic0Window.LowBound=tmp;
	}

#if (DEBUG_LEVEL>=1)
	//window size check
	if (
		(
		fLogic1Window.LowBound<=fLogic0Window.UpBound 
		&& fLogic1Window.UpBound>=fLogic0Window.UpBound
		)
		|| 
		(
		fLogic1Window.LowBound<=fLogic0Window.LowBound 
		&& fLogic1Window.UpBound>=fLogic0Window.LowBound
		)
		)
		Warning(Here("ReadDatabase"),
		"ADC Window Overlapped for logic state 0 and 1\n"
		"Check %s.ADCLogic1Window and %s.ADCLogic0Window setting",
		GetPrefix(),GetPrefix());
#endif

#if DEBUG_LEVEL>=3//start show info
	TString sDebugOutput;
	sDebugOutput=GetName();
	sDebugOutput+=" ReadRunDatabase finish in successfully with:";

	sDebugOutput+="\n \t fPermInvalidADC\t= ";
	sDebugOutput+=fPermInvalidADC;

	sDebugOutput+="\n \t fSpinUpChan\t= ";
	sDebugOutput+=fSpinUpChan.roc; sDebugOutput+=" : ";
	sDebugOutput+=fSpinUpChan.slot; sDebugOutput+=" : ";
	sDebugOutput+=fSpinUpChan.chan;

	sDebugOutput+="\n \t fSpinDownChan\t= ";
	sDebugOutput+=fSpinDownChan.roc; sDebugOutput+=" : ";
	sDebugOutput+=fSpinDownChan.slot; sDebugOutput+=" : ";
	sDebugOutput+=fSpinDownChan.chan;

	sDebugOutput+="\n \t fLogic1Window\t= ";
	sDebugOutput+=fLogic1Window.LowBound; sDebugOutput+=" -> ";
	sDebugOutput+=fLogic1Window.UpBound;

	sDebugOutput+="\n \t fLogic0Window\t= ";
	sDebugOutput+=fLogic0Window.LowBound; sDebugOutput+=" -> ";
	sDebugOutput+=fLogic0Window.UpBound;

	sDebugOutput+="\n";
	Info(Here("ReadRunDatabase()"),sDebugOutput.Data());
#endif  

	fIsInit = true;
	return kOK;
}

//____________________________________________________________________
void THaADCHe3Spin::Clear( Option_t* opt ) 
{
	// Clear the event data

	THaHe3SpinModule::ClearEvent(opt) ;
	THaDetector::Clear( opt ) ;
	fADCRawUp = kBig;
	fADCRawDown  = kBig;
}



Double_t THaADCHe3Spin::GetData(const ChanDef_t &c,const THaEvData& evdata )
{
	Int_t roc = c.roc, slot = c.slot, chan = c.chan;

	if ( !evdata.GetNumHits( roc, slot, chan ))
	{
		DEBUG_MASSINFO(Here("GetData"),"No hit for roc=%d, slot=%d, chan=%d ",
			roc, slot, chan);

		return kBig;
	}
	else
	{
		Double_t data = 
			static_cast<Double_t>(evdata.GetData( roc, slot, chan, 0 ));

		DEBUG_MASSINFO(Here("GetData"),"Get Data %f for roc=%d, slot=%d, chan=%d ",
			data, roc, slot, chan);

		return data;
	}
}

//____________________________________________________________________
Int_t THaADCHe3Spin::Decode( const THaEvData& evdata )
{
	// Decode Helicity data.
	// Return 1 if helicity was assigned, 0 if not, -1 if error.

	// Only the first two channels defined in the detector map are used
	// here, regardless of how they are defined (consecutive channels
	// in same module or otherwise). ReadDatabase guarantees that two channels
	// are present and warns about extra channels.

	DEBUG_MASSINFO(Here("Decode"),"Enter Decode Section, fIsInit=%d",(Int_t)fIsInit);
	if( !fIsInit )
		return -1;

	Int_t ret = 0;
	

	fADCRawUp=GetData(fSpinUpChan,evdata);
	fADCRawDown=GetData(fSpinDownChan,evdata);

	if (fADCRawUp==kBig || fADCRawDown==kBig)		
	{//ignore missing ADC if it's not too frequent
		fMissingADCCount++;
		if (fMissingADCCount<=fPermInvalidADC) 
		{//complain and ignore event
			DEBUG_INFO(Here("Decode"),
				"Missing %d ADC Spin State Signal for %s @ event #%d"
				"Spin Up ADC = %f ; Spin Down ADC = %f",
				fMissingADCCount, GetPrefix() ,
				evdata.GetEvNum(),fADCRawUp,fADCRawDown );

			return ret; 
		}
	}
	else fMissingADCCount=0;

	Int_t up=ADC2Logic(fADCRawUp);
	Int_t down=ADC2Logic(fADCRawDown);

	SetSpinState((THaHe3SpinModule::EHe3Spin)(
		up*THaHe3SpinModule::kPara
		+down*THaHe3SpinModule::kAntiPara));
	SetSpinStateValidity(
		!(
		up<0 
		|| down<0
		|| (up==1&&down==1)
		));

	if (!GetSpinStateValidity()) 
		DEBUG_WARNING(Here("Decode"),
		"Invalid Spin State Signal for %s @ event #%d\n"
		"Spin Up ADC = %f ; Spin Down ADC = %f",
		GetPrefix() ,evdata.GetEvNum(),fADCRawUp,fADCRawDown );

	return ret;
}


ClassImp(THaADCHe3Spin)

