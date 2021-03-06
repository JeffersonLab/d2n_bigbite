//////////////////////////////////////////////////////////////////////////
//
// Hall A analyzer replay script core
//
// https://hallaweb.jlab.org/wiki/index.php/ReplayCore
//
// Call this script after loading all the detectors to run replay
//
// if there is no rootlogon.C, execute following command in analyzer first
//   gSystem->Load("<your lib name>.so");
//  if you want to compile this script then you will also need
//   gSystem->AddIncludePath("-I<your include path>");
//   gInterpreter->AddIncludePath("<your include path>");
//
//////////////////////////////////////////////////////////////////////////
//  
//  Author : Jin Huang (jinhuang@jlab.org)    Aug 2007
//
//  Modify History:
//      Mar 2008    Jin Huang
//          add physics module support
//      Mar 2008    Jin Huang
//          separate two modes: physics replay and detector replay
//          also make it smarter
//      Apr 2008    Jin Huang
//          rootfile overwrite proof
//      May 2008    Jin Huang    
//          Separate core code of replay script to ReplayCore.C
//      May 2008    Jin Huang
//          Support Start Analyzing from an arbitary event number
//          Support Exit when getting an invalid answer.
//          Fix Bugs
//      Oct 2008    Jin Huang
//          Support raw data file name which do not have an endfix index
//      Nov 2008    Jin Huang
//          Clear Cut list before run
//      Nov 2008    Jin Huang
//          Support >2G file size on older compilers
//      Dec 2008    Jin Huang
//          Auto Runs
//
//////////////////////////////////////////////////////////////////////////


#ifndef ROOT_FARM_REPLAY_CORE
#define ROOT_FARM_REPLAY_CORE


//#include "def.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>


#ifndef __CINT__
//header file for compile

#include "TSystem.h"
#include "TROOT.h"
#include "THaEvent.h"
#include "THaRun.h"
#include "THaAnalyzer.h"

#include "TTree.h"
#include "TFile.h"
#include "TString.h"
#endif//#ifdef __CINT__

#define START_SCRIPT "./StartType.pl"

Bool_t IsFileExist(const Char_t * fname);

using namespace std;

