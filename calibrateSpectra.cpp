#include <iostream>
#include <string>
using namespace std;
#include <cmath>
#include <vector>

void calibrateSpectra(TString file, const Int_t KLower, const Int_t KUpper, const Int_t TlLower, const Int_t TlUpper)
{
  //Script to take an uncalibrated NaI spectrum and perform a two-point calibration using potassium peak (1460keV) and thalium peak (2615keV)
  //TString file: path to file containing tree with spectra
  //Int_t KLower: lower bound of fit for potassium peak
  //Int_t KUpper: upper bound of fit for potassium peak
  //Int_t TlLower: lower bound of fit for thalium peak
  //Int_t TlUpper: upper bound of fit for thalium peak

  //Load file
  TFile *F = new TFile(file);
  TTree *T = (TTree *)F->Get("waveformTree");

  //Variables
  const Double_t KEnergy = 1460.0; //energy of potassium peak
  const Double_t TlEnergy = 2615.0; //energy of thalium peak
  Double_t KPeak = 0; //location of potassium peak
  Double_t TlPeak = 0; //location of thalium peak
  Double_t energy = 0; //uncalibrated
  Double_t calibratedEnergy = 0; //calibrated energy values
  const Int_t numberEntries = T->GetEntries();
  Double_t slope = 0;
  Double_t offset = 0; //We perform a linear transformation from the uncalibrated to the calibrated data sets
  Double_t foo = 0;

  T->SetBranchAddress("energy", &energy);
  
  //Spectrum variables
  const Int_t spectrumBins = 1000;
  const Int_t spectrumStart = 0;
  const Int_t spectrumEnd = 1e5; //uncalibrated spectra
  const Int_t energyBins = 1000;
  const Int_t energyStart = 0;
  const Int_t energyEnd = 1e4; //calibrated spectra

  //Create histograms to store both uncalibrated and calibrated spectra
  TH1D* spectrumHist = new TH1D("spectrumHist", "spectrumHist", spectrumBins, spectrumStart, spectrumEnd);
  TH1D* energyHist = new TH1D("energyHist", "energyHist", energyBins, energyStart, energyEnd); 

  //First, we get the uncalibrated spectrum
  for (Int_t eventNumber = 0; eventNumber < numberEntries; eventNumber++)
  {
    energy = 0;
    if (eventNumber%1000 == 0)
    {
      printf("Currently on event %d of %d\n", eventNumber, numberEntries);
    }
    T->GetEntry(eventNumber);
    spectrumHist->Fill(energy);
  }

  //The peaks are a gaussian+exponential fit
  TF1 *KFit = new TF1("KFit", "expo(0)+gaus(3)", KLower, KUpper);
  TF1 *KExpo = new TF1("KExpo", "expo", KLower, KUpper);
  TF1 *KGaus = new TF1("KGaus", "gaus", KLower, KUpper);
  Double_t parK[5]; //Store the fitting variables
  //First fit using just the exponential and gaussian, then use those parameters to seed the combined fit
  spectrumHist->Fit(KExpo, "qR0");
  spectrumHist->Fit(KGaus, "qR0");
  KExpo->GetParameters(&parK[0]);
  KGaus->GetParameters(&parK[2]);
  KFit->SetParameters(parK);
  spectrumHist->Fit(KFit, "qR0");
  KPeak = KFit->GetParameter(3); //Store the peak found using fit

  TF1 *TlFit = new TF1("TlFit", "expo(0)+gaus(3)", TlLower, TlUpper);
  TF1 *TlExpo = new TF1("TlExpo", "expo", TlLower, TlUpper);
  TF1 *TlGaus = new TF1("TlGaus", "gaus", TlLower, TlUpper);
  Double_t parTl[5];
  spectrumHist->Fit(TlExpo, "qR0");
  spectrumHist->Fit(TlGaus, "qR0");
  TlExpo->GetParameters(&parTl[0]);
  TlGaus->GetParameters(&parTl[2]);
  TlFit->SetParameters(parTl);
  spectrumHist->Fit(TlFit, "qR0");
  TlPeak = TlFit->GetParameter(3);
  
  slope = (TlEnergy-KEnergy)/(TlPeak-KPeak);
  offset = KEnergy - slope*KPeak;

  //Perform the calibration
  for (Int_t eventNumber = 0; eventNumber < numberEntries; eventNumber++)
  {
    energy = 0;
    calibratedEnergy = 0;
    if (eventNumber%1000 == 0)
    {
      printf("Calibrating event %d of %d\n", eventNumber, numberEntries);
    }
    T->GetEntry(eventNumber);
    
    calibratedEnergy = slope*energy*1.0+offset;
    energyHist->Fill(calibratedEnergy);
  }

  //Graphing
  TCanvas *c1 = new TCanvas("c1", "", 1000, 600);
  gStyle->SetOptStat(0);
  c1->SetLogy();
  energyHist->GetXaxis()->SetTitle("Energy(keV)");
  energyHist->Draw("hist");
}
