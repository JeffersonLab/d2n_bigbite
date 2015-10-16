//*-- Author :    Bob Michaels,  March 2002

//////////////////////////////////////////////////////////////////////////
//
// BBDecData
//  Originally THaDecData... adapted for use with BigBiteLib
//  by Bryan Moffit, March 2008.
//
// Hall A miscellaneous decoder data, which typically does not 
// belong to a detector class.
// Provides global data to analyzer, and
// a place to rapidly add new channels.
//
// Normally the user should have a file "decdata.map" in their pwd
// to define the locations of raw data for this class (only).
// But if this file is not found, we define a default mapping, which
// was valid at least at one point in history.
//
// The scheme is as follows:
//
//    1. In Init() we define a list of global variables which are tied
//       to the variables of this class.  E.g. "timeroc2".
//
//    2. Next we build a list of "BBdataLoc" objects which store information
//       about where the data are located.  These data are either directly
//       related to the variables of this class (e.g. timeroc2 is a raw
//       data word) or one must analyze them to obtain a variable.
//
//    3. The BBdataLoc objects may be defined by decdata.map which has an
//       obvious notation (see ~/examples/decdata.map).  The entries are either 
//       locations in crates or locations relative to a unique header.
//       If decdata.map is not in the pwd where you run analyzer, then this
//       class uses its own internal DefaultMap().
//
//    4. The BBdataLoc objects pertain to one data channel (e.g. a fastbus
//       channel) and and may be multihit.
//
//    5. To add a new variable, if it is on a single-hit channel, you may
//       imitate 'synchadc1' if you know the (crate,slot,chan), and 
//       imitate 'timeroc2' if you know the (crate,header,no-to-skip).
//       If your variable is more complicated and relies on several 
//       channels, imitate the way 'bits' leads to 'evtypebits'.
//
// R. Michaels, March 2002
//
// OR  (NEW as of April 2004:  R.J. Feuerbach)
//       If you are simply interested in the readout of a channel, create
//       a name for it and give the location in the map file and a
//       global variable will be automatically created to monitor that channel.
//     Unfortunately, this leads to a limitation of using arrays as opposed
//     to variable-sized vector for the readout. Currently limited to 16 hits
//     per channel per event.
//
//////////////////////////////////////////////////////////////////////////

//#define WITH_DEBUG 1
#define NEW
#include "BBDecData.h"
#include "THaVarList.h"
#include "THaVar.h"
#include "THaGlobals.h"
#include "THaEvData.h"
#include "THaDetMap.h"
#include "TDatime.h"
#include "TH1.h"
#include "TClass.h"
#include "TNamed.h"

#include "VarDef.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <cctype>
#include <vector>
#include <string>
#include <stdlib.h>

using namespace std;

