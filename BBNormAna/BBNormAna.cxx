//*-- Author :  R. Michaels, May 2004
//
//////////////////////////////////////////////////////////////////////////
//
// BBNormAna
//
// Code to summarize the information needed 
// to normalize an experiment and obtain a 
// cross section, including
//
// 1. Prescale factors
// 2. Calibration of charge monitor
// 3. Integrated Charge of a run
// 4. Livetime by trigger type, by helicity, and 
//    an approximate average livetime.
// 5. Number of triggers from bit pattern.
//
// This class requires scalers to be enabled
// in the analyzer.  For scalers, see 
// http://hallaweb.jlab.org/equipment/daq/THaScaler.html
//
// At present the run summary is just a printout,
// eventually one could write to a database.
// This needs some work (hopefully a student will do it).
// In addition, some global variables are
// registered for possible Tree analysis, e.g.
// cutting on helicity-correlated deadtime.
//
// How to use this class:
// 
// 1. Do something like this in the analysis script:
//      BBNormAna* norm = 
//        new BBNormAna("N","Normalization Analysis");
//      gHaPhysics->Add(norm);
// 
// 2. Add variables N.* to the output tree.
//
// 3. BBNormAna will produce an end-of-run printout.
//
// 4. Calibrate scaler BCMs versus EPICS (assumes EPICS
//    is accurate & reliable) as follows: 
//    First, before analyzing a run
//       normana->DoBcmCalib().
//    Do a CODA run with several beam currents, staying at
//    each current setting for 5 minutes.
//    Look at N.bcmu3 vs fEvtHdr.fEvtNum to decide cut 
//    intervals of stable beam, and analyze again after
//       normana->SetEventInt(Int_t *event)
//    (see method for documentation).
//    Fit the 2D histograms of bcm vs EPICS in a macro 
//    to get the parameters.   
//
//////////////////////////////////////////////////////////////////////////
//23rd March 2009 : Scaler reading from BBITE 

//#define WITH_DEBUG 1

#include "BBNormAna.h"
#include "THaVarList.h"
#include "THaVar.h"
#include "THaGlobals.h"
#include "THaAnalyzer.h"
#include "THaScalerGroup.h"
#include "THaString.h"
#include "TList.h"
#include "THaEvData.h"
#include "TDatime.h"
#include "THaScaler.h"
#include "THaApparatus.h"
#include "THaADCHelicity.h"
#include "THaEvent.h"
#include "TMath.h"
#include "TH1.h"
#include "TH2.h"
#include "VarDef.h"
#include <fstream>
#include <iostream>

using namespace std;
using THaString::CmpNoCase;
//Int_t runnum;
char fname[200];
Int_t maxtrig;
//_____________________________________________________________________________
BBNormAna::BBNormAna( const char* name, const char* descript ,const char* filename ,const Int_t maxt) :
  THaPhysicsModule( name, descript )
{
  fSetPrescale = kFALSE;
  fHelEnable = kFALSE;
  fDoBcmCalib = kFALSE;
  myscaler = 0;
  normdata = new BNormData();
  // There are 12 triggers
  nhit = new Int_t[12];
  // There are 6 hits possible on a TDC channel
  tdcdata = new Int_t[6*12];
  alive = 0;
  // runnum=runnumber;
  maxtrig=maxt;
  sprintf(fname,filename);
  cout<<fname;
  hpos_pos_alive = 0;
  hneg_pos_alive = 0;
  hpos_neg_alive = 0;
  hneg_neg_alive = 0;
  //  charge_u1 = 0;

  roc11_bcmu3 = 0;
  roc11_bcmu10 = 0;
  roc11_t1 = 0;
  roc11_t2 = 0;
  roc11_t3 = 0;
  roc11_t4 = 0;
  roc11_t5 = 0;
  roc11_clk1024 = 0;
  roc11_clk104k = 0;
  eventint = new Int_t[fgMaxEvInt];
  memset(eventint, 0, fgMaxEvInt*sizeof(Int_t));
  norm_scaler = new Double_t[fgNumRoc * fgNumChan];   
  norm_plus_plus = new Double_t[fgNumRoc * fgNumChan];   
  norm_minus_plus = new Double_t[fgNumRoc * fgNumChan];   
  norm_plus_minus = new Double_t[fgNumRoc * fgNumChan];   
  norm_minus_minus = new Double_t[fgNumRoc * fgNumChan];  

  for (int i = 0; i < fgNumRoc*fgNumChan; i++) {
      norm_scaler[i] = 0; norm_plus_plus[i] = 0; norm_minus_plus[i] = 0; norm_minus_plus[i] = 0; norm_minus_minus[i] = 0;
  }
  InitRocScalers();
}

//_____________________________________________________________________________
BBNormAna::~BBNormAna()
{

  delete normdata;
  delete [] nhit;
  delete [] tdcdata;
  delete [] eventint;
  delete [] norm_scaler;
  delete [] norm_plus_plus;
  delete [] norm_minus_plus;
  delete [] norm_plus_minus;
  delete [] norm_minus_minus;

  for (vector<BRocScaler*>::iterator ir = fRocScaler.begin();
       ir != fRocScaler.end(); ir++) delete *ir;
  SetupRawData( NULL, kDelete ); 

}

//_____________________________________________________________________________
Int_t BBNormAna::SetupRawData( const TDatime* run_time, EMode mode )
{
  Int_t retval = 0;

  RVarDef vars[] = {
    { "evtypebits", "event type bit pattern",      "evtypebits" },  
    // This first part is based on event type 140 and is "older" code.
    { "bcmu3",   "bcm upstream with x3 gain",      "bcmu3" },
    { "alive",   "approx average livetime from scaler", "alive" },
    { "dlive",   "differential livetime from scaler ",  "dlive" },  
    { "hpos_pos_alive", "helicity positive target positive livetime", "hpos_pos_alive" },  
    { "hneg_pos_alive", "helicity positive target negative livetime", "hneg_pos_alive" }, 
    { "hpos_neg_alive", "helicity positive target positive livetime", "hpos_neg_alive" },  
    { "hneg_neg_alive", "helicity negative target negative livetime", "hneg_neg_alive" }, 
    //    { "charge_u1", "charge u1", "charge_u1" }, 

    // Now we add roc11 data, it is "newer" code.
    { "roc11_bcmu3", "ROC11 BCM upsgream, gain 3", "roc11_bcmu3" },  
    { "roc11_bcmu10", "ROC11 BCM upsgream, gain 10", "roc11_bcmu10" },  
    { "roc11_t1", "ROC11 trigger 1 counts", "roc11_t1" },  
    { "roc11_t2", "ROC11 trigger 2 counts", "roc11_t2" },  
    { "roc11_t3", "ROC11 trigger 3 counts", "roc11_t3" },  
    { "roc11_t4", "ROC11 trigger 4 counts", "roc11_t4" },  
    { "roc11_t5", "ROC11 trigger 5 counts", "roc11_t5" },  
    { "roc11_clk1024", "ROC11 1024Hz clock", "roc11_clk1024" },  
    { "roc11_clk104k", "ROC11 nom. 104Khz clock", "roc11_clk104k" },  

    { 0 }
  };

  if( mode != kDefine || !fIsSetup )
    retval = DefineVarsFromList( vars, mode );

  fIsSetup = ( mode == kDefine );
  
  return retval;
}
 
