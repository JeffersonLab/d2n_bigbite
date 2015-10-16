//*-- Author :    Ole Hansen, Jefferson Lab   14-Jun-2007

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// TreeSearch::SpecialWirePlane                                                    //
//                                                                          //
//                                                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "SpecialWirePlane.h"
#include "Hit.h"
#include "MWDC.h"
#include "TimeToDistConv.h"
#include "Projection.h"

#include "THaDetMap.h"
#include "THaEvData.h"
#include "TClonesArray.h"

#include "TClass.h"

#include <iostream>
#include <string>

using namespace std;

// Database uses ns for TDC offsets
static const float kTDCscale = 1e-9;

namespace TreeSearch {

//_____________________________________________________________________________
SpecialWirePlane::SpecialWirePlane( const char* name, const char* description,
		      THaDetectorBase* parent )
  : WirePlane(name,description,parent)
{
  return;
}


// //_____________________________________________________________________________
// SpecialWirePlane::~SpecialWirePlane()
// {
//   // Destructor.

//   if( fIsSetup )
//     RemoveVariables();

//   delete fFitCoords;
//   delete fHits;
// }


//_____________________________________________________________________________
 Int_t SpecialWirePlane::Decode( const THaEvData& evData )
  {    
    // Extract this plane's hit data from the raw evData.
    //
    // This routine can handle both the old Fastbus readout and the new CAEN
    // VME pipeline TDCs. The latter require a reference channel map and
    // cross-references to reference channels in the regular detector map
    // of the plane.
    
    //  static const char* const here = "Decode";
    
    UInt_t nHits = 0;
    bool no_time_cut = fMWDC->TestBit(MWDC::kDoTimeCut) == kFALSE;
    //bool mc_data     = fMWDC->TestBit(MWDC::kMCdata);
    
    // Decode data. This is done fairly efficiently by looping over only the 
    // channels with hits on each module. 
    // FIXME: If a module is shared with another plane (common here), we waste
    // time skipping hits that don't belong to us.
    // NB: certain indices below are guaranteed to be in range by construction
    // in ReadDatabase, so we can avoid unneeded checks.
    Float_t localx = ((THaBBShower*)(fMWDC->fshower))->GetX(0);
    Float_t localy = ((THaBBShower*)(fMWDC->fshower))->GetY(0);
    Int_t iw=0;
    Double_t time = 1.E-8;
    
    //for (Int_t i=1;i!=2;i++){
      if (GetType()==2){
	iw = Int_t((localx - GetWireStart())/GetWireSpacing());
	//cout << localy << "\t" << GetWireStart() + iw * GetWireSpacing() << endl;
      }else if (GetType()==0){
	iw = Int_t((localx*sqrt(3.)/2.+localy/2. - GetWireStart())/GetWireSpacing());
      }else if (GetType()==1){
	iw = Int_t((localx*sqrt(3.)/2.-localy/2. - GetWireStart())/GetWireSpacing());
      }
      

      if( no_time_cut || (fMinTime < time && time < fMaxTime) ) {
	Hit* theHit;
	
	theHit = new( (*fHits)[nHits++] )
	  Hit( iw, 
	       GetWireStart() + iw * GetWireSpacing(),
	       1,
	       time,
	       GetResolution(),1,
	       this
	       );
	// Preliminary calculation of drift distance. Once tracks are known,
	// the distance can be recomputed using the track slope.
	theHit->ConvertTimeToDist( 0.0 );
	
	//cout << theHit->ConvertTimeToDist( 0.0 ) << endl;
	
	// We can test the ordering of the hits on the fly - they should
	// come in sorted if the lowest logical channel corresponds to
	// the smallest wire positiion. If they do, we can skip 
	// quicksorting an already-sorted array ;)
	// }
    }
    

    return nHits;
  }

}

//ClassImp(TreeSearch::SpecialWirePlane)

///////////////////////////////////////////////////////////////////////////////

