//////////////////////////////////////////////////////////////////////////////
//                                                                           //
// THaMWDCPlane                                                              //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Here:                                                                     //
//        Units of measurements:                                             //
//        For cluster position (center) and size  -  wires;                  //
//        For X, Y, and Z coordinates of track    -  meters;                 //
//        For Theta and Phi angles of track       -  in tangents.            //
//                                                                           //
// The wire numbers run from 0 to nWires - 1                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "THaMWDCPlane.h"
#include "THaMWDC.h"
#include "THaDetMap.h"
#include "THaEvData.h"
#include "TString.h"
#include "TMath.h"
#include "VarDef.h"
#include "VarType.h"
#include "THaVDCAnalyticTTDConv.h"
#include "THaVDCTableTTDConv.h"
#include "THaVDC2DTableTTDConv.h"
#include "THaVDCLinearTTDConv.h"
#include "THaVDCFuncTTDConv.h"
#include "THaVDCWire.h"
#include "THaMWDCHit.h"

#include <cstring>
#include <vector>
#include <iostream>
#include <cstdio>
#include <fstream>


using namespace std;

//_____________________________________________________________________________
THaMWDCPlane::THaMWDCPlane( const char* name, const char* description,
			    THaDetectorBase* parent )
  : THaSubDetector(name,description,parent), fTTDConv(NULL),
    fMWDC(parent)
{
  // Constructor


  // Since TCloneArrays can resize, the size here is fairly unimportant
  fHits     = new TClonesArray("THaMWDCHit", 0 );
  fWires    = new TClonesArray("THaVDCWire", 0 );

  fRefHits     = new TClonesArray("THaMWDCHit", 0 );
  fRefCh       = new TClonesArray("THaVDCWire", 0 );

  // cbr for debugging
  
  fRawT = NULL;
  fCorT = NULL;
  fTcounter = NULL;
  fRefT = NULL;
  fCellHeight = 0.006;   // :(  Setting the cell height to 6mm  Need to put this in the database 

  if( !parent && GetDetector() )
    fMWDC = static_cast<THaDetector*>(GetDetector());
//Jin Huang:Change from fMWDC = static_cast<THaSubDetector*>(GetDetector())->GetDetector(); for compability to Ananlyzer v1.5.0

  fTotalHits = 0;
  fRejectedHits = 0;
}

//_____________________________________________________________________________
Int_t THaMWDCPlane::GetParameter( FILE* file, const TString tag, Double_t* value  )
{  
  static const char* const here = "ReadDatabase::GetParameter";
  const int LEN = 200;
  char buff[LEN];

  if( !file ) return kFileError;
  if (fgets (buff, LEN, file) == NULL ) {
    Error(Here(here), "Unexpected end of database file" );
    fclose(file);
    return kFileError;
  }
  TString line = buff;
  if ( line.BeginsWith(tag) ) {
    line.Replace(0,tag.Length(),"");
    line.Append("\0");
    sscanf(line.Data(),"%lf", value); 
    return 0 ; 
  } else {
    Error(Here(here), "Database file corrupted" );
    return kFileError;
  }
}

//_____________________________________________________________________________
Int_t THaMWDCPlane::GetOptionalParameter( FILE* file, const TString tag, Double_t* value  )
{  
  static const char* const here = "ReadDatabase::GetParameter";
  const int LEN = 200;
  char buff[LEN];
  
  fpos_t currentpos;
  fgetpos(file, &currentpos);

  if( !file ) return kFileError;
  if (fgets (buff, LEN, file) == NULL ) {
    Error(Here(here), "Unexpected end of database file" );
    fclose(file);
    return kFileError;
  }
  TString line = buff;
  if ( line.BeginsWith(tag) ) {
    line.Replace(0,tag.Length(),"");
    line.Append("\0");
    sscanf(line.Data(),"%lf", value); 
    return 0 ; 
  } 
  else
    {
      // Move back to where we were before
      fsetpos(file, &currentpos);
      return 1;
    }
}