class BBdataLoc : public TNamed {
    // Utility class used by BBDecData.
    // Data location, either in (crates, slots, channel), or
    // relative to a unique header in a crate or in an event.
    static const Int_t MxHits=16;
public:
    // c'tor for (crate,slot,channel) selection
    BBdataLoc ( const char* nm, Int_t cra, Int_t slo, Int_t cha ) :
      TNamed(nm,nm), crate(cra), slot(slo), chan(cha), header(0), ntoskip(0), 
          search_choice(0) { Clear(); }
      // c'tor for header search (note, the only diff to above is 3rd arg is UInt_t)
      //FIXME: this kind of overloading (Int/UInt) is asking for trouble...
      BBdataLoc ( const char* nm, Int_t cra, UInt_t head, Int_t skip ) :
      TNamed(nm,nm), crate(cra), slot(0), chan(0), header(head), ntoskip(skip),
          search_choice(1) { Clear(); }
      Bool_t IsSlot() { return (search_choice == 0); }
#ifdef NEW
      Bool_t isRefCh;
      Bool_t needsRefChSub;
#endif
      void Clear( const Option_t* opt="" ) { ndata=0;  loaded_once = kFALSE; }
      void Load(UInt_t data) {
          if (ndata<MxHits) rdata[ndata++]=data;
          loaded_once = kTRUE;
      }
      Bool_t DidLoad() { return loaded_once; }
      Int_t NumHits() { return ndata; }
      UInt_t Get(Int_t i=0) { 
          return (i >= 0 && ndata > i) ? rdata[i] : 0; }
      //FIXME: this should be operator==( const char* )
      Bool_t ThisIs(const char* aname) { return fName==aname;}
      // operator== and != compare the hardware definitions of two BBdataLoc's
      Bool_t operator==( const BBdataLoc& rhs ) const {
          return ( search_choice == rhs.search_choice && crate == rhs.crate &&
              ( (search_choice == 0 && slot == rhs.slot && chan == rhs.chan) ||
              (search_choice == 1 && header == rhs.header && 
              ntoskip == rhs.ntoskip)
              )  
              );
      }
      Bool_t operator!=( const BBdataLoc& rhs ) const { return !(*this == rhs ); }
      void SetSlot( Int_t cr, Int_t sl, Int_t ch ) {
          crate = cr; slot = sl; chan = ch; header = ntoskip = search_choice = 0;
      }
      void SetHeader( Int_t cr, UInt_t hd, Int_t sk ) {
          crate = cr; header = hd; ntoskip = sk; slot = chan = 0; search_choice = 1;
      }
#ifdef NEW
      void SubRef( BBdataLoc *refloc ) 
      {
          if(refloc->isRefCh) {
#ifdef DEBUGNEW
              cout << "Doing Ref CH sub" << endl;
#endif
              Int_t wrap=65526;
              Int_t reftime = refloc->rdata[0];
#ifdef DEBUGNEW
              cout << "reftime = " << reftime << endl;
#endif
              for(Int_t idata=0; idata<ndata; idata++) {
#ifdef DEBUGNEW
                  cout << rdata[idata] << " ";
#endif
                  //Int_t orig = rdata[idata];
                  Int_t diff = rdata[idata] - reftime;
                  if( diff < -(wrap/2.) ) {
                      rdata[idata] = diff - wrap;
                  } else if ( diff > (wrap/2.) ) {
                      rdata[idata] = diff - wrap;
                  } else {
                      rdata[idata] = diff;
                  }
                  // 	if(abs(rdata[idata])>wrap) {
                  // 	  cout << "rdata = " << rdata[idata]
                  // 	       << "\torig = " << orig
                  // 	       << "\tref = " << reftime 
                  // 	       << "\tdiff = " << diff
                  // 	       << endl;
                  // 	}
#ifdef DEBUGNEW
                  cout << rdata[idata] << " ";
                  cout << diff << " ";
#endif
              }
#ifdef DEBUGNEW
              cout << endl << endl;
#endif
          } else {
              cout << "Passed pointer not a reference channel" << endl;
          }
      }
#endif
      ~BBdataLoc() {}

      Int_t  crate, slot, chan;   // where to look in crates
      UInt_t header;              // header (unique either in data or in crate)
      Int_t ntoskip;              // how far to skip beyond header

      Int_t rdata[MxHits];       //[ndata] raw data (to accom. multihit chanl)
      Int_t  ndata;               // number of relevant entries
private:
    Int_t  search_choice;       // whether to search in crates or rel. to header
    Bool_t loaded_once;
    BBdataLoc();
    BBdataLoc(const BBdataLoc& dataloc);
    BBdataLoc& operator=(const BBdataLoc& dataloc);
};

typedef vector<BBdataLoc*>::iterator Iter_t;

BBDecData* BBDecData::fgThis = NULL;  //Pointer to single instance of this class

//_____________________________________________________________________________
static UInt_t header_str_to_base16(const char* hdr) {
    // Utility to convert string header to base 16 integer
    const char chex[] = "0123456789abcdef";
    if( !hdr ) return 0;
    const char* p = hdr+strlen(hdr);
    UInt_t result = 0;  UInt_t power = 1;
    while( p-- != hdr ) {
        const char* q = strchr(chex,tolower(*p));
        if( q ) {
            result += (q-chex)*power; 
            power *= 16;
        }
    }
    return result;
};


//_____________________________________________________________________________
BBDecData::BBDecData( const char* name, const char* descript ) : 
THaApparatus( name, descript )
{
    if( fgThis ) {
        Error("BBDecData", "Only one instance of BBDecData allowed. "
            "Object creation failed.");
        MakeZombie();
        return;
    }
    Reset();

    fgThis = this;
}