//_____________________________________________________________________________
void BBNormAna::InitRocScalers()
{ // setup the ROC scalers we want to decode.

  fRocScaler.push_back(new BRocScaler(11,"trigger-1",0xabc20000, 0xabc40000, 0xabc30000, 0xabc50000, 0xabc10000, 0, &roc11_t1));
  fRocScaler.push_back(new BRocScaler(11,"trigger-2",0xabc20000, 0xabc40000, 0xabc30000, 0xabc50000, 0xabc10000, 1, &roc11_t2));
  fRocScaler.push_back(new BRocScaler(11,"trigger-3",0xabc20000, 0xabc40000, 0xabc30000, 0xabc50000, 0xabc10000, 2, &roc11_t3));
  fRocScaler.push_back(new BRocScaler(11,"trigger-4",0xabc20000, 0xabc40000, 0xabc30000, 0xabc50000, 0xabc10000, 3, &roc11_t4));
  fRocScaler.push_back(new BRocScaler(11,"trigger-5",0xabc20000, 0xabc40000, 0xabc30000, 0xabc50000, 0xabc10000, 4, &roc11_t5));
  fRocScaler.push_back(new BRocScaler(11,"clock1024",0xabc20000, 0xabc40000, 0xabc30000, 0xabc50000, 0xabc10000, 7, &roc11_clk1024));
  fRocScaler.push_back(new BRocScaler(11,"bcm_u3",0xabc20000, 0xabc40000, 0xabc30000, 0xabc50000, 0xabc10000, 16, &roc11_bcmu3));
  fRocScaler.push_back(new BRocScaler(11,"bcm_u10",0xabc20000, 0xabc40000, 0xabc30000, 0xabc50000, 0xabc10000, 8, &roc11_bcmu10));
}

//_____________________________________________________________________________
Int_t BBNormAna::End( THaRunBase* run ) 
{
  
  PrintSummary();
  WriteHist();
  return 0;
}


