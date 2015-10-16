//////////////////////////////////////////////////////////////////////////
//
//    THaOpticsE06010
//
//////////////////////////////////////////////////////////////////////////
//    
// Author: Xin Qian <mailto:xqian@jlab.org>    Mar 2009
//    Modify History: 
//	22 November 2010: Modified by D. Parno for E06-014
//			a. Rolled back to first-order momentum (positive and negative)
//			b. Added additional factor of 1.041 for negative optics.
//			c. Added smoothing function to low-momentum correction (positive and negative)
//			d. Removed energy loss from momentum calculation.
//			e. New "BB.optics.p_firstorder" variable gives tree access to momentum BEFORE
//			low-momentum correction, for future flexibility.
//			f. Changed sign on final value of TargetTheta, in order to give it a sign
//			consistent with detector coordinate system.
//
//////////////////////////////////////////////////////////////////////////
#include "THaSpectrometer.h"
#include "THaTrack.h"
#include "THaBeam.h"
#include "THaUnRasteredBeam.h"
#include "VarDef.h"
#include <iostream>
#include "TClass.h"
#include "TList.h"
#include "TString.h"

using namespace std;
//_____________________________________________________________________________

//put this header file below all other headers
#include "theoptics.h"
#include "THaOpticsE06010.h"


//_____________________________________________________________________________
THaOpticsE06010::THaOpticsE06010(const char* name, const char* desc, THaSpectrometer *pspec, TString strBeamDetectorName)
    :THaOptics(name,desc,pspec,strBeamDetectorName)
				  // constructor function of THaOpticsE06010
{
    DEBUG_LEVEL_RELATED_PERFORMACE_CHECKER;
    
    DEBUG_HALL_A_ANALYZER_DEBUGER_INIT;

    ftroptics = new TClonesArray("theoptics", 200);
}
//_____________________________________________________________________________
  
THaOpticsE06010::~THaOpticsE06010() {
    if( fIsSetup )
      RemoveVariables();
    delete ftroptics;
}
  
void THaOpticsE06010::Clear(Option_t* opt ){
    ftroptics->Clear(opt);
}
  
  //_____________________________________________________________________________
Int_t THaOpticsE06010::ReadDatabase(const TDatime& date)
{
	THaOptics::ReadDatabase(date);
	//static const char* const here = "ReadDatabase";

	//const int LEN = 200;
	//char buff[LEN];
	FILE* file = OpenFile( date );
	if( !file ) return kFileError;



	DEBUG_INFO(Here(here),"\tDB File open OK" );

	vector<vector<double> > *temp = new vector<vector<double> >; 
	vector<vector<double> > *temp1 = new vector<vector<double> >; 
	vector<vector<double> > *temp2 = new vector<vector<double> >; 
	vector<vector<double> > *temp3 = new vector<vector<double> >; 
	vector<vector<double> > *temp4 = new vector<vector<double> >; 
	vector<vector<double> > *temp5 = new vector<vector<double> >; 

	vector<vector<double> > *temp6 = new vector<vector<double> >; 
	vector<vector<double> > *temp7 = new vector<vector<double> >; 
	vector<vector<double> > *temp8 = new vector<vector<double> >; 


	const DBRequest request[]={
		{"nofield_flag", &nofield_flag,     kInt},
		{"calib_flag"  , &calib_flag,       kInt},
		{"sieve_angle" , &sieve_angle,       kDoubleV, 0},
		{"middle_dis"  , &middle_pos,       kDouble},
		{"middle_angle", &middle_angle,     kDouble},
		{"sieve_pos",     &sieve_pos,        kDoubleV, 0},
		{"chamber_angle", &chamber_angle,    kDoubleV, 0},
		{"chamber_pos",   &chamber_pos,      kDoubleV, 0},
		{"vzcorr_1",      temp,              kDoubleM, 9},
		{"vzcorr_2",      temp1,              kDoubleM, 9},
		{"vzcorr_3",      temp2,              kDoubleM, 9},
		{"sievecorr_1",   temp3,              kDoubleM, 3},
		{"momcorr_1",     temp4,              kDoubleM, 5},
		{"momcorr_2",     temp5,              kDoubleM, 4},
		{"vzcorr_p1",     temp6,              kDoubleM, 7},
		{"vzcorr_p2",     temp7,              kDoubleM, 7},
		{"momcorr_p1",    temp8,              kDoubleM, 4},
		{ 0 }
	};

	Int_t err =LoadDB( file, date, request, "" );

	Double_t targetfoils[7]={0.0019-0.2,0.0019-0.2+0.2/3.,0.0019-0.2+0.2/3.*2.,0.0019,0.0019+0.2/3.,0.0019+0.2/3.*2.,+0.0019+0.2};

	vector<vector<double> >::iterator it = temp->begin(); 
	for (Int_t i=0;i!=12;i++){
		for (Int_t j=0;j!=46;j++){
			vector<double>& row = *it;
			vz_x[i][j] = row[0];
			vz_y[i][j] = row[1];
			vz_z[i][j][0] = row[2] + targetfoils[0];
			vz_corr[i][j][0] = row[2];
			vz_z[i][j][1] = row[3] + targetfoils[1];
			vz_corr[i][j][1] = row[3];
			vz_z[i][j][2] = row[4] + targetfoils[2];
			vz_corr[i][j][2] = row[4];
			vz_z[i][j][3] = row[5] + targetfoils[3];
			vz_corr[i][j][3] = row[5];
			vz_z[i][j][4] = row[6] + targetfoils[4];
			vz_corr[i][j][4] = row[6];
			vz_z[i][j][5] = row[7] + targetfoils[5];
			vz_corr[i][j][5] = row[7];
			vz_z[i][j][6] = row[8] + targetfoils[6];
			vz_corr[i][j][6] = row[8];
			if (it != temp->end()){
				++it;
			}else{
				cout << "Something wrong with vzcorr_1 !! " << endl;
			}
		}
	}

	it = temp1->begin();
	for (Int_t i=0;i!=9;i++){
		for (Int_t j=0;j!=24;j++){
			vector<double>& row = *it;
			vz_x1[i][j] = row[0];
			vz_y1[i][j] = row[1];
			vz_z1[i][j][0] = row[2] + targetfoils[0];
			vz_corr1[i][j][0] = row[2];
			vz_z1[i][j][1] = row[3] + targetfoils[1];
			vz_corr1[i][j][1] = row[3];
			vz_z1[i][j][2] = row[4] + targetfoils[2];
			vz_corr1[i][j][2] = row[4];
			vz_z1[i][j][3] = row[5] + targetfoils[3];
			vz_corr1[i][j][3] = row[5];
			vz_z1[i][j][4] = row[6] + targetfoils[4];
			vz_corr1[i][j][4] = row[6];
			vz_z1[i][j][5] = row[7] + targetfoils[5];
			vz_corr1[i][j][5] = row[7];
			vz_z1[i][j][6] = row[8] + targetfoils[6];
			vz_corr1[i][j][6] = row[8];
			if (it != temp->end()){
				++it;
			}else{
				cout << "Something wrong with vzcorr_2 !! " << endl;
			}
		}
	}

	it = temp2->begin();
	for (Int_t i=0;i!=9;i++){
		for (Int_t j=0;j!=16;j++){
			vector<double>& row = *it;
			vz_x2[i][j] = row[0];
			vz_y2[i][j] = row[1];
			vz_z2[i][j][0] = row[2] + targetfoils[0];
			vz_corr2[i][j][0] = row[2];
			vz_z2[i][j][1] = row[3] + targetfoils[1];
			vz_corr2[i][j][1] = row[3];
			vz_z2[i][j][2] = row[4] + targetfoils[2];
			vz_corr2[i][j][2] = row[4];
			vz_z2[i][j][3] = row[5] + targetfoils[3];
			vz_corr2[i][j][3] = row[5];
			vz_z2[i][j][4] = row[6] + targetfoils[4];
			vz_corr2[i][j][4] = row[6];
			vz_z2[i][j][5] = row[7] + targetfoils[5];
			vz_corr2[i][j][5] = row[7];
			vz_z2[i][j][6] = row[8] + targetfoils[6];
			vz_corr2[i][j][6] = row[8];
			if (it != temp->end()){
				++it;
			}else{
				cout << "Something wrong with vzcorr_3 !! " << endl;
			}
		}
	}


	it = temp3->begin();
	for (Int_t i=0;i!=22;i++){
		for (Int_t j=0;j!=12;j++){
			vector<double>& row = *it;
			sieve_x[i][j] = row[0];
			sieve_y[i][j] = row[1];
			sieve_corr[i][j] = row[2];
			if (it != temp->end()){
				++it;
			}else{
				cout << "Something wrong with sievecorr_1 !! " << endl;
			}
		}
	}

	it = temp4->begin();
	for (Int_t i=0;i!=24;i++){
		for (Int_t j=0;j!=11;j++){
			vector<double>& row = *it;
			mom_x[j][i] = row[0];
			mom_y[j][i] = row[1];
			mom_corr1[j][i] = row[2];
			mom_corr2[j][i] = row[3];
			mom_corr3[j][i] = row[4];

			if (it != temp->end()){
				++it;
			}else{
				cout << "Something wrong with momcorr_1 !! " << endl;
			}
		}
	}
	it = temp5->begin();
	for (Int_t i=0;i!=24;i++){
		for (Int_t j=0;j!=11;j++){
			vector<double>& row = *it;
			mom1_x[j][i] = row[0];
			mom1_y[j][i] = row[1];
			mom1_corr1[j][i] = row[2];
			mom1_corr2[j][i] = row[3];
			if (it != temp->end()){
				++it;
			}else{
				cout << "Something wrong with momcorr_2 !! " << endl;
			}
		}
	}


	it = temp6->begin(); 
	for (Int_t i=0;i!=12;i++){
		for (Int_t j=0;j!=26;j++){
			vector<double>& row = *it;
			xp1[i][j] = row[0];
			yp1[i][j] = row[1];
			zp1[i][j][0] = row[2] + targetfoils[0];
			corrp1[i][j][0] = row[2];
			zp1[i][j][1] = row[3] + targetfoils[1];
			corrp1[i][j][1] = row[3];
			zp1[i][j][2] = row[4] + targetfoils[2];
			corrp1[i][j][2] = row[4];
			zp1[i][j][3] = row[5] + targetfoils[3];
			corrp1[i][j][3] = row[5];
			zp1[i][j][4] = row[6] + targetfoils[4];
			corrp1[i][j][4] = row[6];
			if (it != temp->end()){
				++it;
			}else{
				cout << "Something wrong with vzcorr_p1 !! " << endl;
			}
		}
	}

	it = temp7->begin();
	for (Int_t i=0;i!=11;i++){
		for (Int_t j=0;j!=16;j++){
			vector<double>& row = *it;
			xp2[i][j] = row[0];
			yp2[i][j] = row[1];
			zp2[i][j][0] = row[2] + targetfoils[0];
			corrp2[i][j][0] = row[2];
			zp2[i][j][1] = row[3] + targetfoils[1];
			corrp2[i][j][1] = row[3];
			zp2[i][j][2] = row[4] + targetfoils[2];
			corrp2[i][j][2] = row[4];
			zp2[i][j][3] = row[5] + targetfoils[3];
			corrp2[i][j][3] = row[5];
			zp2[i][j][4] = row[6] + targetfoils[4];
			corrp2[i][j][4] = row[6];
			if (it != temp->end()){
				++it;
			}else{
				cout << "Something wrong with vzcorr_p2 !! " << endl;
			}
		}
	}


	it = temp8->begin();
	for (Int_t i=0;i!=12;i++){
		for (Int_t j=0;j!=12;j++){
			vector<double>& row = *it;
			xp4[j][i] = row[0];
			yp4[j][i] = row[1];
			corrp4_1[j][i] = row[2];
			corrp4_2[j][i] = row[3];
			if (it != temp->end()){
				++it;
			}else{
				cout << "Something wrong with momcorr_p1 !! " << endl;
			}
		}
	}


	if( err ){
		fclose(file);
		return kInitError;
	}


	fclose(file);

	return kOK;
}





Double_t THaOpticsE06010::sieve(Double_t x = 0 , Double_t y = 0 , Double_t xp = 0, Double_t yp = 0, Double_t vertex_x = 0,Double_t vertex_y = 0, Double_t vertex_z =0,Double_t dis = 0){
	// flag = 0 : middle
	// flag = 1 : front only
	// flag = 2 : back only
	// dis distance from the edge.

	// 27 slots in total inch
	Double_t thickness = 1.5;
	Double_t par[27][6]={{0.,11.7081,0.,11.7081+ thickness*tan(18./180.*3.1415926),9.75/2.,0.5/2.},
		{0.,9.6704,0.,9.6704+ thickness*tan(18./180.*3.1415926),9.75/2.,0.5/2.},
		{2.312/2.+1.125+2.594/2.,7.7978,2.312/2.+1.125+2.594/2.,7.7978+thickness*tan(9./180.*3.1415926),2.594/2.,0.5/2.},
		{0.,7.7978,0.,7.7978+thickness*tan(9./180.*3.1415926),2.312/2.,0.5/2.},
		{-2.312/2.-1.125-2.594/2.,7.7978,-2.312/2.-1.125-2.594/2.,7.7978+thickness*tan(9./180.*3.1415926),2.594/2.,0.5/2.},
		{0.,5.8356,0.,5.8356+thickness*tan(9./180.*3.1415926),9.75/2.,0.5/2.},
		{2.312/2.+1.125+2.594/2.,3.8734,2.312/2.+1.125+2.594/2.,3.8734+thickness*tan(9./180.*3.1415926),2.594/2.,0.5/2.},
		{0.,3.8734,0.,3.8734+thickness*tan(9./180.*3.1415926),2.312/2.,0.5/2.},
		{-2.312/2.-1.125-2.594/2,3.8734,-2.312/2.-1.125-2.594/2,3.8734+thickness*tan(9./180.*3.1415926),2.594/2.,0.5/2.},
		{0.,1.938,0.,1.938,9.75/2.,0.5/2.},
		{2.312/2.+1.125+2.594/2.,0.,2.312/2.+1.125+2.594/2.,0.,2.594/2.,0.5/2.},
		{0.968,0.,0.968,0.,0.5/2.,0.5/2.},
		{0.,0.,0.,0.,0.5/2.,0.5/2.},
		{-0.968,0.,-0.968,0.,0.5/2.,0.5/2.},
		{-2.312/2.-1.125-2.594/2,0.,-2.312/2.-1.125-2.594/2,0.,2.594/2.,0.5/2.},
		{2.312/2.+1.125+2.594/2.,-1.938,2.312/2.+1.125+2.594/2.,-1.938,2.594/2.,0.5/2.},
		{0.,-1.938,0.,-1.938,2.312/2.,0.5/2.},
		{-2.312/2.-1.125-2.594/2,-1.938,-2.312/2.-1.125-2.594/2,-1.938,2.594/2.,0.5/2.},
		{0.,-3.8734,0.,-3.8734-thickness*tan(9./180.*3.1415926),9.75/2.,0.5/2.},
		{2.312/2.+1.125+2.594/2.,-5.8356,2.312/2.+1.125+2.594/2.,-5.8356-thickness*tan(9./180.*3.1415926),2.594/2.,0.5/2.},
		{0.,-5.8356,0.,-5.8356-thickness*tan(9./180.*3.1415926),2.312/2.,0.5/2.},
		{-2.312/2.-1.125-2.594/2,-5.8356,-2.312/2.-1.125-2.594/2,-5.8356-thickness*tan(9./180.*3.1415926),2.594/2.,0.5/2.},
		{0.,-7.7978,0.,-7.7978-thickness*tan(9./180.*3.1415926),9.75/2.,0.5/2.},
		{2.312/2.+1.125+2.594/2.,-9.6704,2.312/2.+1.125+2.594/2.,-9.6704-thickness*tan(18./180.*3.1415926),2.594/2.,0.5/2.},
		{0.,-9.6704,0.,-9.6704-thickness*tan(18./180.*3.1415926),2.312/2.,0.5/2.},
		{-2.312/2.-1.125-2.594/2,-9.6704,-2.312/2.-1.125-2.594/2,-9.6704-thickness*tan(18./180.*3.1415926),2.594/2.,0.5/2.},
		{0.,-11.7081,0.,-11.7081-thickness*tan(18./180.*3.1415926),9.75/2.,0.5/2.}
	};
	Int_t shape[27]={0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0};

	//convert from inch to m
	for (Int_t i=0;i!=27;i++){
		for (Int_t j=0;j!=6;j++){
			par[i][j] = par[i][j]*2.54/100.;
		}
	}
	thickness = thickness*2.54/100.;

	// check if it is inside.
	Double_t xtemp, ytemp;
	Double_t xtemp1, ytemp1;
	Int_t sieve_front = -1;
	Int_t sieve_back = -1;
	Int_t sieve_total = -1;

	xtemp = x; ytemp = y;
	for (Int_t i=0;i!=27;i++){
		xtemp1 = par[i][0]; ytemp1 = par[i][1];
		if (shape[i]==0){
			if (fabs(xtemp-xtemp1)<par[i][4] + 0.0&& fabs(ytemp-ytemp1)<(par[i][5])*(1-dis)){
				sieve_front = i;
				goto abc;
			}
		}else{
			if (sqrt((xtemp-xtemp1)*(xtemp-xtemp1)+(ytemp-ytemp1)*(ytemp-ytemp1))<(par[i][4])*(1-dis)){
				sieve_front = i;
				goto abc;
			}
		}
	}

abc: 
	xtemp = x + thickness*xp; ytemp = y + thickness*yp;
	for (Int_t i=0;i!=27;i++){
		xtemp1 = par[i][2]; ytemp1 = par[i][3];
		if (shape[i]==0){
			if (fabs(xtemp-xtemp1)<par[i][4] + 0.0&& fabs(ytemp-ytemp1)<par[i][5]*(1-dis)){
				sieve_back = i;
				goto bcd;
			}
		}else{
			if (sqrt((xtemp-xtemp1)*(xtemp-xtemp1)+(ytemp-ytemp1)*(ytemp-ytemp1))<par[i][4]*(1-dis)){
				sieve_back = i;
				goto bcd;
			}
		}
	}

bcd:  
	xtemp = x + thickness*xp/2.; ytemp = y + thickness*yp/2.;
	for (Int_t i=0;i!=27;i++){
		xtemp1 = (par[i][2]+par[i][0])/2.; ytemp1 = (par[i][3]+par[i][1])/2.;
		if (shape[i]==0){
			if (fabs(xtemp-xtemp1)<par[i][4] + 0.0&& fabs(ytemp-ytemp1)<par[i][5]*(1-dis)){
				sieve_total = i;
				goto edf;
			}
		}else{
			if (sqrt((xtemp-xtemp1)*(xtemp-xtemp1)+(ytemp-ytemp1)*(ytemp-ytemp1))<par[i][4]*(1-dis)){
				sieve_total = i;
				goto edf;
			}
		}
	}


edf:

	if (sieve_back == sieve_front && sieve_back == sieve_total){
		sieve_total = sieve_back;
	}else{
		sieve_total = -1;
	}



	Double_t tr_y[5];
	Double_t d;
	if (sieve_total > -1){
		tr_y[0] = (par[sieve_total][1]+par[sieve_total][5]-vertex_y)/(0.-vertex_z);
		tr_y[1] = (par[sieve_total][1]-par[sieve_total][5]-vertex_y)/(0.-vertex_z);
		tr_y[2] = (par[sieve_total][3]+par[sieve_total][5]-vertex_y)/(thickness-vertex_z);
		tr_y[3] = (par[sieve_total][3]-par[sieve_total][5]-vertex_y)/(thickness-vertex_z);


		// if (fabs(tr_y[0])>fabs(tr_y[1])){
		//       tr_y[4] = (tr_y[1] + tr_y[2])/2.;
		//     }else{
		//       tr_y[4] = (tr_y[0] + tr_y[3])/2.;
		//     }
		tr_y[4] = (tr_y[2] + tr_y[3])/2.;

		//    cout << tr_y[4] << "\t" << (tr_y[0] + tr_y[1] + tr_y[2] + tr_y[3])/4. << endl;
		d = y - (tr_y[4]*(-vertex_z)+vertex_y);
	}else{
		d=-100.;
	}


	//if (sieve_total>-1){
	//    d = y +thickness*yp/2.- (par[sieve_total][1]+par[sieve_total][3])/2.;
	//d = (x +thickness*xp/2.- (par[sieve_total][0]+par[sieve_total][2])/2. )*(x +thickness*xp/2.- (par[sieve_total][0]+par[sieve_total][2])/2. )/par[sieve_total][4] /par[sieve_total][4] + (y +thickness*yp/2.- (par[sieve_total][1]+par[sieve_total][3])/2. )*(y +thickness*yp/2.- (par[sieve_total][1]+par[sieve_total][3])/2. )/par[sieve_total][5]/par[sieve_total][5];
	// }else if (sieve_total>-1.&&flag==1){
	//d = y - (par[sieve_total][1]+par[sieve_total][1])/2. ;
	//d = (x - (par[sieve_total][0]+par[sieve_total][0])/2. )*(x - (par[sieve_total][0]+par[sieve_total][0])/2. )/par[sieve_total][4] /par[sieve_total][4] + (y - (par[sieve_total][1]+par[sieve_total][1])/2. )*(y - (par[sieve_total][1]+par[sieve_total][1])/2. )/par[sieve_total][5]/par[sieve_total][5];
	// }else if (sieve_total>-1.&&flag==2){
	//d = y +thickness*yp - (par[sieve_total][3]+par[sieve_total][3])/2. ;
	//d = (x +thickness*xp- (par[sieve_total][2]+par[sieve_total][2])/2. )*(x +thickness*xp- (par[sieve_total][2]+par[sieve_total][2])/2. )/par[sieve_total][4] /par[sieve_total][4] + (y +thickness*yp - (par[sieve_total][3]+par[sieve_total][3])/2. )*(y +thickness*yp - (par[sieve_total][3]+par[sieve_total][3])/2. )/par[sieve_total][5]/par[sieve_total][5];
	//}else{
	//d = -100.;
	//}
	return d;

}


