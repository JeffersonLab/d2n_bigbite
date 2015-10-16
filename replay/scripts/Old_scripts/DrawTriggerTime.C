// Scalar Display Script
//   by Jin Huang <jinhuang@jlab.org>
//
// This is part of online display script for E06010 Transvesity scalar replay

#include <stdlib.h>
#include <TH1.h>

void DrawTriggerTime(Int_t CallType)
{
	TString sDraw, 
		sHistNameBase, 
		sTitle, 
		sXTitle, 
		sYTitle, 
		sCut,
		sOption;
		
	Bool_t bLogy=kFALSE,
		bShowStat=kTRUE;
	
	
	Double_t evbbite_fclk=0,evleft_fclk=0,fclkDiffOffSet=0;
	//T->SetBranchAddress("evbbite_fclk",&evbbite_fclk);
	//T->SetBranchAddress("evleft_fclk",&evleft_fclk);
	//T->GetEntry(100);
	//T->ResetBranchAddresses();
	fclkDiffOffSet=413088;//evbbite_fclk-evleft_fclk;
	
	switch (CallType){
//////////////////////////////////////////////
//      Scalar Fast Clock Count Diff 1D
//////////////////////////////////////////////		
	case 1:
		//T->Draw("(evbbite_fclk-evleft_fclk)>>hdiff","evbbite_fclk>1000");
		sDraw="(evbbite_fclk-evleft_fclk)-";
		sDraw+=fclkDiffOffSet;
		sHistNameBase="hdiff";
		sTitle="Ungated 103kHz Clock Diff Between Left & Bigbite scalar";
		sXTitle="Time Diff - Initial OffSet /103kHz";
		sYTitle="Count";
		sCut="evbbite_fclk>10"; //wait for start run sync
		bLogy=kTRUE;
		bShowStat=kFALSE;
		break;
	case 2:
		//T->Draw("(evbbite_fclkpp-evleft_fclkpp)>>hdiffmm","evbbite_fclk>1000");
		sDraw="(evbbite_fclkpp-evleft_fclkpp)-";
		sDraw+=0;
		sHistNameBase="hdiffpp";
		sTitle="(++) 103kHz Clock Diff ";
		sXTitle="Time Diff /103kHz";
		sYTitle="Count";
		sCut="evbbite_fclk>10"; //wait for start run sync
		bLogy=kTRUE;
		bShowStat=kFALSE;
		break;
	case 3:
		//T->Draw("(evbbite_fclkpm-evleft_fclkpm)>>hdiffpm","evbbite_fclk>1000");
		sDraw="(evbbite_fclkpm-evleft_fclkpm)-";
		sDraw+=0;
		sHistNameBase="hdiffpm";
		sTitle="(+-) 103kHz Clock Diff";
		sXTitle="Time Diff/103kHz";
		sYTitle="Count";
		sCut="evbbite_fclk>10"; //wait for start run sync
		bLogy=kTRUE;
		bShowStat=kFALSE;
		break;
	case 4:
		//T->Draw("(evbbite_fclkmp-evleft_fclkmp)>>hdiffmp","evbbite_fclk>1000");
		sDraw="(evbbite_fclkmp-evleft_fclkmp)-";
		sDraw+=0;
		sHistNameBase="hdiffmp";
		sTitle="(-+) 103kHz Clock Diff";
		sXTitle="Time Diff/103kHz";
		sYTitle="Count";
		sCut="evbbite_fclk>10"; //wait for start run sync
		bLogy=kTRUE;
		bShowStat=kFALSE;
		break;
	case 5:
		//T->Draw("(evbbite_fclkmm-evleft_fclkmm)>>hdiffmm","evbbite_fclk>1000");
		sDraw="(evbbite_fclkmm-evleft_fclkmm)-";
		sDraw+=0;
		sHistNameBase="hdiffmm";
		sTitle="(--) 103kHz Clock Diff";
		sXTitle="Time Diff/103kHz";
		sYTitle="Count";
		sCut="evbbite_fclk>10"; //wait for start run sync
		bLogy=kTRUE;
		bShowStat=kFALSE;
		break;
		
//////////////////////////////////////////////
//      Scalar Fast Clock Count Diff 2D
//////////////////////////////////////////////	
	
	case 11:
		//T->Draw("(evbbite_fclk-evleft_fclk):evbbite_fclk","evbbite_fclk):evbbite_fclk",
		//"evbbite_fclk>1000","COLZ");
		sDraw="(evbbite_fclk-evleft_fclk-";
		sDraw+=fclkDiffOffSet;
		sDraw+="):evbbite_fclk/6180000";
		sHistNameBase="hdiffvstime";
		sXTitle="Time/min";
		sYTitle="Count Diff";
		sTitle="Ungated Scalar Fast Clock Diff vs Time/103kHz";
		sOption="";
		bShowStat=kFALSE;
		sCut="evbbite_fclk>10"; //wait for start run sync
		break;
	case 12:
		//T->Draw("(evbbite_fclkpp-evleft_fclkpp)>>hdiffmm","evbbite_fclk>1000");
		sDraw="(evbbite_fclkpp-evleft_fclkpp-";
		sDraw+=0;
		sDraw+="):evbbite_fclk/6180000";
		sHistNameBase="hdiffppvstime";
		sTitle="(++) 103kHz Clock Diff";
		sXTitle="Time/min";
		sYTitle="Time Diff /103kHz";
		sCut="evbbite_fclk>10"; //wait for start run sync
		//bLogy=kTRUE;
		bShowStat=kFALSE;
		break;
	case 13:
		//T->Draw("(evbbite_fclkpm-evleft_fclkpm)>>hdiffpm","evbbite_fclk>1000");
		sDraw="(evbbite_fclkpm-evleft_fclkpm-";
		sDraw+=0;
		sDraw+="):evbbite_fclk/6180000";
		sHistNameBase="hdiffpmvstime";
		sTitle="(+-) 103kHz Clock Diff";
		sXTitle="Time/min";
		sYTitle="Time Diff/103kHz";
		sCut="evbbite_fclk>10"; //wait for start run sync
		//bLogy=kTRUE;
		bShowStat=kFALSE;
		break;
	case 14:
		//T->Draw("(evbbite_fclkmp-evleft_fclkmp)>>hdiffmp","evbbite_fclk>1000");
		sDraw="(evbbite_fclkmp-evleft_fclkmp-";
		sDraw+=0;
		sDraw+="):evbbite_fclk/6180000";
		sHistNameBase="hdiffmpvstime";
		sTitle="(-+) 103kHz Clock Diff";
		sXTitle="Time/min";
		sYTitle="Time Diff/103kHz";
		sCut="evbbite_fclk>10"; //wait for start run sync
		//bLogy=kTRUE;
		bShowStat=kFALSE;
		break;
	case 15:
		//T->Draw("(evbbite_fclkmm-evleft_fclkmm)>>hdiffmm","evbbite_fclk>1000");
		sDraw="(evbbite_fclkmm-evleft_fclkmm-";
		sDraw+=0;
		sDraw+="):evbbite_fclk/6180000";
		sHistNameBase="hdiffmmvstime";
		sTitle="(--) 103kHz Clock Diff";
		sXTitle="Time/min";
		sYTitle="Time Diff/103kHz";
		sCut="evbbite_fclk>10"; //wait for start run sync
		//bLogy=kTRUE;
		bShowStat=kFALSE;
		break;
//////////////////////////////////////////////
//      Data Taking
//////////////////////////////////////////////		
	case 21:
		//T->Draw("he3.Spin:evbbite_fclk","he3.IsSpinValid","COLZ");
		sDraw="he3.Spin:evbbite_fclk/6180000";
		sHistNameBase="SpinvsTime";
		sTitle="Data Rate vs Spin State vs Time";
		sXTitle="Time/min";
		sYTitle="Target Spin";
		sCut="evbbite_fclk>10&&he3.IsSpinValid"; //wait for start run sync
		sOption="COLZ";
		bShowStat=kFALSE;
		break;	
	case 22:
		//T->Draw("fEvtHdr.fEvtNum:evbbite_fclk>>EvtVSTime","evbbite_fclk>1000");
		sDraw="fEvtHdr.fEvtNum:evbbite_fclk/6180000";
		sHistNameBase="EvtVSTime";
		sTitle="Data Rate vs Time";
		sXTitle="Time/min";
		sYTitle="Event Number";
		sCut="evbbite_fclk>10"; //wait for start run sync
		sOption="";
		bShowStat=kFALSE;
		break;
	case 23:
		//T->Draw("fEvtHdr.fEvtNum:evbbite_fclk>>EvtVSTime","evbbite_fclk>1000");
		sDraw="(evbbite_u3r+evbbite_d3r)/13000:evbbite_fclk/6180000";
		sHistNameBase="HBCMVSTime";
		sTitle="Beam current vs Time";
		sXTitle="Time/min";
		sYTitle="Aproxy Beam Current (uA)";
		sCut="evbbite_fclk>10"; //wait for start run sync
		sOption="";
		bShowStat=kFALSE;
		break;
		

		
//////////////////////////////////////////////
//      Scalar Fast Clock Count Diff 2D
//////////////////////////////////////////////	
	
	case 31:
		sDraw="(evbbite_l1ac-evleft_l1ac-";
		sDraw+=0;
		sDraw+="):evbbite_fclk/6180000";
		sHistNameBase="hl1acdiff";
		sXTitle="Time/min";
		sYTitle="L1 Accept Count Diff";
		sTitle="L1 Accept vs time (min)";
		sOption="";
		bShowStat=kFALSE;
		sCut="evbbite_fclk>10"; //wait for start run sync
		break;
	
	case 32:
		sDraw="(evbbite_t1c-evleft_t1c-";
		sDraw+=0;
		sDraw+="):evbbite_fclk/6180000";
		sHistNameBase="ht1diff";
		sXTitle="Time/min";
		sYTitle="T1 Count Diff";
		sTitle="T1 diff vs time (min)";
		sOption="";
		bShowStat=kFALSE;
		sCut="evbbite_fclk>10"; //wait for start run sync
		break;
		
	case 33:
		sDraw="(evbbite_t3c-evleft_t3c-";
		sDraw+=0;
		sDraw+="):evbbite_fclk/6180000";
		sHistNameBase="ht3diff";
		sXTitle="Time/min";
		sYTitle="T3 Count Diff";
		sTitle="T3 diff vs time (min)";
		sOption="";
		bShowStat=kFALSE;
		sCut="evbbite_fclk>10"; //wait for start run sync
		break;
		
	case 34:
		sDraw="(evbbite_t5c-evleft_t5c-";
		sDraw+=0;
		sDraw+="):evbbite_fclk/6180000";
		sHistNameBase="ht5diff";
		sXTitle="Time/min";
		sYTitle="T5 Count Diff";
		sTitle="T5 diff vs time (min)";
		sOption="";
		bShowStat=kFALSE;
		sCut="evbbite_fclk>10"; //wait for start run sync
		break;
		
	case 35:
		sDraw="(evbbite_t6c-evleft_t6c-";
		sDraw+=0;
		sDraw+="):evbbite_fclk/6180000";
		sHistNameBase="ht6diff";
		sXTitle="Time/min";
		sYTitle="T6 Count Diff";
		sTitle="T6 diff vs time (min)";
		sOption="";
		bShowStat=kFALSE;
		sCut="evbbite_fclk>10"; //wait for start run sync
		break;	
		
//////////////////////////////////////////////
//      Tests
//////////////////////////////////////////////				
	case 101:
		//T->Draw("(evbbite_fclk-evleft_fclk):evbbite_fclk","evbbite_fclk):evbbite_fclk",
		//"evbbite_fclk>1000","COLZ");
		sDraw="(evbbite_fclk-evleft_fclk):evbbite_fclk/6180000";
		sHistNameBase="hdiffvstime";
		sTitle="Ungated Scalar Fast Clock Diff vs Time/min";
		sXTitle="Time/min";
		sYTitle="Ungated 103kHz Clock Diff Between Left & Bigbite";
		sCut="evbbite_fclk>10"; //wait for start run sync
		sOption="";
		bShowStat=kFALSE;
		break;
		
	default :
		cout<<"DrawTriggerTime : Unknown Call Type "<<DrawTriggerTime<<endl;
	}
	
	
//////////////////////////////////////////////
//			Draw it!
//////////////////////////////////////////////
	sHistNameBase+=rand ();
	sDraw+=">>";
	sDraw+=sHistNameBase;
	
	gStyle->SetPalette(1);
	cout<<"DrawTriggerTime : T->Draw(\""<<sDraw<<"\",\""<<sCut<<"\",\""<<sOption<<"\");\n";
	T->Draw(sDraw.Data(),sCut.Data(),sOption.Data());
	gPad->SetLogy(bLogy);
	
	TH1 * hist=NULL;
	if (hist=(TH1 *)((TH1 *)(gDirectory->FindObject(sHistNameBase.Data()))))
	{
		cout<<"DrawTriggerTime : Tagging the graph"<<endl;
		hist->GetXaxis()->SetTitle(sXTitle.Data());
		hist->GetYaxis()->SetTitle(sYTitle.Data());//sYTitle.Data()
		hist->SetTitle(sTitle.Data());
		hist->SetStats(bShowStat);
	}
	
}