//_____________________________________________________________________________
Int_t BBNormAna::PrintSummary() const
{
// Print the summary.  Eventually want this to go to a
// database or a file.

  if (myscaler == 0) {
    cout << "Sorry, no scalers.  Therefore BBNormAna ";
    cout << "failed to work."<<endl;
    return 0;
  }

  // This repeats what is in THaScaler::PrintSummary()
  printf("\n ------------ BBNormAna Scaler Summary   ---------------- \n");
  Double_t clockrate = 1024;
  Double_t time_sec = myscaler->GetPulser("clock")/clockrate;
  Double_t time_sec_pp = myscaler->GetNormData(1,1,"clock",0)/clockrate;
  Double_t time_sec_pn = myscaler->GetNormData(1,-1,"clock",0)/clockrate;
  Double_t time_sec_np = myscaler->GetNormData(-1,1,"clock",0)/clockrate;
  Double_t time_sec_nn = myscaler->GetNormData(-1,-1,"clock",0)/clockrate;

  cout<<"ungate time = "<<time_sec<<"; "<<"pp = "<<time_sec_pp<<"; "<<"pn = "<<time_sec_pn<<"; "
  <<"np = "<<time_sec_np<<"; "<<"nn = "<<time_sec_nn<<endl;

   FILE* outfile;
  //char filename[200];
  // sprintf(filename,"scaler%d.log",runnum);
  outfile=fopen(fname,"w");
 
  if (time_sec == 0) {
     cout << "THaScaler: WARNING:  Time of run = ZERO (\?\?)\n"<<endl;
  } else {
   Double_t time_min = time_sec/60;  
   Double_t curr_u1  = ((Double_t)myscaler->GetBcm("bcm_u1")/time_sec - off_u1)/calib_u1;
   Double_t curr_u3  = ((Double_t)myscaler->GetBcm("bcm_u3")/time_sec - off_u3)/calib_u3;
   Double_t curr_u10 = ((Double_t)myscaler->GetBcm("bcm_u10")/time_sec - off_u10)/calib_u10;
   Double_t curr_d1  = ((Double_t)myscaler->GetBcm("bcm_d1")/time_sec - off_d1)/calib_d1;
   Double_t curr_d3  = ((Double_t)myscaler->GetBcm("bcm_d3")/time_sec - off_d3)/calib_d3;
   Double_t curr_d10 = ((Double_t)myscaler->GetBcm("bcm_d10")/time_sec - off_d10)/calib_d10;

   //hack 0.49 may not right!!!
   // need to check the target negative helicity state!!!
   //   You need to stop hacking around... and do it correctly.
   //   ... and why do you think this is YOURscaler?  it's MYscaler

   Double_t curr_u1_pp  = ((Double_t)myscaler->GetNormData(1,1,"bcm_u1",0)/time_sec_pp - off_u1)/calib_u1;
   Double_t curr_u3_pp  = ((Double_t)myscaler->GetNormData(1,1,"bcm_u3",0)/time_sec_pp - off_u3)/calib_u3;
   Double_t curr_u10_pp = ((Double_t)myscaler->GetNormData(1,1,"bcm_u10",0)/time_sec_pp - off_u10)/calib_u10;
   Double_t curr_d1_pp  = ((Double_t)myscaler->GetNormData(1,1,"bcm_d1",0)/time_sec_pp - off_d1)/calib_d1;
   Double_t curr_d3_pp  = ((Double_t)myscaler->GetNormData(1,1,"bcm_d3",0)/time_sec_pp - off_d3)/calib_d3;
   Double_t curr_d10_pp = ((Double_t)myscaler->GetNormData(1,1,"bcm_d10",0)/time_sec_pp - off_d10)/calib_d10;
  
   Double_t curr_u1_pn  = ((Double_t)myscaler->GetNormData(1,-1,"bcm_u1",0)/time_sec_pn - off_u1)/calib_u1;
   Double_t curr_u3_pn  = ((Double_t)myscaler->GetNormData(1,-1,"bcm_u3",0)/time_sec_pn - off_u3)/calib_u3;
   Double_t curr_u10_pn = ((Double_t)myscaler->GetNormData(1,-1,"bcm_u10",0)/time_sec_pn - off_u10)/calib_u10;
   Double_t curr_d1_pn  = ((Double_t)myscaler->GetNormData(1,-1,"bcm_d1",0)/time_sec_pn - off_d1)/calib_d1;
   Double_t curr_d3_pn  = ((Double_t)myscaler->GetNormData(1,-1,"bcm_d3",0)/time_sec_pn - off_d3)/calib_d3;
   Double_t curr_d10_pn = ((Double_t)myscaler->GetNormData(1,-1,"bcm_d10",0)/time_sec_pn - off_d10)/calib_d10;

   Double_t curr_u1_np  = ((Double_t)myscaler->GetNormData(-1,1,"bcm_u1",0)/time_sec_np - off_u1)/calib_u1;
   Double_t curr_u3_np  = ((Double_t)myscaler->GetNormData(-1,1,"bcm_u3",0)/time_sec_np - off_u3)/calib_u3;
   Double_t curr_u10_np = ((Double_t)myscaler->GetNormData(-1,1,"bcm_u10",0)/time_sec_np - off_u10)/calib_u10;
   Double_t curr_d1_np  = ((Double_t)myscaler->GetNormData(-1,1,"bcm_d1",0)/time_sec_np - off_d1)/calib_d1;
   Double_t curr_d3_np  = ((Double_t)myscaler->GetNormData(-1,1,"bcm_d3",0)/time_sec_np - off_d3)/calib_d3;
   Double_t curr_d10_np = ((Double_t)myscaler->GetNormData(-1,1,"bcm_d10",0)/time_sec_np - off_d10)/calib_d10;
   
   Double_t curr_u1_nn  = ((Double_t)myscaler->GetNormData(-1,-1,"bcm_u1",0)/time_sec_nn - off_u1)/calib_u1;
   Double_t curr_u3_nn  = ((Double_t)myscaler->GetNormData(-1,-1,"bcm_u3",0)/time_sec_nn - off_u3)/calib_u3;
   Double_t curr_u10_nn = ((Double_t)myscaler->GetNormData(-1,-1,"bcm_u10",0)/time_sec_nn - off_u10)/calib_u10;
   Double_t curr_d1_nn  = ((Double_t)myscaler->GetNormData(-1,-1,"bcm_d1",0)/time_sec_nn - off_d1)/calib_d1;
   Double_t curr_d3_nn  = ((Double_t)myscaler->GetNormData(-1,-1,"bcm_d3",0)/time_sec_nn - off_d3)/calib_d3;
   Double_t curr_d10_nn = ((Double_t)myscaler->GetNormData(-1,-1,"bcm_d10",0)/time_sec_nn - off_d10)/calib_d10;

   //   charge_u1 = curr_u1*time_sec; 
 
   printf("Time of run  %7.2f min \n",time_min);
   fprintf(outfile,"Time of run  %7.2f min \n",time_min);
   
   printf("Triggers:     1 = %d    2 = %d    3 = %d   4 = %d    5 = %d\n",
         myscaler->GetTrig(1),
         myscaler->GetTrig(2),
         myscaler->GetTrig(3),
         myscaler->GetTrig(4),
         myscaler->GetTrig(5)
         );

   fprintf(outfile,"Triggers:     1 = %d    2 = %d    3 = %d   4 = %d    5 = %d\n",
         myscaler->GetTrig(1),
         myscaler->GetTrig(2),
         myscaler->GetTrig(3),
         myscaler->GetTrig(4),
         myscaler->GetTrig(5)
         );

   printf("Accepted triggers:   %d \n",myscaler->GetNormData(0,"TS-accept"));
   fprintf(outfile,"Accepted triggers:   %d \n",myscaler->GetNormData(0,"TS-accept"));

   printf("Accepted triggers by target and helicity state:\n");
   fprintf(outfile,"Accepted triggers by target and helicity state:\n");

   printf("  (++) = %d    (+-) = %d   (-+) = %d    (--) = %d\n",
	  myscaler->GetNormData(1,1,"TS-accept"),myscaler->GetNormData(1,-1,"TS-accept"),myscaler->GetNormData(-1,1,"TS-accept"),myscaler->GetNormData(-1,-1,"TS-accept"));
   fprintf(outfile,"  (++) = %d    (+-) = %d   (-+) = %d    (--) = %d\n",
	  myscaler->GetNormData(1,1,"TS-accept"),myscaler->GetNormData(1,-1,"TS-accept"),myscaler->GetNormData(-1,1,"TS-accept"),myscaler->GetNormData(-1,-1,"TS-accept"));

   printf("Charge Monitors  (Micro Coulombs)\n");
   fprintf(outfile,"Charge Monitors  (Micro Coulombs)\n");


   printf("  Upstream   BCM gain x1 %8.2f     x3 %8.2f     x10 %8.2f\n",
	  curr_u1*time_sec,curr_u3*time_sec,curr_u10*time_sec);
   fprintf(outfile,"  Upstream   BCM gain x1 %8.2f     x3 %8.2f     x10 %8.2f\n",
	  curr_u1*time_sec,curr_u3*time_sec,curr_u10*time_sec);
   printf("  Downstream BCM gain x1 %8.2f     x3 %8.2f     x10 %8.2f\n",
	  curr_d1*time_sec,curr_d3*time_sec,curr_d10*time_sec);
   fprintf(outfile,"  Downstream BCM gain x1 %8.2f     x3 %8.2f     x10 %8.2f\n",
	  curr_d1*time_sec,curr_d3*time_sec,curr_d10*time_sec);


   printf("Target +     Beam + \n");
   fprintf(outfile,"Target +     Beam + \n");

   printf("  Upstream   BCM gain x1 %8.2f     x3 %8.2f     x10 %8.2f\n",
	  curr_u1_pp*time_sec_pp,curr_u3_pp*time_sec_pp,curr_u10_pp*time_sec_pp);
   fprintf(outfile,"  Upstream   BCM gain x1 %8.2f     x3 %8.2f     x10 %8.2f\n",
	  curr_u1_pp*time_sec_pp,curr_u3_pp*time_sec_pp,curr_u10_pp*time_sec_pp);
   printf("  Downstream BCM gain x1 %8.2f     x3 %8.2f     x10 %8.2f\n",
	  curr_d1_pp*time_sec_pp,curr_d3_pp*time_sec_pp,curr_d10_pp*time_sec_pp);
   fprintf(outfile,"  Downstream BCM gain x1 %8.2f     x3 %8.2f     x10 %8.2f\n",
	  curr_d1_pp*time_sec_pp,curr_d3_pp*time_sec_pp,curr_d10_pp*time_sec_pp);


   //cout << (Double_t)myscaler->GetNormData(1,1,"bcm_u1",0)/(Double_t)myscaler->GetNormData(0,"bcm_u1",0) << "\t" << (Double_t)myscaler->GetNormData(1,1,"bcm_d1",0)/(Double_t)myscaler->GetNormData(0,"bcm_d1",0) << "\t" << curr_u1_pp/curr_u1  << "\t" << curr_d1_pp/curr_d1 << endl;

   printf("Target -     Beam + \n");
   fprintf(outfile,"Target -     Beam + \n");

   printf("  Upstream   BCM gain x1 %8.2f     x3 %8.2f     x10 %8.2f\n",
	  curr_u1_np*time_sec_np,curr_u3_np*time_sec_np,curr_u10_np*time_sec_np);
   fprintf(outfile,"  Upstream   BCM gain x1 %8.2f     x3 %8.2f     x10 %8.2f\n",
	  curr_u1_np*time_sec_np,curr_u3_np*time_sec_np,curr_u10_np*time_sec_np);
   printf("  Downstream BCM gain x1 %8.2f     x3 %8.2f     x10 %8.2f\n",
	  curr_d1_np*time_sec_np,curr_d3_np*time_sec_np,curr_d10_np*time_sec_np);
   fprintf(outfile,"  Downstream BCM gain x1 %8.2f     x3 %8.2f     x10 %8.2f\n",
	  curr_d1_np*time_sec_np,curr_d3_np*time_sec_np,curr_d10_np*time_sec_np);


   printf("Target +     Beam - \n");
   fprintf(outfile,"Target +     Beam - \n");

   printf("  Upstream   BCM gain x1 %8.2f     x3 %8.2f     x10 %8.2f\n",
	  curr_u1_pn*time_sec_pn,curr_u3_pn*time_sec_pn,curr_u10_pn*time_sec_pn);
   fprintf(outfile,"  Upstream   BCM gain x1 %8.2f     x3 %8.2f     x10 %8.2f\n",
	  curr_u1_pn*time_sec_pn,curr_u3_pn*time_sec_pn,curr_u10_pn*time_sec_pn);

   printf("  Downstream BCM gain x1 %8.2f     x3 %8.2f     x10 %8.2f\n",
	  curr_d1_pn*time_sec_pn,curr_d3_pn*time_sec_pn,curr_d10_pn*time_sec_pn);

   fprintf(outfile,"  Downstream BCM gain x1 %8.2f     x3 %8.2f     x10 %8.2f\n",
	  curr_d1_pn*time_sec_pn,curr_d3_pn*time_sec_pn,curr_d10_pn*time_sec_pn);


   printf("Target -     Beam - \n");
   fprintf(outfile,"Target -     Beam - \n");

   printf("  Upstream   BCM gain x1 %8.2f     x3 %8.2f     x10 %8.2f\n",
	  curr_u1_nn*time_sec_nn,curr_u3_nn*time_sec_nn,curr_u10_nn*time_sec_nn);
   fprintf(outfile,"  Upstream   BCM gain x1 %8.2f     x3 %8.2f     x10 %8.2f\n",
	  curr_u1_nn*time_sec_nn,curr_u3_nn*time_sec_nn,curr_u10_nn*time_sec_nn);
   printf("  Downstream BCM gain x1 %8.2f     x3 %8.2f     x10 %8.2f\n",
	  curr_d1_nn*time_sec_nn,curr_d3_nn*time_sec_nn,curr_d10_nn*time_sec_nn);
   fprintf(outfile,"  Downstream BCM gain x1 %8.2f     x3 %8.2f     x10 %8.2f\n",
	  curr_d1_nn*time_sec_nn,curr_d3_nn*time_sec_nn,curr_d10_nn*time_sec_nn);
   

    cout<<endl;
  }


  printf("\n ------------ BBNormAna Trigger Summary   ---------------- \n");
  fprintf(outfile,"\n ------------ BBNormAna Trigger Summary   ---------------- \n");


  cout << "Trigger      Prescale      Num evt        Num in        Livetime"<<endl;
  cout << "---           Factor       CODA file      Scalers"<<endl;

  fprintf(outfile, "#Trigger      Prescale      Num evt        Num in        Livetime\n");
  fprintf(outfile, "#---           Factor       CODA file      Scalers\n");

  Int_t nhel=1;
  if (fHelEnable) nhel=5;
  for (int ihel = 0; ihel < nhel; ihel++) {
    Int_t jhel = normdata->InvHel(ihel);  // jhel = 0, -1, +1
    if (jhel==0) {
      cout << "\nIrrespective of helicity "<<endl;
    } else {
      if (jhel == 1){
	cout << "Target Spin:  -    \t  Beam Helicity  +  " << endl;
      }else if (jhel==2){
	cout << "Target Spin:  -    \t  Beam Helicity  -  " << endl;
      }else if (jhel==-1){
	cout << "Target Spin:  +    \t  Beam Helicity  +  " << endl;
      }else if (jhel==-2){
	cout << "Target Spin:  +    \t  Beam Helicity  -  " << endl;
      }
    }
    
    Double_t denom=0.;
    Double_t ave_w=0.;
    Double_t w=0.;
    for (int itrig = 0; itrig < maxtrig; itrig++) {
      // Again, be careful about indices.  E.g. GetTrig starts at itrig=1.
      TString qtemp;
      Int_t qx;
      //      qtemp.Form("trigger-%d",itrig+1);
     qtemp.Form("T%d",itrig+1);

      if (ihel==0){
	qx = myscaler->GetNormData(0,qtemp,0);
      }else if(ihel==1){
	qx = myscaler->GetNormData(1,1,qtemp,0);
      }else if(ihel==2){
	qx = myscaler->GetNormData(-1,1,qtemp,0);
      }else if(ihel==3){
	qx = myscaler->GetNormData(1,-1,qtemp,0);
      }else if(ihel==4){
	qx = myscaler->GetNormData(-1,-1,qtemp,0);
      }
      

      printf("  %2d     %12.8g  %12.8g  %12d  %12.3f\n",	     
	     itrig+1,
	     normdata->GetPrescale(itrig),
	     normdata->GetTrigCount(itrig,jhel),
	     qx,
	     //myscaler->GetTrig(jhel,itrig+1),
	     normdata->GetLive(itrig,jhel)
	     );
      fprintf(outfile,"  %2d     %12.8g  %12.8g  %12d  %12.3f\n",	     
	      itrig+1,
	      normdata->GetPrescale(itrig),
	      normdata->GetTrigCount(itrig,jhel),
	      qx,
	      //myscaler->GetTrig(jhel,itrig+1),
	      normdata->GetLive(itrig,jhel)
          );


      if(normdata->GetTrigCount(itrig,jhel)>0) {
        w=sqrt(normdata->GetTrigCount(itrig,jhel));
        denom+=w;
        ave_w+=w*normdata->GetLive(itrig,jhel);
      }
    }
    ave_w=ave_w/denom;

    Double_t stddev_w=0.;
    Double_t diff=0.;
    for (int itrig = 0; itrig < 12; itrig++) {
      if(normdata->GetTrigCount(itrig,jhel)>0) {
        w=sqrt(normdata->GetTrigCount(itrig,jhel));
        diff=ave_w-normdata->GetLive(itrig,jhel);
        stddev_w+=w* pow(diff, 2 );
        /* 
         * Warn if the livetime difference for a trigger exceeds 5% and the std
         * dev on the trigger is less than 10%.  Handwavy goodness here.
         */
        if( fabs(diff) > 0.05 && w > 10 ) {
          printf("%c[5;1;41m;%s%d%s%c[0m\n",
              0x1B,
              "Warning: T",itrig+1," livetime is significantly different than the average.",
              0x1B
              );
	  fprintf(outfile,
		  "#Warning: T%d livetime is significantly different than the average.",itrig+1);
        }
      }
    }
    stddev_w=stddev_w/denom;
    stddev_w=sqrt(stddev_w);
    printf("\nWeighted averaged livetime  = %6.3f +- %6.3f\n",ave_w,stddev_w);
    fprintf(outfile,"\nWeighted averaged livetime  = %6.3f +- %6.3f\n",ave_w,stddev_w);
    cout << endl<<"Number of events = "<<normdata->GetEvCount(jhel)<<endl;
    fprintf(outfile,"Number of events = %i\n",(int)normdata->GetEvCount(jhel));
/* This run-averaged calculation makes assumptions about correlations that are
 * impossible for e04007 (pi0).  (BDS)
 *  cout << "Run-averaged livetime from scalers = ";
 *  cout << normdata->GetAvgLive(jhel)<<endl;
 */
    cout << "Correction for non-synch of scalers = ";
    cout << normdata->GetCorrfact(jhel)<<endl;
    fprintf(outfile,"\n#Correction for non-synch of scalers = %6.3f\n",
	    normdata->GetCorrfact(jhel));
  }
  fclose(outfile);

  return 1;
}

