//////////////////////////////////////////////////////////////////////////
//                                                                           
// THaMWDC                                                                   
//                                                                           
// class for horizontal drift chamber used in BigBite    
// using old AGen tracking code.
// for MWDC with new tracking code, please use TreeSearch::MWDC
//                                                                           
// Units used:                                                               
//        For X, Y, and Z coordinates of track    -  meters                  
//        For Theta and Phi angles of track       -  tan(angle)              
//                                                                           
//////////////////////////////////////////////////////////////////////////
//	
//	Author : Copy from AGen Lib
//	Modify History:
//		 Jin Huang <mailto:jinhuang@jlab.org>    July 2007
//			Making MWDC standing alone
//			open interface for future tracking algorithm
//
//////////////////////////////////////////////////////////////////////////

#ifdef DO_TRACKING_BBE
#include "THaBBe.h"
#include "THaBBTotalShower.h"
#include "THaBBShower.h"
#elif defined DO_TRACKING_BIGBITE
#include "THaBigBite.h"
#endif

#include "THaMWDCPlane.h"
#include "THaMWDCGroup.h"
#include "THaMWDCHit.h"


#include "THaScintillator.h"
#include "THaEvData.h"
#include "THaDetMap.h"
#include "THaTrack.h"
#include "THaTrackProj.h"
#include "THaBenchmark.h"
#include "VarDef.h"
#include "TMath.h"
#include "TList.h"
#include "TH1F.h"
#include "TError.h"
#include "TClonesArray.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#ifndef _WIN32
#include <sys/time.h>
#endif

#include "THaMWDC.h"

using namespace std;

//_____________________________________________________________________________
THaMWDC::THaMWDC( const char* name, const char* description,
                 THaApparatus* apparatus ) :
THaTrackingDetector(name,description,apparatus), fNtracks(0),
//    fDoBench(kFALSE)
fDoBench(kTRUE)
{  // Constructor

    // since the number of planes needs to be read in from
    // the database, the detector is actually constructed in the 
    // readdatabase method

    // Since TCloneArrays can resize, the size here is fairly unimportant
    fPlanes = new TClonesArray("THaMWDCPlane",0);

    // Default behavior for now
    // SetBit( kHardTDCcut );
    //  ResetBit( kIgnoreNegDrift );
    SetBit( kIgnoreNegDrift );

    fNmatrices = 0;

    fShowerStatus = 0;

    fTotalConsideredEvents = 0;

    fTotalTrackingEvents = 0;
    fRejectedTrackingEvents = 0;
    fHardMaxTrackingEvents = 0;
    fSoftMaxTrackingEvents = 0;

    fNtracks = 0;


    fBench = new THaBenchmark;
}

//_____________________________________________________________________________
THaMWDC::~THaMWDC()
{
    // Destructor. Delete subdetectors and unregister variables
    if (fIsSetup)
        RemoveVariables();

    delete fBench;          fBench = 0;
    delete fPlanes;         fPlanes = 0;
}

//_____________________________________________________________________________
THaAnalysisObject::EStatus THaMWDC::Init( const TDatime& date )
{
    // Initialize MWDC. Calls standard Init(), then initializes subdetectors.

    Int_t useshowercuts=0;

    fReconstructBits = 0;

    if( IsZombie() ) {
        return fStatus = kInitError;
    }
    EStatus status;
    status = (THaTrackingDetector::Init( date )) ;
    if (status!=0) return fStatus = status;

    for (UInt_t i=0; i<GetNPlanes(); i++) {
        status=GetPlane(i)->Init( date );
        if (status!=0)return fStatus = status;
        fReconstructBits |= (BIT(i)*GetPlane(i)->IsInReconstruction() );
    }

#ifdef DO_TRACKING_BBE
    // Need to pull quite a bit from THaBBe's database:
    THaBBe* bbe = dynamic_cast<THaBBe*>(GetApparatus());
    if( bbe ) {
        fMaxGroups = (UInt_t) bbe->GetMaxGroups();
        fHardMaxGroups =  (UInt_t) bbe->GetHardMaxGroups();
        fMinimumPlanes = (UInt_t) bbe->GetMinimumPlanesInRecon();
        fShowerXExt = bbe->GetShowerXExtension();
        fShowerYExt = bbe->GetShowerYExtension();
        fTargetXExt = bbe->GetTargetXExtension();
        fTargetYExt = bbe->GetTargetYExtension();
        fTargetWindowXOffset = bbe->GetTargetWindowXOffset();
        useshowercuts = bbe->UseShowerCuts();
        fMaxCallThreshold = bbe->GetMaxCallThreshold();
    }
    else {
        fMaxGroups = 1000000;
        fHardMaxGroups = 1000000;
        fMinimumPlanes =4;
        fShowerXExt = 0.0;
        fShowerYExt = 0.0;
        fTargetXExt = 0.0;
        fTargetYExt = 0.0;
        fTargetWindowXOffset = 0.0; 
        useshowercuts = false;
        fMaxCallThreshold = 1000000;
    }
#elif defined DO_TRACKING_BIGBITE
    //TODO: edit code for other tracking algorithm
    THaBigBite* bb = dynamic_cast<THaBigBite*>(GetApparatus());
    if( bb ) {
        //fMaxGroups = (UInt_t) bb->GetMaxGroups();
        //fHardMaxGroups =  (UInt_t) bb->GetHardMaxGroups();
        //fMinimumPlanes = (UInt_t) bb->GetMinimumPlanesInRecon();
        fShowerXExt = bb->GetShowerXExtension();
        fShowerYExt = bb->GetShowerYExtension();
        fTargetXExt = bb->GetTargetXExtension();
        fTargetYExt = bb->GetTargetYExtension();
        fTargetWindowXOffset = bb->GetTargetWindowXOffset();
        //useshowercuts = bb->UseShowerCuts();
        //fMaxCallThreshold = bb->GetMaxCallThreshold();
    }
    else {
        //fMaxGroups = 1000000;
        //fHardMaxGroups = 1000000;
        //fMinimumPlanes =4;
        fShowerXExt = 0.0;
        fShowerYExt = 0.0;
        fTargetXExt = 0.0;
        fTargetYExt = 0.0;
        fTargetWindowXOffset = 0.0; 
        //useshowercuts = false;
        //fMaxCallThreshold = 1000000;
    }

#else
    //fMaxGroups = 1000000;
    //fHardMaxGroups = 1000000;
    //fMinimumPlanes =4;
    fShowerXExt = 0.0;
    fShowerYExt = 0.0;
    fTargetXExt = 0.0;
    fTargetYExt = 0.0;
    fTargetWindowXOffset = 0.0; 
    //useshowercuts = false;
    //fMaxCallThreshold = 1000000;
#endif




    SetUseShowerCuts(useshowercuts);

#ifdef DO_TRACKING_BBE
    // Generate transformation matrices
    GenerateConstructMatrices();

    clog << "Requiring at least " << fMinimumPlanes << " in reconstruction" << endl;
    clog << "Only using planes ";
#endif //#ifdef DO_TRACKING_BBE

    for( UInt_t i = 0; i < GetNPlanes(); i++ )
    {
        if( GetPlane(i)->IsInReconstruction() )
            clog << GetPlane(i)->GetName() << " ";
    }

#ifdef DO_TRACKING_BBE
    clog << "in reconstruction" << endl;

    clog << "Skipping tracking on events that will have more than " << fMaxCallThreshold 
        << " estimated calls to consider."<< endl;
#endif //#ifdef DO_TRACKING_BBE

    // If using shower cuts print out some useful information
    if( UseShowerCuts() )
    {      
        clog << endl << "Shower cut specifications: " << endl 
            << "-------------------------------------------" << endl;
        clog << "Shower X Range (+/- m):\t\t"   << fShowerXExt << endl;
        clog << "Shower Y Range (+/- m):\t\t"   << fShowerYExt << endl;
        clog << "Target X Extension (+/- m):\t" << fTargetXExt << endl;
        clog << "Target Y Extension (+/- m):\t" << fTargetYExt << endl;

        clog << "Target X Offset (m):\t\t"<< fTargetWindowXOffset << endl;
        clog << "-------------------------------------------" << endl;
    }
    else
    {
        clog << endl << "*Not using cuts on shower*" << endl;
    }

    // Load up target information from THaBBe
    LoadTargetData();

    return fStatus = kOK;
}

