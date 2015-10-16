#ifndef ROOT_BBNormAna
#define ROOT_BBNormAna

//////////////////////////////////////////////////////////////////////////
//
// BBNormAna
//
//  Normalization Analysis
//
//////////////////////////////////////////////////////////////////////////

#include "THaPhysicsModule.h"
#include "TBits.h"
#include <vector>
#include <string>
#include <cstring>
#include <iostream>

#define NTRIG 12

using namespace std;

class TH1;
class THaScaler;
class THaADCHelicity;

class BRocScaler {
// Container class of ROC scaler data in BBNormAna.  All data public, so
// it is like a "struct".
 public:
  BRocScaler(Int_t ir, char *nm, Int_t headpp, Int_t headnp, Int_t headpn, Int_t headnn, Int_t headall, Int_t ch, Double_t *d) : roc(ir), chan_name(nm), header_PP(headpp), header_NP(headnp), header_PN(headpn), header_NN(headnn), header_ALL(headall), chan(ch) { data = d; }
  Int_t roc;                 // roc num you want
  char *chan_name;           // name of channel
  Int_t header_PP, header_NP, header_PN, header_NN, header_ALL;  // headers of + & - helicity data
  Int_t chan;                // channel in scaler
  Double_t *data;            // pointer to data
};

class BNormData {
// Container class used by BBNormAna
public:
  BNormData() {
    evcnt = new Int_t[5];
    avglive = new Double_t[5];
    corrfact = new Double_t[5];
    trigcnt = new Int_t[5*NTRIG];
    live = new Double_t[5*NTRIG];
    psfact = new Int_t[NTRIG];
    memset(evcnt,0,5*sizeof(Int_t));
    memset(avglive,0,5*sizeof(Double_t));
    memset(corrfact,0,5*sizeof(Double_t));
    memset(trigcnt,0,5*NTRIG*sizeof(Int_t));
    memset(live,0,5*NTRIG*sizeof(Double_t));
    memset(psfact,0,NTRIG*sizeof(Int_t));
  }
  virtual ~BNormData() {
    delete [] evcnt;
    delete [] avglive;
    delete [] corrfact;
    delete [] trigcnt;
    delete [] live;
    delete [] psfact;
  }
  void Print(Int_t ihel=0) {
    cout << "Print of normdata";
    cout << " for helicity "<<ihel<<endl; 
    cout << "EvCount "<<GetEvCount(ihel)<<endl;
    cout << "Avg Live "<<GetAvgLive(ihel)<<endl;
    cout << "Correction "<<GetCorrfact(ihel)<<endl;
    for (int itrig = 0; itrig < 12; itrig++) {
      cout << "Trig "<<itrig+1;
      cout << "  PS "<<GetPrescale(itrig);
      cout << "  Latch "<< GetTrigCount(itrig,ihel);
      cout << "  Live "<<GetLive(itrig,ihel)<<endl;
    }
  }
  Int_t InvHel(Int_t hel) {
// hel = (1,0,2)  maps to -1, 0, +1  
// 0 = no helicity in either case
    if (hel == 1) return -1;
    if (hel == 2) return 1;
    if (hel == 3) return -2;
    if (hel == 4) return 2;
    return 0;
  }
  void EvCount(int hel=0) { 
  // By my def, hel=0 is irrespective of helicity
    evcnt[0]++;
    if (hel!=0) evcnt[Hel(hel)]++; 
  }
  void TrigCount(int trig, int hel=0) {
  // By my def, hel=0 is irrespective of helicity
    trigcnt[Trig(trig)]++;
    if (hel!=0) trigcnt[NTRIG*Hel(hel)+Trig(trig)]++;
  }
  void SetLive(Int_t trig, Double_t ldat, Int_t hel=0) {
     live[NTRIG*Hel(hel)+Trig(trig)] = ldat;
  }
  void SetPrescale(Int_t trig, Int_t ps) {
     psfact[Trig(trig)] = ps;
  }
  void SetAvgLive(Double_t alive, Double_t corr, 
             Int_t hel=0) {
     avglive[Hel(hel)] = alive;
     corrfact[Hel(hel)] = corr;
  }
  Double_t GetAvgLive(Int_t hel=0) {
     return avglive[Hel(hel)];
  }
  Double_t GetCorrfact(Int_t hel=0) {
     return corrfact[Hel(hel)];
  }
  Double_t GetLive(Int_t trig, Int_t hel=0) {
     return live[NTRIG*Hel(hel)+Trig(trig)];
  }
  Double_t GetPrescale(Int_t trig) {
      return (Double_t)psfact[Trig(trig)];
  }
  Double_t GetEvCount(Int_t hel=0) {
    return (Double_t)evcnt[Hel(hel)];
  }
  Double_t GetTrigCount(Int_t trig, Int_t hel=0) {
    return (Double_t)trigcnt[NTRIG*Hel(hel)+Trig(trig)];
  }
private:
  Int_t *evcnt, *trigcnt, *psfact;
  Double_t *avglive, *corrfact, *live;
  Int_t Trig(Int_t idx) {
// Trigger indices are 0,1,2... for T1,T2,..
    if (idx >= 0 && idx < NTRIG) return idx;
    return 0;
  }
  Int_t Hel(Int_t hel) {
// hel = -1, 0, +1  maps to (1,0,2)  
// 0 = no helicity in either case
    if (hel == -1) return 1;
    if (hel == 1) return 2;
    if (hel == 2) return 4;
    if (hel == -2) return 3;
    return 0;
  }
};


