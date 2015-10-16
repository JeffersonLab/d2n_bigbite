//////////////////////////////////////////////////////////////////////////
//
//    THaOptics
//
//  Read in track X,Y,Theta,Phi in TRCS and fill in any thing else in THaTrack
//
//  abstract class basis for general optics class
//  handle general optics interface, manage beam information
//  derived class should implete the last reload function of ApplyOptics
//    
//////////////////////////////////////////////////////////////////////////
//    
// Author: Jin Huang <mailto:jinhuang@jlab.org>    Jan 2008
//    Modify History:
//
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_THaOptics
#define ROOT_THaOptics

#include "THaAnalysisObject.h"
#include "TVector3.h"
#include "TVector2.h"
#include "TRotation.h"
#include "Rtypes.h"
#include "TRef.h"

class THaTrack;
class THaSpectrometer;
class TString;
class THaBeam;
class TVector2;

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


class THaOptics : public THaAnalysisObject 
{
public:
    class TOpticsDir;
    class TOpticsPos;

    friend class TOpticsDir;
    friend class TOpticsPos;

public:
    THaOptics(const char* name, const char* desc, THaSpectrometer *pspec, TString strBeamDetectorName="");
    virtual ~THaOptics();
    
public:
    //apply optics : do the job
    virtual Int_t ApplyOptics(THaTrack *track);
    virtual Int_t ApplyOptics(THaTrack *track, TVector3 beampos, TVector3 beamdir);
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
    virtual void    Clear( Option_t* opt="" ){};
    

public:
    //service functions:
    //virtual void ExtractTRCSInfo(
    //    THaTrack *track,      //input
    //    Double_t &trackX,    //output:    Track X             TRCS
    //    Double_t &trackY,    //output:    Track Y             TRCS
    //    Double_t &trackTheta,//output:    Track Theta         TRCS
    //    Double_t &trackPhi   //output:    Track Phi           TRCS
    //);

    TVector2 CrossingLineLine2D(TVector2 o1,TVector2 d1,TVector2 o2,TVector2 d2);
    TVector3 CrossingSurfLine3D(TVector3 OSurf,TVector3 NSurf,TVector3 OLine,TVector3 DLine);

public:
    //settings and gettings
    Double_t GetPcentral() const;
    Double_t GetCollDist() const;
    inline Double_t GetCurrent() const {return fCurrent;}
    Bool_t   SetBeamName(const char*);
    //TRotation       GetDetectorToTarg() { return fDetToTarg; }
    //TVector3        GetDetectorOffset() { return fDetOffset; }   

protected:
    virtual Int_t   ReadDatabase( const TDatime& date );
    virtual void    MakePrefix();
    virtual THaAnalysisObject* FindModuleNoCheck
        ( const char* name,const char* classname );
    virtual Int_t   ReadRunDatabase( const TDatime& date );
        
protected:
    TRef            fSpec; // pointer to BigBite
    TString         fDefBeamName;
    TRef            fBeam;
    //Double_t        *fMatrix;

protected:
    //coordinate transformations
    TRotation       fHCS2TCS;
    TRotation       fTCS2TRCS;

    TRotation       fTCS2HCS;
    TRotation       fTRCS2TCS;

    TVector3        fTCSOffSetInHCS;
    //TVector3        fDCSOffSetInTCS;
    TVector3        fTRCSOffSetInTCS;

    //Double_t        fPcentral;              //Central momentum (GeV)
    Double_t        fCurrent;               //Central momentum (GeV)
    //Double_t        fCollDist;              //Distance from collimator to target center (m)

