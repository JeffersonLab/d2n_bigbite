///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// THaVDCTableTTDConv                                                        //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>
#include "THaVDCTableTTDConv.h"
#include "TMath.h"

using namespace std;

THaVDCTableTTDConv::THaVDCTableTTDConv()
{
}

THaVDCTableTTDConv::THaVDCTableTTDConv( const char *file_name )
{
  LoadTable( file_name );
}


void THaVDCTableTTDConv::LoadTable( const char *file_name )
{
  Double_t time;
  Double_t dist;

  ifstream tablefile;
  tablefile.open( file_name );

  tablefile >> fDriftVelocity;
  
  while( !tablefile.eof() )
    {
      tablefile >> time;
      tablefile >> dist;
      fTimes.push_back(time);
      fDistances.push_back(dist);
    }

  tablefile.close();
}

Double_t THaVDCTableTTDConv::ConvertTimeToDist( Double_t time, Double_t tanTheta, Double_t *drift_dist )
{
 
  UInt_t counter;
  counter = 0;
  while( ( fTimes[counter] < time  ) && ( counter < fTimes.size()) )
    { 
      counter++;
    }

  if( counter == fTimes.size() )
    {
      if( drift_dist != 0 )
	{
	  *drift_dist = time * fDriftVelocity / ( TMath::Cos( TMath::ATan( tanTheta ) ) );
	}
      return time * fDriftVelocity / ( TMath::Cos( TMath::ATan( tanTheta ) ) );
    }
  else
    {
      if( counter == 0 ) return fDistances[counter];
      else
	{

	  if( drift_dist != 0 )
	    {
	      //*drift_dist = (( ( (fDistances[counter] - fDistances[counter-1] ) / ( fTimes[counter] - fTimes[counter-1] ) ) * (time - fTimes[counter-1] ) ) + fDistances[counter-1]);
		  	      *drift_dist = (( ( (fDistances[counter] - fDistances[counter-1] ) / ( fTimes[counter] - fTimes[counter-1] ) ) * (time - fTimes[counter-1] ) ) + fDistances[counter-1]) / ( TMath::Cos( TMath::ATan( tanTheta ) ) );
	    }
	  return (( ( (fDistances[counter] - fDistances[counter-1] ) / ( fTimes[counter] - fTimes[counter-1] ) ) * (time - fTimes[counter-1] ) ) + fDistances[counter-1]) / ( TMath::Cos( TMath::ATan( tanTheta ) ) );
	  //  return (( ( (fDistances[counter] - fDistances[counter-1] ) / ( fTimes[counter] - fTimes[counter-1] ) ) * (time - fTimes[counter-1] ) ) + fDistances[counter-1]);
	}
    }
}


THaVDCTableTTDConv::~THaVDCTableTTDConv()
{
}

ClassImp(THaVDCTableTTDConv)
