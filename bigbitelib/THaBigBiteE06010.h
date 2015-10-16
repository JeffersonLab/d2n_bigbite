//////////////////////////////////////////////////////////////////////////
//
// THaBigBiteE06010
//
// specalized class for E06010 bigbite electron package
// Perform the trigger timing corretion
//
//////////////////////////////////////////////////////////////////////////
//    
// Author: Jin Huang <mailto:jinhuang@jlab.org>    Mar 2009
//	Modifications:
//		Apr 2009	Jin Huang <mailto:jinhuang@jlab.org> 
//			added THaBigBiteE06010::MaxADC to ignore events with abnormally large ADC value
//
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_THaBigBiteE06010
#define ROOT_THaBigBiteE06010

#include "THaScintPlaneDet.h"
#include "THaBBTotalShower.h"
#include "THaBBShower.h"
#include "THaBigBite.h"

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
//  >=5 Decode dump  
#define DEBUG_LEVEL   2

#include    "DebugDef.h"
//------------------------------------------------------//


class THaBigBiteE06010 : public THaBigBite {

//------------------------------------------------------//
// Hard coded constants
	public:
	static const UInt_t NSumBars/*	= 26*/;
	static const Double_t TimeWalkCoef[2][26][4];//TimeWalkCoef[side, y<0,y>0][bar][coef list]
	static const Double_t WalkCoefUpLim;
	static const Double_t WalkCoefLowLim;
	static const Double_t MaxADC;	//max adc valuel, an adc beyound will be treated as abnormal event, no time walk correction would be applied
	//void LoadTimeWalkCoef(void);
	
	static const UInt_t BITXCrate/*	= 8*/; 
	static const UInt_t BITXSlot/*	= 17*/; 
	static const UInt_t BIT5Chan/*	= 20*/; 
	static const UInt_t BIT6Chan/*	= 21*/; 
	static const UInt_t BITXLowLim/*	= 200*/;
	static const UInt_t BITXHighLim/*	= 1500*/;

//------------------------------------------------------//

public:
    virtual ~THaBigBiteE06010();
    THaBigBiteE06010( const char* name, const char* description );//constructor

protected:
	THaScintPlaneDet	* fSum;
	THaBBTotalShower	* fTS;
	THaBBShower			* fPS;

	Int_t fDoT5T6Corr, fDoWalkCorr;
	Double_t fArbitaryOffSet;

	//virtual Double_t CalTriggerTimingOffset();
	virtual Double_t GetT5T6TimingOffset(const THaEvData& evdata);
	Double_t fT5_OffSet, fT6_OffSet;

	virtual Double_t GetSumWalkTimingOffset(const THaEvData& evdata);

	virtual Int_t Decode( const THaEvData& evdata );
    virtual Int_t ReadDatabase( const TDatime& date );



private:
    ClassDef(THaBigBiteE06010,0) //ABC for Hall A BigBite
};


#endif



