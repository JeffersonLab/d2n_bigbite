#ifndef ROOT_THaVDCLinearTTDConv
#define ROOT_THaVDCLinearTTDConv

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// THaVDCLinearTTDConv                                                        //
//                                                                           //
// Uses a table to convert time (ns) into distance (cm)                      //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "THaVDCTimeToDistConv.h"

class THaVDCLinearTTDConv : public THaVDCTimeToDistConv{

public:
  THaVDCLinearTTDConv();
  THaVDCLinearTTDConv(Double_t driftvelocity);
  virtual ~THaVDCLinearTTDConv();

  virtual Double_t ConvertTimeToDist(Double_t time, Double_t tanTheta = 0.0, Double_t *drift_dist = 0);
  Double_t ConvertDistToTime(Double_t time, Double_t tanTheta);
  
  Double_t GetDriftVelocity() {return fDriftVelocity;}

protected:
  
  Double_t fDriftVelocity;                  // Average Drift Velocity
  ClassDef(THaVDCLinearTTDConv, 0)             // VDC Linear TTD Conv class
};


////////////////////////////////////////////////////////////////////////////////

#endif