Double_t THaOpticsE06010::sieve1(Double_t x = 0 , Double_t y = 0 , Double_t xp = 0, Double_t yp = 0, Double_t vertex_x = 0,Double_t vertex_y = 0, Double_t vertex_z =0,Double_t dis = 0){
  Double_t thickness = 1.5;
  Double_t par[27][6]={{0.,11.7081,0.,11.7081+ thickness*tan(18./180.*3.1415926),9.75/2.,0.5/2.},
		       {0.,9.6704,0.,9.6704+ thickness*tan(18./180.*3.1415926),9.75/2.,0.5/2.},
		       {2.312/2.+1.125+2.594/2.,7.7978,2.312/2.+1.125+2.594/2.,7.7978+thickness*tan(9./180.*3.1415926),2.594/2.,0.5/2.},
		       {0.,7.7978,0.,7.7978+thickness*tan(9./180.*3.1415926),2.312/2.,0.5/2.},
		       {-2.312/2.-1.125-2.594/2.,7.7978,-2.312/2.-1.125-2.594/2.,7.7978+thickness*tan(9./180.*3.1415926),2.594/2.,0.5/2.},
		       {0.,5.8356,0.,5.8356+thickness*tan(9./180.*3.1415926),9.75/2.,0.5/2.},
		       {2.312/2.+1.125+2.594/2.,3.8734,2.312/2.+1.125+2.594/2.,3.8734+thickness*tan(9./180.*3.1415926),2.594/2.,0.5/2.},
		       {0.,3.8734,0.,3.8734+thickness*tan(9./180.*3.1415926),2.312/2.,0.5/2.},
		       {-2.312/2.-1.125-2.594/2,3.8734,-2.312/2.-1.125-2.594/2,3.8734+thickness*tan(9./180.*3.1415926),2.594/2.,0.5/2.},
		       {0.,1.938,0.,1.938,9.75/2.,0.5/2.},
		       {2.312/2.+1.125+2.594/2.,0.,2.312/2.+1.125+2.594/2.,0.,2.594/2.,0.5/2.},
		       {0.968,0.,0.968,0.,0.5/2.,0.5/2.},
		       {0.,0.,0.,0.,0.5/2.,0.5/2.},
		       {-0.968,0.,-0.968,0.,0.5/2.,0.5/2.},
		       {-2.312/2.-1.125-2.594/2,0.,-2.312/2.-1.125-2.594/2,0.,2.594/2.,0.5/2.},
		       {2.312/2.+1.125+2.594/2.,-1.938,2.312/2.+1.125+2.594/2.,-1.938,2.594/2.,0.5/2.},
		       {0.,-1.938,0.,-1.938,2.312/2.,0.5/2.},
		       {-2.312/2.-1.125-2.594/2,-1.938,-2.312/2.-1.125-2.594/2,-1.938,2.594/2.,0.5/2.},
		       {0.,-3.8734,0.,-3.8734-thickness*tan(9./180.*3.1415926),9.75/2.,0.5/2.},
		       {2.312/2.+1.125+2.594/2.,-5.8356,2.312/2.+1.125+2.594/2.,-5.8356-thickness*tan(9./180.*3.1415926),2.594/2.,0.5/2.},
		       {0.,-5.8356,0.,-5.8356-thickness*tan(9./180.*3.1415926),2.312/2.,0.5/2.},
		       {-2.312/2.-1.125-2.594/2,-5.8356,-2.312/2.-1.125-2.594/2,-5.8356-thickness*tan(9./180.*3.1415926),2.594/2.,0.5/2.},
		       {0.,-7.7978,0.,-7.7978-thickness*tan(9./180.*3.1415926),9.75/2.,0.5/2.},
		       {2.312/2.+1.125+2.594/2.,-9.6704,2.312/2.+1.125+2.594/2.,-9.6704-thickness*tan(18./180.*3.1415926),2.594/2.,0.5/2.},
		       {0.,-9.6704,0.,-9.6704-thickness*tan(18./180.*3.1415926),2.312/2.,0.5/2.},
		       {-2.312/2.-1.125-2.594/2,-9.6704,-2.312/2.-1.125-2.594/2,-9.6704-thickness*tan(18./180.*3.1415926),2.594/2.,0.5/2.},
		       {0.,-11.7081,0.,-11.7081-thickness*tan(18./180.*3.1415926),9.75/2.,0.5/2.}
  };
  Int_t shape[27]={0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0};
  
  //convert from inch to m
  for (Int_t i=0;i!=27;i++){
    for (Int_t j=0;j!=6;j++){
      par[i][j] = par[i][j]*2.54/100.;
    }
  }
  thickness = thickness*2.54/100.;

  // check if it is inside.
  Double_t xtemp, ytemp;
  Double_t xtemp1, ytemp1;
  Int_t sieve_front = -1;
  Int_t sieve_back = -1;
  Int_t sieve_total = -1;
  Int_t sieve_flag = 0;

  xtemp = x; ytemp = y;
  for (Int_t i=0;i!=27;i++){
    xtemp1 = par[i][0]; ytemp1 = par[i][1];
    if (shape[i]==0){
      if (fabs(xtemp-xtemp1)<par[i][4] + 0.005 && fabs(ytemp-ytemp1)<(par[i][5])*(1-dis)){
	sieve_front = i;
	if (fabs(xtemp-xtemp1-par[i][4])<0.005){
	  sieve_flag = 2;
	}else if (fabs(xtemp-xtemp1+par[i][4])<0.005){
	  sieve_flag = 3;
	}
	goto abc;
      }
    }else{
      if (sqrt((xtemp-xtemp1)*(xtemp-xtemp1)+(ytemp-ytemp1)*(ytemp-ytemp1))<(par[i][4])*(1-dis)){
	sieve_front = i;
	sieve_flag = 1;
	goto abc;
      }
    }
  }
  
 abc: 
  xtemp = x + thickness*xp; ytemp = y + thickness*yp;
  for (Int_t i=0;i!=27;i++){
    xtemp1 = par[i][2]; ytemp1 = par[i][3];
    if (shape[i]==0){
      if (fabs(xtemp-xtemp1)<par[i][4] + 0.005 && fabs(ytemp-ytemp1)<par[i][5]*(1-dis)){
	sieve_back = i;
	if (fabs(xtemp-xtemp1-par[i][4])<0.005){
	  sieve_flag = 2;
	}else if (fabs(xtemp-xtemp1+par[i][4])<0.005){
	  sieve_flag = 3;
	}
	goto bcd;
      }
    }else{
      if (sqrt((xtemp-xtemp1)*(xtemp-xtemp1)+(ytemp-ytemp1)*(ytemp-ytemp1))<par[i][4]*(1-dis)){
	sieve_back = i;
	sieve_flag = 1;
	goto bcd;
      }
    }
  }
  
 bcd:  
  xtemp = x + thickness*xp/2.; ytemp = y + thickness*yp/2.;
  for (Int_t i=0;i!=27;i++){
    xtemp1 = (par[i][2]+par[i][0])/2.; ytemp1 = (par[i][3]+par[i][1])/2.;
    if (shape[i]==0){
      if (fabs(xtemp-xtemp1)<par[i][4] + 0.005 && fabs(ytemp-ytemp1)<par[i][5]*(1-dis)){
	if (fabs(xtemp-xtemp1-par[i][4])<0.005){
	  sieve_flag = 2;
	}else if (fabs(xtemp-xtemp1+par[i][4])<0.005){
	  sieve_flag = 3;
	}
	sieve_total = i;
	goto edf;
      }
    }else{
      if (sqrt((xtemp-xtemp1)*(xtemp-xtemp1)+(ytemp-ytemp1)*(ytemp-ytemp1))<par[i][4]*(1-dis)){
	sieve_total = i;
	sieve_flag = 1;
	goto edf;
      }
    }
  }


 edf:

  if (sieve_back == sieve_front && sieve_back == sieve_total){
    sieve_total = sieve_back;
  }else{
    sieve_total = -1;
  }
  
  Double_t tr_y;
  Double_t d;
  if (sieve_total > -1 && sieve_flag > 0.){
    if (sieve_flag==1){
      tr_y = (par[sieve_total][0]-vertex_x)/(0.-vertex_z)/2.;
      tr_y += (par[sieve_total][2]-vertex_x)/(thickness-vertex_z)/2.;
      d = x - (tr_y*(-vertex_z)+vertex_x);
    }else if (sieve_flag==2){
      tr_y  = (par[sieve_total][0]+par[sieve_total][4]-vertex_x)/(0.-vertex_z)/2.;
      tr_y += (par[sieve_total][2]+par[sieve_total][4]-vertex_x)/(thickness-vertex_z)/2.;
      d = x - (tr_y*(-vertex_z)+vertex_x);
    }else{
      tr_y  = (par[sieve_total][0]-par[sieve_total][4]-vertex_x)/(0.-vertex_z)/2.;
      tr_y += (par[sieve_total][2]-par[sieve_total][4]-vertex_x)/(thickness-vertex_z)/2.;
      d = x - (tr_y*(-vertex_z)+vertex_x);
    }
  }else{
    d=-100.;
  }
  return d;
  
}





Double_t THaOpticsE06010::sndcorr(Double_t x1, Double_t y1 , Double_t z1, Double_t *x, Double_t *y, Double_t *z, Double_t *corr){
  // first get a feeling of where is x and y
  Int_t qx_x,qx_y;
  qx_x = Int_t((x1+0.1)/0.02);
  qx_x = check(qx_x,0,11);
  qx_y = Int_t((0.40-y1)/0.02);
  qx_y = check(qx_y,0,45);

  // get the cloest one, small one, large one for x, y, and z;
  Int_t qx_x_clo=0,qx_x_min;
  Int_t qx_y_clo=0,qx_y_min;
  
  // get closest one first
  Int_t qx_x_temp,qx_y_temp;  
  Double_t min = 1000000.,temp;
  for (Int_t i=-1;i!=2;i++){
    for (Int_t j=-1;j!=2;j++){
      qx_x_temp = check(qx_x + i,0,11);
      qx_y_temp = check(qx_y + j,0,45);
      temp = (x1-*(x+46*qx_x_temp + qx_y_temp))*(x1-*(x+46*qx_x_temp + qx_y_temp)) + (y1-*(y+46*qx_x_temp + qx_y_temp))*(y1-*(y+46*qx_x_temp + qx_y_temp));
      if (temp < min){
	qx_x_clo = qx_x_temp;
	qx_y_clo = qx_y_temp;
	min = temp;
      }
    }
  }
  
  //get min and max for x and y
  if (qx_x_clo == 0){
    qx_x_min = 0;
    if (qx_y_clo == 0){
      qx_y_min = 0;
    }else if (qx_y_clo == 45){
      qx_y_min = 44;
    }else if (y1 < *(y+ 46*qx_x_clo + qx_y_clo)){
      qx_y_min = qx_y_clo;
    }else{
      qx_y_min = qx_y_clo - 1;
    }
  }else if (qx_x_clo == 11){
    qx_x_min = 10;
    if (qx_y_clo == 0){
      qx_y_min = 0;
    }else if (qx_y_clo == 45){
      qx_y_min = 44;
    }else if (y1 < *(y + 46*qx_x_clo + qx_y_clo)){
      qx_y_min = qx_y_clo;
    }else{
      qx_y_min = qx_y_clo - 1;
    }
  }else{
    if (x1 > *(x+46*qx_x_clo+qx_y_clo)){
      qx_x_min = qx_x_clo;
    }else{
      qx_x_min = qx_x_clo - 1;
    }
    if (qx_y_clo ==0){
      qx_y_min = 0;
    }else if (qx_y_clo == 45){
      qx_y_min = 44;
    }else if (y1 < *(y+46*qx_x_clo + qx_y_clo)){
      qx_y_min = qx_y_clo;
    }else{
      qx_y_min = qx_y_clo - 1;
    }
  }
  
  //get min and max for z
  Int_t qx_z_min;
  if (z1 <= *(z+46*7*qx_x_clo + 7*qx_y_clo + 1)){
    qx_z_min = 0;
  }else if (z1 > *(z+46*7*qx_x_clo + 7*qx_y_clo + 1)&& z1 <= *(z+46*7*qx_x_clo + 7*qx_y_clo + 2)){
    qx_z_min = 1;
  }else if (z1 > *(z+46*7*qx_x_clo + 7*qx_y_clo + 2) && z1 <= *(z+46*7*qx_x_clo + 7*qx_y_clo + 3)){
    qx_z_min = 2;
  }else if (z1 > *(z+46*7*qx_x_clo + 7*qx_y_clo + 3) && z1 <= *(z+46*7*qx_x_clo + 7*qx_y_clo + 4)){
    qx_z_min = 3;
  }else if(z1 > *(z+46*7*qx_x_clo + 7*qx_y_clo + 4) && z1 <= *(z+46*7*qx_x_clo + 7*qx_y_clo + 5)){
    qx_z_min = 4;
  }else{
    qx_z_min = 5;
  }
  
  Double_t final_corr = 0.;
  
  final_corr = *(corr + 46*7*qx_x_min + 7*qx_y_min + qx_z_min)       * 
    ( *(x + 46*(qx_x_min + 1)+qx_y_clo) - x1)/(*(x + 46*(qx_x_min + 1)+qx_y_clo) - *(x + 46*qx_x_min + qx_y_clo)) * 
    ( *(y + 46*qx_x_clo + qx_y_min + 1) - y1)/(*(y + 46*qx_x_clo + qx_y_min + 1) - *(y + 46*qx_x_clo + qx_y_min)) *
    ( *(z + 46*7*qx_x_clo + 7*qx_y_clo + qx_z_min + 1) - z1)/(*(z + 46*7*qx_x_clo + 7*qx_y_clo +qx_z_min + 1) - *(z + 46*7*qx_x_clo + 7*qx_y_clo + qx_z_min));
  final_corr += *(corr + 46*7*qx_x_min + 7*qx_y_min + qx_z_min + 1)       * 
    ( *(x + 46*(qx_x_min + 1)+qx_y_clo) - x1)/(*(x + 46*(qx_x_min + 1)+qx_y_clo) - *(x + 46*qx_x_min + qx_y_clo)) * 
    ( *(y + 46*qx_x_clo + qx_y_min + 1) - y1)/(*(y + 46*qx_x_clo + qx_y_min + 1) - *(y + 46*qx_x_clo + qx_y_min)) *
    ( - *(z + 46*7*qx_x_clo + 7*qx_y_clo + qx_z_min) + z1)/(*(z + 46*7*qx_x_clo + 7*qx_y_clo +qx_z_min + 1) - *(z + 46*7*qx_x_clo + 7*qx_y_clo + qx_z_min));
  final_corr += *(corr + 46*7*qx_x_min + 7*(qx_y_min+1) + qx_z_min)       * 
    ( *(x + 46*(qx_x_min + 1)+qx_y_clo) - x1)/(*(x + 46*(qx_x_min + 1)+qx_y_clo) - *(x + 46*qx_x_min + qx_y_clo)) * 
    ( -*(y + 46*qx_x_clo + qx_y_min) + y1)/(*(y + 46*qx_x_clo + qx_y_min + 1) - *(y + 46*qx_x_clo + qx_y_min)) *
    ( *(z + 46*7*qx_x_clo + 7*qx_y_clo + qx_z_min + 1) - z1)/(*(z + 46*7*qx_x_clo + 7*qx_y_clo +qx_z_min + 1) - *(z + 46*7*qx_x_clo + 7*qx_y_clo + qx_z_min));
  final_corr += *(corr + 46*7*qx_x_min + 7*(qx_y_min+1) + qx_z_min + 1)       * 
    ( *(x + 46*(qx_x_min + 1)+qx_y_clo) - x1)/(*(x + 46*(qx_x_min + 1)+qx_y_clo) - *(x + 46*qx_x_min + qx_y_clo)) * 
    ( -*(y + 46*qx_x_clo + qx_y_min) + y1)/(*(y + 46*qx_x_clo + qx_y_min + 1) - *(y + 46*qx_x_clo + qx_y_min)) *
    ( -*(z + 46*7*qx_x_clo + 7*qx_y_clo + qx_z_min) + z1)/(*(z + 46*7*qx_x_clo + 7*qx_y_clo +qx_z_min + 1) - *(z + 46*7*qx_x_clo + 7*qx_y_clo + qx_z_min));
  final_corr += *(corr + 46*7*(qx_x_min+1) + 7*qx_y_min + qx_z_min)       * 
    ( -*(x + 46*(qx_x_min)+qx_y_clo) + x1)/(*(x + 46*(qx_x_min + 1)+qx_y_clo) - *(x + 46*qx_x_min + qx_y_clo)) * 
    ( *(y + 46*qx_x_clo + qx_y_min + 1) - y1)/(*(y + 46*qx_x_clo + qx_y_min + 1) - *(y + 46*qx_x_clo + qx_y_min)) *
    ( *(z + 46*7*qx_x_clo + 7*qx_y_clo + qx_z_min + 1) - z1)/(*(z + 46*7*qx_x_clo + 7*qx_y_clo +qx_z_min + 1) - *(z + 46*7*qx_x_clo + 7*qx_y_clo + qx_z_min));
  final_corr += *(corr + 46*7*(qx_x_min+1) + 7*qx_y_min + qx_z_min+1)       * 
    ( -*(x + 46*(qx_x_min)+qx_y_clo) + x1)/(*(x + 46*(qx_x_min + 1)+qx_y_clo) - *(x + 46*qx_x_min + qx_y_clo)) * 
    ( *(y + 46*qx_x_clo + qx_y_min + 1) - y1)/(*(y + 46*qx_x_clo + qx_y_min + 1) - *(y + 46*qx_x_clo + qx_y_min)) *
    ( -*(z + 46*7*qx_x_clo + 7*qx_y_clo + qx_z_min) + z1)/(*(z + 46*7*qx_x_clo + 7*qx_y_clo +qx_z_min + 1) - *(z + 46*7*qx_x_clo + 7*qx_y_clo + qx_z_min));
  final_corr += *(corr + 46*7*(qx_x_min+1) + 7*(qx_y_min+1) + qx_z_min)       * 
    ( -*(x + 46*(qx_x_min)+qx_y_clo) + x1)/(*(x + 46*(qx_x_min + 1)+qx_y_clo) - *(x + 46*qx_x_min + qx_y_clo)) * 
    ( -*(y + 46*qx_x_clo + qx_y_min) + y1)/(*(y + 46*qx_x_clo + qx_y_min + 1) - *(y + 46*qx_x_clo + qx_y_min)) *
    ( *(z + 46*7*qx_x_clo + 7*qx_y_clo + qx_z_min + 1) - z1)/(*(z + 46*7*qx_x_clo + 7*qx_y_clo +qx_z_min + 1) - *(z + 46*7*qx_x_clo + 7*qx_y_clo + qx_z_min));
  final_corr += *(corr + 46*7*(qx_x_min+1) + 7*(qx_y_min+1) + qx_z_min+1)       * 
    ( -*(x + 46*(qx_x_min)+qx_y_clo) + x1)/(*(x + 46*(qx_x_min + 1)+qx_y_clo) - *(x + 46*qx_x_min + qx_y_clo)) * 
    ( -*(y + 46*qx_x_clo + qx_y_min) + y1)/(*(y + 46*qx_x_clo + qx_y_min + 1) - *(y + 46*qx_x_clo + qx_y_min)) *
    ( -*(z + 46*7*qx_x_clo + 7*qx_y_clo + qx_z_min) + z1)/(*(z + 46*7*qx_x_clo + 7*qx_y_clo +qx_z_min + 1) - *(z + 46*7*qx_x_clo + 7*qx_y_clo + qx_z_min));

  return final_corr;
}




