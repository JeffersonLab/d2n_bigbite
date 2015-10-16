//////////////////////////////////////////////////////////////////////////
//                                                                           
// THaTriggerPlane                                                           
//                                                                           
// A trigger plane for BigBite, consistinf of a eplane and a deplane.        
//                                                                           
//////////////////////////////////////////////////////////////////////////
//	
// Author: Jin Huang <mailto:jinhuang@jlab.org>     July 2007	
//				base on analyzer for SRC experiment
//	Modify History:
//		Mar 2008   Jin Huang <mailto:jinhuang@jlab.org>
//          read in geometry of trigger plane
//		Mar 2008   Jin Huang <mailto:jinhuang@jlab.org>
//          match hits with tracks
//
//////////////////////////////////////////////////////////////////////////

#include <cmath>

#include "THaEvData.h"
#include "TClonesArray.h"
#include "TDatime.h"

#include "THaScintPlane.h"
#include "VarType.h"
#include "VarDef.h"
#include "TMath.h"
#include "THaScintHit.h"
#include "THaScintBar.h"
#include "THaScintPMT.h"
#include "THaTriggerPlaneHit.h"
#include "THaTrackProj.h"
#include "THaTrack.h"

//put this header file below all other headers
#include "THaTriggerPlane.h"

using namespace std;


ClassImp(THaTriggerPlane)

//_____________________________________________________________________________
THaTriggerPlane::THaTriggerPlane( const char* name, const char* description, THaApparatus* a) :
THaNonTrackingDetector(name,description,a), 
fEplane(NULL), fdEplane(NULL),
fHitAcceptanceDx(1000),// just a large value to include all
fHitAcceptanceDy(1000),
fHitAcceptanceDz(1000)
{
    // Constructor. With this method, the subdetectors are created using
    // this detector's prefix followed by "e" and "de", respectively,
    // and variable names like "BB.tp.e.nhits".

    fHitAcceptanceMinDt=-1000;
    fHitAcceptanceMaxDt=1000;
    fMatchRatiodE=fMatchRatioE=fMatchRatioTrack=0;

    DEBUG_LEVEL_RELATED_PERFORMACE_CHECKER;

    DEBUG_HALL_A_ANALYZER_DEBUGER_INIT;

    // Setup the e and de planes
    Setup( name, "e", "de", description, false );

    fHits   = new TClonesArray("THaTriggerPlaneHit", 200);
    fPartialHits   = new TClonesArray("THaTriggerPlaneHit", 200);
    fTrackProj = new TClonesArray( "THaTrackProj", 5 );
}

//_____________________________________________________________________________
void THaTriggerPlane::Setup( const char* name,
                            const char* eplane_name,
                            const char* deplane_name,
                            const char* description,
                            bool subnames )
{
    // Set up the total shower counter. Called by constructor.

    static const char* const here = "Setup()";
    static const char* const message = 
        "Must construct %s detector with valid name! Object construction failed.";

    // Base class constructor failed?
    if( IsZombie()) return;

    size_t e, de;
    if( !eplane_name || (e = strlen(eplane_name)) == 0 ) {
        Error( Here(here), message, "Eplane" );
        MakeZombie();
        return;
    }
    if( !deplane_name || (de = strlen(deplane_name)) == 0 ) {
        Error( Here(here), message, "dEplane" );
        MakeZombie();
        return;
    }

    size_t nlen = strlen(name);
    size_t slen = TMath::Max(e,de);
    size_t len = slen;
    if( subnames )
        len += nlen+1;
    char* subname = new char[ len+1 ];
    const char* sname;
    if( subnames ) {
        strcpy( subname, name );
        strcat( subname, "." );
        strcat( subname, eplane_name );
        sname = subname;
    } else 
        sname = eplane_name;

    char* desc = new char[ 50+strlen(description) ];
    if( description && strlen(description)>0 )
        strcpy( desc, description );
    else {
        strcpy( desc, "BigBite Trigger Plane" );
        SetTitle( desc );
    }
    size_t dlen = strlen(desc);
    strcat( desc, " E-plane subdetector" );

#if DEBUG_LEVEL>=3//start show info
    Info(Here(here),"\tConstruct subdetector Name=%s, Desc=%s", sname,desc);
#endif//#if DEBUG_LEVEL>=3

    fEplane = new THaScintPlane( sname, desc , this);
    if( !fEplane || fEplane->IsZombie() ) {
        Error( Here(here), "Fail to initiate %s", "Eplane" );
        MakeZombie();
        goto exit;
    }

    if( subnames )
        strcpy( subname+nlen+1, deplane_name );
    else
        sname = deplane_name;
    strcpy( desc+dlen, " dE-plane subdetector" );


#if DEBUG_LEVEL>=3//start show info
    Info(Here(here),"\tConstruct subdetector Name=%s, Desc=%s", sname,desc);
#endif//#if DEBUG_LEVEL>=3

    fdEplane = new THaScintPlane( sname, desc  , this);
    if( !fdEplane && fdEplane->IsZombie() ) {
        Error( Here(here), "Fail to initiate %s", "dEplane" );
        MakeZombie();
        goto exit;
    }

exit:
    delete [] subname;
    delete [] desc;
    return;
}

