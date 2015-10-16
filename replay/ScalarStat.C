// Scalar Display Script
//   by Jin Huang <jinhuang@jlab.org>
//
// This is part of online display script for E06010 Transvesity scalar replay

#include <stdlib.h>
#include <TString.h>

UInt_t totEnt=1000000;

Double_t GetDiffRate(TString chan)
{
	Double_t bl=0,bh=0,
		ll=0,lh=0;
	TString sb="evbbite_", sl = "evleft_";
	sb+=chan;sl+=chan;

	cout<<"GetDiffRate : Set Branch of "<<sb<<" and "<<sl<<endl;
	T->SetBranchAddress(sb.Data(),&bl);
	T->SetBranchAddress(sl.Data(),&ll);
	T->GetEntry(100);
	T->SetBranchAddress(sb.Data(),&bh);
	T->SetBranchAddress(sl.Data(),&lh);
	T->GetEntry(totEnt-100);
	T->ResetBranchAddresses();
	//cout<<"GetDiffRate : "<<"bl="<<bl<<"; bh="<<bh<<"; ll="<<ll<<"; lh="<<lh<<endl;
	
	return 2 * ((bh-bl) - (lh-ll)) / ((bh-bl) + (lh-ll)+1e-10);
}


Double_t GetHeliDiffRate(TString chan,TString SpinState)
{

	Double_t pl=0,ph=0,ml=0,mh=0;
	
	TString pre="evbbite_";
	TString sp=pre+chan+SpinState, sm=pre+chan+SpinState;
	sp+="p";sm+="m";
	
	cout<<"GetDiffRate : Set Branch of "<<sp<<" and "<<sm<<endl;
	T->SetBranchAddress(sp.Data(),&pl);
	T->SetBranchAddress(sm.Data(),&ml);
	T->GetEntry(100);
	T->SetBranchAddress(sp.Data(),&ph);
	T->SetBranchAddress(sm.Data(),&mh);
	T->GetEntry(totEnt-100);
	T->ResetBranchAddresses();
		
	return 2 * ((ph-pl) - (mh-ml)) / ((ph-pl) + (mh-ml)+1e-10);
}

Double_t GetBCMDiffRate(TString chan,TString SpinHelState)
{

	Double_t pl=0,ph=0,ml=0,mh=0;
	
	TString pre="evbbite_";
	TString sp=pre+"u"+chan+SpinHelState, sm=pre+"d"+chan+SpinHelState;
	
	cout<<"GetDiffRate : Set Branch of "<<sp<<" and "<<sm<<endl;
	T->SetBranchAddress(sp.Data(),&pl);
	T->SetBranchAddress(sm.Data(),&ml);
	T->GetEntry(100);
	T->SetBranchAddress(sp.Data(),&ph);
	T->SetBranchAddress(sm.Data(),&mh);
	T->GetEntry(totEnt-100);
	T->ResetBranchAddresses();
	
	cout<<"GetDiffRate : "<<"pl="<<pl<<"; ph="<<ph<<"; ml="<<ml<<"; mh="<<mh<<endl;
	return 2 * ((ph-pl) - (mh-ml)) / ((ph-pl) + (mh-ml)+1e-10);
}

Double_t GetBCMLossRate(TString chan)
{

	Double_t th,tl,pph,ppl,pmh,pml,mph,mpl,mmh,mml;
	
	TString pre="evbbite_";
	TString st=pre+chan+"c",
		spp=pre+chan+"pp",
		spm=pre+chan+"pm",
		smp=pre+chan+"mp",
		smm=pre+chan+"mm";
	
	cout<<"GetDiffRate : Set Branch of "<<st<<spp<<spm<<smp<<smm<<endl;
	T->SetBranchAddress(st.Data(),&tl);
	T->SetBranchAddress(spp.Data(),&ppl);
	T->SetBranchAddress(spm.Data(),&pml);
	T->SetBranchAddress(smp.Data(),&mpl);
	T->SetBranchAddress(smm.Data(),&mml);
	T->GetEntry(100);
	T->SetBranchAddress(st.Data(),&th);
	T->SetBranchAddress(spp.Data(),&pph);
	T->SetBranchAddress(spm.Data(),&pmh);
	T->SetBranchAddress(smp.Data(),&mph);
	T->SetBranchAddress(smm.Data(),&mmh);
	T->GetEntry(totEnt-100);
	T->ResetBranchAddresses();
	
	return ((th-tl)- (pph-ppl) - (pmh-pml) - (mph-mpl) - (mmh-mml)) / (th-tl+1e-10);
}