//_____________________________________________________________________________
BBDecData::~BBDecData()
{

    // Dtor. Remove global variables.

    if( fgThis == this ) {
        SetupDecData( NULL, kDelete ); 
        fgThis = NULL;
    }
}

//_____________________________________________________________________________
void BBDecData::Clear( Option_t* opt ) 
{
    // Clear the object (set event-by-event data to zero)

    evtypebits = 0;
    evtype     = 0;
    ctimel     = 0;
    ctimer     = 0;
    pulser1    = 0;
    synchadc1  = 0;
    synchadc2  = 0;
    synchadc3  = 0;
    synchadc4  = 0;
    synchadc14 = 0;
    timestamp  = 0;
    timeroc1   = 0;
    timeroc2   = 0;
    timeroc3   = 0;
    timeroc4   = 0;
    timeroc14  = 0;
    rftime1    = 0;
    rftime2    = 0;
    edtpl      = 0;
    edtpr      = 0;
    lenroc12   = 0;
    lenroc16   = 0;
    for( Iter_t p = fWordLoc.begin();  p != fWordLoc.end(); p++)  (*p)->Clear();
    for( Iter_t p = fCrateLoc.begin(); p != fCrateLoc.end(); p++) (*p)->Clear();
}

//_____________________________________________________________________________
void BBDecData::Reset( Option_t* opt ) 
{
    // Reset the object (zero all data, including histograms)

    Clear();
}

