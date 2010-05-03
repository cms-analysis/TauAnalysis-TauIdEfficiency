#!/usr/bin/env python

'''
Example of use of TauNtuple software

Author: Evan K. Friis, UC Davis
'''

import ROOT
from TauAnalysis.TauIdEfficiency.ntauples.TauNtupleManager import TauNtupleManager
import TauAnalysis.TauIdEfficiency.ntauples.plotting as plot


if __name__ == "__main__":
    file = ROOT.TFile("example_ntuple.root", "READ")
    
    # Get the events tree (this can also be a TChain)
    events = file.Get("Events")

    # Get the ntuple we produced
    manager = TauNtupleManager(events, "exampleNtuple")

    # Get the list of collections availabe for our ntuple
    print manager
    # returns
    #== TauNtupleManager
    #===  Available ntuples for label exampleNtuple
    #==== allLayer1Taus

    # So lets use allLayer1Taus
    allLayer1Taus = manager.get_ntuple("allLayer1Taus")

    # All this helper funciton does it makes it easy for use
    # to interface to TTree::Draw, etc.

    # Get list of variables available
    print allLayer1Taus
    # returns
    # Tau Ntuple - collection allLayer1Taus
    #  byIsolation
    #  byLeadPionPt
    #  byTaNCfrOne
    #  absEta
    #  pt
    
    # Now it easy to create expressions using the expr method
    print allLayer1Taus.expr('$pt')
    # returns
    # exampleNtuple#allLayer1Taus#pt

    # You can also build selection string, with operator overloading
    # or without.  The following are logically equivalent
    my_selection = allLayer1Taus.expr('$pt > 5 && $pt < 20')
    print my_selection

    # or
    my_first_selection = allLayer1Taus.expr('$pt') > 5 
    my_second_selection = allLayer1Taus.expr('$pt') < 20 
    # NB the notation for logical AND
    my_selection = my_first_selection & my_second_selection
    print my_selection

    # The following operators are available
    # + - * / & | < > <= >=

    # There are also tools to make plots.  Here is an example to draw the Pt spectrum
    # for taus in the barrel 
    canvas = ROOT.TCanvas("example", "example", 500, 500)

    pt_hist = plot.draw(expression=allLayer1Taus.expr('$pt'), 
                        selection=allLayer1Taus.expr('$absEta < 1.0'),
                        binning=(10, 0, 50))

    # pt_hist is a TH1F
    pt_hist.Draw()
    canvas.SaveAs("pt_spectrum.png")

    # We can easily make an efficiency plot as well
    # Let's compute the TaNC eff w.r.t Leading Piont for taus with Pt > 5 
    # with eta < 2.5
    denom_selection = allLayer1Taus.expr('$pt > 5') & \
            allLayer1Taus.expr('$absEta < 2.5') & \
            allLayer1Taus.expr('$byLeadPionPt > 0.5')

    numerator_selection = denom_selection & allLayer1Taus.expr('$byTaNCfrOne')

    # The efficiency function returns a tuple with
    # a histo background + a TGraph asymmerrors
    bkg_histo, efficiency = plot.efficiency(
        expression=allLayer1Taus.expr('$pt'),
        numerator=numerator_selection,
        denominator=denom_selection,
        binning=(10, 0, 50))

    bkg_histo.Draw()
    efficiency.Draw("s, p")

    canvas.SaveAs("pt_eff.png")







