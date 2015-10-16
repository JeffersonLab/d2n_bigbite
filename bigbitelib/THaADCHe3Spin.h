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


#ifndef ROOT_THaADCHe3Spin
#define ROOT_THaADCHe3Spin

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
#define DEBUG_LEVEL   2

#include    "DebugDef.h"
//------------------------------------------------------//



class THaADCHe3Spin : public THaDetector, public THaHe3SpinModule {

public:

	THaADCHe3Spin( const char* name, const char* description, 
		THaApparatus* a = NULL );
	virtual ~THaADCHe3Spin();
	THaADCHe3Spin() {}  // For ROOT I/O only

	virtual void Clear( Option_t* opt );
	const char* GetDBFileName() const
	{
		// All target related object read from apparatus (he3 target) Database. 
		// keys should (but do not have to) be prefixed by each detector's prefix,
		// e.g. he3.la.SpinDownChan

		return GetApparatus()->GetPrefix();
	}

	inline Int_t ADC2Logic(Double_t rawadc)
	{//0 as logic 0; 1 as logic 1; -1 as error
		Bool_t b1=(rawadc<=fLogic1Window.UpBound && rawadc>=fLogic1Window.LowBound);
		Bool_t b0=(rawadc<=fLogic0Window.UpBound && rawadc>=fLogic0Window.LowBound);

		return (b1==b0)?-1:b1;
	}
protected:
	Double_t   fADCRawUp;  // ADC raw data for spin up channel
	Double_t   fADCRawDown;  // ADC raw data for spin down channel

	struct ADCStateBound{
		Double_t UpBound; 
		Double_t LowBound; 
	} fLogic1Window, fLogic0Window;//cut windows

	// Simplified detector map for the two data channels
	struct ChanDef_t {
		Int_t roc;            // ROC to read out
		Int_t slot;           // Slot of module
		Int_t chan;           // Channel within module
	} fSpinUpChan, fSpinDownChan; //ADC Channels

	Int_t fPermInvalidADC;   //Max permissible count of continuous events, which do not contain target signal ADC reading.
	static const Int_t DEFAULT_PERM_INVALID_ADC=3;
	Int_t fMissingADCCount;

	virtual Int_t Decode( const THaEvData& evdata );
	virtual Double_t GetData(const ChanDef_t &c,const THaEvData& evdata );

	virtual Int_t DefineVariables( EMode mode = kDefine );
	virtual Int_t ReadDatabase( const TDatime& date );

	ClassDef(THaADCHe3Spin,1);     // Beam helicity from ADC (in time))
};

#endif 

