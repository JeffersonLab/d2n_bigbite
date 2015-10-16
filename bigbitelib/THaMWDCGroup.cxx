#include "THaMWDCGroup.h"
#include "THaMWDCHit.h" 
#include "TMath.h"

using namespace std;

//_____________________________________________________________________________

// Constructors and destructors

// Constructor for vector of hits
THaMWDCGroup::THaMWDCGroup(vector<THaMWDCHit*> H, UInt_t planebits, TMatrixD *matrix): TObject(), fHits(H), fPlaneBits(planebits), fConstructMatrix(matrix)
{
  Reset();
  GeneratePlaneIndex();
  CheckHitData();
}

// Constructor for array of hits
THaMWDCGroup::THaMWDCGroup( THaMWDCHit **H, Int_t n, UInt_t planebits, TMatrixD *matrix): TObject(), fPlaneBits(planebits), fConstructMatrix(matrix)
{
  Int_t counter;
  
  for( counter = 0; counter < n; counter++ )
    {
      fHits.push_back( H[counter] );
    }

  Reset();
  GeneratePlaneIndex();
  CheckHitData();
}

// Copy constructor
THaMWDCGroup::THaMWDCGroup(THaMWDCGroup &G){
  int i;
  fHits = G.GetHits();
  fLRbits = G.GetLRbits();
  fConstructMatrix = G.GetConstructMatrix();
  fGroupStatus = G.GetStatusBits();
  fPlaneBits = G.GetPlaneBits();
  fChi2NDoF = G.GetChi2NDoF();;
  GeneratePlaneIndex();
  for( i = 0; i < MAX_PLANES; i++ )
    {
      fChi2Cont[i] = G.GetChi2Cont(i);
      fCoarseChi2Cont[i] = G.GetCoarseChi2Cont(i);
    }
}

// Destructor
THaMWDCGroup::~THaMWDCGroup(){
  fHits.clear();
}

//_____________________________________________________________________________

// External set functions for THaMWDC

void THaMWDCGroup::SetTrackNumber(Int_t track_number)
// Set the track number to associate hits with track in ROOT file
{

  UInt_t counter;

  for( counter = 0; counter < GetNHits(); counter++ )
    {
      GetHit(counter)->SetTrackNumber(track_number);
    }
}

void THaMWDCGroup::SetChi2Cont()
// Set the chi2 contribution to THaMWDCHit for output
{

  UInt_t counter;

  for( counter = 0; counter < GetNHits(); counter++ )
    {
      GetHit(counter)->SetChi2Cont(fChi2Cont[counter]);
      GetHit(counter)->SetCoarseChi2Cont(fCoarseChi2Cont[counter]);
    }
}

//_____________________________________________________________________________
Int_t THaMWDCGroup::DoCoarseTracking()
// Find coarse track (just use wire positions)
{
  // Get coarse track
  fTrack = FindTrack(kCoarseTracking, fCoarseChi2Cont);

  // Save coarse chi^2 contributions in final chi^2 contributions
  // for now.  Will be overwritten in fine tracking
  memcpy(fChi2Cont, fCoarseChi2Cont, GetNHits()*sizeof(Double_t));
  
  //Save us from doing this calculation a million times:
  fChi2NDoF =  fTrack.GetChi2()/fTrack.GetNDoF();

  //Group is no longer fresh and has had coarse tracking done
  fGroupStatus &= ~kFresh;
  fGroupStatus |= kCoarse;

  //Make sure the track is reasonable
  DetermineValidTrack();

  return 0;
}
//_____________________________________________________________________________

Int_t THaMWDCGroup::DoFineTracking(Double_t offset)
// Find fine track (use drift times, scintillator times)
{
  // Get fine track
  fTrack = FindTrack( kFineTracking, fChi2Cont, offset );

  //Save us from doing this calculation a million times:
  fChi2NDoF =  fTrack.GetChi2()/fTrack.GetNDoF();

  //Group is no longer fresh and has had fine tracking done
  fGroupStatus &= ~kFresh;
  fGroupStatus |= kFine;

  //Make sure track is reasonable
  DetermineValidTrack();

  return 0;
}

