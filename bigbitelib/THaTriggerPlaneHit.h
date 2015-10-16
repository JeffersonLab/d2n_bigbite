//////////////////////////////////////////////////////////////////////////
//
//	THaTriggerPlaneHit
//
//	class used to describe a hit on BigBite trigger plane
//	
//////////////////////////////////////////////////////////////////////////
//	
// Author: Jin Huang <mailto:jinhuang@jlab.org>    Aug 2007
//	Modify History:
//
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_THaTriggerPlaneHit
#define ROOT_THaTriggerPlaneHit

#include "TObject.h"
#include "TClonesArray.h"
#include "TRef.h"
#include "THaScintHit.h"


//------------------------------------------------------//
//
//	Debug Definitions
//	place this section below any other head files
//
//------------------------------------------------------//
#ifdef DEBUG_LEVEL
#   undef DEBUG_LEVEL
#endif

//	DEBUG_LEVEL;	
//	=0	or not define: no debug, full speed 
//	>=1	enable debug extra warning (suggested setting)
//	>=2	above + enable debug assert
//	>=3	above + enable debug extra info
//	>=4	above + massive info (in a for or while)
#define DEBUG_LEVEL   2

#include    "DebugDef.h"
//------------------------------------------------------//

class TVector3;

class THaTriggerPlaneHit :
    public TObject
{
public:
    //-------------

    THaTriggerPlaneHit(const THaScintHit* HitOnE,const THaScintHit* HitOndE);
    virtual ~THaTriggerPlaneHit(void);

    //-------------

    inline THaScintHit* GetEHit() const 
    { return (THaScintHit*)fEHit.GetObject(); }
    inline THaScintHit* GetdEHit() const 
    { return (THaScintHit*)fdEHit.GetObject(); }

    //-------------
    // tests

    inline Bool_t IsFullHit() const {return (fEHit.IsValid() && fdEHit.IsValid());}
    //test both HitOnE and HitOndE is valid

    inline Bool_t IsPartialHit() const {return (fEHit.IsValid() ^ fdEHit.IsValid());}
    //test only one of HitOnE and HitOndE is missing

    inline Int_t Status() const
    {
        //1=dE get hit only,2=E get hit only,3=Both plane gets hit;other=Error
        Int_t status=0;

        status+=fEHit.IsValid()?2:0;
        status+=fdEHit.IsValid()?1:0;

        return status;
    }

    //-------------
    //positions & time

    Double_t GetHitXPos() const;  
    //x position: position of dE hit, if available; otherwise return that of E hit

    Double_t GetHitYPos() const;  
    //y position: position of dE hit, if available; otherwise return that of E hit

    Double_t GetHitZPos() const;  
    //z position: position of dE hit, if available; otherwise return that of E hit

    Double_t GetHitTOF() const;  
    //time info of dE hit, if available; otherwise return that of E hit

    Double_t GetSecondaryHitXPos() const;  
    //x position: position of E hit, if both E&dE hit available; otherwise return that kBig

    Double_t GetSecondaryHitYPos() const;  
    //y position: position of E hit, if both E&dE hit available; otherwise return that kBig

    Double_t GetSecondaryHitZPos() const;  
    //z position: position of E hit, if both E&dE hit available; otherwise return that kBig

    //-------------
    //preliminary direction of track

    TVector3 GetDirection() const;    
    //return pos difference between hit on E - that on dE

    inline Double_t Theta() const { return GetDirection().Theta(); }
    //polar angle of track

    inline Double_t Phi() const { return GetDirection().Phi(); }
    //azimuth angle of track

    //-------------
    //Energy deposite

    inline Double_t GetHitEdep() const 
    { return fEHit.IsValid()?GetEHit()->GetHitEdep():0; }

    inline Double_t GetHitdEdep() const 
    { return fdEHit.IsValid()?GetdEHit()->GetHitEdep():0; }

    inline Double_t GetHitTotalEdep() const 
    { return GetHitEdep()+GetHitdEdep(); }

    //-------------
protected:
    TRef fEHit;     //ref to hit on E plane
    TRef fdEHit;    //ref to hit on dE plane

public:
    ClassDef(THaTriggerPlaneHit,0) // Describes hits on Trigger Plane

};

#endif



