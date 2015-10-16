//////////////////////////////////////////////////////////////////////////
//
//    THaOpticsNoFeild
//
//////////////////////////////////////////////////////////////////////////
//    
// Author: Jin Huang <mailto:jinhuang@jlab.org>    Mar 2008
//    Modify History:
//
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_THaOpticsNoFeild
#define ROOT_THaOpticsNoFeild

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
#define DEBUG_LEVEL   4

#include    "DebugDef.h"
//------------------------------------------------------//


class THaOpticsNoFeild : public THaOptics 
{
public:
    THaOpticsNoFeild(const char* name, const char* desc, THaSpectrometer *pspec, TString strBeamDetectorName="");
    virtual ~THaOpticsNoFeild();

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

protected:
    virtual Int_t   ReadDatabase( const TDatime& date );


private:
    ClassDef(THaOpticsNoFeild,0);//optics class with analytical calculation
};

#endif

