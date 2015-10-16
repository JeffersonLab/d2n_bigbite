#ifndef ROOT_TreeSearch_SpecialWirePlane
#define ROOT_TreeSearch_SpecialWirePlane

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// TreeSearch::SpecialWirePlane                                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "Types.h"
#include "THaSubDetector.h"
#include "TClonesArray.h"
#include "TVector2.h"
#include "TMath.h"
#include "WirePlane.h"
#include <vector>
#include <string>
#include <cassert>
#include <functional>

using std::vector;
using std::string;

class TVector2;

namespace TreeSearch {

  class TimeToDistConv;
  class Projection;
  class MWDC;
  class Hit;
  class FitCoord;
  extern const Double_t kBig;

  class SpecialWirePlane : public WirePlane {

  public:
    SpecialWirePlane( const char* name, const char* description = "",
	       THaDetectorBase* parent = 0 );
    // virtual ~SpecialWirePlane();

    virtual Int_t   Decode( const THaEvData& );
  
  protected:
    
    UInt_t fSpecial;

/*     // Geometry, configuration */

/*     UInt_t      fPlaneNum;     // Ordinal of this plane within its projection */
/*     EProjType   fType;         // Plane type (x,y,u,v) */
/*     Double_t    fWireStart;    // Position of 1st wire (along wire coord) (m) */
/*     Double_t    fWireSpacing;  // Wire spacing (assumed constant) (m) */
/*     Double_t    fCoordOffset;  // Wire coord offset wrt MWDC due to fOrigin */
/*     SpecialWirePlane*  fPartner;      //! Partner plane (usually with staggered wires) */
/*     Projection* fProjection;   //! The projection that we belong to */
/*     MWDC*       fMWDC;         //! Our parent detector */

/*     // Parameters, calibration, flags */

/*     Double_t    fResolution;   // Drift distance resolution (sigma) (m) */
/*     Double_t    fMinTime;      // Minimum drift time for a hit (s) */
/*     Double_t    fMaxTime;      // Maximum drift time for a hit (s) */
/*     UInt_t      fMaxHits;      // Maximum # hits before flagging decode error */

/*     TimeToDistConv* fTTDConv;   // Drift time->distance converter */
/*     vector<float>   fTDCOffset; // [fNelem] TDC offsets for each wire */

/*     // Event data, hits etc. */

/*     TClonesArray*   fHits;      // Hit data */
/*     TClonesArray*   fFitCoords; // Hit coordinates used by good fits in roads */

/* #ifdef TESTCODE */
/*     UInt_t          fNmiss;     // Statistics: Decoder channel misses */
/*     UInt_t          fNrej;      // Statistics: Rejected hits */
/*     Int_t           fWasSorted; // Statistics: hits were sorted (0/1) */
/*     UInt_t          fNhitwires; // Statistics: wires with one or more hits */
/*     UInt_t          fNmultihit; // Statistics: wires with multiple hits */
/*     UInt_t          fNmaxmul;   // Statistics: largest num hits on any wire */
/*     UInt_t          fNcl;       // Statistics: number of hit "clusters" */
/*     UInt_t          fNdbl;      // Statistics: num wires with neighboring hits */
/*     UInt_t          fClsiz;     // Statistics: max cluster size */
/* #endif */

/*     // Bits for SpecialWirePlanes */
/*     enum { */
/*       kIsRequired  = BIT(14), // Tracks must have a hit in this plane */
/*       kCalibrating = BIT(15)  // Plane in calibration mode (implies !required) */
/*     }; */

    //ClassDef(SpecialWirePlane,1)      // One MWDC wire plane
  };

 

} // end namespace TreeSearch


#endif