Double_t THaOpticsE06010::thrcorr(Double_t x1, Double_t y1 , Double_t z1, Double_t *x, Double_t *y, Double_t *z, Double_t *corr){
  // first get a feeling of where is x and y
  Int_t qx_x=0,qx_y;
  Float_t par[10]={0.,0.7,0.8,0.9,1.0,1.1,1.2,1.4,1.6,2.0};
  for (Int_t i=0;i!=9;i++){
    if (x1 >= par[i] && x1 <=par[i+1]){
      qx_x = i;
    }else if (x1 >= par[9]){
      qx_x = 8;
    }else if (x1 < par[0]){
      qx_x = 0;
    }
  }

  

  qx_y = Int_t((0.40-y1)/0.04);
  qx_y = check(qx_y,0,23);
  
  
  // get the cloest one, small one, large one for x, y, and z;
  Int_t qx_x_clo=0,qx_x_min;
  Int_t qx_y_clo=0,qx_y_min;
  
  // get closest one first
  Int_t qx_x_temp,qx_y_temp;  
  Double_t min = 1000000.,temp;
  
  for (Int_t i=-1;i!=2;i++){
    for (Int_t j=-1;j!=2;j++){
      qx_x_temp = check(qx_x + i,0,8);
      qx_y_temp = check(qx_y + j,0,23);
      temp = (x1-*(x+24*qx_x_temp + qx_y_temp))*(x1-*(x+24*qx_x_temp + qx_y_temp)) + (y1-*(y+24*qx_x_temp + qx_y_temp))*(y1-*(y+24*qx_x_temp + qx_y_temp));
      
      if (temp < min){
	qx_x_clo = qx_x_temp;
	qx_y_clo = qx_y_temp;
	min = temp;
      }
    }
  }
  
  //get min and max for x and y
  if (qx_x_clo == 0){
    qx_x_min = 0;
    if (qx_y_clo == 0){
      qx_y_min = 0;
    }else if (qx_y_clo == 23){
      qx_y_min = 22;
    }else if (y1 < *(y+ 24*qx_x_clo + qx_y_clo)){
      qx_y_min = qx_y_clo;
    }else{
      qx_y_min = qx_y_clo - 1;
    }
  }else if (qx_x_clo == 8){
    qx_x_min = 7;
    if (qx_y_clo == 0){
      qx_y_min = 0;
    }else if (qx_y_clo == 23){
      qx_y_min = 22;
    }else if (y1 < *(y + 24*qx_x_clo + qx_y_clo)){
      qx_y_min = qx_y_clo;
    }else{
      qx_y_min = qx_y_clo - 1;
    }
  }else{
    if (x1 > *(x+24*qx_x_clo+qx_y_clo)){
      qx_x_min = qx_x_clo;
    }else{
      qx_x_min = qx_x_clo - 1;
    }
    if (qx_y_clo ==0){
      qx_y_min = 0;
    }else if (qx_y_clo == 23){
      qx_y_min = 22;
    }else if (y1 < *(y+24*qx_x_clo + qx_y_clo)){
      qx_y_min = qx_y_clo;
    }else{
      qx_y_min = qx_y_clo - 1;
    }
  }


  //get min and max for z
  Int_t qx_z_min;
  if (z1 <= *(z+24*7*qx_x_clo + 7*qx_y_clo + 1)){
    qx_z_min = 0;
  }else if (z1 > *(z+24*7*qx_x_clo + 7*qx_y_clo + 1)&& z1 <= *(z+24*7*qx_x_clo + 7*qx_y_clo + 2)){
    qx_z_min = 1;
  }else if (z1 > *(z+24*7*qx_x_clo + 7*qx_y_clo + 2) && z1 <= *(z+24*7*qx_x_clo + 7*qx_y_clo + 3)){
    qx_z_min = 2;
  }else if (z1 > *(z+24*7*qx_x_clo + 7*qx_y_clo + 3) && z1 <= *(z+24*7*qx_x_clo + 7*qx_y_clo + 4)){
    qx_z_min = 3;
  }else if (z1 > *(z+24*7*qx_x_clo + 7*qx_y_clo + 4) && z1 <= *(z+24*7*qx_x_clo + 7*qx_y_clo + 5)){
    qx_z_min = 4;
  }else{
    qx_z_min = 5;
  }
  

 
  Double_t final_corr = 0.;
  
  
  final_corr = *(corr + 24*7*qx_x_min + 7*qx_y_min + qx_z_min)       * 
    ( *(x + 24*(qx_x_min + 1)+qx_y_clo) - x1)/(*(x + 24*(qx_x_min + 1)+qx_y_clo) - *(x + 24*qx_x_min + qx_y_clo)) * 
    ( *(y + 24*qx_x_clo + qx_y_min + 1) - y1)/(*(y + 24*qx_x_clo + qx_y_min + 1) - *(y + 24*qx_x_clo + qx_y_min)) *
    ( *(z + 24*7*qx_x_clo + 7*qx_y_clo + qx_z_min + 1) - z1)/(*(z + 24*7*qx_x_clo + 7*qx_y_clo +qx_z_min + 1) - *(z + 24*7*qx_x_clo + 7*qx_y_clo + qx_z_min));
  final_corr += *(corr + 24*7*qx_x_min + 7*qx_y_min + qx_z_min + 1)       * 
    ( *(x + 24*(qx_x_min + 1)+qx_y_clo) - x1)/(*(x + 24*(qx_x_min + 1)+qx_y_clo) - *(x + 24*qx_x_min + qx_y_clo)) * 
    ( *(y + 24*qx_x_clo + qx_y_min + 1) - y1)/(*(y + 24*qx_x_clo + qx_y_min + 1) - *(y + 24*qx_x_clo + qx_y_min)) *
    ( - *(z + 24*7*qx_x_clo + 7*qx_y_clo + qx_z_min) + z1)/(*(z + 24*7*qx_x_clo + 7*qx_y_clo +qx_z_min + 1) - *(z + 24*7*qx_x_clo + 7*qx_y_clo + qx_z_min));
  final_corr += *(corr + 24*7*qx_x_min + 7*(qx_y_min+1) + qx_z_min)       * 
    ( *(x + 24*(qx_x_min + 1)+qx_y_clo) - x1)/(*(x + 24*(qx_x_min + 1)+qx_y_clo) - *(x + 24*qx_x_min + qx_y_clo)) * 
    ( -*(y + 24*qx_x_clo + qx_y_min) + y1)/(*(y + 24*qx_x_clo + qx_y_min + 1) - *(y + 24*qx_x_clo + qx_y_min)) *
    ( *(z + 24*7*qx_x_clo + 7*qx_y_clo + qx_z_min + 1) - z1)/(*(z + 24*7*qx_x_clo + 7*qx_y_clo +qx_z_min + 1) - *(z + 24*7*qx_x_clo + 7*qx_y_clo + qx_z_min));
  final_corr += *(corr + 24*7*qx_x_min + 7*(qx_y_min+1) + qx_z_min + 1)       * 
    ( *(x + 24*(qx_x_min + 1)+qx_y_clo) - x1)/(*(x + 24*(qx_x_min + 1)+qx_y_clo) - *(x + 24*qx_x_min + qx_y_clo)) * 
    ( -*(y + 24*qx_x_clo + qx_y_min) + y1)/(*(y + 24*qx_x_clo + qx_y_min + 1) - *(y + 24*qx_x_clo + qx_y_min)) *
    ( -*(z + 24*7*qx_x_clo + 7*qx_y_clo + qx_z_min) + z1)/(*(z + 24*7*qx_x_clo + 7*qx_y_clo +qx_z_min + 1) - *(z + 24*7*qx_x_clo + 7*qx_y_clo + qx_z_min));
  final_corr += *(corr + 24*7*(qx_x_min+1) + 7*qx_y_min + qx_z_min)       * 
    ( -*(x + 24*(qx_x_min)+qx_y_clo) + x1)/(*(x + 24*(qx_x_min + 1)+qx_y_clo) - *(x + 24*qx_x_min + qx_y_clo)) * 
    ( *(y + 24*qx_x_clo + qx_y_min + 1) - y1)/(*(y + 24*qx_x_clo + qx_y_min + 1) - *(y + 24*qx_x_clo + qx_y_min)) *
    ( *(z + 24*7*qx_x_clo + 7*qx_y_clo + qx_z_min + 1) - z1)/(*(z + 24*7*qx_x_clo + 7*qx_y_clo +qx_z_min + 1) - *(z + 24*7*qx_x_clo + 7*qx_y_clo + qx_z_min));
  final_corr += *(corr + 24*7*(qx_x_min+1) + 7*qx_y_min + qx_z_min+1)       * 
    ( -*(x + 24*(qx_x_min)+qx_y_clo) + x1)/(*(x + 24*(qx_x_min + 1)+qx_y_clo) - *(x + 24*qx_x_min + qx_y_clo)) * 
    ( *(y + 24*qx_x_clo + qx_y_min + 1) - y1)/(*(y + 24*qx_x_clo + qx_y_min + 1) - *(y + 24*qx_x_clo + qx_y_min)) *
    ( -*(z + 24*7*qx_x_clo + 7*qx_y_clo + qx_z_min) + z1)/(*(z + 24*7*qx_x_clo + 7*qx_y_clo +qx_z_min + 1) - *(z + 24*7*qx_x_clo + 7*qx_y_clo + qx_z_min));
  final_corr += *(corr + 24*7*(qx_x_min+1) + 7*(qx_y_min+1) + qx_z_min)       * 
    ( -*(x + 24*(qx_x_min)+qx_y_clo) + x1)/(*(x + 24*(qx_x_min + 1)+qx_y_clo) - *(x + 24*qx_x_min + qx_y_clo)) * 
    ( -*(y + 24*qx_x_clo + qx_y_min) + y1)/(*(y + 24*qx_x_clo + qx_y_min + 1) - *(y + 24*qx_x_clo + qx_y_min)) *
    ( *(z + 24*7*qx_x_clo + 7*qx_y_clo + qx_z_min + 1) - z1)/(*(z + 24*7*qx_x_clo + 7*qx_y_clo +qx_z_min + 1) - *(z + 24*7*qx_x_clo + 7*qx_y_clo + qx_z_min));
  final_corr += *(corr + 24*7*(qx_x_min+1) + 7*(qx_y_min+1) + qx_z_min+1)       * 
    ( -*(x + 24*(qx_x_min)+qx_y_clo) + x1)/(*(x + 24*(qx_x_min + 1)+qx_y_clo) - *(x + 24*qx_x_min + qx_y_clo)) * 
    ( -*(y + 24*qx_x_clo + qx_y_min) + y1)/(*(y + 24*qx_x_clo + qx_y_min + 1) - *(y + 24*qx_x_clo + qx_y_min)) *
    ( -*(z + 24*7*qx_x_clo + 7*qx_y_clo + qx_z_min) + z1)/(*(z + 24*7*qx_x_clo + 7*qx_y_clo +qx_z_min + 1) - *(z + 24*7*qx_x_clo + 7*qx_y_clo + qx_z_min));

  return final_corr;
}