void FarmReplayCore(
                Int_t runnumber=0,            //run #
				Int_t filesuffix=0,			  //suffix of data file, e.g. 0 for *.dat.0
                Int_t all=0,                  //-1=replay all;0=ask for a number
                Int_t DefReplayNum=-1,        //defaut replay event num
                char* OutFileFormat="%s/e06014_test_%d_%d.root", //output file format
                char* OutDefineFile="replay_test.odef",       //out define
                char* CutDefineFile="replay_test.cdef",       //cut define
                Bool_t EnableScalar=false,                    //Enable Scalar?
                Int_t FirstEventNum=0,         //First Event To Replay
                Bool_t QuietRun = kFALSE       //whether not ask question?
                )
{
		//general replay script core
		//
		// to do replay, use a script to load detectors then call this function
		//
		// it's also convenient to load detector manually in analyzer 
		// command line then call me

		// step 0: Run StartType.pl script

		cout << endl << "------------------------------------------------------" << endl;
		TString cmd = Form("%s %d", START_SCRIPT, runnumber);
		cout << "Running: " << cmd << endl;
		Int_t result = gSystem->Exec(cmd);
		if( result != 0 ) {
				cout << Form("ERROR:  '%s' failed") << endl;
				return;
		}
		cout << "------------------------------------------------------" << endl;

		// step 1: Init analyzer
		cout<<"replay: Init analyzer ..."<<endl;
		THaAnalyzer* analyzer = THaAnalyzer::GetInstance();
		if( analyzer ) {
				analyzer->Close();
		} else {
				analyzer = new THaAnalyzer;
		}
		gHaCuts->Clear();

		//Enable scalers
		if (EnableScalar)
		{
				cout<<"replay: Enabling Scalars"<<endl;
				analyzer->EnableScalers();
		}

		// step 2: setup run information
		cout<<"replay: configuring analyzer ..."<<endl;
		int nrun, nev;
		int found = 0;
		const char** path = 0;
		char filename[300],buf[300];  

		while( found==0 ) {
				if (runnumber<=0)
				{
						if (QuietRun)
								break;
						else
						{
								cout << "\nreplay: Please enter a Run Number (-1 to exit):";
								cin >> nrun;
								fgets(buf,300,stdin);//get the extra '\n' from stdin
								if( nrun<=0 ) break;
						}
				}
				else 
						nrun=runnumber;

				path=PATHS;

				while ( path && *path ) {
						sprintf(filename,RAW_DATA_FORMAT,*path,nrun,filesuffix);
						cout<<"replay: Try file "<<filename<<endl;
						if (IsFileExist(filename)) {
								found = 1;
								cout << "replay: Will analyze file "<<filename<<endl;
								break;
						}
						path++;
				}
				if ( (!path || !*path) && !found ) {
						cout << "replay: File not found. \n";
						if (runnumber>0) runnumber=0;
				}
		}
		if(nrun<=0) {
				gHaApps->Delete();
				gHaPhysics->Delete();
				gHaScalers->Delete();
				analyzer->Close();
				return;
		}


		if (all==0)
		{
				if (QuietRun)
				{
						nev=DefReplayNum;
				}
				else
				{
						cout << "\nreplay: Number of Events to analyze "
								<<" (default=";
						if (DefReplayNum>0)
								cout<<DefReplayNum<<"):";
						else 
								cout<<"replay all):";   


						//trick to support blank inputs
						fgets(buf,300,stdin);
						if (sscanf(buf,"%d\n",&nev)<1)
								nev=DefReplayNum;
				}
		}
		else 
				nev=all;


		char outname[300];
		sprintf(outname,OutFileFormat,STD_REPLAY_OUTPUT_DIR,nrun,filesuffix);
		found=0;    

		//rootfile overwrite proof
		while (found==0 && !QuietRun)
		{
				cout << "replay: Testing file "<<outname<<" for overwrite proof."<<endl;

				if (!IsFileExist(outname)) {found=1;break;}
				else
				{
						Long64_t NEnt=0;
						TFile *f=new TFile(outname);
						if (f)
						{
								TTree *t=(TTree *)f->Get("T");
								if (t!=0)  NEnt = (t->GetEntries());
						}
						delete f;
						TString DefOverWriting;
						if (NEnt==DefReplayNum || DefReplayNum<0 || NEnt==0)
								DefOverWriting="no";
						else DefOverWriting="yes";

						if (NEnt<=0) {
								cout <<endl << "replay: "<<outname
										<<" is an invalid root file, "
										<<"or other people is replaying it. ";
						}
						else{
								cout << "replay: "<<outname<<", which contains "<<NEnt
										<<" events, already exists. ";
						}

						cout<<"Do you want to overwrite it? "
								<<"(default="<<DefOverWriting.Data()<<"; enter \"c\" means exit):";

						//trick to support blank inputs
						fgets(buf,300,stdin);
						TString s(buf);
						s.Chop();
						s.ToLower();
						if (s.IsNull()) s=DefOverWriting;

						if (s=="y" || s=="yes") {found=1;break;}
						else if (s=="n" || s=="no"){
								sprintf(outname,OutFileFormat,
												CUSTOM_REPLAY_OUTPUT_DIR,nrun,filesuffix);
								cout<<endl
										<<"replay: please enter the output root file name. "<<endl
										<<"        leave blank = "<<outname<<endl
										<<"Root File:";

								//trick to support blank inputs
								fgets(buf,300,stdin);
								if (buf[0]!='\n' and buf[0]!=0){
										strcpy(outname,buf);
								}

								//clear the last char if it's '\n'
								if (outname[strlen(outname)-1]=='\n') outname[strlen(outname)-1]=0;
						}
						else {
								cout<<"replay: " 
										<<s.Data()
										<<"is not a valid input; Exiting."
										<<endl;
								gHaApps->Delete();
								gHaPhysics->Delete();
								analyzer->Close();
								return;
						}
				}
		}

		cout<<endl<<"----------------------------------------------"<<endl;
		cout<<"replay: Inputs Summary:"<<endl;
		cout<<"        Raw data: "<<filename<<endl;
		cout<<"        Event   : ";
		if (nev<0) cout <<"all events";
		else cout << nev <<" events";
		cout<<", starting from Event # "<<FirstEventNum<<endl;
		cout<<"        Outputs : "<<outname<<endl;
		cout<<"----------------------------------------------"<<endl<<endl;

		cout<<"replay: Setup run inputs/outputs ..."<<endl;

		analyzer->SetOutFile( outname );
		analyzer->SetOdefFile(OutDefineFile);
		analyzer->SetCutFile(CutDefineFile);  
		char sumname[300];
		sprintf(sumname,SUMMARY_PHYSICS_FORMAT,nrun);
		analyzer->SetSummaryFile(sumname); // optional
		analyzer->SetEvent( new THaEvent );
		//analyzer->SetMarkInterval(ANA_MARK_INTERVAL);

		//correct the offset on the last event if first event is above 0
		if (nev>=0) nev+=FirstEventNum;

		// step 3: run it
		cout<<"replay: Start Process ..."<<endl;

		TString oldfilename="";
		THaRun *oldrun=0, *run, *runlist[30]={0};Int_t runidx=0;
		Bool_t exit=false;
		//UInt_t  NeveAna=0;
		// Removed loop over nsplit: farm jobs will be one per data file
		sprintf(filename,RAW_DATA_FORMAT,"raw data paths",nrun,filesuffix);
		cout<<"replay: trying file "<<filename<<endl;

		path=PATHS;found=0;
		while ( path && *path ) {
				sprintf(filename,RAW_DATA_FORMAT,*path,nrun,filesuffix);
				cout<<"replay: Try file "<<filename<<endl;

				if (IsFileExist(filename)) {
						found = 1;
						cout << "replay: Will analyze file "<<filename<<endl;
						break;
				}
				path++;
		}

		//cout<<"======================\n"<<oldfilename<<endl<<filename<<endl;
		if ( ((!path || !*path) && !found) || oldfilename==filename) {
				cout << "replay: no more raw data file to analyze"<<endl;
				exit=true;
		}
		else{
				oldfilename=filename;
				//do the analysis
				if( oldrun ) {
						run = new THaRun(*oldrun);
						run->SetFilename(filename);
				} else {
						run = new THaRun(filename);
				}
				runlist[runidx]=run; runidx++;

				if(nev>=0) run->SetLastEvent(nev);
				run->SetFirstEvent(FirstEventNum);

				//run->Print();
				analyzer->Process(run); 
				//NeveAna=run->GetNumAnalyzed();
				//cout << "replay: "<<NeveAna<<"events were analyzed."<<endl;

				//if (NeveAna-1>=(UInt_t)nev) exit=true;
				run->Close();
				if (!oldrun) oldrun = run;
				//else delete run;
		}



		// step 3: clean up
		cout<<"replay: Cleaning up ... "<<endl;
		for (runidx--;runidx>=0;runidx--){
				assert(runlist[runidx]);
				delete runlist[runidx];
		}

		//====================================
		//quick fix for end-of-replay crashes:
		//====================================

		/*  gHaApps->Delete();
			gHaPhysics->Delete();
			gHaScalers->Delete();
			analyzer->Close();
			*/
		cout<<"replay: YOU JUST ANALYZED RUN NUMBER "<<nrun<<"."<<endl;
}

/////////////////////////////////////////////////////////////////

Bool_t IsFileExist(const Char_t * fname)
{
    fstream testfile;

    testfile.open(fname,ios_base::in);
    Bool_t isopen=testfile.is_open();
    testfile.close();

    return isopen;
}


// DO NOT DELETE!
#endif//#define ROOT_REPLAY_CORE
