#ifndef ROOT_THaVDCFuncTTDConv
#define ROOT_THaVDCFuncTTDConv

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// THaVDCFuncTTDConv                                                        //
//                                                                           //
// Uses a table to convert time (ns) into distance (cm)                      //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "THaVDCTimeToDistConv.h"

class THaVDCFuncTTDConv : public THaVDCTimeToDistConv{

public:
  THaVDCFuncTTDConv();
  THaVDCFuncTTDConv(Double_t driftvelocity, Double_t c1, Double_t c2, Double_t c3);
  virtual ~THaVDCFuncTTDConv();

  virtual Double_t ConvertTimeToDist(Double_t time, Double_t tanTheta = 0.0, Double_t *drift_dist = 0);
  Double_t ConvertDistToTime(Double_t time, Double_t tanTheta);
  
  Double_t GetDriftVelocity() {return fDriftVelocity;}

protected:
  
  Double_t fDriftVelocity;                  // Average Drift Velocity
  Double_t fC1;
  Double_t fC2;
  Double_t fC3;
  ClassDef(THaVDCFuncTTDConv, 0)             // VDC Func TTD Conv class
};


////////////////////////////////////////////////////////////////////////////////

#endif
