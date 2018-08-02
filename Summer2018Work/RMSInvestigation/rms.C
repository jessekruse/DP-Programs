/*
  Author: Jesse Kruse
  Purpose: This program is designed to investigate the effect of 
  my smoothing function on the rms. If we know what the rms 
  will be after smoothing, then we know what the new threshold should
  be so that it is proportional to the new waveform
*/
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <unistd.h>
#include <assert.h>
#include <cstdlib>
#include <math.h>
#include <vector>
#include "time.h"

using namespace std;

double getRMS(vector<double> vec);
vector<double> smoothWaveform(vector<double> vec);

int main() {
  
  // create an array of random numbers
  const int SIZE = 1000;
  vector<double> randVec;
  double rms = 0;

  // input random numbers into random number vector
  for (int i=0; i<SIZE; i++) {
    randVec.push_back(10*(double)rand()/(double)RAND_MAX);
  }

  // smooth random number vector
  randVec = smoothWaveform(randVec);

  // get rms
  rms = getRMS(randVec);

  printf("RMS = %f\n",rms);
  
}

/*
  This function is designed to smooth the vector of 
  random numbers like my smooth waveform program in 
  /home/jkruse/ARA/Summer2018Work/RealData/
*/
vector<double> smoothWaveform(vector<double> vec) {
  
  int size = vec.size();
  vector<double> newX;
  double aveX = 0;
  int count = 1;
  int COUNTLIM = 15;

  for (int i=0; i<size; i++) {
    
    aveX += vec[i];

    if (count == COUNTLIM) {
      newX.push_back(aveX/COUNTLIM);
      aveX = 0;
      count = 0;
    }

    count++;
  }

  return newX;

}

double getRMS(vector<double> vec) {

  int size = vec.size();
  double aveX = 0;
  double aveXX = 0;
  
  for (int i=0; i<size; i++) {
    aveX += vec[i];
    aveXX += vec[i]*vec[i];
  }

  aveX = aveX/size;
  aveXX = aveXX/size;
  
  cout << size << endl;
  cout << aveX << endl;
  cout << aveXX << endl;

  double rms = sqrt(aveXX-aveX*aveX);

  return rms;

}