    protected:
	Double_t     fThShift,fPhShift,fPShift,fPathLenShift;
	//0st order corection
    public:
	virtual Int_t  ZeroOrderCorrection
	  (Double_t & Th,Double_t & Ph,Double_t & P,Double_t & PathLen){
	  //shift reconstructed variables by 0st order corection

	  Th+=fThShift;
	  Ph+=fPhShift;
	  P+=fPShift;
	  PathLen+=fPathLenShift;

		return 0;
	}
	virtual Int_t ApplyCorrection
	  (Double_t & Th,Double_t & Ph,Double_t & P,Double_t & PathLen){
	  //shift reconstructed variables by selected orders of corections
		return ZeroOrderCorrection( Th, Ph, P, PathLen);
	}

private:
    ClassDef(THaOptics,1);//basic class for optics calculation

public://child classes for vector with coordinate transformation capability
    //_____________________________________________________________________________
    // a vector class with information of it's Coordinate System
    // also handles Coordinate transformation. But it only handle 
    // rotation of the vector. It's useful to describe a direction
    // For example, p vector
    class TOpticsDir:public TVector3
    {
    public:
        enum ECoordSys {
            kNA,    //not used
            kHCS,   //Hall Coordinate System (HCS)
            kTCS,   //Target Coordinate System (TCS)
            //kDCS, //Detector Coordinate System (DCS)
            kTRCS,  //Transport Coordinate System (TRCS) at the focal plane
            kFCS    //Focal plane Coordinate System (FCS)
        };  //describe which Coordinate System this vector is using
        //definition of all coordinate see HallA tech note 02-012

    public:
        TOpticsDir(const THaOptics::TOpticsDir& v)
            :TVector3(v.X(),v.Y(),v.Z()),fCoordSys(v.fCoordSys),fOptics(v.fOptics){}
        TOpticsDir(ECoordSys coordsys,THaOptics * optics,
            const TVector3& v)
            :TVector3(v),fCoordSys(coordsys),fOptics(optics){}
        TOpticsDir(ECoordSys coordsys,THaOptics * optics, 
            Double_t x = 0.0, Double_t y = 0.0, Double_t z = 0.0)
            :TVector3(x,y,z),fCoordSys(coordsys),fOptics(optics){}
        TOpticsDir(THaOptics::TOpticsDir& v)
            :TVector3(v.X(),v.Y(),v.Z()),fCoordSys(v.fCoordSys),fOptics(v.fOptics){}
        virtual ~TOpticsDir() {}

    public:
        //gettings 
        inline  TVector3 GetVector() {TVector3 v(X(),Y(),Z()); return v;}
        inline  ECoordSys GetCoordSys() const {return fCoordSys;}
        inline  THaOptics* GetOptics() const {return (THaOptics *)(fOptics.GetObject());}
        //settings
        virtual inline  THaOptics::TOpticsDir & SetAll(
            ECoordSys coordsys,
            THaOptics * optics, 
            Double_t x = 0.0, Double_t y = 0.0, Double_t z = 0.0
            )
        {SetXYZ(x,y,z);fCoordSys=coordsys;fOptics=optics;return *this;}
        //reset everything to new value

    public:
        //do transformation
        virtual THaOptics::TOpticsDir & 
            TransCoordSys(ECoordSys coordsys,bool dorecurent=true);

    protected:
        virtual void TRCS2FCS();//reload this function to perform TRCS2FCS trans
        virtual void FCS2TRCS();//reload this function to perform FCS2TRCS trans

    public:
        THaOptics::TOpticsDir & operator += (const THaOptics::TOpticsDir &);

        THaOptics::TOpticsDir & operator -= (const THaOptics::TOpticsDir &);

        inline THaOptics::TOpticsDir & operator = (const THaOptics::TOpticsDir &v) 
        {fCoordSys=v.GetCoordSys();fOptics=v.GetOptics();SetXYZ(v.X(),v.Y(),v.Z()); return *this;}
        inline THaOptics::TOpticsDir & operator = (const TVector3 &v) 
        {SetXYZ(v.X(),v.Y(),v.Z()); return *this;}

        inline THaOptics::TOpticsDir & operator += (const TVector3 &p)
        {SetXYZ(X()+p.X(),Y()+p.Y(),Z()+p.Z());return *this;}

        inline THaOptics::TOpticsDir & operator -= (const TVector3 &p)
        {SetXYZ(X()-p.X(),Y()-p.Y(),Z()-p.Z());return *this;}

