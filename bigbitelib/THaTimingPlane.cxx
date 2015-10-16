//////////////////////////////////////////////////////////////////////////
//                                                                           
// THaTimingPlane                                                           
//                                                                           
// Bigbite Shower detector Timing related data decode        
//                                                                           
//////////////////////////////////////////////////////////////////////////
//	
// Author: Jin Huang <mailto:jinhuang@jlab.org>     July 2007	
//				base on analyzer for SRC experiment
//	Modify History:
//		Mar 2008   Jin Huang <mailto:jinhuang@jlab.org>
//          read in geometry of trigger plane
//		Mar 2008   Jin Huang <mailto:jinhuang@jlab.org>
//          match hits with tracks
//      Sept 2008	Jin Huang <mailto:jinhuang@jlab.org>
//			Modify into THaTimingPlane for Bigbite Shower detector
//
//////////////////////////////////////////////////////////////////////////

#include <cmath>

#include "THaEvData.h"
#include "TClonesArray.h"
#include "TDatime.h"

#include "THaScintPlane.h"
#include "VarType.h"
#include "VarDef.h"
#include "TMath.h"
#include "THaScintHit.h"
#include "THaScintBar.h"
#include "THaScintPMT.h"
#include "THaTrackProj.h"
#include "THaTrack.h"

//put this header file below all other headers
#include "THaTimingPlane.h"

using namespace std;


ClassImp(THaTimingPlane)

//_____________________________________________________________________________
THaTimingPlane::THaTimingPlane( const char* name, const char* description, THaApparatus* a) :
THaNonTrackingDetector(name,description,a), 
fEplane(NULL), fdEplane(NULL)
{

	DEBUG_LEVEL_RELATED_PERFORMACE_CHECKER;

	DEBUG_HALL_A_ANALYZER_DEBUGER_INIT;

	// Setup the e and de planes
	Setup( name, "e", "de", description, false );
}

//_____________________________________________________________________________
void THaTimingPlane::Setup( const char* name,
						   const char* eplane_name,
						   const char* deplane_name,
						   const char* description,
						   bool subnames )
{
	// Set up the total shower counter. Called by constructor.

	static const char* const here = "Setup()";
	static const char* const message = 
		"Must construct %s detector with valid name! Object construction failed.";

	// Base class constructor failed?
	if( IsZombie()) return;

	size_t e, de;
	if( !eplane_name || (e = strlen(eplane_name)) == 0 ) {
		Error( Here(here), message, "Eplane" );
		MakeZombie();
		return;
	}
	if( !deplane_name || (de = strlen(deplane_name)) == 0 ) {
		Error( Here(here), message, "dEplane" );
		MakeZombie();
		return;
	}

	size_t nlen = strlen(name);
	size_t slen = TMath::Max(e,de);
	size_t len = slen;
	if( subnames )
		len += nlen+1;
	char* subname = new char[ len+1 ];
	const char* sname;
	if( subnames ) {
		strcpy( subname, name );
		strcat( subname, "." );
		strcat( subname, eplane_name );
		sname = subname;
	} else 
		sname = eplane_name;

	char* desc = new char[ 50+strlen(description) ];
	if( description && strlen(description)>0 )
		strcpy( desc, description );
	else {
		strcpy( desc, "BigBite Trigger Plane" );
		SetTitle( desc );
	}
	size_t dlen = strlen(desc);
	strcat( desc, " E-plane subdetector" );

#if DEBUG_LEVEL>=3//start show info
	Info(Here(here),"\tConstruct subdetector Name=%s, Desc=%s", sname,desc);
#endif//#if DEBUG_LEVEL>=3

	fEplane = new THaScintPlane( sname, desc , this);
	if( !fEplane || fEplane->IsZombie() ) {
		Error( Here(here), "Fail to initiate %s", "Eplane" );
		MakeZombie();
		goto exit;
	}

	if( subnames )
		strcpy( subname+nlen+1, deplane_name );
	else
		sname = deplane_name;
	strcpy( desc+dlen, " dE-plane subdetector" );


#if DEBUG_LEVEL>=3//start show info
	Info(Here(here),"\tConstruct subdetector Name=%s, Desc=%s", sname,desc);
#endif//#if DEBUG_LEVEL>=3

	fdEplane = new THaScintPlane( sname, desc  , this);
	if( !fdEplane && fdEplane->IsZombie() ) {
		Error( Here(here), "Fail to initiate %s", "dEplane" );
		MakeZombie();
		goto exit;
	}

exit:
	delete [] subname;
	delete [] desc;
	return;
}

