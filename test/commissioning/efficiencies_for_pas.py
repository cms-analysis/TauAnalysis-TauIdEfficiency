#!/usr/bin/env python

'''
Plot efficiency of different tau id algorithms for ZTT hadronic taus

Used to make plots for PFT-10-004

Authors: Aruna Nayak, Evan K. Friis

'''

import ROOT
import samples_cache as samples
from TauAnalysis.TauIdEfficiency.ntauples.plotting import draw, efficiencyLogHack
import TauAnalysis.TauIdEfficiency.ntauples.styles as style
import sys
import copy

# Define labels for different algorithms
algo_label_template = ROOT.TPaveText(0.50, 0.14, 0.88, 0.18, "NDC")
algo_label_template.SetTextAlign(33)
algo_label_template.SetTextSize(0.035)
algo_label_template.SetTextColor(2)
algo_label_template.SetFillStyle(0)
algo_label_template.SetBorderSize(0)

algo_label_fixed = copy.deepcopy(algo_label_template)
algo_label_fixed.AddText("Fixed Cone")

algo_label_shrinking = copy.deepcopy(algo_label_template)
algo_label_shrinking.AddText("Shrinking Cone")

algo_label_tanc = copy.deepcopy(algo_label_template)
algo_label_tanc.AddText("TaNC")

algo_label_hps = copy.deepcopy(algo_label_template)
algo_label_hps.AddText("HPS")

algo_label_calo = copy.deepcopy(algo_label_template)
algo_label_calo.AddText("TCTau")

maxNumEntries = 1000000000 # draw all evevnts
#maxNumEntries = 5         # to be used for debugging purposes only

def drawEfficiency(graph, isFirst):

    if isFirst:
        graph.Draw("Ae1p")
        graph.GetHistogram().GetXaxis().SetTitle(x_var_info['label'])
        graph.GetHistogram().GetYaxis().SetTitle("Efficiency")
        graph.GetHistogram().GetYaxis().SetRangeUser(0., 1.5)
    else:
        graph.Draw("e1p, same")

def makeEfficiencyComparisonPlots(numerators):

    # Make comparison plot of efficiencies after all tau id. criteria are applied
    # for different algorithms
    for x_var, x_var_info in parameterizations.iteritems():

        canvas.Clear()

        # build legend
        legend = ROOT.TLegend(0.45, 0.68, 0.88, 0.88, "","brNDC")
        legend.SetTextSize(0.03)
        legend.SetFillColor(0)
        legend.SetLineColor(1)
        legend.SetBorderSize(1)

        isFirst = True

        if numerators['fixedCone'] != "":
            numerator_fixed = numerators['fixedCone']
            efficiency_fixed = efficiency_results[x_var]['fixedCone'][numerator_fixed]
            efficiency_fixed.SetMarkerStyle(20)
            efficiency_fixed.SetMarkerColor(ROOT.EColor.kGreen + 2)
            efficiency_fixed.SetLineColor(ROOT.EColor.kGreen + 2)
            drawEfficiency(efficiency_fixed, isFirst)
            isFirst = False
            legend.AddEntry(efficiency_fixed, "Fixed signal cone", "P")

        if numerators['shrinkingCone'] != "":
            numerator_shrinking = numerators['shrinkingCone']
            efficiency_shrinking = efficiency_results[x_var]['shrinkingCone'][numerator_shrinking]
            efficiency_shrinking.SetMarkerStyle(21)
            efficiency_shrinking.SetMarkerColor(ROOT.EColor.kRed)
            efficiency_shrinking.SetLineColor(ROOT.EColor.kRed)
            drawEfficiency(efficiency_shrinking, isFirst)
            isFirst = False
            legend.AddEntry(efficiency_shrinking, "Shrinking signal cone", "P")

        if numerators['TaNC'] != "":
            numerator_tanc = numerators['TaNC']
            efficiency_tanc = efficiency_results[x_var]['TaNC'][numerator_tanc]
            efficiency_tanc.SetMarkerStyle(22)
            efficiency_tanc.SetMarkerColor(ROOT.EColor.kBlue)
            efficiency_tanc.SetLineColor(ROOT.EColor.kBlue)
            drawEfficiency(efficiency_tanc, isFirst)
            isFirst = False
            legend.AddEntry(efficiency_tanc, "TaNC medium", "P")

        if numerators['hps'] != "":
            numerator_hps = numerators['hps']
            efficiency_hps = efficiency_results[x_var]['hps'][numerator_hps]
            efficiency_hps.SetMarkerStyle(23)
            efficiency_hps.SetMarkerColor(28)
            efficiency_hps.SetLineColor(28)
            drawEfficiency(efficiency_hps, isFirst)
            isFirst = False
            legend.AddEntry(efficiency_hps, "HPS medium", "P")

        if numerators['calo'] != "":
            numerator_tctau = numerators['calo']
            efficiency_tctau = efficiency_results[x_var]['calo'][numerator_tctau]
            efficiency_tctau.SetMarkerStyle(29)
            efficiency_tctau.SetMarkerColor(ROOT.EColor.kBlack)
            efficiency_tctau.SetLineColor(ROOT.EColor.kBlack)
            drawEfficiency(efficiency_tctau, isFirst)
            legend.AddEntry(efficiency_tctau, "TCTau", "P")

        # Draw legend
        legend.Draw()

        # Draw the preliminary label
        style.CMS_PRELIMINARY_UPPER_LEFT.Draw()
        style.ZTAUTAU_LABEL_UPPER_LEFT.Draw()
        style.SQRTS_LABEL_UPPER_LEFT.Draw()
        x_var_info["cutLabel"].Draw()

        # Save the plot
        canvas.SaveAs("plots/%s.png" % '_'.join(['efficiency_algo_comparison', 'vs', x_var]))
        canvas.SaveAs("plots/%s.pdf" % '_'.join(['efficiency_algo_comparison', 'vs', x_var]))