//_____________________________________________________________________________
Int_t THaMWDCPlane::GetParameter( FILE* file, const TString tag, char *string  )
{  
  static const char* const here = "ReadDatabase::GetParameter";
  const int LEN = 200;
  char buff[LEN];
  char errorbuff[LEN];

  if( !file ) return kFileError;
  if (fgets (buff, LEN, file) == NULL ) {
    Error(Here(here), "Unexpected end of database file" );
    fclose(file);
    return kFileError;
  }
  TString line = buff;
  if ( line.BeginsWith(tag) ) {
    line.Replace(0,tag.Length(),"");
    line.Append("\0");
    sscanf(line.Data(),"%s", string); 
    return 0 ; 
  } else {
    sprintf( errorbuff, "Couldn't find parameter %s", tag.Data() );
    Error(Here(here), errorbuff);
    return kFileError;
  }
}



//_____________________________________________________________________________
Int_t THaMWDCPlane::ReadDatabase( const TDatime& date )
{
  // Allocate TClonesArray objects and load plane parameters from database


  FILE* file = OpenFile( date );

  if( !file ) return kFileError;

  // Use default values until ready to read from database
  
  static const char* const here = "ReadDatabase";
  const int LEN = 200;
  char buff[LEN];

  
  // Build the search tag and find it in the file. Search tags
  // are of form [ <prefix> ], e.g. [ R.vdc.u1 ].
  TString tag(fPrefix); tag.Prepend("["); 
  tag.Replace( tag.Length()-1, 1, "" );  // delete trailing dot of prefix
  tag.Append(".cratemap]"); 
  TString line, tag2(tag);
  tag.ToLower();
  bool found = false;
  while (!found && fgets (buff, LEN, file) != NULL) {
    char* buf = ::Compress(buff);  //strip blanks
    line = buf;
    delete [] buf;
    if( line.EndsWith("\n") ) line.Chop();  //delete trailing newline
    line.ToLower();
    if ( tag == line ) 
      found = true;
  }
  if( !found ) {
    Error(Here(here), "Database entry \"%s\" not found!", tag2.Data() );
    fclose(file);
    return kInitError;
  }
  
  //Found the entry for this plane
  //first are definitions of the reference channels
  //then the definitions of the tdc channels for individiual wires
  //
  Int_t nWires = 0;    // Number of wires to create
  Int_t nRefCh = 0;    // Number of ref.channels to create
  Int_t prev_first = 0, prev_nwires = 0;

  // clean-up in-case re-initializing
  fWires->Delete();
  fRefCh->Delete();
  if (fRawT) {
    delete [] fRawT;      fRawT=0;
    delete [] fCorT;      fCorT=0;
    delete [] fTcounter;  fTcounter=0;
    delete [] fRefT;      fRefT=0;
  }
  RemoveVariables();
  
  // Set up the detector map
  fDetMap->Clear();

  Int_t crate, slot, lo, hi, model, refindex;
  Int_t lolo;

  crate = 0 ;
  // Get crate, slot, low channel and high channel from file
  // for the reference channels
  while (crate!=-1) {
    fgets( buff, LEN, file );
    if( sscanf( buff, "%d%d%d%d%d%d", &crate, &slot, &lo, &hi, &model, &refindex ) != 6 ) {
      if( *buff ) buff[strlen(buff)-1] = 0; //delete trailing newline
      Error( Here(here), "Error reading detector map line: %s", buff );
      fclose(file);
      return kInitError;
    }
    if (crate!=-1) {
      Int_t first = prev_first + prev_nwires;
      // Add module to the detector map
      if ( fDetMap->AddModule(crate, slot, lo, hi, first, model, refindex) < 0 ) {
	Error( Here(here), "Too many DetMap modules (maximum allowed - %d).", 
	       THaDetMap::kDetMapSize);
	fclose(file);
	return kInitError;
      }
      prev_first = first;
      prev_nwires = (hi - lo + 1 );
      nRefCh += prev_nwires;
    }
  }
  if (nRefCh>0) {
    clog << GetName() << " is using reference channels" << endl;
  }
  else {
    clog << GetName() << " not using reference channels" << endl;
  }

  // Get crate, slot, low channel and high channel from file
  // for the wires
  crate = 0;
  while (crate!=-1) {
    fgets( buff, LEN, file );
    if( sscanf( buff, "%d%d%d%d%d%d", &crate, &slot, &lo, &hi, &model, &refindex ) != 6 ) {
      if( *buff ) buff[strlen(buff)-1] = 0; //delete trailing newline
      Error( Here(here), "Error reading detector map line: %s", buff );
      fclose(file);
      return kInitError;
    }
    if (crate!=-1) {
      if (lo<=hi) {
	Int_t first = prev_first + prev_nwires;
	// Add module to the detector map
	if ( fDetMap->AddModule(crate, slot, lo, hi, first, model, refindex) < 0 ) {
	  Error( Here(here), "Too many DetMap modules (maximum allowed - %d).", 
		 THaDetMap::kDetMapSize);
	  fclose(file);
	  return kInitError;
	}
	prev_first = first;
	prev_nwires = (hi - lo + 1 );
	nWires += prev_nwires;
      } else {
	lolo = lo ;
	while (lolo>=hi) {
	  Int_t first = prev_first + prev_nwires;
	  if ( fDetMap->AddModule(crate, slot, lolo, lolo, first, model, refindex) < 0 ) {
	    Error( Here(here), "Too many DetMap modules (maximum allowed - %d).", 
		   THaDetMap::kDetMapSize);
	    fclose(file);
	    return kInitError;
	  }
	  prev_first = first;
	  prev_nwires =  1 ;
	  nWires += prev_nwires;
	  lolo -= 1 ;
	}
      }
    }
  }

  // previous section means: the first nRefCh entries in the detector map are the
  // reference channels, the next nWires entries are the Wires.

  // search for the geometry section
  // it is assumed to be right after the detector map  

  tag=fPrefix; tag.Prepend("["); 
  tag.Replace( tag.Length()-1, 1, "" ); 
  tag.Append(".geom]");  
  tag.ToLower();
  found=false;
  do {
    if (fgets (buff, LEN, file) == NULL ) {
      Error(Here(here), "Database entry \"%s\" not found!", tag.Data() );
      fclose(file);
      return kInitError;
    }
    char* buf = ::Compress(buff);  //strip blanks
    line = buf;
    delete [] buf;
    if( line.EndsWith("\n") ) line.Chop();  //delete trailing newline
    line.ToLower();
    if ( tag == line ) 
      found = true;
  } while (!found);

  Double_t wireangle;
  Double_t reconstructwith;
  Double_t thetaindeg;
  Double_t phiindeg;
  
  Int_t noc1 = 0;
  Int_t noc2 = 0;

  fDriftC1 = 0.0;
  fDriftC2 = 0.0;
  fDriftC3 = 0.0;

  if (GetParameter(file,"plane.z = ",&fZ)!=0) return kFileError;
  if (GetParameter(file,"wire.1xp = ",&fWBeg)!=0) return kFileError;
  if (GetParameter(file,"wire.spacing = ",&fWSpac)!=0) return kFileError;
  if (GetParameter(file,"wire.angle = ",&wireangle)!=0) return kFileError;
  if (GetParameter(file,"tdc.res = ",&fTDCRes)!=0) return kFileError;
  if (GetParameter(file,"xp.res = ",&fPosRes)!=0) return kFileError;
  if (GetParameter(file,"drift.min = ",&fMinTime)!=0) return kFileError;
  if (GetParameter(file,"drift.max = ",&fMaxTime)!=0) return kFileError;
  if (GetParameter(file,"drift.v = ",&fDriftVel)!=0) return kFileError;
  if (GetOptionalParameter(file,"drift.c1 = ", &fDriftC1)!=0){ noc1 = 1;}
  else
    {
       if (GetOptionalParameter(file,"drift.c2 = ", &fDriftC2)!=0){ noc2 = 1;}
       else
	 if (GetParameter(file,"drift.c3 = ",&fDriftC3)!=0) return kFileError;
    }
  if (GetParameter(file,"geo.maxx = ",&fXMax)!=0) return kFileError;
  if (GetParameter(file,"geo.minx = ",&fXMin)!=0) return kFileError;
  if (GetParameter(file,"geo.maxy = ",&fYMax)!=0) return kFileError;
  if (GetParameter(file,"geo.miny = ",&fYMin)!=0) return kFileError;
  if (GetParameter(file,"geo.minth = ",&fTHMin)!=0) return kFileError;
  if (GetParameter(file,"geo.maxth = ",&fTHMax)!=0) return kFileError;
  if (GetParameter(file,"geo.minph = ",&fPHMin)!=0) return kFileError;
  if (GetParameter(file,"geo.maxph = ",&fPHMax)!=0) return kFileError;

  if( GetParameter( file, "geo.xcorr = ", &fXCorr ) != 0 ) fXCorr = 0.0;
  if( GetParameter( file, "geo.ycorr = ", &fYCorr ) != 0 ) fYCorr = 0.0;
  if( GetParameter( file, "geo.thetacorr = ", &thetaindeg ) != 0 ) thetaindeg = 0.0;
  if( GetParameter( file, "geo.phicorr = ", &phiindeg ) != 0 ) phiindeg = 0.0;
  if( GetParameter( file, "reconstructwith = ", &reconstructwith ) != 0 ) 
    {
      fUseInReconstruct = kTRUE;
    }
  else
    {
      if( reconstructwith > 0.0 )
	fUseInReconstruct = kTRUE;
      else
	fUseInReconstruct = kFALSE;
    }

  SetThetaDegCorr( thetaindeg );
  SetPhiDegCorr( phiindeg );

  delete fTTDConv;
 
  char conversiontablename[255];
  
  ifstream testfile;
 
  
   
  if(GetParameter(file,"conv.tablename = ",conversiontablename)!=0)
    {
      if( noc1 )
     	{
     	  clog << "Using linear time-distance conversion for plane " << GetName() << " with drift velocity of " << fDriftVel << " m/ns" <<endl;
     	  fTTDConv = new THaVDCLinearTTDConv(fDriftVel);
     	}
      else
     	{
     	  clog << "Using function time-distance conversion for plane " << GetName() << " with drift velocity of " << fDriftVel << " m/ns" << endl;
	  fTTDConv = new THaVDCFuncTTDConv(fDriftVel, fDriftC1, fDriftC2, fDriftC3);
     	}
    }
  else
    {
      testfile.open( conversiontablename );
      if( testfile.good() )
	{
	  clog << "Found 2D time-distance conversion table " << conversiontablename << " for plane " <<  endl;
	  fTTDConv = new THaVDC2DTableTTDConv(conversiontablename);
	  THaVDC2DTableTTDConv *temp = new THaVDC2DTableTTDConv(conversiontablename);
	  fDriftVel = temp->GetDriftVelocity();
	}
      else
	{  
	  if( noc1 )
	    {
	      clog << "Using linear time-distance conversion for plane " << GetName() << " with drift velocity of " << fDriftVel << " m/ns" <<endl;
	      fTTDConv = new THaVDCLinearTTDConv(fDriftVel);
	    }
	  else
	    {
	      clog << "Using function time-distance conversion for plane " << GetName() << " with drift velocity of " << fDriftVel << " m/ns and c1 " << fDriftC1 << endl;
	      fTTDConv = new THaVDCFuncTTDConv(fDriftVel, fDriftC1, fDriftC2, fDriftC3);
	    }
	  
	}
    }

  SetWAngleDeg(wireangle);

  fXWidth = fXMax -  fXMin;
  fYWidth = fYMax -  fYMin;
  
  fNWiresHit = 0;
  
  // search for the TDC Offset section
  // it is assumed to be right after the geometry section

  tag=fPrefix; tag.Prepend("["); 
  tag.Replace( tag.Length()-1, 1, "" ); 
  tag.Append(".offsets]");  
  tag.ToLower();
  do {
    if (fgets (buff, LEN, file) == NULL ) {
      Error(Here(here), "Database entry \"%s\" not found!", tag.Data() );
      fclose(file);
      return kInitError;
    }
    char* buf = ::Compress(buff);  //strip blanks
    line = buf;
    delete [] buf;
    if( line.EndsWith("\n") ) line.Chop();  //delete trailing newline
    line.ToLower();
    found = ( tag == line );
  } while (!found);

  // first read in the time offsets for the wires
  float* wire_offsets = new float[nWires];

  for (int i=0; i<nWires; i++) {
    wire_offsets[i]=0.;
  }
  int wnum=0;
  float offset=0.0;
  while  ((2==fscanf(file, "%d%f", &wnum, &offset))&&(wnum>=0)) {
    if (wnum<nWires) {
      wire_offsets[wnum]=offset;
    }
  }

  // the VDC lookup table seems not to be appropriate, for now I will skip



  // Define time-to-drift-distance converter
  // Currently, we use the analytic converter. 
  // FIXME: Let user choose this via a parameter
  

  for (int i = 0; i < nWires; i++) {
//    THaVDCWire* wire = new((*fWires)[i]) 
      new((*fWires)[i]) 
	  THaVDCWire( i, fWBeg+(i)*fWSpac, wire_offsets[i], fTTDConv );
  }
  delete [] wire_offsets;

  for (int i = 0; i < nRefCh; i++) {
    THaVDCWire* wire = new((*fRefCh)[i]) 
      THaVDCWire( i, 0, 0 , fTTDConv );
    wire->SetFlag(0);
  }

  fOrigin.SetXYZ( 0.0, 0.0, fZ );
  THaDetectorBase* parent = GetDetector();
  if( parent )
    fOrigin += parent->GetOrigin();

  // cbr for debugging

  fNWires = GetNWires();
  fNRefCh = GetNRefCh();

  fRawT = new Double_t[GetNWires()];
  fCorT = new Double_t[GetNWires()];
  fTcounter = new Int_t[GetNWires()];
  fRefT = new Double_t[GetNRefCh()];

  fIsInit = true;
  fclose(file);
  return kOK;
}

