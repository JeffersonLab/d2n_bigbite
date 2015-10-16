//////////////////////////////////////////////////////////////////////////
//
//    THaOpticsAGen
//
//  Derived class of THaOptics, which uses the optics algorithm for 
//  BigBite developed by AGen experiment
//    
//////////////////////////////////////////////////////////////////////////
//    
// Author: Jin Huang <mailto:jinhuang@jlab.org>    Jan 2008
//          Reference to AGenLib code of Bodo Reitz and Seamus Riordan
//    Modify History:
//
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_THaOpticsAGen
#define ROOT_THaOpticsAGen

#include "THaOptics.h"

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
//    >=4    above + massive info (in a for or while)
#define DEBUG_LEVEL   2

#include    "DebugDef.h"
//------------------------------------------------------//


class THaOpticsAGen : public THaOptics 
{
public:
    THaOpticsAGen(const char* name, const char* desc, THaSpectrometer *pspec, TString strBeamDetectorName="");
    virtual ~THaOpticsAGen();

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

protected:
    Double_t        fC[10];          // Simple dipole coefficients
    Double_t        fCv[10];         // Vertex Coefficients
    //TVector3        fMagnetCenter;  // point at center of BigBite Magnet
    TVector3        fBendPlaneCenter;
    Double_t        fBendTheta;

private:
    ClassDef(THaOpticsAGen,1);//optics class with old AGen algorithm
};




#endif

