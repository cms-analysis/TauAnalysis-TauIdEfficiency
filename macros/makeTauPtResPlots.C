
void drawHistogram(TCanvas* canvas, const TH1* me, int color, const TString& drawOption, double yShiftStats)
{
  me->Scale(1./me->Integral());
  me->SetLineColor(color);
  me->SetLineWidth(2); 
  // CV: need to call TH1::Draw and TCanvas::Update,
  //     in order for the Stats box of the histogram to be created,
  //     c.f. http://root.cern.ch/root/html/TPaveStats.html
  me->SetStats(true);
  me->Draw("sames");
  canvas->Update();
  TPaveStats* stats = (TPaveStats*)me->FindObject("stats");
  stats->SetX1NDC(stats->GetX1NDC());
  stats->SetY1NDC(stats->GetY1NDC() - yShiftStats);
  stats->SetX2NDC(stats->GetX2NDC());
  stats->SetY2NDC(stats->GetY2NDC() - yShiftStats);
  TPaveText* text = new TPaveText(stats->GetX1NDC(), stats->GetY1NDC(), stats->GetX2NDC(), stats->GetY2NDC(), "brNDC");
  int numLines = stats->GetSize();
  for ( int iLine = 1; iLine < numLines; ++iLine ) {  
    //std::cout << "iLine = " << iLine << ": Line = " << stats->GetLine(iLine)->GetTitle() << std::endl;
    text->AddText(stats->GetLine(iLine)->GetTitle());
  }
  text->AddText(Form("RMS/Mean = %1.4f", me->GetRMS()/me->GetMean()));
  me->SetStats(false);
  me->Draw(drawOption);
  text->SetTextColor(color);
  text->Draw();
}

void makeTauPtResPlot(TCanvas* canvas, TFile* inputFile, 
		      const TString& title, double yMax, double yMin, const TString& tauDecayMode, const TString& outputFileName,
		      const TString& meName1, int color1, const TString& legendEntry1,
		      const TString& meName2, int color2, const TString& legendEntry2,
		      const TString& meName3 = "", int color3 = 0, const TString& legendEntry3 = "",
		      const TString& meName4 = "", int color4 = 0, const TString& legendEntry4 = "",
		      const TString& meName5 = "", int color5 = 0, const TString& legendEntry5 = "")
{
  TH1* me1 = dynamic_cast<TH1*>(inputFile->Get(TString(meName1).Append(tauDecayMode)));
  std::cout << "meName1 = " << meName1 << ": me1 = " << me1 << std::endl;
  assert(me1);
  if ( title != "" ) me1->SetTitle(title);
  me1->GetXaxis()->SetTitle("P_{T}^{rec} / P_{T}^{gen}");
  me1->GetXaxis()->SetTitleOffset(1.2);
  me1->GetYaxis()->SetTitle("a.u.");
  me1->GetXaxis()->SetTitleOffset(1.15);
  drawHistogram(canvas, me1, color1, "", 0.07);
  me1->SetMaximum(yMax);
  me1->SetMinimum(yMin);
  unsigned numHistograms = 1;

  TH1* me2 = dynamic_cast<TH1*>(inputFile->Get(TString(meName2).Append(tauDecayMode)));
  std::cout << "meName2 = " << meName2 << ": me2 = " << me2 << std::endl;
  assert(me2);
  drawHistogram(canvas, me2, color2, "sames", 0.25);
  ++numHistograms;

  TH1* me3 = 0;
  if ( meName3 != "" ) {
    me3 = dynamic_cast<TH1*>(inputFile->Get(TString(meName3).Append(tauDecayMode)));
    //std::cout << "meName3 = " << meName3 << ": me3 = " << me3 << std::endl;
    assert(me3);
    drawHistogram(canvas, me3, color3, "sames", 0.43);
    ++numHistograms;
  }

  TH1* me4 = 0;
  if ( meName4 != "" ) {
    me4 = dynamic_cast<TH1*>(inputFile->Get(TString(meName4).Append(tauDecayMode)));
    //std::cout << "meName4 = " << meName4 << ": me4 = " << me4 << std::endl;
    assert(me4);
    drawHistogram(canvas, me4, color4, "sames", 0.61);
    ++numHistograms;
  }

  TH1* me5 = 0;
  if ( meName5 != "" ) {
    me5 = dynamic_cast<TH1*>(inputFile->Get(TString(meName5).Append(tauDecayMode)));
    //std::cout << "meName5 = " << meName5 << ": me5 = " << me5 << std::endl;
    assert(me5);
    drawHistogram(canvas, me5, color5, "sames", 0.79);
    ++numHistograms;
  }

  TLegend legend(0.12, 0.87 - 0.050*numHistograms, 0.39, 0.87, "", "brNDC"); 
  legend.SetBorderSize(0);
  legend.SetFillColor(0);
  legend.SetTextSize(0.035);
  legend.AddEntry(me1, legendEntry1, "l");
  legend.AddEntry(me2, legendEntry2, "l");
  if ( me3 ) legend.AddEntry(me3, legendEntry3, "l");
  if ( me4 ) legend.AddEntry(me4, legendEntry4, "l");
  if ( me5 ) legend.AddEntry(me5, legendEntry5, "l");
  legend.Draw();

  canvas->Update();
  canvas->Print(outputFileName.Data());
}