//_____________________________________________________________________________
Int_t THaMWDCPlane::DefineVariables( EMode mode )
{
  // initialize global variables


  if( mode == kDefine && fIsSetup ) return kOK;
  fIsSetup = ( mode == kDefine );

  // Register variables in global list

  RVarDef vars[] = {
    { "nhit",   "Number of hits",             "GetNHits()" },
    { "wire",   "Active wire numbers",        "fHits.THaMWDCHit.GetWireNum()" },
    { "rawtime","Raw TDC values of wires",    "fHits.THaMWDCHit.fRawTime" },
    { "time",   "TDC values of active wires", "fHits.THaMWDCHit.fTime" },
    { "dist",   "Drift distances",            "fHits.THaMWDCHit.fDist" },
    { "track",  "Track number used in reconstruction", "fHits.THaMWDCHit.GetTrackNumber()" },
    { "chi2",  "Chi2 Contribution", "fHits.THaMWDCHit.GetChi2Cont()" },
    { "cchi2",  "Coarse Chi2 Contribution", "fHits.THaMWDCHit.GetCoarseChi2Cont()" },
    { "refchokay", "All relevant ref channels are okay", "AreRefChOkay()" },
    { "rawtimeof", "Raw time of particular wire", "fRawT"},
    { "cortimeof", "Corrected time of particular wire", "fCorT"},
    { "reftimeof", "Time on particular ref. channel}", "fRefT"},
    { "NumHitsof", "Number of hits of particular wire", "fTcounter"},
    { "myref", "Reference time of a channel", "fHits.THaMWDCHit.fRefTime" },
    { "offset", "TDC offset used in a calculation", "fHits.THaMWDCHit.fOffset" },
    { "showercut", "Boolean result of cut by shower clusters", "fHits.THaMWDCHit.IsValidByShowerDouble()" },
    { "verbyshower", "Boolean result of cut by shower clusters", "fHits.THaMWDCHit.VerifiedByShowerDouble()" },
    { "wirepos", "Wire position in plane coords", "fHits.THaMWDCHit.GetPos()" },
    { 0 }
  };
  return DefineVarsFromList( vars, mode );

}