if __name__ == "__main__":
    ROOT.gROOT.SetBatch(True)
    ROOT.gROOT.SetStyle("Plain")
    ROOT.gStyle.SetOptStat(0)

    # Get ZTT samples
    ##ztt = samples.ztautau_mc
    ##ztt = samples.zttPU156bx_mc
    ztt = samples.zttPOWHEGz2_mc 

    # Get the ntuple we produced
    ntuple_manager = ztt.build_ntuple_manager("tauIdEffNtuple")

    # Get generator level tau ntuple
    genTaus = ntuple_manager.get_ntuple("tauGenJets")

    # Binning for PT
    ##pt_bins = (
    ##    0, 2.5, 5., 7.5, 10, 12.5, 15, 17.5, 20, 22.5, 25, 27.5, 30, 32.5,
    ##    35, 37.5, 40, 42.5, 45, 47.5, 50, 52.5, 55, 57.5, 60, 63, 66, 69, 72,
    ##    76, 80, 85, 90, 95, 100, 110, 120, 130, 140, 150
    ##)
    pt_bins = ( 0, 10, 15, 20, 25, 30, 40, 60, 80, 100, 150 )

    # Define styles of all of the numerators
    numerators = {
        'matching' : {
            'expr_str': '1',
            'label' : "Matched",
        },
        'byLeadTrackFinding': {
            'expr_str': '$byLeadTrackFinding',
            'label' : "Lead Track Finding"
        },
        'byLeadTrackPtCut': {
            'expr_str': '$byLeadTrackPtCut',
            'label' : "Lead Track P_{T} Cut"
        },
        'byTrackIsolation': {
            'expr_str': '$byTrackIsolation',
            'label' : "Charged Hadron Isolation"
        },
        'byEcalIsolation': {
            'expr_str': '$byEcalIsolation',
            'label' : "Photon Isolation"
        },
        # For calotau
        'byIsolation' : {
            'expr_str': '$byIsolation',
            'label' : 'Track Isolation'
        },
        'OneOrThreeProng': {
            'expr_str': '$numChargedParticlesSignalCone == 1 || $numChargedParticlesSignalCone == 3',
            'label' : "1 or 3 Prong"
        },
        'byEcalIsolation_calo' : {
            'expr_str': '$etSumIsolationECAL < 5',
            'label' : 'ECAL Isolation'
        },
        'OneOrThreeProng_calo': {
            'expr_str': '$numSignalTracks ==1 || $numSignalTracks ==3',
            'label' : "1 or 3 Prong"
        },
        # For TaNC
        'byTaNCfrOnePercent': {
            'expr_str': '$byTaNCfrOnePercent',
            'label' : "TaNC 1.0%"
        },
        'byTaNCfrHalfPercent': {
            'expr_str': '$byTaNCfrHalfPercent',
            'label' : "TaNC 0.5%"
        },
        'byTaNCfrQuarterPercent': {
            'expr_str': '$byTaNCfrQuarterPercent',
            'label' : "TaNC 0.25%"
        },
        # For TaNC
        'byTaNCloose': {
            'expr_str': '$byTaNCloose',
            'label' : "TaNC loose"
        },
        'byTaNCmedium': {
            'expr_str': '$byTaNCmedium',
            'label' : "TaNC medium"
        },
        'byTaNCtight': {
            'expr_str': '$byTaNCtight',
            'label' : "TaNC tight"
        },
        # For HPS
        'byDecayModeFinding': {
            'expr_str': '$byDecayMode',
            'label' : "Decay Mode Finding"
        },
        'byHPSloose' : {
            'expr_str': '$byHPSloose',
            'label': "HPS loose"
        },
        'byHPSmedium' : {
            'expr_str': '$byHPSmedium',
            'label': "HPS medium"
        },
        'byHPStight' : {
            'expr_str': '$byHPStight',
            'label': "HPS tight"
        },
        'byLooseIsolation' : {
            'expr_str': '$byIsolationLoose',
            'label': "HPS loose"
        },
        'byMediumIsolation' : {
            'expr_str': '$byIsolationMedium',
            'label': "HPS medium"
        },
        'byTightIsolation' : {
            'expr_str': '$byIsolationTight',
            'label': "HPS tight"
        },
    }

    parameterizations = {
        'pt' : {
            'expr_str': '$genPt',
            'binning': pt_bins,
            'label': 'Generated #tau visible P_{T} [GeV/c]',
            'cutLabel': style.ETAVIS_CUT_LABEL_UPPER_LEFT,
        },
        'eta' : {
            'expr_str': '$genEta',
            ##'binning': (50, -2.5, 2.5),
            'binning': (10, -2.5, 2.5),
            'label': 'Generated #tau visible #eta',
            'cutLabel': style.PTVIS_CUT_LABEL_UPPER_LEFT,
        }
    }

    # Define 'orders' for tau discriminators
    # traditional tau ID sequence
    standard_sequence = [
        'matching',
        'byLeadTrackFinding',
        'byLeadTrackPtCut',
        #'byTrackIsolation',
        #'byEcalIsolation',
        #'OneOrThreeProng',
        'byTaNCfrOnePercent',
        'byTaNCfrHalfPercent',
        'byTaNCfrQuarterPercent',
        #'byTaNCfrTenthPercent',
    ]

    tanc_sequence = [
        'matching',
        'byLeadTrackFinding',
        'byLeadTrackPtCut',
        'byTaNCloose',
        'byTaNCmedium',
        'byTaNCtight'
    ]

    hps_sequence = [
        'matching',
        'byLeadTrackFinding',
        #'byDecayModeFinding',
        'byLooseIsolation',
        'byMediumIsolation',
        'byTightIsolation',
    ]

    calo_sequence = [
        'matching',
        'byLeadTrackFinding',
        'byLeadTrackPtCut',
        'byIsolation',
        'byEcalIsolation_calo',
        'OneOrThreeProng_calo'
   ]

    # Match up sequences to tau algos
    sequences_and_algos = [
        #( "shrinkingCone", standard_sequence ),
        #( "fixedCone", standard_sequence ),
        ( "TaNC", tanc_sequence ),
        #( "hps", hps_sequence ),
        #( "calo", calo_sequence )
    ]

    nTuples = {
        "shrinkingCone": "patPFTausDijetTagAndProbeShrinkingCone",
        "fixedCone": "patPFTausDijetTagAndProbeFixedCone",
        "TaNC": "patPFTausDijetTagAndProbeHPSpTaNC",
        #"hps": "patPFTausDijetTagAndProbeHPSpTaNC",
        "hps": "patPFTausDijetTagAndProbeHPS",
        "calo": "patCaloTausDijetTagAndProbe"
    }

    denom_selection = genTaus.expr('$genDecayMode > 1.5 && $genPt > 10 && abs($genEta) < 2.5')

    ##denom_selection_from_reco_str = '$genMatch > 0.5 && $genDecayMode > 1.5 && $genPt > 10 && abs($genEta) < 2.5'
    denom_selection_from_reco_str = '$genDecayMode > 1.5 && $genPt > 10 && abs($genEta) < 2.5'

    ztt_events = list(ztt.events_and_weights())[0][0]

    efficiency_results = {}

    extra_labels = {}
    extra_labels['fixedCone'] = [ algo_label_fixed, ]
    extra_labels['shrinkingCone'] = [ algo_label_shrinking, ]
    extra_labels['TaNC'] = [ algo_label_tanc, ]
    extra_labels['hps'] = [ algo_label_hps, ]
    extra_labels['calo'] = [ algo_label_calo, ]

    canvas = ROOT.TCanvas("canvas", "canvas", 500, 500)
    canvas.SetLeftMargin(0.11)
    canvas.SetGridy(1)

    for algorithm, sequence in sequences_and_algos:
        print "Plotting", algorithm
        # Get the ntuple
        ntuple = ntuple_manager.get_ntuple(nTuples[algorithm])
        # Loop over different horizontal axes
        for x_var, x_var_info in parameterizations.iteritems():

            # build legend
            legend = ROOT.TLegend(0.45, 0.68, 0.88, 0.88, "","brNDC")
            legend.SetTextSize(0.03)
            legend.SetFillColor(0);
            legend.SetLineColor(1);
            legend.SetBorderSize(1);

            # build denominator
            print "Building denominator for", x_var
            denominator = draw(
                ztt_events,
                expression = genTaus.expr(x_var_info['expr_str']),
                selection = denom_selection,
                binning = x_var_info['binning'],
                output_name = '_'.join(["denom", x_var, algorithm]),
                maxNumEntries = maxNumEntries
            )
            print "denominator: entries = %i" % denominator.GetEntries()

            # Loop over sequence of numerators
            numerator_effs = []
            running_cut = ntuple.expr(denom_selection_from_reco_str)
            for numerator_name in sequence:

                # Get the description of this numerator
                numerator_info = numerators[numerator_name]
                print "Building numerator for", numerator_name
                running_cut = ntuple.expr(numerator_info['expr_str']) & running_cut

                # Draw numerator
                print x_var_info['expr_str']
                print running_cut
                numerator = draw(
                    ztt_events,
                    expression = ntuple.expr(x_var_info['expr_str']),
                    selection = running_cut,
                    binning = x_var_info['binning'],
                    output_name = '_'.join([numerator_name, x_var, algorithm]),
                    maxNumEntries = maxNumEntries
                )
                print "numerator: entries = %i" % numerator.GetEntries()

                #raise ValueError("STOPped for debugging")

                # FIXME: clean this up
                from math import sqrt
                nNum = float(numerator.Integral())
                nDenom = denominator.Integral()
                err = 1/nDenom*sqrt(nNum*(1-nNum/nDenom) )
                efficiencyLogHack("%s -> %s: " % (nTuples[algorithm], numerator_name), timestamp=True)
                efficiencyLogHack("%e / %e = %e +- %e\n" % (nNum, nDenom, nNum/nDenom,err))
                # end cleanup

                my_eff = ROOT.TGraphAsymmErrors(numerator, denominator)

                # Overwrite the additional spacing by root
                if len(x_var_info['binning']) == 3:
                    my_eff.GetXaxis().SetRangeUser(x_var_info['binning'][1],
                                                   x_var_info['binning'][2])
                else:
                    my_eff.GetXaxis().SetRangeUser(min(x_var_info['binning']),
                                                   max(x_var_info['binning']))

                # Update style
                # CV: temporarily disabled
                #style.update_histo_style(
                #    my_eff, style.EFFICIENCY_STYLES[numerator_name])

                numerator_effs.append(my_eff)

                efficiency_results.setdefault(x_var, {})
                efficiency_results[x_var].setdefault(algorithm, {})
                efficiency_results[x_var][algorithm].setdefault(algorithm, {})
                efficiency_results[x_var][algorithm][numerator_name] = my_eff

                legend.AddEntry(my_eff, numerator_info['label'], "P")

            # Make the actual plots
            print "Building plots"
            for index, numerator_eff in enumerate(numerator_effs):
                # Plot the background on the first go-round
                if index == 0:
                    numerator_eff.Draw("Ap")
                    numerator_eff.GetHistogram().GetXaxis().SetTitle(x_var_info['label'])
                    numerator_eff.GetHistogram().GetYaxis().SetTitle("Efficiency")
                    numerator_eff.GetHistogram().GetYaxis().SetTitleOffset(1.2)
                    numerator_eff.GetHistogram().GetYaxis().SetRangeUser(0, 1.5)
                else:
                    numerator_eff.Draw("p, same")

            # Draw legend
            legend.Draw()

            # Draw the preliminary label
            style.CMS_PRELIMINARY_UPPER_LEFT.Draw()
            style.ZTAUTAU_LABEL_UPPER_LEFT.Draw()
            style.SQRTS_LABEL_UPPER_LEFT.Draw()
            x_var_info["cutLabel"].Draw()
            for extra_label in extra_labels[algorithm]:
                extra_label.Draw()

            # Save the plot
            canvas.SaveAs("plots/%s.png" % '_'.join([algorithm, "efficiency", 'vs', x_var]))
            canvas.SaveAs("plots/%s.pdf" % '_'.join([algorithm, "efficiency", 'vs', x_var]))

    # Make comparison plot of efficiencies after all tau id. criteria are applied
    # for different algorithms
    numerators = {}
    #numerators['fixedCone'] = ""
    numerators['shrinkingCone'] = 'OneOrThreeProng'
    #numerators['TaNC'] = 'byTaNCmedium'
    numerators['TaNC'] = 'byTaNCfrHalfPercent'
    #numerators['hps'] = 'byHPSmedium'
    numerators['hps'] = 'byMediumIsolation'
    #numerators['calo'] = 'OneOrThreeProng_calo'
    #makeEfficiencyComparisonPlots(numerators)