//_____________________________________________________________________________
Int_t THaMWDC::Decode( const THaEvData& evdata )
{
    // Reset for new event
    Clear();

    // Decode all planes
    for (UInt_t i=0; i<GetNPlanes() ;i++) {
        GetPlane(i)->Decode(evdata);
    }

    return 0;
}

//_____________________________________________________________________________
Int_t THaMWDC::CoarseTrack( TClonesArray& tracks )
{
#ifndef _WIN32
    timeval start, stop;

    // Get time when starting coarse tracking

    gettimeofday(&start, NULL);
#endif

    SetValidShowerData(false);    

    // Clear existing tracks and find coarse tracks
    tracks.Clear("C");
    fNtracks = 0;
    fNtracks = ConstructTracks( &tracks, kCoarseTracking );

#ifndef _WIN32
    // Get time when stopping coarse tracking
    gettimeofday(&stop, NULL);

    // Calculate time it took to do coarse tracking (s)
    fCoarseProcTime = stop.tv_sec+stop.tv_usec/1000000.0-start.tv_sec-start.tv_usec/1000000.0;
#else
    fCoarseProcTime=0;
#endif
    return fNtracks;
}

//_____________________________________________________________________________
Int_t THaMWDC::FineTrack( TClonesArray& tracks )
{

#ifndef _WIN32
    timeval start, stop;

    // Get time when starting fine tracking  
    gettimeofday(&start, NULL);

#endif	//#ifndef _WIN32

    // Give up if we're only to do coarse tracking
    if( TestBit(kCoarseOnly) )
        return 0;

    // Clear existing tracks and find coarse tracks
    tracks.Clear("C");
    fNtracks = 0;
    fNtracks = ConstructTracks( &tracks, kFineTracking );

#ifndef _WIN32
    // Get time when stopping fine tracking
    gettimeofday(&stop, NULL);

    // Calculate time it took to do fine tracking (s)
    fFineProcTime = stop.tv_sec+stop.tv_usec/1000000.0-start.tv_sec-start.tv_usec/1000000.0;
#else
    fFineProcTime=0;
#endif	//#ifndef _WIN32

    return fNtracks;
}

//_____________________________________________________________________________
Int_t THaMWDC::DefineVariables( EMode mode )
{
    // Initialize global variables and lookup table for decoder


    if( mode == kDefine && fIsSetup ) return kOK;
    fIsSetup = ( mode == kDefine );

    // Register variables in global list

    RVarDef vars[] = {
        { "trackskipped", "Tracking skipped or truncated", "fTooBusy"       },
        { "cproctime",    "Coarse Processing Time",        "fCoarseProcTime"},
        { "fproctime",    "Fine Processing Time",          "fFineProcTime"  },
        { "estngrp",      "Estimated Number of Groups",    "fEstNGroups"    },
        { "estncall",     "Estimated Number of Calls",     "fEstNCalls"     },
        { "ngrp",         "Number of Groups",              "fNGroups"       },
        { "ncall",        "Number of recursive calls",     "fNCalls"        },
        { 0 }
    };
    DefineVarsFromList( vars, mode );

    return 0;
}

//_____________________________________________________________________________
void THaMWDC::Print(const Option_t* opt) const {
    THaTrackingDetector::Print(opt);

    clog<<"Number of Planes "<<GetNPlanes()<<endl;
    fBench->Print();

    return;
}


//_____________________________________________________________________________
void THaMWDC::SetDebug( Int_t level )
{
    // Set debug level of this detector, including all wire planes (subdetectors)

    THaTrackingDetector::SetDebug( level );

    for( UInt_t i = 0; i<GetNPlanes(); i++ ) {
        THaMWDCPlane* thePlane = static_cast<THaMWDCPlane*>( fPlanes->At(i) );
        thePlane->SetDebug( level );
    }
}

