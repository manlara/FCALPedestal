#include <inttypes.h>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <utility>
#include <vector>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <sstream>

#include "TText.h"
#include "TLatex.h"
#include "TLine.h"
#include "TLegend.h"
#include "TPaveStats.h"
#include "TVirtualPad.h"
#include "TFrame.h"
#include "TGraphPainter.h"
#include "TString.h"
#include "TCollection.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TF1.h"
#include "TGraph.h"
#include "TMultiGraph.h"
#include "TGraphErrors.h"
#include "TMinuit.h"
#include "TKey.h"
#include "TDatime.h"
#include "TMath.h"
#include "TAxis.h"
#include "TTree.h"
#include "TBranch.h"
#include "TH2F.h"
#include "TColor.h"
#include "TStyle.h"
#include "TPaletteAxis.h"
#include "TFitResultPtr.h"
#include "TFitResult.h"


#include <TGClient.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TRandom.h>
#include <TGButton.h>
#include <TGFrame.h>
#include <TRootEmbeddedCanvas.h>
#include <RQ_OBJECT.h>

#include "StringUtilities.h"
#include "Pedestal.h"


void PedAnalysis(TString filename, bool plot_fits = false){
    
  TFile * f = new TFile(filename);
  
  TTree* FCAL_Analysis = (TTree*)f->Get("FCAL_Analysis");
  
  uint32_t rocid;
  uint32_t slot;
  uint32_t channel;
  int x;
  int y;
  uint32_t nSamples;
  uint32_t nEvents;
  uint32_t eventnum;
  uint32_t run;
  double avg_pedestal;
  double avg_rms;
  double rms_pedestal;
  double rms_rms;
  uint32_t nentries_rms;
  uint32_t nentries_pedestal;
  
  FCAL_Analysis->SetBranchAddress("rocid",    &rocid  );    // vme crate number
  FCAL_Analysis->SetBranchAddress("slot",     &slot   );     // board number
  FCAL_Analysis->SetBranchAddress("channel",  &channel);  // board channel number
  FCAL_Analysis->SetBranchAddress("x",        &x      );        // x coordinate where the center of (0,0)
  FCAL_Analysis->SetBranchAddress("y",        &y      );        // y coordinate where the center of (0,0)
  FCAL_Analysis->SetBranchAddress("rocid",    &rocid  );    // vme crate number
  FCAL_Analysis->SetBranchAddress("avg_pedestal", &avg_pedestal); // pedestal calculated by averaging nEvents
  FCAL_Analysis->SetBranchAddress("avg_rms",      &avg_rms     );      // root-mean-square of pedestal average
  FCAL_Analysis->SetBranchAddress("nentries_pedestal",      &nentries_pedestal);      // number of entries used in pedestal average
  FCAL_Analysis->SetBranchAddress("rms_pedestal", &rms_pedestal); // rms of pedestal calculated by averaging nEvents
  FCAL_Analysis->SetBranchAddress("rms_rms",      &rms_rms);      // root-mean-square of pedestal rms
  FCAL_Analysis->SetBranchAddress("nentries_rms",      &nentries_rms);      // number of entries used in pedestal rms
  FCAL_Analysis->SetBranchAddress("nSamples", &nSamples); // the number of samples used in pedestal average
  FCAL_Analysis->SetBranchAddress("nEvents", &nEvents);    // the number of events to be averaged in the pedestal calc
  FCAL_Analysis->SetBranchAddress("eventnum", &eventnum); // event number
  FCAL_Analysis->SetBranchAddress("run",      &run);      // run number
  
  map<TString, TGraphErrors*> map_pedestal;
  map<TString, TGraph*> map_rms;
  
  int maxEvents = 20;
  
  // The rocid is between rocid>=11 && rocid<=22
  int max_rocid = 22, min_rocid=11;
  // 3 to 19
  // skip 11 and 12
  int max_slot = 19, min_slot=3;
  // 16 channels starting from 0
  int n_channels = 16;
  for (int m_roc=min_rocid; m_roc<=max_rocid; m_roc++){
    for (int m_slot=min_slot; m_slot<=max_slot; m_slot++){
      for (int m_channel=0; m_channel<n_channels; m_channel++){
        for (int m_nevents=1; m_nevents<=maxEvents; m_nevents++){
          char name[50];
          sprintf(name,"%02i/%02i/%02i",m_roc,m_slot,m_channel);
          TString key = TString(name)+"_"+StringUtilities::int2TString(m_nevents);
          map_pedestal[key] = new TGraphErrors();
          map_pedestal[key]->SetName(name);
          map_rms[key]      = new TGraph();
          map_rms[key]->SetName(name);
        }
      }
    }
  }
  
  
  int nEntries = FCAL_Analysis->GetEntries();
  for (int i=0; i<nEntries; i++){
    FCAL_Analysis->GetEntry(i);
    
    if (nEvents>maxEvents) continue;
    
    char name_daq[50];
    sprintf(name_daq,"%02i/%02i/%02i",rocid,slot,channel);
    
    char name_xy[50];
    sprintf(name_xy,"(%i,%i)",x,y);
    
    TString key = TString(name_daq)+"_"+StringUtilities::int2TString(nEvents);
    
    int n_ped = map_pedestal[key]->GetN();
    map_pedestal[key]->SetPoint(n_ped, nSamples, avg_pedestal);
    map_pedestal[key]->SetPointError(n_ped, 0, rms_pedestal);
    if (n_ped==0){
      map_pedestal[key]->SetTitle("Pedestal, Events = "+StringUtilities::int2TString(nEvents)+": "+name_xy);
    }
    
    int n_rms = map_rms[key]->GetN();
    map_rms[key]->SetPoint(n_ped, nSamples, rms_pedestal);
    if (n_rms==0){
      map_rms[key]->SetTitle("RMS of Pedestal, Events = "+StringUtilities::int2TString(nEvents)+": "+name_xy);
    }
  }
  
  // Histogram the fit paramaters A and B in A + B/sqrt(N) for each nEvents
  TH1F* histA[maxEvents];
  TH1F* histB[maxEvents];
  double maxA[maxEvents];
  double minA[maxEvents];
  double maxB[maxEvents];
  double minB[maxEvents];
  for (int i=1; i<=maxEvents; i++){
    TString evt = StringUtilities::int2TString(i);
    histA[i-1] = new TH1F("histA_"+evt,"A in A+B/#sqrt(Samples Averaged) for nEvents = "+evt,4096*100,0,4096);
    histB[i-1] = new TH1F("histB_"+evt,"B in A+B/#sqrt(Samples Averaged) for nEvents = "+evt,2*4096*100,-4096,4096);
    maxA[i-1] = 0;
    minA[i-1] = 1e3;
    maxB[i-1] = 0;
    minB[i-1] = 1e3;
  }
  
  // A single canvas for plotting that gets overridden
  TCanvas* c_ped = new TCanvas("c_ped","c_ped",900,900);
  c_ped->Divide(4,4);
  int counter = 0;
  int c_counter = 1;
  
  // Fit options
  TString fitOptions = "RS"; // R=fit in TF1 range, S=save fit result, and W=ignore error bars
  if (!plot_fits) fitOptions += "0N"; // do not draw
  
  // Since the 2 maps used in this analysis share the same key
  // loop over both at once
  for (map<TString, TGraphErrors*>::const_iterator iter=map_pedestal.begin(); iter!=map_pedestal.end(); ++iter){    
    
    TString key = iter->first;
    TGraphErrors* gr_ped = iter->second;
    TGraph* gr_rms = map_rms[key];
    
    vector<TString> parseKey = StringUtilities::parseTString(key,"_");
    int nevt = StringUtilities::TString2int(parseKey[1]);
    
    gr_ped->Sort();
    gr_rms->Sort();
    
    Int_t N_ped = gr_ped->GetN();
    Int_t N_rms = gr_rms->GetN();
    
    if (N_ped==0 || N_rms==0) continue;
    
    counter++;
    
    // Fit rms of pedestal with
    // A + B * 1/sqrt(NSamples)
    TF1* fitShape = new TF1(key,"[0] + [1]*1/TMath::Sqrt(x)",2,14); // Range = 2 to 14 nSamples
    fitShape->SetParameter(0, 0.2);
    fitShape->SetParameter(1, 1.0);
    
    // Draw 
    if (plot_fits){
      c_ped->cd(counter);
      c_ped->Update();
      gr_rms->Draw("A");
      gStyle->SetTitleFontSize(0.1);
      gr_rms->GetYaxis()->SetTitle("rms pedestal");
      gr_rms->GetXaxis()->SetTitle("samples averaged");
      gr_rms->GetYaxis()->SetTitleOffset(0.8);
      gr_rms->GetYaxis()->SetTitleSize(0.06);
      gr_rms->GetXaxis()->SetTitleSize(0.06);
      gr_rms->Draw("AP");
    }
    
    TFitResultPtr r = gr_rms->Fit(fitShape,fitOptions);
    cout << plot_fits << endl;
    if (plot_fits && counter%16==0) {
      c_ped->Update();
      c_ped->SaveAs("plots/FitRmsVsSample/FitRmsVsSample-"+StringUtilities::int2TString(c_counter)+".jpg");
      counter=0;
      c_counter++;
    }
    TString status = gMinuit->fCstatu;
    if (!(status.Contains("CONVERGED"))) {
      cerr << "Warning: Didn't Converge... " << status << "\t" << key << endl;
    }
    
    Int_t fitStatus = r;
    // 0 means OK
    if (fitStatus!=0){
      cerr << "Fitting error... " << r << "\t" << key << endl;
    } 
    
    // Write params to histograms
    Double_t par0 = r->Value(0);
    histA[nevt-1]->Fill(par0);
    if (maxA[nevt-1]<par0) maxA[nevt-1] = par0;
    if (minA[nevt-1]>par0) minA[nevt-1] = par0;
    Double_t par1 = r->Value(1);
    histB[nevt-1]->Fill(par1);
    if (maxB[nevt-1]<par0) maxB[nevt-1] = par1;
    if (minB[nevt-1]>par0) minB[nevt-1] = par1;
  }
  
  
  TCanvas* c_par[maxEvents];
  for (int i=1; i<=maxEvents; i++){
    TString evt = StringUtilities::int2TString(i);
    
    histA[i-1]->GetXaxis()->SetRangeUser(minA[i-1]*0.9,maxA[i-1]*1.1);
    histB[i-1]->GetXaxis()->SetRangeUser(minB[i-1]*0.9,maxB[i-1]*1.1);
    
    c_par[i-1] = new TCanvas("c_par_"+evt,"Event "+evt,1200,900);
    c_par[i-1]->Divide(2,1);
    c_par[i-1]->cd(1);
    histA[i-1]->Draw();
    c_par[i-1]->cd(2);
    histB[i-1]->Draw();
    
    c_par[i-1]->SaveAs("plots/FitParams-"+evt+".pdf");
  }
}


