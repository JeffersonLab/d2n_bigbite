//////////////////////////////////////////////////////////////////////////
//
//    THaOpticsNoFeild
//
// class THaOpticsNoFeild is an specilized optics class, designed for 
// test runs with magnetic field turned off.
//
//////////////////////////////////////////////////////////////////////////
//    
// Author: Jin Huang <mailto:jinhuang@jlab.org>    Mar 2008
//    Modify History:
//
//////////////////////////////////////////////////////////////////////////

#include "THaSpectrometer.h"
#include "THaTrack.h"
#include "THaBeam.h"
#include "VarDef.h"
#include <iostream>
#include "TClass.h"
#include "TList.h"
#include "TString.h"

using namespace std;
//_____________________________________________________________________________

//put this header file below all other headers
#include "THaOpticsNoFeild.h"
//_____________________________________________________________________________

THaOpticsNoFeild::THaOpticsNoFeild(const char* name, const char* desc, THaSpectrometer *pspec, TString strBeamDetectorName)
:THaOptics(name,desc,pspec,strBeamDetectorName)
// constructor function of THaOpticsNoFeild
{
    DEBUG_LEVEL_RELATED_PERFORMACE_CHECKER;

    DEBUG_HALL_A_ANALYZER_DEBUGER_INIT;
}
//_____________________________________________________________________________

THaOpticsNoFeild::~THaOpticsNoFeild() {}
//_____________________________________________________________________________

Int_t THaOpticsNoFeild::ReadDatabase(const TDatime& date)
{
    THaOptics::ReadDatabase(date);
    static const char* const here = "ReadDatabase";
    //const int LEN = 200;
    //char buff[LEN];
    FILE* file = OpenFile( date );
    if( !file ) return kFileError;

    DEBUG_INFO(Here(here),"\tDB File open OK" );



    fclose(file);
    return kOK;
}
//_____________________________________________________________________________

Int_t THaOpticsNoFeild::
ApplyOptics(
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
            )
{
    //reloaded gateway function do the calculation.


    return kOK;
};

//_____________________________________________________________________________