//_____________________________________________________________________________
void THaMWDC::EnableBenchmarks( Bool_t b )
{
    fDoBench = b;
}

//_____________________________________________________________________________
Int_t THaMWDC::ReadDatabase( const TDatime& date )
{
    // Read MWDC database

    FILE* file = OpenFile( date );
    if( !file ) return kFileError;

    // load global MWDC parameters
    static const char* const here = "ReadDatabase";
    const int LEN = 200;
    char buff[LEN];

    // Look for the section [<prefix>.global] in the file, e.g. [ R.global ]
    TString tag1(fPrefix);
    TString tag2(fPrefix);

    Ssiz_t pos = tag1.Index("."); 
    if( pos != kNPOS )
        tag1 = tag1(0,pos+1);
    else
        tag1.Append(".");
    tag1.Prepend("[");
    tag1.Append("global]"); 
    pos = tag2.Index("."); 
    if( pos != kNPOS )
        tag2 = tag2(0,pos+1);
    else
        tag2.Append(".");
    tag2.Prepend("[");
    tag2.Append("global.done]"); 



    TString line, tag3(tag1);
    tag1.ToLower();
    tag2.ToLower();


    bool found = false;
    while (!found && fgets (buff, LEN, file) != NULL) {
        char* buf = ::Compress(buff);  //strip blanks
        line = buf;
        delete [] buf;
        if( line.EndsWith("\n") ) line.Chop();
        line.ToLower();
        if ( tag1 == line ) 
            found = true;
    }
    if( !found ) {
        Error(Here(here), "Database entry %s not found!", tag3.Data() );
        fclose(file);
        return kInitError;
    }

    if( GetNPlanes()>0 || fIsInit ) {
        Warning(Here(here), "Database has already be read in.  Using the planes we already have." );
        Warning(Here(here), "Hope you have the same configureation in the databases." );
        //fclose(file);
        //return kInitError;
    }

    // We found the section, now read the data

    // read in some basic constants first
    else
    {
        TString planename = " ";
        TString planedescr = " ";
        Int_t nPlanes = 0;  
        while ((planename!=tag2)&&(fgets(buff, LEN, file)!=NULL)) {
            char* buf = ::Compress(buff);
            planename = buf;
            delete [] buf;
            if( planename.EndsWith("\n") ) planename.Chop();
            planename.ToLower();
            if (planename!=tag2) {
                fgets(buff, LEN, file);
                planedescr = buff;
                if( planedescr.EndsWith("\n") ) planedescr.Chop();
                planedescr.ToLower();
                THaMWDCPlane* thePlane = 
                    new((*fPlanes)[nPlanes]) THaMWDCPlane(planename,planedescr,this);
                thePlane->SetDebug(fDebug);

                //	  EStatus status=thePlane->Init( date );
                //	  if (status!=0)return fStatus = status;

                nPlanes++;
            }
        }
    }

    fclose(file);
    // final sanity check
    if (MAX_PLANES<GetNPlanes()) {
        Error(Here(here),"More planes in database than permitted by MAX_PLANES!");
        return kInitError;
    }

    fIsInit = true;
    return kOK;
}

//_____________________________________________________________________________
void THaMWDC::Clear( Option_t* opt )
{
    THaTrackingDetector::Clear(opt);

    fTooBusy = 0;

    for( UInt_t counter = 0; counter < GetNGroups(); counter++ )
    {
        delete GetGroup(counter);
    }
    fGroups.clear();
    fNGroups = 0;

    fCoarseProcTime = -1e9;
    fFineProcTime = -1e9;
    fEstNGroups = 0;
    fEstNCalls = 0;
    fNCalls = 0;

}

//_______________________________________________________________________________
Int_t THaMWDC::End(THaRunBase *run)
{
    PrintCutInfo();
    fBench->Print();
    return THaTrackingDetector::End(run);
}

