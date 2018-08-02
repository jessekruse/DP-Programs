

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

  // Jesse's variables and objects
  double timeDiff = 0;
  double timeDiffTotal = 0;
  int count = 0;
  double viewAngle[2];
  TCanvas *can = new TCanvas("can","can",0,0,600,600);
  TH1D *hist = new TH1D("hist","hist",100,-500,2500);
  hist->SetTitle("Arrival Time Differences; Time [ns]");
  hist->SetLineWidth(2);
  hist->GetXaxis()->SetTitleOffset(1.25);
  //

  TFile *f = new TFile(argv[1]);
  cout << "Loading:  " << argv[1] << endl;
  
  TTree *at=(TTree*)f->Get("AraTree");
  TTree *at2=(TTree*)f->Get("AraTree2");
  TTree *et=(TTree*)f->Get("eventTree");

  at->SetBranchAddress("detector",&detector);
  at->SetBranchAddress("icemodel",&icemodel);
  at->SetBranchAddress("settings",&settings);
  at2->SetBranchAddress("report",&report);
  at2->SetBranchAddress("event",&event);
  et->SetBranchAddress("UsefulAtriStationEvent",&realAtriEvtPtr);

  at->GetEntry(0);
  int nBins = settings->NFOUR/2;

  int nevt = at2->GetEntries();

  for(int iEvt=0; iEvt<nevt; iEvt++) {
  //int iEvt = 2367;
  //int iEvt = 2324;

    at2->GetEntry(iEvt);
    printf("Event %i \n",iEvt+1);
    int cnt = 0;
    int ecnt = 0;


    int istr, iant;

    for(int iStation=0; iStation<nStations; iStation++) {
      for(int istr=0; istr<nStrings; istr++) {
	for(int iant=0; iant<maxAntPerString; iant++) {

	  double arrv[2] = {report->stations[0].strings[istr].antennas[iant].arrival_time[0],report->stations[0].strings[istr].antennas[iant].arrival_time[1]};
	  int skip[2] = {report->stations[0].strings[istr].antennas[iant].skip_bins[0],report->stations[0].strings[istr].antennas[iant].skip_bins[1]};

	  //	  printf("Channel: %i \t LikelySol: %i \t Arr: %.3f   %.3f \n",CHANMAP[istr][iant],report->stations[0].strings[istr].antennas[iant].Likely_Sol,arrv[0]*1e9,arrv[1]*1e9);

	  timeDiff = (arrv[1]-arrv[0])*1e9;
	  viewAngle[0] = report->stations[0].strings[istr].antennas[iant].view_ang[0]*RAD2DEG;
	  viewAngle[1] = report->stations[0].strings[istr].antennas[iant].view_ang[1]*RAD2DEG;
	  

	  //	  if (fabs(viewAngle[0] - 56) < 1 && fabs(viewAngle[1] - 56) < 1 && timeDiff > 50) {
	  //	    printf("Channel: %i \t timeDiff: %f ns \t View Angle: %.3f   %.3f \n", CHANMAP[istr][iant],timeDiff,viewAngle[0],viewAngle[1]);
	    hist->Fill(timeDiff);
	    count++;
	    timeDiffTotal += timeDiff;
	    //	  }
	  

	}
      }
    }
  }

  double aveTimeDiff = timeDiffTotal/count;
  printf("Average Arrival Time Difference: %.3f ns \n",aveTimeDiff);


  can->cd();
  hist->Draw();

  can->Print("/home/jkruse/ARA/Summer2018Work/DPTimeDiffs/timeDiffDist.pdf");

 
}
