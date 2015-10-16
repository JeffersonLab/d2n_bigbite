#ifndef ROOT_THaVDCTableTTDConv
#define ROOT_THaVDCTableTTDConv

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// THaVDCTableTTDConv                                                        //
//                                                                           //
// Uses a table to convert time (ns) into distance (cm)                      //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "THaVDCTimeToDistConv.h"
#include <vector>

#define MAX_X_GRID 500
#define MAX_Y_GRID 500

class THaVDCTableTTDConv : public THaVDCTimeToDistConv{

public:
  THaVDCTableTTDConv();
  THaVDCTableTTDConv(const char *filename);
  virtual ~THaVDCTableTTDConv();

  virtual Double_t ConvertTimeToDist(Double_t time, Double_t tanTheta = 0.0, Double_t *drift_dist = 0);
  void LoadTable( const char *filename );
  
  Double_t GetDriftVelocity() {return fDriftVelocity;}

protected:
  std::vector<Double_t> fTimes;
  std::vector<Double_t> fDistances;
  Double_t fDriftVelocity;                  // Average Drift Velocity
  
  Double_t TableX[MAX_X_GRID];
  Double_t TableY[MAX_Y_GRID];
  Double_t TableT[MAX_X_GRID][MAX_Y_GRID];
  ClassDef(THaVDCTableTTDConv, 0)             // VDC Table TTD Conv class
};


////////////////////////////////////////////////////////////////////////////////

#endif
