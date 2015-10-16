void vdc_draw(TString vHist) {

  TH1D *vdc = (TH1D*)gDirectory->Get(vHist);

  gStyle->SetOptStat(0);
  vdc->SetMinimum(0.8);
  vdc->Draw();

}
