///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// THaVDC2DTableTTDConv                                                        //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>
#include "THaVDC2DTableTTDConv.h"
#include "TMath.h"

using namespace std;

THaVDC2DTableTTDConv::THaVDC2DTableTTDConv()
{
}

THaVDC2DTableTTDConv::THaVDC2DTableTTDConv( const char *file_name )
{
  LoadTable( file_name );
}


void THaVDC2DTableTTDConv::LoadTable( const char *file_name )
{
  Int_t counterx;
  Int_t countery;

  x0 = -1;
  y0 = -1;

  ifstream tablefile;
  tablefile.open( file_name );

  tablefile >> fDriftVelocity;
  tablefile >> Nx;
  tablefile >> Ny;
  
  if( Nx > MAX_X_GRID ) 
    {
      clog << "Too many x points.  Cutting to " <<  MAX_X_GRID << endl; 
      Nx = MAX_X_GRID;
    }
  if( Ny > MAX_Y_GRID )
    { 
      clog << "Too many y points.  Cutting to " <<  MAX_Y_GRID << endl;
      Ny = MAX_Y_GRID;
    }

  for( counterx = 0; counterx < Nx; counterx++ )
    {
      tablefile >> TableX[counterx];
      if( TableX[counterx] > 0 &&  x0 == -1 ) x0 = counterx-1;
    }
  

  for( countery = 0; countery < Ny; countery++ )
    {
      tablefile >> TableY[countery];
      if( TableY[countery] > 0 &&  y0 == -1 )
	{
	  y0 = countery-1;
	}
      
      for( counterx = 0; counterx < Nx; counterx++ )
	{
	  tablefile >> TableT[counterx][countery];
	  //clog << counterx << " " << countery << ": " << TableT[counterx][countery] << endl;
	}
    }

  tablefile.close();

  // This is somewhat of a kludge...   Garfield tables seem to like to put 0s at the extreme borders, so we'll just make sure we don't read them in,
  // It's just a few less datapoints regardless

  Nx--;
  Ny--;

}

void THaVDC2DTableTTDConv::PrintTable()
{ 
  Int_t counterx;
  Int_t countery; 
  clog << "Printing data" << endl;

  ofstream fout;
  fout.open("temp.dat", ios::out );

  fout << "\t";
  
  for( counterx = 0; counterx < Nx; counterx++ )
    {
      fout << TableX[counterx] << "\t";
    }
  fout << endl;

  for( countery = 0; countery < Ny; countery++ )
    {
      fout << TableY[countery] << "\t";
      for( counterx = 0; counterx < Nx; counterx++ )
	{
	  fout <<  TableT[counterx][countery] << "\t";
	}
      fout << endl;
    }
  fout << endl;
  fout.close();
  clog << "Done printing table" << endl;
}



