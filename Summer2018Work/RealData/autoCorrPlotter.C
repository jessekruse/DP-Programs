/*
  Author: Jesse Kruse
  Purpose: This program is designed to autocorrelate the 
  waveforms for an event
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

#include "FileReader.h"
#include "StationGeometry.h"
#include "Channel.h"
#include "Pos.h"
#include "OpticalIce.h"
#include "ChannelCollection.h"
#include "FFTtools.h"

using namespace std;

TGraph* AutoCorrWaveform(TGraph *waveform);
TGraph* SmoothWaveform(TGraph* waveform);
TGraph* NormalizeWaveform(TGraph* waveform);
TGraph* getFFTCorrGraph(TGraph *waveform);

int main(int argc, char **argv) {

  if (argc<2) {printf("Need input file names\n"); return 1;}

  cout << "\nLoading: " << argv[1] << endl;
  cout << "Event: " << argv[2] << "\n\n";

  // Declare some variables
  TGraph *waveforms[16];
  int EVT = atoi(argv[2]);
  int numEvts;

  // Load the file containing the filtered events
  FileReader reader(argv[1]);


  // Get the station geometry from the file
  StationGeometry *geom=reader.getStationGeometry();
  Channel::setGeometry(geom);
  CurvedRay::setOpticalIce(new OpticalIce);
  Pos::setOriginStatic(geom->getOriginPosition());
  OpticalIce *ice = new OpticalIce();

  // Set max number of events for the reader
  numEvts = reader.getTotalNumEvents()-1;
  reader.setMaxNumEvents(numEvts);
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

  // Load event 
  reader.loadEvent(EVT);

  // gt channel collection
  ChannelCollection chan = reader.getChannelCollection();
  int nChl = chan.getNumChans();

  // do stuff with waveforms
  for (int ichl=0; ichl<nChl; ichl++) {
    waveforms[ichl] = chan.getChannel(ichl)->getWaveform();
    waveforms[ichl] = NormalizeWaveform(SmoothWaveform(AutoCorrWaveform(waveforms[ichl])));
    //    waveforms[ichl] = getFFTCorrGraph(waveforms[ichl]);
    //    cans[ichl->cd();
    can->cd(ichl+1);
    waveforms[ichl]->Draw();
  }
  /*
  cans[0]->Print("autoCorrWaveforms.pdf(");
  cans[1]->Print("autoCorrWaveforms.pdf");
  cans[2]->Print("autoCorrWaveforms.pdf");
  cans[3]->Print("autoCorrWaveforms.pdf");
  cans[4]->Print("autoCorrWaveforms.pdf");
  cans[5]->Print("autoCorrWaveforms.pdf");
  cans[6]->Print("autoCorrWaveforms.pdf");
  cans[7]->Print("autoCorrWaveforms.pdf");
  cans[8]->Print("autoCorrWaveforms.pdf");
  cans[9]->Print("autoCorrWaveforms.pdf");
  cans[10]->Print("autoCorrWaveforms.pdf");
  cans[11]->Print("autoCorrWaveforms.pdf");
  cans[12]->Print("autoCorrWaveforms.pdf");
  cans[13]->Print("autoCorrWaveforms.pdf");
  cans[14]->Print("autoCorrWaveforms.pdf");
  cans[15]->Print("autoCorrWaveforms.pdf)");
  */
  can->Print("autoCorrWaveforms.pdf");

  cout << "Finished." << endl;

}

/*
  This function autocorrelates waveforms via the definition of 
  autocorrelation. If you are unsure of what autocorrelation is,
  just google it. There is a pretty good explanation on wikipedia
  along with the summation formula I used for this function.
*/
TGraph* AutoCorrWaveform(TGraph* waveform) {

  
  int npnt = waveform->GetN(); //get the number of points in the waveform
  double autoCorrY[npnt]; // create arrays for the new x and y coordinates
  double autoCorrX[npnt];
  double sum;

  for (int i=0; i<npnt; i++) {
    sum = 0;
    
    for (int j=0; j<(npnt-i); j++) {
      sum += fabs(waveform->GetY()[j]*waveform->GetY()[j+i]); 
    }
    autoCorrX[i] = waveform->GetX()[i]-waveform->GetX()[0];
    autoCorrY[i] = sum;

  }

  TGraph *AutoCorrWaveform = new TGraph(npnt,autoCorrX,autoCorrY); // create new graph with new x and y points

  return AutoCorrWaveform;

}

/*
  This function is designed to smooth the given waveform 
  by taking the average of consecutive sets of point. 
  It is currently set to average every 3 points (COUNTLIM = 3),
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
  int COUNTLIM = 3;

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
  This function is designed to normalize the given
  waveform to the maximum Y value.
*/

TGraph* NormalizeWaveform(TGraph *waveform) {

  int npnt = waveform->GetN();
  double maxY = waveform->GetY()[0];
  double normY[npnt];

  // Find max y
  for (int ipnt=0; ipnt<npnt; ipnt++) {
    if (waveform->GetY()[ipnt] > maxY) 
      maxY = waveform->GetY()[ipnt];
  }

  // normalize
  for (int ipnt=0; ipnt<npnt; ipnt++) {
    normY[ipnt] = waveform->GetY()[ipnt]/maxY;
  }

  // create new graph with norm waveform
  TGraph *normWaveform = new TGraph(npnt,waveform->GetX(),normY);

  // send it back
  return normWaveform;

}


/*
  This function is designed to autocorrelate the given waveform
  but with the fast fourier transform function getCorrelationGraph 
  and the waveform as both arguments.
*/
TGraph* getFFTCorrGraph(TGraph *waveform) {

  // autocorrelate
  TGraph *autogra = FFTtools::getCorrelationGraph(waveform,waveform);

  // This is to get just the right side because it is symmetic with
  // respect to y=0. 
  int npnt = autogra->GetN();
  vector<double> xVec;
  vector<double> yVec;

  for (int ipnt=0; ipnt<npnt; ipnt++) {
    if (autogra->GetX()[ipnt] >= 0) {
      xVec.push_back(autogra->GetX()[ipnt]);
      yVec.push_back(autogra->GetY()[ipnt]);
    }
  }


  // turn the vectors into arrays
  double xArr[xVec.size()];
  double yArr[yVec.size()];
  
  for (int i=0; i<xVec.size(); i++) {
    xArr[i] = xVec[i];
    yArr[i] = yVec[i];
  }

  // make a new graph with the right positve x auto
  // correlated graph
  autogra = new TGraph(xVec.size(),xArr,yArr);

  // send it back
  return autogra;

}
