#ifndef ROOT_THaVDC2DTableTTDConv
#define ROOT_THaVDC2DTableTTDConv

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// THaVDC2DTableTTDConv                                                        //
//                                                                           //
// Uses a 2D table to convert time (ns) into distance (cm)                      //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "THaVDCTimeToDistConv.h"

#define MAX_X_GRID 500
#define MAX_Y_GRID 500

class THaVDC2DTableTTDConv : public THaVDCTimeToDistConv{

public:
  THaVDC2DTableTTDConv();
  THaVDC2DTableTTDConv(const char *filename);
  virtual ~THaVDC2DTableTTDConv();

  virtual Double_t ConvertTimeToDist(Double_t time, Double_t tanTheta = 0.0, Double_t *drift_dist = 0);
  Double_t ConvertDistToTime( Double_t plane_intercept, Double_t slope );
  void LoadTable( const char *filename );
  void PrintTable();
  Double_t GetDriftVelocity() {return fDriftVelocity;}

protected:
  Int_t x0;
  Int_t y0;

  Int_t Nx;
  Int_t Ny;

  Double_t fDriftVelocity;                  // Average Drift Velocity
  
  Double_t TableX[MAX_X_GRID];
  Double_t TableY[MAX_Y_GRID];
  Double_t TableT[MAX_X_GRID][MAX_Y_GRID];
  ClassDef(THaVDC2DTableTTDConv, 0)             // VDC 2D Table TTD Conv class
};


////////////////////////////////////////////////////////////////////////////////

#endif