//_____________________________________________________________________________
Bool_t THaMWDCGroup::IsSimilarTrack(THaTrack track)
{
  //Determine if the two track are similar or not spacially

  Int_t i,nhits, nsim;
  
  Double_t track1c, track2c, z;

  nhits = GetNHits();
  nsim = 0;

  for( i = 0; i < nhits; i++ )
    {
      z = GetHit(i)->GetPlane()->GetZ();
      // Get the plane coordinate track intersections for plane i
      track1c = GetHit(i)->GetPlane()->XYtoPlaneCoord(fTrack.GetX(z), fTrack.GetY(z));
      track2c = GetHit(i)->GetPlane()->XYtoPlaneCoord(track.GetX(z), track.GetY(z));

      // If they re within the wirespacing for that plane they are similar
      if( TMath::Abs( track1c - track2c ) <  GetHit(i)->GetPlane()->GetWSpac() )
	{
	  nsim++;
	}
    }

  if( nsim > MAX_SIMILAR_PLANES_FOR_UNIQUE_TRACK )
    return true;

  return false;
}

//_____________________________________________________________________________

// Internal functions for tracking

//_____________________________________________________________________________
THaTrack THaMWDCGroup::FindTrack( ETrackingMode mode, Double_t chi2cont[], Double_t offset )
{
  // Find track given a mode:
  // kCoarseTracking: Don't use drift times

  // kFineTracking: Do LR differenation.  Use drift times, slope and scint correction

  // kTrialFineTracking: Assume LR differenation.  Use drift times, slope and scint correction
  //      TrialFineTracking is used to iterate over all LR combinations and get a chi^2

  UInt_t hit;
  UInt_t nhits;
  
  Double_t basecoord;
  Double_t slope;

  Double_t coordelms[MAX_PLANES];
  Double_t trackelms[N_TRACK_ELEMENTS];

  THaTrack track;

  memset(coordelms,0,MAX_PLANES*sizeof(*coordelms));

  // If we're doing Fine Tracking, do LR differentiation
  // LR information is then stored in the hits
  if( mode == kFineTracking ) 
    {
      DoLRDiff();
    }

  nhits = GetNHits();

  // Put plane coordinate information in to array
  for( hit = 0; hit < nhits; hit++ )
    {
      switch( mode )
	{
	case kCoarseTracking:
	  // Make sure we're not using residual LR information
	  GetHit(hit)->UnfixLR();
	  basecoord = GetHit(hit)->GetCoord();
	  break;
	case kFineTracking:
	case kTrialFineTracking:
	  // LR information must be set in the hits by this point
	  // For kFineTracking it's done in DoLRDiff()
	  // For kTrialFineTracking it's done before calling this function
	  slope = GetHit(hit)->GetPlane()-> XYtoPlaneCoord( track.GetTheta(), track.GetPhi());
	  basecoord = GetHit(hit)->GetCoord(slope);
	  break;
	default:
	  basecoord = 0.0;
	  break;
	}

      coordelms[hit] = 
	(basecoord+GetHit(hit)->GetPlane()->GetXYCorr())/GetHit(hit)->GetPlane()->GetPosRes();
    }

  // Find track coords and put in trackelms
  TransformCoordToTrack(coordelms,trackelms);
  
  track = THaTrack(trackelms[0], trackelms[2], trackelms[1], trackelms[3] );
  // Group is no longer fresh
  // Group now has Coarse track or fine track
  fGroupStatus &= ~kFresh;
  track.SetChi2( CalcChiSquared(mode, coordelms, track, chi2cont ), nhits - N_TRACK_ELEMENTS );

  return track;
}

