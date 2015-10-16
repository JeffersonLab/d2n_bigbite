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




#include "THaRunBase.h"
#include "THaRunParameters.h"
#include "THaGlobals.h"
#include "THaDetector.h"
#include "TList.h"
#include "THaADCHe3Spin.h"
#include "THaBitHe3Spin.h"
#include <string>
#include <TMath.h>

#include "THaHe3Target.h"
typedef string::size_type ssiz_t;
// #using namespace std;
//_____________________________________________________________________________
THaHe3Target::THaHe3Target( const char* name, const char* desc ) : 
THaApparatus( name,desc )
{
	// Constructor.
	// Protected. Can only be called by derived classes.
	//fTree=0;

	DEBUG_LEVEL_RELATED_PERFORMACE_CHECKER;
	DEBUG_HALL_A_ANALYZER_DEBUGER_INIT;
}


//_____________________________________________________________________________
THaHe3Target::~THaHe3Target()
{
	// Destructor
	//if (fTree) delete fTree;

	RemoveVariables();
}

//____________________________________________________________________
Int_t THaHe3Target::ReadDatabase( const TDatime& date )
{
	DEBUG_INFO(Here("ReadDatabase"),"Start ReadDatabase");

	string adcspinlist,bitspinlist;
	fPolarization = 0;	//He3 Polarizaion in percent
	fTheta = fFieldTheta = 0;		//Polarizaion Direction - Theta: Diviation of spin + vector from vertical up
	fPhi = fFieldPhi = 0;			//Polarizaion Direction - Phi: Angle of spin + vector projected to horizonal plane; Downstream is 0, Trans+ = 90
	
	DBRequest request[] = {
		{ "ADCSpinList",       &adcspinlist,       kString,	0,	1},
		  { "BitSpinList",       &bitspinlist,       kString,	0,	1},
	{ "Polarization",       &fPolarization,       kDouble,	0,	1},
 { "Theta",       &fFieldTheta,       kDouble,	0,	1},
 { "Phi",       &fFieldPhi,       kDouble,	0,	1},
		{ 0 }
	};
	
	FILE* file = OpenFile( date );
	if( !file ) return kFileError;
	LoadDB( file, date, request, fPrefix );
	fclose(file);
	
	DEBUG_INFO(Here("ReadDatabase"),"Polarization=%f %%; Theta=%f; Phi=%f",
			   fPolarization,fFieldTheta,fFieldPhi);
	fFieldTheta	=fFieldTheta/180*TMath::Pi();
	fFieldPhi	=fFieldPhi/180*TMath::Pi();
	fTheta	= 0;
	fPhi	= 0;

	TrkIfoClear();
	//Set(Double_t p, Double_t dp, Double_t x, Double_t y, Double_t th, Double_t ph, Double_t px, Double_t py, Double_t pz)
	fTrkIfo.Set(fPolarization/100,0,0,0,0,0,
				fPolarization/100*TMath::Sin(fTheta)*TMath::Sin(fPhi),
				fPolarization/100*TMath::Cos(fTheta),
				fPolarization/100*TMath::Sin(fTheta)*TMath::Cos(fPhi)
			   );

	vector<string> adcs = vsplit(adcspinlist);
	if (adcs.size()>0)
		Info(Here("ReadDatabase"),"to Load %d copy of THaADCHe3Spin : '%s'",
		adcs.size(),adcspinlist.c_str());
	for( ssiz_t i=0; i<adcs.size(); ++i ) {
		assert( !adcs[i].empty() );
		const char* name = adcs[i].c_str();

		THaADCHe3Spin* adc = new THaADCHe3Spin( name, name, this );
		if( !adc or adc->IsZombie() ) {
			// Urgh. Something is very bad
			Error(Here("ReadDatabase"), 
				"Error creating THaADCHe3Spin %s. Call expert.",
				name );
			return kInitError;
		}
		DEBUG_INFO(Here("ReadDatabase"),"Add THaADCHe3Spin %s",name);
		AddDetector(adc);
	}

	vector<string> bits = vsplit(bitspinlist);
	if (bits.size()>0)
		Info(Here("ReadDatabase"),"to Load %d copy of THaBitHe3Spin : '%s'",
		bits.size(),bitspinlist.c_str());
	for( ssiz_t i=0; i<bits.size(); ++i ) {
		assert( !bits[i].empty() );
		const char* name = bits[i].c_str();

		THaBitHe3Spin* bit = new THaBitHe3Spin( name, name, this );
		if( !bit or bit->IsZombie() ) {
			// Urgh. Something is very bad
			Error(Here("ReadDatabase"), 
				"Error creating THaBitHe3Spin %s. Call expert.",
				name );
			return kInitError;
		}
		DEBUG_INFO(Here("ReadDatabase"),"Add THaBitHe3Spin %s",name);
		AddDetector(bit);
	}



	return kOK;
}
//_____________________________________________________________________________
THaAnalysisObject::EStatus THaHe3Target::Init( const TDatime& run_time )
{
	// Init method for a beam apparatus. Calls the standard THaApparatus
	// initialization and, in addition, finds pointer to the current 
	// run parameters.

	//fDetectors;

	//fTree = new TTree("Test","Hall A Analyzer Output DST");
	//if (!fTree) return kInitError;
	//fTree->SetAutoSave(200000000);
	//fTree->Branch("fSpinState",&fSpinState,"pfSpinState");

	EStatus stat=THaApparatus::Init(run_time);
	if (stat) return stat;

	fSpinModule.clear();
	TIter next(fDetectors);
	while( THaDetector* theDetector = 
		static_cast<THaDetector*>( next() ))
	{
		if(theDetector->IsA()->InheritsFrom("THaHe3SpinModule"))
		{
			DEBUG_INFO(Here("Init"),
				"Find THaHe3SpinModule %s",theDetector->GetPrefix());
			fSpinModule.push_back( dynamic_cast<THaHe3SpinModule *>(theDetector));
		}
	}
	if (fSpinModule.size()==0)
		DEBUG_WARNING(Here("Init"),
		"No Spin state Module found. I can not get spin state of target");


	return kOK;
}