//_____________________________________________________________________________
Int_t BBDecData::SetupDecData( const TDatime* run_time, EMode mode )
{
    // Register global variables, open decdata map file, and parse it.
    // If mode == kDelete, remove global variables.

    Int_t retval = 0;

    RVarDef vars[] = {
        { "evtypebits", "event type bit pattern",      "evtypebits" },  
        { "evtype",     "event type from bit pattern", "evtype" },  
        { "ctimel",     "coincidence time on L-arm",   "ctimel" },  
        { "ctimer",     "coincidence time on R-arm",   "ctimer" },  
        { "pulser1",    "pulser in a TDC",             "pulser1" },  
        { "synchadc1",  "synch check adc 1",           "synchadc1" },       
        { "synchadc2",  "synch check adc 2",           "synchadc2" },       
        { "synchadc3",  "synch check adc 3",           "synchadc3" },       
        { "synchadc4",  "synch check adc 4",           "synchadc4" },       
        { "synchadc14", "synch check adc 14",          "synchadc14" },
        { "times100k",  "100kHz time stamp",           "timestamp" },       
        { "timeroc1",   "time stamp roc 1",            "timeroc1" },         
        { "timeroc2",   "time stamp roc 2",            "timeroc2" },         
        { "timeroc3",   "time stamp roc 3",            "timeroc3" },         
        { "timeroc4",   "time stamp roc 4",            "timeroc4" },         
        { "timeroc14",  "time stamp roc 14",           "timeroc14" },         
        { "rftime1",    "RF time copy 1",              "rftime1" },         
        { "rftime2",    "RF time copy 2",              "rftime2" },         
        { "edtpl",      "EDT pulser on L-arm",         "edtpl" },   
        { "edtpr",      "EDT pulser on R-arm",         "edtpr" },   
        { "lenroc12",   "ROC12 event length",         "lenroc12" },   
        { "lenroc16",   "ROC16 event length",         "lenroc16" },   
        { 0 }
    };

    Bool_t re_init = ((mode == kDefine) && fIsSetup);
    if( !re_init )
        retval = DefineVarsFromList( vars, mode );

    fIsSetup = ( mode == kDefine );

    if( mode != kDefine ) {
        // Undefine the dynamically-defined global variables
        for( Iter_t p = fCrateLoc.begin(); p != fCrateLoc.end(); p++ ) {
            DefineChannel(*p,mode);
            if( mode == kDelete )
                delete *p;
        }
        for( Iter_t p = fWordLoc.begin();  p != fWordLoc.end(); p++ ) {
            DefineChannel(*p,mode);
            if( mode == kDelete )
                delete *p;
        }
        if( mode == kDelete ) {
            fCrateLoc.clear();   
            fWordLoc.clear(); 
        }

    } else {

        // Set up the locations in raw data corresponding to variables of this class. 
        // Each element of a BBdataLoc is one channel or word.  Since a channel 
        // may be a multihit device, the BBdataLoc stores data in a vector.

        if( !re_init ) {
            fCrateLoc.clear();   
            fWordLoc.clear(); 
        }  

        ifstream decdatafile;

        const char* const here = "SetupDecData()";
        const char* name = GetDBFileName();

        TDatime date;
        if( run_time ) date = *run_time;
        vector<string> fnames = GetDBFileList( name, date, Here(here));
        // always look for 'decdata.map' in the current directory first.
        fnames.insert(fnames.begin(),string("decdata.map"));
        if( !fnames.empty() ) {
            vector<string>::iterator it = fnames.begin();
            do {
                if( fDebug>0 ) {
                    cout << "<" << IsA()->GetName() << "::" << Here(here)
                        << ">: Opening database file " << *it;
                }
                decdatafile.clear();  // Forget previous failures before attempt
                decdatafile.open((*it).c_str());

                if( fDebug>0 ) 
                    if( !decdatafile ) cout << " ... failed" << endl;
                    else               cout << " ... ok" << endl;
            } while ( !decdatafile && ++it != fnames.end() );
        }
        if( fnames.empty() || !decdatafile ) {
            if( !re_init ) {
                if( fDebug>0 )
                    Warning( Here(here), "File db_%s.dat not found.\nAn example of this "
                    "file should be in the examples directory.\nWill proceed "
                    "with default mapping for BBDecData.", name );
                return DefaultMap();
            } else {
                if( fDebug>0 )
                    Warning( Here(here), "File db_%s.dat not found for timestamp %s.\n"
                    "Variable definitions unchanged from prior initialization.\n"
                    "Update database to be sure you have valid data.",
                    name, date.AsString() );
                return retval;
            }
        }

        string sinput;
        const string comment = "#";
        while (getline(decdatafile, sinput)) {
            if( fDebug>3 )
                Info( Here(here), "sinput = %s", sinput.c_str() );
            vector<string> strvect( vsplit(sinput) );
            if (strvect.size() < 5 || strvect[0] == comment) continue;
            bool found = false;
            RVarDef* pdef = vars;
            while( pdef->name && !found ) {
                if( strvect[0] == pdef->name ) { 
                    found = true; break;
                }
                pdef++;
            }
            Int_t slot(0), chan(0), skip(0);
            UInt_t header(0);
            Int_t crate = (Int_t)atoi(strvect[2].c_str());  // crate #
            bool is_slot = (strvect[1] == "crate");
#ifdef NEW
            bool is_ref =  (strvect[1] == "refchan");
            bool needs_ref = (strvect[1] == "needsref");
            if ( is_ref ) is_slot = true;
            if ( needs_ref ) is_slot = true;
#endif
            BBdataLoc* b;
            if( is_slot ) {  // Crate data ?
                slot = (Int_t)atoi(strvect[3].c_str());
                chan = (Int_t)atoi(strvect[4].c_str());
                b = new BBdataLoc(strvect[0].c_str(), crate, slot, chan);
#ifdef NEW
                if( is_ref )
                    b->isRefCh = true;
                if( needs_ref ) {
                    b->needsRefChSub = true;
#ifdef DEBUGNEW
                    cout << "FOUND A REF SUB CH!" << endl;
#endif
                }
#endif
            } else {         // Data is relative to a header
                header = header_str_to_base16(strvect[3].c_str());
                skip = (Int_t)atoi(strvect[4].c_str());
                b = new BBdataLoc(strvect[0].c_str(), crate, header, skip);
            }

            bool already_defined = false;
            if( re_init ) {
                // When reinitializing, use simple logic, assuming things didn't 
                // change much:
                // - if the name exists, update it with the chan/slot read from the 
                //   database for the new time
                // - if the name is new, add it - leave it up to the user to decide
                //   whether this is sensible
                // - if a name disappears, things are ambiguous. Just leave it as 
                //   it is, although something is probably wrong with the database
                //
                TString bname = b->GetName();
                Iter_t p;
                for( p = fWordLoc.begin();  p != fWordLoc.end(); p++ ) {
                    if( bname == (*p)->GetName() ) {
                        already_defined = true;
                        break;
                    }
                }
                if( !already_defined ) {
                    for( p = fCrateLoc.begin(); p != fCrateLoc.end(); p++ ) {
                        if( bname == (*p)->GetName() ) {
                            already_defined = true;
                            break;
                        }
                    }
                }

                if( already_defined ) {
                    if ( **p != *b ) {
                        if( fDebug>2 ) 
                            Info( Here(here), 
                            "Updating variable %s", (*p)->GetName() );
                        //FIXME:  Ouch! If we change from slot to header or vice versa, we
                        // must move the object from fCrateLoc to fWordLoc or vice versa...
                        if( is_slot ) {
                            (*p)->SetSlot( crate, slot, chan );
#ifdef NEW
                            if ( is_ref )
                                (*p)->isRefCh = true;
                            if ( needs_ref )
                                (*p)->needsRefChSub = true;
#endif
                        } else
                            (*p)->SetHeader( crate, header, skip );
                    } else {
                        if( fDebug>2 )
                            Info( Here(here),
                            "Variable %s already defined and not changed",
                            (*p)->GetName() );
                    }
                }
            }

            if( !already_defined ) {
                if( found && fDebug>2 ) 
                    Info( Here(here), "Defining standard variable %s", 
                    b->GetName() );
                else if( !found && fDebug>2 ) 
                    // !found is ok, but might be a typo error too, so I print to warn you.
                    Info( Here(here), 
                    "New variable %s will become global", b->GetName() );

                if( is_slot ) {
                    fCrateLoc.push_back(b);
                } else {
                    fWordLoc.push_back(b);
                }

                if (!found) {
                    // if not one of the pre-defined global variables, make a new global
                    // variable for this BBdataLoc
                    DefineChannel(b,mode);
                }
            }
        }
    }

    return retval;
}

