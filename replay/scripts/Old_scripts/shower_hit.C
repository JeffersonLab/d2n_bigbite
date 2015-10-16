void shower_hit(TString vHist){
  TH2F *hits = (TH2F*)gDirectory->Get(vHist);
  gStyle->SetPalette(1,0);
  gStyle->SetOptStat(0);
  hits->Draw("colz");
}
