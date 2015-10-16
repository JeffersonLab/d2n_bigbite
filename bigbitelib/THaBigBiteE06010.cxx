//////////////////////////////////////////////////////////////////////////
//
// THaBigBiteE06010
//
// specalized class for E06010 bigbite electron package
// Perform the trigger timing corretion
//
// pre-loaded with BB.sum and BB.sh to extract trigger timing offsets
// do CoarseProcess of BB.sum and BB.ts before decode of any detector
//
//////////////////////////////////////////////////////////////////////////
//    
// Author: Jin Huang <mailto:jinhuang@jlab.org>    Mar 2009
//
//////////////////////////////////////////////////////////////////////////


#include "THaTrackingDetector.h"
#include "THaTrack.h"
#include "THaSpectrometer.h"
#include "THaParticleInfo.h"
#include "THaTrackingDetector.h"
#include "THaNonTrackingDetector.h"
#include "THaPidDetector.h"
#include "THaPIDinfo.h"
#include "THaTrack.h"
#include "TClass.h"
#include "TMath.h"
#include "TList.h"
#include "VarDef.h"
#include <cmath>
#include <string>
#include "TMath.h"
#include "TString.h"
#include "THaEvData.h"
#include "THaDetMap.h"
#include "TClonesArray.h"


//put this header file below all other headers
#include "THaBigBiteE06010.h"

using namespace std;


ClassImp(THaBigBiteE06010);


//__________________________________________________________________________
// Hard coded constants

const UInt_t THaBigBiteE06010::BITXCrate	= 8;
const UInt_t THaBigBiteE06010::BITXSlot		= 17;
const UInt_t THaBigBiteE06010::BIT5Chan		= 20;
const UInt_t THaBigBiteE06010::BIT6Chan		= 21;
const UInt_t THaBigBiteE06010::BITXLowLim	= 200;
const UInt_t THaBigBiteE06010::BITXHighLim	= 1500;

const UInt_t THaBigBiteE06010::NSumBars			= 26;
const Double_t THaBigBiteE06010::WalkCoefUpLim	= 15;
const Double_t THaBigBiteE06010::WalkCoefLowLim	=-15;
const Double_t THaBigBiteE06010::MaxADC			= 5000;	//max adc valuel an adc beyound will be treated as abnormal event, no time walk correction would be applied
const Double_t THaBigBiteE06010::TimeWalkCoef[2][26][4]=
{{
	{0,     1702.78,        1756.13,        -0.0039235},
 {1,     1912.53,        1968.87,        -0.00356444},
 {2,     2129.13,        2173.1, -0.00242718},
 {3,     2337.85,        2382.91,        -0.00229508},
 {4,     2542.99,        2594.78,        -0.00237353},
 {5,     2751.95,        2803.25,        -0.00220382},
 {6,     2956.05,        3016.4, -0.00248458},
 {7,     3168.29,        3222.28,        -0.0020769},
 {8,     3381.82,        3426.95,        -0.00177658},
 {9,     3583.1, 3642.19,        -0.0021479},
 {10,    3790.08,        3853.19,        -0.00219856},
 {11,    4000.72,        4060.07,        -0.00193041},
 {12,    4207.2, 4270.21,        -0.00193392},
 {13,    4420.54,        4472.64,        -0.0014672},
 {14,    4620.82,        4688.29,        -0.00194445},
 {15,    4848.14,        4881.82,        -0.000909538},
 {16,    5033.82,        5110.47,        -0.00201267},
 {17,    5256.44,        5306.06,        -0.00120959},
 {18,    5453.79,        5524.58,        -0.00167442},
 {19,    5660.64,        5734.87,        -0.00172417},
 {20,    5865.06,        5947.46,        -0.00183558},
 {21,    6076.77,        6152.7, -0.00161694},
 {22,    6296.48,        6349.07,        -0.00120675},
 {23,    6506.81,        6553.4, -0.00104854},
 {24,    6712.46,        6767.39,        -0.00129837},
 {25,    6917.53,        6976.9, -0.00122456}
},{
	{0,     7132.56,        7176.39,        -0.000784117},
 {1,     7335.32,        7387.53,        -0.000913718},
 {2,     7547.9, 7589.64,        -0.000703784},
 {3,     7752.8, 7798.63,        -0.000722711},
 {4,     7956.59,        8009.03,        -0.00081919},
 {5,     8169.07,        8210.6, -0.00062083},
 {6,     8372.52,        8420.91,        -0.000727519},
 {7,     8578.86,        8629.18,        -0.000728004},
 {8,     8788.08,        8836.07,        -0.000739783},
 {9,     8995.93,        9044.05,        -0.000722981},
 {10,    9204.17,        9252.07,        -0.000723424},
 {11,    9411.55,        9461.09,        -0.00072921},
 {12,    9614.16,        9675.16,        -0.000871593},
 {13,    9827.4, 9879.2, -0.000734697},
 {14,    10035.3,        10087.6,        -0.000727444},
 {15,    10254.5,        10287.4,        -0.00043272},
 {16,    10440.7,        10516.7,        -0.000991015},
 {17,    10648.5,        10725.8,        -0.000979576},
 {18,    10858.4,        10932.8,        -0.000910476},
 {19,    11069.5,        11138.6,        -0.000825562},
 {20,    11273,  11351.5,        -0.000927791},
 {21,    11484.1,        11557.4,        -0.000844406},
 {22,    11704.5,        11753.2,        -0.000607997},
 {23,    11913.1,        11958.1,        -0.000547868},
 {24,    12109.5,        12173.4,        -0.000755972},
 {25,    12334.7-25.5,        12386.2,        -0.000912365}
}};



