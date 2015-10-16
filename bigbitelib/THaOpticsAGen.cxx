//////////////////////////////////////////////////////////////////////////
//
//    THaOpticsAGen
//
//  Derived class of THaOptics, which uses the optics algorithm for 
//  BigBite developed by AGen experiment
//    
//////////////////////////////////////////////////////////////////////////
//    
// Author: Jin Huang <mailto:jinhuang@jlab.org>     Jan 2008
//          Reference to AGenLib code of Bodo Reitz and Seamus Riordan
//    Modify History:
//
//////////////////////////////////////////////////////////////////////////

#include "THaSpectrometer.h"
#include "THaTrack.h"
#include "THaBeam.h"
#include "VarDef.h"
#include <iostream>
#include "TClass.h"
#include "TMath.h"
#include "TList.h"
using namespace std;

//put this header file below all other headers
#include "THaOpticsAGen.h"


THaOpticsAGen::THaOpticsAGen(const char* name, const char* desc, THaSpectrometer *pspec, TString strBeamDetectorName)
:THaOptics(name,desc,pspec,strBeamDetectorName)
// constructor function of THaOpticsAGen
{
    DEBUG_LEVEL_RELATED_PERFORMACE_CHECKER;


    DEBUG_HALL_A_ANALYZER_DEBUGER_INIT
}
//_____________________________________________________________________________

THaOpticsAGen::~THaOpticsAGen() {}
//_____________________________________________________________________________

Int_t THaOpticsAGen::ReadDatabase(const TDatime& date)
{
    THaOptics::ReadDatabase(date);
    const char * here="ReadDatabase";
    FILE* file = OpenFile( date );
    if( !file ) return kFileError;
#if DEBUG_LEVEL>=3//start show info
    Info(Here(here),"\tDB File open OK" );
#endif//#if DEBUG_LEVEL>=3

    //Double_t mag_x=0.0, mag_y=0.0, mag_z=0.0;
    Double_t bendz = 0.0, bendy = 0.0 , bendx = 0.0;
    Double_t bendthetadeg=0, trans_rot_y;

    fC[0] = 0.3;     // Inverse deflection angle coeff
    fC[1] = 0.0;     // X-bend inverse deflection angle coeff
    fC[2] = 0.0;     // theta targ (rad) coeff
    fC[3] = 0.0;     // y coeff
    fC[4] = 0.0;     // phi coeff
    fC[5] = 0.0;     // a coeff

    fCv[0] = 1.0;
    fCv[1] = 0.0;
    fCv[2] = 0.0;
    fCv[3] = 0.0;
    fCv[4] = 0.0;

    const TagDef tags[]={
        //{"magnet_x", &mag_x },
        //{"magnet_y", &mag_y },
        //{"magnet_z", &mag_z },
        {"effbendz", &bendz},
        {"effbendy", &bendy},
        {"effbendx", &bendx},
        {"effbendplanetheta", &bendthetadeg},
        {"trans_rot_y", &trans_rot_y },
        {"ciDef", &fC[0] },
        {"ciDefXbend", &fC[1] },
        {"cThetaTarg", &fC[2] }, 
        {"cY", &fC[3]},
        {"cPhi", &fC[4] },
        {"ca", &fC[5] },
        {"cV", &fCv[0] },
        {"cVX", &fCv[1] }, 
        {"cVXp", &fCv[2] },
        {"cVY", &fCv[3] }, 
        {"cVYp", &fCv[4] },
        { 0 }
    };
    Int_t err =LoadDB( file, date,tags, "" );
    if( err ){
        Error(Here(here), "Required tag %s missing in the "
            "run databse. \nBigBite initialization failed.",
            tags[err-1].name );
        fclose(file);
        return kInitError;
    }
#if DEBUG_LEVEL>=3//start show info
    TString sDebugOutput;
    sDebugOutput=GetName();
    sDebugOutput+=" Database read in successfully with:";
    sDebugOutput+="\n \teffbendz\t= ";    sDebugOutput+=bendz;
    sDebugOutput+="\n \teffbendy\t= ";    sDebugOutput+=bendy;
    sDebugOutput+="\n \teffbendx\t= ";    sDebugOutput+=bendx;
    sDebugOutput+="\n \teffbendplanetheta\t= ";sDebugOutput+=bendthetadeg;
    sDebugOutput+="\n \ttrans_rot_y\t= ";sDebugOutput+=trans_rot_y;
    sDebugOutput+="\n \tfC[]\t= ";
    sDebugOutput+=fC[0];sDebugOutput+=',';
    sDebugOutput+=fC[1];sDebugOutput+=',';
    sDebugOutput+=fC[2];sDebugOutput+=',';
    sDebugOutput+=fC[3];sDebugOutput+=',';
    sDebugOutput+=fC[4];sDebugOutput+=',';
    sDebugOutput+=fC[5];
    sDebugOutput+="\n \tfCv[]\t= ";
    sDebugOutput+=fCv[0];sDebugOutput+=',';
    sDebugOutput+=fCv[1];sDebugOutput+=',';
    sDebugOutput+=fCv[2];sDebugOutput+=',';
    sDebugOutput+=fCv[3];sDebugOutput+=',';
    sDebugOutput+=fCv[4];
    sDebugOutput+="\n";
    Info(Here(here),sDebugOutput.Data());
#endif    

    //fMagnetCenter.SetXYZ(mag_x,mag_y,mag_z);
    fBendPlaneCenter.SetXYZ( bendx, bendy, bendz );
    // fBendTheta is given in target coords, but we want trans coords
    fBendTheta = TMath::Pi()/180.0 * ( bendthetadeg + trans_rot_y );

    fclose(file);
    return kOK;
}
//_____________________________________________________________________________