//_____________________________________________________________________________
BBdataLoc* BBDecData::DefineChannel(BBdataLoc *b, EMode mode, const char* desc)
{
    if( gHaVars ) {
        string nm(fPrefix); nm += b->GetName();
        if (mode==kDefine)
            gHaVars->Define(nm.c_str(),desc,b->rdata[0],&(b->ndata));
        else if (mode==kDelete) {
            gHaVars->RemoveName(nm.c_str());
            b = NULL;
        }
    }
    return b;
}

//_____________________________________________________________________________
Int_t BBDecData::End( THaRunBase* run ) 
{
    return 0;
}


//_____________________________________________________________________________
THaAnalysisObject::EStatus BBDecData::Init( const TDatime& run_time ) 
{
    // Custom Init() method. Since this apparatus has no detectors, we
    // skip the detector initialization.

    fStatus = kNotinit;
    if( IsZombie() )
        return fStatus;

    MakePrefix();
    cnt1 = 0;
    return fStatus = static_cast<EStatus>( SetupDecData( &run_time ) );
}

//_____________________________________________________________________________


Int_t BBDecData::DefaultMap() {
    // Default setup of mapping of data in this class to locations in the raw data.
    // This is valid for a particular time.  If you have 'decdata.map' in your
    // pwd, the code would use that instead.  See /examples directory for an
    // example of decdata.map

    // ADCs that show data synch.
    Int_t crate = 1, slot = 25, chan = 16;   
    fCrateLoc.push_back(new BBdataLoc("synchadc1", crate, slot, chan));
    fCrateLoc.push_back(new BBdataLoc("synchadc2", 2, (Int_t) 24, 48));
    fCrateLoc.push_back(new BBdataLoc("synchadc3", 3, (Int_t) 22, 0));
    fCrateLoc.push_back(new BBdataLoc("synchadc4", 4, (Int_t) 17, 48));
    fCrateLoc.push_back(new BBdataLoc("synchadc14", 14, (Int_t) 1, 5));

    // Coincidence time, etc
    fCrateLoc.push_back(new BBdataLoc("ctimel", 4, (Int_t) 21, 48));
    fCrateLoc.push_back(new BBdataLoc("ctimer", 2, (Int_t) 16, 32));
    fCrateLoc.push_back(new BBdataLoc("pulser1", 3, (Int_t) 3, 7));

    // 100 kHz time stamp in roc14, at 2 words beyond header=0xfca56000
    fCrateLoc.push_back(new BBdataLoc("timestamp", 14, (UInt_t)0xfca56000, 2)); 

    // vxWorks time stamps
    UInt_t header = 0xfabc0004;
    Int_t ntoskip = 4;
    crate = 1;
    fWordLoc.push_back(new BBdataLoc("timeroc1", crate, header, ntoskip));
    fWordLoc.push_back(new BBdataLoc("timeroc2", 2, (UInt_t)0xfabc0004, 4));
    fWordLoc.push_back(new BBdataLoc("timeroc3", 3, (UInt_t)0xfabc0004, 4));
    fWordLoc.push_back(new BBdataLoc("timeroc4", 4, (UInt_t)0xfabc0004, 4));
    fWordLoc.push_back(new BBdataLoc("timeroc14", 14, (UInt_t)0xfadcb0b4, 1));

    // RF time
    fCrateLoc.push_back(new BBdataLoc("rftime1", 2, (Int_t) 16, 50));
    fCrateLoc.push_back(new BBdataLoc("rftime2", 2, (Int_t) 16, 51));

    // EDTM pulser
    fCrateLoc.push_back(new BBdataLoc("edtpl", 3, (Int_t) 9, 81));
    fCrateLoc.push_back(new BBdataLoc("edtpr", 2, (Int_t) 12, 48));

    // Bit pattern for trigger definition

    for (UInt_t i = 0; i < bits.GetNbits(); i++) {
        fCrateLoc.push_back(new BBdataLoc(Form("bit%d",i+1), 3, (Int_t) 5, 64+i));
    }

    // Anything else you want here...

    return 0;
}