//_____________________________________________________________________________
  void BBNormAna::WriteHist()
{
  for (UInt_t i = 0; i < hist.size(); i++) hist[i]->Write();
}

//_____________________________________________________________________________
  void BBNormAna::BookHist()
{
// TDCs used for trigger latch pattern.  Used to tune cuts
// that define "simultaneity" of triggers.  Optionally fill
// histograms for BCM calibration (scalers versus EPICS).
  
  Int_t nbin = 100;
  Float_t xlo = 0;
  Float_t xhi = 4000;
  char cname[50],ctitle[50];
  for (int i = 0; i < 12; i++) {
    sprintf(cname,"Tlatch%d",i+1);
    sprintf(ctitle,"Trigger Latch TDC %d",i+1);
    hist.push_back(new TH1F(cname,ctitle,nbin,xlo,xhi));
  }
  if (fDoBcmCalib) {
// Fit these histograms to get calibration factors.
    hist.push_back(new TH2F("bcmu1","bcm u1 vs hac_bcm_average",
			    200,0,20,200,0,100000));
    hist.push_back(new TH2F("bcmu3","bcm u3 vs hac_bcm_average",
			    200,0,20,200,0,100000));
    hist.push_back(new TH2F("bcmu10","bcm u10 vs hac_bcm_average",
			    200,0,20,200,0,100000));
    hist.push_back(new TH2F("bcmd1","bcm d1 vs hac_bcm_average",
			    200,0,20,200,0,100000));
    hist.push_back(new TH2F("bcmd3","bcm d3 vs hac_bcm_average",
			    200,0,20,200,0,100000));
    hist.push_back(new TH2F("bcmd10","bcm d10 vs hac_bcm_average",
			    200,0,20,200,0,100000));
   }
}