Double_t THaVDC2DTableTTDConv::ConvertTimeToDist( Double_t time, Double_t tanTheta, Double_t *drift_dist )
{

  Int_t counterx = x0;
  Int_t countery = y0;

  Double_t x1 = 0.0;
  Double_t y1 = 0.0;
  Double_t t1 = 0.0;
  Double_t x2 = 0.0;
  Double_t y2 = 0.0;
  Double_t t2 = 0.0;;
  Double_t x, y;

  Double_t invdtdy = 0.0, invdtdx = 0.0;

  Double_t planeintercept;

  Bool_t debug = false;

  tanTheta = TMath::Abs(tanTheta);

  if( debug ) clog << "Slope " << tanTheta << endl; 

  if( debug ) clog << "x0 " << x0 <<  " y0 " << y0 << endl;
  
  while( (t2 < time) && (counterx < Nx-1) && (countery < Ny-1) )
    { 
      if( debug ) clog << "Cell: " <<  TableX[counterx] << ", " << TableY[countery] << endl;

      // Work out far point
      if( (( TableY[countery+1] / TableX[counterx+1] ) > tanTheta) || ( tanTheta < 1e-6) )
	{
	  x2 = TableX[counterx+1];
	  y2 = x2 * tanTheta; 
	  
	  t2 = ( ( TableT[counterx+1][countery+1] - TableT[counterx+1][countery] ) / ( TableY[countery+1] - TableY[countery] ) ) *  ( y2 - TableY[countery] ) + TableT[counterx+1][countery]; 
	  if( debug ) clog << "\tFar end: " << ( TableY[countery+1] / TableX[counterx+1] ) << endl;
	  if( debug ) clog << "\tTime2: " << t2 << endl;
	}
      else
	{
	  y2 = TableY[countery+1];
	  x2 = y2 / tanTheta;

	  t2 = ( ( TableT[counterx+1][countery+1] - TableT[counterx][countery+1] )/ (TableX[counterx+1] - TableX[counterx] ) ) * ( x2 - TableX[counterx] ) + TableT[counterx][countery+1];
	  if( debug ) clog << "\tFar end: " << ( TableY[countery+1] / TableX[counterx+1] ) << endl;
	  if( debug ) clog << "\tTime2: " << t2 << endl;
	}

      // Work out near point

      if( TableX[counterx] < 1e-6 )
	{
	  x1 = TableX[counterx];
	  y1 = TableY[countery];
	  t1 = TableT[counterx][countery];

	  if( debug ) clog << "\tNear end: " << ( TableY[countery] / TableX[counterx] ) << endl;
	  if( debug ) clog << "\tTime1: " << t1 << endl;
	}
      else
	{
	  if( (( TableY[countery] / TableX[counterx] ) > tanTheta ) && ( tanTheta > 1e-6) )
	    { 
	      y1 = TableY[countery];
	      x1 = y1 / tanTheta;  
	      
	      t1 = ( ( TableT[counterx+1][countery] - TableT[counterx][countery] ) / ( TableX[counterx+1] - TableX[counterx] ) ) * ( x1 - TableX[counterx] ) + TableT[counterx][countery];
	      if( debug ) clog << "\tNear end: " << ( TableY[countery] / TableX[counterx] ) << endl;
	      if( debug ) clog << "\tTime1: " << t1 << endl;
	    }
	  else
	    {
	      
	      x1 = TableX[counterx];
	      y1 = x1 * tanTheta;
	      
	      t1 = ( ( TableT[counterx][countery+1] - TableT[counterx][countery] ) / (TableY[countery+1] - TableY[countery] ) ) * ( y1 - TableY[countery] ) +  TableT[counterx][countery];
	      if( debug ) clog << "\tNear end: " << ( TableY[countery] / TableX[counterx] ) << endl;
	      if( debug ) clog << "\tTime1: " << t1 << endl;
	    }
	}
      invdtdx = ( x2-x1 )/( t2-t1 );
      invdtdy = ( y2-y1 )/( t2-t1 );

      //      if( t1 > t2 ) clog << "Near time = " << t1 << " Far time = " << t2 << endl;


      if( (TableY[countery+1] / TableX[counterx+1]) < tanTheta )
	{
	  countery++;
	  if( debug ) clog << "Moving up" << endl;
	}
      else
	{
	  counterx++;
	  if( debug ) clog << "Moving right" << endl;
	}
    }

  Double_t DriftVelocity;


  if( ((counterx >= Nx-1 ) || (countery >= Ny-1 )) && (t2<time))
    {
      if( (counterx >=  Nx-1) && (tanTheta < 1e-6) )
	{
	  x1 = TableX[Nx-1];
	  y1 = TableY[countery];
	  t1 = TableT[Nx-1][countery];

	  if( debug ) clog << "\tNear end: " << ( TableY[countery] / TableX[counterx] ) << endl;
	  if( debug ) clog << "\tTime1: " << t1 << endl;
	}
      else
	{
	  if( countery >= Ny-1  )
	    { 
	      y1 = TableY[Ny-1];
	      x1 = y1 / tanTheta;  
	      
	      t1 = ( ( TableT[counterx+1][countery] - TableT[counterx][countery] ) / ( TableX[counterx+1] - TableX[counterx] ) ) * ( x1 - TableX[counterx] ) + TableT[counterx][countery];
	      if( debug ) clog << "\tNear end: " << ( TableY[countery] / TableX[counterx] ) << endl;
	      if( debug ) clog << "\tTime1: " << t1 << endl;
	    }
	  else
	    {	      
	      x1 = TableX[counterx];
	      y1 = x1 * tanTheta;
	      
	      t1 = ( ( TableT[counterx][countery+1] - TableT[counterx][countery] ) / (TableY[countery+1] - TableY[countery] ) ) * ( y1 - TableY[countery] ) +  TableT[counterx][countery];
	      if( debug ) clog << "\tNear end: " << ( TableY[countery] / TableX[counterx] ) << endl;
	      if( debug ) clog << "\tTime1: " << t1 << endl;
	    }
	}
   

      DriftVelocity = TMath::Sqrt( x1*x1 + y1*y1 ) / t1;
     
      if(debug) clog << "Fell of the grid...  Resorting to a constant drift velocity of " << DriftVelocity << endl;
      //       clog << "Fell of the grid...  Resorting to a constant drift velocity of " << DriftVelocity << endl;
      // clog << TMath::Sqrt( x1*x1 + y1*y1 ) << "/" << t1 << endl;



      x = ( time * DriftVelocity ) *  TMath::Cos( TMath::ATan( tanTheta ) );
      y = ( time * DriftVelocity ) *  TMath::Sin( TMath::ATan( tanTheta ) );
      planeintercept = time * DriftVelocity / TMath::Cos( TMath::ATan( tanTheta ) );
    }
  else
    { 
      if( debug ) clog << time << " is between " << t1 << " and " << t2 << endl;
      x = (( time - t1 ) * invdtdx) + x1;
      y = (( time - t1 ) * invdtdy) + y1;

      planeintercept = TMath::Sqrt( TMath::Power( x, 2.0 ) + TMath::Power( y, 2.0 ) ) /  TMath::Cos( TMath::ATan( tanTheta ) );      
    }

  if( drift_dist != NULL ) *drift_dist = planeintercept;
  
  if( debug ) clog << "Drift time of " << time << " and slope " << tanTheta << " coverts to (x,y): ( " << x  << " , " << y << " ) and distance " << planeintercept << " from wire" <<  endl;

  return planeintercept;
}

