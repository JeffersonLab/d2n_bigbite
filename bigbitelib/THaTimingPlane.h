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

#ifndef ROOT_THaTimingPlane
#define ROOT_THaTimingPlane

#include "THaNonTrackingDetector.h"
#include "TClonesArray.h"

//------------------------------------------------------//
//
//	Debug Definitions
//	place this section below any other head files
//
//------------------------------------------------------//
#ifdef DEBUG_LEVEL
#   undef DEBUG_LEVEL
#endif

//	DEBUG_LEVEL;	
//	=0	or not define: no debug, full speed 
//	>=1	enable debug extra warning (suggested setting)
//	>=2	above + enable debug assert
//	>=3	above + enable debug extra info
//	>=4	above + massive info (in a for or while)
//  >=5 above + read database dump
#define DEBUG_LEVEL   5

#include    "DebugDef.h"
//------------------------------------------------------//

class THaScintPlane;
class THaScintPMT;
class THaScintHit;
class THaScintBar;

class THaTimingPlane : public THaNonTrackingDetector {

public:
	virtual ~THaTimingPlane();
	//THaTimingPlane( const char* name, const char* description,
	//			  THaApparatus* a = NULL);
	THaTimingPlane( const char* name, const char* description, THaApparatus* a = NULL  );

	Int_t          Decode( const THaEvData& );
	EStatus        Init( const TDatime& run_time);
	void           SetApparatus( THaApparatus* );

	virtual Int_t      CoarseProcess( TClonesArray& tracks );
	virtual Int_t      FineProcess( TClonesArray& tracks );


	THaScintPlane* GetEPlane() const { return fEplane; }
	THaScintPlane* GetdEPlane() const { return fdEplane; }

protected:
	// Subdetectors
	THaScintPlane* fEplane;     //E plane subdetector
	THaScintPlane* fdEplane;    //dE plane subdetector


	THaTimingPlane& operator=( const THaTimingPlane& ) {return *this; }

	virtual Int_t  DefineVariables( EMode mode = kDefine );
	virtual Int_t  ReadDatabase( const TDatime& date );
	//function for jump through lines starting with #
	char* ReadNumberSignStartComment( FILE* fp, char *buf, const int len );

private:
	void           Setup( const char* name,  const char* desc, 
		const char* enam, const char* denam,
		bool mode );


	ClassDef(THaTimingPlane,1)//Trigger plane class, consist of dE and E plane
};

#endif
