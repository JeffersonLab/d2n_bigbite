//////////////////////////////////////////////////////////////////////////
//
//	THaTriggerPlaneHit.cxx
//
//	class used to describe a hit on BigBite trigger plane
//	
//////////////////////////////////////////////////////////////////////////
//	
// Author: Jin Huang <mailto:jinhuang@jlab.org>    Aug 2007
//	Modify History:
//
//////////////////////////////////////////////////////////////////////////

#include "TMath.h"
#include "TVector3.h"
#include "THaAnalysisObject.h"

#include "THaTriggerPlaneHit.h"

//_____________________________________________________________________________
THaTriggerPlaneHit::THaTriggerPlaneHit(const THaScintHit* HitOnE,const THaScintHit* HitOndE)
{

    fEHit=const_cast<THaScintHit*>(HitOnE);
    fdEHit=const_cast<THaScintHit*>(HitOndE);

    if (HitOnE) assert(fEHit.IsValid());
    if (HitOndE) assert(fdEHit.IsValid());

    if (!(HitOnE||HitOndE)) {
        Error(("Constructor"),
            "THaTriggerPlaneHit initiated with both dE hit and E hit = NULL");
        MakeZombie();
    }
}

//_____________________________________________________________________________
THaTriggerPlaneHit::~THaTriggerPlaneHit(void)
{

}

//_____________________________________________________________________________

Double_t THaTriggerPlaneHit::GetHitXPos() const {
    //x position: position of dE hit, if available; otherwise return that of E hit

    if (fdEHit.IsValid())
        return GetdEHit()->GetHitXPos();
    else if (fEHit.IsValid())
        return GetEHit()->GetHitXPos();
    else{
        Error(("GetHitXPos"),
            "no information for either dE hit or E hit, return THaAnalysisObject::kBig");
        return THaAnalysisObject::kBig;
    }
}

//_____________________________________________________________________________
Double_t THaTriggerPlaneHit::GetHitYPos()  const{
    //y position: position of dE hit, if available; otherwise return that of E hit

    if (fdEHit.IsValid())
        return GetdEHit()->GetHitYPos();
    else if (fEHit.IsValid())
        return GetEHit()->GetHitYPos();
    else{
        Error(("GetHitYPos"),
            "no information for either dE hit or E hit, return THaAnalysisObject::kBig");
        return THaAnalysisObject::kBig;
    }
}

//_____________________________________________________________________________
Double_t THaTriggerPlaneHit::GetHitZPos()  const{
    //z position: position of dE hit, if available; otherwise return that of E hit

    if (fdEHit.IsValid())
        return GetdEHit()->GetHitZPos();
    else if (fEHit.IsValid())
        return GetEHit()->GetHitZPos();
    else{
        Error(("GetHitZPos"),
            "no information for either dE hit or E hit, return THaAnalysisObject::kBig");
        return THaAnalysisObject::kBig;
    }
}


//_____________________________________________________________________________

Double_t THaTriggerPlaneHit::GetSecondaryHitXPos() const {
    //x position: position of E hit, if both E&dE hit available; otherwise return that THaAnalysisObject::kBig

    switch(Status())
    {
    case 3:
        return GetEHit()->GetHitXPos();
        break;
    case 0:
        Error(("GetSecondaryHitXPos"),
            "no information for either dE hit or E hit, return THaAnalysisObject::kBig");
        return THaAnalysisObject::kBig;
        break;
    default:
        return THaAnalysisObject::kBig;
        break;    
    }
}

//_____________________________________________________________________________
Double_t THaTriggerPlaneHit::GetSecondaryHitYPos()  const{
    //y position: position of E hit, if both E&dE hit available; otherwise return that THaAnalysisObject::kBig

    switch(Status())
    {
    case 3:
        return GetEHit()->GetHitYPos();
        break;
    case 0:
        Error(("GetSecondaryHitYPos"),
            "no information for either dE hit or E hit, return THaAnalysisObject::kBig");
        return THaAnalysisObject::kBig;
        break;
    default:
        return THaAnalysisObject::kBig;
        break;    
    }
}

//_____________________________________________________________________________
Double_t THaTriggerPlaneHit::GetSecondaryHitZPos()  const{
    //z position: position of E hit, if both E&dE hit available; otherwise return that THaAnalysisObject::kBig

    switch(Status())
    {
    case 3:
        return GetEHit()->GetHitZPos();
        break;
    case 0:
        Error(("GetSecondaryHitZPos"),
            "no information for either dE hit or E hit, return THaAnalysisObject::kBig");
        return THaAnalysisObject::kBig;
        break;
    default:
        return THaAnalysisObject::kBig;
        break;    
    }
}

//_____________________________________________________________________________
Double_t THaTriggerPlaneHit::GetHitTOF()  const{
    //z position: position of dE hit, if available; otherwise return that of E hit

    if (fdEHit.IsValid())
        return GetdEHit()->GetHitTOF();
    else if (fEHit.IsValid())
        return GetEHit()->GetHitTOF();
    else{
        Error(("GetHitTOF"),
            "no information for either dE hit or E hit, return THaAnalysisObject::kBig");
        return THaAnalysisObject::kBig;
    }
}

//_____________________________________________________________________________

TVector3 THaTriggerPlaneHit::GetDirection() const
{   
    //return pos difference between hit on E - that on dE

    TVector3 v;

    if (IsFullHit())
        v.SetXYZ(
        GetEHit()->GetHitXPos() - GetdEHit()->GetHitXPos(),
        GetEHit()->GetHitYPos() - GetdEHit()->GetHitYPos(),
        GetEHit()->GetHitZPos() - GetdEHit()->GetHitZPos()
        );
    else
    {
#if DEBUG_LEVEL>=1//start show warning 
        Warning("GetDirection","\tEither E or dE hit is missing, return 0 vector" );
#endif//#if DEBUG_LEVEL>=1        
    }

    return v;
}


//_____________________________________________________________________________

//_____________________________________________________________________________

//_____________________________________________________________________________

//_____________________________________________________________________________
ClassImp(THaTriggerPlaneHit)