Double_t THaVDC2DTableTTDConv::ConvertDistToTime( Double_t plane_intercept, Double_t slope )
{
  Double_t x, y, dtdx, dtdy, dx, dy;

  Int_t counter, xcounter, ycounter;

  // clog << "Calculating x and y" << endl;

  x = TMath::Abs(plane_intercept) * TMath::Cos( TMath::ATan( TMath::Abs(slope) ) ) * TMath::Cos( TMath::ATan( TMath::Abs(slope) ) );
  y = TMath::Abs(plane_intercept) * TMath::Cos( TMath::ATan(  TMath::Abs(slope) ) ) * TMath::Sin( TMath::ATan( TMath::Abs(slope) ) );

  //clog <<  "Found x and y.  Now finding the corresponding place in the x array " << endl;

  counter = 0;
  while( ( x > TableX[counter] ) && ( counter < MAX_X_GRID ) ) counter++;
  
  //clog << "X counter = " << counter <<  endl << x << " is inbetween " << TableX[counter-1] << " and " << TableX[counter] << endl;
  xcounter = counter;

  counter = 0;
  while( ( y > TableY[counter] ) && ( counter < MAX_Y_GRID ) ) counter++;
  ycounter = counter;

  if( xcounter >= MAX_X_GRID )
    {
      //   clog << "Off the table" << endl;
      return TMath::Sqrt( (x-TableX[MAX_X_GRID-1])*(x-TableX[MAX_X_GRID-1]) + (y- TableY[ycounter])*(y- TableY[ycounter]) ) / ( TMath::Sqrt( TableY[ycounter]* TableY[ycounter] +  TableX[MAX_X_GRID-1] *TableX[MAX_X_GRID-1] )  / TableT[MAX_X_GRID-1][ycounter] ) + TableT[MAX_X_GRID-1][ycounter];
    }


  if( ycounter >= MAX_Y_GRID )
    {
      //    clog << "Off the table" << endl;
      return TMath::Sqrt( (x-TableX[xcounter] )*(x-TableX[xcounter] ) + (y-TableY[MAX_Y_GRID-1] )*(y-TableY[MAX_Y_GRID-1]) ) / ( TMath::Sqrt( TableY[MAX_Y_GRID-1]* TableY[MAX_Y_GRID-1] +  TableX[xcounter] *TableX[xcounter] )  / TableT[xcounter][MAX_Y_GRID-1] ) + TableT[xcounter][MAX_Y_GRID-1] ;
    }
  
  //clog << "Y counter = " << counter << endl << y << " is inbetween " << TableY[counter-1] << " and " << TableY[counter] << endl;
 
  //clog << "Table T: " << endl << TableT[xcounter-1][ycounter] << "\t" << TableT[xcounter][ycounter] << endl << TableT[xcounter-1][ycounter-1] << "\t" << TableT[xcounter][ycounter-1] << endl;

  dtdx = ( TableT[xcounter][ycounter-1] - TableT[xcounter-1][ycounter-1] )  /( TableX[xcounter] - TableX[xcounter-1] );
  dtdy = ( TableT[xcounter-1][ycounter] - TableT[xcounter-1][ycounter-1] )  /( TableY[ycounter] - TableY[ycounter-1] );

  //clog << " dt/dx: " <<  dtdx << "   dt/dy: " << dtdy << endl;
  
  dx = x - TableX[xcounter-1];
  dy = y - TableY[ycounter-1];

  return TableT[xcounter-1][ycounter-1] + dtdx * dx + dtdy * dy;

}



THaVDC2DTableTTDConv::~THaVDC2DTableTTDConv()
{
}

ClassImp(THaVDC2DTableTTDConv)
