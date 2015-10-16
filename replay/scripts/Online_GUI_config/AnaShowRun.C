TTimer* tmer = NULL;
void flash()
{
	TCanvas* flashCanvas = new TCanvas("flashCanvas","flashCanvas",0,0,1200,900);
	flashCanvas->Draw();
	if(!tmer) tmer = new TTimer();
	if(tmer){
		tmer->SetCommand("if(gROOT->GetListOfCanvases()->FindObject(\"flashCanvas\")){flashCanvas->SetFillColor(gRandom->Rndm()*100);  flashCanvas->Modified(1); flashCanvas->Update();}");
		tmer->Start(1000);
	}
}

// void LHRS(int runN)
// {
//  replay_LHRS(runN,20000); 
//  online("detectorL",runN);
// flash();
// }

////////////////////////////////
#define REPLAY_EVT_NUM 50000
////////////////////////////////

void BB(int runN)
{
	replay_det_BB(runN,REPLAY_EVT_NUM); 
	online("detctorBB",runN);
 // online("mwdcBB",runN);
	flash();
}


void phys(int runN)
{
	replay_phys(runN,REPLAY_EVT_NUM); 
	online("physics",runN);
	flash();
}

void LHRS(int runN)
{
//	replay_det_L(runN,REPLAY_EVT_NUM);
	online("detectorL",runN);
//	online("richL",runN);
	online("pionrejL",runN);
//	flash();
}


Bool_t  IsScalarRootFileThere(int runN)
{
	const char ScalarRootFileForm[]="%s/e06014_scalar_%d.root";
	char filename[1000]="";
	Bool_t found=kFALSE;
	
	sprintf(filename,ScalarRootFileForm,"ROOTfiles",runN);
	
	
	cout << "Looking for "<<filename<<" for overwritten protection"<<endl;
	FILE *fd = fopen(filename,"r");
	if (fd != NULL) {
		found = 1;
		cout << "Found Scalar Root File : "<<filename<<endl;
		fclose(fd);
		found=kTRUE;
	}
	
	return found;
}

void Scalar(int runN)
{
	if (IsScalarRootFileThere(runN))
		online("scalar",runN);
	online("scalar",runN);
	flash();
}

void ParallelReplayLastRun(int runN)
{
	//!! Look for last run, full replay it's T5 events
	char * RunStrForm="xterm -g 80x10+0+400 -title \"Do NOT Close! Full Run T5 Replay\" -e ssh adaql4 \"export PATH=/bin:/opt/analyzer/bin:/opt/ROOT/pro/bin:/usr/bin; export LD_LIBRARY_PATH=/adaqfs/home/adaq/lib:/adaqfs/coda/2.2/Linux/lib:/opt/analyzer/lib:/opt/ROOT/pro/lib:/opt/lib;export DB_DIR=$ODIR/DB; cd %s; echo \"Replaying run %d for all coincident events!\"; analyzer -q 'replay_asym.C(%d,-1,0,1)' >> summaryfiles/replay_asym_%d.log; cd ROOTfiles; ln -sfv e06014_asym_%d.root e06014_scalar_%d.root; echo 'Done Replay'; sleep 20s;\"&";  
	cout<<"gSystem->Exec("<<RunStrForm<<");\n";
	char RunStr[1000]="";
	sprintf(RunStr,RunStrForm, gSystem->pwd(),runN-1,runN-1,runN-1,runN-1,runN-1);  
	cout<<"gSystem->Exec("<<RunStr<<");\n";
	gSystem->Exec(RunStr);
}

#define sdetectorBB "sdetectorBB"
#define sdetectorL "sdetectorL"
#define sphysics "sphysics"



void show_all(int runN)
{

	online("detectorBB",runN);
	online("detectorL",runN);
	online("pionrejL",runN);
//	online("physics",runN);
//	if (IsScalarRootFileThere(runN))
//		online("scalar",runN);
//	else if (IsScalarRootFileThere(runN-1))
//		online("scalar",runN-1);

//	online("richL",runN);
}

void Hopeless(int runN)
{
	online("detectorBB",runN);
	online("detectorL",runN);
	online("pionrejL",runN);
	online("physics",runN);
	if (IsScalarRootFileThere(runN))
		online("scalar",runN);
	else if (IsScalarRootFileThere(runN-1))
		online("scalar",runN-1);

	online("richL",runN);
}

void show_simple(int runN)
{
//	online("sdetectorBB",runN);
	online("sdetectorL",runN);
//	online("sphysics",runN);
}

void do_all(int runN)
{
	
	ParallelReplayLastRun(runN);
  	
  	//cout<<"waiting for data taking for 10s ...\n";
	//gSystem->Exec("sleep 10s");
  	
	TString cmd="online",run="";
	run+=runN;

	replay_det_L(runN,REPLAY_EVT_NUM);
	replay_det_BB(runN,REPLAY_EVT_NUM);
	gSystem->Exec((cmd+" -f sdetectorL -r ")+run+"&");
	gSystem->Exec((cmd+" -f sdetectorBB -r ")+run+"&");

  	//replay sclar of run N-1 because it needs full run replay
  	//cout<<"do_all : replay scalar for last run #"<<(runN-1)<<endl;
  	//if (!IsScalarRootFileThere(runN-1))
  	//replay_scalar(runN-1,-1);
  	

	replay_phys(runN,REPLAY_EVT_NUM*2);
	gSystem->Exec((cmd+" -f sphysics -r ")+run+"&");
  

  //show_simple(runN);
  
	flash();
}

// do_range(startRun,endRun)
// {
//  for(int runN = startRun; runN <= endRun; runN++)
//  {
//    replay_det(runN,REPLAY_EVT_NUM);·
//    replay_phys(runN,REPLAY_EVT_NUM);
//    replay_deadtime(runN,100000);
//   // online("detectorL",runN);
//    online("bigbite",runN);
//    online("physics",runN);
//  }
// flash();
// }


int AnaShowRun(UInt_t myrun/*, UInt_t nevt=-1*/)
{
	do_all(myrun);
  //replay(myrun,nevt);
  //online("replay",myrun);
}