//_______________________________________________________________
Int_t THaMWDC::ConstructTracks( TClonesArray* tracks, ETrackingMode mode )
{  
#if !DO_THAMWDC_TRACKING
    {
        //const char *lbl="THaMWDC::Tracking DISABLED";
        //static void *use_lbl = (void*)&lbl;
    }
    // REMOVE THIS LINE TO ENABLE TRACKING
    return 0;
#elif defined DO_TRACKING_OTHER
    //TODO: add code for other tracking algorithm

#elif defined DO_TRACKING_BBE
    {
        const char *lbl="THaMWDC::Tracking ENABLED";
        static void *use_lbl = (void*)&lbl;
    }

    vector<THaMWDCGroup*> groups_with_track;
    vector<THaMWDCGroup*> unique_groups;

    UInt_t i, j;
    Int_t nTracks = 0;

    if (fDoBench) fBench->Begin("THaMWDC::ConstructTracks");

    // Find groups if we just starting coarse tracking
    if( mode == kCoarseTracking )
    {
        if (fDoBench) fBench->Begin("THaMWDC::FindGroups");
        FindGroups();
        if (fDoBench) fBench->Stop("THaMWDC::FindGroups");
    }  

    // Get scintillator time correction if we're now doing fine tracking
    if( mode == kFineTracking )
    {
        SetScintCorrection();
    }


    // For all groups, try to find a track
    if (fDoBench) fBench->Begin("THaMWDC::FindTrack");
    for( i = 0; i < GetNGroups(); i++ )
    {
        if( GetGroup(i) )
        {
            if( mode == kCoarseTracking )
            {
                GetGroup(i)->DoCoarseTracking();

                // If track is good, save it
                if( GetGroup(i)->IsValidTrack() )
                    groups_with_track.push_back(GetGroup(i));
            }

            if( mode == kFineTracking && GetGroup(i)->IsL1Group() )
            {
                GetGroup(i)->DoFineTracking();

                // If track is good, save it
                if( GetGroup(i)->IsValidTrack() )
                    groups_with_track.push_back(GetGroup(i));
            }
        }
    }

    if (fDoBench) fBench->Stop("THaMWDC::FindTrack");

    if (fDoBench) fBench->Begin("THaMWDC::SortTracksGroups");

    // HeapSort groups_with_track by chi^2
    HeapSortGroupsByChi2(groups_with_track);

    Bool_t isUnique;

    // Keep top N groups for coarse tracking
    if( mode == kCoarseTracking )
    {
        for( i = 0; i < TOPNCOARSETRACKS && i < groups_with_track.size(); i++ )
        {
            groups_with_track[i]->SetL1Group(true);
            unique_groups.push_back( groups_with_track[i] );
        }
    }

    // Keep only top N UNIQUE groups for fine tracking
    if( mode == kFineTracking )
    {
        i = 0;
        while( unique_groups.size() < TOPNTRACKS && i < groups_with_track.size() )
        {
            isUnique = true;
            for( j = 0; j < unique_groups.size(); j++ )
            {
                if( groups_with_track[i]->IsSimilarTrack( unique_groups[j]->GetTrack() ) )
                {
                    isUnique = false;
                }
            }
            if( isUnique )
            {
                groups_with_track[i]->SetL2Group(true);
                unique_groups.push_back( groups_with_track[i] );
            }
            i++;
        }
    }


    if (fDoBench) fBench->Stop("THaMWDC::SortTracksGroups");


    if (fDoBench) fBench->Begin("THaMWDC::CheckTracks");
    for( i = 0; i < unique_groups.size(); i++ )
    {
        // Create our final track object to be returned to THaBBe
        THaTrack *tr = new((*tracks)[nTracks++]) THaTrack( unique_groups[i]->GetX(), unique_groups[i]->GetY(),
            unique_groups[i]->GetTheta(), unique_groups[i]->GetPhi() );
        tr->SetChi2( unique_groups[i]->GetChi2(), unique_groups[i]->GetNDoF() );

        // Mark our groups' track numbers and fix the chi^2 output
        // Set the tracking level flag for THaTrack
        if( mode == kCoarseTracking )
        {
            tr->SetFlag(1);
        }
        if( mode == kFineTracking )
        {
            unique_groups[i]->SetTrackNumber(nTracks-1);
            unique_groups[i]->SetChi2Cont();
            tr->SetFlag(2);
        }
    }

    if (fDoBench) fBench->Stop("THaMWDC::CheckTracks");

    if (fDoBench) fBench->Begin("THaMWDC::CleanTracks");


    if (fDoBench) fBench->Stop("THaMWDC::CleanTracks");
    if (fDoBench) fBench->Stop("THaMWDC::ConstructTracks");

    groups_with_track.clear();
    unique_groups.clear();

    return nTracks;
#endif //#ifndef  DO_TRACKING_BBE
}

//____________________________________________________________________________
#ifdef DO_TRACKING_BBE

void THaMWDC::FindGroups()
{

    // Find groups (clusters) of plane hits. Done once at CoarseReconstruct stage

    // This is where we string together sets of hits and where the magic of tracking
    // happens

    UInt_t nplanes = GetNPlanes();

    if( nplanes < fMinimumPlanes ) return;

    UInt_t planebits; 
    UInt_t sortcounter;
    UInt_t plane;
    UInt_t counter;
    UInt_t trunc = 0;

    UInt_t estimatedgroups = 0;
    UInt_t estimatedcalls = 0;
    Int_t validwires = 0;

    Int_t hitplanes = 0;
    Int_t sum;
    Int_t product;

    vector<THaMWDCGroup*> unsortedgroups;
    vector<THaMWDCHit*>   currenthits;

    UInt_t usablebits;
    Int_t usablewires[MAX_PLANES];

    UInt_t lastbits = (1 << nplanes)-1;  // 2^(nplanes)-1

    if( UseShowerCuts() )
    {
        GetShowerData();
    }	  

    // Let's just check all the hits before we do anything and get a sense of how bad
    // things are going to be for tracking...  Let's not bother if we have too many
    // estimated groups

    usablebits = 0;
    product = 1;

    // Count up the number of usable wires (wires that pass shower cuts) in
    // each plane

    for( UInt_t planecounter = 0; planecounter < nplanes; planecounter++ )
    {
        if( GetPlane(planecounter)->IsInReconstruction() )
        {
            validwires = 0;
            for( counter = 0; counter < GetPlane(planecounter)->GetNHits(); counter++ )
            {	      
                HitValidByShower( GetPlane(planecounter)->GetHit(counter) );
            }

            for( counter = 0; counter < GetPlane(planecounter)->GetNHits(); counter++ )
            {
                if( GetPlane(planecounter)->GetHit(counter)->IsValidByShower() )
                    validwires++;
            }
            if( validwires > 0 )
            {
                product *= validwires;
                estimatedgroups += product;
                hitplanes++;
                usablebits |= BIT(planecounter);
            }
            usablewires[planecounter] = validwires;
        }
    }

    fEstNGroups = estimatedgroups;

    Int_t ncombos;
    // Try and estimate number of calls based on the number of usable wires
    // This calculation is pretty accurate

    for( planebits = lastbits; planebits > 0; planebits-- )
    {
        sum = 0;
        ncombos = 1;
        if( fValidBits[planebits] && ((usablebits & planebits)==planebits) )
        {
            for( plane = 0; plane < nplanes; plane++ )
            {
                if( BIT(plane) & planebits )
                {
                    ncombos *= usablewires[plane];
                    sum += ncombos;
                }
            }
            estimatedcalls += sum;
        }
    }

    fEstNCalls = estimatedcalls;
    fTotalTrackingEvents++;

    // If we have too many calls set the appropriate toobusy bit and just give up
    if( estimatedcalls > fMaxCallThreshold )
    {
        fTooBusy |= kRejected;
        fRejectedTrackingEvents++;
        return;
    }

    // This variable keeps track of how many times we call AllCombos, which basically
    // gives up how much time CoarseTracking takes
    fNCalls = 0;

    // Check to make sure we have at least as many planes we need to do
    // the reconstruction
    if( hitplanes >= (Int_t) fMinimumPlanes )
    {
        // for all combinations of planes given by planebits
        for( planebits = lastbits; planebits > 0; planebits-- )
        {
            // if it is a valid combination of planes and all those planes 
            // have at least one valid hit
            if( fValidBits[planebits] && ((usablebits & planebits)==planebits) )
            {
                // Put together all valid combinations of hits for 
                // those planes in planebits
                AllCombos( planebits, unsortedgroups, currenthits, 1 );
            }
        }
    }

    // if we have the number of groups that is our maximum, we did a cutoff
    // and we count it as a hardmax tracking event and set the flag
    if(fHardMaxGroups == (UInt_t) unsortedgroups.size() )
    {
        fHardMaxTrackingEvents++;
        fTooBusy |= kHardMax;
    }

    // Sorting Groups by number of hits
    // We start with the maximum number of planes and work our way down
    // This basically let's us keep the groups with larger numbers of hits
    // first
    for( sortcounter = nplanes; sortcounter >= fMinimumPlanes; sortcounter-- )
    {	   
        // If the number of groups we're keeping is still less than our soft maximum
        if( GetNGroups() < fMaxGroups )
        {
            // Save all the groups with sortcounter number of hits
            for( counter = 0; counter < unsortedgroups.size(); counter++ )  
            {
                if( unsortedgroups[counter] != NULL )
                {
                    if( unsortedgroups[counter]->GetNHits() == sortcounter )
                        fGroups.push_back( unsortedgroups[counter] );
                }	    
            }
        }
        else
            // We have hit our soft maximum.  Now we delete all groups with
            // sortcounter number of hits
        {
            for( counter = 0; counter < unsortedgroups.size(); counter++ )
            {
                if( unsortedgroups[counter] != NULL )
                {
                    // We're truncating our groups, mark it and count it
                    if( !trunc  )
                    {
                        trunc = 1;
                        fSoftMaxTrackingEvents++;
                        fTooBusy |= kSoftMax;
                    }

                    if( unsortedgroups[counter]->GetNHits() == sortcounter )
                    {
                        delete unsortedgroups[counter];
                        unsortedgroups[counter] = NULL;
                    }

                }
            }
        }

    }

    unsortedgroups.clear();
    currenthits.clear();
    fNGroups = GetNGroups();
    return;

}
#endif //#ifdef DO_TRACKING_BBE
//_____________________________________________________________________________
#ifdef DO_TRACKING_BBE

