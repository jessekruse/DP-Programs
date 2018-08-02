/*
  Author: Jesse Kruse
  Purpose: This program is designed to smooth waveforms 
  for an event amongst other things
*/

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <unistd.h>
#include <assert.h>
#include "time.h"

#include "TFile.h"
#include "TTree.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TString.h"

#include "SimFileReader.h"
#include "FileReader.h"
#include "StationGeometry.h"
#include "Channel.h"
#include "Pos.h"
#include "OpticalIce.h"
#include "ChannelCollection.h"

using namespace std;

TGraph* SmoothWaveform(TGraph *waveform);
void printAbsOfWaveforms(ChannelCollection chan);
void printWaveforms(ChannelCollection chan);
double getNewRMS(TGraph *waveform);

int main(int argc, char **argv) {

  if (argc<2) {printf("Need input file names\n"); return 1;}

  cout << "\nLoading: " << argv[1] << endl;
  cout << "Event: " << argv[2] << "\n\n";

  // Declare some variables
  TGraph *waveforms[16];
  int EVT = atoi(argv[2]);
  int numEvts;

  // Load the file containing the filtered events
  SimFileReader reader(argv[1]);

  numEvts = reader.getTotalNumEvents()-1;

  // Get the station geometry from the file
  StationGeometry *geom=reader.getStationGeometry();
  Channel::setGeometry(geom);
  CurvedRay::setOpticalIce(new OpticalIce);
  Pos::setOriginStatic(geom->getOriginPosition());
  OpticalIce *ice = new OpticalIce();

  // Set max number of events for the reader
  reader.setMaxNumEvents(numEvts);

  /*  TCanvas *cans[16];

  cans[0] = new TCanvas("can0","can0",600,600);
  cans[1] = new TCanvas("can1","can1",600,600);
  cans[2] = new TCanvas("can2","can2",600,600);
  cans[3] = new TCanvas("can3","can3",600,600);  
  cans[4] = new TCanvas("can4","can4",600,600);
  cans[5] = new TCanvas("can5","can5",600,600);
  cans[6] = new TCanvas("can6","can6",600,600);
  cans[7] = new TCanvas("can7","can7",600,600);
  cans[8] = new TCanvas("can8","can8",600,600);
  cans[9] = new TCanvas("can9","can9",600,600);
  cans[10] = new TCanvas("can10","can10",600,600);
  cans[11] = new TCanvas("can11","can11",600,600);
  cans[12] = new TCanvas("can12","can12",600,600);
  cans[13] = new TCanvas("can13","can13",600,600);
  cans[14] = new TCanvas("can14","can14",600,600);
  cans[15] = new TCanvas("can15","can15",600,600);
  */
  
  TCanvas *can = new TCanvas("can","can",800,800);
  can->Divide(4,4);
  
  // Load event 
  reader.loadEvent(EVT);
  
  ChannelCollection chan = reader.getChannelCollection();
  int nChl = chan.getNumChans();
  /*
  for (int ichl=0; ichl<nChl; ichl++) {
    double rms = chan.getChannel(ichl)->getGeometry()->getPosition(ichl).getVRMS();
    waveforms[ichl] = chan.getChannel(ichl)->getWaveform();
    waveforms[ichl] = SmoothWaveform(waveforms[ichl]);

    // Create new threshold line
    double newThresh = 6*rms/sqrt(10);
    TLine *line = new TLine(waveforms[ichl]->GetX()[0],newThresh,waveforms[ichl]->GetX()[0]+400,newThresh);
    line->SetLineColor(kRed);
    line->SetLineWidth(4);

    //    cans[ichl]->cd();
    can->cd(ichl+1);
    waveforms[ichl]->Draw();
    can->Update();
    line->Draw();
  }

  
  cans[0]->Print("smoothedWaveforms.pdf(");
  cans[1]->Print("smoothedWaveforms.pdf");
  cans[2]->Print("smoothedWaveforms.pdf");
  cans[3]->Print("smoothedWaveforms.pdf");
  cans[4]->Print("smoothedWaveforms.pdf");
  cans[5]->Print("smoothedWaveforms.pdf");
  cans[6]->Print("smoothedWaveforms.pdf");
  cans[7]->Print("smoothedWaveforms.pdf");
  cans[8]->Print("smoothedWaveforms.pdf");
  cans[9]->Print("smoothedWaveforms.pdf");
  cans[10]->Print("smoothedWaveforms.pdf");
  cans[11]->Print("smoothedWaveforms.pdf");
  cans[12]->Print("smoothedWaveforms.pdf");
  cans[13]->Print("smoothedWaveforms.pdf");
  cans[14]->Print("smoothedWaveforms.pdf");
  cans[15]->Print("smoothedWaveforms.pdf)");
  */

  //  can->Print("/home/jkruse/ARA/Summer2018Work/SimData/smoothedWaveforms.pdf");

  //  printAbsOfWaveforms(chan);
  printWaveforms(chan);
  

  cout << "Finished." << endl;

}

