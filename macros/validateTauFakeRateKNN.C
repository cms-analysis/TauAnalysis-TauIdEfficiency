
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

//const int maxEntriesToProcess = 10000; // for testing purposes only
const int maxEntriesToProcess = -1;

TH1* fillHistogram(TTree* testTree, const std::string& varName, const std::string& selection, const std::string& frWeightName,
		   const std::string& histogramName, unsigned numBinsX, double xMin, double xMax)
{
  std::cout << "<fillHistogram>:" << std::endl;
  std::cout << " testTree = " << testTree << std::endl;
  std::cout << " varName = " << varName << std::endl;
  std::cout << " selection = " << selection << std::endl;
  std::cout << " frWeightName = " << frWeightName << std::endl;
  std::cout << " histogramName = " << histogramName << std::endl;

  TH1* histogram = new TH1F(histogramName.data(), histogramName.data(), numBinsX, xMin, xMax);
  histogram->Sumw2();

  TFile* dummyOutputFile = new TFile("dummyOutputFile.root", "RECREATE");

  TTree* selTree = ( selection != "" ) ? testTree->CopyTree(selection.data()) : testTree;
  std::cout << " selTree = " << selTree << std::endl;

  Float_t eventWeight = 1.;
  selTree->SetBranchAddress("weight", &eventWeight);

  Float_t var = 0.;
  selTree->SetBranchAddress(varName.data(), &var);
  
  Float_t frWeight = 1.;
  if ( frWeightName != "" ) {
    std::cout << "--> setting branch-address of fake-rate weight..." << std::endl;
    selTree->SetBranchAddress(frWeightName.data(), &frWeight);
  }

  int numEntries = selTree->GetEntries();
  std::cout << "--> numEntries = " << numEntries << std::endl;
  //if ( maxEntriesToProcess != -1 ) numEntries = TMath::Min(numEntries, maxEntriesToProcess);
  for ( int iEntry = 0 ; iEntry < numEntries; ++iEntry ) {
    selTree->GetEvent(iEntry);  

    //std::cout << "iEntry = " << iEntry << ": var = " << var << ", frWeight = " << frWeight << std::endl;

    if ( frWeightName != "" ) {
      if ( TMath::Abs(frWeight) < 1. ) // some entries have weight O(-100)
                                       // --> indication of technical problem with k-NearestNeighbour tree ?
	histogram->Fill(var, frWeight*eventWeight);
    } else {
      histogram->Fill(var, eventWeight);
    }
  }

  delete dummyOutputFile;

  return histogram;
}


