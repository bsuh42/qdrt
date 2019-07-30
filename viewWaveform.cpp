#include <iostream>
#include <string>
using namespace std;
#include <cmath>
#include <vector>

void viewWaveform(TString file, const Int_t waveformNumber = 0, const Int_t baselineLength = 0)
{
  //View waveform
  //TString file: file containing waveform
  //Int_t waveformNumber: which waveform to view
  //Int_t baselineLength: set length of baseline-finding algorithm. Larger value gives better statistics, but risk of including signal. If =0, use baseline value from TTree

  //Load file
  TFile *F = new TFile(file);
  TTree *T = (TTree *)F->Get("waveformTree");

  //Variables
  Double_t baseline = 0;
  Double_t polarity = 0;
  Double_t temp = 0;
  vector<UShort_t> *waveform = 0; //Create a vector object to store the waveform from the tree. 
  
  //Read data from the tree
  T->SetBranchAddress("waveform", &waveform);
  T->SetBranchAddress("baseline", &baseline);
  T->SetBranchAddress("polarity", &polarity); //Specifies which branches we want to read and where to store those variables
  T->GetEntry(waveformNumber); //Reads the branch and gets the data corresponding to the specified waveform
  vector<UShort_t> readWaveform = *waveform; //We create a new vector object to store the tree waveform. I'm not entirely sure why this is needed, but I've run into problems when I don't have it

  const Int_t waveformStart = 0;
  const Int_t waveformEnd = readWaveform.size();
  const Int_t waveformLength = waveformEnd-waveformStart;
  TH1D* hist = new TH1D("", "", waveformLength, waveformStart, waveformEnd);
  //Create a histogram to store the waveform. We then view that histogram
  
  if (baselineLength != 0)
  {
    baseline = 0;
    //If we don't use the baseline value stored in the tree, calculate baseline by averaging the first x samples.
    for (Int_t i = 0; i < baselineLength; i++)
    {
      temp = readWaveform[i];
      baseline += temp;
    }
    baseline /= baselineLength;
  }

  for (Int_t i = 0; i < readWaveform.size(); i++)
  {
    //Now we loop through the waveform and do baseline subtraction and positivizing
    temp = (readWaveform[i]-baseline)*polarity;
    hist->Fill(i, temp);
  }

  //Now we draw the histogram
  TCanvas *c1 = new TCanvas("c1", "", 1000, 600); //We need to create a canvas to draw on
  gStyle->SetOptStat(0); //This disables some stat boxes
  hist->GetXaxis()->SetTitle("Sample");
  hist->GetYaxis()->SetTitle("Amplitude (ADC)");
  hist->Draw("hist");
  c1->Update();

  //Let's also print out the baseline value
  printf("Baseline=%f\n", baseline);
}