void PedAnalysis_FitShape(){
  
  // A and B vs nEvents
  double A[] = {0.2153, 0.1582, 0.1336, 0.1192, 0.1095, 0.1022, 0.09648, 0.09263, 0.08891, 0.08563, 0.08363, 0.08146, 0.07813, 0.07776, 0.07528, 0.07422, 0.07248, 0.0715, 0.07043, 0.06962};
  double B[] = {1.106,  0.7751, 0.6266, 0.5396, 0.4791, 0.4348, 0.39740, 0.37070, 0.34950, 0.33050, 0.31090, 0.29770, 0.28450, 0.26860, 0.26220, 0.25280, 0.24490, 0.2353, 0.22780, 0.22140};
  int n = sizeof(A)/sizeof(*A);
  double nevents[n];
  for (int i=0; i<n; i++){
    nevents[i] = i+1;
  }
  
  TGraph* grA = new TGraph(n, nevents, A);
  TGraph* grB = new TGraph(n, nevents, B);
  grA->SetTitle("A vs Events used in average");
  grB->SetTitle("B vs Events used in average");
  grA->GetXaxis()->SetTitle("events used in average");
  grB->GetXaxis()->SetTitle("events used in average");
  
  TCanvas* c_par = new TCanvas("c_par","Fit Params",1200,900);
  c_par->Divide(2,1);
  c_par->cd(1);
  grA->Draw("AP");
  c_par->cd(2);
  grB->Draw("AP");
  
  // Draw the fit pdf A + B/sqrt(N)
  TCanvas* c_shape = new TCanvas("c_shape","c_shape",900,900);
  c_shape->cd();
  
  TLegend* leg = new TLegend(0.6, 0.45, 0.9, 0.85);
  TGraph* fitShape[n];
  TMultiGraph* mg = new TMultiGraph();
  //mg->SetTitle("Average RMS of Pedestal, Events used in average 1-20;sampls used in average; adc counts");
  mg->SetTitle("Avg(A) + Avg(B)/sqrt(N);samples used in average; adc counts");
  for (int i=0; i<n; i++){
    TString evt = StringUtilities::int2TString(i+1);
    TF1* fitLine = new TF1("fitShape","[0] + [1]/sqrt(x)", 1,20);
    fitLine->SetParameter(0, A[i]);
    fitLine->SetParameter(1, B[i]);
    fitShape[i] = new TGraph();
    for (int j=1; j<=20; j++){
      fitShape[i]->SetPoint(j-1, j, fitLine->Eval(j));
    }
    if (i<9) fitShape[i]->SetLineColor(i+1);
    else fitShape[i]->SetLineColor(i+19);
    mg->Add(fitShape[i]);
    leg->AddEntry(fitShape[i],"Event = "+evt,"l");
  }
  mg->Draw("ac");
  // Draw minimun y line
  double minA = TMath::MinElement(n,A);
  TLine* line = new TLine(0,minA,20,minA);
  line->SetLineColor(kRed);
  line->SetLineStyle(kDashed);
  line->SetLineWidth(2);
  line->Draw();
  // Draw legend
  leg->Draw();
  c_shape->SaveAs("plots/AverageRMS_Event1-20.pdf");
  /*
  TF1* fitShape[n];
  for (int i=0; i<n; i++){
    TString evt = StringUtilities::int2TString(i+1);
    fitShape[i] = new TF1("fitShape-"+evt,"[0] + [1]/sqrt(x)", 1,20);
    fitShape[i]->SetParameter(0, A[i]);
    fitShape[i]->SetParameter(1, B[i]);
    fitShape[i]->SetRange(1, 0, 20, 1.2);
    if (i<9) fitShape[i]->SetLineColor(i+1);
    else fitShape[i]->SetLineColor(i+19);
    if (i==0) fitShape[i]->Draw();
    else fitShape[i]->Draw("SAME");
  }
  */
}