/*Int_t CmpNoCase( const string& r, const string& s )
{
  // Case insensitive compare.  Returns -1 if "less", 0 if equal, 1 if
  // "greater".

  string::const_iterator p =  r.begin();
  string::const_iterator p2 = s.begin();

  while (p != r.end() && p2 != s.end()) {
    if (toupper(*p) != toupper(*p2))
      return (toupper(*p) < toupper(*p2)) ? -1 : 1;
    ++p;
    ++p2;
  }

  return (r.size() == s.size()) ? 0 : (r.size() < s.size()) ? -1 : 1;
}*/





//_____________________________________________________________________________
THaAnalysisObject::EStatus BBNormAna::Init( const TDatime& run_time ) 
{

  fStatus = kNotinit;
  MakePrefix();

  cout << "BBNormAna:: Init !"<<endl<<flush;

  // Grab the scalers.
  // Of course these scalers are the event type 140 scaler data 
  THaAnalyzer* theAnalyzer = THaAnalyzer::GetInstance();
  TList* appList = theAnalyzer->GetApps();
  TIter next1(appList);
  while (THaApparatus *apps = static_cast<THaApparatus*>(next1())){
    if (CmpNoCase("B",apps->GetName()) == 0){
      myhelicity  = (THaADCHelicity*)(apps->GetDetector("adchel.L"));
      break;
    }
  }

  TList* scalerList = theAnalyzer->GetScalers();
  TIter next(scalerList);
  while( THaScalerGroup* tscalgrp = static_cast<THaScalerGroup*>( next() )) {
    THaScaler *scaler = tscalgrp->GetScalerObj();
    //THaString mybank("Left");
    // if (mybank.CmpNoCase(scaler->GetName()) == 0) {
    // if (CmpNoCase("Left",scaler->GetName()) == 0) {
//          myscaler = scaler;   // event type 140 data
//          break;
// 	 }

    if (CmpNoCase("bbite",scaler->GetName()) == 0) {
         myscaler = scaler;   // event type 140 data
         break;
    }

  }

 
 
// #ifdef TRY1
//   // Dont want this.  It illustrates how to use a
//   // private THaScaler to process the data, but at 
//   // present neither THaApparatus::Decode nor
//   // a THaPhysicsModule::Process receive event type 140 
//   // However, it's probably not too bad an idea to use
//   // the THaScaler from THaAnalyzer (see above) anyway.
//   myscaler = new THaScaler("Left");
//   if (myscaler) {
//     if( myscaler->Init(run_time) == -1) {
//         cout << "Error init scalers "<<endl;
//         myscaler = 0;
//     }
//   }
// #endif

  BookHist();

// Calibration of BCMs.  Probably needs adjustment.
  // calib_u1 = 1345;  calib_u3 = 4114;  calib_u10 = 12515; 
//   calib_d1 = 1303;  calib_d3 = 4034;  calib_d10 = 12728; 
//   off_u1 = 92.07;   off_u3 = 167.06;  off_u10 = 102.62;  
//   off_d1 = 72.19;   off_d3 = 81.08;   off_d10 = 199.51;

// // Calibration of BCMs.  run 1400 E04-018 dec 2006
// // right u10 odd values need to check scaler map so old constant here
//   calib_u1 = 2363.11;  calib_u3 = 7266.15;  calib_u10 = 12515; 
//   calib_d1 = 2392.87;  calib_d3 = 7403.55;  calib_d10 = 22430.2; 
//   off_u1 = 345.61;   off_u3 = 386.17;  off_u10 = 102.62;  
//   off_d1 = 179.57;   off_d3 = 214.80;   off_d10 = 3540.19;


// 2parameter fit (Nov 2008)
//   calib_u1 = 2.077557e+03;  calib_u3 = 6.390797e+03;   
//   calib_d1 = 2.162851e+03;  calib_d3 = 6.696152e+03;  calib_d10 = 2.112068e+04; 
//   off_u1 = 3.698497e+02;   off_u3 = 4.069899e+02;    
//   off_d1 = 9.957301e+01;   off_d3 = -1.492612e+00;   off_d10 = -7.735930e+01;
  


//   //1 parameter fit with offset fixed:
//   calib_u1 = 2070.73;  off_u1 = 451.612;  
//   calib_u3 = 6369.94;   off_u3 = 656.857;
//   calib_d1 = 2159.27;   off_d1 = 142.538;
//   calib_d3 = 6685.99;   off_d3 = 120.257;
//   calib_d10 = 21091;  off_d10 = 278.758;
         
        
//  //  //1 parameter fit with offset fixed: (Nov 2008)
//   calib_u1 = 2.077557e+03;   off_u1 =  452.383;  
//   calib_u3 = 6.390797e+03;   off_u3 = 661.282;
//   calib_d1 = 2.162851e+03;   off_d1 = 145.418;
//   calib_d3 = 6.696152e+03;   off_d3 = 120.299;
//   calib_d10 = 2.112068e+04;  off_d10 = 289.062;

  //1 parameter fit with offsets fixed.(Feb 2009)
/*    calib_u1 =  2101.87; off_u1 =  395.804;
    calib_u3 =  6464.39; off_u3 =  453.4;
    calib_u10 = 19718.3; off_u10 = 770.516; 
    calib_d1 =  2147.88; off_d1 =  154.577;
    calib_d3 = 6645.89; off_d3 =  133.319;
    calib_d10 = 20952; off_d10 = 293.463;*/

	// 1-Parameter Fit for d2n (offsets fixed; updated Mar 2010; calibration run from Feb 2009)
	calib_u1 = 2102.26;
	off_u1 = 395.8;
	calib_u3 = 6480.30;
	off_u3 = 453.393;
	calib_u10 = 19731.3;
	off_u10 = 770.66;
	calib_d1 = 2152.44;
	off_d1 = 154.58;
	calib_d3 = 6658.34;
	off_d3 = 133.27;
	calib_d10 = 21007.6;
	off_d10 = 293.48;	
 
  return fStatus = static_cast<EStatus>( SetupRawData( &run_time ) );


}