Double_t THaOpticsE06010::foucorr(Double_t x1, Double_t y1 , Double_t z1, Double_t *x, Double_t *y, Double_t *z, Double_t *corr){
  // first get a feeling of where is x and y
  Int_t qx_x=0,qx_y;
  Float_t par[10]={0.,0.7,0.8,0.9,1.0,1.1,1.2,1.4,1.6,2.0};
  for (Int_t i=0;i!=9;i++){
    if (x1 >= par[i] && x1 <=par[i+1]){
      qx_x = i;
    }else if (x1 >= par[9]){
      qx_x = 8;
    }else if (x1 < par[0]){
      qx_x = 0;
    }
  }

  

  qx_y = Int_t((0.13-y1)/0.017);
  qx_y = check(qx_y,0,15);
  
  
  // get the cloest one, small one, large one for x, y, and z;
  Int_t qx_x_clo=0,qx_x_min;
  Int_t qx_y_clo=0,qx_y_min;
  
  // get closest one first
  Int_t qx_x_temp,qx_y_temp;  
  Double_t min = 1000000.,temp;
  
  for (Int_t i=-1;i!=2;i++){
    for (Int_t j=-1;j!=2;j++){
      qx_x_temp = check(qx_x + i,0,8);
      qx_y_temp = check(qx_y + j,0,15);
      temp = (x1-*(x+16*qx_x_temp + qx_y_temp))*(x1-*(x+16*qx_x_temp + qx_y_temp)) + (y1-*(y+16*qx_x_temp + qx_y_temp))*(y1-*(y+16*qx_x_temp + qx_y_temp));
      
      if (temp < min){
	qx_x_clo = qx_x_temp;
	qx_y_clo = qx_y_temp;
	min = temp;
      }
    }
  }
  
  //get min and max for x and y
  if (qx_x_clo == 0){
    qx_x_min = 0;
    if (qx_y_clo == 0){
      qx_y_min = 0;
    }else if (qx_y_clo == 15){
      qx_y_min = 14;
    }else if (y1 < *(y+ 16*qx_x_clo + qx_y_clo)){
      qx_y_min = qx_y_clo;
    }else{
      qx_y_min = qx_y_clo - 1;
    }
  }else if (qx_x_clo == 8){
    qx_x_min = 7;
    if (qx_y_clo == 0){
      qx_y_min = 0;
    }else if (qx_y_clo == 15){
      qx_y_min = 14;
    }else if (y1 < *(y + 16*qx_x_clo + qx_y_clo)){
      qx_y_min = qx_y_clo;
    }else{
      qx_y_min = qx_y_clo - 1;
    }
  }else{
    if (x1 > *(x+16*qx_x_clo+qx_y_clo)){
      qx_x_min = qx_x_clo;
    }else{
      qx_x_min = qx_x_clo - 1;
    }
    if (qx_y_clo ==0){
      qx_y_min = 0;
    }else if (qx_y_clo == 15){
      qx_y_min = 14;
    }else if (y1 < *(y+16*qx_x_clo + qx_y_clo)){
      qx_y_min = qx_y_clo;
    }else{
      qx_y_min = qx_y_clo - 1;
    }
  }


  //get min and max for z
  Int_t qx_z_min;
  if (z1 <= *(z+16*7*qx_x_clo + 7*qx_y_clo + 1)){
    qx_z_min = 0;
  }else if (z1 > *(z+16*7*qx_x_clo + 7*qx_y_clo + 1)&& z1 <= *(z+16*7*qx_x_clo + 7*qx_y_clo + 2)){
    qx_z_min = 1;
  }else if (z1 > *(z+16*7*qx_x_clo + 7*qx_y_clo + 2) && z1 <= *(z+16*7*qx_x_clo + 7*qx_y_clo + 3)){
    qx_z_min = 2;
  }else if (z1 > *(z+16*7*qx_x_clo + 7*qx_y_clo + 3) && z1 <= *(z+16*7*qx_x_clo + 7*qx_y_clo + 4)){
    qx_z_min = 3;
  }else if (z1 > *(z+16*7*qx_x_clo + 7*qx_y_clo + 4) && z1 <= *(z+16*7*qx_x_clo + 7*qx_y_clo + 5)){
    qx_z_min = 4;
  }else{
    qx_z_min = 5;
  }
  

 
  Double_t final_corr = 0.;
  
  
  final_corr = *(corr + 16*7*qx_x_min + 7*qx_y_min + qx_z_min)       * 
    ( *(x + 16*(qx_x_min + 1)+qx_y_clo) - x1)/(*(x + 16*(qx_x_min + 1)+qx_y_clo) - *(x + 16*qx_x_min + qx_y_clo)) * 
    ( *(y + 16*qx_x_clo + qx_y_min + 1) - y1)/(*(y + 16*qx_x_clo + qx_y_min + 1) - *(y + 16*qx_x_clo + qx_y_min)) *
    ( *(z + 16*7*qx_x_clo + 7*qx_y_clo + qx_z_min + 1) - z1)/(*(z + 16*7*qx_x_clo + 7*qx_y_clo +qx_z_min + 1) - *(z + 16*7*qx_x_clo + 7*qx_y_clo + qx_z_min));
  final_corr += *(corr + 16*7*qx_x_min + 7*qx_y_min + qx_z_min + 1)       * 
    ( *(x + 16*(qx_x_min + 1)+qx_y_clo) - x1)/(*(x + 16*(qx_x_min + 1)+qx_y_clo) - *(x + 16*qx_x_min + qx_y_clo)) * 
    ( *(y + 16*qx_x_clo + qx_y_min + 1) - y1)/(*(y + 16*qx_x_clo + qx_y_min + 1) - *(y + 16*qx_x_clo + qx_y_min)) *
    ( - *(z + 16*7*qx_x_clo + 7*qx_y_clo + qx_z_min) + z1)/(*(z + 16*7*qx_x_clo + 7*qx_y_clo +qx_z_min + 1) - *(z + 16*7*qx_x_clo + 7*qx_y_clo + qx_z_min));
  final_corr += *(corr + 16*7*qx_x_min + 7*(qx_y_min+1) + qx_z_min)       * 
    ( *(x + 16*(qx_x_min + 1)+qx_y_clo) - x1)/(*(x + 16*(qx_x_min + 1)+qx_y_clo) - *(x + 16*qx_x_min + qx_y_clo)) * 
    ( -*(y + 16*qx_x_clo + qx_y_min) + y1)/(*(y + 16*qx_x_clo + qx_y_min + 1) - *(y + 16*qx_x_clo + qx_y_min)) *
    ( *(z + 16*7*qx_x_clo + 7*qx_y_clo + qx_z_min + 1) - z1)/(*(z + 16*7*qx_x_clo + 7*qx_y_clo +qx_z_min + 1) - *(z + 16*7*qx_x_clo + 7*qx_y_clo + qx_z_min));
  final_corr += *(corr + 16*7*qx_x_min + 7*(qx_y_min+1) + qx_z_min + 1)       * 
    ( *(x + 16*(qx_x_min + 1)+qx_y_clo) - x1)/(*(x + 16*(qx_x_min + 1)+qx_y_clo) - *(x + 16*qx_x_min + qx_y_clo)) * 
    ( -*(y + 16*qx_x_clo + qx_y_min) + y1)/(*(y + 16*qx_x_clo + qx_y_min + 1) - *(y + 16*qx_x_clo + qx_y_min)) *
    ( -*(z + 16*7*qx_x_clo + 7*qx_y_clo + qx_z_min) + z1)/(*(z + 16*7*qx_x_clo + 7*qx_y_clo +qx_z_min + 1) - *(z + 16*7*qx_x_clo + 7*qx_y_clo + qx_z_min));
  final_corr += *(corr + 16*7*(qx_x_min+1) + 7*qx_y_min + qx_z_min)       * 
    ( -*(x + 16*(qx_x_min)+qx_y_clo) + x1)/(*(x + 16*(qx_x_min + 1)+qx_y_clo) - *(x + 16*qx_x_min + qx_y_clo)) * 
    ( *(y + 16*qx_x_clo + qx_y_min + 1) - y1)/(*(y + 16*qx_x_clo + qx_y_min + 1) - *(y + 16*qx_x_clo + qx_y_min)) *
    ( *(z + 16*7*qx_x_clo + 7*qx_y_clo + qx_z_min + 1) - z1)/(*(z + 16*7*qx_x_clo + 7*qx_y_clo +qx_z_min + 1) - *(z + 16*7*qx_x_clo + 7*qx_y_clo + qx_z_min));
  final_corr += *(corr + 16*7*(qx_x_min+1) + 7*qx_y_min + qx_z_min+1)       * 
    ( -*(x + 16*(qx_x_min)+qx_y_clo) + x1)/(*(x + 16*(qx_x_min + 1)+qx_y_clo) - *(x + 16*qx_x_min + qx_y_clo)) * 
    ( *(y + 16*qx_x_clo + qx_y_min + 1) - y1)/(*(y + 16*qx_x_clo + qx_y_min + 1) - *(y + 16*qx_x_clo + qx_y_min)) *
    ( -*(z + 16*7*qx_x_clo + 7*qx_y_clo + qx_z_min) + z1)/(*(z + 16*7*qx_x_clo + 7*qx_y_clo +qx_z_min + 1) - *(z + 16*7*qx_x_clo + 7*qx_y_clo + qx_z_min));
  final_corr += *(corr + 16*7*(qx_x_min+1) + 7*(qx_y_min+1) + qx_z_min)       * 
    ( -*(x + 16*(qx_x_min)+qx_y_clo) + x1)/(*(x + 16*(qx_x_min + 1)+qx_y_clo) - *(x + 16*qx_x_min + qx_y_clo)) * 
    ( -*(y + 16*qx_x_clo + qx_y_min) + y1)/(*(y + 16*qx_x_clo + qx_y_min + 1) - *(y + 16*qx_x_clo + qx_y_min)) *
    ( *(z + 16*7*qx_x_clo + 7*qx_y_clo + qx_z_min + 1) - z1)/(*(z + 16*7*qx_x_clo + 7*qx_y_clo +qx_z_min + 1) - *(z + 16*7*qx_x_clo + 7*qx_y_clo + qx_z_min));
  final_corr += *(corr + 16*7*(qx_x_min+1) + 7*(qx_y_min+1) + qx_z_min+1)       * 
    ( -*(x + 16*(qx_x_min)+qx_y_clo) + x1)/(*(x + 16*(qx_x_min + 1)+qx_y_clo) - *(x + 16*qx_x_min + qx_y_clo)) * 
    ( -*(y + 16*qx_x_clo + qx_y_min) + y1)/(*(y + 16*qx_x_clo + qx_y_min + 1) - *(y + 16*qx_x_clo + qx_y_min)) *
    ( -*(z + 16*7*qx_x_clo + 7*qx_y_clo + qx_z_min) + z1)/(*(z + 16*7*qx_x_clo + 7*qx_y_clo +qx_z_min + 1) - *(z + 16*7*qx_x_clo + 7*qx_y_clo + qx_z_min));

  return final_corr;
}


Double_t THaOpticsE06010::sievecorr(Double_t x1, Double_t y1, Double_t *x, Double_t *y, Double_t *corr){
  
  // x1 = -0.050018; y1=-0.100294;
  // first get a feeling of where is x and y
  Int_t qx_x,qx_y;
  qx_x = Int_t((x1+0.1)/0.01);
  qx_x = check(qx_x,0,21);
  qx_y = Int_t((y1+1.938*5.*2.54/100.)/(1.938*2.54/100.));
  qx_y = check(qx_y,0,11);

  // get the cloest one, small one, large one for x, y;
  Int_t qx_x_clo=0,qx_x_min;
  Int_t qx_y_clo=0,qx_y_min;
  
  // get closest one first
  Int_t qx_x_temp,qx_y_temp;  
  Double_t min = 1000000.,temp;
  for (Int_t i=-1;i!=2;i++){
    for (Int_t j=-1;j!=2;j++){
      qx_x_temp = check(qx_x + i,0,21);
      qx_y_temp = check(qx_y + j,0,11);
      temp = (x1-*(x+12*qx_x_temp + qx_y_temp))*(x1-*(x+12*qx_x_temp + qx_y_temp)) + (y1-*(y+12*qx_x_temp + qx_y_temp))*(y1-*(y+12*qx_x_temp + qx_y_temp));
      if (temp < min){
	qx_x_clo = qx_x_temp;
	qx_y_clo = qx_y_temp;
	min = temp;
      }
    }
  }
  //get min and max for x and y
  if (qx_x_clo == 0){
    qx_x_min = 0;
    if (qx_y_clo == 0){
      qx_y_min = 0;
    }else if (qx_y_clo == 11){
      qx_y_min = 10;
    }else if (y1 > *(y+ 12*qx_x_clo + qx_y_clo)){
      qx_y_min = qx_y_clo;
    }else{
      qx_y_min = qx_y_clo - 1;
    }
  }else if (qx_x_clo == 21){
    qx_x_min = 20;
    if (qx_y_clo == 0){
      qx_y_min = 0;
    }else if (qx_y_clo == 11){
      qx_y_min = 10;
    }else if (y1 > *(y + 12*qx_x_clo + qx_y_clo)){
      qx_y_min = qx_y_clo;
    }else{
      qx_y_min = qx_y_clo - 1;
    }
  }else{
    if (x1 > *(x+12*qx_x_clo+qx_y_clo)){
      qx_x_min = qx_x_clo;
    }else{
      qx_x_min = qx_x_clo - 1;
    }
    if (qx_y_clo ==0){
      qx_y_min = 0;
    }else if (qx_y_clo == 11){
      qx_y_min = 10;
    }else if (y1 > *(y+12*qx_x_clo + qx_y_clo)){
      qx_y_min = qx_y_clo;
    }else{
      qx_y_min = qx_y_clo - 1;
    }
  }
  
  //qx_x_min = 14; qx_x_clo = 14; qx_y_min =0 ; qx_y_clo = 1;

  Double_t final_corr = 0.;
  //cout << qx_x_min << "\t" << qx_x_clo << "\t" << qx_y_min << "\t" << qx_y_clo << "\t" << x1 << "\t" << y1 << endl;
  final_corr += *(corr + 12*qx_x_min + qx_y_min) *
    ( *(x + 12*(qx_x_min + 1)+qx_y_clo) - x1)/(*(x + 12*(qx_x_min + 1)+qx_y_clo) - *(x + 12*qx_x_min + qx_y_clo)) * 
    ( *(y + 12*qx_x_clo + qx_y_min + 1) - y1)/(*(y + 12*qx_x_clo + qx_y_min + 1) - *(y + 12*qx_x_clo + qx_y_min))
  + *(corr + 12*qx_x_min + qx_y_min + 1) *
    ( *(x + 12*(qx_x_min + 1)+qx_y_clo) - x1)/(*(x + 12*(qx_x_min + 1)+qx_y_clo) - *(x + 12*qx_x_min + qx_y_clo)) * 
    ( - *(y + 12*qx_x_clo + qx_y_min) + y1)/(*(y + 12*qx_x_clo + qx_y_min + 1) - *(y + 12*qx_x_clo + qx_y_min))
  + *(corr + 12*(qx_x_min+1) + qx_y_min) *
    ( - *(x + 12*qx_x_min+qx_y_clo) + x1)/(*(x + 12*(qx_x_min + 1)+qx_y_clo) - *(x + 12*qx_x_min + qx_y_clo)) * 
    ( *(y + 12*qx_x_clo + qx_y_min + 1) - y1)/(*(y + 12*qx_x_clo + qx_y_min + 1) - *(y + 12*qx_x_clo + qx_y_min))
  + *(corr + 12*(qx_x_min+1) + qx_y_min+1) *   
    ( -*(x + 12*qx_x_min+qx_y_clo) + x1)/(*(x + 12*(qx_x_min + 1)+qx_y_clo) - *(x + 12*qx_x_min + qx_y_clo)) * 
    ( -*(y + 12*qx_x_clo + qx_y_min) + y1)/(*(y + 12*qx_x_clo + qx_y_min + 1) - *(y + 12*qx_x_clo + qx_y_min));
  //cout << qx_x_min << "\t" << qx_x_clo << "\t" << qx_y_min << "\t" << qx_y_clo << "\t" << x1 << "\t" << y1 << endl;
  //cout << x1<< "\t" << y1 << "\t" << final_corr << endl;
  return final_corr;
}


Double_t THaOpticsE06010::momcorr1(Double_t x1, Double_t y1, Double_t *x, Double_t *y, Double_t *corr){
  
  Int_t qx_x,qx_y;
  qx_x = Int_t((x1 + 0.08)/0.02 + 1);
  qx_x = check(qx_x,0,10);
  qx_y = Int_t((y1 + 0.52)/0.04);
  qx_y = check(qx_y,0,23);

 

  // get the cloest one, small one, large one for x, y;
  Int_t qx_x_clo=0,qx_x_min;
  Int_t qx_y_clo=0,qx_y_min;
  
  // get closest one first
  Int_t qx_x_temp,qx_y_temp;  
  Double_t min = 1000000.,temp;
  for (Int_t i=-1;i!=2;i++){
    for (Int_t j=-1;j!=2;j++){
      qx_x_temp = check(qx_x + i,0,10);
      qx_y_temp = check(qx_y + j,0,23);
      temp = (x1-*(x+24*qx_x_temp + qx_y_temp))*(x1-*(x+24*qx_x_temp + qx_y_temp)) + (y1-*(y+24*qx_x_temp + qx_y_temp))*(y1-*(y+24*qx_x_temp + qx_y_temp));
      if (temp < min){
	qx_x_clo = qx_x_temp;
	qx_y_clo = qx_y_temp;
	min = temp;
      }
    }
  }
  //cout <<  *(x + 24*qx_x_min+qx_y_clo)  << "\t" << x1 << "\t" << *(y+24*qx_x_temp + qx_y_temp) << "\t" << y1 << endl;
  
  //get min and max for x and y
  if (qx_x_clo == 0){
    qx_x_min = 0;
    if (qx_y_clo == 0){
      qx_y_min = 0;
    }else if (qx_y_clo == 23){
      qx_y_min = 22;
    }else if (y1 > *(y+ 24*qx_x_clo + qx_y_clo)){
      qx_y_min = qx_y_clo;
    }else{
      qx_y_min = qx_y_clo - 1;
    }
  }else if (qx_x_clo == 10){
    qx_x_min = 9;
    if (qx_y_clo == 0){
      qx_y_min = 0;
    }else if (qx_y_clo == 23){
      qx_y_min = 22;
    }else if (y1 > *(y + 24*qx_x_clo + qx_y_clo)){
      qx_y_min = qx_y_clo;
    }else{
      qx_y_min = qx_y_clo - 1;
    }
  }else{
    if (x1 > *(x+24*qx_x_clo+qx_y_clo)){
      qx_x_min = qx_x_clo;
    }else{
      qx_x_min = qx_x_clo - 1;
    }
    if (qx_y_clo ==0){
      qx_y_min = 0;
    }else if (qx_y_clo == 23){
      qx_y_min = 22;
    }else if (y1 > *(y+24*qx_x_clo + qx_y_clo)){
      qx_y_min = qx_y_clo;
    }else{
      qx_y_min = qx_y_clo - 1;
    }
  }

  
  
  Double_t final_corr = 0.;
  final_corr += *(corr + 24*qx_x_min + qx_y_min) *
    ( *(x + 24*(qx_x_min + 1)+qx_y_clo) - x1)/(*(x + 24*(qx_x_min + 1)+qx_y_clo) - *(x + 24*qx_x_min + qx_y_clo)) * 
    ( *(y + 24*qx_x_clo + qx_y_min + 1) - y1)/(*(y + 24*qx_x_clo + qx_y_min + 1) - *(y + 24*qx_x_clo + qx_y_min))
  + *(corr + 24*qx_x_min + qx_y_min + 1) *
    ( *(x + 24*(qx_x_min + 1)+qx_y_clo) - x1)/(*(x + 24*(qx_x_min + 1)+qx_y_clo) - *(x + 24*qx_x_min + qx_y_clo)) * 
    ( - *(y + 24*qx_x_clo + qx_y_min) + y1)/(*(y + 24*qx_x_clo + qx_y_min + 1) - *(y + 24*qx_x_clo + qx_y_min))
  + *(corr + 24*(qx_x_min+1) + qx_y_min) *
    ( - *(x + 24*qx_x_min+qx_y_clo) + x1)/(*(x + 24*(qx_x_min + 1)+qx_y_clo) - *(x + 24*qx_x_min + qx_y_clo)) * 
    ( *(y + 24*qx_x_clo + qx_y_min + 1) - y1)/(*(y + 24*qx_x_clo + qx_y_min + 1) - *(y + 24*qx_x_clo + qx_y_min))
  + *(corr + 24*(qx_x_min+1) + qx_y_min+1) *   
    ( -*(x + 24*qx_x_min+qx_y_clo) + x1)/(*(x + 24*(qx_x_min + 1)+qx_y_clo) - *(x + 24*qx_x_min + qx_y_clo)) * 
    ( -*(y + 24*qx_x_clo + qx_y_min) + y1)/(*(y + 24*qx_x_clo + qx_y_min + 1) - *(y + 24*qx_x_clo + qx_y_min));
  
  
  
  
  return final_corr;
}


Int_t THaOpticsE06010::check(Int_t qx, Int_t qx1, Int_t qx2){
  if (qx < qx1){
    qx = qx1;
  }else if (qx > qx2){
    qx = qx2;
  }
  return qx;
}

