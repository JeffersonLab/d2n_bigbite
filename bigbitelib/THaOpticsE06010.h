//////////////////////////////////////////////////////////////////////////
//
//    THaOpticsE06010
//
//////////////////////////////////////////////////////////////////////////
//    
// Author: Xin Qian <mailto:xqian@jlab.org>    Mar 2009
//    Modify History: 
//
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_THaOpticsE06010
#define ROOT_THaOpticsE06010

#include "THaOptics.h"
#include "TMath.h"
#include "TComplex.h"
#include "TClonesArray.h"
#include <vector>


class THaTrack;
class THaSpectrometer;
class TVector3;
class TString;
class THaBeam;

using std::vector;

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
//    >=4    above + step by step debug
#define DEBUG_LEVEL   2

#include    "DebugDef.h"
//------------------------------------------------------//

 
  
  class THaOpticsE06010 : public THaOptics 
  {
  public:
    THaOpticsE06010(const char* name, const char* desc, THaSpectrometer *pspec, TString strBeamDetectorName="");
    virtual ~THaOpticsE06010();
    
    virtual Int_t ApplyOptics(THaTrack *track);
    virtual Int_t ApplyOptics(THaTrack *track, TVector3 beampos, TVector3 beamdir);
    
    //apply optics : do the job
    virtual Int_t ApplyOptics(
			      Double_t trackX,    //input:    Track X             TRCS
			      Double_t trackY,    //input:    Track Y             TRCS
			      Double_t trackTheta,//input:    Track Theta         TRCS
			      Double_t trackPhi,  //input:    Track Phi           TRCS
			      TVector3 beamPos,   //input:    Beam Position       HCS
			      TVector3 beamDir,   //input:    Beam Direction      HCS
			      TVector3 &P,        //output:   vector momentum     HCS
			      TVector3 &Vertex,   //output:   vertex cordinate    HCS
			    Double_t &TargetX,     //output:   Target X            TCS
			      Double_t &TargetY,     //output:   Target Y            TCS
			      Double_t &TargetTheta, //output: Target Theta        TCS
			      Double_t &TargetPhi,   //output:   Target Phi          TCS
			      Double_t &PathLen      //output:   Length of path      any coordinate
			      );
    
    virtual void    Clear( Option_t* opt="" );
    Int_t           GetN() const { return ftroptics->GetLast()+1; }
    
  protected:
    virtual Int_t   ReadDatabase( const TDatime& date );  
    
    virtual Int_t   DefineVariables( EMode mode = kDefine );
    
    Double_t sieve(Double_t x   , Double_t y   , Double_t xp  , Double_t yp  , Double_t vertex_x  ,Double_t vertex_y  , Double_t vertex_z,Double_t dis  );
    Double_t sieve1(Double_t x, Double_t y   , Double_t xp  , Double_t yp  , Double_t vertex_x  ,Double_t vertex_y  , Double_t vertex_z,Double_t dis  );
    Double_t sndcorr(Double_t x1, Double_t y1 , Double_t z1, Double_t *x, Double_t *y, Double_t *z, Double_t *corr);
    Double_t thrcorr(Double_t x1, Double_t y1 , Double_t z1, Double_t *x, Double_t *y, Double_t *z, Double_t *corr);
    Double_t foucorr(Double_t x1, Double_t y1 , Double_t z1, Double_t *x, Double_t *y, Double_t *z, Double_t *corr);
    Double_t sievecorr(Double_t x1, Double_t y1, Double_t *x, Double_t *y, Double_t *corr);
    Double_t momcorr1(Double_t x1, Double_t y1, Double_t *x, Double_t *y, Double_t *corr);
    

    Double_t sndcorr_p(Double_t x1, Double_t y1 , Double_t z1, Double_t *x, Double_t *y, Double_t *z, Double_t *corr);
    Double_t foucorr_p(Double_t x1, Double_t y1, Double_t z1, Double_t *x, Double_t *y, Double_t *z, Double_t *corr);
    Double_t momcorr1_p(Double_t x1, Double_t y1, Double_t *x, Double_t *y, Double_t *corr);
    Int_t check(Int_t qx, Int_t qx1, Int_t qx2);
    
  protected:
    Double_t vz_x[12][46],vz_y[12][46],vz_z[12][46][7],vz_corr[12][46][7];
    Double_t vz_x1[9][24],vz_y1[9][24],vz_z1[9][24][7],vz_corr1[9][24][7];
    Double_t vz_x2[9][16],vz_y2[9][16],vz_z2[9][16][7],vz_corr2[9][16][7];
    Double_t sieve_x[22][12],sieve_y[22][12],sieve_corr[22][12];
    Double_t mom_x[11][24],mom_y[11][24],mom_corr1[11][24],mom_corr2[11][24],mom_corr3[11][24];
    Double_t mom1_x[11][24],mom1_y[11][24],mom1_corr1[11][24],mom1_corr2[11][24];

    Double_t xp1[12][26],yp1[12][26],zp1[12][26][5],corrp1[12][26][5];
    Double_t xp2[11][16],yp2[11][16],zp2[11][16][5],corrp2[11][16][5];
    Double_t xp4[12][12],yp4[12][12],corrp4_1[12][12],corrp4_2[12][12];

    Double_t middle_pos,middle_angle;
    
    vector<double> sieve_pos;
    vector<double> sieve_angle;
    vector<double> chamber_angle;
    vector<double> chamber_pos;
    
    Int_t nofield_flag;
    Int_t calib_flag;
    
    Double_t sievex;      //output:  sieve_x           sieve CS     x left , y up, z into the magnet from front
    Double_t sievey;      // output: sieve_y           sieve CS
    Double_t sievexp;     // output: sieve_xp          sieve CS
    Double_t sieveyp;     // output" sieve_yp          sieve CS
    Double_t bendx;        // middle plane x (left)
    Double_t bendy;       // middle plan y (up)
    Double_t charge;       // charge of the particle
    Double_t vzflag;       // acceptance cut for optics
    Double_t bend_theta;   // bend angle
    Double_t theta_elastic;  // theta for elastics calculation
    Double_t sieve_flag;     // sieve
    Double_t sieve_flag2;     // sieve
    Double_t eloss;
		Double_t p_firstorder;
    
    TClonesArray* ftroptics;
    
  private:
    ClassDef(THaOpticsE06010,0);//optics class with analytical calculation
  };

#endif 
  