//_____________________________________________________________________________
THaMWDCPlane::~THaMWDCPlane()
{
  // Destructor.

  if( fIsSetup )
    RemoveVariables();
  delete fWires;
  delete fHits;
  delete fRefCh;
  delete fRefHits;
  delete fTTDConv;
  delete fRawT;
  delete fCorT;
  delete fRefT;
  delete fTcounter;
}

//_____________________________________________________________________________
THaAnalysisObject::EStatus THaMWDCPlane::Init( const TDatime& date )
{
  // Calls its own Init(), then initializes subdetectors, then calculates
  // some local geometry data.

    if( IsZombie() )
    return fStatus = kInitError;

  EStatus status;
  status = (THaSubDetector::Init( date )) ;
  if (status!=0) return fStatus = status;

  THaDetectorBase *parent = GetDetector();
  if( parent )
    fOrigin = parent->GetOrigin();   // is this right? you're overwriting it

  fEventType = gHaVars->Find("g.evtyp");
  fBit3 = gHaVars->Find("D.bit3");

  return fStatus = kOK;
}



//_____________________________________________________________________________
inline
void THaMWDCPlane::Clear( Option_t* opt )
{    
  // Clears the contents of the and hits and clusters
  fNWiresHit = 0;
  fHits->Clear();
  fRefHits->Clear();

  // cbr for debbuging only reset per event data
  for (Int_t i=0; i<GetNWires(); i++) {
    fRawT[i]=-1.e35;
    fCorT[i]=-1.e35;
    fTcounter[i]=0;
  }
  for (Int_t i=0; i<GetNRefCh();i++) {
    fRefT[i]=-2.e35;
  }
  fRefOkay = true ;
}

