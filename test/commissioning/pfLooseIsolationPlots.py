#!/usr/bin/env python

'''
Make control plots comparing PFLooseIsolation Pt sums for
 o tau --> e
 o tau --> mu
 o tau --> had.
decays
Authors: Christian Veelken

'''

import ROOT
import samples_cache as samples
from TauAnalysis.TauIdEfficiency.ntauples.PlotManager import PlotManager
from TauAnalysis.TauIdEfficiency.ntauples.plotting import draw
import TauAnalysis.TauIdEfficiency.ntauples.styles as style
import sys
import copy

maxNumEntries = 1000000000
#maxNumEntries = 10000

def makeLoosePFIsoPlot(ztt_events,
                       genSelection_e, genSelection_mu, genSelection_had,
                       recSelection, recExpression,
                       binning,
                       outputFileName):
    canvas = ROOT.TCanvas("canvas", "canvas", 500, 500)
    canvas.SetLeftMargin(0.11)
    canvas.SetGridy(1)

    isFirstHistogram = True

    histogram_e = None
    if genSelection_e is not None:
        histogram_e = draw(
            ztt_events,
            expression = recExpression,
            selection = genSelection_e & recSelection,
            binning = (20, 0, 10),
            output_name = 'e',
            maxNumEntries = maxNumEntries
        )
        if not histogram_e.GetSumw2N():
            histogram_e.Sumw2()
        histogram_e.Scale(1./histogram_e.Integral())
        histogram_e.SetMarkerSize(2)
        histogram_e.SetMarkerStyle(20)
        histogram_e.SetMarkerColor(3)
        histogram_e.SetLineColor(3)
        histogram_e.Draw("e1p")
        isFirstHistogram = False

    histogram_mu = None
    if genSelection_mu is not None:
        histogram_mu = draw(
            ztt_events,
            expression = recExpression,
            selection = genSelection_mu & recSelection,
            binning = (20, 0, 10),
            output_name = 'mu',
            maxNumEntries = maxNumEntries
        )
        if not histogram_mu.GetSumw2N():
            histogram_mu.Sumw2()
        histogram_mu.Scale(1./histogram_mu.Integral())
        histogram_mu.SetMarkerSize(2)
        histogram_mu.SetMarkerStyle(20)
        histogram_mu.SetMarkerColor(4)
        histogram_mu.SetLineColor(4)
        if isFirstHistogram:
            histogram_mu.Draw("e1p")
        else:
            histogram_mu.Draw("e1psame")
        isFirstHistogram = False

    histogram_had = None
    if genSelection_had is not None:
        histogram_had = draw(
            ztt_events,
            expression = recExpression,
            selection = genSelection_had & recSelection,
            binning = (20, 0, 10),
            output_name = 'had',
            maxNumEntries = maxNumEntries
        )
        if not histogram_had.GetSumw2N():
            histogram_had.Sumw2()
        histogram_had.Scale(1./histogram_had.Integral())
        histogram_had.SetMarkerSize(2)
        histogram_had.SetMarkerStyle(20)
        histogram_had.SetMarkerColor(2)
        histogram_had.SetLineColor(2)
        if isFirstHistogram:
            histogram_had.Draw("e1p")
        else:
            histogram_had.Draw("e1psame")
        
    # Draw the legend - you can pass NDC xl, yl, xh, yh coordinates to make_legend(...)
    legend = ROOT.TLegend(0.60, 0.69, 0.88, 0.88)
    if histogram_e is not None:
        legend.AddEntry(histogram_e, "#tau #rightarrow e", "p")
    if histogram_mu is not None:     
        legend.AddEntry(histogram_mu, "#tau #rightarrow #mu", "p")
    if histogram_had is not None: 
        legend.AddEntry(histogram_had, "#tau #rightarrow had.", "p")
    legend.Draw()
    
    canvas.SaveAs(outputFileName[:outputFileName.rfind(".")] + ".png")
    canvas.SaveAs(outputFileName[:outputFileName.rfind(".")] + ".pdf")
    canvas.SaveAs(outputFileName[:outputFileName.rfind(".")] + ".root")
    