//_____________________________________________________________________________
Int_t THaMWDCGroup::DoLRDiff()
{
  Int_t i, nhits, nunfixed;
  vector<THaMWDCPlane *>unfixed;

  UInt_t LRbits, maxbits;
  UInt_t bestLRbits = 0;
  Double_t bestChi2 = 1e9;

  Double_t chi2cont[MAX_PLANES];

  THaTrack trialtrack;

  // Find LR differnation for all planes
  // First, do for X, U, V planes

  // Then do for left/right unfixed planes

  DoLRDiffForType( THaMWDCPlane::kXchamber );
  DoLRDiffForType( THaMWDCPlane::kUchamber );
  DoLRDiffForType( THaMWDCPlane::kVchamber );

  // Identify planes for hits still unfixed
  nhits = GetNHits();
  for( i = 0; i < nhits; i++ )
    {
      if( !GetHit(i)->IsLRFixed() )
	unfixed.push_back(GetHit(i)->GetPlane());
    }


  nunfixed = unfixed.size();

  // Iterate over all possibilities, find the best Chi^2
  bestLRbits = 0;
  bestChi2 = 1e9;

  if( nunfixed > 0 )
    {
      // This sets bit pattern 111...1111 for howeve many bits we have
      maxbits = (1 << (nunfixed+1))-1;
      for( LRbits = 0; LRbits <= maxbits; LRbits++ )
	{
	  // Set L/R in hits defined by LRbits
	  for( i = 0; i < nunfixed; i++ )
	    {
	      //If i'th bit is set use left or i'th hit, otherwise use right
	      GetHit(i)->SetLR( LRbits&(1<<i)? THaMWDCHit::kL : THaMWDCHit::kR );
	    }

	  // Get the track for L/R combination defined by LRbits
	  trialtrack = FindTrack( kTrialFineTracking, chi2cont );

	  // if we have the best chi^2 so far, remember this combination
	  if( trialtrack.GetChi2() < bestChi2 )
	    {
	      bestLRbits = LRbits;
	      bestChi2 = trialtrack.GetChi2();
	    }
	}
    }

  // Fix LR bits as defined by bestLRbits
  for( i = 0; i < nunfixed; i++ )
    {
      GetHit(i)->SetLR( bestLRbits&(1<<i)? THaMWDCHit::kL : THaMWDCHit::kR );
    }

  // Save LR bits for this group
  fLRbits = 0;
  for( i = 0; i < nhits; i++ )
    {
      if(GetHit(i)->GetLR()==THaMWDCHit::kL) fLRbits |= 1<<i;
    }

  unfixed.clear();

  return 0;
}

//_____________________________________________________________________________
Int_t THaMWDCGroup::DoLRDiffForType( THaMWDCPlane::EPlaneType type )
{
  // Does L/R differentiation for hits in planes of the same type

  UInt_t LRbits, bestLRbits = 0;
  
  Double_t chi2, bestChi2;

  UInt_t nhits;
  UInt_t maxbits;
  vector<THaMWDCHit*> hits;

  UInt_t i;

  Double_t slope;
  Double_t x[MAX_PLANES];
  Double_t y[MAX_PLANES];
  Double_t Si[MAX_PLANES];

  Double_t a,b;

  Double_t S, Sx, Sxx, Sxy, Sy, Delta;

  // Get hits of the same type
  hits = GetHitsOfType( type );

  nhits = hits.size();

  bestChi2 = 1e9;

  // Need at least 3 planes to get nonzero chi^2
  // if we have 2 or fewer, LR differenation will get done with other 
  // unfixed planes together

  if( nhits <= 2 ) return 0;

  // Slope is the same for all planes since they're of the same type
  slope = hits[0]->GetPlane()->XYtoPlaneCoord(fTrack.GetTheta(), fTrack.GetPhi() );
  

  // For all LR combos, find the one that gives the best chi^2

  // This sets bit pattern 111...111 of length nhits
  maxbits = (1<<(nhits+1))-1;

  // for all bit patterns
  for( LRbits = 0; LRbits <= maxbits; LRbits++ )
    {
      S = Sx = Sxx = Sxy = Sy = Delta = 0.0;

      // This method is a 2D minimum chi^2 fit
      for( i = 0; i < nhits; i++ )
	{
	  // if i'th bit is set i'th hit as left, otherwise set right
	  hits[i]->SetLR(LRbits&(1<<i)? THaMWDCHit::kL : THaMWDCHit::kR);
	  y[i] = hits[i]->GetCoord(slope)+ hits[i]->GetPlane()->GetXYCorr();
	  x[i] = hits[i]->GetPlane()->GetZ();
	  Si[i] = 1.0/pow(hits[i]->GetPlane()->GetPosRes(),2.0);

	  S += Si[i];
	  Sx += x[i]*Si[i];
	  Sy += y[i]*Si[i];
	  Sxx += x[i]*x[i]*Si[i];
	  Sxy += x[i]*y[i]*Si[i];
	}

      // Find description of minimum chi^2 line
      Delta = S*Sxx - Sx*Sx;
      a = ( Sxx * Sy - Sx * Sxy )/Delta;
      b = ( S * Sxy - Sx * Sy )/Delta;
  
      // Find chi^2
      chi2 = 0.0;
      for( i = 0; i < nhits; i++ )
	{
	  chi2 += (y[i] - a - b*x[i])*(y[i] - a - b*x[i])*Si[i];
	}

      if( chi2 < bestChi2 )
	{
	  bestChi2 = chi2;
	  bestLRbits = LRbits;
	}

    }

  // Fix LR bits for best combo
  for( i = 0; i < nhits; i++ )
    {
      hits[i]->FixLR( bestLRbits&(1<<i)? THaMWDCHit::kL : THaMWDCHit::kR );
    }

  hits.clear();
  return 1;
}