//_____________________________________________________________________________
THaTriggerPlane::~THaTriggerPlane()
{
    // Destructor. Remove variables from global list.
    assert(fHits&&fPartialHits&&fEplane&&fdEplane&&fTrackProj);

    delete fHits;
    delete fPartialHits;
    if (fTrackProj) {
        fTrackProj->Clear();
        delete fTrackProj; fTrackProj = 0;
    }

    delete fEplane;
    delete fdEplane;
    if( fIsSetup )
        RemoveVariables();
}

//_____________________________________________________________________________
THaDetectorBase::EStatus THaTriggerPlane::Init( const TDatime& run_time )
{
    // Set up trigger plane counter. This function 
    // reads the trigger plane parameters from local database files 
    // (e.g. "db_tp.e.dat"), then calls Init() for
    // the shower and preshower subdetectors, respectively.

    if( IsZombie() || !fEplane || !fdEplane )
        return fStatus = kInitError;

    MakePrefix();

    EStatus status;
    if( (status = THaNonTrackingDetector::Init( run_time )) ||
        (status = fEplane->Init( run_time )) ||
        (status = fdEplane->Init( run_time )) )
        return fStatus = status;

    return fStatus;
}
//_____________________________________________________________________________
Int_t THaTriggerPlane::DefineVariables( EMode mode )
{
    // Initialize global variables and lookup table for decoder

    if( mode == kDefine && fIsSetup ) return kOK;
    fIsSetup = ( mode == kDefine );

    THaNonTrackingDetector::DefineVariables(mode);

    // Register variables in global list
    RVarDef vars[] = {
        // complete reconstructed hits
        { "nhit",  "Number of reconstructed hits on trigger plane",  "GetNHits()"},
        { "hit_xpos",           "X position of hit on dEplane",      "fHits.THaTriggerPlaneHit.GetHitXPos()"},
        { "hit_ypos",           "Y position of hit on dEplane",      "fHits.THaTriggerPlaneHit.GetHitYPos()"},
        { "hit_tof",            "TOF of hit on dEplane",             "fHits.THaTriggerPlaneHit.GetHitTOF()"},
        { "hit_Edep",           "Energy dep in Eplane",              "fHits.THaTriggerPlaneHit.GetHitEdep()"},
        { "hit_dEdep",          "Energy dep in dEplane",             "fHits.THaTriggerPlaneHit.GetHitdEdep()"},
        { "hit_Theta",          "polar angle of track",              "fHits.THaTriggerPlaneHit.Theta()"},
        { "hit_Phi",            "azimuth angle of track",            "fHits.THaTriggerPlaneHit.Phi()"},

#       if EXPORT_PARTIAL_HIT
        //partially reconstructed hits
        { "nparthit","Number of partially reconstructed hits on trigger plane", "GetNPartialHits()"},
        { "parthit_whichplane",     "1=dE,2=E,3=Both;other=Error",              "fPartialHits.THaTriggerPlaneHit.Status()"},
        { "parthit_xpos",           "X position of hit",                        "fPartialHits.THaTriggerPlaneHit.GetHitXPos()"},
        { "parthit_ypos",           "Y position of hit",                        "fPartialHits.THaTriggerPlaneHit.GetHitYPos()"},
        { "parthit_tof",            "TOF of hit on dEplane",                    "fPartialHits.THaTriggerPlaneHit.GetHitTOF()"},
        { "parthit_Edep",           "Energy dep in hit",                        "fPartialHits.THaTriggerPlaneHit.GetHitTotalEdep()"},
#       endif//#if PROCESS_PARTIAL_HIT
        //
        { "trx",    "x-position of track in trigger plane",  "fTrackProj.THaTrackProj.fX" },
        { "try",    "y-position of track in trigger plane",  "fTrackProj.THaTrackProj.fY" },
        { "trpath", "TRCS pathlen of track to trigger plane","fTrackProj.THaTrackProj.fPathl" },
        { "trdx",   "track deviation in x-position (m)", "fTrackProj.THaTrackProj.fdX" },
        { "trHitIndex",  "the index of THaTriggerPlaneHit associated with track, -1 means no match",  "fTrackProj.THaTrackProj.fChannel" },

        // match ratios statistic
        //# of matched THaTriggerPlaneHit / # of total THaScintHit on THaScintPlane
        {"MatchRatioE", "matched Hits / total Hits on E plane",     "GetMatchRatioE()"},
        {"MatchRatiodE","matched Hits / total Hits on dE plane",    "GetMatchRatiodE()"},
        {"MatchRatioTrack","matched Hits / num of tracks",    "GetMatchRatioTrack()"},


        //end
        {0}
    };

    Int_t value = DefineVarsFromList( vars, mode );

    return value;
}
//_____________________________________________________________________________
Int_t THaTriggerPlane::ReadDatabase( const TDatime& date )
{
    static const char*  here = "ReadDatabase()";
    static const Int_t LEN=300;
    char buff[LEN];
    char cbuff[LEN];	//commment buffer

    FILE* file = OpenFile( date );
    if( !file ) 
    {
#if DEBUG_LEVEL>=1//start show warning 
        Warning(Here(here),"\tTHaTriggerPlane(%s) ReadDatabase file open failed.",GetName());
#endif//#if DEBUG_LEVEL>=1
        return kFileError;
    }

#if DEBUG_LEVEL>=3//start show info
    Info(Here(here),"\tStart reading Database");
#endif//#if DEBUG_LEVEL>=3

    // Build the search tag and find it in the file. Search tags
    // are of form [ <prefix> ], e.g. [ N.bar.n1 ].
    TString line;

    TString prefix=fPrefix;
    prefix.Chop();  // remove trailing dot of prefix

    ///////////////////////// geometry ///////////////////////////
    TString tag = Form("[%s.%s]",prefix.Data(),  "geometry"  );

#if DEBUG_LEVEL>=3//start show info
    Info(Here(here),"\tTrying to read in database section %s", tag.Data());
#endif

    tag.ToLower();
    bool found = false;
    while (!found && fgets (buff, LEN, file) != NULL) {
        char* buf = ::Compress(buff);  //strip blanks
        line = buf;
        delete [] buf;
#if DEBUG_LEVEL>=5//start show info
        Info(Here(here),"\tTrying to search line <%s>", line.Data());
#endif
        if( line.EndsWith("\n") ) line.Chop();  //delete trailing newline
#if DEBUG_LEVEL>=5//start show info
        Info(Here(here),"\tafter line.Chop() <%s>", line.Data());
#endif
        line.ToLower();
        if ( tag == line ) 
            found = true;
    }
    if( !found ) {
        Error(Here(here), "Database entry \"%s\" not found!", tag.Data() );
        fclose(file);
        return kInitError;
    }

    //jump throug comment lines
    while ( ReadNumberSignStartComment( file, cbuff, LEN ));

    Double_t xtmp,ytmp,ztmp;
    fgets( buff, LEN, file );
    if( sscanf( buff, "%lg%lg%lg",&xtmp,&ytmp,&ztmp) != 3 ) {
        if( *buff ) buff[strlen(buff)-1] = 0; //delete trailing newline
        Error( Here(here), "Error 4 reading %s (fOrigin): %s", tag.Data(), buff );
        fclose(file);
        return kInitError;
    }
    fOrigin.SetXYZ(xtmp,ytmp,ztmp);

    //jump throug comment lines
    while ( ReadNumberSignStartComment( file, cbuff, LEN ));

    fgets( buff, LEN, file );
    if( sscanf( buff, "%lg%lg%lg",&xtmp,&ytmp,&ztmp) != 3 ) {
        if( *buff ) buff[strlen(buff)-1] = 0; //delete trailing newline
        Error( Here(here), "Error 4 reading %s (fXax): %s", tag.Data(), buff );
        fclose(file);
        return kInitError;
    }
    fXax.SetXYZ( xtmp,ytmp,ztmp );

    //jump throug comment lines
    while ( ReadNumberSignStartComment( file, cbuff, LEN ));

    fgets( buff, LEN, file );
    if( sscanf( buff, "%lg%lg%lg",&xtmp,&ytmp,&ztmp) != 3 ) {
        if( *buff ) buff[strlen(buff)-1] = 0; //delete trailing newline
        Error( Here(here), "Error 4 reading %s (fZax): %s", tag.Data(), buff );
        fclose(file);
        return kInitError;
    }
    fYax.SetXYZ( xtmp,ytmp,ztmp );
    fZax = fXax.Cross(fYax);


    ///////////////////////// hit_acceptance ///////////////////////////

    tag = Form("[%s.%s]",prefix.Data(),  "hit_acceptance"  );

#if DEBUG_LEVEL>=3//start show info
    Info(Here(here),"\tTrying to read in database section %s", tag.Data());
#endif

    tag.ToLower();
    found = false;
    while (!found && fgets (buff, LEN, file) != NULL) {
        char* buf = ::Compress(buff);  //strip blanks
        line = buf;
        delete [] buf;
#if DEBUG_LEVEL>=5//start show info
        Info(Here(here),"\tTrying to search line <%s>", line.Data());
#endif
        if( line.EndsWith("\n") ) line.Chop();  //delete trailing newline
#if DEBUG_LEVEL>=5//start show info
        Info(Here(here),"\tafter line.Chop() <%s>", line.Data());
#endif
        line.ToLower();
        if ( tag == line ) 
            found = true;
    }
    if( !found ) {
        Error(Here(here), "Database entry \"%s\" not found!", tag.Data() );
        fclose(file);
        return kInitError;
    }

    //jump throug comment lines
    while ( ReadNumberSignStartComment( file, cbuff, LEN ));	
    //
    //# when x y z distance between 2 hits in E and dE plane is smaller
    //# than the number below, they will be combined as hit in Triggerplane
    fgets( buff, LEN, file );
    if( sscanf( buff, "%lg%lg%lg",&fHitAcceptanceDx,&fHitAcceptanceDy,&fHitAcceptanceDz) != 3 ) {
        if( *buff ) buff[strlen(buff)-1] = 0; //delete trailing newline
        Error( Here(here), "Error 4 reading %s : %s", tag.Data(), buff );
        fclose(file);
        return kInitError;
    }

    //jump throug comment lines
    while ( ReadNumberSignStartComment( file, cbuff, LEN ));	

    //#accepatble time difference between 2 hits in E and dE plane is smaller
    //#time difference=TOF(dE)-TOF(E)
    fgets( buff, LEN, file );
    if( sscanf( buff, "%lg%lg",&fHitAcceptanceMinDt,&fHitAcceptanceMaxDt) != 2 ) {
        if( *buff ) buff[strlen(buff)-1] = 0; //delete trailing newline
        Error( Here(here), "Error 4 reading %s : %s", tag.Data(), buff );
        fclose(file);
        return kInitError;
    }

    //jump throug comment lines
    while ( ReadNumberSignStartComment( file, cbuff, LEN ));	

    //#accepatble time difference between 2 hits in E and dE plane is smaller
    //#time difference=TOF(dE)-TOF(E)
    fgets( buff, LEN, file );
    if( sscanf( buff, "%lg%lg",&fTrackAcceptanceDx,&fTrackAcceptanceDy) != 2 ) {
        if( *buff ) buff[strlen(buff)-1] = 0; //delete trailing newline
        Error( Here(here), "Error 4 reading %s : %s", tag.Data(), buff );
        fclose(file);
        return kInitError;
    }

#if DEBUG_LEVEL>=3//start show info
    Info(Here(here),"Printing Origin and x,y,z axis vector:");
    fOrigin.Print();
    fXax.Print();
    fYax.Print();
    fZax.Print();

    TString sDebugOutput;
    sDebugOutput=GetName();
    sDebugOutput+=" Database read in successfully with:";
    sDebugOutput+="\n \thit_acceptance_dx\t= ";	sDebugOutput+=fHitAcceptanceDx;
    sDebugOutput+="\n \thit_acceptance_dy\t= ";	sDebugOutput+=fHitAcceptanceDy;
    sDebugOutput+="\n \thit_acceptance_dz\t= ";	sDebugOutput+=fHitAcceptanceDz;
    sDebugOutput+="\n \tHitAcceptanceMinDt\t= ";sDebugOutput+=fHitAcceptanceMinDt;
    sDebugOutput+="\n \tHitAcceptanceMaxDt\t= ";sDebugOutput+=fHitAcceptanceMaxDt;
    sDebugOutput+="\n \tTrackAcceptanceDx\t= ";	sDebugOutput+=fTrackAcceptanceDx;
    sDebugOutput+="\n \tTrackAcceptanceDy\t= ";	sDebugOutput+=fTrackAcceptanceDy;
    sDebugOutput+="\n";
    Info(Here(here),sDebugOutput.Data());
#endif	

    fclose(file);
    return kOK;
}
//_____________________________________________________________________________
Int_t THaTriggerPlane::Decode( const THaEvData& evdata )
{
    // Decode triggerplane detector. Calls Decode() of fdEplane and fEplane.
    // Return the return value of fEplane->Decode().
    if( !IsOK() ) 
        return -1;

    fHits->Clear("C");
    fPartialHits->Clear("C");
    fTrackProj->Clear("C");

    return fEplane->Decode( evdata )+fdEplane->Decode( evdata );
}