//_____________________________________________________________________________
Int_t THaMWDCPlane::Decode( const THaEvData& evData)
{    
  // Converts the raw data into hit information
  // Assumes channels & wires  are numbered in order
  // Assumes that the first channels are for reference channels
  // Make sure the wires are numbered in order, even if the channels aren't
              

  //clog << "Decoding plane " << GetName() << endl;

  Clear();  //Clear the last event

  if (!evData.IsPhysicsTrigger()) return -1;
  
  Int_t nextHit = 0;

  bool no_negative;
  if( fMWDC ) {
    // If true, ignore negativ drift times completely
    no_negative      = fMWDC->TestBit(THaMWDC::kIgnoreNegDrift);
  } else {
    no_negative = false;
  }

  // Loop over all detector modules containing reference lines
  // they have to be at the beginning of the detector map
  // and we need one line (=one module) per reference channel
  // otherwise only the first will be used
  Int_t i=0;
  Int_t data;
  Bool_t valid;

  while ((  i < GetNRefCh() )  &&  (i < fDetMap->GetSize()) ) {
    THaDetMap::Module * d = fDetMap->GetModule(i);	
    
    // Get number of channels with hits
    valid = true;
    Int_t chan=d->lo;
    Int_t nHits = evData.GetNumHits(d->crate, d->slot, chan);
    if (nHits<1) {
      //      clog<<"Warning: Event Number "<<dec<<evData.GetEvNum()<<"; No Hit for Reference Channel "<<i<<" of detector"<<fPrefix<<endl;
      data = 0;
      valid = false;
    } else {
      if (nHits>1) {
	//clog<<"Warning: Event Number "<<evData.GetEvNum()<<"; Multiple Hits for Reference Channel "<<i<<" of detector"<<fPrefix<<endl;
	//clog<<"Using first one"<<endl;
      }
      data = evData.GetData(d->crate, d->slot, chan, nHits-1);
    }
    // Wire numbers and channels go in the same order ... 
    THaVDCWire* wire = GetRefCh(i);
    if( !wire ) { 
      clog<<"THaMWDCPlane::Decode : Ref Channels are not initialized"<<endl;
      clog<<"Skipping event"<<endl;
      return -1;
    }
    
    // FIXME:  work in resolution for reference channel which is different from TDC
    Double_t time = -0.05 * data;
    new( (*fRefHits)[i] )  THaMWDCHit(this, wire, data, time, 0.0, 0.0, valid );    
    i++;
  }
  if (i!=GetNRefCh()) {
    clog<<"THaMWDCPlane::Decode : Mismatch between fNRefCh and hits on RefLines"<<endl;
    clog<<i<<" "<<GetNRefCh()<<endl;
    return -1;
  }

  //  clog << "Scanning through hits" << endl;

  while (i < fDetMap->GetSize()){
    //  clog << "Module index " << i << " of " << fDetMap->GetSize() << endl;
    THaDetMap::Module * d = fDetMap->GetModule(i);
    
    // Get number of channels with hits
    Int_t nChan = evData.GetNumChan(d->crate, d->slot);
    //clog << "Number of channels here: " << nChan << endl;

    for (Int_t chNdx = 0; chNdx < nChan; chNdx++) {
      // Use channel index to loop through channels that have hits

      Int_t chan = evData.GetNextChan(d->crate, d->slot, chNdx);
      if (chan < d->lo || chan > d->hi) 
	continue; //Not part of this detector

      // Wire numbers and channels go in the same order ... 
      Int_t wireNum  = d->first + chan - d->lo;

      THaVDCWire* wire = GetWire(wireNum-GetNRefCh());
      if( !wire || wire->GetFlag() != 0 ) { continue;}
      // Get number of hits for this channel and loop through hits
      Int_t nHits = evData.GetNumHits(d->crate, d->slot, chan);
      for (Int_t hit = 0; hit < nHits; hit++) {
	// Now get the TDC data for this hit
	Int_t data = evData.GetData(d->crate, d->slot, chan, hit);   	
	Double_t refoffset = 0.0;
	if ((d->refindex)>=0) {
	  THaMWDCHit* ahit = GetRefHit(d->refindex);
	  if ((!ahit)||(!(ahit->IsDataValid()))) {
	    fRefOkay = false ;
	    //	    clog<<"Warning: Event Number "<<dec<<evData.GetEvNum()<<"; No Hit for Reference Channel of detector"<<fPrefix<<endl;
	    //clog<<"Warning THaMWDCPlane::Decode : no hit on RefLine"<<endl;
	  } else {
	    refoffset = ahit->GetTime();
	  }
	}

	Double_t time = 0;

	time = fTDCRes * data - wire->GetTOffset() - refoffset;
	fTotalHits++;
	if ((!no_negative)||((time>=fMinTime)&&(time<=fMaxTime))) {
	  new( (*fHits)[nextHit++] )  THaMWDCHit(this, wire, data, time,  wire->GetTOffset(), refoffset );
	}
	else
	  {
	    fRejectedHits++;
	  }
      } // End hit loop
    } // End channel index loop
    i++;
  }
   
  // Sort the hits in order of increasing wire number and (for the same wire
  // number) increasing time (NOT rawtime)

  fHits->Compress();
  fHits->Sort();
  
  // cbr for debugging  fill the per event data 
  
  Int_t weirno=0;
  for (i=0;i< (Int_t) GetNHits();i++) { 
    weirno=((THaMWDCHit*)fHits->At(i))->GetWireNum();
    fTcounter[weirno]++;
    if (fTcounter[weirno]==1) {
      fRawT[weirno]=((THaMWDCHit*)fHits->At(i))->GetRawTime();
      fCorT[weirno]=((THaMWDCHit*)fHits->At(i))->GetTime();
    }
  }
  for (i=0; i<GetNRefHits(); i++) {
    weirno=((THaMWDCHit*)fRefHits->At(i))->GetWireNum();
    if ((fRefT[weirno]<-1.e34)&&(((THaMWDCHit*)fRefHits->At(i))->IsDataValid()))
      { 
	fRefT[weirno]=((THaMWDCHit*)fRefHits->At(i))->GetRawTime();
      }
  }
  
  return 0;
  
}
  