Double_t THaOpticsE06010::sndcorr_p(Double_t x1, Double_t y1 , Double_t z1, Double_t *x, Double_t *y, Double_t *z, Double_t *corr){
  // first get a feeling of where is x and y
  Int_t qx_x,qx_y;
  qx_x = Int_t((x1+0.1)/0.02);
  qx_x = check(qx_x,0,11);
  qx_y = Int_t((0.44-y1)/0.02);
  qx_y = check(qx_y,0,25);

  // get the cloest one, small one, large one for x, y, and z;
  Int_t qx_x_clo=0,qx_x_min;
  Int_t qx_y_clo=0,qx_y_min;
  
  // get closest one first
  Int_t qx_x_temp,qx_y_temp;  
  Double_t min = 1000000.,temp;
  for (Int_t i=-1;i!=2;i++){
    for (Int_t j=-1;j!=2;j++){
      qx_x_temp = check(qx_x + i,0,11);
      qx_y_temp = check(qx_y + j,0,25);
      temp = (x1-*(x+26*qx_x_temp + qx_y_temp))*(x1-*(x+26*qx_x_temp + qx_y_temp)) + (y1-*(y+26*qx_x_temp + qx_y_temp))*(y1-*(y+26*qx_x_temp + qx_y_temp));
      if (temp < min){
	qx_x_clo = qx_x_temp;
	qx_y_clo = qx_y_temp;
	min = temp;
      }
    }
  }
  
  //get min and max for x and y
  if (qx_x_clo == 0){
    qx_x_min = 0;
    if (qx_y_clo == 0){
      qx_y_min = 0;
    }else if (qx_y_clo == 25){
      qx_y_min = 24;
    }else if (y1 < *(y+ 26*qx_x_clo + qx_y_clo)){
      qx_y_min = qx_y_clo;
    }else{
      qx_y_min = qx_y_clo - 1;
    }
  }else if (qx_x_clo == 11){
    qx_x_min = 10;
    if (qx_y_clo == 0){
      qx_y_min = 0;
    }else if (qx_y_clo == 25){
      qx_y_min = 24;
    }else if (y1 < *(y + 26*qx_x_clo + qx_y_clo)){
      qx_y_min = qx_y_clo;
    }else{
      qx_y_min = qx_y_clo - 1;
    }
  }else{
    if (x1 > *(x+26*qx_x_clo+qx_y_clo)){
      qx_x_min = qx_x_clo;
    }else{
      qx_x_min = qx_x_clo - 1;
    }
    if (qx_y_clo ==0){
      qx_y_min = 0;
    }else if (qx_y_clo == 25){
      qx_y_min = 24;
    }else if (y1 < *(y+26*qx_x_clo + qx_y_clo)){
      qx_y_min = qx_y_clo;
    }else{
      qx_y_min = qx_y_clo - 1;
    }
  }
  
  //get min and max for z
  Int_t qx_z_min;
  if (z1 <= *(z+26*5*qx_x_clo + 5*qx_y_clo + 1)){
    qx_z_min = 0;
  }else if (z1 > *(z+26*5*qx_x_clo + 5*qx_y_clo + 1)&& z1 <= *(z+26*5*qx_x_clo + 5*qx_y_clo + 2)){
    qx_z_min = 1;
  }else if (z1 > *(z+26*5*qx_x_clo + 5*qx_y_clo + 2) && z1 <= *(z+26*5*qx_x_clo + 5*qx_y_clo + 3)){
    qx_z_min = 2;
  }else{
    qx_z_min = 3;
  }
  
  Double_t final_corr = 0.;
  
  final_corr = *(corr + 26*5*qx_x_min + 5*qx_y_min + qx_z_min)       * 
    ( *(x + 26*(qx_x_min + 1)+qx_y_clo) - x1)/(*(x + 26*(qx_x_min + 1)+qx_y_clo) - *(x + 26*qx_x_min + qx_y_clo)) * 
    ( *(y + 26*qx_x_clo + qx_y_min + 1) - y1)/(*(y + 26*qx_x_clo + qx_y_min + 1) - *(y + 26*qx_x_clo + qx_y_min)) *
    ( *(z + 26*5*qx_x_clo + 5*qx_y_clo + qx_z_min + 1) - z1)/(*(z + 26*5*qx_x_clo + 5*qx_y_clo +qx_z_min + 1) - *(z + 26*5*qx_x_clo + 5*qx_y_clo + qx_z_min));
  final_corr += *(corr + 26*5*qx_x_min + 5*qx_y_min + qx_z_min + 1)       * 
    ( *(x + 26*(qx_x_min + 1)+qx_y_clo) - x1)/(*(x + 26*(qx_x_min + 1)+qx_y_clo) - *(x + 26*qx_x_min + qx_y_clo)) * 
    ( *(y + 26*qx_x_clo + qx_y_min + 1) - y1)/(*(y + 26*qx_x_clo + qx_y_min + 1) - *(y + 26*qx_x_clo + qx_y_min)) *
    ( - *(z + 26*5*qx_x_clo + 5*qx_y_clo + qx_z_min) + z1)/(*(z + 26*5*qx_x_clo + 5*qx_y_clo +qx_z_min + 1) - *(z + 26*5*qx_x_clo + 5*qx_y_clo + qx_z_min));
  final_corr += *(corr + 26*5*qx_x_min + 5*(qx_y_min+1) + qx_z_min)       * 
    ( *(x + 26*(qx_x_min + 1)+qx_y_clo) - x1)/(*(x + 26*(qx_x_min + 1)+qx_y_clo) - *(x + 26*qx_x_min + qx_y_clo)) * 
    ( -*(y + 26*qx_x_clo + qx_y_min) + y1)/(*(y + 26*qx_x_clo + qx_y_min + 1) - *(y + 26*qx_x_clo + qx_y_min)) *
    ( *(z + 26*5*qx_x_clo + 5*qx_y_clo + qx_z_min + 1) - z1)/(*(z + 26*5*qx_x_clo + 5*qx_y_clo +qx_z_min + 1) - *(z + 26*5*qx_x_clo + 5*qx_y_clo + qx_z_min));
  final_corr += *(corr + 26*5*qx_x_min + 5*(qx_y_min+1) + qx_z_min + 1)       * 
    ( *(x + 26*(qx_x_min + 1)+qx_y_clo) - x1)/(*(x + 26*(qx_x_min + 1)+qx_y_clo) - *(x + 26*qx_x_min + qx_y_clo)) * 
    ( -*(y + 26*qx_x_clo + qx_y_min) + y1)/(*(y + 26*qx_x_clo + qx_y_min + 1) - *(y + 26*qx_x_clo + qx_y_min)) *
    ( -*(z + 26*5*qx_x_clo + 5*qx_y_clo + qx_z_min) + z1)/(*(z + 26*5*qx_x_clo + 5*qx_y_clo +qx_z_min + 1) - *(z + 26*5*qx_x_clo + 5*qx_y_clo + qx_z_min));
  final_corr += *(corr + 26*5*(qx_x_min+1) + 5*qx_y_min + qx_z_min)       * 
    ( -*(x + 26*(qx_x_min)+qx_y_clo) + x1)/(*(x + 26*(qx_x_min + 1)+qx_y_clo) - *(x + 26*qx_x_min + qx_y_clo)) * 
    ( *(y + 26*qx_x_clo + qx_y_min + 1) - y1)/(*(y + 26*qx_x_clo + qx_y_min + 1) - *(y + 26*qx_x_clo + qx_y_min)) *
    ( *(z + 26*5*qx_x_clo + 5*qx_y_clo + qx_z_min + 1) - z1)/(*(z + 26*5*qx_x_clo + 5*qx_y_clo +qx_z_min + 1) - *(z + 26*5*qx_x_clo + 5*qx_y_clo + qx_z_min));
  final_corr += *(corr + 26*5*(qx_x_min+1) + 5*qx_y_min + qx_z_min+1)       * 
    ( -*(x + 26*(qx_x_min)+qx_y_clo) + x1)/(*(x + 26*(qx_x_min + 1)+qx_y_clo) - *(x + 26*qx_x_min + qx_y_clo)) * 
    ( *(y + 26*qx_x_clo + qx_y_min + 1) - y1)/(*(y + 26*qx_x_clo + qx_y_min + 1) - *(y + 26*qx_x_clo + qx_y_min)) *
    ( -*(z + 26*5*qx_x_clo + 5*qx_y_clo + qx_z_min) + z1)/(*(z + 26*5*qx_x_clo + 5*qx_y_clo +qx_z_min + 1) - *(z + 26*5*qx_x_clo + 5*qx_y_clo + qx_z_min));
  final_corr += *(corr + 26*5*(qx_x_min+1) + 5*(qx_y_min+1) + qx_z_min)       * 
    ( -*(x + 26*(qx_x_min)+qx_y_clo) + x1)/(*(x + 26*(qx_x_min + 1)+qx_y_clo) - *(x + 26*qx_x_min + qx_y_clo)) * 
    ( -*(y + 26*qx_x_clo + qx_y_min) + y1)/(*(y + 26*qx_x_clo + qx_y_min + 1) - *(y + 26*qx_x_clo + qx_y_min)) *
    ( *(z + 26*5*qx_x_clo + 5*qx_y_clo + qx_z_min + 1) - z1)/(*(z + 26*5*qx_x_clo + 5*qx_y_clo +qx_z_min + 1) - *(z + 26*5*qx_x_clo + 5*qx_y_clo + qx_z_min));
  final_corr += *(corr + 26*5*(qx_x_min+1) + 5*(qx_y_min+1) + qx_z_min+1)       * 
    ( -*(x + 26*(qx_x_min)+qx_y_clo) + x1)/(*(x + 26*(qx_x_min + 1)+qx_y_clo) - *(x + 26*qx_x_min + qx_y_clo)) * 
    ( -*(y + 26*qx_x_clo + qx_y_min) + y1)/(*(y + 26*qx_x_clo + qx_y_min + 1) - *(y + 26*qx_x_clo + qx_y_min)) *
    ( -*(z + 26*5*qx_x_clo + 5*qx_y_clo + qx_z_min) + z1)/(*(z + 26*5*qx_x_clo + 5*qx_y_clo +qx_z_min + 1) - *(z + 26*5*qx_x_clo + 5*qx_y_clo + qx_z_min));

  return final_corr;
}


Double_t THaOpticsE06010::foucorr_p(Double_t x1, Double_t y1 , Double_t z1, Double_t *x, Double_t *y, Double_t *z, Double_t *corr){
  // first get a feeling of where is x and y
  Int_t qx_x=0,qx_y;
  Float_t par[12]={0,3.0,3.3,3.5,3.7,3.9,4.1,4.3,4.5,4.7,5.0,100};
  for (Int_t i=0;i!=11;i++){
    if (x1 >= par[i] && x1 <=par[i+1]){
      qx_x = i;
    }else if (x1 >= par[11]){
      qx_x = 10;
    }else if (x1 < par[0]){
      qx_x = 0;
    }
  }

  

  qx_y = Int_t((0.13-y1)/0.017);
  qx_y = check(qx_y,0,15);
  
  
  // get the cloest one, small one, large one for x, y, and z;
  Int_t qx_x_clo=0,qx_x_min;
  Int_t qx_y_clo=0,qx_y_min;
  
  // get closest one first
  Int_t qx_x_temp,qx_y_temp;  
  Double_t min = 1000000.,temp;
  
  for (Int_t i=-1;i!=2;i++){
    for (Int_t j=-1;j!=2;j++){
      qx_x_temp = check(qx_x + i,0,10);
      qx_y_temp = check(qx_y + j,0,15);
      temp = (x1-*(x+16*qx_x_temp + qx_y_temp))*(x1-*(x+16*qx_x_temp + qx_y_temp)) + (y1-*(y+16*qx_x_temp + qx_y_temp))*(y1-*(y+16*qx_x_temp + qx_y_temp));
      
      if (temp < min){
	qx_x_clo = qx_x_temp;
	qx_y_clo = qx_y_temp;
	min = temp;
      }
    }
  }
  
  //get min and max for x and y
  if (qx_x_clo == 0){
    qx_x_min = 0;
    if (qx_y_clo == 0){
      qx_y_min = 0;
    }else if (qx_y_clo == 15){
      qx_y_min = 14;
    }else if (y1 < *(y+ 16*qx_x_clo + qx_y_clo)){
      qx_y_min = qx_y_clo;
    }else{
      qx_y_min = qx_y_clo - 1;
    }
  }else if (qx_x_clo == 10){
    qx_x_min = 9;
    if (qx_y_clo == 0){
      qx_y_min = 0;
    }else if (qx_y_clo == 15){
      qx_y_min = 14;
    }else if (y1 < *(y + 16*qx_x_clo + qx_y_clo)){
      qx_y_min = qx_y_clo;
    }else{
      qx_y_min = qx_y_clo - 1;
    }
  }else{
    if (x1 > *(x+16*qx_x_clo+qx_y_clo)){
      qx_x_min = qx_x_clo;
    }else{
      qx_x_min = qx_x_clo - 1;
    }
    if (qx_y_clo ==0){
      qx_y_min = 0;
    }else if (qx_y_clo == 15){
      qx_y_min = 14;
    }else if (y1 < *(y+16*qx_x_clo + qx_y_clo)){
      qx_y_min = qx_y_clo;
    }else{
      qx_y_min = qx_y_clo - 1;
    }
  }


  //get min and max for z
  Int_t qx_z_min;
  if (z1 <= *(z+16*5*qx_x_clo + 5*qx_y_clo + 1)){
    qx_z_min = 0;
  }else if (z1 > *(z+16*5*qx_x_clo + 5*qx_y_clo + 1)&& z1 <= *(z+16*5*qx_x_clo + 5*qx_y_clo + 2)){
    qx_z_min = 1;
  }else if (z1 > *(z+16*5*qx_x_clo + 5*qx_y_clo + 2) && z1 <= *(z+16*5*qx_x_clo + 5*qx_y_clo + 3)){
    qx_z_min = 2;
  }else {
    qx_z_min = 3;
  }
  

 
  Double_t final_corr = 0.;
  
  
  final_corr = *(corr + 16*5*qx_x_min + 5*qx_y_min + qx_z_min)       * 
    ( *(x + 16*(qx_x_min + 1)+qx_y_clo) - x1)/(*(x + 16*(qx_x_min + 1)+qx_y_clo) - *(x + 16*qx_x_min + qx_y_clo)) * 
    ( *(y + 16*qx_x_clo + qx_y_min + 1) - y1)/(*(y + 16*qx_x_clo + qx_y_min + 1) - *(y + 16*qx_x_clo + qx_y_min)) *
    ( *(z + 16*5*qx_x_clo + 5*qx_y_clo + qx_z_min + 1) - z1)/(*(z + 16*5*qx_x_clo + 5*qx_y_clo +qx_z_min + 1) - *(z + 16*5*qx_x_clo + 5*qx_y_clo + qx_z_min));
  final_corr += *(corr + 16*5*qx_x_min + 5*qx_y_min + qx_z_min + 1)       * 
    ( *(x + 16*(qx_x_min + 1)+qx_y_clo) - x1)/(*(x + 16*(qx_x_min + 1)+qx_y_clo) - *(x + 16*qx_x_min + qx_y_clo)) * 
    ( *(y + 16*qx_x_clo + qx_y_min + 1) - y1)/(*(y + 16*qx_x_clo + qx_y_min + 1) - *(y + 16*qx_x_clo + qx_y_min)) *
    ( - *(z + 16*5*qx_x_clo + 5*qx_y_clo + qx_z_min) + z1)/(*(z + 16*5*qx_x_clo + 5*qx_y_clo +qx_z_min + 1) - *(z + 16*5*qx_x_clo + 5*qx_y_clo + qx_z_min));
  final_corr += *(corr + 16*5*qx_x_min + 5*(qx_y_min+1) + qx_z_min)       * 
    ( *(x + 16*(qx_x_min + 1)+qx_y_clo) - x1)/(*(x + 16*(qx_x_min + 1)+qx_y_clo) - *(x + 16*qx_x_min + qx_y_clo)) * 
    ( -*(y + 16*qx_x_clo + qx_y_min) + y1)/(*(y + 16*qx_x_clo + qx_y_min + 1) - *(y + 16*qx_x_clo + qx_y_min)) *
    ( *(z + 16*5*qx_x_clo + 5*qx_y_clo + qx_z_min + 1) - z1)/(*(z + 16*5*qx_x_clo + 5*qx_y_clo +qx_z_min + 1) - *(z + 16*5*qx_x_clo + 5*qx_y_clo + qx_z_min));
  final_corr += *(corr + 16*5*qx_x_min + 5*(qx_y_min+1) + qx_z_min + 1)       * 
    ( *(x + 16*(qx_x_min + 1)+qx_y_clo) - x1)/(*(x + 16*(qx_x_min + 1)+qx_y_clo) - *(x + 16*qx_x_min + qx_y_clo)) * 
    ( -*(y + 16*qx_x_clo + qx_y_min) + y1)/(*(y + 16*qx_x_clo + qx_y_min + 1) - *(y + 16*qx_x_clo + qx_y_min)) *
    ( -*(z + 16*5*qx_x_clo + 5*qx_y_clo + qx_z_min) + z1)/(*(z + 16*5*qx_x_clo + 5*qx_y_clo +qx_z_min + 1) - *(z + 16*5*qx_x_clo + 5*qx_y_clo + qx_z_min));
  final_corr += *(corr + 16*5*(qx_x_min+1) + 5*qx_y_min + qx_z_min)       * 
    ( -*(x + 16*(qx_x_min)+qx_y_clo) + x1)/(*(x + 16*(qx_x_min + 1)+qx_y_clo) - *(x + 16*qx_x_min + qx_y_clo)) * 
    ( *(y + 16*qx_x_clo + qx_y_min + 1) - y1)/(*(y + 16*qx_x_clo + qx_y_min + 1) - *(y + 16*qx_x_clo + qx_y_min)) *
    ( *(z + 16*5*qx_x_clo + 5*qx_y_clo + qx_z_min + 1) - z1)/(*(z + 16*5*qx_x_clo + 5*qx_y_clo +qx_z_min + 1) - *(z + 16*5*qx_x_clo + 5*qx_y_clo + qx_z_min));
  final_corr += *(corr + 16*5*(qx_x_min+1) + 5*qx_y_min + qx_z_min+1)       * 
    ( -*(x + 16*(qx_x_min)+qx_y_clo) + x1)/(*(x + 16*(qx_x_min + 1)+qx_y_clo) - *(x + 16*qx_x_min + qx_y_clo)) * 
    ( *(y + 16*qx_x_clo + qx_y_min + 1) - y1)/(*(y + 16*qx_x_clo + qx_y_min + 1) - *(y + 16*qx_x_clo + qx_y_min)) *
    ( -*(z + 16*5*qx_x_clo + 5*qx_y_clo + qx_z_min) + z1)/(*(z + 16*5*qx_x_clo + 5*qx_y_clo +qx_z_min + 1) - *(z + 16*5*qx_x_clo + 5*qx_y_clo + qx_z_min));
  final_corr += *(corr + 16*5*(qx_x_min+1) + 5*(qx_y_min+1) + qx_z_min)       * 
    ( -*(x + 16*(qx_x_min)+qx_y_clo) + x1)/(*(x + 16*(qx_x_min + 1)+qx_y_clo) - *(x + 16*qx_x_min + qx_y_clo)) * 
    ( -*(y + 16*qx_x_clo + qx_y_min) + y1)/(*(y + 16*qx_x_clo + qx_y_min + 1) - *(y + 16*qx_x_clo + qx_y_min)) *
    ( *(z + 16*5*qx_x_clo + 5*qx_y_clo + qx_z_min + 1) - z1)/(*(z + 16*5*qx_x_clo + 5*qx_y_clo +qx_z_min + 1) - *(z + 16*5*qx_x_clo + 5*qx_y_clo + qx_z_min));
  final_corr += *(corr + 16*5*(qx_x_min+1) + 5*(qx_y_min+1) + qx_z_min+1)       * 
    ( -*(x + 16*(qx_x_min)+qx_y_clo) + x1)/(*(x + 16*(qx_x_min + 1)+qx_y_clo) - *(x + 16*qx_x_min + qx_y_clo)) * 
    ( -*(y + 16*qx_x_clo + qx_y_min) + y1)/(*(y + 16*qx_x_clo + qx_y_min + 1) - *(y + 16*qx_x_clo + qx_y_min)) *
    ( -*(z + 16*5*qx_x_clo + 5*qx_y_clo + qx_z_min) + z1)/(*(z + 16*5*qx_x_clo + 5*qx_y_clo +qx_z_min + 1) - *(z + 16*5*qx_x_clo + 5*qx_y_clo + qx_z_min));

  return final_corr;
}


