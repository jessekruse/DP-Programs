/*
  Author: Jesse Kruse
  Purpose: This program is designed to find the exact number of 
  double pulse events in a given AraSim root file
*/

#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include "time.h"

#include "TFile.h"
#include "TTree.h"
#include "TH1.h"
#include "TCanvas.h"

#include "Tools.h"
#include "Constants.h"
#include "Vector.h"
#include "Position.h"
#include "EarthModel.h"
#include "IceModel.h"
#include "Efficiencies.h"
#include "Spectra.h"
#include "Event.h"
#include "Trigger.h"
#include "Detector.h"
#include "Settings.h"
//#include "counting.hh"
#include "Primaries.h"
#include "signal.hh"
#include "secondaries.hh"

#include "UsefulAtriStationEvent.h"

#include "ASlibEdit.h"
#include "outLoadSP.h"

int CHANMAP[4][4] = { {5,13,1,9},{6,14,2,10},{7,15,3,11},{4,12,0,8}};
double RAD2DEG = 180.0/TMath::Pi();

int main(int argc, char **argv) {

  if (argc<2) {printf("Need input file name\n"); return 1;}

  UsefulAtriStationEvent *realAtriEvtPtr=0;
  Detector *detector=0; 
  Settings *settings=0;
  IceModel *icemodel=0;
  Report   *report=0;
  Event    *event=0;

  int nStations = 1;
  int nStrings = 4;
  int maxAntPerString = 4;
  int nMaxAnt = 16;

  TFile *f = new TFile(argv[1]);
  cout << "Loading:  " << argv[1] << endl;
  
  // Get the trees from the root file
  TTree *at=(TTree*)f->Get("AraTree");
  TTree *at2=(TTree*)f->Get("AraTree2");
  TTree *et=(TTree*)f->Get("eventTree");

  // get the branches from the trees
  at->SetBranchAddress("detector",&detector);
  at->SetBranchAddress("icemodel",&icemodel);
  at->SetBranchAddress("settings",&settings);
  at2->SetBranchAddress("report",&report);
  at2->SetBranchAddress("event",&event);
  et->SetBranchAddress("UsefulAtriStationEvent",&realAtriEvtPtr);

  // retrieve data from AraTree
  at->GetEntry(0);
  int nBins = settings->NFOUR/2;

  int nevt = at2->GetEntries();

  // create canvases and histograms for the various graphs
  // I wanted to make
  TCanvas *can1 = new TCanvas("can1","can1",0,0,600,600);
  TCanvas *can2 = new TCanvas("can2","can2",0,0,600,600);
  TCanvas *can3 = new TCanvas("can3","can3",0,0,600,600);
  TCanvas *can4 = new TCanvas("can4","can4",0,0,600,600);
  TCanvas *can5 = new TCanvas("can5","can5",0,0,600,600);
  TCanvas *can15 = new TCanvas("can15","can15",0,0,600,600);
  TCanvas *can16 = new TCanvas("can16","can16",0,0,600,600);
  TCanvas *can22 = new TCanvas("can22","can22",0,0,600,600);
  TCanvas *can23 = new TCanvas("can23","can23",0,0,600,600);
  TH1D *hist1 = new TH1D("hist1","hist1",100,0,6000);
  TH1D *hist2 = new TH1D("hist2","hist2",100,0,6000);
  TH2D *hist3 = new TH2D("hist3","hist3",100,0,3500,100,-3500,0);
  TH1D *hist4 = new TH1D("hist4","hist4",100,0,2300);
  TH2D *hist5 = new TH2D("hist5","hist5",100,0,3500,100,-3500,0);
  TH2D *hist6 = new TH2D("hist6","hist6",100,0,3500,100,-3500,0);
  TH2D *hist7 = new TH2D("hist7","hist7",100,0,3500,100,-3500,0);
  TH2D *hist8 = new TH2D("hist8","hist8",100,0,3500,100,-3500,0);
  TH2D *hist9 = new TH2D("hist9","hist9",100,0,3500,100,-3500,0);
  TH2D *hist10 = new TH2D("hist10","hist10",100,0,3500,100,-3500,0);
  TH2D *hist11 = new TH2D("hist11","hist11",100,0,3500,100,-3500,0);
  TH2D *hist12 = new TH2D("hist12","hist12",100,0,3500,100,-3500,0);
  TH2D *hist13 = new TH2D("hist13","hist13",100,0,3500,100,-3500,0);
  TH2D *hist14 = new TH2D("hist14","hist14",100,0,3500,100,-3500,0);
  TH1D *hist15 = new TH1D("hist15","hist15",100,0,1000);
  TH2D *hist16 = new TH2D("hist16","hist16",100,0,3500,100,-3500,0);
  TH2D *hist17 = new TH2D("hist17","hist17",100,0,3500,100,-3500,0);
  TH2D *hist18 = new TH2D("hist18","hist18",100,0,3500,100,-3500,0);
  TH2D *hist19 = new TH2D("hist19","hist19",100,0,3500,100,-3500,0);
  TH2D *hist20 = new TH2D("hist20","hist20",100,0,3500,100,-3500,0);
  TH2D *hist21 = new TH2D("hist21","hist21",100,0,3500,100,-3500,0);
  TH1D *hist22 = new TH1D("hist22","hist22",100,0,100);
  TH2D *hist23 = new TH2D("hist23","hist23",100,0,4000,100,0,4000);

  hist1->SetTitle("Direct Max Voltage; Voltage [mV]");
  hist2->SetTitle("Reflected Max Voltage; Voltage [mV]");
  hist3->SetTitle("Double Pulse Events; Radius [m]; Depth [m]");
  hist4->SetTitle("Time Differences; Time Diff [ns]");
  hist5->SetTitle("Time Difference Positions; Radius [m]; Depth [m]");
  hist15->SetTitle("DP Time Differences; Time Diff [ns]");
  hist16->SetTitle("< 200mV; Radius [m]; Depth [m]");
  hist17->SetTitle("> 200mV + < 400mV; Radius [m]; Depth [m]");
  hist18->SetTitle("> 400mV + < 800mV; Radius [m]; Depth [m]");
  hist19->SetTitle("> 800mV + < 1600mV; Radius [m]; Depth [m]");
  hist20->SetTitle("> 1600mV + < 3200mV; Radius [m]; Depth [m]");
  hist21->SetTitle("> 3200mV; Radius [m]; Depth [m]");
  hist22->SetTitle("Number of Threshold Excursions for Each Pulse;");
  hist23->SetTitle("Direct Max V vs. Reflected Max V; Voltage [mV]; Voltage [mV]");

  hist1->SetLineWidth(2);
  hist2->SetLineWidth(2);
  hist3->SetLineWidth(2);
  hist4->SetLineWidth(2);
  hist5->SetLineWidth(2);
  hist6->SetLineWidth(2);
  hist7->SetLineWidth(2);
  hist8->SetLineWidth(2);
  hist9->SetLineWidth(2);
  hist10->SetLineWidth(2);
  hist11->SetLineWidth(2);
  hist12->SetLineWidth(2);
  hist13->SetLineWidth(2);
  hist14->SetLineWidth(2);
  hist15->SetLineWidth(2);
  hist16->SetLineWidth(2);
  hist17->SetLineWidth(2);
  hist18->SetLineWidth(2);
  hist19->SetLineWidth(2);
  hist20->SetLineWidth(2);
  hist21->SetLineWidth(2);
  hist22->SetLineWidth(2);
  hist23->SetLineWidth(2);

  hist1->GetXaxis()->SetTitleOffset(1.25);
  hist2->GetXaxis()->SetTitleOffset(1.25);
  hist3->GetXaxis()->SetTitleOffset(1.25);
  hist3->GetYaxis()->SetTitleOffset(1.25);
  hist4->GetXaxis()->SetTitleOffset(1.25);
  hist15->GetXaxis()->SetTitleOffset(1.25);
  hist23->GetXaxis()->SetTitleOffset(1.25);
  hist23->GetYaxis()->SetTitleOffset(1.25);

  hist5->SetLineColor(kRed);
  hist6->SetLineColor(kBlue);
  hist7->SetLineColor(kGreen);
  hist8->SetLineColor(kYellow);  
  hist9->SetLineColor(kMagenta);
  hist10->SetLineColor(kCyan);
  hist11->SetLineColor(kOrange);
  hist12->SetLineColor(kViolet);  
  hist13->SetLineColor(kGray);
  hist14->SetLineColor(kBlack);
  hist16->SetLineColor(kRed);
  hist17->SetLineColor(kBlue);
  hist18->SetLineColor(kGreen);
  hist19->SetLineColor(kCyan);
  hist20->SetLineColor(kOrange);
  hist21->SetLineColor(kViolet);


  double source[4];
  double stationCenter[3];

  // get station center coordinates
  stationCenter[0] = detector->stations[0].GetX();
  stationCenter[1] = detector->stations[0].GetY();
  stationCenter[2] = detector->stations[0].GetZ();




  // loop over events
  for(int iEvt=0; iEvt<nevt; iEvt++) {

    at2->GetEntry(iEvt);
    et->GetEntry(iEvt);
    //    printf("Event %i \n",iEvt+1);
    int ndp = 0;
    int ichl = 0;

    for(int iStation=0; iStation<nStations; iStation++) {
      for(int istr=0; istr<nStrings; istr++) {
	for(int iant=0; iant<maxAntPerString; iant++) {

	  double dirMaxV = report->stations[iStation].strings[istr].antennas[iant].V[0][0];
	  double refMaxV = report->stations[iStation].strings[istr].antennas[iant].V[1][0];
	  int voltVecSize = report->stations[iStation].strings[istr].antennas[iant].V[0].size(); /* I checked both the direct and ref
												    and they are both the same */
	  int numHits = 0;//

	  /* Loop over points in direct and reflected voltage
	     arrays and find the max value for both
	  */
	  for (int ipnt=0;ipnt<voltVecSize;ipnt++) {
	    if (report->stations[iStation].strings[istr].antennas[iant].V[0][ipnt] > dirMaxV)
	      dirMaxV = report->stations[iStation].strings[istr].antennas[iant].V[0][ipnt];
	    if (report->stations[iStation].strings[istr].antennas[iant].V[1][ipnt] > refMaxV)
	      refMaxV = report->stations[iStation].strings[istr].antennas[iant].V[1][ipnt];
	  }

	  // Get waveform from antenna channel
	  TGraph *gra = realAtriEvtPtr->getGraphFromRFChan(CHANMAP[istr][iant]);
	  // Get propagation time from source to antenna for 
	  // direct and reflected path
	  double dirPathT = report->stations[iStation].strings[istr].antennas[iant].arrival_time[0]*1e9;
	  double refPathT = report->stations[iStation].strings[istr].antennas[iant].arrival_time[1]*1e9;
	  // calculate time difference between direct and 
	  // reflected paths
	  double timeDiff = refPathT - dirPathT;

	  // find source coordinates relative to station center
	  source[0] = event->Nu_Interaction[0].posnu.GetX() - stationCenter[0];
	  source[1] = event->Nu_Interaction[0].posnu.GetY() - stationCenter[1];
	  source[2] = event->Nu_Interaction[0].posnu.GetZ();
	  source[3] = event->Nu_Interaction[0].posnu.R() - icemodel->Surface(event->Nu_Interaction[0].posnu.Lon(),event->Nu_Interaction[0].posnu.Lat());

	  // Convert direct and reflected max voltage from V 
	  // to mV
	  dirMaxV *= 1000;
	  refMaxV *= 1000;

	  double hitTime1 = 0;
	  double hitTime2 = 0;
	  bool hit1 = false;
	  bool hit2 = false;
	  int npnt = gra->GetN();

	  // loop through points of waveform from antenna channel
	  for (int ipnt=0;ipnt<npnt;ipnt++) {

	    double val = fabs(gra->GetY()[ipnt]);
	    double time = gra->GetX()[ipnt];

	    // if voltage exceeds rms*6 then hit1 = true and 
	    // record time
	    if (val > 228 && !hit1) {
	      hit1 = true;
	      hitTime1 = time;
	      numHits++;//
	    }

	    // count number of excursions beyond threshold in
	    // time delay
	    //
	    if (hit1 && time<hitTime1+timeDiff*0.6) {
	      if (val > 228) numHits++;
	    }
	    //

	    // If we find a second hit in the waveform then it's
	    // a DP
	    if (timeDiff > 20 && dirMaxV > 228 && refMaxV > 228) {
	      if ( time > (hitTime1+timeDiff*0.6) && val > 228 && !hit2) {
		hit2 = true;
		ndp++;
	      }
	    }
	  }

	  
	  hist1->Fill(dirMaxV);
	  hist2->Fill(refMaxV);
	  hist4->Fill(timeDiff);
	  if (hit2)
	    hist15->Fill(timeDiff);


	  //
	  if (hit2){
	    printf("Event: %d\t NumHits: %d\n",iEvt,numHits);
	    hist22->Fill(numHits);
	  }
	  //

	  hist23->Fill(refMaxV,dirMaxV);

	  // Spatial Position of events with different timeDiffs
	  if (timeDiff < 200)
	    hist5->Fill(sqrt(source[0]*source[0]+source[1]*source[1]),source[3]);
	  if (timeDiff > 200 && timeDiff < 400)
	    hist6->Fill(sqrt(source[0]*source[0]+source[1]*source[1]),source[3]);
	  if (timeDiff > 400 && timeDiff < 600)
	    hist7->Fill(sqrt(source[0]*source[0]+source[1]*source[1]),source[3]);
	  if (timeDiff > 600 && timeDiff < 800)
	    hist8->Fill(sqrt(source[0]*source[0]+source[1]*source[1]),source[3]);
	  if (timeDiff > 800 && timeDiff < 1000)
	    hist9->Fill(sqrt(source[0]*source[0]+source[1]*source[1]),source[3]);
	  if (timeDiff > 1000 && timeDiff < 1200)
	    hist10->Fill(sqrt(source[0]*source[0]+source[1]*source[1]),source[3]);
	  if (timeDiff > 1200 && timeDiff < 1400)
	    hist11->Fill(sqrt(source[0]*source[0]+source[1]*source[1]),source[3]);
	  if (timeDiff > 1400 && timeDiff < 1600)
	    hist12->Fill(sqrt(source[0]*source[0]+source[1]*source[1]),source[3]);
	  if (timeDiff > 1600 && timeDiff < 1800)
	    hist13->Fill(sqrt(source[0]*source[0]+source[1]*source[1]),source[3]);
	  if (timeDiff > 1800)
	    hist14->Fill(sqrt(source[0]*source[0]+source[1]*source[1]),source[3]);

	  // Spatial position of events with different directMaxVs
	  if (refMaxV < 200)
	    hist16->Fill(sqrt(source[0]*source[0]+source[1]*source[1]),source[3]);
	  if (refMaxV > 200 && refMaxV < 400)
	    hist17->Fill(sqrt(source[0]*source[0]+source[1]*source[1]),source[3]);
	  if (refMaxV > 400 && refMaxV < 800)
	    hist18->Fill(sqrt(source[0]*source[0]+source[1]*source[1]),source[3]);
	  if (refMaxV > 800 && refMaxV < 1600)
	    hist19->Fill(sqrt(source[0]*source[0]+source[1]*source[1]),source[3]);
	  if (refMaxV > 1600 && refMaxV < 3200)
	    hist20->Fill(sqrt(source[0]*source[0]+source[1]*source[1]),source[3]);
	  if (refMaxV > 3200)
	    hist21->Fill(sqrt(source[0]*source[0]+source[1]*source[1]),source[3]);
	 
	 
	}
      }
    }

    if (ndp > 0){
      printf("%d\n",iEvt);
      hist3->Fill(sqrt(source[0]*source[0]+source[1]*source[1]),source[3]);
    }
      
  }
  


  can1->cd();
  hist1->Draw();
  can2->cd();
  hist2->Draw();
  can3->cd();
  hist3->Draw("box");
  can4->cd();
  hist4->Draw();
  can15->cd();
  hist15->Draw();
  
  // drawing timeDiffs in space
  can5->cd();
  hist5->Draw("box");
  can5->Update();
  hist6->Draw("same box");
  can5->Update();
  hist7->Draw("same box");
  can5->Update();
  hist8->Draw("same box");
  can5->Update();
  hist9->Draw("same box");
  can5->Update();
  hist10->Draw("same box");
  can5->Update();
  hist11->Draw("same box");
  can5->Update();
  hist12->Draw("same box");
  can5->Update();
  hist13->Draw("same box");
  can5->Update();
  hist14->Draw("same box");
  hist5->SetStats(false);
  TLegend *legend = new TLegend(0.75,0.75,0.95,0.95);
  legend->AddEntry(hist5,"< 200ns","f");
  legend->AddEntry(hist6,"> 200ns + < 400ns","f");
  legend->AddEntry(hist7,"> 400ns + < 600ns","f");
  legend->AddEntry(hist8,"> 600ns + < 800ns","f");
  legend->AddEntry(hist9,"> 800ns + < 1000ns","f");
  legend->AddEntry(hist10,"> 1000ns + < 1200ns","f");
  legend->AddEntry(hist11,"> 1200ns + < 1400ns","f");
  legend->AddEntry(hist12,"> 1400ns + < 1600ns","f");
  legend->AddEntry(hist13,"> 1600ns + < 1800ns","f");
  legend->AddEntry(hist14,"> 1800ns","f");
  legend->Draw();

  // Drawing directMaxVs positions
  can16->Divide(2,3);
  can16->cd(1);
  hist16->Draw("box");
  can16->cd(2);
  hist17->Draw("box");
  can16->cd(3);
  hist18->Draw("box");
  can16->cd(4);
  hist19->Draw("box");
  can16->cd(5);
  hist20->Draw("box");
  can16->cd(6);
  hist21->Draw("box");

  can22->cd();
  hist22->Draw();
  gStyle->SetPalette(1);
  can23->cd();
  hist23->Draw("colz");

  can1->Print("/home/jkruse/ARA/Summer2018Work/SimData/ExactNumAraSimDPs/exactNumAraSimDPs.pdf(");
  can2->Print("/home/jkruse/ARA/Summer2018Work/SimData/ExactNumAraSimDPs/exactNumAraSimDPs.pdf");
  can3->Print("/home/jkruse/ARA/Summer2018Work/SimData/ExactNumAraSimDPs/exactNumAraSimDPs.pdf");
  can4->Print("/home/jkruse/ARA/Summer2018Work/SimData/ExactNumAraSimDPs/exactNumAraSimDPs.pdf");
  can5->Print("/home/jkruse/ARA/Summer2018Work/SimData/ExactNumAraSimDPs/exactNumAraSimDPs.pdf");
  can15->Print("/home/jkruse/ARA/Summer2018Work/SimData/ExactNumAraSimDPs/exactNumAraSimDPs.pdf");
  can16->Print("/home/jkruse/ARA/Summer2018Work/SimData/ExactNumAraSimDPs/exactNumAraSimDPs.pdf");
  can22->Print("/home/jkruse/ARA/Summer2018Work/SimData/ExactNumAraSimDPs/exactNumAraSimDPs.pdf");
  can23->Print("/home/jkruse/ARA/Summer2018Work/SimData/ExactNumAraSimDPs/exactNumAraSimDPs.pdf)");

}


  