/*
  This function is designed to smooth the given waveform 
  by taking the average of consecutive sets of point. 
  It is currently set to average every 10 points (COUNTLIM = 10),
  but that could be changed depending on how smooth you want the
  waveform to look.
*/
TGraph* SmoothWaveform(TGraph* waveform) {
  
  int npnt = waveform->GetN();
  vector<double> xVec; // I use vectors because then I don't need to know what the size is before hand
  vector<double> yVec;
  double aveX = 0;
  double aveY = 0;
  int count = 1;
  int COUNTLIM = 10;
  
  for (int ipnt=0; ipnt<npnt; ipnt++) {

    // add consecutive x and y's until countlim
    aveX += waveform->GetX()[ipnt];
    aveY += fabs(waveform->GetY()[ipnt]);

    // get average and reset counters
    if (count == COUNTLIM) {
      xVec.push_back(aveX/COUNTLIM);
      yVec.push_back(aveY/COUNTLIM);
      aveX = 0;
      aveY = 0;
      count = 0;
    }

    count++;

  }  

  /* 
     Need to transfer the vector data to
     array form because TGraph only takes
     arrays as arguments
  */
  double xArr[xVec.size()];
  double yArr[yVec.size()];
  
  for (int i=0; i<xVec.size(); i++) {
    xArr[i] = xVec[i];
    yArr[i] = yVec[i];
    //    printf("Ave Y = %.3f \n", yArr[i]);
  }

  // make new graph with smoothed waveform    
  TGraph *SmoothedWaveform = new TGraph(xVec.size(),xArr,yArr);

  // send it back
  return SmoothedWaveform;
}

/*
  This function is designed to take the absolute value 
  of the given waveforms in the channel collection
*/
void printAbsOfWaveforms(ChannelCollection chan) {
  /*
  TCanvas *cans[16];

  cans[0] = new TCanvas("can0","can0",600,600);
  cans[1] = new TCanvas("can1","can1",600,600);
  cans[2] = new TCanvas("can2","can2",600,600);
  cans[3] = new TCanvas("can3","can3",600,600);  
  cans[4] = new TCanvas("can4","can4",600,600);
  cans[5] = new TCanvas("can5","can5",600,600);
  cans[6] = new TCanvas("can6","can6",600,600);
  cans[7] = new TCanvas("can7","can7",600,600);
  cans[8] = new TCanvas("can8","can8",600,600);
  cans[9] = new TCanvas("can9","can9",600,600);
  cans[10] = new TCanvas("can10","can10",600,600);
  cans[11] = new TCanvas("can11","can11",600,600);
  cans[12] = new TCanvas("can12","can12",600,600);
  cans[13] = new TCanvas("can13","can13",600,600);
  cans[14] = new TCanvas("can14","can14",600,600);
  cans[15] = new TCanvas("can15","can15",600,600);
  */
  TCanvas *can = new TCanvas("can","can",800,800);
  can->Divide(4,4);

  int nChl = chan.getNumChans();
  TGraph *waveforms[16];

  for (int ichl=0; ichl<nChl; ichl++) {
    waveforms[ichl] = chan.getChannel(ichl)->getWaveform();
    int npnt = waveforms[ichl]->GetN();
  cout << npnt << endl;

    double rms = chan.getChannel(ichl)->getGeometry()->getPosition(ichl).getVRMS();
    double thresh = 6*38;      /* !!!!!! Using rms=38 because that's what Andrew said AraSim noise rms is !!!!!*/
    double absY[npnt];
    double X[npnt];

    // get new x and y coordinates
    for (int ipnt=0; ipnt<npnt; ipnt++) {
      absY[ipnt] = fabs(waveforms[ichl]->GetY()[ipnt]);
      X[ipnt] = waveforms[ichl]->GetX()[ipnt];
    }

    // make new graph with abs value of waveform
    waveforms[ichl] = new TGraph(npnt,X,absY);
    // make threshold line
    TLine *line = new TLine(X[0],thresh,X[0]+400,thresh);
    line->SetLineColor(kRed);
    line->SetLineWidth(4);
    //    cans[ichl]->cd();

    // print all 16 waveforms on one canvas
    can->cd(ichl+1);
    waveforms[ichl]->Draw();
    can->Update();
    line->Draw();
  }
  /*
  cans[0]->Print("regularWaveforms.pdf(");
  cans[1]->Print("regularWaveforms.pdf");
  cans[2]->Print("regularWaveforms.pdf");
  cans[3]->Print("regularWaveforms.pdf");
  cans[4]->Print("regularWaveforms.pdf");
  cans[5]->Print("regularWaveforms.pdf");
  cans[6]->Print("regularWaveforms.pdf");
  cans[7]->Print("regularWaveforms.pdf");
  cans[8]->Print("regularWaveforms.pdf");
  cans[9]->Print("regularWaveforms.pdf");
  cans[10]->Print("regularWaveforms.pdf");
  cans[11]->Print("regularWaveforms.pdf");
  cans[12]->Print("regularWaveforms.pdf");
  cans[13]->Print("regularWaveforms.pdf");
  cans[14]->Print("regularWaveforms.pdf");
  cans[15]->Print("regularWaveforms.pdf)");
  */
  can->Print("/home/jkruse/ARA/Summer2018Work/SimData/regularWaveforms.pdf");
}