void ScalarStat(Int_t CallType)
{

	totEnt=T->GetEntries();		
	switch (CallType){
//////////////////////////////////////////////
//      Scalar Fast Clock Count Diff 1D
//////////////////////////////////////////////		
	case 11:
		cout<<GetDiffRate("t1c")<<endl;
		break;
	case 12:
		cout<<GetDiffRate("t3c")<<endl;
		break;
	case 14:
		cout<<GetHeliDiffRate("t3","p")<<endl;
		break;
	case 15:
		cout<<GetBCMDiffRate("1","c")<<endl;
		break;
	case 16:
		cout<<GetBCMLossRate("d3")<<endl;
		break;
	case 1://helicity diff + vs -
		Double_t rate[12][5]={0};	
		TString rowname[]={"l1a","t1","t3","t5","t6","fclk","u1","u3","u10","d1","d3","d10"};
		TString colname[]={"p","m"};
		TString collabel[]={"Spin = +1","Spin = -1"};
		TPaveText *t=new TPaveText(0.05,0.05,0.95,0.95);
		t->SetTextFont(82);

		t->AddText("Scalar Counts Difference Rate (Hel=+1 - Hel=-1)/Average");
		t->AddText("=======================================================");
		TString tmpline="";
		for (Int_t i=0;i<12;i++)
			for (Int_t j=0;j<2;j++)
			{
				rate[i][j]=GetHeliDiffRate(rowname[i],colname[j]);
				//cout<<rowname[i]+colname[j]<<" = "<<rate[i][j]<<endl;
			}

		tmpline="Chan";
		for (Int_t j=0;j<2;j++)
		{
			char buff[1000]="";
			sprintf(buff,"%10s",collabel[j].Data());
			tmpline+=buff;
		}
		t->AddText(tmpline);

		t->AddText("------------------------------------------------------");
		for (Int_t i=0;i<12;i++)
		{
			char buff[1000]="";
			sprintf(buff,"%4s",rowname[i].Data());
			tmpline=buff;
			for (Int_t j=0;j<2;j++)
			{
				char buff[1000]="";
				sprintf(buff,"%10.2g",rate[i][j]);
				tmpline+=buff;
			}
			t->AddText(tmpline);
			
		}
		
		t->AddText("======================================================");
		t->AddText("");
		t->Draw();
		break;
	case 2://bcm diff upstream vs down stream
		Double_t rate[12]={0};	
		TString rowname[]={"l1a","t1","t3","t5","t6","fclk","u1","u3","u10","d1","d3","d10"};
		TPaveText *t=new TPaveText(0.05,0.05,0.95,0.95);
		t->SetTextFont(82);

		t->AddText("Flip Loss (Ungated - Sum All Gated)/Ungated");
		t->AddText("Scalar Count Diff due to Hel. and Spin Flip");
		t->AddText("===========================================");
		TString tmpline="";
		for (Int_t i=0;i<12;i++)
			for (Int_t j=0;j<2;j++)
			{
				rate[i]=GetBCMLossRate(rowname[i]);
				//cout<<rowname[i]+colname[j]<<" = "<<rate[i][j]<<endl;
			}

		for (Int_t i=0;i<12;i++)
		{
			char buff[1000]="";
			sprintf(buff,"Loss Rate of %4s",rowname[i].Data());
			tmpline=buff;
			sprintf(buff," = %10.2g",rate[i]);
			tmpline+=buff;
			t->AddText(tmpline);
			
		}
		
		t->AddText("===========================================");
		t->AddText("");
		t->Draw();
		break;
	case 3://scalar diff left vs bbite
		Double_t rate[12][5]={0};	
		TString rowname[]={"l1a","t1","t3","t5","t6","fclk","u1","u3","u10","d1","d3","d10"};
		TString colname[]={"c","pp","pm","mp","mm"};
		TString collabel[]={"ungated","s=+,h=+","s=+,h=-","s=-,h=+","s=-,h=-"};
		TPaveText *t=new TPaveText(0.05,0.05,0.95,0.95);
		t->SetTextFont(82);

		t->AddText("Scalar Counts Difference Rate (Bigbite - LHRS)/Average");
		t->AddText("======================================================");
		TString tmpline="";
		for (Int_t i=0;i<12;i++)
			for (Int_t j=0;j<5;j++)
			{
				rate[i][j]=GetDiffRate(rowname[i]+colname[j]);
				//cout<<rowname[i]+colname[j]<<" = "<<rate[i][j]<<endl;
			}

		tmpline="Chan";
		for (Int_t j=0;j<5;j++)
		{
			char buff[1000]="";
			sprintf(buff,"%10s",collabel[j].Data());
			tmpline+=buff;
		}
		t->AddText(tmpline);

		t->AddText("------------------------------------------------------");
		for (Int_t i=0;i<12;i++)
		{
			char buff[1000]="";
			sprintf(buff,"%4s",rowname[i].Data());
			tmpline=buff;
			for (Int_t j=0;j<5;j++)
			{
				char buff[1000]="";
				sprintf(buff,"%10.2g",rate[i][j]);
				tmpline+=buff;
			}
			t->AddText(tmpline);
			
		}
		
		t->AddText("======================================================");
		t->AddText("");
		t->Draw();
		break;
		
	default :
		cout<<"ScalarStat : Unknown Call Type "<<CallType<<endl;
	}
	
	
	
}