Double_t THaOpticsE06010::momcorr1_p(Double_t x1, Double_t y1, Double_t *x, Double_t *y, Double_t *corr){
  
  Int_t qx_x,qx_y;
  qx_x = Int_t((x1 + 0.08)/0.02 + 1);
  qx_x = check(qx_x,0,11);
  qx_y = Int_t((y1 + 0.04)/0.04);
  qx_y = check(qx_y,0,11);

  // get the cloest one, small one, large one for x, y;
  Int_t qx_x_clo=0,qx_x_min;
  Int_t qx_y_clo=0,qx_y_min;
  
  // get closest one first
  Int_t qx_x_temp,qx_y_temp;  
  Double_t min = 1000000.,temp;
  for (Int_t i=-1;i!=2;i++){
    for (Int_t j=-1;j!=2;j++){
      qx_x_temp = check(qx_x + i,0,11);
      qx_y_temp = check(qx_y + j,0,11);
      temp = (x1-*(x+12*qx_x_temp + qx_y_temp))*(x1-*(x+12*qx_x_temp + qx_y_temp)) + (y1-*(y+12*qx_x_temp + qx_y_temp))*(y1-*(y+12*qx_x_temp + qx_y_temp));
      if (temp < min){
	qx_x_clo = qx_x_temp;
	qx_y_clo = qx_y_temp;
	min = temp;
      }
    }
  }
  //cout <<  qx_x_clo << "\t" << qx_y_clo  << "\t" << *(x + 12*qx_x_clo+qx_y_clo)  << "\t" << x1 << "\t" << *(y+12*qx_x_clo + qx_y_clo) << "\t" << y1 << endl;
  
  //get min and max for x and y
  if (qx_x_clo == 0){
    qx_x_min = 0;
    if (qx_y_clo == 0){
      qx_y_min = 0;
    }else if (qx_y_clo == 11){
      qx_y_min = 10;
    }else if (y1 > *(y+ 12*qx_x_clo + qx_y_clo)){
      qx_y_min = qx_y_clo;
    }else{
      qx_y_min = qx_y_clo - 1;
    }
  }else if (qx_x_clo == 11){
    qx_x_min = 10;
    if (qx_y_clo == 0){
      qx_y_min = 0;
    }else if (qx_y_clo == 11){
      qx_y_min = 10;
    }else if (y1 > *(y + 12*qx_x_clo + qx_y_clo)){
      qx_y_min = qx_y_clo;
    }else{
      qx_y_min = qx_y_clo - 1;
    }
  }else{
    if (x1 > *(x+12*qx_x_clo+qx_y_clo)){
      qx_x_min = qx_x_clo;
    }else{
      qx_x_min = qx_x_clo - 1;
    }
    if (qx_y_clo ==0){
      qx_y_min = 0;
    }else if (qx_y_clo == 11){
      qx_y_min = 10;
    }else if (y1 > *(y+12*qx_x_clo + qx_y_clo)){
      qx_y_min = qx_y_clo;
    }else{
      qx_y_min = qx_y_clo - 1;
    }
  }

  
  
  Double_t final_corr = 0.;
  final_corr += *(corr + 12*qx_x_min + qx_y_min) *
    ( *(x + 12*(qx_x_min + 1)+qx_y_clo) - x1)/(*(x + 12*(qx_x_min + 1)+qx_y_clo) - *(x + 12*qx_x_min + qx_y_clo)) * 
    ( *(y + 12*qx_x_clo + qx_y_min + 1) - y1)/(*(y + 12*qx_x_clo + qx_y_min + 1) - *(y + 12*qx_x_clo + qx_y_min))
  + *(corr + 12*qx_x_min + qx_y_min + 1) *
    ( *(x + 12*(qx_x_min + 1)+qx_y_clo) - x1)/(*(x + 12*(qx_x_min + 1)+qx_y_clo) - *(x + 12*qx_x_min + qx_y_clo)) * 
    ( - *(y + 12*qx_x_clo + qx_y_min) + y1)/(*(y + 12*qx_x_clo + qx_y_min + 1) - *(y + 12*qx_x_clo + qx_y_min))
  + *(corr + 12*(qx_x_min+1) + qx_y_min) *
    ( - *(x + 12*qx_x_min+qx_y_clo) + x1)/(*(x + 12*(qx_x_min + 1)+qx_y_clo) - *(x + 12*qx_x_min + qx_y_clo)) * 
    ( *(y + 12*qx_x_clo + qx_y_min + 1) - y1)/(*(y + 12*qx_x_clo + qx_y_min + 1) - *(y + 12*qx_x_clo + qx_y_min))
  + *(corr + 12*(qx_x_min+1) + qx_y_min+1) *   
    ( -*(x + 12*qx_x_min+qx_y_clo) + x1)/(*(x + 12*(qx_x_min + 1)+qx_y_clo) - *(x + 12*qx_x_min + qx_y_clo)) * 
    ( -*(y + 12*qx_x_clo + qx_y_min) + y1)/(*(y + 12*qx_x_clo + qx_y_min + 1) - *(y + 12*qx_x_clo + qx_y_min));
  
  
  
  
  return final_corr;
}










//_____________________________________________________________________________
Int_t THaOpticsE06010::ApplyOptics(THaTrack *track)
{
    // gateway function. calculation optics for *track, and fill result back to it.
    // it will use the beam position provided by detector with name fDefBeamName,
    // as long as fDefBeamName is not NULL
    //
    // The calculation is reading in track X,Y,Theta,Phi in TRCS and fill in any 
    // thing else in THaTrack

    TVector3 beampos(0.0, 0.0, 0.0);
    TVector3 beamdir(0.0, 0.0, 1.0);
    if( fBeam.IsValid() ) {
      ((THaUnRasteredBeam *)(fBeam.GetObject()))->Reconstruct(); 
        beampos = ((THaBeam *)(fBeam.GetObject())) ->GetPosition();
        beamdir = ((THaBeam *)(fBeam.GetObject())) ->GetDirection();
    }
    else 
    {
#if DEBUG_LEVEL>=1//start show warning 
        Warning(Here("ApplyOptics"),"\t No beam detector setting. Assuming a beam on exact center line of the hall." );
#endif//#if DEBUG_LEVEL>=1
    }

    

    return ApplyOptics(track,beampos,beamdir);
}

//_____________________________________________________________________________
Int_t THaOpticsE06010::ApplyOptics(THaTrack *track, TVector3 beampos, TVector3 beamdir)
{
	//Read in track X,Y,Theta,Phi in DCS and fill in any thing else in THaTrack

	if( !track ) {
#if DEBUG_LEVEL>=1//start show warning 
		Warning(Here("ApplyOptics"),"\tEmpty track input");
		return kInitError;
#endif//#if DEBUG_LEVEL>=1
	}

	//variables
	Double_t TargetX;
	Double_t TargetY;
	Double_t TargetTheta;
	Double_t TargetPhi;

	Double_t p=0, PathLen=0;    
	TVector3 vertex, vp;

	//do it
	Int_t statustmp;
	statustmp=ApplyOptics(
			track->GetX(),
			track->GetY(),
			track->GetTheta(),
			track->GetPhi(),
			beampos,
			beamdir,
			vp, 
			vertex,
			TargetX,
			TargetY,
			TargetTheta,
			TargetPhi,
			PathLen
			);  
	if (statustmp!=kOK) return statustmp;

	//feed back to track
	p=vp.Mag();

	// D2n correction: TargetTheta is pointing upward (away from floor). We want positive TargetTheta
	// to point downward (toward floor) to be consistent with other coordinate systems. This
	// sets the sign of the BB.tr.tg_th variable. No correction to TargetPhi is necessary.
	track->SetTarget(
			TargetX,
			TargetY,
			-1.*TargetTheta,
			TargetPhi
			);
	track->SetPathLen( PathLen );
	track->SetVertex( vertex );
	track->SetPvect( vp );
	track->SetMomentum( p );
	track->SetDp( p/GetPcentral()-1 );

	return kOK;
}