//_____________________________________________________________________________
void THaTriggerPlane::SetApparatus( THaApparatus* app )
{
    // Set the apparatus of this detector as well as the subdetectors

    THaNonTrackingDetector::SetApparatus( app );
    //fEplane->SetApparatus( app );
    //fdEplane->SetApparatus( app );
    return;
}

//_____________________________________________________________________________
Int_t THaTriggerPlane::FineProcess( TClonesArray& tracks )
{
    // Scintillator fine processing.
    // Not implemented. Does nothing, returns 0.
    fEplane->FineProcess(tracks);
    fdEplane->FineProcess(tracks);

    //match hit
    Int_t state=FineMatchingHits(tracks);
    assert(state==kOK);
    state++;//trick to avoid warning

    return kOK;// THaNonTrackingDetector::FineProcess(tracks);
}

//_____________________________________________________________________________
Int_t THaTriggerPlane::CoarseProcess( TClonesArray& tracks )
{
    // Scintillator fine processing.
    fEplane->CoarseProcess(tracks);
    fdEplane->CoarseProcess(tracks);

    //match hit
    Int_t state=CoarseMatchingHits();
    assert(state==kOK);
    state++;//trick to avoid warning

    return kOK;//THaNonTrackingDetector::CoarseProcess(tracks);
}

