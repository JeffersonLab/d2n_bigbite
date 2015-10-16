//////////////////////////////////////////////////////////////////////////
//
//    THaOptics
//
//  Read in track X,Y,Theta,Phi in TRCS and fill in any thing else in THaTrack
//
//////////////////////////////////////////////////////////////////////////
//    
// Author: Jin Huang <mailto:jinhuang@jlab.org>    Jan 2008
//    Modify History:
//      Mar 2007    Jin Huang <mailto:jinhuang@jlab.org>
//          Modify ApplyOptics to give a central trajectory 
//
//////////////////////////////////////////////////////////////////////////
/*


THIS IS an abstract class basis for general optics class.
It has following functions:
> handles general optics interface
> managing beam information 
> supporting coordinate transformation (see child class TOpticsDir and TOpticsPos).
> supporting some geometry calculation such as CrossingSurfLine3D(...)



TERMINOLOGY used in this class follows conventions of Hall A 
Tech Notes JLAB-TN-02-012:  
http://hallaweb.jlab.org/publications/Technotes/files/2002/02-012.pdf



DERIVED CLASS should reload function of virtual Int_t ApplyOptics(...).



FOUR DERIVED OPTICS CLASS have been implemented, each with difference algorithm,
but using same interface defined by this class. They are THaOpticsAnalytical,
THaOpticsHRS, THaOpticsNoFeild and THaOpticsAGen.



USAGE: to make a spectrometer support this series of optics classes,
following code have to be included:

0.#include "THaOptics.h"

1.in class definition:
public:
virtual Bool_t   MountOptics(THaOptics *);
protected:
TRef            fOptics;  //pointer to optics calculation class
public:
inline THaOptics *  GetOptics() const
{return (THaOptics *)(fOptics.GetObject());}

2.in destructor:
if (fOptics.IsValid()) delete (THaOptics *)(fOptics.GetObject());

3.anywhere in spectrometer.cxx:
Bool_t THaBigBite::MountOptics(THaOptics *Optics)
{
//mount a optics class to the Spectrometer. 
//return true if successful.
//Notice: This function have to be called 
//before Init() to make sure readdatebase
//        is called for the optics class.

if (fOptics.IsValid()) {
DEBUG_WARNING("MountOptics",
"The Optics has already been mounted. Deleting the old one and mounting new.");
delete fOptics.GetObject();
}

fOptics=Optics;

return fOptics.IsValid();
}

4.reload THaSpectrometer::FindVertices(TClonesArray& tracks ) and add:
if (fOptics.IsValid()) 
{
Int_t n_exist = tracks.GetLast()+1;

for( Int_t t = 0; t < n_exist; t++ ) {
THaTrack* theTrack = static_cast<THaTrack*>( tracks.At(t) );
assert(fOptics.IsValid());
((THaOptics *)(fOptics.GetObject()))-> ApplyOptics (theTrack);
}
}

5.reload THaSpectrometer::Init( const TDatime& run_time ) and add:
THaAnalysisObject::EStatus stat=THaSpectrometer::Init(run_time);
if (stat!=kOK) return stat;

//Optics is the last one to be initialized. 
//So it could use any other variable
if (fOptics.IsValid()) 
stat=((THaOptics *)(fOptics.GetObject()))->Init(run_time);

6.(optional)in constructor:
Bool_t mr=MountOptics(new <the optics class name>(<name>,<description>,this));

7.(optional)in the code, one can always reassign a difference optics
class to the spectrometer by calling MountOptics(THaOptics *);


_________________________________________________________________________

Child Class THaOptics::TOpticsDir

a vector class with information of it's Coordinate System
also handles Coordinate transformation. But it only handle 
rotation of the vector. It's useful to describe a direction
For example, p vector


_________________________________________________________________________

Child Class THaOptics::TOpticsPos

a vector class derived from THaOptics::TOpticsDir, specially designed 
for position coordinate, ex vertex.

*/

#include "THaSpectrometer.h"
#include "THaTrack.h"
#include "THaBeam.h"
#include "THaUnRasteredBeam.h"
#include "VarDef.h"
#include <iostream>
#include "TClass.h"
#include "TMath.h"
#include "TList.h"
using namespace std;

//put this header file below all other headers
#include "THaOptics.h"


