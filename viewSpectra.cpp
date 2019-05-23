#include <iostream>
#include <string>
using namespace std;
#include <cmath>
#include <vector>

void viewSpectraWaveform(TString file)
{
  //File to create a spectra assuming it isn't stored in tree or if you have a different region of interest
  //TString file: file where waveforms are stored
  
  //Load file and tree
  TFile *F = new TFile(file);
  TTree *T = (TTree *)F->Get("waveformTree");

  //Variables
  const Int_t baselineLength = 50; //We assume the baseline is at the start of each waveform
  const Int_t roiStart = 51; //We assume the region of interest comes after the baseline. If it doesn't, this particular script runs into some issues
  const Int_t roiEnd = 500;
  const Int_t spectrumBins = 1000; //Number of bins in our spectrum histogram. I tend to like making this number larger then shrinking it as needed
  const Int_t spectrumStart = 0;
  const Int_t spectrumEnd = 1e5; //Region of interest for spectrum

  Double_t baseline = 0; //Calculated baseline
  Double_t integral = 0; //Integral of each waveform. Related to energy
  Double_t polarity = 0; //Grab this value from tree. Pulse could be negative or positive going, we store this value in the tree
  Double_t foo = 0;
  vector<UShort_t> *waveform = 0;
  vector<UShort_t> readWaveform;

  //Create a histogram to store the spectrum
  TH1D* spectrumHist = new TH1D("spectrumHist", "spectrumHist", spectrumBins, spectrumStart, spectrumEnd);

  //Loop through tree and look at each waveform
  const Int_t numberEntries = T->GetEntries();
  T->SetBranchAddress("waveform", &waveform);
  T->SetBranchAddress("polarity", &polarity);

  for (Int_t eventNumber = 0; eventNumber < numberEntries; eventNumber++)
  {
    integral = 0;
    baseline = 0;
    polarity = 0; //Reset variables to zero so we don't mix things up when moving between waveforms

    if (eventNumber%1000 == 0)
    {
      printf("Currently on event %d of %d\n", eventNumber, numberEntries); //I like to include a progress bar to show that program is stil running
    }
    T->GetEntry(eventNumber);
    readWaveform = *waveform;

    //Loop through waveform, baseline subtraction, and get integral
    for (Int_t i = 0; i < readWaveform.size(); i++)
    {
      if (i < baselineLength)
      {
	//Baseline calculation
        baseline += (readWaveform[i]*1.0/baselineLength*1.0);
      }

      if (i >= roiStart && i <= roiEnd)
      {
	//Calculate integral
        foo = (readWaveform[i]-baseline)*polarity;
	integral += foo;
      }
    }

    //Once we have the integral of a waveform, put it in the histogram
    spectrumHist->Fill(integral);
  }

  //Graphing
  TCanvas *c1 = new TCanvas("c1", "", 1000, 600);
  gStyle->SetOptStat(0);
  c1->SetLogy(); //Set the y-axis to be log-scale. Makes peaks more prominent
  spectrumHist->GetXaxis()->SetTitle("Energy(Uncalibrated)");
  spectrumHist->Draw("hist");
}

void viewSpectraTree(TString file)
{
  //View the spectra from the integral values in tree
  //TString file: path to file containing tree

  //Load file
  TFile *F = new TFile(file);
  TTree *T = (TTree *)F->Get("waveformTree");

  //Variables
  const Int_t numberEntries = T->GetEntries();
  const Int_t spectrumBins = 1000;
  const Int_t spectrumStart = 0;
  const Int_t spectrumEnd = 1e5; //Values for spectrum histogram size
  Double_t energy = 0; //Get integral value from tree

  T->SetBranchAddress("energy", &energy);
  //Create histogram
  TH1D* spectraHist = new TH1D("spectrumHist", "spectrumHist", spectrumBins, spectrumStart, spectrumEnd);

  for (Int_t eventNumber = 0; eventNumber < numberEntries; eventNumber++)
  {
    energy = 0; //reset variables
    if (eventNumber%1000 == 0)
    {
      printf("Currently on event %d of %d\n", eventNumber, numberEntries);
    }
    T->GetEntry(eventNumber);
    spectraHist->Fill(energy);
  }

  //Graphing
  TCanvas *c1 = new TCanvas("c1", "", 1000, 600);
  gStyle->SetOptStat(0);
  c1->SetLogy();
  spectraHist->GetXaxis()->SetTitle("Energy(Uncalibrated)");
  spectraHist->Draw("hist");
}