    public:
        virtual void Print( Option_t* opt ="") const;

    protected:
        ECoordSys    fCoordSys;  //specify which Coordinate System this vector is using
        TRef         fOptics;    //the optics class using this vector ,THaOptics

    private:
        ClassDef(THaOptics::TOpticsDir,0);//direction vector with coordinate info attached   
    };
    //
    //_____________________________________________________________________________
    // a vector class derived from THaOptics::TOpticsDir, specially designed 
    // for position coordinate, ex vertex.
    //
    class TOpticsPos:public THaOptics::TOpticsDir
    {
    public:
        TOpticsPos(ECoordSys coordsys,THaOptics * optics,
            const TVector3& v) 
            :THaOptics::TOpticsDir(coordsys,optics,v){}
        TOpticsPos(ECoordSys coordsys,THaOptics * optics, 
            Double_t x = 0.0, Double_t y = 0.0, Double_t z = 0.0)
            :THaOptics::TOpticsDir(coordsys,optics,x,y,z){}
        TOpticsPos(const THaOptics::TOpticsPos& v) 
            :THaOptics::TOpticsDir(v.fCoordSys,v.GetOptics(),v.X(),v.Y(),v.Z()){}
        TOpticsPos(const THaOptics::TOpticsDir& v) 
            :THaOptics::TOpticsDir(v.GetCoordSys(),v.GetOptics(),v.X(),v.Y(),v.Z()){}
        virtual ~TOpticsPos() {}

    public:
        virtual THaOptics::TOpticsPos & 
            TransCoordSys(ECoordSys coordsys,bool dorecurent=true);

    public:
        inline THaOptics::TOpticsPos & operator = (const THaOptics::TOpticsPos &v) 
        {
            fCoordSys=v.GetCoordSys();fOptics=v.GetOptics();
            SetXYZ(v.X(),v.Y(),v.Z()); return *this;
        }
        // Assignment.
        inline THaOptics::TOpticsPos & operator = (const THaOptics::TOpticsDir &v) 
        {
            fCoordSys=v.GetCoordSys();fOptics=v.GetOptics();
            SetXYZ(v.X(),v.Y(),v.Z()); return *this;
        }
        // Assignment.

        inline THaOptics::TOpticsPos & operator += (const TVector3 &p)
        {SetXYZ(X()+p.X(),Y()+p.Y(),Z()+p.Z());return *this;}

        inline THaOptics::TOpticsPos & operator -= (const TVector3 &p)
        {SetXYZ(X()-p.X(),Y()-p.Y(),Z()-p.Z());return *this;}

    private:
        ClassDef(THaOptics::TOpticsPos,0);//position vector with coordinate info attached
    };
    //_____________________________________________________________________________

};

THaOptics::TOpticsDir operator + (const THaOptics::TOpticsDir &, const THaOptics::TOpticsDir &);
// Addition of TOpticsDir with fool-proof check on Coordinate System consistency.

THaOptics::TOpticsDir operator - (const THaOptics::TOpticsDir &, const THaOptics::TOpticsDir &);
// Subtraction of TOpticsDir with fool-proof check on Coordinate System consistency.

THaOptics::TOpticsDir operator + (const THaOptics::TOpticsDir &, const TVector3 &);
// Addition of TOpticsDir

THaOptics::TOpticsDir operator - (const THaOptics::TOpticsDir &, const TVector3 &);
// Subtraction of TOpticsDir

Double_t operator * (const THaOptics::TOpticsDir &, const THaOptics::TOpticsDir &);
// Scalar product of TOpticsDir with fool-proof check on Coordinate System consistency.

Double_t operator * (const THaOptics::TOpticsDir &, const TVector3 &);
// Scalar product of TOpticsDir.

THaOptics::TOpticsDir operator * (const THaOptics::TOpticsDir &, Double_t a);
THaOptics::TOpticsDir operator * (Double_t a, const THaOptics::TOpticsDir &);
// Scaling of 3-vectors with a real number


#endif