THaOptics::THaOptics(const char* name, const char* desc, 
                     THaSpectrometer *pspec, TString strBeamDetectorName)
                     :THaAnalysisObject(name, desc), fSpec(pspec), fDefBeamName(strBeamDetectorName)
                     // constructor function of THaOptics
{
    DEBUG_LEVEL_RELATED_PERFORMACE_CHECKER;

    DEBUG_HALL_A_ANALYZER_DEBUGER_INIT;

    assert(fSpec.IsValid()); 
}
//_____________________________________________________________________________

THaOptics::~THaOptics() {}
//_____________________________________________________________________________

Int_t THaOptics::ReadDatabase(const TDatime& date)
{
    const char * here="ReadDatabase";
    FILE* file = OpenFile( date );
    if( !file ) return kFileError;
#if DEBUG_LEVEL>=3//start show info
    Info(Here(here),"\tDB File open OK" );
#endif//#if DEBUG_LEVEL>=3

    //********************* Beam *********************
    //test load fDefBeamName
    if( fDefBeamName.IsNull() or 
        (dynamic_cast<THaBeam *>( FindModuleNoCheck(fDefBeamName.Data(),"THaBeam") ))==NULL)
    {
#if DEBUG_LEVEL>=3//start show info
        Info(Here(here),"\tTrying to find beam detector info..." );
#endif//#if DEBUG_LEVEL>=3
        Int_t err =LoadDBvalue( file, date,"BeamDetectorName", fDefBeamName );

        if( err ){
            fDefBeamName="";
#if DEBUG_LEVEL>=3//start show info
            Info(Here(here),"\tNot Found" );
#endif//#if DEBUG_LEVEL>=3
        }
        else
        {
#if DEBUG_LEVEL>=3//start show info
            Info(Here(here),
                "\tFound. Will set default beam detector to be %s", 
                fDefBeamName.Data());
#endif//#if DEBUG_LEVEL>=3    #endif    
        }
    }

    //init fBeam
    if( !fDefBeamName.IsNull() )
    {
        fBeam = (( FindModuleNoCheck(fDefBeamName.Data(),"THaBeam") ));
	
        if (fBeam.IsValid())
        {
#if DEBUG_LEVEL>=3//start show info
            Info(Here(here),
                "\tTHaOptics is set to use beam detector : %s",
                fDefBeamName.Data());
#endif//#if DEBUG_LEVEL>=3    
        }
        else
        {
#if DEBUG_LEVEL>=1//start show warning 
            Warning(Here(here),
                "\tbeam detector with following name can not be found: %s" ,
                fDefBeamName.Data());
#endif//#if DEBUG_LEVEL>=1
        }
    }
    else
    {
        fBeam = NULL;
    }

    //********************* Rotations & off sets *********************
    //Double_t det_rot_x=0.0, det_rot_y = 0.0, det_rot_z = 0.0;
    //Double_t det_x=0.0, det_y=0.0, det_z=0.0;
    //Double_t tar_x=0.0, tar_y=0.0, tar_z=0.0;
    Double_t trans_x=0.0, trans_y=0.0, trans_z=0.0;
    Double_t trans_rot_x=0.0, trans_rot_y = 0.0, trans_rot_z = 0.0;
    static const Double_t degrad = TMath::Pi()/180.0;
    fThShift=fPhShift=fPShift=fPathLenShift=0;

    const TagDef tags[]={
        {"trans_rot_x", &trans_rot_x ,1},
        {"trans_rot_y", &trans_rot_y ,2},
        {"trans_rot_z", &trans_rot_z ,3},

        {"trans_x", &trans_x ,4},
        {"trans_y", &trans_y ,5},
        {"trans_z", &trans_z ,6},

        {"ThShift", &fThShift },
        {"PhShift", &fPhShift },
        {"PShift", &fPShift },
        {"PathLenShift", &fPathLenShift },

        //{"det_rot_x", &det_rot_x },
        //{"det_rot_y", &det_rot_y },
        //{"det_rot_z", &det_rot_z },
        //{"det_x", &det_x },
        //{"det_y", &det_y },
        //{"det_z", &det_z },
        //{"target_x", &tar_x },
        //{"target_y", &tar_y },
        //{"target_z", &tar_z },
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
    sDebugOutput+="\n \ttrans_rot_x\t= ";    sDebugOutput+=trans_rot_x;
    sDebugOutput+="\n \ttrans_rot_y\t= ";    sDebugOutput+=trans_rot_y;
    sDebugOutput+="\n \ttrans_rot_z\t= ";    sDebugOutput+=trans_rot_z;

    sDebugOutput+="\n \ttrans_x\t= ";sDebugOutput+=trans_x;
    sDebugOutput+="\n \ttrans_y\t= ";sDebugOutput+=trans_y;
    sDebugOutput+="\n \ttrans_z\t= ";sDebugOutput+=trans_z;

    sDebugOutput+="\n \t fThShift\t= ";sDebugOutput+=fThShift;
    sDebugOutput+="\n \t fPhShift\t= ";sDebugOutput+=fPhShift;
    sDebugOutput+="\n \t fPShift\t= ";sDebugOutput+=fPShift;
    sDebugOutput+="\n \t fPathLenShift\t= ";sDebugOutput+=fPathLenShift;

    //sDebugOutput+="\n \ttarget_x\t= ";sDebugOutput+=tar_x;
    //sDebugOutput+="\n \ttarget_y\t= ";sDebugOutput+=tar_y;
    //sDebugOutput+="\n \ttarget_z\t= ";sDebugOutput+=tar_z;
    sDebugOutput+="\n";
    Info(Here(here),sDebugOutput.Data());
#endif    

    //fTCSOffSetInHCS.SetXYZ(tar_x,tar_y,tar_z);
    //fDCSOffSetInTCS.SetXYZ(det_x,det_y,det_z);
    fTRCSOffSetInTCS.SetXYZ(trans_x,trans_y,trans_z);

    assert(fSpec.IsValid());
    fHCS2TCS=((THaSpectrometer *)fSpec.GetObject())->GetToTraRot();
    fTCS2HCS=((THaSpectrometer *)fSpec.GetObject())->GetToLabRot();

    //fDCS2TCS.SetToIdentity();
    //fDCS2TCS.RotateX(det_rot_x*degrad)
    //    .RotateY(det_rot_y*degrad)
    //    .RotateZ(det_rot_z*degrad);
    //fTCS2DCS=fDCS2TCS.Inverse();

    fTRCS2TCS.SetToIdentity();
    fTRCS2TCS.RotateX(trans_rot_x*degrad)
        .RotateY(trans_rot_y*degrad)
        .RotateZ(trans_rot_z*degrad);
    fTCS2TRCS=fTRCS2TCS.Inverse();


    fclose(file);
    return THaAnalysisObject::ReadDatabase(date);
}

//_____________________________________________________________________________
Int_t THaOptics::ReadRunDatabase( const TDatime& date )
{
    // Query the run database for parameters specific to this spectrometer

    Int_t err = THaAnalysisObject::ReadRunDatabase( date );
    if( err ) return err;

    FILE* file = OpenRunDBFile( date );
    if( !file ) return kFileError;

    DEBUG_INFO(Here("ReadRunDatabase()"),"database opened.");

    Double_t off_x = 0.0, off_y = 0.0, off_z = 0.0;

    const TagDef tags[] = {
        { "off_x",    &off_x, 1},
        { "off_y",    &off_y, 2},
        { "off_z",    &off_z, 3},
        { "current",  &fCurrent, 4 },
        { 0 }
    };
    err = LoadDB( file, date, tags, ((THaSpectrometer *)(fSpec.GetObject()))->GetPrefix() );
    if( err ) {    
        if (err==4) fCurrent=-1;
        else if( err>0 )
            Error( Here("ReadRunDatabase()"), "Required tag %s%s missing in the "
            "run database.\nSpectrometer initialization failed.",
            fPrefix, tags[err-1].name );
        fclose(file);
        return kInitError;
    }        
    fTCSOffSetInHCS.SetXYZ( off_x, off_y, off_z );

    fclose(file);

#if DEBUG_LEVEL>=3//start show info
    TString sDebugOutput;
    sDebugOutput=GetName();
    sDebugOutput+=" ReadRunDatabase finish in successfully with:";
    sDebugOutput+="\n \t off_x\t= ";    sDebugOutput+=off_x;
    sDebugOutput+="\n \t off_y\t= ";    sDebugOutput+=off_y;
    sDebugOutput+="\n \t off_z\t= ";    sDebugOutput+=off_z;
    sDebugOutput+="\n \t fCurrent\t= ";    sDebugOutput+=fCurrent;
    sDebugOutput+="\n";
    Info(Here("ReadRunDatabase()"),sDebugOutput.Data());
#endif  
    return kOK;
}


//_____________________________________________________________________________
Int_t THaOptics::ApplyOptics(THaTrack *track)
{
    // gateway function. calculation optics for *track, and fill result back to it.
    // it will use the beam position provided by detector with name fDefBeamName,
    // as long as fDefBeamName is not NULL
    //
    // The calculation is reading in track X,Y,Theta,Phi in TRCS and fill in any 
    // thing else in THaTrack

    TVector3 beampos(0.0, 0.0, 0.0);
    TVector3 beamdir(0.0, 0.0, 1.0);
    if( fBeam.IsValid() ) {
      //((THaUnRasteredBeam *)(fBeam.GetObject()))->Reconstruct();
        beampos = ((THaBeam *)(fBeam.GetObject())) ->GetPosition();
        beamdir = ((THaBeam *)(fBeam.GetObject())) ->GetDirection();
    }
    else 
    {
#if DEBUG_LEVEL>=1//start show warning 
        Warning(Here("ApplyOptics"),"\t No beam detector setting. Assuming a beam on exact center line of the hall." );
#endif//#if DEBUG_LEVEL>=1
    }

    

    return ApplyOptics(track,beampos,beamdir);
}

//_____________________________________________________________________________
Int_t THaOptics::ApplyOptics(THaTrack *track, TVector3 beampos, TVector3 beamdir)
{
    //Read in track X,Y,Theta,Phi in DCS and fill in any thing else in THaTrack

    if( !track ) {
#if DEBUG_LEVEL>=1//start show warning 
        Warning(Here("ApplyOptics"),"\tEmpty track input");
        return kInitError;
#endif//#if DEBUG_LEVEL>=1
    }

    //variables
    Double_t TargetX;
    Double_t TargetY;
    Double_t TargetTheta;
    Double_t TargetPhi;

    Double_t p=0, PathLen=0;    
    TVector3 vertex, vp;

    //do it
    Int_t statustmp;
    statustmp=ApplyOptics(
        track->GetX(),
        track->GetY(),
        track->GetTheta(),
        track->GetPhi(),
        beampos,
        beamdir,
        vp, 
        vertex,
        TargetX,
        TargetY,
        TargetTheta,
        TargetPhi,
        PathLen
        );  
    if (statustmp!=kOK) return statustmp;

    //feed back to track
    p=vp.Mag();

    track->SetTarget(
        TargetX,
        TargetY,
        TargetTheta,
        TargetPhi
        );
    track->SetPathLen( PathLen );
    track->SetVertex( vertex );
    track->SetPvect( vp );
    track->SetMomentum( p );
    track->SetDp( p/GetPcentral()-1 );

    return kOK;
}

//_____________________________________________________________________________

Int_t THaOptics::ApplyOptics(
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
    //gateway function to main optics calculation.
    //any derived optics class should reload this function.
    //
    //as base class, THaOptics is not supposed to provide any optics calculation,
    //therefore THaOptics::ApplyOptics only outputs feature of a central trajectory
    //
    //Warning: THaOptics::ApplyOptics is for test only.

    DEBUG_LINE_INFO(AbstractMethod("ApplyOptics"));
    //warning a derived class should reload this function

    //return features of central trajectory
    TOpticsDir pcentral(TOpticsDir::kTCS,this,0,0,GetPcentral());
    pcentral.TransCoordSys(TOpticsDir::kHCS);
    P=pcentral.GetVector();

    TOpticsPos vertexcentral(TOpticsDir::kTCS,this,0,0,0);
    vertexcentral.TransCoordSys(TOpticsDir::kHCS);
    Vertex=vertexcentral.GetVector();
    TargetX=TargetY=TargetTheta=TargetPhi=0;

    //extreme rough guess of path length 
    PathLen=2*GetCollDist();

    return kOK;
}

//_____________________________________________________________________________
THaAnalysisObject* THaOptics::FindModuleNoCheck( const char* name,
                                                const char* classname )
{
    // Locate the object 'name' in the global list of Analysis Modules 
    // and check if it inherits from 'classname' (if given), 
    // BUT, I do NOT check whether the modules is properly initialized.
    //
    // Return pointer to valid object, else return NULL and set fStatus.
  /*
    static const char* const here = "FindModuleNoCheck()";
    static const char* const anaobj = "THaAnalysisObject";

    if( !name || !*name ) {
        Error( Here(here), "No module name given." );
        return NULL;
    }
    DEBUG_INFO( Here(here), 
        "Looking for Modules with name %s and derived from class %s."
        , name, classname );

    DEBUG_INFO( Here(here), "Looking for Module %s in fgModules.",
            name);
    TObject* obj = fgModules->FindObject( name );

    if( !obj ) {
        DEBUG_INFO( Here(here), "Module %s does not exist.", name );
        return NULL;
    }
    DEBUG_INFO( Here(here), "Module %s (%s) is found.",
            obj->GetName(), obj->GetTitle() );

    if( !obj->IsA()->InheritsFrom( anaobj )) {
        DEBUG_INFO( Here(here), "Module %s (%s) is not a %s.",
            obj->GetName(), obj->GetTitle(), anaobj );
        return NULL;
    }
    if( classname && *classname && strcmp(classname,anaobj) &&
        !obj->IsA()->InheritsFrom( classname )) {
            DEBUG_INFO( Here(here), "Module %s (%s) is not a %s.",
                obj->GetName(), obj->GetTitle(), classname );
            return NULL;
    }
    THaAnalysisObject* aobj = static_cast<THaAnalysisObject*>( obj );
    assert(aobj);
    //if( !aobj->IsOK() ) {
    //  Error( Here(here), "Module %s (%s) not initialized.",
    //  obj->GetName(), obj->GetTitle() );
    //  return NULL;
    //}
    return aobj;*/

    return FindModule(name,classname,false);
}


//_____________________________________________________________________________
void THaOptics::MakePrefix()
{
    // Set up name prefix to include spectrumeter name
    const char* basename = NULL;
    if( fSpec.IsValid() )
        basename = ((THaSpectrometer *)(fSpec.GetObject()))->GetName();
    THaAnalysisObject::MakePrefix( basename );

}

//_____________________________________________________________________________
Bool_t THaOptics::SetBeamName(const char* name)
{
    //override the beam detector setting.
    //return 0 if successfull

    fDefBeamName=name;

    fBeam = (( FindModuleNoCheck(name,"THaBeam") ));

    return fBeam.IsValid();
}

//_____________________________________________________________________________
Double_t THaOptics::GetPcentral() const
{
    //return the central momentum

    assert(fSpec.IsValid()); 

    return ((THaSpectrometer *)(fSpec.GetObject())) ->GetPcentral();

}
//_____________________________________________________________________________
Double_t THaOptics::GetCollDist() const
{
    //return the central momentum

    assert(fSpec.IsValid()); 

    return ((THaSpectrometer *)(fSpec.GetObject())) ->GetCollDist();

}
//_____________________________________________________________________________

TVector2 THaOptics::CrossingLineLine2D(TVector2 o1,TVector2 d1,TVector2 o2,TVector2 d2)
{
    //return crossing point of two 2D lines. 
    //Each line is defined by a point on the line oi and an direction di

    Double_t x1=o1.X();
    Double_t y1=o1.Y();
    Double_t d1x=d1.X();
    Double_t d1y=d1.Y();

    Double_t x2=o2.X();
    Double_t y2=o2.Y();
    Double_t d2x=d2.X();
    Double_t d2y=d2.Y();

    if (-(d1y*d2x) + d1x*d2y==0)
    {
        DEBUG_WARNING(Here("Crossing"),"the two input line have crossing.");
        return TVector2(0,0);
    }

    return TVector2(
        -((d1y*d2x*x1 - d1x*d2y*x2 - d1x*d2x*y1 + d1x*d2x*y2)/(-(d1y*d2x) + d1x*d2y)),
        -((-(d1y*d2y*x1) + d1y*d2y*x2 + d1x*d2y*y1 - d1y*d2x*y2)/(d1y*d2x - d1x*d2y))
        );
}
//_____________________________________________________________________________

TVector3 THaOptics::CrossingSurfLine3D
(TVector3 OSurf,TVector3 NSurf,TVector3 OLine,TVector3 DLine)
{
    //return crossing point of a surface and a line. 
    //surface is defined by a point on it and normal direction
    //line is defined by a point on it and its direction

    Double_t x,y,z;

    x=(DLine.Y()*NSurf.Y()*OLine.X() + DLine.Z()*NSurf.Z()*OLine.X() + 
        DLine.X()*(NSurf.X()*OSurf.X() + NSurf.Y()*(-OLine.Y() + OSurf.Y()) + 
        NSurf.Z()*(-OLine.Z() + OSurf.Z())))/
        (DLine.X()*NSurf.X() + DLine.Y()*NSurf.Y() + DLine.Z()*NSurf.Z());

    y=((DLine.X()*NSurf.X() + DLine.Z()*NSurf.Z())*OLine.Y() + 
        DLine.Y()*(NSurf.X()*(-OLine.X() + OSurf.X()) + NSurf.Y()*OSurf.Y() + 
        NSurf.Z()*(-OLine.Z() + OSurf.Z())))/
        (DLine.X()*NSurf.X() + DLine.Y()*NSurf.Y() + DLine.Z()*NSurf.Z());

    z=((DLine.X()*NSurf.X() + DLine.Y()*NSurf.Y())*OLine.Z() + 
        DLine.Z()*(NSurf.X()*(-OLine.X() + OSurf.X()) + NSurf.Y()*(-OLine.Y() + OSurf.Y()) + 
        NSurf.Z()*OSurf.Z()))/(DLine.X()*NSurf.X() + DLine.Y()*NSurf.Y() + DLine.Z()*NSurf.Z());

    return TVector3(x,y,z);
}

//_____________________________________________________________________________

ClassImp(THaOptics);

//_____________________________________________________________________________

THaOptics::TOpticsDir & THaOptics::TOpticsDir::TransCoordSys(ECoordSys coordsys,bool dorecurent)
{
    //rotate the vector to the coordinate system specified by coordsys
    //dorecurent=false is only useful for TOpticsPos reusing this function

#if DEBUG_LEVEL>=4//massive info 
    Info("TransCoordSys","\tSetCoordSys called to set to coord %d with current state:",coordsys );
    Print();
#endif//#if DEBUG_LEVEL>=4

    if (coordsys>fCoordSys)
    {
        switch (fCoordSys)
        {
        case kHCS:
            Transform(((THaOptics *)(fOptics.GetObject()))->fHCS2TCS);
            break;
        case kTCS:
            Transform(((THaOptics *)(fOptics.GetObject()))->fTCS2TRCS);
            break;
            //case kDCS:
            //    Transform(((THaOptics *)(fOptics.GetObject()))->fDCS2TRCS);
        case kTRCS:
            TRCS2FCS();
            break;
        default:
#if DEBUG_LEVEL>=1//start show warning 
            Warning("TOpticsDir::TransCoordSys","\tUnexpected coordinate type encountered. Coordinate Transform request ignored." );
#endif//#if DEBUG_LEVEL>=1
            return *this;
            break;
        }
        fCoordSys=(ECoordSys)(fCoordSys+1);
    }
    else if (coordsys<fCoordSys)
    {
        switch (fCoordSys)
        {
        case kTCS:
            Transform(((THaOptics *)(fOptics.GetObject()))->fTCS2HCS);
            //case kDCS:
            //    Transform(((THaOptics *)(fOptics.GetObject()))->fDCS2TCS);
            break;
        case kTRCS:
            Transform(((THaOptics *)(fOptics.GetObject()))->fTRCS2TCS);            
            break;
        case kFCS:
            FCS2TRCS();            
            break;
        default:
#if DEBUG_LEVEL>=1//start show warning 
            Warning("TOpticsDir::TransCoordSys","\tUnexpected coordinate type encountered. Coordinate Transform request ignored." );
#endif//#if DEBUG_LEVEL>=1
            return *this;
            break;
        }
        fCoordSys=(ECoordSys)(fCoordSys-1);
    }
    else return *this;

    if (dorecurent) return TransCoordSys(coordsys);
    else return *this;
}
//_____________________________________________________________________________

void THaOptics::TOpticsDir::TRCS2FCS()
{
    //reload this function to perform TRCS2FCS trans
#if DEBUG_LEVEL>=1//start show warning 
    Warning("TRCS2FCS","\tFunction TRCS2FCS has not been implemented. Please Reload THaOptics::TOpticsDir::TRCS2FCS()" );
#endif//#if DEBUG_LEVEL>=1
}

void THaOptics::TOpticsDir::FCS2TRCS()
{
    //reload this function to perform FCS2TRCS trans
#if DEBUG_LEVEL>=1//start show warning 
    Warning("FCS2TRCS","\tFunction FCS2TRCS has not been implemented. Please Reload THaOptics::TOpticsDir::FCS2TRCS()" );
#endif//#if DEBUG_LEVEL>=1
}

//_____________________________________________________________________________

THaOptics::TOpticsDir & THaOptics::TOpticsDir::operator += (const THaOptics::TOpticsDir & p) 
{     
    // Addition with fool-proof check on Coordinate System consistency.

    assert(fCoordSys==p.fCoordSys);
    assert(GetOptics()==p.GetOptics());
    SetXYZ(X()+p.X(),Y()+p.Y(),Z()+p.Z());
    return *this;
}

THaOptics::TOpticsDir & THaOptics::TOpticsDir::operator -= (const THaOptics::TOpticsDir & p) 
{
    // Subtraction with fool-proof check on Coordinate System consistency.

    assert(fCoordSys==p.fCoordSys);
    assert(GetOptics()==p.GetOptics());
    SetXYZ(X()-p.X(),Y()-p.Y(),Z()-p.Z());
    return *this;
}

THaOptics::TOpticsDir operator + (const THaOptics::TOpticsDir & a, const THaOptics::TOpticsDir & b) 
{
    assert(a.GetOptics()==b.GetOptics());
    assert(a.GetCoordSys()==b.GetCoordSys());
    return THaOptics::TOpticsDir(a.GetCoordSys(),a.GetOptics(),
        a.X() + b.X(), a.Y() + b.Y(), a.Z() + b.Z());
}

THaOptics::TOpticsDir operator - (const THaOptics::TOpticsDir & a, const THaOptics::TOpticsDir & b) 
{
    assert(a.GetOptics()==b.GetOptics());
    assert(a.GetCoordSys()==b.GetCoordSys());
    return THaOptics::TOpticsDir(a.GetCoordSys(),a.GetOptics(),
        a.X() - b.X(), a.Y() - b.Y(), a.Z() - b.Z());
}

THaOptics::TOpticsDir operator + (const THaOptics::TOpticsDir & a, const TVector3 & b) 
{
    return THaOptics::TOpticsDir(a.GetCoordSys(),a.GetOptics(),
        a.X() + b.X(), a.Y() + b.Y(), a.Z() + b.Z());
}

THaOptics::TOpticsDir operator - (const THaOptics::TOpticsDir & a, const TVector3 & b) 
{
    return THaOptics::TOpticsDir(a.GetCoordSys(),a.GetOptics(),
        a.X() - b.X(), a.Y() - b.Y(), a.Z() - b.Z());
}

Double_t operator * (const THaOptics::TOpticsDir &a, const THaOptics::TOpticsDir &b)
{
    assert(a.GetOptics()==b.GetOptics());
    assert(a.GetCoordSys()==b.GetCoordSys());
    return a.Dot(b);
}

Double_t operator * (const THaOptics::TOpticsDir &a, const TVector3 &b)
{
    return a.Dot(b);
}

THaOptics::TOpticsDir operator * (const THaOptics::TOpticsDir & p, Double_t a) {
    return THaOptics::TOpticsDir(p.GetCoordSys(),p.GetOptics(),
        a*p.X(), a*p.Y(), a*p.Z());
}

THaOptics::TOpticsDir operator * (Double_t a, const THaOptics::TOpticsDir & p) {
    return THaOptics::TOpticsDir(p.GetCoordSys(),p.GetOptics(),
        a*p.X(), a*p.Y(), a*p.Z());
}


//_____________________________________________________________________________
void THaOptics::TOpticsDir::Print( Option_t* opt ) const
{

    Printf("%s %s (x,y,z)=(%f,%f,%f) (rho,theta,phi)=(%f,%f,%f) fCoordSys=%d",
        GetName(),GetTitle(),X(),Y(),Z(),
        Mag(),Theta()*TMath::RadToDeg(),Phi()*TMath::RadToDeg(),
        fCoordSys);

    return;
}

//_____________________________________________________________________________
//
THaOptics::TOpticsPos & THaOptics::TOpticsPos::TransCoordSys(ECoordSys coordsys,bool dorecurent)
{
    //rotate the vector to the coordinate system specified by coordsys

    if (coordsys>fCoordSys)
    {
        switch (fCoordSys)
        {
        case kHCS:
            *this-=((THaOptics *)(fOptics.GetObject()))->fTCSOffSetInHCS;
            break;
        case kTCS:
            *this-=((THaOptics *)(fOptics.GetObject()))->fTRCSOffSetInTCS;
            //case kDCS:
            //    *this-=((THaOptics *)(fOptics.GetObject()))->fTRCSOffSetInDCS;
            break;
        case kTRCS:
            //nothing needed
            break;
        default:
#if DEBUG_LEVEL>=1//start show warning 
            Warning("TOpticsPos::TransCoordSys","\tUnexpected coordinate type encountered. Coordinate Transform request ignored." );
#endif//#if DEBUG_LEVEL>=1
            return *this;
            break;
        }//switch (fOptics)
        TOpticsDir::TransCoordSys(coordsys,false);
    }
    else if (coordsys<fCoordSys)
    {
        TOpticsDir::TransCoordSys(coordsys,false);
        switch (fCoordSys+1)
        {
        case kTCS:
            *this+=((THaOptics *)(fOptics.GetObject()))->fTCSOffSetInHCS;
            break;
            //case kDCS:
            //    *this+=((THaOptics *)(fOptics.GetObject()))->fDCSOffSetInTCS;
            break;
        case kTRCS:
            *this+=((THaOptics *)(fOptics.GetObject()))->fTRCSOffSetInTCS;
            break;
        case kFCS:
            //nothing needed
            break;
        default:
#if DEBUG_LEVEL>=1//start show warning 
            Warning("TOpticsPos::TransCoordSys","\tUnexpected coordinate type encountered. Coordinate Transform request ignored." );
#endif//#if DEBUG_LEVEL>=1
            return *this;
            break;
        }//switch (fOptics)
    }
    else return *this;

    if (dorecurent) return TransCoordSys(coordsys);
    else return *this;
}

//_____________________________________________________________________________
ClassImp(THaOptics::TOpticsPos);
ClassImp(THaOptics::TOpticsDir);


////_____________________________________________________________________________
//
//const char* THaAnalysisObject::Here( const char* here ) const
//{
//  // Return a string consisting of 'here' followed by fPrefix.
//  // Used for generating diagnostic messages.
//  // The return value points to an internal static buffer that
//  // one should not try to delete ;)
//
//  static char buf[256];
//  strcpy( buf, "\"" );
//  if(fPrefix != NULL) {
//    strcat( buf, fPrefix );
//    *(buf+strlen(buf)-1) = 0;   // Delete trailing dot of prefix
//  }
//  strcat( buf, "\"::" );
//  strcat( buf, here );
//  return buf;
//}

//_____________________________________________________________________________

//void THaOptics::ExtractTRCSInfo(
//    THaTrack *track,      //input
//    Double_t &trackX,    //output:    Track X             TRCS
//    Double_t &trackY,    //output:    Track Y             TRCS
//    Double_t &trackTheta,//output:    Track Theta         TRCS
//    Double_t &trackPhi   //output:    Track Phi           TRCS
//    )
//{
//    // calculate TRCS variables from DCS info provided by THaTrack
//    // Reload this function if you get better idea
//
//    Double_t x0 = track->GetDX();
//    Double_t y0 = track->GetDY();
//    Double_t xp = track->GetDTheta();
//    Double_t yp = track->GetDPhi();
//
//    TOpticsDir vp(TOpticsDir::kDCS,this,xp,yp,1);
//    TOpticsPos pos(TOpticsDir::kDCS,this,x0,y0,0);
//
//    vp.TransCoordSys(TOpticsDir::kTRCS);
//    pos.TransCoordSys(TOpticsDir::kTRCS);
//
//    trackTheta=vp.X()/vp.Z();
//    trackPhi=vp.Y()/vp.Z();
//
//    trackX=pos.X()-pos.Z()*trackTheta;
//    trackY=pos.Y()-pos.Z()*trackPhi;
//
//    //WARNING: OVERWRITE TRCS INFO IN *TRACK
//#if DEBUG_LEVEL>=3//start show info
//    Info(Here("ExtractTRCSInfo"),"\tCompability Warning: overwriting TRCS info in *track."
//        "Make sure DCS to TRCS transform info in Database is correct.");
//#endif//#if DEBUG_LEVEL>=3
//    track->Set(
//         trackX,    // Track X             TRCS
//         trackY,    // Track Y             TRCS
//         trackTheta,// Track Theta         TRCS
//         trackPhi  // Track Phi           TRCS
//        );
//
//    return;
//}


