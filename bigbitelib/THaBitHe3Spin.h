//////////////////////////////////////////////////////////////////////////
//
//    THaBitHe3Spin
//
//  Read target state from CAMAC controller control bit input 
//  1 channel as spin + another as spin -
//
//////////////////////////////////////////////////////////////////////////
//    
// Author: Jin Huang <mailto:jinhuang@jlab.org>    Oct 2008
//    Modify History:
//
//////////////////////////////////////////////////////////////////////////


#ifndef ROOT_THaBitHe3Spin
#define ROOT_THaBitHe3Spin

#include "THaHe3SpinModule.h"
#include "THaDetector.h"

//------------------------------------------------------//
//
//    Debug Definitions
//    place this section below any other head files
//
//------------------------------------------------------//
#ifdef DEBUG_LEVEL
#   undef DEBUG_LEVEL
#endif

//    DEBUG_LEVEL;    
//    =0    or not define: no debug, full speed 
//    >=1    enable debug extra warning (suggested setting)
//    >=2    above + enable debug assert
//    >=3    above + enable debug extra info
//    >=4    above + massive info (in a for or while)
#define DEBUG_LEVEL  2

#include    "DebugDef.h"
//------------------------------------------------------//



class THaBitHe3Spin : public THaDetector, public THaHe3SpinModule {

public:

	THaBitHe3Spin( const char* name, const char* description, 
		THaApparatus* a = NULL );
	virtual ~THaBitHe3Spin();
	THaBitHe3Spin() {}  // For ROOT I/O only

	virtual void Clear( Option_t* opt );
	const char* GetDBFileName() const
	{
		// All target related object read from apparatus (he3 target) Database. 
		// keys should (but do not have to) be prefixed by each detector's prefix,
		// e.g. he3.la.SpinDownChan

		return GetApparatus()->GetPrefix();
	}

protected:
	UInt_t   fRawWord;  // raw data word
	static const UInt_t   NULL_RAW_WORD=0xFFFFFFFF;  // raw data word

	UInt_t   fROC;
	UInt_t   fSpinWordHeader;
	UInt_t   fSpinWordOffSet;
	UInt_t   fRawWordConverter;
	UInt_t   fSpinUpMask;
	UInt_t   fSpinDownMask;

	Int_t fPermInvalidEvt;   //Max permissible count of continuous events, which do not contain target signal ADC reading.
	static const Int_t DEFAULT_PERM_INVALID_EVT=0;
	Int_t fMissingEvtCount;

	virtual Int_t Decode( const THaEvData& evdata );

	virtual Int_t DefineVariables( EMode mode = kDefine );
	virtual Int_t ReadDatabase( const TDatime& date );

	ClassDef(THaBitHe3Spin,1);     // Beam helicity from ADC (in time))
};

#endif 