void THaMWDC::AllCombos( Int_t planebits, vector<THaMWDCGroup*> &groups, vector<THaMWDCHit *> hits, Int_t depth )
{
    // We must be VERY CAREFUL of what we put in here.  This function gets called
    // MANY MANY MANY times and will basically define how long you spend doing
    // coarse tracking.  We must keep this as stripped down as possible.

    fNCalls++;
    Int_t counter;
    THaMWDCPlane *plane;
    vector<THaMWDCHit *> tempvector;

    // Hard maximum.  Stop making more groups
    if( groups.size() >= fHardMaxGroups )
    {
        return;
    }

    // fPlaneNumbersForBits == -1 when we are past our maximum depth
    // i.e. we've already done all the planes for planebits
    if( fPlaneNumbersForBits[planebits][depth-1] != -1 )
    {
        plane = GetPlane(fPlaneNumbersForBits[planebits][depth-1]);
        // for all valid hits, make a copy of the string of hits and add
        // the valid hit onto the end.  Pass that string onto the next
        // level
        for( counter = 0; counter < (Int_t) plane->GetNHits(); counter++ )
        { 
            if( HitValidByShower( plane->GetHit(counter) ) )
            {
                tempvector = hits;
                tempvector.push_back( plane->GetHit(counter) );
                AllCombos( planebits, groups, tempvector, depth+1 );
                tempvector.clear();
            }
        } 
    }
    else
    {
        // If we're past our maximum depth, we have a complete string of hits
        // Make the new group and put it in the pile
        groups.push_back( new THaMWDCGroup( hits, planebits, fConstructMatrix[planebits] ) );
    }

    return;

}
#endif //#ifdef DO_TRACKING_BBE
//_______________________________________________________________________________

#ifdef DO_TRACKING_BBE
Bool_t THaMWDC::HitValidByShower( THaMWDCHit *hit )
{	// Determine if a hit wire is within our shower window

    Bool_t inside = false;

    Double_t wirepos, wirex1, wirex2, Angle, maxlimitx, maxlimity, minlimitx, minlimity,
        targetxhigh, targetyhigh, targetxlow, targetylow, targetz;

    wirepos = wirex1 = wirex2 = Angle = maxlimitx = maxlimity = minlimitx = minlimity =
        targetxhigh = targetyhigh = targetxlow = targetylow = targetz = 0.0;

    //If we've already checked this wire return what we already found
    if( hit->VerifiedByShower() )
    {
        return hit->IsValidByShower();
    }

    // If we don't have valid shower data just set it as true
    if( !HaveValidShowerData() )
    {
        hit->SetVerifiedByShower(true);
        hit->SetIsValidByShower(true);
        return 1;
    }

    Angle = hit->GetPlane()->GetWAngle();

    wirepos = hit->GetWire()->GetPos() + hit->GetPlane()->GetXYCorr();

    targetxhigh = fTargetXhigh;
    targetyhigh = fTargetYhigh;
    targetxlow = fTargetXlow;
    targetylow = fTargetYlow;
    targetz = fTargetZ;

    // Calculate our our shower window projected onto the plane
    maxlimitx = ( (fShowerOrigin.X() + fShowerXExt - targetxhigh) / (fShowerOrigin.Z() - targetz ) ) 
        * (hit->GetPlane()->GetZ() - targetz) + targetxhigh;
    minlimitx = ( (fShowerOrigin.X() - fShowerXExt - targetxlow) / (fShowerOrigin.Z() - targetz ) ) 
        * (hit->GetPlane()->GetZ() - targetz) + targetxlow;

    maxlimity = ( (fShowerOrigin.Y() + fShowerYExt - targetyhigh) / (fShowerOrigin.Z() - targetz ) ) 
        * (hit->GetPlane()->GetZ() - targetz) + targetyhigh;
    minlimity = ( (fShowerOrigin.Y() - fShowerYExt - targetylow) / (fShowerOrigin.Z() - targetz ) ) 
        * (hit->GetPlane()->GetZ() - targetz) + targetylow;

    // Calculate the x intercepts on the top and bottom of this window
    wirex1 = (wirepos / TMath::Sin(TMath::Abs(Angle))) + (minlimity * TMath::Cos(Angle)/TMath::Sin(Angle));
    wirex2 = (wirepos / TMath::Sin(TMath::Abs(Angle))) + (maxlimity * TMath::Cos(Angle)/TMath::Sin(Angle));

    // if these x intercepts are within our x limits, we're inside the window.  Otherwise we're not
    if( !( (wirex1 > maxlimitx && wirex2 > maxlimitx) || (wirex1 < minlimitx && wirex2 < minlimitx )) )
    { 
        inside = true;
    }
    else
    {
        inside = false;
    }

    hit->SetVerifiedByShower(true);
    hit->SetIsValidByShower(inside);
    return inside;

}
#endif //#ifdef DO_TRACKING_BBE
//______________________________________________________________________________________

