
/*
  Author: Jesse Kruse
  Purpose: This program is designed to plot the waveforms for a 
  single event or a collection of events.

  This program crashes at the end with a segmentation fault, but
  it still plots the waveforms, soooooooo... it works.
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

#define str(x) #x
#define glue(a,b) a ## b

using namespace std;

void printAllEventWaveforms(FileReader reader, char* filename);
void printEventWaveforms(ChannelCollection chan, int EVT);

int main(int argc, char **argv) {

  if (argc<2) {printf("Need input file names\n"); return 1;}

  cout << "\nLoading: " << argv[1] << endl;
  cout << "Second Argument: " <<argv[2] << "\n\n";
  

  // Declare some variables
  int EVT = 0;
  int numEvts;
  char *filename = argv[2];
  ifstream inFile(filename);
  

  // Check if second argument is a text file
  // If not, it's an event.
  if (!inFile) {
    EVT = atoi(argv[2]);
    printf("Not text file found. Using event %d\n",EVT);
  }

  // Load the file containing the filtered events
  FileReader reader(argv[1]);
  

  // Set max number of events for the reader
  numEvts = reader.getTotalNumEvents()-1;
  reader.setMaxNumEvents(numEvts);

  // Get the station geometry from the file
  StationGeometry *geom=reader.getStationGeometry();
  Channel::setGeometry(geom);
  CurvedRay::setOpticalIce(new OpticalIce);
  Pos::setOriginStatic(geom->getOriginPosition());
  OpticalIce *ice = new OpticalIce();

  // If we have a text file, call printAllEventWaveforms
  if (inFile) {
    cout << "Loading: " << filename << endl;
    printAllEventWaveforms(reader,filename);
  }
  else {

    // Scan software triggers for vrms
    reader.scanSoftwareTriggersForVRMS();
    cout << "Done Scanning Software Triggers\n\n";

    reader.loadEvent(EVT);
    ChannelCollection chan = reader.getChannelCollection();
    printEventWaveforms(chan,EVT);
  }

  cout << "\n\nHere?\n\n";


}


/*
  This function is designed to print the waveforms for a collection
  of events. These events are specified by a text file containing 
  theevent numbers. 
*/
void printAllEventWaveforms(FileReader reader, char* filename) {

  //  ifstream inFile(filename);  
  FILE *inFile = fopen(filename,"r");
  char buff[255];
  int SIZE;

  // Figure out how many events there are in the file
  while(fscanf(inFile,"%s",buff) != EOF)
    SIZE++;

  // Gotta close and reopen the file to get the right
  // values.
  fclose(inFile);
  inFile = fopen(filename,"r");

  int EVT;

  // Create an array of canvases This saves sooooo much space.


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

    if (reader.loadEvent(EVT))
      cout << "Event " << EVT << endl;

    // get channel collection for that event
    ChannelCollection chan = reader.getChannelCollection();
    int nChl = chan.getNumChans();

    // print waveforms on canvases
    TGraph *waveforms[16];
    for (int ichl=0; ichl<nChl; ichl++){
      waveforms[ichl] = chan.getChannel(ichl)->getWaveform();
      canArr[i]->cd(ichl+1);
      waveforms[ichl]->DrawClone("al");
    }
      //    }
  }      

  
  // Save waveforms to a pdf
  canArr[0]->Print("/home/jkruse/ARA/Summer2018Work/RealData/waveforms.pdf(");
  for (int i=1; i<SIZE-1; i++) {
    canArr[i]->Print("/home/jkruse/ARA/Summer2018Work/RealData/waveforms.pdf");
  }
  canArr[SIZE-1]->Print("/home/jkruse/ARA/Summer2018Work/RealData/waveforms.pdf)");

  fclose(inFile);

  
}

/*
  This function is designed to print the waveforms for a single
  event
*/
void printEventWaveforms(ChannelCollection chan, int EVT) {

  TCanvas *can = new TCanvas("can","can",800,800);
  can->Divide(4,4);

  int nChl = chan.getNumChans();
  
  TGraph *waveforms[16];

  for (int ichl=0; ichl<nChl; ichl++) {
    double rms = chan.getChannel(ichl)->getGeometry()->getPosition(ichl).getVRMS();
    double thresh = 6*rms;
    printf("Threshold %d: %f\n",ichl,thresh);
    waveforms[ichl] = chan.getChannel(ichl)->getWaveform();
    double X = waveforms[ichl]->GetX()[0];
    TLine *line = new TLine(X,thresh,X+500,thresh);
    line->SetLineColor(kRed);
    line->SetLineWidth(4);

    can->cd(ichl+1);
    waveforms[ichl]->Draw();
    can->Update();
    line->Draw();
  }

  can->Print("/home/jkruse/ARA/Summer2018Work/RealData/eventWaveforms.pdf");

}