Int_t THaOpticsE06010::
ApplyOptics(
            Double_t trackX,    //input:    Track X             TRCS
            Double_t trackY,    //input:    Track Y             TRCS
            Double_t trackTheta,//input:    Track Theta         TRCS
            Double_t trackPhi,  //input:    Track Phi           TRCS
            TVector3 beamPos,   //input:    Beam Position       HCS
            TVector3 beamDir,   //input:    Beam Direction      HCS
            TVector3 &P,        //output:   vector momentum     HCS
            TVector3 &Vertex,   //output:   vertex cordinate    HCS
            Double_t &TargetX,  //output:   Target X            TCS
            Double_t &TargetY, //output:   Target Y            TCS
            Double_t &TargetTheta,//output: Target Theta        TCS
            Double_t &TargetPhi,//output:   Target Phi          TCS
            Double_t &PathLen   //output:   Length of path      any coordinate
            )
{

	Double_t bend_x[7],bend_y[7],bend_z[7];
	Double_t vertex_x,vertex_y,vertex_z;

	// define chamber, sieve position
	TVector3 C1_center(chamber_pos[0],chamber_pos[1],chamber_pos[2]);
	TVector3 sieve_center(sieve_pos[0],sieve_pos[1],sieve_pos[2]);

	TRotation chamber_rot;
	chamber_rot.SetXEulerAngles(chamber_angle[0],chamber_angle[2],chamber_angle[1]);
	TRotation chamber_inverse_rot = chamber_rot.Inverse();

	TRotation sieve_rot;
	sieve_rot.SetXEulerAngles(sieve_angle[0],sieve_angle[2],sieve_angle[1]); 
	TRotation sieve_inverse_rot = sieve_rot.Inverse();

	TVector3 sieve_dir(0.,0.,1.);
	sieve_dir *= sieve_rot;

	Double_t sieve1_x,sieve1_y,sieve1_z;
	Double_t sieve_dir_x,sieve_dir_y,sieve_dir_z;

	sieve1_x = sieve_center.X();
	sieve1_y = sieve_center.Y();
	sieve1_z = sieve_center.Z();
	sieve_dir_x = sieve_dir.X();
	sieve_dir_y = sieve_dir.Y();
	sieve_dir_z = sieve_dir.Z();

	Double_t vertex_hall_x,vertex_hall_y,vertex_hall_z,vz_new,radius,bend_theta=0.00001;
	Double_t tr_x,tr_y,tr_xp,tr_yp;
	tr_x = trackX;
	tr_y = trackY;
	tr_xp = trackTheta;
	tr_yp = trackPhi;


	//define middle plane and several other planes
	TVector3 bendtarget3(0,0,middle_pos);
	TVector3 bendtarget3_dir(0.,tan(middle_angle/180.*3.1415926),1.);

	TVector3 bendtarget1(0,0.6*sin(2.*middle_angle/180.*3.1415926),middle_pos + 0.6*cos(2.*middle_angle/180.*3.1415926));
	TVector3 bendtarget1_dir(0.,tan(2.*middle_angle/180.*3.1415926),1.);

	TVector3 bendtarget5(0,0,middle_pos - 0.6);
	TVector3 bendtarget5_dir(0.,0.,1.);

	TVector3 bendtarget4(0,0,middle_pos);
	TVector3 bendtarget4_dir(0.,tan(middle_angle/180.*3.1415926),1.);

	if (beamDir.Z()!=5.131){
		cout << "Warning: beam Dir Z = " << beamDir.Z() << "!!!" << endl;
		beamDir.SetXYZ(beamDir.X(),beamDir.Y(),5.131);
	}

	//cout << tr_x << "\t" << beamPos.X() << "\t"  << "\t" << beamPos.Y() << "\t" << beamDir.X() << "\t" << beamDir.Y() << "\t" << beamDir.Z() << endl;


	// get beam direction
	beamDir.SetXYZ(beamDir.X()/beamDir.Z(),beamDir.Y()/beamDir.Z(),1.);
	beamPos *= sieve_inverse_rot;
	beamDir *= sieve_inverse_rot;

	// get chamber tracks
	TVector3 chamber_pos(trackX,trackY,0);  // detector frame 
	TVector3 chamber_slope(trackTheta,trackPhi,1.); // detector frame
	chamber_pos   *=chamber_rot;
	chamber_pos   += C1_center;
	chamber_pos   *= sieve_inverse_rot;

	chamber_slope *= chamber_rot;	   	    	    
	chamber_slope *= sieve_inverse_rot;

	// find middle plane hit position
	bend_z[3] = (chamber_pos.Z()*chamber_slope.Y()/chamber_slope.Z()*bendtarget3_dir.Y()/bendtarget3_dir.Z() + (bendtarget3.Y()-chamber_pos.Y())*bendtarget3_dir.Y()/bendtarget3_dir.Z() + bendtarget3.Z())/(1.+chamber_slope.Y()/chamber_slope.Z()*bendtarget3_dir.Y()/bendtarget3_dir.Z());
	bend_y[3] = (bend_z[3] - chamber_pos.Z())*chamber_slope.Y()/chamber_slope.Z() + chamber_pos.Y();
	bend_x[3] = (bend_z[3] - chamber_pos.Z())*chamber_slope.X()/chamber_slope.Z() + chamber_pos.X();

	bend_z[1] = (chamber_pos.Z()*chamber_slope.Y()/chamber_slope.Z()*bendtarget1_dir.Y()/bendtarget1_dir.Z() + (bendtarget1.Y()-chamber_pos.Y())*bendtarget1_dir.Y()/bendtarget1_dir.Z() + bendtarget1.Z())/(1.+chamber_slope.Y()/chamber_slope.Z()*bendtarget1_dir.Y()/bendtarget1_dir.Z());
	bend_y[1] = (bend_z[1] - chamber_pos.Z())*chamber_slope.Y()/chamber_slope.Z() + chamber_pos.Y();
	bend_x[1] = (bend_z[1] - chamber_pos.Z())*chamber_slope.X()/chamber_slope.Z() + chamber_pos.X();


	if ((tr_x+0.690678)*(0.320151+0.119864)-(tr_xp+0.119864)*(0.70339+0.690678)>0.){
		charge = -1;
	}else if ((tr_x+0.690678)*(0.422414+0.0587285)-(tr_xp+0.0587285)*(0.622881+0.690678)<0.){
		// positive charged particle cut
		charge = 1.;
	}else{
		charge = 0.;
	}



	bendx = bend_x[3];
	bendy = bend_y[3];

	if (bend_y[3]>=-0.51&&bend_y[3]<0.44&&bend_x[3]<0.14&&bend_x[3]>-0.12){
		if (bend_x[3]<-0.08 && bend_y[3]>-0.43){
			vzflag = 1.;
		}else if (bend_x[3]<-0.06 && bend_x[3]>=-0.08 && bend_y[3]>-0.45){
			vzflag = 1;
		}else if (bend_x[3]<-0.04 && bend_x[3]>=-0.06 && bend_y[3]>-0.465){
			vzflag = 1;
		}else if (bend_x[3]<-0.0 && bend_x[3]>=-0.04 && bend_y[3]>-0.49){
			vzflag = 1.;
		}else if (bend_x[3]<0.04 && bend_x[3]>=-0.0 && bend_y[3]>-0.51){
			vzflag = 1.;
		}else if (bend_x[3]<0.06 && bend_x[3]>=0.04 && bend_y[3]>-0.5){
			vzflag = 1.;
		}else if (bend_x[3]<0.08 && bend_x[3]>=0.06 && bend_y[3]>-0.49){
			vzflag = 1.;
		}else if (bend_x[3]<0.1 && bend_x[3]>=0.08 && bend_y[3]>-0.46){
			vzflag = 1.;
		}else if (bend_x[3]<0.12 && bend_x[3]>=0.1 && bend_y[3]>-0.43){
			vzflag = 1.;
		}else if (bend_x[3]>=0.12 && bend_y[3]>-0.43){
			vzflag = 1.;
		}else{
			vzflag = 0.;
		}
	}else{
		vzflag = 0;
	} 

	Double_t qa,qb,qc,qd,qe,qf;
	Double_t QA,QB,QC;
	Double_t k1,k2;
	Double_t tr_hall_x,tr_hall_y,tr_hall_z;
	Double_t tr_hall_xp,tr_hall_yp,tr_hall_zp;
	Double_t temp1,temp2;
	Double_t patz,patx,paty;
	Double_t sievez;
	Double_t mom_corr[5];

	if (nofield_flag == 0){  
		qa = bend_z[3];
		qb = bend_y[3] + beamPos.Z()*beamDir.Y()/beamDir.Z() - beamPos.Y();
		qc = beamDir.Y()/beamDir.Z();
		qd = chamber_slope.X()/sqrt(chamber_slope.Y()*chamber_slope.Y() + chamber_slope.Z()*chamber_slope.Z());
		qe = bend_x[3] + beamPos.Z()*beamDir.X()/beamDir.Z() - beamPos.X();
		qf = beamDir.X()/beamDir.Z();

		QA = qa*qc*qd-qb*qd;
		QB = qa*qf-qe;
		QC = qe*qc-qb*qf;

		k1 = (-2*QB*QC+sqrt(4.*QB*QB*QC*QC-4*(QC*QC-QA*QA)*(QB*QB-QA*QA)))/(2.*(QB*QB-QA*QA));
		k2 = (-2*QB*QC-sqrt(4.*QB*QB*QC*QC-4*(QC*QC-QA*QA)*(QB*QB-QA*QA)))/(2.*(QB*QB-QA*QA));

		if (fabs((qa*k1-qb)/(k1-qc)-(qa*qd*sqrt(k1*k1+1)-qe)/(qd*sqrt(k1*k1+1)-qf))<0.001){
			k1 = k1;
		}else{
			k1 = k2;
		}

		vertex_z = (qa*k1-qb)/(k1-qc);
		vertex_x = (vertex_z - beamPos.Z())*beamDir.X()/beamDir.Z() + beamPos.X();
		vertex_y = (vertex_z - beamPos.Z())*beamDir.Y()/beamDir.Z() + beamPos.Y();



		TVector3 vertex(vertex_x,vertex_y,vertex_z);
		vertex *= sieve_rot;

		vertex_hall_z = vertex.Z();


		if (charge==-1){
			vz_new = vertex_hall_z;
			vz_new = vertex_hall_z+0.106887*tr_y-0.0107593+0.0019;
			vz_new = vz_new-(-0.000913+0.002992*vz_new + (-0.003164 + 0.059138*vz_new)*tr_x)-(0.000145+-0.001216*vz_new + (-0.015993 + 0.104864*vz_new)*tr_y)-(0.000261+-0.000037*vz_new + (-0.001399 + 0.032387*vz_new)*tr_yp)-(-0.001306+0.000877*vz_new + (-0.007950 + 0.026156*vz_new)*tr_xp)-(0.000530+-0.000340*vz_new + (0.002758 + -0.003928*vz_new)*tr_x)-(0.000019+0.000304*vz_new + (0.003418 + -0.007471*vz_new)*bend_x[3])-(-0.000071+-0.000234*vz_new + (-0.000928 + 0.000663*vz_new)*bend_y[3]);
			vz_new = vz_new-(-0.000455+0.001506*vz_new + (-0.001968 + 0.020751*vz_new)*tr_x)-(0.000178+-0.000214*vz_new + (-0.005771 + 0.026698*vz_new)*tr_y)-(0.000273+-0.000268*vz_new + (-0.002215 + 0.027309*vz_new)*tr_yp)-(-0.001196+0.000476*vz_new + (-0.006743 + 0.016716*vz_new)*tr_xp)-(0.000467+-0.000398*vz_new + (0.002419 + -0.002910*vz_new)*tr_x)-(0.000034+0.000239*vz_new + (0.003390 + -0.009842*vz_new)*bend_x[3])-(-0.000089+-0.000216*vz_new + (-0.000787 + 0.001638*vz_new)*bend_y[3]);

			vz_new = 0.994695 * vz_new + 0.000572978 * bend_x[3] + -0.000228579   * bend_y[3] +  0.000425165  * tr_x +  0.00154942 * tr_xp + 0.00310859 * tr_y + 0.0105233 * tr_yp + -3.85446e-05 ;  

			// get a first idea of the momentum for the vertex correction
			beamPos *= sieve_rot;
			beamDir *= sieve_rot;


			vertex_z = vz_new;
			vertex_x = (vertex_z - beamPos.Z())*beamDir.X()/beamDir.Z() + beamPos.X();
			vertex_y = (vertex_z - beamPos.Z())*beamDir.Y()/beamDir.Z() + beamPos.Y();
			TVector3 tg_dir((bend_x[3]-vertex_x)/(bend_z[3]-vertex_z),(bend_y[3]-vertex_y)/(bend_z[3]-vertex_z),1.);

			bend_z[5] = (vertex.Z()*tg_dir.Y()/tg_dir.Z()*bendtarget5_dir.Y()/bendtarget5_dir.Z() + (bendtarget5.Y()-vertex.Y())*bendtarget5_dir.Y()/bendtarget5_dir.Z() + bendtarget5.Z())/(1.+tg_dir.Y()/tg_dir.Z()*bendtarget5_dir.Y()/bendtarget5_dir.Z());
			bend_y[5] = (bend_z[5] - vertex.Z())*tg_dir.Y()/tg_dir.Z() + vertex.Y();
			bend_x[5] = (bend_z[5] - vertex.Z())*tg_dir.X()/tg_dir.Z() + vertex.X();

			bend_z[4] = (vertex.Z()*tg_dir.Y()/tg_dir.Z()*bendtarget4_dir.Y()/bendtarget4_dir.Z() + (bendtarget4.Y()-vertex.Y())*bendtarget4_dir.Y()/bendtarget4_dir.Z() + bendtarget4.Z())/(1.+tg_dir.Y()/tg_dir.Z()*bendtarget4_dir.Y()/bendtarget4_dir.Z());
			bend_y[4] = (bend_z[4] - vertex.Z())*tg_dir.Y()/tg_dir.Z() + vertex.Y();
			bend_x[4] = (bend_z[4] - vertex.Z())*tg_dir.X()/tg_dir.Z() + vertex.X();

			bend_theta = atan((bend_y[1]-bend_y[3])/(bend_z[1]-bend_z[3])) - atan((bend_y[4]-vertex.Y())/(bend_z[4]-vertex.Z()));
			//start momentum
			radius = (sqrt((bend_y[3]-bend_y[1])*(bend_y[3]-bend_y[1]) + (bend_z[3]-bend_z[1])*(bend_z[3]-bend_z[1])) 
					+ sqrt((bend_y[3]-bend_y[5])*(bend_y[3]-bend_y[5]) + (bend_z[3]-bend_z[5])*(bend_z[3]-bend_z[5])))
				/2./tan(bend_theta/2.)
				*(sqrt(tg_dir.X()*tg_dir.X() +tg_dir.Y()*tg_dir.Y()  + 1)/sqrt(tg_dir.Y()*tg_dir.Y()+ 1) 
						+ sqrt(chamber_slope.X()*chamber_slope.X() + chamber_slope.Y()*chamber_slope.Y() + chamber_slope.Z()*chamber_slope.Z())/sqrt(chamber_slope.Y()*chamber_slope.Y() + chamber_slope.Z()*chamber_slope.Z()))
				/2.;
			radius = radius * (2.260402e-01-4.704221e-02*bend_y[3]-2.384442e-02*bend_y[3]*bend_y[3])*(9.972603e-01+1.736234e-01 *bend_x[3])*(9.995608e-01-5.052159e-02*vz_new)*(9.993404e-01-1.094946e-02*tr_x-1.851953e-02*tr_x*tr_x)*(9.950031e-01-1.018607e-01*tr_xp-3.577766e-01*tr_xp*tr_xp)*(1.000601e+00-5.142707e-03*tr_y+ 4.840272e-03*tr_y*tr_y)*(1.000393e+00-3.330072e-03*tr_yp-2.629414e-01*tr_yp*tr_yp);
			radius = radius*(9.978739e-01-8.262172e-03*bend_y[3]+8.268577e-02*bend_y[3]*bend_y[3])*(9.992858e-01+2.491770e-03*bend_x[3]+2.331594e-01*bend_x[3]*bend_x[3])*(1.028797e+00-2.533582e-02*radius);

			if (radius < 3. && radius>0.){
				//fist order momentum correction  fit with bendx
				vz_new = vz_new - (vz_new*(0.0315185-0.0249438*radius) + ((-0.104883+0.0493902 *radius)*(1.01461-0.463881*vz_new) + exp((2.20935-5.90607 *radius)*(1.03831-0.253001 * vz_new)))*bend_x[3]);
				vz_new = vz_new - (vz_new*(0.0261969-0.0275801*radius) + ((-0.0454597+0.128227*radius)*(0.274729+2.42204*vz_new) + exp((1.52379-6.64186 *radius)*( 1.09829+0.786964* vz_new)))*bend_x[3]);
			}

			vz_new = vz_new - sndcorr(bend_x[3],bend_y[3],vz_new,&(vz_x[0][0]),&(vz_y[0][0]),&(vz_z[0][0][0]),&(vz_corr[0][0][0]));
			if (radius < 3.&&radius>0.){
				// parameterization terms
				vz_new = vz_new - thrcorr(radius,bend_y[3],vz_new,&vz_x1[0][0],&vz_y1[0][0],&vz_z1[0][0][0],&vz_corr1[0][0][0]);
				vz_new = vz_new - foucorr(radius,bend_x[3],vz_new,&vz_x2[0][0],&vz_y2[0][0],&vz_z2[0][0][0],&vz_corr2[0][0][0]);
			}

			vertex_z = vz_new;
			vertex_x = (vertex_z - beamPos.Z())*beamDir.X()/beamDir.Z() + beamPos.X();
			vertex_y = (vertex_z - beamPos.Z())*beamDir.Y()/beamDir.Z() + beamPos.Y();

			tr_hall_x = vertex_x;
			tr_hall_y = vertex_y;
			tr_hall_z = vertex_z;

			Vertex.SetXYZ(tr_hall_x,tr_hall_y,tr_hall_z);


			vertex.SetXYZ(vertex_x,vertex_y,vertex_z);
			vertex *= sieve_inverse_rot;	    

			bend_x[6] = bend_x[3];
			bend_y[6] = bend_y[3];
			bend_z[6] = bend_z[3];

			TVector3 track_hall_slope((bend_x[6]-vertex.X())/(bend_z[6]-vertex.Z()),(bend_y[6]-vertex.Y())/(bend_z[6]-vertex.Z()),1.);

			track_hall_slope *=sieve_rot; //lab frame

			tr_hall_xp = track_hall_slope.X()/track_hall_slope.Z();
			tr_hall_yp = track_hall_slope.Y()/track_hall_slope.Z();
			tr_hall_zp = 1.;

			temp1 = (sieve_dir_x*tr_hall_xp+sieve_dir_y*tr_hall_yp+sieve_dir_z);
			temp2 = (sieve1_z*sieve_dir_z+(sieve1_y-tr_hall_y)*sieve_dir_y+tr_hall_z*tr_hall_yp*sieve_dir_y+(sieve1_x-tr_hall_x)*sieve_dir_x+tr_hall_z*tr_hall_xp*sieve_dir_x);
			patz = temp2/temp1;
			patx = (patz-tr_hall_z)*tr_hall_xp + tr_hall_x;
			paty = (patz-tr_hall_z)*tr_hall_yp + tr_hall_y;

			TVector3 pattern(patx,paty,patz);
			TVector3 pat_dir(tr_hall_xp,tr_hall_yp,tr_hall_zp);
			pattern -= sieve_center;
			pattern *= sieve_inverse_rot;
			pat_dir *= sieve_inverse_rot;

			vertex *= sieve_rot;
			vertex -= sieve_center;
			vertex *= sieve_inverse_rot;

			// sieve frame
			vertex_hall_x = vertex.X();
			vertex_hall_y = vertex.Y();
			vertex_hall_z = vertex.Z();

			//  Double_t sievex,sievey,sievez,sievexp,sieveyp,PathLen;


			sievex = pattern.X()-0.002545;//-0.002515;
			sievex += 0.011879598*bend_x[3]-0.00029976+0.004*bend_y[3];

			// abitrary offset
			sievey = pattern.Y()-0.003985; 

			if (tr_xp<0.){
				sievey -= (-2.148095e-03 -2.062012e-02*tr_xp);
			}else{
				sievey -= (-2.148095e-03 +2.062012e-02*tr_xp);
			}

			sievey -= (-4.440656e-04 + 3.564498e-03*tr_y+3.190652e-02*tr_y*tr_y);

			sievey -= (1.393863e-04 + 1.119292e-03*bend_x[3] -3.881592e-02*bend_x[3]*bend_x[3]);

			sievey -= (8.572001e-04-7.472350e-04*radius);
			//vz
			sievey -= (-9.696860e-05-7.483888e-04*vz_new-3.618945e-03*vz_new*vz_new);
			//yp
			sievey -= (-7.651346e-05-2.230147e-04*tr_yp+3.660121e-02*tr_yp*tr_yp);
			//y???
			sievey -= 1.423358e-04 + 6.671543e-04 *tr_x;
			//xp
			if (tr_xp<0.){
				sievey -= 3.787455e-04 + 2.058874e-03*tr_xp;
			}else{
				sievey -= 3.787455e-04 -3.076939e-02 *tr_xp;
			}
			if (fabs(sievex)<0.2&&fabs(sievey)<0.5){
				sievey -= sievecorr(sievex,sievey,&sieve_x[0][0],&sieve_y[0][0],&sieve_corr[0][0]);
			}
			//   //sieve z
			sievez = pattern.Z();
			sievexp = (sievex-vertex_hall_x)/(sievez - vertex_hall_z);
			sieveyp = (sievey-vertex_hall_y)/(sievez - vertex_hall_z);

			TargetTheta = sieveyp;
			TargetPhi = sievexp;

			pattern.SetXYZ(sievex - sievexp *(sqrt(sieve_pos[0]*sieve_pos[0] + sieve_pos[2]*sieve_pos[2])),sievey - sieveyp *(sqrt(sieve_pos[0]*sieve_pos[0] + sieve_pos[2]*sieve_pos[2])),-sqrt(sieve_pos[0]*sieve_pos[0] + sieve_pos[2]*sieve_pos[2]));

			pattern *= sieve_rot;
			pattern += sieve_center;
			pattern *= sieve_inverse_rot;

			TargetX = pattern.X();
			TargetY = pattern.Y();

			if (calib_flag==1){
				sieve_flag = (Double_t)sieve(sievex,sievey,sievexp,sieveyp,vertex_hall_x,vertex_hall_y,vertex_hall_z,-1.);
				sieve_flag2 = (Double_t)sieve1(sievex,sievey,sievexp,sieveyp,vertex_hall_x,vertex_hall_y,vertex_hall_z,-1.);
			}else{
				sieve_flag = 0.;
				sieve_flag2 = 0.;
			}

			vertex *= sieve_rot;
			vertex += sieve_center;
			vertex *= sieve_inverse_rot;

			tg_dir.SetXYZ(sievexp,sieveyp,1.);

			bend_z[5] = (vertex.Z()*tg_dir.Y()/tg_dir.Z()*bendtarget5_dir.Y()/bendtarget5_dir.Z() + (bendtarget5.Y()-vertex.Y())*bendtarget5_dir.Y()/bendtarget5_dir.Z() + bendtarget5.Z())/(1.+tg_dir.Y()/tg_dir.Z()*bendtarget5_dir.Y()/bendtarget5_dir.Z());
			bend_y[5] = (bend_z[5] - vertex.Z())*tg_dir.Y()/tg_dir.Z() + vertex.Y();
			bend_x[5] = (bend_z[5] - vertex.Z())*tg_dir.X()/tg_dir.Z() + vertex.X();


			bend_z[4] = (vertex.Z()*tg_dir.Y()/tg_dir.Z()*bendtarget4_dir.Y()/bendtarget4_dir.Z() + (bendtarget4.Y()-vertex.Y())*bendtarget4_dir.Y()/bendtarget4_dir.Z() + bendtarget4.Z())/(1.+tg_dir.Y()/tg_dir.Z()*bendtarget4_dir.Y()/bendtarget4_dir.Z());
			bend_y[4] = (bend_z[4] - vertex.Z())*tg_dir.Y()/tg_dir.Z() + vertex.Y();
			bend_x[4] = (bend_z[4] - vertex.Z())*tg_dir.X()/tg_dir.Z() + vertex.X();


			PathLen = sqrt((bend_x[5]-vertex.X())*(bend_x[5]-vertex.X()) + (bend_y[5]-vertex.Y())*(bend_y[5]-vertex.Y()) + (bend_z[5]-vertex.Z())*(bend_z[5]-vertex.Z()));
			PathLen += sqrt((bend_x[1]-chamber_pos.X())*(bend_x[1]-chamber_pos.X()) + (bend_y[1]-chamber_pos.Y())*(bend_y[1]-chamber_pos.Y()) + (bend_z[1]-chamber_pos.Z())*(bend_z[1]-chamber_pos.Z()));
			bend_x[2] = (bend_x[3]+bend_x[4])/2.;
			bend_y[2] = (bend_y[3]+bend_y[4])/2.;
			bend_z[2] = (bend_z[3]+bend_z[4])/2.;

			bend_theta = atan((bend_y[1]-bend_y[3])/(bend_z[1]-bend_z[3])) - atan((bend_y[4]-bend_y[5])/(bend_z[4]-bend_z[5]));

			PathLen += sqrt((bend_x[2]-bend_x[5])*(bend_x[2]-bend_x[5]) + ((bend_y[2]-bend_y[5])*(bend_y[2]-bend_y[5]) + (bend_z[2]-bend_z[5])*(bend_z[2]-bend_z[5]))/2./tan(bend_theta/2.)*bend_theta);
			PathLen += sqrt((bend_x[2]-bend_x[1])*(bend_x[2]-bend_x[1]) + ((bend_y[2]-bend_y[1])*(bend_y[2]-bend_y[1]) + (bend_z[2]-bend_z[1])*(bend_z[2]-bend_z[1]))/2./tan(bend_theta/2.)*bend_theta);

			//original momentum
			radius = (sqrt((bend_y[2]-bend_y[1])*(bend_y[2]-bend_y[1]) + (bend_z[2]-bend_z[1])*(bend_z[2]-bend_z[1])) 
					+ sqrt((bend_y[2]-bend_y[5])*(bend_y[2]-bend_y[5]) + (bend_z[2]-bend_z[5])*(bend_z[2]-bend_z[5])))
				/2./tan(bend_theta/2.)*sqrt(sieveyp*sieveyp + sievexp*sievexp + 1)/sqrt(sieveyp*sieveyp + 1); 

			radius *= 0.222506 -0.03816242*bend_y[3]-0.0278703*bend_y[3]*bend_y[3];

			tg_dir *= sieve_rot;
			theta_elastic = acos(tg_dir * beamDir /tg_dir.Mag()/beamDir.Mag());

			// D2n correction: global scaling factor
			radius *= 1.041;
			p_firstorder = radius;

			// Low-momentum correction to place Delta at 1232 GeV
			// Linear correction at low p and smoothing function at transitional p
			// 50 MeV = half-size of transitional region
			if(radius < 0.85){
				radius += 0.148*(radius - 0.9);
			}
			if ((radius >= 0.85) && (radius <= 0.95)){
				radius -= 0.148/(4*0.05) * (radius - 0.95)*(radius - 0.95);
			}

			//Double_t eloss;
			eloss = (exp(-0.0566922*sin(16./180.*3.1415926)/sin(theta_elastic) -0.0001496 - 0.0016839/0.5123*(PathLen-0.8)));
//			radius =radius/eloss; 	   

			// landau correction
//			radius = radius * (1.+ (0.85+0.75)/2.*0.00263*(1.-3.2/6.5));
			eloss = 1. - (1. - eloss)*3.2/6.5;

			P.SetXYZ(radius*tg_dir.X()/tg_dir.Mag(),radius*tg_dir.Y()/tg_dir.Mag(),radius*tg_dir.Z()/tg_dir.Mag());
		}else if (charge==1){
			vz_new = (vertex_hall_z -0.65*tr_y-0.004)*0.855;
			vz_new = vz_new-(0.000491+-0.005800*vz_new + (-0.000767 + -0.059834*vz_new)*tr_x)-(-0.000472+0.014846*vz_new + (0.017914 + -0.215280*vz_new)*tr_y + (0.000000 + 0.000000*vz_new)*tr_y*tr_y + (0.000000 + 0.000000*vz_new)*tr_y*tr_y*tr_y)-(0.004058+0.043320*vz_new + (0.114351 + 0.939992*vz_new)*tr_yp)-(-0.003164+-0.008391*vz_new + (-0.000346 + -0.013934*vz_new)*tr_xp)-(-0.000081+-0.000087*vz_new + (-0.001377 + -0.000541*vz_new)*tr_x)-(0.002064+0.020079*vz_new + (-0.008314 + -0.318566*vz_new)*bend_x[3])-(-0.000561+-0.001019*vz_new + (-0.004828 + -0.008213*vz_new)*bend_y[3]);
			// get a first idea of the momentum for the vertex correction
			beamPos *= sieve_rot;
			beamDir *= sieve_rot;

			vertex_z = vz_new;
			vertex_x = (vertex_z - beamPos.Z())*beamDir.X()/beamDir.Z() + beamPos.X();
			vertex_y = (vertex_z - beamPos.Z())*beamDir.Y()/beamDir.Z() + beamPos.Y();

			TVector3 tg_dir((bend_x[3]-vertex_x)/(bend_z[3]-vertex_z),(bend_y[3]-vertex_y)/(bend_z[3]-vertex_z),1.);

			bend_z[5] = (vertex.Z()*tg_dir.Y()/tg_dir.Z()*bendtarget5_dir.Y()/bendtarget5_dir.Z() + (bendtarget5.Y()-vertex.Y())*bendtarget5_dir.Y()/bendtarget5_dir.Z() + bendtarget5.Z())/(1.+tg_dir.Y()/tg_dir.Z()*bendtarget5_dir.Y()/bendtarget5_dir.Z());
			bend_y[5] = (bend_z[5] - vertex.Z())*tg_dir.Y()/tg_dir.Z() + vertex.Y();
			bend_x[5] = (bend_z[5] - vertex.Z())*tg_dir.X()/tg_dir.Z() + vertex.X();

			bend_z[4] = (vertex.Z()*tg_dir.Y()/tg_dir.Z()*bendtarget4_dir.Y()/bendtarget4_dir.Z() + (bendtarget4.Y()-vertex.Y())*bendtarget4_dir.Y()/bendtarget4_dir.Z() + bendtarget4.Z())/(1.+tg_dir.Y()/tg_dir.Z()*bendtarget4_dir.Y()/bendtarget4_dir.Z());
			bend_y[4] = (bend_z[4] - vertex.Z())*tg_dir.Y()/tg_dir.Z() + vertex.Y();
			bend_x[4] = (bend_z[4] - vertex.Z())*tg_dir.X()/tg_dir.Z() + vertex.X();

			bend_x[2] = (bend_x[3]+bend_x[4])/2.;
			bend_y[2] = (bend_y[3]+bend_y[4])/2.;
			bend_z[2] = (bend_z[3]+bend_z[4])/2.;

			bend_theta = atan((bend_y[1]-bend_y[3])/(bend_z[1]-bend_z[3])) - atan((bend_y[4]-bend_y[5])/(bend_z[4]-bend_z[5]));
			radius = (sqrt((bend_y[2]-bend_y[1])*(bend_y[2]-bend_y[1]) + (bend_z[2]-bend_z[1])*(bend_z[2]-bend_z[1])) 
					+ sqrt((bend_y[2]-bend_y[5])*(bend_y[2]-bend_y[5]) + (bend_z[2]-bend_z[5])*(bend_z[2]-bend_z[5])))
				/2./tan(bend_theta/2.)*sqrt(sieveyp*sieveyp + sievexp*sievexp + 1)/sqrt(sieveyp*sieveyp + 1); 
			radius = -1*radius;


			vz_new = vz_new-(0.031851+-0.016657*vz_new + (-0.007501 + 0.001613*vz_new)*radius);
			vz_new = vz_new -(((-7.187380e-02+2.337543e-02*radius-1.517749e-03*radius*radius) +(-6.334465e-02+2.784096e-02*radius-1.691392e-03*radius*radius)*vz_new + (2.272095e-01+3.118184e-02*radius)*vz_new*vz_new) + ((2.827942-1.102471*radius+9.982384e-02*radius*radius)+(-4.800997+2.075539*radius-2.476673e-01*radius*radius)*vz_new)*bend_x[3]);
			vz_new = 0.988434 * vz_new + 0.0199035*bend_x[3] -0.0022569 * bend_y[3] +0.00179572*tr_x + 0.00234134 * tr_xp + 0.023467*tr_y -0.00388046*tr_yp -0.000612327;
			vz_new = vz_new - sndcorr_p(bend_x[3],bend_y[3],vz_new,&xp1[0][0],&yp1[0][0],&zp1[0][0][0],&corrp1[0][0][0]);

			if (radius>0.&&radius<100){
				vz_new = vz_new - foucorr_p(radius,bend_x[3],vz_new,&xp2[0][0],&yp2[0][0],&zp2[0][0][0],&corrp2[0][0][0]);
			}

			vertex_z = vz_new;
			vertex_x = (vertex_z - beamPos.Z())*beamDir.X()/beamDir.Z() + beamPos.X();
			vertex_y = (vertex_z - beamPos.Z())*beamDir.Y()/beamDir.Z() + beamPos.Y();

			tg_dir.SetXYZ((bend_x[3]-vertex_x)/(bend_z[3]-vertex_z),(bend_y[3]-vertex_y)/(bend_z[3]-vertex_z),1.);

			tr_hall_x = vertex_x;
			tr_hall_y = vertex_y;
			tr_hall_z = vertex_z;

			Vertex.SetXYZ(tr_hall_x,tr_hall_y,tr_hall_z);

			vertex.SetXYZ(vertex_x,vertex_y,vertex_z);
			vertex *= sieve_inverse_rot;	    
			// get new x0 value at the middle plane, y0 and z0 is the same.
			bend_x[6] = bend_x[3];
			bend_y[6] = bend_y[3];
			bend_z[6] = bend_z[3];

			TVector3 track_hall_slope((bend_x[6]-vertex.X())/(bend_z[6]-vertex.Z()),(bend_y[6]-vertex.Y())/(bend_z[6]-vertex.Z()),1.);

			track_hall_slope *=sieve_rot; //lab frame

			tr_hall_xp = track_hall_slope.X()/track_hall_slope.Z();
			tr_hall_yp = track_hall_slope.Y()/track_hall_slope.Z();
			tr_hall_zp = 1.;

			temp1 = (sieve_dir_x*tr_hall_xp+sieve_dir_y*tr_hall_yp+sieve_dir_z);
			temp2 = (sieve1_z*sieve_dir_z+(sieve1_y-tr_hall_y)*sieve_dir_y+tr_hall_z*tr_hall_yp*sieve_dir_y+(sieve1_x-tr_hall_x)*sieve_dir_x+tr_hall_z*tr_hall_xp*sieve_dir_x);
			patz = temp2/temp1;
			patx = (patz-tr_hall_z)*tr_hall_xp + tr_hall_x;
			paty = (patz-tr_hall_z)*tr_hall_yp + tr_hall_y;

			TVector3 pattern(patx,paty,patz);
			TVector3 pat_dir(tr_hall_xp,tr_hall_yp,tr_hall_zp);
			pattern -= sieve_center;
			pattern *= sieve_inverse_rot;
			pat_dir *= sieve_inverse_rot;

			vertex *= sieve_rot;
			vertex -= sieve_center;
			vertex *= sieve_inverse_rot;

			// sieve frame
			vertex_hall_x = vertex.X();
			vertex_hall_y = vertex.Y();
			vertex_hall_z = vertex.Z();

			sievex = pattern.X()-0.004545-0.001088;//-0.002515;
			sievey = pattern.Y()-0.007985-0.001546; 

			sievex += 0.01*bend_y[3] -3.41813e-04;

			if (tr_xp>0.){
				sievey -= (-2.422227e-02+7.012280e-02*sievey) + (4.923416e-02-1.800220e-01*sievey)*tr_xp;
			}else{
				sievey -= (-2.422227e-02+7.012280e-02*sievey) - (4.923416e-02-1.800220e-01*sievey)*tr_xp;
			}

			sievey -= (1.132111e-03-1.449619e-03*sievey) + (3.191519e-03-1.691200e-02*sievey)*tr_y + (-1.092879e-01+7.330379e-02*sievey)*tr_y*tr_y;

			sievey -= (-8.174652e-05+5.687255e-04*sievey) + (1.531981e-03-9.835313e-03*sievey)*bend_x[3];

			sievey -= (-3.151397e-04+2.360020e-02*sievey)+(-2.597560e-04-2.999620e-03*sievey)*radius;

			sievey -= (-1.275592e-03+7.306276e-03*sievey) + (-1.818995e-03+4.139379e-03*sievey)*tr_yp;

			sievey -=(-1.493051e-03+5.252217e-02*sievey-2.422359e-01*sievey*sievey)+(-1.524059e-03+3.275626e-02*sievey-1.509446e-01*sievey*sievey)*vz_new;

			sievez = pattern.Z();
			sievexp = (sievex-vertex_hall_x)/(sievez - vertex_hall_z);
			sieveyp = (sievey-vertex_hall_y)/(sievez - vertex_hall_z);

			TargetTheta = sieveyp;
			TargetPhi = sievexp;

			pattern.SetXYZ(sievex - sievexp *(sqrt(sieve_pos[0]*sieve_pos[0] + sieve_pos[2]*sieve_pos[2])),sievey - sieveyp *(sqrt(sieve_pos[0]*sieve_pos[0] + sieve_pos[2]*sieve_pos[2])),-sqrt(sieve_pos[0]*sieve_pos[0] + sieve_pos[2]*sieve_pos[2]));

			pattern *= sieve_rot;
			pattern += sieve_center;
			pattern *= sieve_inverse_rot;

			TargetX = pattern.X();
			TargetY = pattern.Y();

			if (calib_flag ==1){
				sieve_flag = (Double_t)sieve(sievex,sievey,sievexp,sieveyp,vertex_hall_x,vertex_hall_y,vertex_hall_z,-0.5);
			}else{
				sieve_flag = 0;
			}

			vertex *= sieve_rot;
			vertex += sieve_center;
			vertex *= sieve_inverse_rot;
			tg_dir.SetXYZ(sievexp,sieveyp,1.);
			bend_z[5] = (vertex.Z()*tg_dir.Y()/tg_dir.Z()*bendtarget5_dir.Y()/bendtarget5_dir.Z() + (bendtarget5.Y()-vertex.Y())*bendtarget5_dir.Y()/bendtarget5_dir.Z() + bendtarget5.Z())/(1.+tg_dir.Y()/tg_dir.Z()*bendtarget5_dir.Y()/bendtarget5_dir.Z());
			bend_y[5] = (bend_z[5] - vertex.Z())*tg_dir.Y()/tg_dir.Z() + vertex.Y();
			bend_x[5] = (bend_z[5] - vertex.Z())*tg_dir.X()/tg_dir.Z() + vertex.X();


			bend_z[4] = (vertex.Z()*tg_dir.Y()/tg_dir.Z()*bendtarget4_dir.Y()/bendtarget4_dir.Z() + (bendtarget4.Y()-vertex.Y())*bendtarget4_dir.Y()/bendtarget4_dir.Z() + bendtarget4.Z())/(1.+tg_dir.Y()/tg_dir.Z()*bendtarget4_dir.Y()/bendtarget4_dir.Z());
			bend_y[4] = (bend_z[4] - vertex.Z())*tg_dir.Y()/tg_dir.Z() + vertex.Y();
			bend_x[4] = (bend_z[4] - vertex.Z())*tg_dir.X()/tg_dir.Z() + vertex.X();


			PathLen = sqrt((bend_x[5]-vertex.X())*(bend_x[5]-vertex.X()) + (bend_y[5]-vertex.Y())*(bend_y[5]-vertex.Y()) + (bend_z[5]-vertex.Z())*(bend_z[5]-vertex.Z()));
			PathLen += sqrt((bend_x[1]-chamber_pos.X())*(bend_x[1]-chamber_pos.X()) + (bend_y[1]-chamber_pos.Y())*(bend_y[1]-chamber_pos.Y()) + (bend_z[1]-chamber_pos.Z())*(bend_z[1]-chamber_pos.Z()));
			bend_x[2] = (bend_x[3]+bend_x[4])/2.;
			bend_y[2] = (bend_y[3]+bend_y[4])/2.;
			bend_z[2] = (bend_z[3]+bend_z[4])/2.;

			bend_theta = atan((bend_y[1]-bend_y[3])/(bend_z[1]-bend_z[3])) - atan((bend_y[4]-bend_y[5])/(bend_z[4]-bend_z[5]));

			PathLen += sqrt((bend_x[2]-bend_x[5])*(bend_x[2]-bend_x[5]) + ((bend_y[2]-bend_y[5])*(bend_y[2]-bend_y[5]) + (bend_z[2]-bend_z[5])*(bend_z[2]-bend_z[5]))/2./tan(bend_theta/2.)*bend_theta);
			PathLen += sqrt((bend_x[2]-bend_x[1])*(bend_x[2]-bend_x[1]) + ((bend_y[2]-bend_y[1])*(bend_y[2]-bend_y[1]) + (bend_z[2]-bend_z[1])*(bend_z[2]-bend_z[1]))/2./tan(bend_theta/2.)*bend_theta);

			//original momentum
			radius = -1.*(sqrt((bend_y[2]-bend_y[1])*(bend_y[2]-bend_y[1]) + (bend_z[2]-bend_z[1])*(bend_z[2]-bend_z[1])) 
					+ sqrt((bend_y[2]-bend_y[5])*(bend_y[2]-bend_y[5]) + (bend_z[2]-bend_z[5])*(bend_z[2]-bend_z[5])))
				/2./tan(bend_theta/2.)*sqrt(sieveyp*sieveyp + sievexp*sievexp + 1)/sqrt(sieveyp*sieveyp + 1); 

			radius = radius/(4.457180+7.618057e-01*bend_y[3]+1.619582e+00*bend_y[3]*bend_y[3]);
			p_firstorder = radius;

			Double_t mom_corr[5];

			// Low-momentum correction to place Delta at 1232 GeV
			// Based on Xin's work
			// Linear correction at low p and smoothing function at transitional p
			// 50 MeV = half-size of transitional region
	
			if (radius < 0.70){
				radius += 0.25*(radius-0.75);
			}
			if ((radius >= 0.70) && (radius <= 0.80)){
				radius -= 0.25/(4.*0.05) * (radius-0.80)*(radius-0.80);
			}

			tg_dir *= sieve_rot;
			theta_elastic = acos(tg_dir * beamDir /tg_dir.Mag()/beamDir.Mag());
			eloss = (exp(-0.0566922*sin(16./180.*3.1415926)/sin(theta_elastic) -0.0001496 - 0.0016839/0.5123*(PathLen-0.8)));
			eloss = 1. - (1-eloss)* 3.2/6.5;
//			radius = radius/eloss;

			P.SetXYZ(radius*tg_dir.X()/tg_dir.Mag(),radius*tg_dir.Y()/tg_dir.Mag(),radius*tg_dir.Z()/tg_dir.Mag());
		}else{

		}
	}else if (nofield_flag == 1){

		beamPos *= sieve_rot;
		beamDir *= sieve_rot;

		Double_t beam_x,beam_y,beam_xp,beam_yp;
		beam_x = beamPos.X();
		beam_y = beamPos.Y();
		beam_xp = beamDir.X()/beamDir.Z();
		beam_yp = beamDir.Y()/beamDir.Z();

		chamber_pos *= sieve_rot;
		chamber_slope *= sieve_rot;

		tr_hall_x = chamber_pos.X();
		tr_hall_y = chamber_pos.Y();
		tr_hall_z = chamber_pos.Z();

		tr_hall_xp = chamber_slope.X()/chamber_slope.Z();
		tr_hall_yp = chamber_slope.Y()/chamber_slope.Z();
		tr_hall_zp = 1.;

		// get close distance betweent the track and the beam assuming no common z. 
		Double_t tempA1,tempA2,tempB,tempC1,tempC2;
		tempA1 = beam_xp*beam_xp + beam_yp*beam_yp + 1.;
		tempA2 = tr_hall_xp*tr_hall_xp + tr_hall_yp*tr_hall_yp + 1.;
		tempB = beam_xp * tr_hall_xp + beam_yp*tr_hall_yp + 1.;
		tempC1 = tr_hall_z*(beam_xp * tr_hall_xp + beam_yp*tr_hall_yp) + (beam_y*beam_yp - tr_hall_y*beam_yp + beam_x*beam_xp - tr_hall_x*beam_xp);
		tempC2 = -tr_hall_z*(tr_hall_xp*tr_hall_xp + tr_hall_yp*tr_hall_yp)+(tr_hall_y*tr_hall_yp-beam_y*tr_hall_yp+tr_hall_x*tr_hall_xp - beam_x*tr_hall_xp);

		vz_new = (-tempC1*tempB-tempC2*tempA1)/(tempA1*tempA2 - tempB*tempB);
		// vz2 = (-tempC1*tempA2 - tempC2*tempB)/(tempA1*tempA2 - tempB*tempB);
		// vz = vz1;
		vertex_z = vz_new;
		vertex_x = (vertex_z - beamPos.Z())*beamDir.X()/beamDir.Z() + beamPos.X();
		vertex_y = (vertex_z - beamPos.Z())*beamDir.Y()/beamDir.Z() + beamPos.Y();

		tr_hall_x = vertex_x;
		tr_hall_y = vertex_y;
		tr_hall_z = vertex_z;

		Vertex.SetXYZ(tr_hall_x,tr_hall_y,tr_hall_z);
		P.SetXYZ(0.,0.,0.);

		temp1 = (sieve_dir_x*tr_hall_xp+sieve_dir_y*tr_hall_yp+sieve_dir_z);
		temp2 = (sieve1_z*sieve_dir_z+(sieve1_y-tr_hall_y)*sieve_dir_y+tr_hall_z*tr_hall_yp*sieve_dir_y+(sieve1_x-tr_hall_x)*sieve_dir_x+tr_hall_z*tr_hall_xp*sieve_dir_x);
		patz = temp2/temp1;
		patx = (patz-tr_hall_z)*tr_hall_xp + tr_hall_x;
		paty = (patz-tr_hall_z)*tr_hall_yp + tr_hall_y;

		TVector3 pattern(patx,paty,patz);
		TVector3 pat_dir(tr_hall_xp,tr_hall_yp,tr_hall_zp);
		pattern -= sieve_center;
		pattern *= sieve_inverse_rot;
		pat_dir *= sieve_inverse_rot;

		sievex = pattern.X();
		sievey = pattern.Y();
		sievez = pattern.Z();

		sievexp = pat_dir.X()/pat_dir.Z();
		sieveyp = pat_dir.Y()/pat_dir.Z();

		TargetTheta = sieveyp;
		TargetPhi = sievexp;

		pattern.SetXYZ(sievex - sievexp *(sqrt(sieve_pos[0]*sieve_pos[0] + sieve_pos[2]*sieve_pos[2])),sievey - sieveyp *(sqrt(sieve_pos[0]*sieve_pos[0] + sieve_pos[2]*sieve_pos[2])),-sqrt(sieve_pos[0]*sieve_pos[0] + sieve_pos[2]*sieve_pos[2]));

		pattern *= sieve_rot;
		pattern += sieve_center;
		pattern *= sieve_inverse_rot;

		TargetX = pattern.X();
		TargetY = pattern.Y();
		PathLen = sqrt((Vertex.X()-tr_hall_x)*(Vertex.X()-tr_hall_x) + (Vertex.Y()-tr_hall_y)*(Vertex.Y()-tr_hall_y) + (Vertex.Z()-tr_hall_z)*(Vertex.Z()-tr_hall_z));

		charge = 0;
		vzflag = 1;
		bend_theta = 0.;

		sieve_flag = 0.;
		sieve_flag2 = 0.;


		TVector3 tg_dir(sievexp,sieveyp,1.);
		tg_dir *= sieve_rot;
		theta_elastic = acos(tg_dir * beamDir /tg_dir.Mag()/beamDir.Mag());
		eloss = (exp(-0.0566922*sin(16./180.*3.1415926)/sin(theta_elastic) -0.0001496 - 0.0016839/0.5123*(PathLen-0.8)));

		eloss = 1. - (1. - eloss)*3.2/6.5;

	}

	theoptics* temp = new( (*ftroptics)[GetN()])
		theoptics(sievex,sievey,sievexp,sieveyp,bendx,bendy,charge,vzflag,bend_theta,theta_elastic,sieve_flag,sieve_flag2,eloss,p_firstorder);
	temp = temp;

	return kOK;
}