//_____________________________________________________________________________
Int_t BBDecData::Decode(const THaEvData& evdata)
{
    // Extract the requested variables from the event data

    if( !IsOK() )
        return -1;

    Clear();

    lenroc12 = evdata.GetRocLength(12);
    lenroc16 = evdata.GetRocLength(16);

    // For each raw data registerd in fCrateLoc, get the data if it belongs to a 
    // combination (crate, slot, chan).

#ifdef NEW
    BBdataLoc *refloc=0;
#endif

    for( Iter_t p = fCrateLoc.begin(); p != fCrateLoc.end(); p++) {
        BBdataLoc *dataloc = *p;
        if ( dataloc->IsSlot() ) {  
            for (Int_t i = 0; i < evdata.GetNumHits(dataloc->crate, 
                dataloc->slot, dataloc->chan); i++) {
                    dataloc->Load(evdata.GetData(dataloc->crate, dataloc->slot, 
                        dataloc->chan, i));
            }
        }
#ifdef NEW
        if(dataloc->isRefCh) {
            refloc=dataloc;
        }
#endif
    }

    // Loop again and subtract the reference from those that need it
#ifdef NEW
    if(refloc->ndata!=0) {
        for( Iter_t p = fCrateLoc.begin(); p != fCrateLoc.end(); p++) {
            BBdataLoc *dataloc = *p;
            if ( dataloc->needsRefChSub && dataloc->ndata!=0) {  
                dataloc->SubRef(refloc);
            }
        }
    } 
#ifdef DEBUGNEW
    else {
      cout << "No reference channel for this event" << endl;
    }
#endif
#endif

    // Crawl through the event and decode elements of fWordLoc which are defined
    // as relative to a header.   fWordLoc are treated seperately from fCrateLoc
    // for performance reasons; this loop could be slow !

    //FIXME: This can be made faster if each header is followed by the offset
    // to the next header. Is it?
    for (Iter_t p = fWordLoc.begin(); p != fWordLoc.end(); p++) {
        BBdataLoc *dataloc = *p;
        for( Int_t i = 0; i < evdata.GetRocLength(dataloc->crate); i++ ) {
            if( static_cast<UInt_t>( evdata.GetRawData(dataloc->crate,i) )
                == dataloc->header)
                dataloc->Load(evdata.GetRawData(dataloc->crate, i + dataloc->ntoskip));
        }
    }

    evtype = evdata.GetEvType();   // CODA event type 

    for( Iter_t p = fCrateLoc.begin(); p != fCrateLoc.end(); p++) {
        BBdataLoc *dataloc = *p;
        bool found = false;

        // bit pattern of triggers

        // FIXME: dozens of string comparisons for every event?!?
        // should store pointer to member data with the corresponding BBdataLoc object
        // otherwise use a hash list
        for (UInt_t i = 0; i < bits.GetNbits(); i++) {
            if ( dataloc->ThisIs(Form("bit%d",i+1)) ) {
                TrigBits(i+1,dataloc);
                found = true;
                break;
            }
        }
        if( found ) continue;

        // synch ADCs
        if ( dataloc->ThisIs("synchadc1") ) synchadc1  = dataloc->Get();
        else if ( dataloc->ThisIs("synchadc2") ) synchadc2  = dataloc->Get();
        else if ( dataloc->ThisIs("synchadc3") ) synchadc3  = dataloc->Get();
        else if ( dataloc->ThisIs("synchadc4") ) synchadc4  = dataloc->Get();
        else if ( dataloc->ThisIs("synchadc14")) synchadc14 = dataloc->Get();

        // coincidence times
        else if ( dataloc->ThisIs("ctimel")) ctimel = dataloc->Get();
        else if ( dataloc->ThisIs("ctimer")) ctimer = dataloc->Get();

        // RF time
        else if ( dataloc->ThisIs("rftime1") ) rftime1  = dataloc->Get();
        else if ( dataloc->ThisIs("rftime2") ) rftime2  = dataloc->Get();

        // EDTM pulser
        else if ( dataloc->ThisIs("edtpl") ) edtpl  = dataloc->Get();
        else if ( dataloc->ThisIs("edtpr") ) edtpr  = dataloc->Get();

    }

    for (Iter_t p = fWordLoc.begin(); p != fWordLoc.end(); p++) {
        BBdataLoc *dataloc = *p;

        // time stamps
        if ( dataloc->ThisIs("timestamp")) timestamp = dataloc->Get();
        else if ( dataloc->ThisIs("timeroc1") ) timeroc1  = dataloc->Get();
        else if ( dataloc->ThisIs("timeroc2") ) timeroc2  = dataloc->Get();
        else if ( dataloc->ThisIs("timeroc3") ) timeroc3  = dataloc->Get();
        else if ( dataloc->ThisIs("timeroc4") ) timeroc4  = dataloc->Get();
        else if ( dataloc->ThisIs("timeroc14")) timeroc14 = dataloc->Get();

    }

    // debug 
    //    Print();

    return 0;
}