//_____________________________________________________________________________
char* THaTriggerPlane::ReadNumberSignStartComment( FILE* fp, char *buf, const int len )
{
    // Read blank and comment lines 
    // return NULL if is not comment line
    int ch = fgetc(fp);  // peak ahead one character
    ungetc(ch,fp);

    if(ch=='#' || ch=='\n')
        return fgets(buf,len,fp); // read the comment
    else
        return NULL;
}

//_____________________________________________________________________________
Int_t THaTriggerPlane::CoarseMatchingHits()
{
    //match hits of de and E plane and save them to fHits
    //pick up a hit in E plane and match it to that in dE in near by bars

    DEBUG_LINE_INFO(static const char* here="CoarseProcess::MatchingHits");

    Int_t nde,ne,tmpidx=0,HitNum=0,dbar;
    THaScintHit *pehit,*pdehit;
    Double_t mindist;
    Double_t dx,dy,dz,dt;

#   if DEBUG_LEVEL>=3//start show info
    Info(Here(here),
        "\tStarting Matching for %d hit(s) on E plane and %d hit(s) on dE plane" 
        ,fEplane->GetNHits(),fdEplane->GetNHits()
        );
#   endif//#if DEBUG_LEVEL>=3

    //at least one of them have no hit
    if (fEplane->GetNHits()+fdEplane->GetNHits()<1) {return kOK;}

#   if PROCESS_PARTIAL_HIT
    //array used to statistic whether a hit is already been matched
    Bool_t *ehitsprocessed, *dehitsprocessed;

    ehitsprocessed=new Bool_t[fEplane->GetNHits()];
    dehitsprocessed=new Bool_t[fdEplane->GetNHits()];

    assert(ehitsprocessed);
    assert(dehitsprocessed);

    for (int i=0;i<fEplane->GetNHits();i++)
        ehitsprocessed[i]=kFALSE;
    for (int i=0;i<fdEplane->GetNHits();i++)
        dehitsprocessed[i]=kFALSE;

#   endif//#if PROCESS_PARTIAL_HIT

#   if MATCHING_HITS_E_PLANE_PRIORITY
    //since hits on both plane is rare, just bare search.
    for (ne=0;ne<fEplane->GetNHits();ne++)
    {
        pehit=fEplane->GetHit(ne);
        assert(pehit);

        pdehit=0;
        mindist=1e10;//just large enough
        for (nde=0;nde<fdEplane->GetNHits();nde++)
        {
            assert(fdEplane->GetHit(nde));

            //check for close bars
            dbar=fdEplane->GetHit(nde)->GetBarNum() 
                - pehit->GetBarNum();
            if (dbar<0&&dbar>1)
                continue;
            else
            {
                if (// check x y z limit
                    abs((double)(
                    dx=fdEplane->GetHit(nde)->GetHitXPos()-pehit->GetHitXPos()
                    ))>fHitAcceptanceDx||abs((double)(
                    dy=fdEplane->GetHit(nde)->GetHitYPos()-pehit->GetHitYPos()
                    ))>fHitAcceptanceDy||abs((double)(
                    dz=fdEplane->GetHit(nde)->GetHitZPos()-pehit->GetHitZPos()
                    ))>fHitAcceptanceDz
                    )
                    continue;
                else
                {
                    dt=fdEplane->GetHit(nde)->GetHitTOF()-pehit->GetHitTOF();
                    if (dt<fHitAcceptanceMinDt||dt>fHitAcceptanceMaxDt)//check time diff
                        continue;
                    else
                    {
                        //just record the nearest match, 
                        mindist=TMath::Sqrt(dx*dx+dy*dy+dz*dz);
                        pdehit=fdEplane->GetHit(nde);
                        tmpidx=nde;
                    }
                }//if (// check x y z limit
            }//else [if (abs(fdEplane->GetHit(nde)->GetBarNum()-pehit->GetBarNum())>1  )]
        }//for (nde=0;nde<fdEplane->GetNHits();nde++)

        nde=tmpidx;//keep track of index of matched hits

#   else//#if MATCHING_HITS_E_PLANE_PRIORITY
    //dE PLANE PRIORITY
    //since hits on both plane is rare, just bare search.
    for (nde=0;nde<fdEplane->GetNHits();nde++)
    {
        pdehit=fdEplane->GetHit(nde);
        assert(pdehit);

        pehit=0;
        mindist=1e10;//just large enough
        for (ne=0;ne<fEplane->GetNHits();ne++)
        {
            assert(fEplane->GetHit(ne));

#           if DEBUG_LEVEL>=5//start show info
            Info(Here(here),"\tCompare this pair:\n\tItem\tE hit\t\tdE hit\n\tBar\t%d\t\t%d\n\tXPos\t%f\t\t%f\n\tYPos\t%f\t\t%f\n\tTOF\t%f\t\t%f\n\tEdep\t%f\t\t%f\n",
                fEplane->GetHit(ne)->GetBarNum(),pdehit->GetBarNum(),
                fEplane->GetHit(ne)->GetHitXPos(),pdehit->GetHitXPos(),
                fEplane->GetHit(ne)->GetHitYPos(),pdehit->GetHitYPos(),
                fEplane->GetHit(ne)->GetHitTOF(),pdehit->GetHitTOF(),
                fEplane->GetHit(ne)->GetHitEdep(),pdehit->GetHitEdep()
                );
#           endif
            //check for close bars
            dbar=pdehit->GetBarNum() 
                -fEplane->GetHit(ne)->GetBarNum();
            if ( dbar<0&&dbar>1 )
                continue;
            else
            {

#               if DEBUG_LEVEL>=5//step by step check (massive info)
                Info(Here(here),"\t...Pass check for close bars" );
#               endif//#if DEBUG_LEVEL>=5

                if (// check x y z limit
                    abs(
                    dx=fEplane->GetHit(ne)->GetHitXPos()-pdehit->GetHitXPos()
                    )>fHitAcceptanceDx||abs(
                    dy=fEplane->GetHit(ne)->GetHitYPos()-pdehit->GetHitYPos()
                    )>fHitAcceptanceDy||abs(
                    dz=fEplane->GetHit(ne)->GetHitZPos()-pdehit->GetHitZPos()
                    )>fHitAcceptanceDz
                    )
                    continue;
                else
                {

#                   if DEBUG_LEVEL>=5//step by step check (massive info)
                    Info(Here(here),"\t...Pass check x y z limit" );
#                   endif//#if DEBUG_LEVEL>=5

                    dt=pdehit->GetHitTOF()-fEplane->GetHit(ne)->GetHitTOF();
                    if (dt<fHitAcceptanceMinDt||dt>fHitAcceptanceMaxDt)//check time diff
                        continue;
                    else
                    {

#                       if DEBUG_LEVEL>=5//step by step check (massive info)
                        Info(Here(here),"\t...Pass check time diff, record" );
#                       endif//#if DEBUG_LEVEL>=5

                        //just record the nearest match, 
                        mindist=TMath::Sqrt(dx*dx+dy*dy+dz*dz);
                        pehit=fEplane->GetHit(ne);
                        tmpidx=ne;
                    }
                }//if (// check x y z limit
            }//else [if (abs(fdEplane->GetHit(nde)->GetBarNum()-pehit->GetBarNum())>1  )]
        }//for (nde=0;nde<fdEplane->GetNHits();nde++)

        ne=tmpidx;//keep track of index of matched hits

#       endif//#if MATCHING_HITS_E_PLANE_PRIORITY

        // since resolution of scincilator is not good, just coarse match the closest hits
        // ignore multihit, close hit, which is job for MWDC
        if (pdehit&&pehit)
        {// one match

#           if DEBUG_LEVEL>=3//start show info
            Info(Here(here),"\tfind one match:\n\tItem\tE hit\t\tdE hit\n\tBar\t%d\t\t%d\n\tXPos\t%f\t\t%f\n\tYPos\t%f\t\t%f\n\tTOF\t%f\t\t%f\n\tEdep\t%f\t\t%f\n",
                pehit->GetBarNum(),pdehit->GetBarNum(),
                pehit->GetHitXPos(),pdehit->GetHitXPos(),
                pehit->GetHitYPos(),pdehit->GetHitYPos(),
                pehit->GetHitTOF(),pdehit->GetHitTOF(),
                pehit->GetHitEdep(),pdehit->GetHitEdep()
                );
#           endif//#if DEBUG_LEVEL>=3      

            new( (*fHits)[HitNum++] ) THaTriggerPlaneHit(pehit,pdehit);

#           if PROCESS_PARTIAL_HIT
            //record which hit has been matched
            assert(ne<fEplane->GetNHits());
            assert(nde<fdEplane->GetNHits());

            ehitsprocessed[ne]=kTRUE;
            dehitsprocessed[nde]=kTRUE;
#           endif//#if PROCESS_PARTIAL_HIT

        }
        else//if (pdehit)
        {//no match

#           if DEBUG_LEVEL>=4//start show info
            Info(Here(here),"\tE plane Hit#%d have no match on de plane",ne);
#           endif//#if DEBUG_LEVEL>=4      

        }//else//if (pdehit)

    }//for (ne=0;ne<fEplane->GetNHits();ne++)

#   if PROCESS_PARTIAL_HIT
    Int_t PartHitNum=0;

    //generate fPartialHits list for rest E-Plane hits
    for (int i=0;i<fEplane->GetNHits();i++)
        if (ehitsprocessed[i]==kFALSE)
            new( (*fPartialHits)[PartHitNum++] ) 
            THaTriggerPlaneHit(fEplane->GetHit(i),0);

    //generate fPartialHits list for rest dE-Plane hits
    for (int i=0;i<fdEplane->GetNHits();i++)
        if (dehitsprocessed[i]==kFALSE)
            new( (*fPartialHits)[PartHitNum++] ) 
            THaTriggerPlaneHit(0,fdEplane->GetHit(i));

    delete [] ehitsprocessed;
    delete [] dehitsprocessed;
#   endif//#if PROCESS_PARTIAL_HIT


    //efficiency statistics
    if (fEplane->GetNHits()>0&&fdEplane->GetNHits()>0)
    {
        fMatchRatioE=(Double_t)(GetNHits())/(fEplane->GetNHits());
        fMatchRatiodE=(Double_t)(GetNHits())/(fdEplane->GetNHits());
#       if DEBUG_LEVEL>=3//start show info
        Info(Here(here),
            "\tEnd Matching, get %d hit(s), %3.0f%% for e-plane and %3.0f%% for de-plane" 
            ,GetNHits()
            ,fMatchRatioE*100
            ,fMatchRatiodE*100
            );
#       endif//#if DEBUG_LEVEL>=3
    }
    else 
    {
        fMatchRatioE=fMatchRatiodE=1;
    }

    return kOK;

}

//_____________________________________________________________________________
#if DEBUG_LEVEL>=3
//  switch whether inserting a arbitary track into FineMatchingHits
#   define FINEMATCHINGHITSTEST   0
#endif

Int_t THaTriggerPlane::FineMatchingHits(TClonesArray& tracks)
{
    //match hits of de and E plane with reference to tracks, modify fHits
    //process golden track if possible
    //
    //build a reference table (fTrackRef).
    //
    int n_track = tracks.GetLast()+1;   // Number of reconstructed tracks
    int matchcount=0; 

#   if FINEMATCHINGHITSTEST
    n_track=1;
#   endif

    for ( int i=0; i<n_track; i++ ) {

#       if FINEMATCHINGHITSTEST
        THaTrack* theTrack=new THaTrack(0,0,0,0);
#       else
        THaTrack* theTrack = static_cast<THaTrack*>( tracks[i] );
#       endif

        Double_t pathl=kBig, xc=kBig, yc=kBig, 
            dx=kBig, dx2=kBig, dxtmp=kBig, dxtmp2=kBig;
        Int_t hitidx=-1;

        if ( ! CalcTrackIntercept(theTrack, pathl, xc, yc) ) { // failed to hit
            new ( (*fTrackProj)[i] )
                THaTrackProj(xc,yc,pathl,dx,hitidx,this);
            DEBUG_WARNING(Here("FineMatchingHits"),
                "\t Track # %d could not be projected to trigger plane",i);
            continue;
        }

        DEBUG_INFO(Here("FineMatchingHits")
            ,"\t Get Track Projection Point (x,y)=(%f,%f)."
            ,xc, yc);

        //look for match
        dx = kBig;
        dx2= kBig;
        THaTriggerPlaneHit* phit;
        for ( Int_t j=0; j<GetNHits(); j++ ) {
            phit=GetHit(j);

            assert(phit);
            DEBUG_INFO(Here("FineMatchingHits")
                ,"\t Trying to match with hit #%d at (x,y)=(%f,%f)."
                ,j,phit->GetHitXPos(), phit->GetHitYPos());

            if(TMath::Abs(xc-phit->GetHitXPos())<fTrackAcceptanceDx and
                TMath::Abs(yc-phit->GetHitYPos())<fTrackAcceptanceDy)
            {
                //one candidate
                dxtmp=TMath::Sqrt(
                    (xc-phit->GetHitXPos())*(xc-phit->GetHitXPos())+
                    (yc-phit->GetHitYPos())*(yc-phit->GetHitYPos()));
                if (dx>=dxtmp)
                {//candidate for update
                    dxtmp2=TMath::Sqrt(dxtmp*dxtmp+
                        (xc-phit->GetSecondaryHitXPos())*(xc-phit->GetSecondaryHitXPos())+
                        (yc-phit->GetSecondaryHitYPos())*(yc-phit->GetSecondaryHitYPos()));
                    if (dx2>=dxtmp2)
                    {
                        dx=dxtmp;
                        dx2=dxtmp2;
                        hitidx=j;
                    }
                }//if (dx>dxtmp)
            }
        }

        DEBUG_INFO(Here("FineMatchingHits")
            ,hitidx>=0?"\t Successfully Matched with hit #%d.with minx=%f":"Matching Failed"
            ,hitidx,dx);

        // record information, found or not
        new ( (*fTrackProj)[i] )
            THaTrackProj(xc,yc,pathl,dx,hitidx,this);
        if (hitidx>=0) matchcount++;
    }//for ( int i=0; i<n_track; i++ ) 


    //statistic
    fMatchRatioTrack=n_track==0?1:((Double_t)matchcount/(Double_t)n_track);
    DEBUG_INFO(Here("FineMatchingHits"),
        "\t %f percent of tracks have matched to hits on trigger plane.",
        fMatchRatioTrack*100);

    return kOK;
}

//_____________________________________________________________________________



///////////////////////////////////////////////////////////////////////////////


