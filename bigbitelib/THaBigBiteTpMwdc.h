//////////////////////////////////////////////////////////////////////////
//
//	THaBigBiteTpMwdc
//	
//////////////////////////////////////////////////////////////////////////
//	
// Author: Jin Huang <mailto:jinhuang@jlab.org>    July 2007
//	Modify History:
//
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_THaBigBiteTpMwdc
#define ROOT_THaBigBiteTpMwdc

#include "THaBigBite.h"

//------------------------------------------------------//
//
//	Debug Definitions
//	place this section below any other head files
//
//------------------------------------------------------//
//
#ifdef DEBUG_LEVEL
#   undef DEBUG_LEVEL
#endif
//
//	DEBUG_LEVEL;	
//	=0	or not define: no debug, full speed 
//	>=1	enable debug extra warning (suggested setting)
//	>=2	above + enable debug assert
//	>=3	above + enable debug extra info
//	>=4	above + massive info (in a for or while)
#define DEBUG_LEVEL   2

#include    "DebugDef.h"
//------------------------------------------------------//

# if USE_AGEN_MWDC
class THaMWDC;
#else
namespace TreeSearch {
    class MWDC;
}
#endif //#if USE_AGEN_MWDC

class THaTriggerPlane;

class THaBigBiteTpMwdc :
    public THaBigBite
{
public:
    THaBigBiteTpMwdc(const char* name, const char* description);
    virtual ~THaBigBiteTpMwdc(void);

#if DEBUG_LEVEL>1 
    virtual Int_t   DebugPort(void *p);
#endif

protected:

#if USE_AGEN_MWDC
    THaMWDC*			fMWDC;	//Pointer to drift Chamber detector
#else //#if USE_AGEN_MWDC
    TreeSearch::MWDC*	fMWDC;	//Pointer to drift Chamber detector
#endif //#if USE_AGEN_MWDC

    THaTriggerPlane*	fTriggerPlane;	//Pointer to trigger plane

private:
    ClassDef(THaBigBiteTpMwdc,0) //BigBite class for Big Family E04-007 & E05-102
};

#endif

