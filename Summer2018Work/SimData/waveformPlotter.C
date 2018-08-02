
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

#include "SimFileReader.h"
#include "StationGeometry.h"
#include "Channel.h"
#include "Pos.h"
#include "OpticalIce.h"
#include "ChannelCollection.h"

#define str(x) #x
#define glue(a,b) a ## b

using namespace std;

void printAllEventWaveforms(SimFileReader reader, int SIZE, char* filename);
void printEventWaveforms(ChannelCollection chan, int EVT);

int main(int argc, char **argv) {

  if (argc<2) {printf("Need input file names\n"); return 1;}

  cout << "\nLoading: " << argv[1] << endl;
  

  // Declare some variables
  int EVT = 0;
  int numEvts;
  int SIZE = 129;
  char* filename = argv[2];
  ifstream inFile(filename);

  // Check if second argument is a text file
  // If not, it's an event.
  if (!inFile) {
    EVT = atoi(argv[2]);
    cout << "Loading Event: " << EVT << endl;
  }


  // Load the file containing the filtered events
  SimFileReader reader(argv[1]);


  // Set max number of events for the reader
  //  numEvts = 99999;
  //  reader.setMaxNumEvents(numEvts);  


  // Get the station geometry from the file
  StationGeometry *geom=reader.getStationGeometry();
  Channel::setGeometry(geom);
  CurvedRay::setOpticalIce(new OpticalIce);
  Pos::setOriginStatic(geom->getOriginPosition());
  OpticalIce *ice = new OpticalIce();


  //  TCanvas *can = new TCanvas("can","can",800,800);
  //  can->Divide(4,4);

  // If we have a text file, call printAllEventWaveforms
  if (inFile) {
    cout << "Loading: " << filename << endl;
    printAllEventWaveforms(reader,SIZE,filename);
  }
  else {
    reader.loadEvent(EVT);
    ChannelCollection chan = reader.getChannelCollection();
    printEventWaveforms(chan,EVT);
  }
}


/*
  This function is designed to print the waveforms for a 
  collection of events. These events are specified by a text 
  file containing the event numbers. 
*/
void printAllEventWaveforms(SimFileReader reader, int SIZE, char* filename) {

  FILE *inFile = fopen(filename,"r");
  char buff[255];

  int EVT;

  // Create an array of canvases This saves sooooo much space.
  TCanvas *canArr[SIZE];
  for (int n=0; n<SIZE; n++) {
    // naming and initializing each canvas
    string num = Form("%i",n);
    TString name = "can" + num;
    canArr[n] = new TCanvas(name,name,800,800);
    canArr[n]->Divide(4,4);
  }


  // Load events from the output of dpFinder
  for (int i=0; i<SIZE; i++) {
    // get line from text file
    fscanf(inFile,"%s",buff);
    EVT = atoi(buff);// convert to integer

    cout << EVT << endl;
    cout << reader.loadEvent(EVT) << endl;
    //    if (reader.loadEvent(EVT))
    //      cout << "Event " << EVT << endl;
      //      printf("Event %d\n",EVT);
    cout << "Here?\n\n";

    // get channel collection for that event
      ChannelCollection chan = reader.getChannelCollection();
      int nChl = chan.getNumChans();

    // print waveforms on canvases
      TGraph *waveforms[16];
      for (int ichl=0; ichl<nChl; ichl++){
	waveforms[ichl] = chan.getChannel(ichl)->getWaveform();
	canArr[i]->cd(ichl+1);
	waveforms[ichl]->DrawClone("al");
	//	waveforms[ichl]->Draw();
      }
      //    }
  }      

  
  // Save waveforms to a pdf
  canArr[0]->Print("/home/jkruse/ARA/Summer2018Work/SimData/waveforms.pdf(");
  for (int i=1; i<SIZE-1; i++) {
    canArr[i]->Print("/home/jkruse/ARA/Summer2018Work/SimData/waveforms.pdf");
  }
  canArr[SIZE-1]->Print("/home/jkruse/ARA/Summer2018Work/SimData/waveforms.pdf)");
  
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
    cout << ichl << endl;
    waveforms[ichl] = chan.getChannel(ichl)->getWaveform();
    can->cd(ichl+1);
    waveforms[ichl]->Draw();
  }

  can->Print("/home/jkruse/ARA/Summer2018Work/SimData/eventWaveforms.pdf");

}