//_____________________________________________________________________________
void THaMWDCPlane::SetWAngle(Double_t a) 
{
  if (TMath::Abs((a*180.0/TMath::Pi())-90.)<10.0) {
    fPlaneType = kXchamber ;
  } 
  else if (TMath::Abs((a*180.0/TMath::Pi())-60.)<10.0) {
    fPlaneType = kVchamber ;
  } 
  else if (TMath::Abs((a*180.0/TMath::Pi())+60.)<10.0) {
    fPlaneType = kUchamber ;
  }
  else if (TMath::Abs((a*180.0/TMath::Pi()))<10.0) {
    fPlaneType = kYchamber ;
  }
  else {
    fPlaneType = kOchamber ;
  }

  fWAngle = a ;
}

//_____________________________________________________________________________
void THaMWDCPlane::SetWAngleDeg(Double_t a) 
{
  if (TMath::Abs(a-90.)<10.0) {
    fPlaneType = kXchamber ;
  } 
  else if (TMath::Abs(a-60.)<10.0) {
    fPlaneType = kVchamber ;
  } 
  else if (TMath::Abs(a+60.)<10.0) {
    fPlaneType = kUchamber ;
  }
  else if (TMath::Abs(a)<10.0) {
    fPlaneType = kYchamber ;
  }
  else {
    fPlaneType = kOchamber ;
  }

  fWAngle = a * TMath::Pi()/180.0; // Convert to radians
}