//_____________________________________________________________________________
Int_t BBNormAna::Process(const THaEvData& evdata)
{
// Process the data:  Get prescale factors, get trigger
// latch pattern.  Process scalers.  All sorted by helicity
// if helicity was enabled.

  Int_t i, j;

  //  cout << Float_t(myscaler->GetNormData(1,1,"bcm_u10"))/Float_t(myscaler->GetNormData(0,"bcm_u10")) << "\t" << Float_t(myscaler->GetNormData(1,-1,"bcm_u10"))/Float_t(myscaler->GetNormData(0,"bcm_u10")) << "\t" << Float_t(myscaler->GetNormData(1,1,"bcm_d10"))/Float_t(myscaler->GetNormData(0,"bcm_d10")) << "\t" << Float_t(myscaler->GetNormData(1,-1,"bcm_d10"))/Float_t(myscaler->GetNormData(0,"bcm_d10")) << endl;
  //cout <<  << "\t" <<  << endl;


  //  cout << "BBNormAna:  process the data ..."<<endl<<flush;

// bcmu3 is an uncalibrated rate in Hz
  bcmu3 = -1;
  if (myscaler) bcmu3 = myscaler->GetNormRate(0,"bcm_u3");

  if (fDoBcmCalib) BcmCalib(evdata);

// helicity is -1,0,1.  here, 0 == irrespective of helicity.
// But if helicity disabled its always 0.

//FIXME: temporary workaround for helicity work-in-progress
  fHelEnable = evdata.HelicityEnabled();
  
  Int_t beamhel = myhelicity->GetHelicity();//evdata.GetHelicity();

  // fHelEnable = kFALSE;  
//   Int_t beamhel = 0;
  
  
  Int_t helicity1=0,helicity2=0;

  Int_t targethel=0;
  Double_t data1 = static_cast<Double_t>(evdata.GetData( 8, 3, 14, 0 ));
  Double_t data2 = static_cast<Double_t>(evdata.GetData( 8, 3, 15, 0 ));
  
  if (data1>10000&&data1<20000&&data2>0&&data2<500){
    targethel = 1;
  }else if (data2>10000&&data2<20000&&data1>0&&data1<500){
    targethel = -1;
  }
  
  //  cout << data1 << "\t" << data2 << "\t" << fHelEnable << "\t" << beamhel << "\t" << targethel << endl;
  //hack for now
  // targethel = 1;

  
  
  
  if (beamhel==1&&targethel==1){
    helicity1 = 1;
    helicity2 = -1;
  }else if (beamhel==1&&targethel==-1){
    helicity1 = 2;
    helicity2 = 1;
  }else if (beamhel==-1&&targethel==1){
    helicity1 = 3;
    helicity2 = -2;
  }else if (beamhel==-1&&targethel==-1){
    helicity1 = 4;
    helicity2 = 2;
  }


  if (evdata.IsPhysicsTrigger()) {
    normdata->EvCount(helicity2);
  }
// If you have ~1000 events you probably have seen 
// prescale event; ok, this might not work for filesplitting
// if prescale event only goes into first file (sigh).
  
  if (!fSetPrescale && evdata.GetEvNum() > 1000) {
    fSetPrescale = kTRUE;
// CAREFUL, must GetPrescaleFactor(J) where J = 1,2,3 .. 
// (index starts at 1).  normdata starts at 0.
//    cout << "Prescale factors.  Origin = "<<evdata.GetOrigPS()<<endl;
    for (i = 0; i < 8; i++) {
         cout << "  trig= "<<i+1<<"   ps= "<<evdata.GetPrescaleFactor(i+1)<<endl;
         normdata->SetPrescale(i,evdata.GetPrescaleFactor(i+1));
    }
    for (i = 8; i < 12; i++) normdata->SetPrescale(i,1);
  }

#ifdef TRY1
  // Example to process the scalers if event type 140
  // came here, which it does not.  See also the TRY1
  // comments elsewhere.
  //  if (evdata.GetEvType() == 140) {
  //     if (myscaler) myscaler->LoadData(evdata);
  //  }
#endif


// TDC for trigger latch pattern.  The crate,slot,startchan 
// might change with experiments (I hope not).

// (BDS) pulled from $DB/20080402/db_DL.dat
  int crate = 8;
  int slot = 17;
  int startchan = 16;

  int ldebug = 0;

  for (i = 0; i < 12; i++) {
    for (j = 0; j < 6; j++) tdcdata[j*12+i] = 0;
    nhit[i] = evdata.GetNumHits(crate, slot, startchan+i);
    if (ldebug) cout << "Latch for trig "<<i+1<<"   Nhit "<<nhit[i]<<endl;
    if (nhit[i] > 6) nhit[i] = 6;  
    for (j = 0; j < nhit[i]; j++) {
      Int_t tdat = evdata.GetData(crate,slot,startchan+i,j);
      if (ldebug) cout << "Raw TDC "<<tdat<<endl;
      tdcdata[j*12+i] = tdat;
      hist[i]->Fill(tdat);
    }        
  }
     
  TrigBits(helicity2);

  LiveTime();

  //  GetRocScalers(evdata);

  //  if (fDEBUG) Print();

  return 0;
}


//_____________________________________________________________________________
void BBNormAna::Print( Option_t* opt ) const {
// Print for purpose of debugging.

   normdata->Print();
   PrintSummary();

}


//_____________________________________________________________________________
void BBNormAna::SetEventInt(Int_t *event)
{
// See also comments at top of code.
// Set the event intervals that define stable beam.
// It is necessary to first look at N.bcmu3:fEvtHdr.fEvtNum
// to decide which event intervals have stable beam.
// Then let  N = event[0] = number of intevals (pairs).
// event[1] to event[2] is a stable beam interval  
// up to: event[2N-1] to event[2N]

  if (event == 0) return;
  Int_t N = event[0];
  if (N == 0) return;
  if (2*N >= fgMaxEvInt) N = fgMaxEvInt/2;
  for (int i = 0; i <= 2*N; i++) eventint[i] = event[i];
  return;
}

//_____________________________________________________________________________
Int_t BBNormAna::BcmCalib(const THaEvData& evdata) {
// Fill histograms of scaler BCM versus EPICS BCM.
// This calibrates the scalers and assumes the EPICS
// variables are well calibrated.

  if (!fDoBcmCalib) return 0;
  if (myscaler == 0) return 0;
  if (hist.size() < 18) return 0;
// See SetEventInt() for definition of eventint.
  Int_t N = eventint[0];
  if (N <= 0) return 0;
  Double_t u1  = myscaler->GetNormRate(0,"bcm_u1");
  Double_t u3  = myscaler->GetNormRate(0,"bcm_u3");
  Double_t u10 = myscaler->GetNormRate(0,"bcm_u10");
  Double_t d1  = myscaler->GetNormRate(0,"bcm_d1");
  Double_t d3  = myscaler->GetNormRate(0,"bcm_d3");
  Double_t d10 = myscaler->GetNormRate(0,"bcm_d10");
  Double_t hacbcm = evdata.GetEpicsData("IBC0L02Current");

  

  // cout << u1 << "\t" << hacbcm << endl;

  for (int i = 1; i <= 2*N; i++) {
    if (evdata.GetEvNum() > eventint[i] && 
        evdata.GetEvNum() < eventint[i+1]) {
      hist[12]->Fill(hacbcm,u1);
      hist[13]->Fill(hacbcm,u3);
      hist[14]->Fill(hacbcm,u10);
      hist[15]->Fill(hacbcm,d1);
      hist[16]->Fill(hacbcm,d3);
      hist[17]->Fill(hacbcm,d10);
      return 0;
    }
  }
  return 0;
}