/* 
   This function is designed to print the waveforms
   of one event
*/
void printWaveforms(ChannelCollection chan) {

  TCanvas *cans[16];

  /*  cans[0] = new TCanvas("can0","can0",600,600);
  cans[1] = new TCanvas("can1","can1",600,600);
  cans[2] = new TCanvas("can2","can2",600,600);
  cans[3] = new TCanvas("can3","can3",600,600);  
  cans[4] = new TCanvas("can4","can4",600,600);
  cans[5] = new TCanvas("can5","can5",600,600);
  cans[6] = new TCanvas("can6","can6",600,600);
  cans[7] = new TCanvas("can7","can7",600,600);
  cans[8] = new TCanvas("can8","can8",600,600);
  cans[9] = new TCanvas("can9","can9",600,600);
  cans[10] = new TCanvas("can10","can10",600,600);
  cans[11] = new TCanvas("can11","can11",600,600);
  cans[12] = new TCanvas("can12","can12",600,600);
  cans[13] = new TCanvas("can13","can13",600,600);
  cans[14] = new TCanvas("can14","can14",600,600);
  cans[15] = new TCanvas("can15","can15",600,600);*/
  
  TCanvas *can = new TCanvas("can","can",800,800);
  can->Divide(4,4);

  int nChl = chan.getNumChans();
  TGraph *waveforms[16];

  for (int ichl=0; ichl<nChl; ichl++) {
    waveforms[ichl] = chan.getChannel(ichl)->getWaveform();
    can->cd(ichl+1);
    waveforms[ichl]->Draw();
  }

  /*  cans[0]->Print("regularWaveforms.pdf(");
  cans[1]->Print("regularWaveforms.pdf");
  cans[2]->Print("regularWaveforms.pdf");
  cans[3]->Print("regularWaveforms.pdf");
  cans[4]->Print("regularWaveforms.pdf");
  cans[5]->Print("regularWaveforms.pdf");
  cans[6]->Print("regularWaveforms.pdf");
  cans[7]->Print("regularWaveforms.pdf");
  cans[8]->Print("regularWaveforms.pdf");
  cans[9]->Print("regularWaveforms.pdf");
  cans[10]->Print("regularWaveforms.pdf");
  cans[11]->Print("regularWaveforms.pdf");
  cans[12]->Print("regularWaveforms.pdf");
  cans[13]->Print("regularWaveforms.pdf");
  cans[14]->Print("regularWaveforms.pdf");
  cans[15]->Print("regularWaveforms.pdf)");*/
  
  can->Print("/home/jkruse/ARA/Summer2018Work/SimData/regularWaveforms.pdf");

}

/*
  This function is designed to calculate the rms
  of a given waveform after it has been smoothed
*/
double getNewRMS(TGraph *waveform) {

  int npnt = waveform->GetN();
  vector<double> xVec;
  vector<double> yVec;
  double aveX = 0;
  double aveY = 0;
  int count = 1;
  int COUNTLIM = 15;

  // smoothing waveform
  for (int ipnt=0; ipnt<npnt; ipnt++) {
    
    aveX += waveform->GetX()[ipnt];
    aveY += waveform->GetY()[ipnt];

    if (count == COUNTLIM) {
      xVec.push_back(aveX/COUNTLIM);
      yVec.push_back(aveY/COUNTLIM);
      aveX = 0;
      aveY = 0;
      count = 0;
    }

    count++;

  }

  // find average of y and y^2
  double newAveY = 0;
  double aveYY = 0;

  for (int i=0; i<xVec.size(); i++) {
    newAveY += yVec[i];
    aveYY += yVec[i]*yVec[i];
  }

  newAveY /= yVec.size();
  aveYY /= yVec.size();

  // calculate rms
  double rms = sqrt(aveYY - newAveY*newAveY);

  return rms;

}