class BBNormAna : public THaPhysicsModule {
  
public:
   BBNormAna( const char* name="N", const char* description="Normalizion Analysis" ,const char* filename="scaler%d.log",const Int_t maxt=12);
   virtual ~BBNormAna();

   // Int_t CmpNoCase( const string& r, const string& s );

   virtual EStatus Init( const TDatime& run_time );
   virtual Int_t   End(THaRunBase* r=0);
   virtual void    WriteHist(); 
   virtual void    DoBcmCalib() { fDoBcmCalib = kTRUE; };
   virtual void    SetEventInt(Int_t *ev);
   virtual Int_t   Reconstruct() { return 0; }
   virtual Int_t   Process( const THaEvData& );
   virtual Int_t   PrintSummary() const;

private:

   TBits  bits;
   
   THaScaler *myscaler;
   THaADCHelicity *myhelicity;
   BNormData *normdata;
   std::vector<BRocScaler*> fRocScaler;
   Bool_t fSetPrescale,fHelEnable,fDoBcmCalib;
   UInt_t evtypebits;    // trigger bit pattern
   Int_t *tdcdata, *nhit;   // tdc for trigger bit

   Double_t alive,hpos_pos_alive,hpos_neg_alive,hneg_pos_alive,hneg_neg_alive; // scaler livetime

   Double_t dlive;   // differential livetime (one helicity)
   Double_t bcmu3;       // a BCM (rate)
   Int_t *eventint;      // event intervals for BCM calib
   // scaler data from roc10 or 11 (counts)
   Double_t roc11_bcmu3, roc11_bcmu10;
   Double_t roc11_t1,roc11_t2,roc11_t3,roc11_t4,roc11_t5;
   Double_t roc11_clk1024, roc11_clk104k;

   Double_t *norm_scaler, *norm_plus_plus, *norm_plus_minus, *norm_minus_plus, *norm_minus_minus;

   // current calibration:
   Double_t off_u1, off_u3, off_u10;
   Double_t off_d1, off_d3, off_d10;
   Double_t calib_u1, calib_u3, calib_u10;
   Double_t calib_d1, calib_d3, calib_d10;

   virtual void Print( Option_t* opt="" ) const;
   std::vector<TH1* > hist;
   void TrigBits( Int_t helicity );
   void LiveTime();
   void CalcAsy();
   void GetRocScalers(const THaEvData& );
   Int_t SetupRawData( const TDatime* runTime = NULL, EMode mode = kDefine );
   void InitRocScalers();
   virtual Int_t BcmCalib( const THaEvData& );
   virtual void BookHist(); 

   static const int fgMaxEvInt = 100;
   static const int fgNumRoc = 2;
   static const int fgNumChan = 32;

   static const int fDEBUG = 0;

  ClassDef(BBNormAna,0)  // Normalization analysis utilities
};

#endif








