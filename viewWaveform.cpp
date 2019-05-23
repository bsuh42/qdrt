#include <iostream>
#include <string>
using namespace std;
#include <cmath>
#include <vector>
//I always import these modules at the start

void viewWaveform(TString file, const Int_t waveformNumber)
{
  //File to view waveforms created by ADAQ.
  //TString file: path to .root file with waveforms to view
  //const Int_t waveformNumber: which waveform you wish to view.
  
  //Load file and get tree
  TFile *F = new TFile(file);
  TTree *T = (TTree *)F->Get("waveformTree");

  //Variables
  const Int_t waveformStart = 0; //This will almost be set to 0. Can change if you want to view a segment of the waveform
  const Int_t waveformEnd = 500; //Similarly, if you wish to view just a segment of the waveform
  const Int_t waveformBin = waveformEnd-waveformStart; //Length of the segment you are trying to view
  const Int_t baselineLength = 50; //Adjust this to determine how many bins you want to look at when calculating baseline. Note that this value is stored in the tree, but this is more for looking at different baseline lengths.

  Double_t baseline = 0; //We could grab this from the tree, but I'll show how to calculate the baseline on your own
  Double_t integral = 0; //Also include some functionality to calculate the integral of a given waveform
  Double_t polarity = 0; //Grab this value from the tree. Rather than having to remember if a pulse is negative or positive going, we instead store that value when we create the .root file
  vector<UShort_t> *waveform = 0; //Initialize a vector object to store the waveform
  Double_t foo = 0;

  //Set branches and grab values from the tree
  T->SetBranchAddress("waveform", &waveform);
  T->SetBranchAddress("polarity", &polarity);
  T->GetEntry(waveformNumber);
  vector<UShort_t> readWaveform = *waveform;

  //Create a histogram object so we can display the waveform
  TH1D* waveformHist = new TH1D("waveformHist", "waveformHist", waveformBin, waveformStart, waveformEnd);

  //Loop through waveform, perform baseline subtraction, and store to the histogram so we can view it
  for (Int_t i = 0; i < baselineLength; i++)
  {
    //We assume the baseline is at the start. To calculate the baseline, we take the average of the values in baselineLength. We don't expect any signal in this region, so they should all be roughly the same value. 
    baseline += (readWaveform[i]*1.0/baselineLength*1.0);
  }

  for (Int_t i = 0; i < readWaveform.size(); i++)
  {
    foo = (readWaveform[i]-baseline)*polarity; //Baseline subtraction, and if necessary, flip the waveform so it is always positive going
    waveformHist->Fill(i, foo);
    integral += foo; //Take the integral by adding up all the values
  }
  printf("Integral = %f\n", integral);

  //Graphing
  TCanvas *c1 = new TCanvas("c1", "", 1000, 600);
  gStyle->SetOptStat(0);
  waveformHist->Draw("HIST"); //HIST option necessary for ROOT 6. Not necessary for ROOT 5?
  c1->Update();
}