//_____________________________________________________________________________
void THaMWDCGroup::TransformCoordToTrack( Double_t coord[], Double_t track[] )
// Take plane coordinates and find track coordinates
{

  UInt_t counter1;
  UInt_t counter2;
  Double_t sum;

  for( counter1 = 0; counter1 < N_TRACK_ELEMENTS; counter1++ )
    {
      sum = 0.0;
      for( counter2 = 0; counter2 < GetNHits(); counter2++ )
	{
	  // We have to find the proper "plane index" for a hit to correspond to the
	  // proper column to multiply by.
	  sum += (*fConstructMatrix)[counter1][GetPlaneIndexForHit(counter2)] * coord[counter2];
	}
      track[counter1] = sum;
    } 
  return;
}

//_____________________________________________________________________________
Double_t THaMWDCGroup::CalcChiSquared( ETrackingMode mode, Double_t coords[], THaTrack track, Double_t chi2cont[] )
{
  // Calculate chi^2 and save chi^2 contributions for track
  
  Double_t chi2;
  Int_t i, nhits;

  Double_t planeres;
  Double_t sigma = 1.0;
  
  Double_t trackc, z;

  chi2 = 0.0;
  nhits = GetNHits();
  for( i = 0; i < nhits; i++ )
    {
      // Coords are in arbitrary units (scaled by plane resolution), so we have to convert
      if( mode == kCoarseTracking )
	sigma = GetHit(i)->GetPlane()->GetWSpac()/2.0;
      else
	sigma = GetHit(i)->GetPlane()->GetPosRes();

      planeres = GetHit(i)->GetPlane()->GetPosRes();

      z = GetHit(i)->GetPlane()->GetZ();
      trackc = GetHit(i)->GetPlane()->XYtoPlaneCoord(track.GetX(z), track.GetY(z));

      chi2cont[i] = pow(coords[i]*planeres-trackc, 2.0)/(sigma*sigma);

      chi2 += chi2cont[i];

    }

  return chi2;

}

//_____________________________________________________________________________
void THaMWDCGroup::DetermineValidTrack()
{
  //Make sure track is reasonable

  Bool_t isvalid;
  UInt_t i, nhits;

  nhits = GetNHits();
  isvalid = true;

  // Make sure it's in the chamber
  isvalid *= FitsChamber();

  // Make sure Chi^2/NDoF is reasonable
  isvalid *= (GetChi2NDoF()<MAX_CHI2NDOF);

  // Make sure that Chi^2 contributions are all reasonable

  for( i = 0; i < nhits; i++ )
    {
      isvalid *= (GetChi2Cont(i)<MAX_CHI2CONT);
    }
 
  // Set or unset kValidTrack flag accordingly
  if( isvalid && IsOK() )
    {
      fGroupStatus |= kValidTrack;
    }
  else
    {
      fGroupStatus &= ~kValidTrack;
    }

  return;
}