Int_t THaOpticsAGen::ApplyOptics(
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
    //--- rough model of simple dipole
    //
    // Going to assume that magnet center is in target coords and fBendTheta 
    // is given in target coords but has already been transformed
    // track back to bending plane in detector coords

    DEBUG_MASSINFO("ApplyOptics","\t------------ ApplyOptics Entry ------------" );

    Double_t x0 = trackX;
    Double_t y0 = trackY;
    Double_t xp = trackTheta;
    Double_t yp = trackPhi;

    TOpticsDir beamdir(TOpticsDir::kHCS,this,beamDir);
    TOpticsPos beampos(TOpticsDir::kHCS,this,beamPos);

    beamdir.TransCoordSys(TOpticsDir::kTRCS);
    beampos.TransCoordSys(TOpticsDir::kTRCS);
#if DEBUG_LEVEL>=4//massive info 
    Info("ApplyOptics","\tbeamdir.Print();beampos.Print();" );
    beamdir.Print();
    beampos.Print();
#endif//#if DEBUG_LEVEL>=4

    // project to y = 0
    beampos.SetX( beampos.X() - beampos.Y()*(beamdir.X()/beamdir.Y()));
    beampos.SetZ( beampos.Z() - beampos.Y()*(beamdir.Z()/beamdir.Y()));
    beampos.SetY(0.0);

    TOpticsPos bendtarget(TOpticsPos::kTCS,this,fBendPlaneCenter);
    bendtarget.TransCoordSys(TOpticsPos::kTRCS);

#if DEBUG_LEVEL>=4//start show info
    Info("ApplyOptics","\tbendtarget.Print();fBendTheta=%f",fBendTheta);
    bendtarget.Print();
#endif//#if DEBUG_LEVEL>=3

    const Double_t bendz0 = bendtarget.Z();
    const Double_t bendx0 = bendtarget.X();
    const Double_t tanBendTheta = TMath::Tan(fBendTheta);

    TOpticsPos vAtDet(TOpticsPos::kTRCS,this,x0, y0, 0.0 );

    TOpticsPos bendpoint(TOpticsPos::kTRCS,this);
    bendpoint.SetZ((bendz0 + (x0-bendx0) * tanBendTheta)/(1-xp*tanBendTheta));
    bendpoint.SetX( x0 + xp * bendpoint.Z() );
    bendpoint.SetY( y0 + yp * bendpoint.Z() );

    TOpticsPos vAtBend(bendpoint);

    Double_t bendxint = bendpoint.X();//in TRCS
    Double_t bendyint = bendpoint.Y();//in TRCS
    Double_t bendzint = bendpoint.Z();//in TRCS

    DEBUG_MASSINFO("ApplyOptics","\tbendxint=%f;bendyint=%f;bendzint=%f;",bendxint,bendyint,bendzint );
    // 
    // this is our vertex
    // These are all still in detector coords

    TOpticsPos vertex(TOpticsPos::kTRCS,this);

    Double_t v0 = (y0+yp*beampos.Z()) / (1 - yp*(beamdir.Z()/beamdir.Y() ));

    DEBUG_MASSINFO("ApplyOptics","\tv0 = (y0+yp*beampos.Z()) / (1 - yp*(beamdir.Z()/beamdir.Y() ))=%f",v0 );

    vertex.SetY( v0 );
    vertex.SetX( beampos.X() + vertex.Y()*(beamdir.X()/beamdir.Y() ) );
    vertex.SetZ( beampos.Z() + vertex.Y()*(beamdir.Z()/beamdir.Y() ) );

    DEBUG_MASSINFO("ApplyOptics","\t-----Vertex is now in detector coords-----" );

    //Vertex is now in detector coords
    //Need in lab coords to apply our corrections

    vertex.TransCoordSys(TOpticsPos::kHCS);
    vertex.SetZ(v0);//?????????????????????????????
    vertex.TransCoordSys(TOpticsPos::kTRCS);

    TOpticsPos vAtBeam(vertex);

    TOpticsDir pdir(bendpoint-vertex);
    pdir = pdir.Unit();


    DEBUG_MASSINFO("ApplyOptics","\t--Now, we want target coordinates of the front track---" );

    // Now, we want target coordinates of the front track
    vertex.TransCoordSys(TOpticsPos::kTCS);
    pdir.TransCoordSys(TOpticsPos::kTCS);
    TOpticsDir pdirold(pdir);//????????????????????????????

    // not to transform???????????????????????????
    bendpoint.TransCoordSys(TOpticsPos::kTCS);
    bendpoint.SetAll(TOpticsDir::kTRCS,this,bendpoint.X(),bendpoint.y(),bendpoint.z());

    //track->SetTarget(vertex.X()- (pdir.X()/pdir.Z())*vertex.Z(), 
    //   vertex.Y()-(pdir.Y()/pdir.Z())*vertex.Z(),
    //   pdir.X()/pdir.Z(),
    //   pdir.Y()/pdir.Z() );

    // determine track direction after bend
    TOpticsDir backangle(TOpticsDir::kTRCS,this, xp, yp, 1.0 );
    backangle = backangle.Unit();
    backangle.TransCoordSys(TOpticsPos::kTCS);

    Double_t deflectionAngle = TMath::ACos( pdir*backangle );


    DEBUG_MASSINFO("ApplyOptics","\t--Now that we have a deflection angle we redo our vertex calculations with---" );

    // Now that we have a deflection angle we redo our vertex calculations with
    // a change in the front track y' 

    // This is the effect of dz changing from deflection
    // y0 -> bendy
    // y' -> y'/cos(theta_def)
    v0 = (bendyint+(yp/cos(deflectionAngle))*(beampos.Z()-bendzint))
        / (1 - (yp/cos(deflectionAngle))*(beamdir.Z()/beamdir.Y() ));


    DEBUG_MASSINFO("ApplyOptics","\tv0 = (bendyint+(yp/cos(deflectionAngle))*(beampos.Z()-bendzint))...=%f",v0 );


    vertex.SetAll(TOpticsDir::kTRCS,this);
    vertex.SetY( v0 );
    vertex.SetX( beampos.X() + vertex.Y()*(beamdir.X()/beamdir.Y() ) );
    vertex.SetZ( beampos.Z() + vertex.Y()*(beamdir.Z()/beamdir.Y() ) );
    vertex.TransCoordSys(TOpticsDir::kHCS);

    v0 = fCv[0]*vertex.Z() 
        + fCv[1]*x0 
        + fCv[2]*xp 
        + fCv[3]*y0 
        + fCv[4]*yp;


    DEBUG_MASSINFO("ApplyOptics","\tv0 =fCv[0]*vertex.Z() + fCv[1]*x0 + fCv[2]*xp + fCv[3]*y0 + fCv[4]*yp=%f",v0 );


    vertex.SetZ( v0 );//?????????????????????????????????

    vertex.TransCoordSys(TOpticsDir::kTRCS);

    vAtBeam = vertex;
    pdir = bendpoint - vertex;
    pdir = pdir.Unit();

#if DEBUG_LEVEL>=4//start show info
    Info("ApplyOptics","\tbendpoint.Print();vertex.Print();pdir.Print():" );
    bendpoint.Print();vertex.Print();pdir.Print();
#endif//#if DEBUG_LEVEL>=3
    vertex.TransCoordSys(TOpticsPos::kTCS);
    pdir.TransCoordSys(TOpticsPos::kTCS);

    TargetX=vertex.X()- (pdir.X()/pdir.Z())*vertex.Z();
    TargetY=vertex.Y()-(pdir.Y()/pdir.Z())*vertex.Z();
    TargetTheta=pdir.X()/pdir.Z();
    TargetPhi=pdir.Y()/pdir.Z();

    //??????????????????????????
    pdir=pdirold;

    Double_t p = 
        (fC[0] + fC[1]* bendxint)/deflectionAngle 
        + fC[2]*TMath::ATan(TargetTheta)
        + fC[3]*trackY 
        + fC[4]*trackPhi
        + fC[5];

#if DEBUG_LEVEL>=4//start show info
    TString sDebugOutput;
    sDebugOutput+="\n \tfC[]\t= ";
    sDebugOutput+=fC[0];sDebugOutput+=',';
    sDebugOutput+=fC[1];sDebugOutput+=',';
    sDebugOutput+=fC[2];sDebugOutput+=',';
    sDebugOutput+=fC[3];sDebugOutput+=',';
    sDebugOutput+=fC[4];sDebugOutput+=',';
    sDebugOutput+=fC[5];
    sDebugOutput+="\n \t bendxint\t= ";sDebugOutput+=bendxint;
    sDebugOutput+="\n \t deflectionAngle\t= ";sDebugOutput+=deflectionAngle;
    sDebugOutput+="\n \t TargetTheta\t= ";sDebugOutput+=TargetTheta;
    sDebugOutput+="\n \t trackY\t= ";sDebugOutput+=trackY;
    sDebugOutput+="\n \t trackPhi\t= ";sDebugOutput+=trackPhi;
    sDebugOutput+="\n \t p\t= ";sDebugOutput+=p;
    Info("ApplyOptics","\t%s\npdir.Print();",sDebugOutput.Data());
    pdir.Print();
#endif//#if DEBUG_LEVEL>=4

    pdir.TransCoordSys(TOpticsPos::kHCS);
    vertex.TransCoordSys(TOpticsPos::kHCS);

    Vertex=vertex.GetVector();
    P=pdir.GetVector()*p;

    TOpticsPos len1(vAtBend - vAtBeam), len2(vAtDet - vAtBend);
    PathLen=len1.Mag() + len2.Mag();

    return kOK;
};

//_____________________________________________________________________________

ClassImp(THaOpticsAGen);