Int_t THaOpticsE06010::DefineVariables( EMode mode )
{
  // Initialize global variables

  if( mode == kDefine and fIsSetup ) return kOK;
  fIsSetup = ( mode == kDefine );

  // Register variables in global list
  
 

  RVarDef vars[] = {
    {"n","n","GetN()"},
    {"eloss","eloss","ftroptics.theoptics.Geteloss()"},
    {"sieve_x","sievex","ftroptics.theoptics.Getsievex()"},
    {"sieve_xp","sievexp","ftroptics.theoptics.Getsievexp()"},
    {"sieve_y","sievey","ftroptics.theoptics.Getsievey()"},
    {"sieve_yp","sieveyp","ftroptics.theoptics.Getsieveyp()"},
    {"bendx","bendx","ftroptics.theoptics.Getbendx()"},
    {"bendy","bendy","ftroptics.theoptics.Getbendy()"},
    {"charge","charge","ftroptics.theoptics.Getcharge()"},
    {"vzflag","vzflag","ftroptics.theoptics.Getvzflag()"},
    {"bend_theta","bend_theta","ftroptics.theoptics.Getbend_theta()"},
    {"theta","theta","ftroptics.theoptics.Gettheta_elastic()"},
    {"sieve_flag","sieve_flag","ftroptics.theoptics.Getsieve_flag()"},
    {"sieve_flag2","sieve_flag2","ftroptics.theoptics.Getsieve_flag2()"},
		{"p_firstorder", "p_firstorder", "ftroptics.theoptics.Getp_firstord()"},
    { 0 }
  };
  DefineVarsFromList( vars, mode );
  return 0;
}