#ifdef DO_TRACKING_BBE

void THaMWDC::GetShowerData()
{

    // Need to get shower origin
    // Need to do the Coarse AND Fine processing before we can do anything,

    // Set HaveValidShowerData to true if we've got a cluster we can work with

    SetValidShowerData(false);
    SetValidClusterData(false);

    // Get the total shower object named "ts", return if it doesn't exist
    THaBBTotalShower *totalshower = dynamic_cast<THaBBTotalShower *>( GetApparatus()->GetDetector("ts"));
    if( totalshower == 0 )
        return;

    // Get the shower object from TotalShower, return if it doesn't exist
    THaBBShower *shower = totalshower->GetShower();
    if( shower == 0 ) 
        return;

    // If we have a cluster, we're in good shape
    if( shower->GetNclust() == 0 ){
        return;
    }
    else
    {
        SetValidClusterData(true);
    }

    // Get the position of the shower for shower cut calculations
    fShowerOrigin = TVector3( shower->GetX(0), shower->GetY(0), shower->GetOrigin().Z() );
    SetValidShowerData(true);

    return;
}
#endif //#ifdef DO_TRACKING_BBE

//_______________________________________________________________________________
void THaMWDC::LoadTargetData()
{
    TVector3 targetstart;
    TVector3 targetend;
    TVector3 targetcenter;

#ifdef DO_TRACKING_BBE
    THaBBe* bb = dynamic_cast<THaBBe*>(GetApparatus());

    // Get information on target position in detector coordinates
    // from THaBBe.  We need this for shower cut calculations.
    if( bb ) {
        targetstart  =  bb->GetTargetStartInDet();
        targetend    =  bb->GetTargetEndInDet();
        targetcenter =  bb->GetTargetCenterInDet();
        fTargetZ     = (bb->GetTargetCenterInDet()).Z();

        if( targetstart. X() > targetend.X() ) {
            fTargetXhigh = targetstart.X() + fTargetXExt + fTargetWindowXOffset;
            fTargetXlow  = targetend.X()   - fTargetXExt + fTargetWindowXOffset;
        } 
        else {
            fTargetXhigh = targetend.X()   + fTargetXExt + fTargetWindowXOffset;
            fTargetXlow  = targetstart.X() - fTargetXExt + fTargetWindowXOffset;
        }

        if( targetstart.Y() > targetend.Y() ) {
            fTargetYhigh = targetstart.Y() + fTargetYExt;
            fTargetYlow  = targetend.Y()   - fTargetYExt;
        }
        else {
            fTargetYhigh = targetend.Y()   + fTargetYExt;
            fTargetYlow  = targetstart.Y() - fTargetYExt;
        }
    }
    else {
        fTargetZ = 0.0;
        fTargetXhigh = 0.0;
        fTargetYhigh = 0.0;
        fTargetXlow = 0.0;
        fTargetYlow = 0.0;
    }   
#elif defined DO_TRACKING_BIGBITE
    //TODO: modify code for other tracking algorithm
    THaBigBite* bb = dynamic_cast<THaBigBite*>(GetApparatus());

    // Get information on target position in detector coordinates
    // from THaBBe.  We need this for shower cut calculations.
    if( bb ) {
        targetstart  =  bb->GetTargetStartInDet();
        targetend    =  bb->GetTargetEndInDet();
        targetcenter =  bb->GetTargetCenterInDet();
        fTargetZ     = (bb->GetTargetCenterInDet()).Z();

        if( targetstart. X() > targetend.X() ) {
            fTargetXhigh = targetstart.X() + fTargetXExt + fTargetWindowXOffset;
            fTargetXlow  = targetend.X()   - fTargetXExt + fTargetWindowXOffset;
        } 
        else {
            fTargetXhigh = targetend.X()   + fTargetXExt + fTargetWindowXOffset;
            fTargetXlow  = targetstart.X() - fTargetXExt + fTargetWindowXOffset;
        }

        if( targetstart.Y() > targetend.Y() ) {
            fTargetYhigh = targetstart.Y() + fTargetYExt;
            fTargetYlow  = targetend.Y()   - fTargetYExt;
        }
        else {
            fTargetYhigh = targetend.Y()   + fTargetYExt;
            fTargetYlow  = targetstart.Y() - fTargetYExt;
        }
    }
    else {
        fTargetZ = 0.0;
        fTargetXhigh = 0.0;
        fTargetYhigh = 0.0;
        fTargetXlow = 0.0;
        fTargetYlow = 0.0;
    }   

#else
    fTargetZ = 0.0;
    fTargetXhigh = 0.0;
    fTargetYhigh = 0.0;
    fTargetXlow = 0.0;
    fTargetYlow = 0.0;
#endif //#ifdef DO_TRACKING_BBE

    clog << "Beam position in detector coords:  (" << targetcenter.X() 
        << ", " << targetcenter.Y() 
        << ", " << targetcenter.Z() << ")" 
        << endl << endl;

    clog << "Beam direction in detector coords: (" 
        << (targetstart - targetend).Unit().X() << ", " 
        << (targetstart - targetend).Unit().Y() << ", "
        << (targetstart - targetend).Unit().Z() <<  ")"
        << endl << endl;

    return;
}