if __name__ == "__main__":
    ROOT.gROOT.SetBatch(True)
    ROOT.gROOT.SetStyle("Plain")
    ROOT.gStyle.SetOptStat(0)

    # Get ZTT samples
    ##ztt = samples.ztautau_mc
    ztt = samples.zttPU156bx_mc

    # Get the ntuple we produced
    ntuple_manager = ztt.build_ntuple_manager("tauIdEffNtuple")

    genTaus = ntuple_manager.get_ntuple("tauGenJets")
    genSelection_e   = genTaus.expr('$genDecayMode == 0   && $genPt > 10 && abs($genEta) < 2.1')
    genSelection_mu  = genTaus.expr('$genDecayMode == 1   && $genPt > 10 && abs($genEta) < 2.1')
    genSelection_had = genTaus.expr('$genDecayMode >  1.5 && $genPt > 10 && abs($genEta) < 2.1')

    recTaus = ntuple_manager.get_ntuple("hpstanc")
    accSelection                = recTaus.expr('$pt > 20 && abs($eta) < 2.3')
    #accSelection                = recTaus.expr('$pt > 20 && abs($eta) < 2.1')
    recSelectionBeforeTaNC      = accSelection            & recTaus.expr('$byLeadTrackFinding > 0.5 && $byLeadTrackPtCut > 0.5')
    recSelectionAfterTaNCloose  = recSelectionBeforeTauId & recTaus.expr('$byTaNCloose  > 0.5')
    recSelectionAfterTaNCmedium = recSelectionBeforeTauId & recTaus.expr('$byTaNCmedium > 0.5')
    recSelectionAfterTaNCtight  = recSelectionBeforeTauId & recTaus.expr('$byTaNCtight  > 0.5')
    recSelectionAfterHPSloose   = recSelectionBeforeTauId & recTaus.expr('$byHPSloose   > 0.5')
    recSelectionAfterHPSmedium  = recSelectionBeforeTauId & recTaus.expr('$byHPSmedium  > 0.5')
    recSelectionAfterHPStight   = recSelectionBeforeTauId & recTaus.expr('$byHPStight   > 0.5')

    recExpressionLoosePFIso04 = recTaus.expr('$ptSumLooseIsolation04')
    recExpressionLoosePFIso06 = recTaus.expr('$ptSumLooseIsolation06')
    binningLoosePFIso = (20, 0, 10)

    recSelectionAfterHPSlooseNoLeadTrackPt  = accSelection & recTaus.expr('$byLeadTrackFinding > 0.5 && $byHPSloose  > 0.5')
    recSelectionAfterHPSmediumNoLeadTrackPt = accSelection & recTaus.expr('$byLeadTrackFinding > 0.5 && $byHPSmedium > 0.5')
    recSelectionAfterHPStightNoLeadTrackPt  = accSelection & recTaus.expr('$byLeadTrackFinding > 0.5 && $byHPStight  > 0.5')

    recExpressionLeadTrackFindingDiscr = recTaus.expr('$byLeadTrackFinding')
    recExpressionLeadTrackPtDiscr = recTaus.expr('$byLeadTrackPtCut')
    binningTauIdDiscr = (2, -0.5, 1.5)

    recSelectionLoosePFIso04     = recSelectionBeforeTauId & recTaus.expr('$ptSumLooseIsolation04 < 2.5')
    recSelectionAntiLoosePFIso04 = recSelectionBeforeTauId & recTaus.expr('$ptSumLooseIsolation04 > 2.5')
    recSelectionLoosePFIso06     = recSelectionBeforeTauId & recTaus.expr('$ptSumLooseIsolation06 < 2.5')
    recSelectionAntiLoosePFIso06 = recSelectionBeforeTauId & recTaus.expr('$ptSumLooseIsolation06 > 2.5')
    recExpressionTaNCloose  = recTaus.expr('$byTaNCloose')
    recExpressionTaNCmedium = recTaus.expr('$byTaNCmedium')
    recExpressionTaNCtight  = recTaus.expr('$byTaNCtight')
    recExpressionHPSloose   = recTaus.expr('$byHPSloose')
    recExpressionHPSmedium  = recTaus.expr('$byHPSmedium')
    recExpressionHPStight   = recTaus.expr('$byHPStight')

    ztt_events = list(ztt.events_and_weights())[0][0]

    ##makeLoosePFIsoPlot(ztt_events,
    ##                   genSelection_e, genSelection_mu, genSelection_had,
    ##                   recSelectionBeforeTauId, recExpressionLoosePFIso04, binningLoosePFIso,
    ##                   "./plots/pfLooseIsolation04_beforeTauId.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   genSelection_e, genSelection_mu, genSelection_had,
    ##                   recSelectionBeforeTauId, recExpressionLoosePFIso06, binningLoosePFIso,
    ##                   "./plots/pfLooseIsolation06_beforeTauId.png")

    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionAfterTaNCloose, recExpressionLoosePFIso04, binningLoosePFIso,
    ##                   "./plots/pfLooseIsolation04_afterTaNCloose.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionAfterTaNCloose, recExpressionLoosePFIso06, binningLoosePFIso,
    ##                   "./plots/pfLooseIsolation06_afterTaNCloose.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionAfterTaNCmedium, recExpressionLoosePFIso04, binningLoosePFIso,
    ##                   "./plots/pfLooseIsolation04_afterTaNCmedium.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionAfterTaNCmedium, recExpressionLoosePFIso06, binningLoosePFIso,
    ##                   "./plots/pfLooseIsolation06_afterTaNCmedium.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionAfterTaNCtight, recExpressionLoosePFIso04, binningLoosePFIso,
    ##                   "./plots/pfLooseIsolation04_afterTaNCtight.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionAfterTaNCtight, recExpressionLoosePFIso06, binningLoosePFIso,
    ##                   "./plots/pfLooseIsolation06_afterTaNCtight.png")

    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionAfterHPSloose, recExpressionLoosePFIso04, binningLoosePFIso,
    ##                   "./plots/pfLooseIsolation04_afterHPSloose.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionAfterHPSloose, recExpressionLoosePFIso06, binningLoosePFIso,
    ##                   "./plots/pfLooseIsolation06_afterHPSloose.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionAfterHPSmedium, recExpressionLoosePFIso04, binningLoosePFIso,
    ##                   "./plots/pfLooseIsolation04_afterHPSmedium.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionAfterHPSmedium, recExpressionLoosePFIso06, binningLoosePFIso,
    ##                   "./plots/pfLooseIsolation06_afterHPSmedium.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionAfterHPStight, recExpressionLoosePFIso04, binningLoosePFIso,
    ##                   "./plots/pfLooseIsolation04_afterHPStight.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionAfterHPStight, recExpressionLoosePFIso06, binningLoosePFIso,
    ##                   "./plots/pfLooseIsolation06_afterHPStight.png")

    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionAfterHPSlooseNoLeadTrackPt, recExpressionLeadTrackPtDiscr, binningTauIdDiscr,
    ##                   "./plots/leadTrackPtDiscr_afterHPSloose.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionAfterHPSmediumNoLeadTrackPt, recExpressionLeadTrackPtDiscr, binningTauIdDiscr,
    ##                   "./plots/leadTrackPtDiscr_afterHPSmedium.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionAfterHPStightNoLeadTrackPt, recExpressionLeadTrackPtDiscr, binningTauIdDiscr,
    ##                   "./plots/leadTrackPtDiscr_afterHPStight.png")

    makeLoosePFIsoPlot(ztt_events,
                       None, None, genSelection_had,
                       accSelection, recExpressionLeadTrackFindingDiscr, binningTauIdDiscr,
                       "./plots/leadTrackFindingDiscr_wrtAcceptance.png")
    makeLoosePFIsoPlot(ztt_events,
                       None, None, genSelection_had,
                       accSelection, recExpressionLeadTrackPtDiscr, binningTauIdDiscr,
                       "./plots/leadTrackPtDiscr_wrtAcceptance.png")

    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionBeforeTauId, recExpressionTaNCloose, binningTauIdDiscr,
    ##                   "./plots/byTaNClooseDiscr_beforeLoosePFIso.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionLoosePFIso04, recExpressionTaNCloose, binningTauIdDiscr,
    ##                   "./plots/byTaNClooseDiscr_afterLoosePFIso04.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionAntiLoosePFIso04, recExpressionTaNCloose, binningTauIdDiscr,
    ##                   "./plots/byTaNClooseDiscr_afterAntiLoosePFIso04.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionLoosePFIso06, recExpressionTaNCloose, binningTauIdDiscr,
    ##                   "./plots/byTaNClooseDiscr_afterLoosePFIso06.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionAntiLoosePFIso06, recExpressionTaNCloose, binningTauIdDiscr,
    ##                   "./plots/byTaNClooseDiscr_afterAntiLoosePFIso06.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionBeforeTauId, recExpressionTaNCmedium, binningTauIdDiscr,
    ##                   "./plots/byTaNCmediumDiscr_beforeLoosePFIso.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionLoosePFIso04, recExpressionTaNCmedium, binningTauIdDiscr,
    ##                   "./plots/byTaNCmediumDiscr_afterLoosePFIso04.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionAntiLoosePFIso04, recExpressionTaNCmedium, binningTauIdDiscr,
    ##                   "./plots/byTaNCmediumDiscr_afterAntiLoosePFIso04.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionLoosePFIso06, recExpressionTaNCmedium, binningTauIdDiscr,
    ##                   "./plots/byTaNCmediumDiscr_afterLoosePFIso06.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionAntiLoosePFIso06, recExpressionTaNCmedium, binningTauIdDiscr,
    ##                   "./plots/byTaNCmediumDiscr_afterAntiLoosePFIso06.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionBeforeTauId, recExpressionTaNCtight, binningTauIdDiscr,
    ##                   "./plots/byTaNCtightDiscr_beforeLoosePFIso.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionLoosePFIso04, recExpressionTaNCtight, binningTauIdDiscr,
    ##                   "./plots/byTaNCtightDiscr_afterLoosePFIso04.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionAntiLoosePFIso04, recExpressionTaNCtight, binningTauIdDiscr,
    ##                   "./plots/byTaNCtightDiscr_afterAntiLoosePFIso04.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionLoosePFIso06, recExpressionTaNCtight, binningTauIdDiscr,
    ##                   "./plots/byTaNCtightDiscr_afterLoosePFIso06.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionAntiLoosePFIso06, recExpressionTaNCtight, binningTauIdDiscr,
    ##                   "./plots/byTaNCtightDiscr_afterAntiLoosePFIso06.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionBeforeTauId, recExpressionHPSloose, binningTauIdDiscr,
    ##                   "./plots/byHPSlooseDiscr_beforeLoosePFIso.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionLoosePFIso04, recExpressionHPSloose, binningTauIdDiscr,
    ##                   "./plots/byHPSlooseDiscr_afterLoosePFIso04.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionAntiLoosePFIso04, recExpressionHPSloose, binningTauIdDiscr,
    ##                   "./plots/byHPSlooseDiscr_afterAntiLoosePFIso04.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionLoosePFIso06, recExpressionHPSloose, binningTauIdDiscr,
    ##                   "./plots/byHPSlooseDiscr_afterLoosePFIso06.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionAntiLoosePFIso06, recExpressionHPSloose, binningTauIdDiscr,
    ##                   "./plots/byHPSlooseDiscr_afterAntiLoosePFIso06.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionBeforeTauId, recExpressionHPSmedium, binningTauIdDiscr,
    ##                   "./plots/byHPSmediumDiscr_beforeLoosePFIso.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionLoosePFIso04, recExpressionHPSmedium, binningTauIdDiscr,
    ##                   "./plots/byHPSmediumDiscr_afterLoosePFIso04.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionAntiLoosePFIso04, recExpressionHPSmedium, binningTauIdDiscr,
    ##                   "./plots/byHPSmediumDiscr_afterAntiLoosePFIso04.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionLoosePFIso06, recExpressionHPSmedium, binningTauIdDiscr,
    ##                   "./plots/byHPSmediumDiscr_afterLoosePFIso06.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionAntiLoosePFIso06, recExpressionHPSmedium, binningTauIdDiscr,
    ##                   "./plots/byHPSmediumDiscr_afterAntiLoosePFIso06.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionBeforeTauId, recExpressionHPStight, binningTauIdDiscr,
    ##                   "./plots/byHPStightDiscr_beforeLoosePFIso.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionLoosePFIso04, recExpressionHPStight, binningTauIdDiscr,
    ##                   "./plots/byHPStightDiscr_afterLoosePFIso04.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionAntiLoosePFIso04, recExpressionHPStight, binningTauIdDiscr,
    ##                   "./plots/byHPStightDiscr_afterAntiLoosePFIso04.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionLoosePFIso06, recExpressionHPStight, binningTauIdDiscr,
    ##                   "./plots/byHPStightDiscr_afterLoosePFIso06.png")
    ##makeLoosePFIsoPlot(ztt_events,
    ##                   None, None, genSelection_had,
    ##                   recSelectionAntiLoosePFIso06, recExpressionHPStight, binningTauIdDiscr,
    ##                   "./plots/byHPStightDiscr_afterAntiLoosePFIso06.png")
    
