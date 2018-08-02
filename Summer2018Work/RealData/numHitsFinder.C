
/*
  Author: Jesse Kruse
  Purpose: This program is designed to count the number of hits 
  within a certain time window after the first hit for each
  waveform
  Date: 05/30/2018
*/

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <assert.h>
#include "time.h"

#include "TFile.h"
#include "TTree.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TH1.h"

#include "FileReader.h"
#include "StationGeometry.h"
#include "Channel.h"
#include "Pos.h"
#include "OpticalIce.h"
#include "ChannelCollection.h"

vector<int> countNumHits(ChannelCollection chan, double TIMEDELAY);

int main(int argc, char **argv) {

  if (argc<3) {printf("Need input file names and event number\n"); return 1;}

  cout << "\nLoading: " << argv[1] << endl;
  cout << "Counting num hits for event: " << argv[2] << "\n\n";

  int numEvts;
  double TIMEDELAY = 75;
  int EVT = atoi(argv[2]);

  // Load the file containing the events
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

  // Load event, count num of hits per channel, and display
  reader.loadEvent(EVT);
  ChannelCollection chan = reader.getChannelCollection();
  vector<int> numHitsPerChnl = countNumHits(chan, TIMEDELAY);
  double totalNumHits = 0.0;

  for (int ichl=0; ichl<numHitsPerChnl.size(); ichl++) {
    printf("Num Hits for Chnl %d: %d\n",ichl,numHitsPerChnl[ichl]);
    totalNumHits += numHitsPerChnl[ichl];
  }

  printf("Average Num Hits for Event %d: %f\n",EVT,totalNumHits/numHitsPerChnl.size());
  
}

vector<int> countNumHits(ChannelCollection chan, double TIMEDELAY) {

  TGraph *waveforms[16];
  vector<int> numHitsPerChnl;

  int nChl = chan.getNumChans();
  int ndp = 0;
  
  // Loop over channels for this event
  for (int ichl=0; ichl<nChl; ichl++) {
    double rms = chan.getChannel(ichl)->getGeometry()->getPosition(ichl).getVRMS();
    bool hit1 = false;
    bool hit2 = false;
    double hitTime1 = 999;
    waveforms[ichl] = chan.getChannel(ichl)->getWaveform();
    int npnt = waveforms[ichl]->GetN();
    int numHits = 0;
    
    // Loop over points in waveform for this channel
    for (int ipnt=0; ipnt<npnt && !hit2; ipnt++) {
      double val = fabs(waveforms[ichl]->GetY()[ipnt]);
      double time = waveforms[ichl]->GetX()[ipnt];
      
      /* If the voltage at this point is greater than rms*6, 
	 then the first-hit boolean is set to true and the 
	 time at which this happens is recorded
      */
      if (val > (rms*6) && !hit1) {
	  hit1 = true;
	  hitTime1 = time;
	  numHits++;
      }


      /* This section counts the number of points that
	 exceed the threshold within the TIMEDELAY
      */
      if (hit1 && time < hitTime1+TIMEDELAY) {
	if (val > (rms*6)) numHits++;
      }


      /* Now that we have first hit-time, we can check if 
	 voltages some time after that time also exceed the 
	 threshold. If they do, then we say that this is a 
	 double pulse event
      */
      if ( (time > hitTime1+TIMEDELAY) && (val > rms*6) ) {
	hit2 = true;
	ndp++;
      }
    }
    numHitsPerChnl.push_back(numHits);
  }  

  return numHitsPerChnl;;

}
