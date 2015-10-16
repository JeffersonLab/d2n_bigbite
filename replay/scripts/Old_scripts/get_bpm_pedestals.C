//
//  script to extract BPM pedestals using the Hall A analyzer
//

// by Bodo Reitz in April 2003
//
// modified by Vince Sulkosky October 2008
//
// other files needed
// - analyzer
// - bpm pedestal run:
//   this is a run taken under special run conditions:
//   1.   MCC hast to follow the following procedure
//   1.1    Open BPM window "BPM Diagnostics - SEE"
//   1.2    From there pull down the "Expert Screens" and open "Gain Control"
//   1.3    For IOCSE10, use the pull down window and change from "auto gain"
//          to "forced gain". Then change the Forced Gain values x and y to zero.
//   2.   CODA has to run in pedestal mode
//   the files have usually a distinct filename


#include "def.h"

using namespace std;

void get_bpm_pedestals(Int_t runnumber=0,Int_t DefReplayNum=-1,Int_t all=0,char* OutFileFormat="%s/e06010_bpmped_%d.root"){

  static const char* RAW_DATA_FORMAT2="%s/e06010_%d.dat.0";
  int found=0;
  int nrun=0, nev;
  const char** path = 0;
  char filename[300],buf[300];

  OutDefineFile="bpmped.odef";       //out define
  CutDefineFile="replay_test.cdef";       //cut define
  FILE *fd;

  // step 1: Init analyzer
  cout<<"replay: Init analyzer ..."<<endl;
  THaAnalyzer* analyzer = THaAnalyzer::GetInstance();
  if( analyzer ) {
    analyzer->Close();
  } else {
    analyzer = new THaAnalyzer;
  }

  // step 2: setup run information
  while( found==0 ) {
    if (runnumber<=0)
      {
	cout<<"Please enter a Run Number (-1 to exit):"<<endl;
	cin >> nrun;
	if( nrun<=0 ) break;
      } else
      nrun = runnumber;

    path=PATHS;

    while ( path && *path ) {
      sprintf(filename,RAW_DATA_FORMAT2,*path,nrun);
      cout<<"replay: Try file "<<filename<<endl;
      fd = fopen(filename,"r");
      if (fd != NULL) {
	found = 1;
	cout << "replay: Will analyze file "<<filename<<endl;
	fclose(fd);
	break;
      }
      path++;
    }

  }

  if(nrun<=0) {
    gHaApps->Delete();
    gHaPhysics->Delete();
    analyzer->Close();
    return;
  }

  if (all==0)
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
  else 
    nev=all;

  char outname[300];
  sprintf(outname,OutFileFormat,STD_REPLAY_OUTPUT_DIR,nrun);
  found=0;    

  //rootfile overwrite proof
  while (found==0)
    {
      cout << "replay: Testing file "<<outname<<" for overwrite proof."<<endl;
      fd = fopen(outname,"r");
      
      if (fd == NULL) {found=1;break;}
      else
	{
	  fclose(fd);
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
		    CUSTOM_REPLAY_OUTPUT_DIR,nrun);
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
		<<" is not a valid input; Exiting."
		<<endl;
	    gHaApps->Delete();
	    gHaPhysics->Delete();
	    analyzer->Close();
	    return;
	  }
	}
    }

  THaRun* run = new THaRun( filename );

// Set up the equipment to be analyzed.

 THaApparatus* BEAM1 = new THaUnRasteredBeam("urb","Beamline");
 gHaApps->Add( BEAM1 );

// Define the analysis parameters

 analyzer->SetEvent( new THaEvent  );
 analyzer->SetOutFile( outname  );
 analyzer->SetOdefFile(OutDefineFile);
 analyzer->SetCutFile(CutDefineFile);  
 if(nev>=0) run->SetLastEvent(nev);
 analyzer->SetMarkInterval(ANA_MARK_INTERVAL);

analyzer->Process(*run);

TCanvas* c1 = new TCanvas("c1","BPM Pedestals");
Int_t pedestal[8];

c1->Clear();
c1->Divide(2,2);

// would be nice to do the following in a loop, but dont know how to create arrays of
// histograms using the outputdef file from the analyzer

c1->cd(1);
bpmaraw1->Draw();
bpmaraw1->Fit("gaus");
TF1* f1=bpmaraw1->GetFunction("gaus");
pedestal[0]=f1->GetParameter("Mean");

c1->cd(2);
bpmaraw2->Draw();
bpmaraw2->Fit("gaus");
TF1* f1=bpmaraw2->GetFunction("gaus");
pedestal[1]=f1->GetParameter("Mean");

c1->cd(3);
bpmaraw3->Draw();
bpmaraw3->Fit("gaus");
TF1* f1=bpmaraw3->GetFunction("gaus");
pedestal[2]=f1->GetParameter("Mean");

c1->cd(4);
bpmaraw4->Draw();
bpmaraw4->Fit("gaus");
TF1* f1=bpmaraw4->GetFunction("gaus");
pedestal[3]=f1->GetParameter("Mean");

cout<<" Please change the pedestals for BPMA to:"<<endl;

for (Int_t i=0; i<4; i++) {
  cout<<" "<<pedestal[i]<<" ";
}

c1->Update();

cout<<endl<<"Press Any Key and Enter to continue"<<endl;
char dummy[255];
cin>>dummy;

c1->Clear();
c1->Divide(2,2);

// would be nice to do the following in a loop, but dont know how to create arrays of
// histograms using the outputdef file from the analyzer

c1->cd(1);
bpmbraw1->Draw();
bpmbraw1->Fit("gaus");
TF1* f1=bpmbraw1->GetFunction("gaus");
pedestal[4]=f1->GetParameter("Mean");

c1->cd(2);
bpmbraw2->Draw();
bpmbraw2->Fit("gaus");
TF1* f1=bpmbraw2->GetFunction("gaus");
pedestal[5]=f1->GetParameter("Mean");

c1->cd(3);
bpmbraw3->Draw();
bpmbraw3->Fit("gaus");
TF1* f1=bpmbraw3->GetFunction("gaus");
pedestal[6]=f1->GetParameter("Mean");

c1->cd(4);
bpmbraw4->Draw();
bpmbraw4->Fit("gaus");
TF1* f1=bpmbraw4->GetFunction("gaus");
pedestal[7]=f1->GetParameter("Mean");

cout<<" Please change the pedestals for BPMB to:"<<endl;
for (Int_t i=4; i<8; i++) {
  cout<<" "<<pedestal[i]<<" ";
}


cout<<endl<<"Looking forward to working with you again, good bye"<<endl;

}
