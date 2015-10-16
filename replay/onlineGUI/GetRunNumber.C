// GetRunNumber.C
//
// Helper macro for OnlineGUI to determine runnumber
// Edited by David Flay on September 17th, 2009
// 
#define USEPODD
#ifdef USEPODD
//#include "/opt/analyzer-1.5/src/src/THaRun.h"
#include "/home/dflay/d2n_analysis/analyzer/src/THaRun.h"
#endif

UInt_t GetRunNumber()
{

#ifdef USEPODD
  // This is specific to the Hall-A C++/ROOT Analyzer (PODD)
  //  Must use "analyzer" to obtain this info.

  THaRun* runinfo = (THaRun*)gROOT->FindObject("Run_Data");
  if(runinfo==NULL) return 0;
  return runinfo->GetNumber();
#else
  return 0;
#endif


}