//_______________________________________________________________________________
void THaMWDC::PrintCutInfo()
{
    // Print info on drift time cuts and group cuts

    UInt_t plane;

    if( TestBit( kIgnoreNegDrift ) )
    {
        clog <<endl << endl << "Drift Time Cuts" << endl;
        clog << "---------------------------------------------------------------------------" << endl;
        clog << "Plane   Rejected   Total     Min Drift  MaxDrift" << endl;
        for( plane = 0; plane < GetNPlanes(); plane++ )
        {
            GetPlane(plane)->PrintDriftCuts();
        }
        clog << "---------------------------------------------------------------------------" << endl;
        clog << endl;
    }  

    if( fTotalTrackingEvents > 0 )
    {
        clog <<endl << endl << "Group Cuts" << endl;
        clog << "---------------------------------------------------------------------------" << endl;
        clog << fRejectedTrackingEvents << "\trejected tracking events" << endl;
        clog << fHardMaxTrackingEvents << "\t events trucated by hard maximum" << endl;
        clog << fSoftMaxTrackingEvents << "\t events trucated by soft maximum" << endl;
        clog << fTotalTrackingEvents << "\tTOTAL tracking events" << endl;
        clog << "---------------------------------------------------------------------------" << endl;
        clog << endl;
    }
    return;
}

#ifdef DO_TRACKING_BBE
//_____________________________________________________________________________
void THaMWDC::GenerateConstructMatrices()
{
    // Generate matrices that transform between plane coordinates and track coordinates
    // for all combinations of planes.  fValidBits will store if the matrix was able
    // to be generated sucessfully.  No matrix will be generated if the plane combination
    // cannot yield a full 3D track.

    if( GetNPlanes() < fMinimumPlanes ) return;

    memset(fValidBits,0,MAX_MATRIX*sizeof(*fValidBits));

    UInt_t lastbits = (1 << GetNPlanes())-1;     // 2^(nplanes)-1

    clog << "Generating Matrices" << endl;
    if( lastbits > MAX_MATRIX )
    {
        clog << "You don't have enough space to store all the matrices." << endl 
            << "\t" << MAX_MATRIX << " maximum array size" << "\t" << lastbits << " potential matrices" 
            << endl << endl 
            << "Please increase the size of the MAX_MATRIX definition in THaMWDC.h to at least " << lastbits << endl;
        //FIXME: AHHHHHRRRRGHHHH
        // At least I'm verbose about it....
        exit(1);
    }

    for( UInt_t trialbits = 0; trialbits <= lastbits; trialbits++ )
    {
        if( (~fReconstructBits & trialbits) == 0 )
            GenerateConstructMatrixForBits( trialbits );
        else
            fValidBits[trialbits] = false;
    }

    clog << "Completed Matrix Generation - " << fNmatrices << " matrices generated" << endl;
    return;
}

//_____________________________________________________________________________
void THaMWDC::GenerateConstructMatrixForBits( Int_t trialbits )
{
    // Refer to the document I've written to explain this calculation.  Basically,
    // we're just generating the NPLANESx4 matrix that, when we multiply it by
    // a column vector of plane coordinates, we get our 4 track variables out
    // that represent the minimum chi^2 fit

    Int_t planenumber[MAX_PLANES];
    UInt_t nplanes;
    UInt_t counter;

    Double_t mvalues[MAX_PLANES*4];


    TMatrixD *M;
    TMatrixD *Mt;
    TMatrixD *Minv;
    TMatrixD *Mfinal;

    for( counter = 0; counter < GetNPlanes(); counter++ )
    {
        mvalues[counter*4+0] = 0.0;   // x
        mvalues[counter*4+1] = 0.0;   // x'
        mvalues[counter*4+2] = 0.0;   // y
        mvalues[counter*4+3] = 0.0;   // y'
    }

    nplanes = 0;
    counter = 0;

    for( counter = 0; counter < GetNPlanes(); counter++ )
    {
        if( trialbits & BIT(counter) )
        {
            nplanes++;
            planenumber[nplanes-1] = counter;
            fPlaneNumbersForBits[trialbits][nplanes-1] = counter;
        }
    }

    fPlaneNumbersForBits[trialbits][nplanes] = -1;

    if( nplanes >= fMinimumPlanes )
    {
        for( counter = 0; counter < nplanes; counter++ )
        {

            mvalues[planenumber[counter]*4+0] = 
                TMath::Sin( TMath::Abs(GetPlane(planenumber[counter])->GetWAngle()) ) 
                / GetPlane(planenumber[counter])->GetPosRes();

            mvalues[planenumber[counter]*4+1] = GetPlane(planenumber[counter])->GetZ() * 
                TMath::Sin( TMath::Abs(GetPlane(planenumber[counter])->GetWAngle()) ) 
                / GetPlane(planenumber[counter])->GetPosRes();

            mvalues[planenumber[counter]*4+2] = -1.0 * (GetPlane(planenumber[counter])->GetWAngle() 
                / TMath::Abs(GetPlane(planenumber[counter])->GetWAngle()) ) 
                * TMath::Cos(GetPlane(planenumber[counter])->GetWAngle() )
                / GetPlane(planenumber[counter])->GetPosRes();

            mvalues[planenumber[counter]*4+3] = GetPlane(planenumber[counter])->GetZ() *
                -1.0 * (GetPlane(planenumber[counter])->GetWAngle() 
                / TMath::Abs(GetPlane(planenumber[counter])->GetWAngle()) ) 
                * TMath::Cos(GetPlane(planenumber[counter])->GetWAngle() )
                / GetPlane(planenumber[counter])->GetPosRes();
        }

        M = new TMatrixD( GetNPlanes(), 4, mvalues );

        Mt = new TMatrixD(4,GetNPlanes());
        Mt->Transpose(*M);

        Minv = new TMatrixD( 4, 4 );
        Minv->Mult( *Mt, *M );

        if( TMath::Abs(Minv->Determinant()) > MIN_DETERMINANT )
        {
            Minv->Invert();

            Mfinal = new TMatrixD( 4, GetNPlanes() );
            Mfinal->Mult(*Minv, *Mt);
            //	  Mfinal->Print();
            fConstructMatrix[trialbits] = Mfinal;
            fValidBits[trialbits] = true;
            fNmatrices++;
        }
        else
        {
            fValidBits[trialbits] = false;
        }

        delete M;
        delete Minv;
        delete Mt;
    }
    return;
}

