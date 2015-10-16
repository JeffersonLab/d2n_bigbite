#ifndef ROOT_BBDecData
#define ROOT_BBDecData

//////////////////////////////////////////////////////////////////////////
//
// BBDecData
//
//////////////////////////////////////////////////////////////////////////

#include "THaApparatus.h"
#include "TBits.h"
//#include "TH1.h"
//#include "TH2.h"
#include <vector>
#include "THashTable.h"

class TH1F;
class BBdataLoc;

class BBDecData : public THaApparatus {

public:
    BBDecData( const char* name="D", const char* description="" );
    virtual ~BBDecData();

    virtual EStatus Init( const TDatime& run_time );
    virtual Int_t   End(THaRunBase* r=0);
    virtual Int_t   Reconstruct() { return 0; }
    virtual Int_t   Decode( const THaEvData& );

    void Reset( Option_t* opt="" );

protected:
    virtual BBdataLoc* DefineChannel(BBdataLoc*, EMode, const char* desc="automatic");

private:
    TBits  bits;
    UInt_t evtypebits, evtype;
    Double_t ctimel, ctimer;
    Double_t pulser1;
    UInt_t synchadc1, synchadc2, synchadc3, 
        synchadc4, synchadc14;
    UInt_t timestamp, timeroc1, timeroc2, timeroc3,  
        timeroc4, timeroc14;
    Double_t rftime1,rftime2;
    Double_t edtpl,edtpr;
    Double_t lenroc12,lenroc16;
    Int_t  cnt1;
    //FIXME: combine into one collection for better efficiency - and decide on the
    // proper data structure (array vs. list/hashlist etc.)
    std::vector < BBdataLoc* > fCrateLoc;   // Raw Data locations by crate, slot, channel
    std::vector < BBdataLoc* > fWordLoc;    // Raw Data locations relative to header word

    virtual void Clear( Option_t* opt="" );
    virtual void Print( Option_t* opt="" ) const;
    //std::vector<TH1F* > hist;
    Int_t DefaultMap();
    void TrigBits(UInt_t ibit, BBdataLoc *dataloc);
    Int_t SetupDecData( const TDatime* runTime = NULL, EMode mode = kDefine );

    static BBDecData* fgThis;   //Pointer to instance of this class

    ClassDef(BBDecData,0)  
};

#endif








