#include <iostream>
#include <string>
using namespace std;
#include <cmath>
#include <vector>

void viewSpectra(TString file, Int_t windowStart = 0, Int_t windowEnd = 0)
{
  //Create a spectra and store the integral output to a new file
  //TString file: data that you want to view spectra of
  //Int_t windowStart: if ==windowEnd==0, use accumulators. Otherwise, integrate waveform between these two values
  
  //Load file
  TFile *F = new TFile(file);
  TTree *T = (TTree *)F->Get("waveformTree");

  //Variables
  Double_t baseline = 0;
  Double_t polarity = 0;
  Double_t temp = 0;
  Double_t integral = 0;
  Double_t accumulator = 0;
  Int_t accumulatorStart = 0;
  Int_t accumulatorEnd = 0;
  Int_t counter = 0;
  Int_t windowLength = 0;
  vector<UShort_t> *waveform = 0;
  vector<UShort_t> readWaveform;
  Bool_t usingAccumulator = 0;

  const Double_t histLower = 0;
  const Double_t histUpper = 100000;
  const Double_t histLength = (histUpper-histLower)/100.0; //Spectra is drawn as a histogram. Set range of that histogram

  //Now let's create a new file and new tree
  TString newFile = file;
  TString extension = ".root";
  newFile = newFile.Remove(newFile.Length()-extension.Length(), extension.Length());
  newFile += "Spectra.root";
  TFile *myFile = new TFile(newFile, "RECREATE"); //Creates the file. If it already exists, overwrites it
  TTree *tree = new TTree("integralTree", "tree");
  //Create branches
  tree->Branch("integral", &integral);

  //Declare what branches to read
  T->SetBranchAddress("waveform", &waveform);
  T->SetBranchAddress("baseline", &baseline);
  T->SetBranchAddress("polarity", &polarity);
  T->SetBranchAddress("accumulator1", &accumulator);
  T->SetBranchAddress("accumulator1Start", &accumulatorStart);
  T->SetBranchAddress("accumulator1End", &accumulatorEnd);
  T->GetEntry(0);

  if (windowStart == 0 && windowEnd == 0)
  {
    //If the window isn't set, use accumulators. What the accumulators do is take the raw integral of a waveform within a certain window
    //The way I have it set up, accumulator0 should be baseline, accumulator1 should be signal, and accumulator2 should be post-signal
    //We'll have to do some baseline subtraction
    windowStart = accumulatorStart;
    windowEnd = accumulatorEnd;
    usingAccumulator = 1;
  }

  windowLength = windowEnd-windowStart+1; //Since we use inclusive counting

  TH1D* hist = new TH1D("", "", histLength, histLower, histUpper); //This is the spectra histogram

  const Int_t numberEntries = T->GetEntries();
  
  for (Int_t eventNumber = 0; eventNumber<numberEntries; eventNumber++)
  {
    temp = 0;
    integral = 0;
    if (eventNumber % 1000 == 0)
    {
      printf("Currently on event %d of %d\n", eventNumber, numberEntries);
    }
    T->GetEntry(eventNumber);
    readWaveform = *waveform;
    integral = (accumulator - baseline*windowLength*1.0)*polarity*1.0;
    //Set the integral using accumulator
    
    if (!usingAccumulator)
    {
      integral = 0;
      for (Int_t i = 0; i < readWaveform.size(); i++)
      {
        temp = (readWaveform[i]-baseline*1.0)*polarity*1.0;
	if (i >= windowStart && i <= windowEnd)
	{
	  //Integrate the waveform between the designated window
          integral += temp;
	}
      }
    }
    hist->Fill(integral); //fill spectra
    tree->Fill();
  }

  //Draw
  TCanvas *c1 = new TCanvas("c1", "", 1000, 600);
  gStyle->SetOptStat(0);
  hist->GetXaxis()->SetTitle("Integral");
  hist->GetYaxis()->SetTitle("Counts");
  hist->Draw("hist");

  hist->Write("hist");
  myFile->Write();
}