//__________________________________________________________________________

THaBigBiteE06010::~THaBigBiteE06010()
{
}

//__________________________________________________________________________

THaBigBiteE06010::THaBigBiteE06010( const char* name, const char* description ) 
:   THaBigBite( name, description )
{
	//directly loaded with BB.sum and BB.sh to extract trigger timing offsets
	DEBUG_LEVEL_RELATED_PERFORMACE_CHECKER;

	DEBUG_HALL_A_ANALYZER_DEBUGER_INIT;

	AddDetector(fSum = new THaScintPlaneDet( "sum", "BB Total sum",this));
	assert(fSum);
	AddDetector(fTS = new THaBBTotalShower( "ts", "BB Total shower",this));
	assert(fTS);
	fPS=fTS->GetPreShower();
	assert(fPS);

	fDoT5T6Corr = fDoWalkCorr = 0;
	fT5_OffSet = fT6_OffSet = fArbitaryOffSet = 0;

} 

//__________________________________________________________________________
Int_t THaBigBiteE06010::ReadDatabase(const TDatime& date)
{

	DEBUG_INFO(Here("ReadDatabase"),"Start ReadDatabase ...");
	
	fIsInit = kFALSE;
	Int_t status = THaBigBite::ReadDatabase(date);
	if (status!=kOK){
		Error(Here("ReadDatabase"),"THaBigBite::ReadDatabase(date) fail = %d",status);
		return status;
	}


	//vector<vector<Int_t> > *sumtwyplus = new vector<vector<Int_t> >;
	//vector<vector<Int_t> > *sumtwyminus = new vector<vector<Int_t> >;
	//assert(sumtwyplus);
	//assert(sumtwyminus);

	DEBUG_INFO(Here("ReadDatabase"),"OpenFile Database ...");
	FILE* file = OpenFile( date );
	if( !file ) {
		Error(Here("ReadDatabase"),"OpenFile( date ) fail = %d",kFileError);
		return kFileError;
	}
	
	fDoT5T6Corr = fDoWalkCorr = 0;
	fT5_OffSet = fT6_OffSet = fArbitaryOffSet = 0;

	//struct DBRequest {
	//	const char*      name;     // Key name
	//	void*            var;      // Pointer to data (default to Double*)
	//	VarType          type;     // (opt) data type (see VarType.h, default Double_t)
	//	UInt_t           nelem;    // (opt) number of array elements (0/1 = 1 or auto)
	//	Bool_t           optional; // (opt) If true, missing key is ok
	//	Int_t            search;   // (opt) Search for key along name tree
	//	const char*      descript; // (opt) Key description (if 0, same as name)
	//};
	DBRequest request[] =
	{
		{ "DoT5T6Corr",			&fDoT5T6Corr,	kInt,	0, 1, 1 },
  { "DoWalkCorr",			&fDoWalkCorr,	kInt,	0, 1, 1 },
		//{ "SumTWYPlus",			sumtwyplus,		kIntM,	3, 0, 1 },
		//{ "SumTWYMinus",		sumtwyminus,	kIntM,	3, 0, 1 },
  { "T5_OffSet",			&fT5_OffSet,	kDouble,0, 1, 1 },
  { "T6_OffSet",			&fT6_OffSet,	kDouble,0, 1, 1 },
  { "ArbitaryOffSet",			&fArbitaryOffSet,	kDouble,0, 1, 1 },
  { 0 }
	};

	DEBUG_INFO(Here("ReadDatabase"),"LoadDB ...");
	Int_t err = LoadDB( file, date, request/*, "E06010"*/ );
	fclose(file);

	if (err<0)
	{
		Error(Here("ReadDatabase"),"LoadDB( ); fail = %d",err);
		return err;
	}
	else if (err>0)
	{
		DEBUG_INFO(Here("ReadDatabase"),"LoadDB( ); missing keys = %d",err);
	}
	//delete sumtwyplus;
	//delete sumtwyminus;

	fIsInit = kTRUE;

	Info(Here("THaBigBiteE06010"),"Trigger Correction: T5T6Corr=%s; WalkCorr=%s; ArbitaryOffSet=%f ns",
		fDoT5T6Corr>0?"Enabled":"Disabled",
		fDoWalkCorr>0?"Enabled":"Disabled",
  		fArbitaryOffSet);


#if DEBUG_LEVEL>=3//start show info
	TString sDebugOutput;
	sDebugOutput=GetName();
	sDebugOutput+=" Database read in successfully with:";
	sDebugOutput+="\n \t fDoT5T6Corr\t= ";    sDebugOutput+=fDoT5T6Corr;
	sDebugOutput+="\n \t fDoWalkCorr\t= ";    sDebugOutput+=fDoWalkCorr;
	sDebugOutput+="\n \t fT5_OffSet\t= ";    sDebugOutput+=fT5_OffSet;
	sDebugOutput+="\n \t fT6_OffSet\t= ";    sDebugOutput+=fT6_OffSet;
	sDebugOutput+="\n \t fArbitaryOffSet\t= ";    sDebugOutput+=fArbitaryOffSet;
	sDebugOutput+="\n";
	Info(Here("ReadDatabase"),sDebugOutput.Data());
#endif    
	return status;
}
//__________________________________________________________________________
Int_t THaBigBiteE06010::Decode( const THaEvData& evdata )
{
	//do CoarseProcess of BB.sum and BB.ts in this step, before decode of any other detector

	DEBUG_INFO(Here("Decode"),"Start BB.sum Decode ...");
	fSum->Decode(evdata);
	DEBUG_INFO(Here("Decode"),"Start BB.sum CoarseProcess ...");
	fSum->CoarseProcess(*fTracks);

	DEBUG_INFO(Here("Decode"),"Start BB.ts Decode ...");
	fTS->Decode(evdata);
	DEBUG_INFO(Here("Decode"),"Start BB.ts CoarseProcess ...");
	fTS->CoarseProcess(*fTracks);

	fTriggerTimingOffset=
			fArbitaryOffSet
			+GetT5T6TimingOffset(evdata)
			+GetSumWalkTimingOffset(evdata);
	DEBUG_INFO(Here("Decode"),"Get Trigger off Set = %f",fTriggerTimingOffset);
	
	DEBUG_INFO(Here("Decode"),"Start Common Decode ...");
	return THaBigBite::Decode(evdata);
}