//_____________________________________________________________________________

void BBNormAna::GetRocScalers(const THaEvData& evdata) {
// Decode the ROC scalers.  See also InitRocScalers()

  static Int_t ldebug = 0;   // put this =1 to get debug output.
  static Int_t myroc[] = { 10, 11 };   
  static Double_t XCORR_FACT = 1.00;  // not sure, I think its 1.015

  Int_t len, data, helicity, gate, qrt, timestamp;
  Int_t nscaler, header, found, index;

  for (int i = 0; i < fgNumRoc*fgNumChan; i++) {
     norm_scaler[i] = 0;
     norm_plus_plus[i] = 0;
     norm_minus_plus[i] = 0;
     norm_plus_minus[i] = 0;
     norm_minus_minus[i] = 0;
  }

  

  for (Int_t iroc = 0; iroc < 2; iroc++) {
     len = evdata.GetRocLength(myroc[iroc]);
     if (len <= 4) continue;
     data = evdata.GetRawData(myroc[iroc],3);   
     if (ldebug) cout<<"ROC "<<myroc[iroc]<<" data = "<<hex<<data<<dec<<endl;

     helicity = (data & 0x10) >> 4;
     qrt = (data & 0x20) >> 5;
     gate = (data & 0x40) >> 6;
     timestamp = evdata.GetRawData(myroc[iroc],4);

     if (myroc[iroc] == 11) roc11_clk104k = timestamp;
     if (ldebug) {
       cout << "helicity " << helicity << "  qrt " << qrt;
       cout << "  gate " << gate << "   time stamp " << timestamp << endl;
     }
     found = 0;
     index = 5;
     while ( !found ) {
       data = evdata.GetRawData(myroc[iroc],index++);
       if ( (data & 0xffff0000) == 0xfb0b0000) found = 1;
       if (index >= len) break;
     }
     if (!found) continue;
     nscaler = data & 0x7;
     if (ldebug) cout << "nscaler in this event  " << nscaler << endl;
     if (nscaler <= 0) continue;
     if (nscaler != 5) {
       cout << "Warning, nscaler !=5 "<<endl;
       nscaler = 5;  // unnecessary (unless I change data structure)
     }
// 32 channels of scaler data for two helicities.
     Int_t roc_offset = iroc;
     if (roc_offset > fgNumRoc) roc_offset = fgNumRoc;
   
     for (int ihel = 0; ihel < nscaler; ihel++) { 
       header = evdata.GetRawData(myroc[iroc],index++);
       if (ldebug) {
         cout << "Scaler[" << ihel << "] = ";
         cout << "  unique header = " << hex << header << endl;
        // This header should be one of the 2 in ScalerMap, but not checked.
        }
       for (int ichan = 0; ichan < 32; ichan++) {
	   data = evdata.GetRawData(myroc[iroc],index++);
           if (ihel == 0) norm_scaler[roc_offset*fgNumChan + ichan] += data;

           if (ihel == 1) norm_plus_plus[roc_offset*fgNumChan + ichan] = data;
           if (ihel == 2) norm_minus_plus[roc_offset*fgNumChan + ichan] = data;
	   if (ihel == 3) norm_plus_minus[roc_offset*fgNumChan + ichan] = data;
           if (ihel == 4) norm_minus_minus[roc_offset*fgNumChan + ichan] = data;

           if (ldebug) {       
              cout << "channel # " << dec << ichan+1;
              cout << "  (hex) data = " << hex << data;
              cout << "    (dec) data = " << dec << data << endl;
	   }
       }
     }
     // Correct for the ~500 usec blanking of pockel cell (gate off).
     for (int ichan = 0; ichan < 32; ichan++) 
       norm_scaler[ichan] = XCORR_FACT * norm_scaler[ichan];
     // Assign data to mapped channels
     for (UInt_t idat = 0; idat < fRocScaler.size(); idat++) {
       if (fRocScaler[idat]->roc == myroc[iroc]) {
  	 Int_t index = roc_offset*fgNumChan + fRocScaler[idat]->chan;
         if (index >= 0 && index < fgNumRoc*fgNumChan) {
           *(fRocScaler[idat]->data) = norm_scaler[index];
           if (ldebug) {
             cout << "\nScaler Chan Name = "<<fRocScaler[idat]->chan_name<<endl;
             cout << "chan# = "<<fRocScaler[idat]->chan;
             Int_t sdata = (Int_t) (*(fRocScaler[idat]->data));
             cout << "  data = 0x" << hex << sdata;
	     cout << "   = (dec) "<< dec << sdata << endl;
	   }
	 }	    
       }
     }
  }

  CalcAsy();

  if (ldebug) {
    cout << "Check of Roc Scaler Data : "<<endl;
    cout << "roc11 trig "<<roc11_t1<<"  "<<roc11_t2<<"  "<<roc11_t3;
    cout << "   "<<roc11_t4<<"   "<<roc11_t5<<endl;
    cout << "bcms "<< roc11_bcmu3<<"  "<<roc11_bcmu10<<endl;
    cout << "clocks "<<roc11_clk1024<<"  "<<roc11_clk104k<<endl;
  }

}


void BBNormAna::CalcAsy( ) {
  // do something with norm_plus and norm_minus ...

}


void BBNormAna::TrigBits( Int_t helicity ) {
// Figure out which triggers got a hit.  These are 
// multihit TDCs, so we need to sort out which hit 
// we want to take by applying cuts.

  int itrig,jhit,localtrig[12];

// These cuts can be decided by looking at the histograms.
// They importantly define "simultaneous" triggers, and 
// may therefore introduce a (small) systematic error

  static const Int_t cutlo = 100;
  static const Int_t cuthi = 1700;

  evtypebits = 0;
  memset(localtrig,0,12*sizeof(int));
  
  for (itrig = 0; itrig < 12; itrig++) {
    if (fDEBUG) {
           cout << "TDC for trig "<<itrig;
           cout << "   nhit = "<<nhit[itrig]<<endl;
    }
    for (jhit = 0; jhit < nhit[itrig]; jhit++) {
      Int_t tdat = tdcdata[12*jhit + itrig];
// Count at most once per event (don't double count)
// That's the purpose of localtrig
      if (fDEBUG) {
        cout << "    TDC dat for hit ";
        cout << jhit<<"   "<<tdat<<endl;
      }
      

      if (tdat > cutlo && tdat < cuthi && localtrig[itrig]==0) {
        localtrig[itrig] = 1;
	normdata->TrigCount(itrig,helicity);
        evtypebits |= (2<<itrig);
        if (fDEBUG) {
          cout << "        trigger bit "<< itrig;
          cout << "   helicity "<< helicity << "  count = ";
          cout << normdata->GetTrigCount(itrig,helicity)<<endl;
	}
      }
    }
  }

  if (fDEBUG) {
     cout << "  trig bit pattern "<<evtypebits<<endl;
  }

}