//_____________________________________________________________________________
Double_t THaMWDCPlane::ConvertDriftTimeToDistance(Double_t time , Double_t tanTheta) 
{
  return fTTDConv->ConvertTimeToDist(time, tanTheta );
}

//_____________________________________________________________________________
Double_t THaMWDCPlane::XYtoPlaneCoord( Double_t x, Double_t y)
{
  Double_t theta = fWAngle;

  return (x)*TMath::Sin(TMath::Abs(theta)) - (y)*(theta/TMath::Abs(theta))*TMath::Cos(theta);
}

Double_t THaMWDCPlane::GetXYCorr()
{
  Double_t theta = fWAngle;

  return (fXCorr)*TMath::Sin(TMath::Abs(theta)) - (fYCorr)*(theta/TMath::Abs(theta))*TMath::Cos(theta);
}
//_____________________________________________________________________________

void THaMWDCPlane::LoadMCHitAt( Double_t x, Double_t y, Double_t xp, Double_t yp )
{
  Double_t t;
  Double_t tp;
  Double_t width = GetWSpac()/2.0;

  Double_t drifttime;
  
  // Clear out previous hits

  Clear();

  t = XYtoPlaneCoord(x,y)-GetXYCorr();
  tp = XYtoPlaneCoord(xp,yp);

  Int_t counter;

  for( counter = 0; counter < GetNWires(); counter++ )
    {
      if( TMath::Abs(GetWire(counter)->GetPos() - t ) < width )
	{
	  drifttime =  (TMath::Abs(GetWire(counter)->GetPos() - t) *TMath::Cos(TMath::ATan(tp)))/GetDriftVel();
	  new( (*fHits)[fHits->GetEntries()] )  THaMWDCHit(this, GetWire(counter), 0, drifttime, 0.0 , 0.0 );
	}
    }
}

//_____________________________________________________________________________

void THaMWDCPlane::PrintDriftCuts()
{
  printf("%s\t%8d%8d\t%3.1f\t%3.1f\t\n", GetName(), fRejectedHits, fTotalHits, fMinTime, fMaxTime );
}

ClassImp(THaMWDCPlane)

///////////////////////////////////////////////////////////////////////////////