//__________________________________________________________________________
Double_t THaBigBiteE06010::GetT5T6TimingOffset(const THaEvData& evdata)
{
	//correct trigger type offsets for early Nov running

	if (fDoT5T6Corr<=0) return 0;

	UInt_t Bit5=0, Bit6=0;

	if (evdata.GetNumHits(BITXCrate, BITXSlot, BIT5Chan))
	{
		Bit5 = evdata.GetData(BITXCrate, BITXSlot, BIT5Chan, 0);
		DEBUG_INFO(Here("GetT5T6TimingOffset"),"Get BB.bit5 = %d",Bit5);
	}
	if (evdata.GetNumHits(BITXCrate, BITXSlot, BIT6Chan))
	{
		Bit6 = evdata.GetData(BITXCrate, BITXSlot, BIT6Chan, 0);
		DEBUG_INFO(Here("GetT5T6TimingOffset"),"Get BB.bit6 = %d",Bit6);
	}

	Double_t offset=0;
	if ( Bit5>BITXLowLim && Bit5<BITXHighLim ) offset+=fT5_OffSet;
	else if ( Bit6>BITXLowLim && Bit6<BITXHighLim ) offset+=fT6_OffSet;

	DEBUG_INFO(Here("GetT5T6TimingOffset"),"GetT5T6TimingOffset() = %f",offset);

	return offset;
}
//__________________________________________________________________________
Double_t THaBigBiteE06010::GetSumWalkTimingOffset(const THaEvData& evdata)
{
	//correct T1 trigger time walk for all running period

	if (fDoWalkCorr<=0) return 0;

	Int_t bar=(Int_t)fSum->GetMaxADCHitBar();
	Double_t adc=fSum->GetMaxADCHit();  //already with ped subtraction
	if (bar < 0 || adc<=0) {
		DEBUG_INFO(Here("GetSumWalkTimingOffset"),"No Shower Sum Hit");
		return 0;
	}

	UInt_t side=-1;
	if (fPS -> GetNclust()==0 ) {
		DEBUG_INFO(Here("GetSumWalkTimingOffset"),"No Shower Sum Hit");
		return 0;
	}
	Double_t max_energy=-1, y=0;
	for (Int_t i=0;i<fPS->GetNclust();i++)
	{
		if (fPS->GetClust(i)->GetE()>max_energy) 
		{
			max_energy = fPS->GetClust(i)->GetE();
			y = fPS->GetClust(i)->GetY();
		}
	}
	if (y>0) side=1;
	else if (y<0) side=0;

	assert(side==0||side==1);
	assert(bar>=0||bar<(Int_t)NSumBars);
	assert(TimeWalkCoef[side][bar][0]==bar);

	Double_t offset=TimeWalkCoef[side][bar][1]
			-TimeWalkCoef[side][bar][2]*pow(adc,TimeWalkCoef[side][bar][3]);
	DEBUG_INFO(Here("GetSumWalkTimingOffset"),"ev#%d: side=%d; bar=%d; adc=%f; Corr=%f",
			   evdata.GetEvNum(),side,bar,adc,offset);
	
	if ( offset>WalkCoefUpLim ) 
	{
		offset=WalkCoefUpLim;
		DEBUG_INFO(Here("GetSumWalkTimingOffset"),"Correction over WalkCoefUpLim");

	}
	else if ( offset<WalkCoefLowLim ) 
	{
		offset=WalkCoefLowLim;
		DEBUG_INFO(Here("GetSumWalkTimingOffset"),"Correction over WalkCoefLowLim");
	}

	if (adc>MaxADC)
	{//max adc valuel, an adc beyound will be treated as abnormal event, no time walk correction would be applied
		offset = 0;
		DEBUG_INFO(Here("GetSumWalkTimingOffset"),"adc = %f is larger than MaxADC, offset -> 0", adc);
	}

	DEBUG_INFO(Here("GetSumWalkTimingOffset"),"GetSumWalkTimingOffset() = %f",offset);

	return offset;
}