//_____________________________________________________________________________
void BBDecData::Print( Option_t* opt ) const {
    // Dump the data for purpose of debugging.
    cout << "Dump of BBDecData "<<endl;
    cout << "event pattern bits : ";
    for (UInt_t i = 0; i < bits.GetNbits(); i++) 
        cout << " "<<i<<" = "<< bits.TestBitNumber(i)<<"  | ";
    cout << endl;
    cout << "event types,  CODA = "<<evtype<<"   bit pattern = "<<evtypebits<<endl;
    cout << "synch adcs "<<"  "<<synchadc1<<"  "<<synchadc2<<"  ";
    cout << synchadc3 <<"  "<<synchadc4<<"   "<<synchadc14<<endl;
    cout <<" time stamps "<<timestamp<<"  "<<timeroc1<<"  "<<timeroc2<<"  ";
    cout << timeroc3<<"  "<<timeroc4<<"  "<<timeroc14<<endl<<endl;
    cout << "RF timing "<<rftime1<<"  "<<rftime2<<endl;
    cout << "EDTM pulser "<<edtpl<<"  "<<edtpr<<endl;
    cout << endl;
    cout << "Crate-type variables: " << fCrateLoc.size() << endl;
    cout << "Word-type variables: " << fWordLoc.size() << endl;
}


//_____________________________________________________________________________
void BBDecData::TrigBits(UInt_t ibit, BBdataLoc *dataloc) {
    // Figure out which triggers got a hit.  These are multihit TDCs, so we
    // need to sort out which hit we want to take by applying cuts.

    if( ibit >= kBitsPerByte*sizeof(UInt_t) ) return; //Limit of evtypebits
    bits.ResetBitNumber(ibit);

    const UInt_t cutlo = 200;
    const UInt_t cuthi = 1500;

    for (int ihit = 0; ihit < dataloc->NumHits(); ihit++) {
        if (dataloc->Get(ihit) > cutlo && dataloc->Get(ihit) < cuthi) {
            bits.SetBitNumber(ibit);
            evtypebits |= BIT(ibit);
        }
    }

}
//_____________________________________________________________________________
ClassImp(BBDecData)