//_____________________________________________________________________________

void BBNormAna::LiveTime() {

// Compute the livetime by trigger type, by
// helicity, and averaged from scalers-only-info.

   if (myscaler == 0) return;

  Int_t itrig, ihel, nhel, ok1;
  Double_t t5corr, t7corr, tcorr;
  Double_t totaltrig, livetime, avglive;
  Double_t numtrig, tsaccept, corrfact;  
  Double_t trate, ttrigrate;
  Double_t t5rate_corr;

  ok1=0;
  nhel = 1;  
  if (fHelEnable) nhel = 5;

  for (ihel = 0; ihel < nhel; ihel++) { 
    Int_t helicity = 0,targethel = 0;
    if (ihel==1){
      helicity = 1;
      targethel = 1;
    }
    if (ihel==2){
      helicity = 1;
      targethel = -1;
    }
    if (ihel==3){
      helicity = -1;
      targethel = 1;
    }
    if (ihel==4){
      helicity = -1;
      targethel = -1;
    }
    

// CAREFUL ABOUT INDEX conventions
// THaScaler::GetTrig starts at itrig=1
// normdata indices all start at 0.

    t5corr = 0;
    t5rate_corr = 0;
    if (normdata->GetPrescale(4) != 0) {
      if (targethel!=0){
	t5corr = (Double_t)myscaler->GetNormData(targethel,helicity,"T5",0)/
	  normdata->GetPrescale(4);
	t5rate_corr = (Double_t)myscaler->GetNormRate(targethel,helicity,"T5")/
	  normdata->GetPrescale(4);
      }else{
	t5corr = (Double_t)myscaler->GetNormData(targethel,"T5",0)/
	  normdata->GetPrescale(4);
	t5rate_corr = (Double_t)myscaler->GetNormRate(targethel,"T5")/
	  normdata->GetPrescale(4);
      }
	//          (Double_t)myscaler->GetTrig(helicity,5)/
	//            normdata->GetPrescale(4);
      
	
      
          // (Double_t)myscaler->GetTrigRate(helicity,5)/
//                       normdata->GetPrescale(4);
      
    }
    t7corr = 0;
    if (normdata->GetPrescale(6) != 0) {
      if (targethel!=0){
	t7corr = (Double_t)myscaler->GetNormData(targethel,helicity,"T7",0)/
	  normdata->GetPrescale(6);
      }else{
	t7corr = (Double_t)myscaler->GetNormData(targethel,"T7",0)/
	  normdata->GetPrescale(6);
      }
      // (Double_t)myscaler->GetTrig(helicity,7)/
//                       normdata->GetPrescale(6);
    }

    tsaccept = 0;
    totaltrig = 0;
    corrfact = 0;
    ttrigrate = 0;
    ok1 = 0;

    for (itrig = 0; itrig < 12; itrig++) {

      TString qtemp;
      qtemp.Form("T%d",itrig+1);
     //  numtrig = (Double_t)myscaler->GetTrig(helicity,itrig+1);
//       trate = myscaler->GetTrigRate(helicity,itrig+1);
      if (targethel!=0){
	numtrig = (Double_t)myscaler->GetNormData(targethel,helicity,qtemp,0);
	trate = myscaler->GetNormRate(targethel,helicity,qtemp);
      }else{
	numtrig = (Double_t)myscaler->GetNormData(targethel,qtemp,0);
	trate = myscaler->GetNormRate(targethel,qtemp);
      }

      //      cout << "trate "<<trate<<endl;

// Livetime by trigger type and by helicity:
// These are exact and physically meaningful, in
// contrast to "average livetime" below.
      
      if (normdata->GetPrescale(itrig) > 0 && numtrig > 0) {
        // livetime = normdata->GetTrigCount(itrig,helicity)*
//              normdata->GetPrescale(itrig)/numtrig;
	livetime = normdata->GetTrigCount(itrig,normdata->InvHel(ihel))*
             normdata->GetPrescale(itrig)/numtrig;
	//cout << numtrig << "\t" << normdata->GetTrigCount(itrig,normdata->InvHel(ihel)) << "\t" << normdata->GetPrescale(itrig) << "\t" << livetime << endl;
	if (targethel!=0){
	  tsaccept = 
	    (Double_t)myscaler->GetNormData(targethel, helicity,"TS-accept",0);
	}else{
	  tsaccept = 
	    (Double_t)myscaler->GetNormData(targethel, "TS-accept",0);
	}

	//cout << (Double_t)myscaler->GetNormData(targethel, helicity,"TS-accept",0) << "\t" << (Double_t)myscaler->GetNormData(targethel, "TS-accept",0) << endl;

// Correction for the non-synch of scalers and events.
// If correction too far from 1 --> failure to estimate.
// At a normal end-of-run, corrfact should be 1.0

        if (tsaccept > 0) corrfact = 
	  normdata->GetEvCount(normdata->InvHel(ihel))/tsaccept;
        if (corrfact > 0.5 && corrfact < 2.0) {
          ok1=1;
          livetime = livetime/corrfact;
        } else {
          livetime = -1;  // failure
	}
        normdata->SetLive(itrig, livetime, normdata->InvHel(ihel)); 

	//	cout << tsaccept << "\t" << corrfact << "\t" << livetime << endl;

// Average livetime using only scaler information
// Assumptions, valid for e94107, are:
// Every T5 is a T1 and T3.  Every T7 is a T5 and T6.
// There are other correlations which are ignored,
// hence this is only approximate.

        tcorr = 0; 
        if (itrig == 0 || itrig == 2) tcorr = t5corr ;
        if (itrig == 4 || itrig == 5) tcorr = t7corr;
        totaltrig = totaltrig + 
           (numtrig-tcorr)/normdata->GetPrescale(itrig);
      }

      avglive = 0;
      if (totaltrig > 0) avglive = tsaccept/totaltrig;
      normdata->SetAvgLive(avglive,corrfact,normdata->InvHel(ihel));

// Differential DT for helicity==0.  Ignores the "corrfact"
      if (helicity==0 && 
        normdata->GetPrescale(itrig) > 0 && trate > 0) {
        tcorr = 0; 
        if (itrig == 0 || itrig == 2) tcorr = t5rate_corr;
        ttrigrate = ttrigrate +
 	   (trate-tcorr)/normdata->GetPrescale(itrig);
	//        cout << "ttrigrate "<<itrig<<"  "<<trate<<"  "<<tcorr<<"   "<<ttrigrate<<endl;
      }


    }

    if (helicity == 0) {   
     dlive = -1;
     if (ttrigrate > 0) dlive = myscaler->GetNormRate(0,"TS-accept")/ttrigrate;
     //     cout << "TS-accept rate  "<<myscaler->GetNormRate(0,"TS-accept")<<"   dlive "<<dlive<<endl;
    }

  }

  alive = normdata->GetAvgLive();
  hpos_pos_alive = normdata->GetAvgLive(-1);
  hneg_pos_alive = normdata->GetAvgLive(1);
  hpos_neg_alive = normdata->GetAvgLive(-2);
  hneg_neg_alive = normdata->GetAvgLive(2);

}


//_____________________________________________________________________________
ClassImp(BBNormAna)