//_____________________________________________________________________________
Bool_t THaMWDCGroup::FitsChamber()
{
  //Determine if the track is within the chamber parameters
  UInt_t i;
  Int_t inchamber = 1;
  Int_t withinangles = 1;
  Double_t z;

  UInt_t nplanes = GetNHits();

  for( i = 0; i < nplanes; i++ )
    {
      z = GetHit(i)->GetPlane()->GetZ();
      if( fTrack.GetX(z) > GetHit(i)->GetPlane()->GetActAreaMaxX() ) inchamber = 0;
      if( fTrack.GetX(z) < GetHit(i)->GetPlane()->GetActAreaMinX() ) inchamber = 0;
      if( fTrack.GetY(z) > GetHit(i)->GetPlane()->GetActAreaMaxY() ) inchamber = 0;
      if( fTrack.GetY(z) < GetHit(i)->GetPlane()->GetActAreaMinY() ) inchamber = 0;
    
      if( fTrack.GetTheta() > GetHit(i)->GetPlane()->GetActAreaMaxTH() ) withinangles = 0;
      if( fTrack.GetTheta() < GetHit(i)->GetPlane()->GetActAreaMinTH() ) withinangles = 0;
      if( fTrack.GetPhi() > GetHit(i)->GetPlane()->GetActAreaMaxPH() ) withinangles = 0;
      if( fTrack.GetPhi() < GetHit(i)->GetPlane()->GetActAreaMinPH() ) withinangles = 0;
    }

  return (inchamber && withinangles );
}

//_____________________________________________________________________________
void THaMWDCGroup::CheckHitData()
{
  // Check is DataValid flag for all hits
  // Group is NOT OK if any hits don't have valid data
  
  Bool_t isvalid;
  Int_t i, nhits;

  nhits = GetNHits();
  isvalid = true;

  for( i = 0; i < nhits; i++ )
    {
      isvalid *= GetHit(i)->IsDataValid();
    }

  if( isvalid )
    {
      fGroupStatus |= kOK;
    }
  else
    {
      fGroupStatus &= ~kOK;
    }
  return;
}

//_____________________________________________________________________________

// Auxillary functions

void THaMWDCGroup::Reset()
{
  // Set group properties to default values

  int i;
  fLRbits = 0;
  fGroupStatus = kFresh;
  fTrack = THaTrack(1e9,1e9,1e9,1e9);

  for(i = 0; i < MAX_PLANES; i++ )
    {
      fChi2Cont[i] = -1.0;
      fCoarseChi2Cont[i] = -1.0;
    }
}


vector<THaMWDCHit*> THaMWDCGroup::GetHitsOfType(THaMWDCPlane::EPlaneType type)
{
  //Find all hits of a specific orientation and return them in a vector
  Int_t i, nhits;
  vector <THaMWDCHit *> hits;

  nhits = GetNHits();
  for(i = 0; i < nhits; i++ )
    {
      if( GetHit(i)->GetPlane()->GetPlaneType()==type )
	{
	  hits.push_back(GetHit(i));
	}
    }

  return hits;
}


void THaMWDCGroup::GeneratePlaneIndex()
{
  // Plane index is used to map the hits (stored sequentially
  // with respect to the plane ordering) to the plane index
  // that is used to do the plane coordinate transformation
  // to track coordinates

  // If we have hits for planes 3, 5, 6, 8, and 9 we should have
  // a bit pattern: 00101101100000
  // We don't know the plane numbers a priori, so we need to figure
  // them out.

  // The indices for the hits in the group are 0, 1, 2, 3, 5
  // So, we need to generate the map:
  // fPlaneIndex = { 3, 5, 6, 8, 9 }

  UInt_t nbits;

  UInt_t bit, hit;

  UInt_t i;

  hit = 0;
  nbits = 0;
  for( i = 0; i < MAX_PLANES; i++ )
    {
      bit = 1 << i;
      
      // if i'th bit is set
      if( fPlaneBits&bit )
	{
	  nbits++;
	  fPlaneIndex[hit] = i;
	  hit++;
	}
    }

  // if we had more bits set than hits something isn't matching up...
  if( nbits != GetNHits() )
    {
      fGroupStatus &= ~kOK;
    }
  else
    {
      fGroupStatus |= kOK;
    }
  return;
}

//_____________________________________________________________________________
ClassImp(THaMWDCGroup)
