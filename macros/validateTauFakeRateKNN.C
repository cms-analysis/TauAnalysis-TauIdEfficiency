
//-------------------------------------------------------------------------------
// test that k-NearestNeighbour tree storing fake-rates has been filled correctly
// 
// NOTE: macro has to be run in ACLiC compiled mode via
//         root
//         .x validateTauFakeRateKNN.C++
//
// 
//-------------------------------------------------------------------------------

#include <TCanvas.h>
#include <TString.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1.h>
#include <TProfile.h>
#include <TLegend.h>
#include <TMath.h>

#include <iostream>
#include <iomanip>

TH1* fillHistogram(TTree* testTree, const std::string& varName, const std::string& selection, const std::string& weightName,
		   const std::string& histogramName, unsigned numBinsX, double xMin, double xMax)
{
  std::cout << "<fillHistogram>:" << std::endl;
  std::cout << " testTree = " << testTree << std::endl;
  std::cout << " varName = " << varName << std::endl;
  std::cout << " selection = " << selection << std::endl;
  std::cout << " weightName = " << weightName << std::endl;
  std::cout << " histogramName = " << histogramName << std::endl;

  TH1* histogram = 0;
  if ( weightName != "" ) {
    histogram = new TProfile(histogramName.data(), histogramName.data(), numBinsX, xMin, xMax);
  } else {
    histogram = new TH1F(histogramName.data(), histogramName.data(), numBinsX, xMin, xMax);
    histogram->Sumw2();
  }

  TTree* selTree = ( selection != "" ) ? testTree->CopyTree(selection.data()) : testTree;
  std::cout << " selTree = " << selTree << std::endl;

  Float_t var = 0.;
  selTree->SetBranchAddress(varName.data(), &var);
  
  Float_t weight = 1.;
  if ( weightName != "" ) {
    std::cout << "--> setting branch-address of weight..." << std::endl;
    selTree->SetBranchAddress(weightName.data(), &weight);
  }

  int numEntries = selTree->GetEntries();
  std::cout << "--> numEntries = " << numEntries << std::endl;
  for ( int iEntry = 0 ; iEntry < numEntries; ++iEntry ) {
    selTree->GetEvent(iEntry);  

    std::cout << "iEntry = " << iEntry << ": var = " << var << ", weight = " << weight << std::endl;

    if ( weightName != "" )
      histogram->Fill(var, weight);
    else
      histogram->Fill(var);
  }

  return histogram;
}


void makePlot(TCanvas* canvas, const std::string& outputFileName, TTree* testTree, const std::string& varName, 
	      unsigned numBinsX, double xMin, double xMax)
{
  std::cout << "creating histogramFakeRateWeighted..." << std::endl;
  TString histogramFakeRateWeightedName = TString("histogramFakeRateWeighted").Append("_").Append(varName.data());
  TH1* histogramFakeRateWeighted = fillHistogram(testTree, varName, "", "MVA_KNN", 
						 histogramFakeRateWeightedName.Data(), numBinsX, xMin, xMax);
  std::cout << "--> histogramFakeRateWeighted = " << histogramFakeRateWeighted << std::endl;

  std::cout << "creating histogramTauIdPassed..." << std::endl;
  TString histogramTauIdPassedName = TString("histogramTauIdPassed").Append("_").Append(varName.data());
  TH1* histogramTauIdPassed = fillHistogram(testTree, varName, "type==1", "",
					    histogramTauIdPassedName.Data(), numBinsX, xMin, xMax);
  std::cout << "--> histogramTauIdPassed = " << histogramTauIdPassed << std::endl;

  std::cout << "creating histogramTauIdFailed..." << std::endl;
  TString histogramTauIdFailedName = TString("histogramTauIdFailed").Append("_").Append(varName.data());
  TH1* histogramTauIdFailed = fillHistogram(testTree, varName, "type==0", "",
					    histogramTauIdFailedName.Data(), numBinsX, xMin, xMax);
  std::cout << "--> histogramTauIdFailed = " << histogramTauIdFailed << std::endl;

  std::cout << "creating histogramTauIdDenominator..." << std::endl;
  TString histogramTauIdDenominatorName = TString("histogramTauIdDenominator").Append("_").Append(varName.data());
  TH1* histogramTauIdDenominator = new TH1F(histogramTauIdDenominatorName.Data(), 
					    histogramTauIdDenominatorName.Data(), numBinsX, xMin, xMax);
  histogramTauIdDenominator->Add(histogramTauIdPassed);
  histogramTauIdDenominator->Add(histogramTauIdFailed);
  std::cout << "--> histogramTauIdDenominator = " << histogramTauIdDenominator << std::endl;

  std::cout << "creating histogramFakeRate..." << std::endl;
  TString histogramFakeRateName = TString("histogramFakeRate").Append("_").Append(varName.data());
  TH1* histogramFakeRate = new TH1F(histogramFakeRateName.Data(), 
				    histogramFakeRateName.Data(), numBinsX, xMin, xMax);
  histogramFakeRate->Add(histogramTauIdPassed);
  histogramFakeRate->Divide(histogramTauIdDenominator);
  std::cout << "--> histogramFakeRate = " << histogramFakeRate << std::endl;

  histogramFakeRate->SetTitle(varName.data());
  histogramFakeRate->SetStats(false);
  histogramFakeRate->SetMinimum(1.e-3);
  histogramFakeRate->SetMaximum(1.e+1);
  histogramFakeRate->SetLineColor(2);
  histogramFakeRate->SetLineWidth(2);
  histogramFakeRate->SetMarkerStyle(20);
  histogramFakeRate->SetMarkerColor(2);
  histogramFakeRate->SetMarkerSize(1);
  histogramFakeRate->Draw("e1p");

  histogramFakeRateWeighted->SetLineColor(4);
  histogramFakeRateWeighted->SetLineWidth(2);
  histogramFakeRateWeighted->SetMarkerStyle(24);
  histogramFakeRateWeighted->SetMarkerColor(4);
  histogramFakeRateWeighted->SetMarkerSize(1);
  histogramFakeRateWeighted->Draw("e1psame");

  TLegend legend(0.11, 0.73, 0.31, 0.89);
  legend.SetBorderSize(0);
  legend.SetFillColor(0);
  legend.AddEntry(histogramFakeRate, "Tau id. discr.", "p");
  legend.AddEntry(histogramFakeRateWeighted, "Fake-Rate weight", "p");
  legend.Draw();

  canvas->Update();
  canvas->Print(outputFileName.data());
}

void validateTauFakeRateKNN()
{
  TCanvas* canvas = new TCanvas("canvas", "canvas", 1, 1, 800, 600);
  canvas->SetFillColor(10);
  canvas->SetBorderSize(2);
  
  canvas->SetLogy();

  TString inputFilePath = "/afs/cern.ch/user/v/veelken/scratch0/CMSSW_3_8_5/src/TauAnalysis/TauIdEfficiency/test/commissioning/train";
  TString inputFileName = "train_FakeRateMethod_output.root";

  TFile* file = TFile::Open(TString(inputFilePath).Append("/").Append(inputFileName));

  TString testTreeName = "TestTree";
  TTree* testTree = (TTree*)file->Get(testTreeName);

  makePlot(canvas, "validateTauFakeRateKNN_JetPt.png", testTree, "JetPt", 10, 0., 100.);
  makePlot(canvas, "validateTauFakeRateKNN_JetEta.png", testTree, "JetEta", 10, -2.5, +2.5);
  
  delete file;

  delete canvas;
}