//_____________________________________________________________________________
THaTimingPlane::~THaTimingPlane()
{
	// Destructor. Remove variables from global list.
	delete fEplane;
	delete fdEplane;
	if( fIsSetup )
		RemoveVariables();
}

//_____________________________________________________________________________
THaDetectorBase::EStatus THaTimingPlane::Init( const TDatime& run_time )
{
	// Set up trigger plane counter. This function 
	// reads the trigger plane parameters from local database files 
	// (e.g. "db_tp.e.dat"), then calls Init() for
	// the shower and preshower subdetectors, respectively.

	if( IsZombie() || !fEplane || !fdEplane )
		return fStatus = kInitError;

	MakePrefix();

	EStatus status;
	if( (status = THaNonTrackingDetector::Init( run_time )) ||
		(status = fEplane->Init( run_time )) ||
		(status = fdEplane->Init( run_time )) )
		return fStatus = status;

	return fStatus;
}
//_____________________________________________________________________________
Int_t THaTimingPlane::DefineVariables( EMode mode )
{
	// Initialize global variables and lookup table for decoder

	if( mode == kDefine && fIsSetup ) return kOK;
	fIsSetup = ( mode == kDefine );

	THaNonTrackingDetector::DefineVariables(mode);

	// Register variables in global list
	RVarDef vars[] = {

		//end
		{0}
	};

	Int_t value = DefineVarsFromList( vars, mode );

	return value;
}
//_____________________________________________________________________________
Int_t THaTimingPlane::ReadDatabase( const TDatime& date )
{
	static const char*  here = "ReadDatabase()";
	static const Int_t LEN=300;
	char buff[LEN];
	char cbuff[LEN];	//commment buffer

	FILE* file = OpenFile( date );
	if( !file ) 
	{
#if DEBUG_LEVEL>=1//start show warning 
		Warning(Here(here),"\tTHaTimingPlane(%s) ReadDatabase file open failed.",GetName());
#endif//#if DEBUG_LEVEL>=1
		return kFileError;
	}

#if DEBUG_LEVEL>=3//start show info
	Info(Here(here),"\tStart reading Database");
#endif//#if DEBUG_LEVEL>=3

	// Build the search tag and find it in the file. Search tags
	// are of form [ <prefix> ], e.g. [ N.bar.n1 ].
	TString line;

	TString prefix=fPrefix;
	prefix.Chop();  // remove trailing dot of prefix

	///////////////////////// geometry ///////////////////////////
	TString tag = Form("[%s.%s]",prefix.Data(),  "geometry"  );

#if DEBUG_LEVEL>=3//start show info
	Info(Here(here),"\tTrying to read in database section %s", tag.Data());
#endif

	tag.ToLower();
	bool found = false;
	while (!found && fgets (buff, LEN, file) != NULL) {
		char* buf = ::Compress(buff);  //strip blanks
		line = buf;
		delete [] buf;
#if DEBUG_LEVEL>=5//start show info
		Info(Here(here),"\tTrying to search line <%s>", line.Data());
#endif
		if( line.EndsWith("\n") ) line.Chop();  //delete trailing newline
#if DEBUG_LEVEL>=5//start show info
		Info(Here(here),"\tafter line.Chop() <%s>", line.Data());
#endif
		line.ToLower();
		if ( tag == line ) 
			found = true;
	}
	if( !found ) {
		Error(Here(here), "Database entry \"%s\" not found!", tag.Data() );
		fclose(file);
		return kInitError;
	}

	//jump throug comment lines
	while ( ReadNumberSignStartComment( file, cbuff, LEN ));

	Double_t xtmp,ytmp,ztmp;
	fgets( buff, LEN, file );
	if( sscanf( buff, "%lg%lg%lg",&xtmp,&ytmp,&ztmp) != 3 ) {
		if( *buff ) buff[strlen(buff)-1] = 0; //delete trailing newline
		Error( Here(here), "Error 4 reading %s (fOrigin): %s", tag.Data(), buff );
		fclose(file);
		return kInitError;
	}
	fOrigin.SetXYZ(xtmp,ytmp,ztmp);

	//jump throug comment lines
	while ( ReadNumberSignStartComment( file, cbuff, LEN ));

	fgets( buff, LEN, file );
	if( sscanf( buff, "%lg%lg%lg",&xtmp,&ytmp,&ztmp) != 3 ) {
		if( *buff ) buff[strlen(buff)-1] = 0; //delete trailing newline
		Error( Here(here), "Error 4 reading %s (fXax): %s", tag.Data(), buff );
		fclose(file);
		return kInitError;
	}
	fXax.SetXYZ( xtmp,ytmp,ztmp );

	//jump throug comment lines
	while ( ReadNumberSignStartComment( file, cbuff, LEN ));

	fgets( buff, LEN, file );
	if( sscanf( buff, "%lg%lg%lg",&xtmp,&ytmp,&ztmp) != 3 ) {
		if( *buff ) buff[strlen(buff)-1] = 0; //delete trailing newline
		Error( Here(here), "Error 4 reading %s (fZax): %s", tag.Data(), buff );
		fclose(file);
		return kInitError;
	}
	fYax.SetXYZ( xtmp,ytmp,ztmp );
	fZax = fXax.Cross(fYax);



#if DEBUG_LEVEL>=3//start show info
	Info(Here(here),"Printing Origin and x,y,z axis vector:");
	fOrigin.Print();
	fXax.Print();
	fYax.Print();
	fZax.Print();
#endif	

	fclose(file);
	return kOK;
}
//_____________________________________________________________________________
Int_t THaTimingPlane::Decode( const THaEvData& evdata )
{
	// Decode triggerplane detector. Calls Decode() of fdEplane and fEplane.
	// Return the return value of fEplane->Decode().
	if( !IsOK() ) 
		return -1;

	return fEplane->Decode( evdata )+fdEplane->Decode( evdata );
}