void makePlot(TCanvas* canvas, const std::string& outputFileName, TTree* testTree, const std::string& varName, 
	      unsigned numBinsX, double xMin, double xMax)
{
  std::cout << "creating histogramTauIdPassed..." << std::endl;
  TString histogramTauIdPassedName = TString("histogramTauIdPassed").Append("_").Append(varName.data());
  TH1* histogramTauIdPassed = fillHistogram(testTree, varName, "type==1", "",
					    histogramTauIdPassedName.Data(), numBinsX, xMin, xMax);
  std::cout << "--> histogramTauIdPassed = " << histogramTauIdPassed << ":" 
	    << " integral = " << histogramTauIdPassed->Integral() << std::endl;

  std::cout << "creating histogramTauIdFailed..." << std::endl;
  TString histogramTauIdFailedName = TString("histogramTauIdFailed").Append("_").Append(varName.data());
  TH1* histogramTauIdFailed = fillHistogram(testTree, varName, "type==0", "",
					    histogramTauIdFailedName.Data(), numBinsX, xMin, xMax);
  std::cout << "--> histogramTauIdFailed = " << histogramTauIdFailed 
	    << " integral = " << histogramTauIdFailed->Integral() << std::endl;

  std::cout << "creating histogramTauIdDenominator..." << std::endl;
  TString histogramTauIdDenominatorName = TString("histogramTauIdDenominator").Append("_").Append(varName.data());
  TH1* histogramTauIdDenominator = new TH1F(histogramTauIdDenominatorName.Data(), 
					    histogramTauIdDenominatorName.Data(), numBinsX, xMin, xMax);
  histogramTauIdDenominator->Add(histogramTauIdPassed);
  histogramTauIdDenominator->Add(histogramTauIdFailed);
  std::cout << "--> histogramTauIdDenominator = " << histogramTauIdDenominator 
	    << " integral = " << histogramTauIdDenominator->Integral() << std::endl;

  std::cout << "creating histogramFakeRate..." << std::endl;
  TString histogramFakeRateName = TString("histogramFakeRate").Append("_").Append(varName.data());
  TH1* histogramFakeRate = new TH1F(histogramFakeRateName.Data(), 
				    histogramFakeRateName.Data(), numBinsX, xMin, xMax);
  histogramFakeRate->Add(histogramTauIdPassed);
  histogramFakeRate->Divide(histogramTauIdDenominator);
  std::cout << "--> histogramFakeRate = " << histogramFakeRate 
	    << " integral = " << histogramFakeRate->Integral() << std::endl;

  std::cout << "creating histogramFakeRateWeighted..." << std::endl;
  TString histogramFakeRateWeightedName = TString("histogramFakeRateWeighted").Append("_").Append(varName.data());
  TH1* histogramFakeRateWeighted = fillHistogram(testTree, varName, "", "MVA_KNN", 
						 histogramFakeRateWeightedName.Data(), numBinsX, xMin, xMax);
  histogramFakeRateWeighted->Divide(histogramTauIdDenominator);
  std::cout << "--> histogramFakeRateWeighted = " << histogramFakeRateWeighted 
	    << " entries = " << histogramFakeRateWeighted->GetEntries() << ","
	    << " integral = " << histogramFakeRateWeighted->Integral() << std::endl;
  // Scale the weighted fake rate histogram

  histogramFakeRate->SetTitle(varName.data());
  histogramFakeRate->SetStats(false);
  histogramFakeRate->SetMinimum(1.e-4);
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

  TString inputFilePath = "/data2/friis/FakeRatesHPSXCheck_V2/fakerate_ewkTauIdHPSloose_WplusJets_data_jetNoEta20FullVal/train";
  //TString inputFilePath = "/data2/friis/FakeRatesHPSXCheck_V2/fakerate_ewkTauIdHPSloose_PPmuX_data_jetNoEta20FullVal/train";
  TString inputFileName = "train_FakeRateMethod_output.root";

  TFile* file = TFile::Open(TString(inputFilePath).Append("/").Append(inputFileName));

  TString testTreeName = "TestTree";
  TTree* testTree = (TTree*)file->Get(testTreeName);

  std::string ptName = "Pt";
  std::string etaName = "AbsEta";
//--- Check if fake-rates are parametrized by jetPt or tau Pt
  if ( testTree->GetBranch("JetPt") ) {
    std::cout << "Using Jet variables" << std::endl;
    ptName = "JetPt";
    etaName = "AbsJetEta";
  }

  makePlot(canvas, "validateTauFakeRateKNN_WplusJets_JetPt.png", testTree, ptName, 10, 0., 100.);
  makePlot(canvas, "validateTauFakeRateKNN_WplusJets_JetEta.png", testTree, etaName, 10, -2.5, +2.5);
  makePlot(canvas, "validateTauFakeRateKNN_WplusJets_JetRadius.png", testTree, "JetWidth", 10, -0.01, 0.51);
  //makePlot(canvas, "validateTauFakeRateKNN_QCD_JetPt.png", testTree, ptName, 10, 0., 100.);
  //makePlot(canvas, "validateTauFakeRateKNN_QCD_JetEta.png", testTree, etaName, 10, -2.5, +2.5);
  //makePlot(canvas, "validateTauFakeRateKNN_QCD_JetRadius.png", testTree, "JetWidth", 10, -0.01, 0.51);
  
  delete file;

  delete canvas;
}