//_____________________________________________________________________________
void THaMWDC::HeapSortGroupsByChi2( vector <THaMWDCGroup *> &v )
{
    // Heap sort groups v by chi^2/NDoF in ascending order

    vector <THaMWDCGroup *> sorted;

    HeapSortBuildHeap(v);

    while( !v.empty() )
    {
        sorted.push_back( v.front() );

        v[0] = v.back();
        v.pop_back();

        HeapSortSiftDown( v, 0);
    }

    v.clear();
    v = sorted;
    sorted.clear();

    return;
}

//_____________________________________________________________________________
void THaMWDC::HeapSortSiftDown( vector <THaMWDCGroup *> &v, UInt_t n )
{
    // Enforce local binary tree properies at a node

    UInt_t node1, node2;

    THaMWDCGroup *temp;

    node1 = 2*n+1;
    node2 = 2*n+2;

    // No node or no decendant node, just quit
    if( n >= v.size() || node1 >= v.size() ) return;

    // We have at least one decendant node
    if( v[node1]->GetChi2NDoF() < v[n]->GetChi2NDoF() )
    {
        // Must do a swap here
        if( node2 < v.size() )
        {
            // We have a second node
            if( v[node1]->GetChi2NDoF() > v[node2]->GetChi2NDoF() )
            {
                // Swap node with n2
                temp = v[n];
                v[n] = v[node2];
                v[node2] = temp;
                HeapSortSiftDown( v, node2 );

            }
            else
            {
                // Swap node with n1
                temp = v[n];
                v[n] = v[node1];
                v[node1] = temp;
                HeapSortSiftDown( v, node1 );
            }
        }
        else
        {
            // No n2, swap with n1
            temp = v[n];
            v[n] = v[node1];
            v[node1] = temp;
        }
    }

    return;  
}

//_____________________________________________________________________________
void THaMWDC::HeapSortBuildHeap( vector <THaMWDCGroup *> &v )
{
    // Turns groups in v into a heap (partially complete binary tree) with
    // respect to chi^2/NDoF

    Int_t i;
    for( i = v.size()/2+1; i >= 0; i-- )
    {
        HeapSortSiftDown(v,(UInt_t) i);
    }
}


//_____________________________________________________________________________
Bool_t THaMWDC::ValidPlaneBits( UInt_t bits )
{
    if( bits >= MAX_MATRIX || bits < 0 )
    {
        clog << "Tried to see if " << bits << " was valid (they're not)" << endl;
        return 0;
    }
    return fValidBits[bits];
}


//___________________________________________________________________________
Double_t THaMWDC::GetScintillatorTime()
{
    // Get time for struck pad in THaScintillator "s"
    Double_t scinttime;

    THaScintillator *scint = dynamic_cast<THaScintillator *>( GetApparatus()->GetDetector("s"));
    if( scint == 0 )
    {
        return 0.0;
    }

    const TClonesArray *TrackHits;
    const Double_t *times;

    if( scint->GetTrackHits()->GetEntries() == 0 ) return 0.0;
    TrackHits = scint->GetTrackHits();
    THaTrackProj* primary_track = (THaTrackProj *) TrackHits->At(0);
    times = scint->GetTimes();

    scinttime = times[primary_track->GetChannel()] - SCINT_OFFSET;

    if( TMath::Abs(scinttime)>SCINT_TIME_WIDTH ) return 0.0;

    return scinttime;

}

//___________________________________________________________________________
Double_t THaMWDC::GetScintillatorZ()
{
    // Get detector Z position of scintillator
    THaScintillator *scint = dynamic_cast<THaScintillator *>( GetApparatus()->GetDetector("s"));
    if( scint == 0 )
    {
        return 0.0;
    }

    return scint->GetOrigin().Z();

}

//___________________________________________________________________________
Double_t THaMWDC::GetScintTimeCorrection(THaMWDCPlane *pl)
{
    return (GetScintillatorTime() - SPEED_OF_LIGHT*( GetScintillatorZ() - pl->GetZ()));
}

#endif //#ifdef DO_TRACKING_BBE

//___________________________________________________________________________

void THaMWDC::SetScintCorrection()
{

    UInt_t plane, hit;

    for( plane = 0; plane < GetNPlanes(); plane++ )
    {
        for( hit = 0; hit < GetPlane(plane)->GetNHits(); hit++ )
        {

#ifdef DO_TRACKING_BBE

            GetPlane(plane)->GetHit(hit)->SetScintOffset(GetScintTimeCorrection(GetPlane(plane)));
#elif defined DO_TRACKING_OTHER
            //TODO: add correction code for other tracking algorithm
#else
            Warning(Here("SetScintCorrection()"),"No SetScintCorrection algorithm yet, set to 0 by default");
            GetPlane(plane)->GetHit(hit)->SetScintOffset(0);
#endif //#ifdef DO_TRACKING_BBE
        }
    }
}

//___________________________________________________________________________

ClassImp(THaMWDC)

////////////////////////////////////////////////////////////////////////////////
