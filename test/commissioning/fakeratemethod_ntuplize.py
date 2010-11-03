#!/usr/bin/env python

'''

Select events and train a TMVA k-Nearest Neighboor classier using the fake rate
method.

Authors: Evan K. Friis, Christian Veelken (UC Davis)

'''

# cheating
from RecoLuminosity.LumiDB import argparse

parser = argparse.ArgumentParser(
    description = "Build an output ntuple of jetPt, eta and width"
    " given an algorithm and numerator/denominator selection."
)
parser.add_argument('--ntuple', help="Name of ntuple")
parser.add_argument('--sample', help="Name of sample")
parser.add_argument('--num', help="Numerator")
parser.add_argument('--den', help="Denominator")
parser.add_argument('--hlt', help="HLT ntuple selection")
parser.add_argument('-passing', action='store_true', default=False,
                    help="Build ntuple for events that pass")
parser.add_argument('-failing', action='store_true', default=False,
                    help="Build ntuple for events that fail")
parser.add_argument('--output', help="Output file")

options=parser.parse_args()

import sys
# Otherwise ROOT will try to parse the options as well Jeeze Louise
sys.argv = []
import ROOT
ROOT.gROOT.SetBatch(True)

# Definition of input files.
import samples_cache as samples

ntuple_manager = getattr(
    samples, options.sample).build_ntuple_manager("tauIdEffNtuple")

# Get our specific ntuple
ntuple = ntuple_manager.get_ntuple(options.ntuple)

# Get the HLT ntuple
hlt = ntuple_manager.get_ntuple("patTriggerEvent")

# Build the denominator query
denominator = hlt.expr(options.hlt) & \
        ntuple.expr(options.den)

# Build the queries for passing and failing
passing = ntuple.expr(options.num) & denominator
failing = ntuple.expr(options.num).false() & denominator

draw_string = ntuple.expr('$jetWidth:$jetEta:$jetPt')

output_file = ROOT.TFile(options.output, "RECREATE")
output_file.cd()

events_list = list(samples.data.events_and_weights())

# WARNING: current method of filling k-NN tree using TPolyMarker3D objects
#          does not support event weights
#         --> need to make sure that all events have the same weights,
#             by requiring sample to either contain one set of files
#             or all sets to have the same weights in case sample contains multiple set of files
#         (Note that this means it is not possible to fill k-NN tree
#          with QCD Monte Carlo generated in different PtHat bins)
import TauAnalysis.TauIdEfficiency.ntauples.helpers as helpers
events = None
if len(events_list) == 1:
    events = events_list[0][0]
elif len(events_list) > 1:
    events = ROOT.TChain("Events")
    isFirst = True
    weight = 0.
    for entry in events_list:
        if isFirst:
            helpers.copy_aliases_from(entry[0], events)
            weight = entry[1]
            isFirst = False
        else:
            if weight != entry[1]:
                raise ValueError("Events with different weights not supported yet !!")
        print("--> adding %s events to TChain..." % entry[0].GetEntries())
        events.Add(entry[0])
if events is None:
    raise ValueError("Failed to access TChain !!")

print "Ntuple has entries: ", events.GetEntries()
# Prevent root from cutting us off at 10^6
events.SetEstimate(100000000L)

if options.passing:
    # Build a TPolyMaker3D with our points
    events.Draw(str(draw_string), str(passing))
    numerator_tuple = ROOT.gPad.GetPrimitive("TPolyMarker3D")
    numerator_tuple.SetName("passing")

    print "Built 'passing' pre-ntuple with %i entries" % \
            numerator_tuple.GetN()

    if numerator_tuple.GetN() == events.GetEstimate():
        raise ValueError("Number of entries produced is at the upper limit " \
                         "of TTree::Draw.  You need to set events.SetEstimate to " \
                         "be larger than the total number of selected rows.")
    numerator_tuple.Write()

if options.failing:
    # Build a TPolyMaker3D with our points
    drawn = events.Draw(str(draw_string), str(failing))
    denominator_tuple = ROOT.gPad.GetPrimitive("TPolyMarker3D")
    denominator_tuple.SetName("failing")
    print "Built 'failing' pre-ntuple with %i entries" % \
            denominator_tuple.GetN()
    if denominator_tuple.GetN() == events.GetEstimate():
        raise ValueError("Number of entries produced is at the upper limit " \
                         "of TTree::Draw.  You need to set events.SetEstimate to " \
                         "be larger than the total number of selected rows.")
    denominator_tuple.Write()