//_____________________________________________________________________________
Int_t THaHe3Target::DefineVariables( EMode mode )
{
	// Initialize global variables and lookup table for decoder

	if( mode == kDefine && fIsSetup ) return kOK;
	fIsSetup = ( mode == kDefine );

	RVarDef vars[] = {
		{ "pol", "He3 Polarizaion in percent", "fPolarization"},
  { "theta", "Polarizaion Direction - Theta: Diviation of spin + vector from vertical up", "fTheta"},
  { "phi", "Polarizaion Direction - Phi: Angle of spin + vector projected to horizonal plane", "fPhi"},
		{ 0 }
	};
	return DefineVarsFromList( vars, mode )
		+DefineVarsFromList(THaHe3SpinModule::GetRVarDef(), mode );
}

Int_t  THaHe3Target::Reconstruct()
{
	//THaApparatus::Reconstruct();

	//THaHe3SpinModule::ClearEvent();

	DEBUG_MASSINFO("Reconstruct","Reconstruct() fSpinModule[%d] test=%d",
		fSpinModule.size(),fSpinModule.begin()==fSpinModule.end());

	EHe3Spin SpinState=kUnknown;
	Bool_t SpinStateValid=kFALSE;
	if (!fSpinModule.empty())
	{
		DEBUG_MASSINFO("Reconstruct","copy info of first spin modules");
		//SpinState=(*(fSpinModule.begin()))->GetSpinState();
		//SpinStateValid=(*(fSpinModule.begin()))->GetSpinStateValidity();
		SpinState=(fSpinModule[0])->GetSpinState();
		SpinStateValid=(fSpinModule[0])->GetSpinStateValidity();

		DEBUG_MASSINFO("Reconstruct","look for more spin modules");
		ssiz_t i=1;
		for (;i<fSpinModule.size() && SpinStateValid;i++)
		{

			DEBUG_MASSINFO("Reconstruct","Get Spin State from fSpinModule[%d]",i);

			if ((fSpinModule[i])->GetSpinStateValidity())
			{
				if (SpinState==(fSpinModule[i])->GetSpinState()) continue;
				else SpinStateValid=kFALSE;
			}
			else SpinStateValid=kFALSE;
		}
		if (!SpinStateValid) 
			DEBUG_INFO(Here("Reconstruct"),
			"Invalid or Inconsistant System Spin State");

	}//if (!fSpinModule.empty())


	DEBUG_MASSINFO("Reconstruct","Set Spin sate=%d, valid=%d",
		SpinState,(Int_t)SpinStateValid);
	SetSpinState(SpinState);
	SetSpinStateValidity(SpinStateValid);

	if (fSpinState == kPara)
	{
		fTheta	=fFieldTheta;
		fPhi	=fFieldPhi;
	}
	else if (fSpinState == kAntiPara)
	{
		fTheta	= TMath::Pi()-fFieldTheta;
		fPhi	= -fFieldPhi;
	}
	else
	{
		fTheta	= 0;
		fPhi	= 0;
	}

	TrkIfoClear();
	//Set(Double_t p, Double_t dp, Double_t x, Double_t y, Double_t th, Double_t ph, Double_t px, Double_t py, Double_t pz)
	fTrkIfo.Set(fPolarization/100,0,0,0,0,0,
				fPolarization/100*TMath::Sin(fTheta)*TMath::Sin(fPhi),
											 fPolarization/100*TMath::Cos(fTheta),
													 fPolarization/100*TMath::Sin(fTheta)*TMath::Cos(fPhi)
			   );

	return 0;
}

Int_t THaHe3Target::End( THaRunBase* run )
{
	//if (fTree != 0) fTree->Write();
	return 0;
}

//_____________________________________________________________________________
ClassImp(THaHe3Target)


