//////////////////////////////////////////////////////////////////////////
//
//    THaBitHe3Spin
//
//  Read target state from CAMAC controller control bit input 
//  1 channel as spin + another as spin -
//
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
#include <stdlib.h>
#include "VarDef.h"

#include "THaBitHe3Spin.h"

using namespace std;

//____________________________________________________________________
THaBitHe3Spin::THaBitHe3Spin( const char* name, const char* description,
							 THaApparatus* app ):
THaDetector(name,description,app)
{
	// Constructor

	fPermInvalidEvt=DEFAULT_PERM_INVALID_EVT;
	fMissingEvtCount=0;

	fSpinWordHeader=0;
	fSpinUpMask=0;
	fSpinDownMask=0;
	fROC=2000;
	fSpinWordOffSet=1;
	fRawWordConverter=0x0;

	Clear("C");

}

//____________________________________________________________________
THaBitHe3Spin::~THaBitHe3Spin() 
{
	DefineVariables( kDelete );
}


//_____________________________________________________________________________
Int_t THaBitHe3Spin::DefineVariables( EMode mode )
{
	// Initialize global variables

	if( mode == kDefine && fIsSetup ) return kOK;
	THaDetector::DefineVariables( mode );

	const RVarDef var[] = {
		{ "RawWord",		"raw data word",		"fRawWord" },
		{ 0 }
	};

	return DefineVarsFromList( var, mode )
		+DefineVarsFromList(THaHe3SpinModule::GetRVarDef(), mode );
}


//____________________________________________________________________
Int_t THaBitHe3Spin::ReadDatabase( const TDatime& date )
{

	Int_t err = THaDetector::ReadDatabase( date );
	if( err )
		return err;

	FILE* file = OpenFile( date );
	if( !file ) return kFileError;

	string spinwordheader, spinupmask, spindownmask,rawwordconverter("0x0");

	const  DBRequest request[] = {
		{ "PermInvalidEvt", &fPermInvalidEvt,	kInt,		0, 1, -1 },
		{ "SpinWordOffSet",	&fSpinWordOffSet,	kUInt,		0, 1, -1 },
		{ "ROC",			&fROC,				kUInt,		0, 0, -1 },
		{ "SpinWordHeader",	&spinwordheader,	kString,	0, 0, -1 },
		{ "SpinWordXorConverter",&rawwordconverter,	kString,0, 1, -1 },
		{ "SpinUpMask",		&spinupmask,		kString,	0, 0, -1 },
		{ "SpinDownMask",	&spindownmask,		kString,	0, 0, -1 },
		{ 0 }
	};
	// Database keys are prefixed with this detector's name, not apparatus.name
	err = LoadDB( file, date, request, fPrefix );
	fclose(file);
	if( err )
		return kInitError;

	fSpinWordHeader=strtoul(spinwordheader.c_str(),NULL,16);
	if (!fSpinWordHeader)
	{
		Warning(Here("ReadRunDatabase"),
			"No Invalid database setting for SpinWordHeader");
	}

	fSpinUpMask=strtoul(spinupmask.c_str(),NULL,16);
	if (!fSpinUpMask)
	{
		Warning(Here("ReadRunDatabase"),
			"No Invalid database setting for SpinUpMask");
	}

	fSpinDownMask=strtoul(spindownmask.c_str(),NULL,16);
	if (!fSpinDownMask)
	{
		Warning(Here("ReadRunDatabase"),
			"No Invalid database setting for SpinDownMask");
	}

	fRawWordConverter=strtoul(rawwordconverter.c_str(),NULL,16);


	DEBUG_INFO(Here("ReadRunDatabase"),"\n"
		"fPermInvalidEvt=%d""\n"
		"fSpinWordHeader=%8X""\n"
		"fSpinUpMask=%8X""\n"
		"fSpinDownMask=%8X""\n"
		"fROC=%d""\n"
		"SpinWordOffSet=%d",
		fPermInvalidEvt, fSpinWordHeader, fSpinUpMask, fSpinDownMask,fROC,fSpinWordOffSet);

	fIsInit = true;
	return kOK;
}

//____________________________________________________________________
void THaBitHe3Spin::Clear( Option_t* opt ) 
{
	// Clear the event data

	THaHe3SpinModule::ClearEvent(opt) ;
	THaDetector::Clear( opt ) ;
	fRawWord=0x0;
}



//____________________________________________________________________
Int_t THaBitHe3Spin::Decode( const THaEvData& evdata )
{
	// Decode Helicity data.
	// Return 1 if helicity was assigned, 0 if not, -1 if error.

	// Only the first two channels defined in the detector map are used
	// here, regardless of how they are defined (consecutive channels
	// in same module or otherwise). ReadDatabase guarantees that two channels
	// are present and warns about extra channels.

	DEBUG_MASSINFO(Here("Decode"),
		"Enter Decode Section, fIsInit=%d",(Int_t)fIsInit);
	if( !fIsInit )
		return -1;

	Int_t ret = 0;

#if DEBUG_LEVEL>=5
	for( Int_t i = 0; i < evdata.GetRocLength(fROC); i++ ) {
		Info(Here("Decode"),"Word %d = %8X ", i,
			static_cast<UInt_t>( evdata.GetRawData(fROC,i) ));
	}
#endif

	fRawWord=NULL_RAW_WORD;
	Bool_t found=kFALSE;
	for( Int_t i = 0; i < evdata.GetRocLength(fROC); i++ ) {
		if (static_cast<UInt_t>( evdata.GetRawData(fROC,i))==fSpinWordHeader)
		{
			if ( i+ (Int_t)fSpinWordOffSet < evdata.GetRocLength(fROC))
			{
				fRawWord=
					static_cast<UInt_t>( 
					evdata.GetRawData(fROC,i+fSpinWordOffSet));
				found=kTRUE;
				break;
			}
		}
	}
	DEBUG_INFO(Here("Decode"),"fRawWord=%8X",fRawWord);

	Bool_t spinup=(fRawWord^fRawWordConverter)&fSpinUpMask;
	Bool_t spindown=(fRawWord^fRawWordConverter)&fSpinDownMask;

	if (!found || (spinup && spindown))		
	{//ignore missing ADC if it's not too frequent
		fMissingEvtCount++;
		if (fMissingEvtCount<=fPermInvalidEvt) 
		{//complain and ignore event
			DEBUG_INFO(Here("Decode"),
				"Missing %d Bit Spin State Signal for %s @ evt #%d (fRawWord=%X)",
				fMissingEvtCount, GetPrefix() ,
				evdata.GetEvNum(),fRawWord);
			return ret; 
		}
	}
	else fMissingEvtCount=0;

	SetSpinState((THaHe3SpinModule::EHe3Spin)(
		spinup*THaHe3SpinModule::kPara
		+spindown*THaHe3SpinModule::kAntiPara));
	SetSpinStateValidity(!(spinup==1&&spindown==1)&&found);

	if (!GetSpinStateValidity()) 
		DEBUG_INFO(Here("Decode"),
		"Invalid Bit Spin State Signal for %s @ evt #%d (fRawWord=%X)",
		GetPrefix() ,evdata.GetEvNum(),fRawWord);

	return ret;
}


ClassImp(THaBitHe3Spin)