void makeTauPtResPlots()
{
  TString inputFileName = "/data1/veelken/tmp/tauPtResStudies/V2exp/analyzeTauPtResHistograms_all_Ztautau_powheg_V2exp.root";
  TFile* inputFile = new TFile(inputFileName);
  //std::cout << "inputFileName = " << inputFileName << ": inputFile = " << inputFile << std::endl;

  TCanvas* canvas = new TCanvas("canvas", "canvas", 800, 640);
  canvas->SetFillColor(10);
  canvas->SetBorderSize(2);
  canvas->SetLeftMargin(0.10);
  canvas->SetRightMargin(0.24);
  canvas->SetBottomMargin(0.12);

  TObjArray tauDecayModesToPlot;
  tauDecayModesToPlot.Add(new TObjString(""));
  tauDecayModesToPlot.Add(new TObjString("GenOneProng0Pi0"));
  tauDecayModesToPlot.Add(new TObjString("GenOneProng1Pi0"));
  tauDecayModesToPlot.Add(new TObjString("GenOneProng2Pi0"));
  tauDecayModesToPlot.Add(new TObjString("GenThreeProng0Pi0"));
  tauDecayModesToPlot.Add(new TObjString("GenThreeProng1Pi0"));

  for ( int i = 0; i < tauDecayModesToPlot.GetEntries(); ++i ) {
    TString tauDecayModeToPlot = ((TObjString*)tauDecayModesToPlot.At(i))->GetString();

    canvas->SetLogy(true);

    makeTauPtResPlot(canvas, inputFile, "", 1.e1, 1.e-4, tauDecayModeToPlot, 
		     Form("tauPtRes_newTags_%s_log.eps", tauDecayModeToPlot.Data()),
		     "tauPtRes", 1, "def. Tau P_{T}",
		     "jetPtRes", 3, "Jet P_{T}");

/*
    makeTauPtResPlot(canvas, inputFile, "", 1.e1, 1.e-4, tauDecayModeToPlot, 
		     Form("tauPtRes_addLowEtPhotons_%s_log.png", tauDecayModeToPlot.Data()),
		     "Default/tauPtRes", 1, "default",
		     "Seed05Add00Strip05/tauPtRes", 2, "add PFGamma");

    makeTauPtResPlot(canvas, inputFile, "", 1.e1, 1.e-4, tauDecayModeToPlot, 
		     Form("tauPtRes_noEleTrackQcuts_%s_log.png", tauDecayModeToPlot.Data()),
		     "Default/tauPtRes", 1, "default",
		     "noEleTrackQcutsDefault/tauPtRes", 2, "no e Track Qcuts");

    //makeTauPtResPlot(canvas, inputFile, "", 1.e1, 1.e-4, tauDecayModeToPlot, 
    //		       Form("tauPtRes_nuclIntEnRecovery_%s_log.png", tauDecayModeToPlot.Data()),
    //		       "Default/tauPtRes", 1, "default",
    //		       "Default/tauPtResManCorrLev1", 2, "corr. Level 1",
    //		       "Default/tauPtResManCorrLev2", 3, "corr. Level 2",
    //		       "Default/tauPtResManCorrLev12", 4, "corr. Level 1+2",
    //		       "Default/tauPtResManCorrLev123", 6, "corr. Level 1+2+3");
    makeTauPtResPlot(canvas, inputFile, "", 1.e1, 1.e-4, tauDecayModeToPlot, 
		     Form("tauPtRes_nuclIntEnRecovery_%s_log.png", tauDecayModeToPlot.Data()),
		     "Default/tauPtRes", 1, "default",
		     "Default/tauPtResManCorrLev1", 2, "corr. Level 1",
		     "Default/tauPtResManCorrLev14", 4, "corr. Level 1+4");

    makeTauPtResPlot(canvas, inputFile, "", 1.e1, 1.e-4, tauDecayModeToPlot, 
		     Form("tauPtRes_combImprovements_%s_log.png", tauDecayModeToPlot.Data()),
		     "Default/tauPtRes", 1, "def. Tau P_{T}",
		     "noEleTrackQcutsDefault/tauPtResManCorrLev14", 2, "impr. Tau P_{T} (eTr+1+4)",
		     "Default/jetPtRes", 3, "Jet P_{T}");

    canvas->SetLogy(false);

    makeTauPtResPlot(canvas, inputFile, "", 0.50, 0., tauDecayModeToPlot, 
		     Form("tauPtRes_combImprovements_%s_linear.png", tauDecayModeToPlot.Data()),
		     "Default/tauPtRes", 1, "def. Tau P_{T}",
		     "noEleTrackQcutsDefault/tauPtResManCorrLev14", 2, "impr. Tau P_{T} (eTr+1+4)",
		     "Default/jetPtRes", 3, "Jet P_{T}");
 */
  }
 
  delete inputFile;
}
