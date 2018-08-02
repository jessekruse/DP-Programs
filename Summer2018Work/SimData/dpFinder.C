
/*
  Author: Jesse Kruse
  Purpose: This program is designed to identify events that have
  two pulses in their waveforms for AraSim Files
  Date: 05/29/2018
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
#include "SimFileReader.h"
#include "StationGeometry.h"
#include "Channel.h"
#include "Pos.h"
#include "OpticalIce.h"
#include "ChannelCollection.h"

int checkIfDP(ChannelCollection chan, double TIMEDELAY);

int main(int argc, char **argv) {

  if (argc<2) {printf("Need input file names\n"); return 1;}

  cout << endl;

  cout << "\nLoading: " << argv[1] << "\n\n";

  int numEvts;
  double TIMEDELAY = 20;
  //  double TIMEDELAY = 45; //? Maybe this will produce better results ?
  //  double TIMEDELAY = 75;

  // Load the file containing the events
  SimFileReader reader(argv[1]);


  // Get the station geometry from the file
  StationGeometry *geom=reader.getStationGeometry();
  Channel::setGeometry(geom);
  Pos::setOriginStatic(geom->getOriginPosition());
  OpticalIce *ice = new OpticalIce();
  CurvedRay::setOpticalIce(ice);

  // Set max number of events for the reader
  numEvts = reader.getTotalNumEvents()-1;
  reader.setMaxNumEvents(numEvts);  

  // loop over events
  for (int ievt=0; ievt<1e6; ievt++) {
    if (!reader.loadNextEvent()) break;

    // get channel collection
    ChannelCollection chan = reader.getChannelCollection();
    int ndp = checkIfDP(chan,TIMEDELAY);

    if (ndp > 0) {
      Printf("%d",ievt);
    }

  }

}



/* 
   This function loops through the channels for an event and 
   counts the number of double pulses it detects (ranging from 0
   to 16). The function returns this number. However if the 
   average number of hits per channel is less than some minimum
   or greater than some maximum, the function returns a -1. 

   Counting the average number of hits or excursions beyond the
   threshold for the event gives a sense of the width or duration
   of the pulse. If it's too large, it might not be a double or 
   single pulse. If it's too small, it might not even be a pulse.   
*/
int checkIfDP(ChannelCollection chan, double TIMEDELAY) {

  TGraph *waveforms[16];

  int nChl = chan.getNumChans();
  int ndp = 0;
  int numHits = 0;
  double aveNumHits = 0;
  
  // Loop over channels for this event
  for (int ichl=0; ichl<nChl; ichl++) {
    //    double rms = chan.getChannel(ichl)->getGeometry()->getPosition(ichl).getVRMS();
    double rms = 38; 
    /* !!! Setting RMS to 38 because Andrew said 
       that's what the Sim noise rms is usually !!! */

    //We don't know if there are hits yet
    bool hit1 = false;
    bool hit2 = false;

    // Just need to initialize this variable to something it can't be.
    // This helps for debugging.   
    double hitTime1 = 999;

    waveforms[ichl] = chan.getChannel(ichl)->getWaveform();
    int npnt = waveforms[ichl]->GetN();
    
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
  }      

  // get average number of hits or excursions above threshold
  // for this event. (average of all 16 channels)
  aveNumHits = numHits/nChl;
  printf("AveNumHits: %f\n",aveNumHits);

  /*  if (aveNumHits > 10 && aveNumHits < 25) 
    return ndp;
  else
    return -1;
  */

  /*
  if (aveNumHits > 15)
    return ndp;
  else
    return -1;
  */

  return ndp;

}