//_____________________________________________________________________________
void THaTimingPlane::SetApparatus( THaApparatus* app )
{
	// Set the apparatus of this detector as well as the subdetectors

	THaNonTrackingDetector::SetApparatus( app );
	//fEplane->SetApparatus( app );
	//fdEplane->SetApparatus( app );
	return;
}

//_____________________________________________________________________________
Int_t THaTimingPlane::FineProcess( TClonesArray& tracks )
{
	// Scintillator fine processing.
	// Not implemented. Does nothing, returns 0.
	fEplane->FineProcess(tracks);
	fdEplane->FineProcess(tracks);

	return kOK;// THaNonTrackingDetector::FineProcess(tracks);
}

//_____________________________________________________________________________
Int_t THaTimingPlane::CoarseProcess( TClonesArray& tracks )
{
	// Scintillator fine processing.
	fEplane->CoarseProcess(tracks);
	fdEplane->CoarseProcess(tracks);

	return kOK;//THaNonTrackingDetector::CoarseProcess(tracks);
}

//_____________________________________________________________________________
char* THaTimingPlane::ReadNumberSignStartComment( FILE* fp, char *buf, const int len )
{
	// Read blank and comment lines 
	// return NULL if is not comment line
	int ch = fgetc(fp);  // peak ahead one character
	ungetc(ch,fp);

	if(ch=='#' || ch=='\n')
		return fgets(buf,len,fp); // read the comment
	else
		return NULL;
}

//_____________________________________________________________________________



///////////////////////////////////////////////////////////////////////////////


