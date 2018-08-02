/*
  Author: Jesse Kruse
  Purpose: This program is designed to plot the waveforms for
  a single event or a collection of events with using AraReco.
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
int nStations = 1;
int nStrings = 4;
int maxAntPerString = 4;
int nMaxAnt = 16;

void printAllEventWaveforms(char* filename, TTree *et, TTree *at);
void printEventWaveforms(int EVT, TTree *et, TTree *at);

int main(int argc, char **argv) {

  if (argc<2) {printf("Need input file name\n"); return 1;}

  Detector *detector=0; 
  Settings *settings=0;
  IceModel *icemodel=0;
  Report   *report=0;
  Event    *event=0;

  TFile *f = new TFile(argv[1]);
  cout << "Loading:  " << argv[1] << endl;

  int EVT = 0;
  char* filename = argv[2];
  FILE *file = fopen(filename,"r");

  // Check if second argument is a text file
  // If not, it's an event.  
  if (!file) {
    EVT = atoi(argv[2]);
    printf("Not text file found. Using event %d\n",EVT);
  }


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

  // retrieve data from AraTree
  at->GetEntry(0);

  if (file) {
    cout << "Loading: " << filename << endl;
    printAllEventWaveforms(filename,et,at);
  }
  else {
    printEventWaveforms(EVT,et,at);
  }


  fclose(file);

}


/*
  This function is designed to print the waveforms for a 
  collection of events without using AraReco. These events are 
  specified by a text file containing the event numbers. 
*/  
void printAllEventWaveforms(char* filename, TTree *et,TTree *at) {

  at->GetEntry(0);

  UsefulAtriStationEvent *realAtriEvtPtr=0;
  et->SetBranchAddress("UsefulAtriStationEvent",&realAtriEvtPtr);

  FILE *inFile = fopen(filename,"r");
  char buff[255];
  int SIZE = 0;
  int EVT = 0;

  // For small files where I can use all of the events
  // Otherwise, just use a smaller sample of say 200

  // Figure out how many events there are in the file
  while (fscanf(inFile,"%s",buff) != EOF) 
    SIZE++;

  // Gotta close and reopen the file to get the right
  // values.
  fclose(inFile);
  inFile = fopen(filename,"r");


  // Create an array of canvases This saves sooooo much space.
  TCanvas *canArr[SIZE];

  for (int n=0; n<SIZE; n++) {
    // naming and initializing each canvas
    string num = Form("%i",n);
    TString name = "can" + num;
    canArr[n] = new TCanvas(name,name,800,800);
    canArr[n]->Divide(4,4);
  }

  // Load events   
  for (int i=0; i<SIZE; i++) {

    // get line from text file
    fscanf(inFile,"%s",buff);
    EVT = atoi(buff); // convert to integer
    cout << EVT << endl;
    et->GetEntry(EVT);

    // get waveform for each antenna (channel) and print it on 
    // canvas
    for(int iStation=0; iStation<nStations; iStation++) {
      for(int istr=0; istr<nStrings; istr++) {
	for(int iant=0; iant<maxAntPerString; iant++) {
	  
	  TGraph *gra = realAtriEvtPtr->getGraphFromRFChan(CHANMAP[istr][iant]);

	  canArr[i]->cd(CHANMAP[istr][iant]+1);
	  gra->DrawClone("al");
	  
	}
      }
    }
  }


  // save waveforms to a pdf
  canArr[0]->Print("/home/jkruse/ARA/Summer2018Work/SimData/waveforms.pdf(");
  for (int j=1; j<SIZE-1; j++) 
      canArr[j]->Print("/home/jkruse/ARA/Summer2018Work/SimData/waveforms.pdf");
  canArr[SIZE-1]->Print("/home/jkruse/ARA/Summer2018Work/SimData/waveforms.pdf)");


  fclose(inFile);

}


/*
  This function is designed to print the waveforms for a single
  event without using AraReco
*/ 
void printEventWaveforms(int EVT, TTree* et, TTree *at) {

  cout << "\n\nHere?\n\n";

  at->GetEntry(0);

  UsefulAtriStationEvent *realAtriEvtPtr=0;
  et->SetBranchAddress("UsefulAtriStationEvent",&realAtriEvtPtr);
  
  TCanvas *can = new TCanvas("can","can",800,800);
  can->Divide(4,4);

  et->GetEntry(EVT);

  for(int iStation=0; iStation<nStations; iStation++) {
    for(int istr=0; istr<nStrings; istr++) {
      for(int iant=0; iant<maxAntPerString; iant++) {
	
	TGraph *gra = realAtriEvtPtr->getGraphFromRFChan(CHANMAP[istr][iant]);
	
	can->cd(CHANMAP[istr][iant]+1);
	gra->Draw();
	
      }
    }
  }  

  can->Print("/home/jkruse/ARA/Summer2018Work/SimData/eventWaveforms.pdf");

}
