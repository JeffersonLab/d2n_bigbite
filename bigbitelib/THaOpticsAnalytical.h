//////////////////////////////////////////////////////////////////////////
//
//    THaOpticsAnalytical
//
//////////////////////////////////////////////////////////////////////////
//    
// Author: Jin Huang <mailto:jinhuang@jlab.org>    Feb 2008
//    Modify History:
//
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_THaOpticsAnalytical
#define ROOT_THaOpticsAnalytical

#include "THaOptics.h"

#include "TMath.h"
#include "TComplex.h"

class THaTrack;
class THaSpectrometer;
class TVector3;
class TString;
class THaBeam;

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


class THaOpticsAnalytical : public THaOptics 
{
public:
    THaOpticsAnalytical(const char* name, const char* desc, THaSpectrometer *pspec, TString strBeamDetectorName="");
    virtual ~THaOpticsAnalytical();

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
        Double_t &TargetX,  //output:   Target X            TCS
        Double_t &TargetY, //output:   Target Y            TCS
        Double_t &TargetTheta,//output: Target Theta        TCS
        Double_t &TargetPhi,//output:   Target Phi          TCS
        Double_t &PathLen   //output:   Length of path      any coordinate
        );

public://calculations
    Double_t    CalxA(
        Double_t xB,
        Double_t zB,
        Double_t TanF,
        Double_t xx,
        Double_t zz
        );

    Double_t    CalR(
        Double_t xA,
        Double_t xB,
        Double_t zB,
        Double_t TanF,
        Double_t xx,
        Double_t zz
        );

    Double_t SetRCentral();

public://service functions

    static inline TComplex CSqrt(Double_t x)
    {return TComplex::Sqrt(TComplex(x,0));}
    static inline TComplex CSqrt(TComplex x)
    { return TComplex::Sqrt(x); }
    static inline Double_t Power(Double_t x, Double_t y)
    { return TMath::Power(x, y); }
    static inline TComplex Power(TComplex x, Double_t y)
    { return TComplex::Power(x, y); }
    static inline TComplex Complex(Double_t re,Double_t im)
    {return TComplex(re,im);}

protected:
    virtual Int_t   ReadDatabase( const TDatime& date );

public://gettings & settings
    inline Double_t GetMagneticFieldFrontShift() {return fMagDL;}
    inline Double_t GetMagneticFieldRearSlopeAngle() {return  fMagTheta;}
    inline Double_t GetMagneticFieldTopCrossing() {return  fMagS;}
    inline Double_t GetCurveRadiusOfCentralMomentum() {return  fRCentral;}

    inline Double_t SetMagneticFieldFrontShift(Double_t v) {return fMagDL=v;}
    inline Double_t SetMagneticFieldRearSlopeAngle(Double_t v) {return  fMagTheta=v;}
    inline Double_t SetMagneticFieldTopCrossing(Double_t v) {return  fMagS=v;}
    inline Double_t SetCurveRadiusOfCentralMomentum(Double_t v) {return  fRCentral=v;}

protected:
    Double_t fMagDL;    //shift of front surface of magnetic field from sieve
    Double_t fMagTheta; //in rad, the angle between two surfaces of magnetic field
    Double_t fMagS;     //the distance between crossing line of two surfaces of magnetic field and central point of front surface
    Double_t fRCentral; //the  curve radius in magnetic field for a particle with central momentum
    Double_t fRadius2Momentum;//conversion constant of bending radius to momentum, units: GeV/m/c

public://gettings & settings
#   if DEBUG_LEVEL>1
    void Fitter(TString sInputFileName);
#   endif

private:
    ClassDef(THaOpticsAnalytical,0);//optics class with analytical calculation
};
#endif 

