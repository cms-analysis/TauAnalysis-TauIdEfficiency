from TauAnalysis.TauIdEfficiency.ntauples.sample_builder import build_sample
import os

'''

samples.py

Central defintion of data sources for commissioning.

$Id: samples.py,v 1.23 2010/11/18 11:12:28 friis Exp $

'''

# Map the complicated PFTau EDProducer names to something more managable.  These
# are the strings that can/should now be used to retrieve the ntuple from the
# ntuple manager.  Other samples (W+jets etc) can rename their tau collections
# to match these.
dijetSampleAliasMap = {
    'patPFTausDijetTagAndProbeHPS': 'hps',
    'patPFTausDijetTagAndProbeShrinkingCone': 'shrinking',
    'patPFTausDijetTagAndProbeFixedCone': 'fixed',
}

muEnrichedSampleAliasMap = {
    'patPFTausCleanedHPS': 'hps',
    'patPFTausCleanedShrinkingCone': 'shrinking',
    'patPFTausCleanedFixedCone': 'fixed',
}

wJetsSampleAliasMap = {
    'patPFTausCleanedHPS': 'hps',
    'patPFTausCleanedShrinkingCone': 'shrinking',
    'patPFTausCleanedFixedCone': 'fixed',
}

# Locations of the luminosity maps
_DATA_LUMI_MAP_FILE = os.path.join(
    os.environ['CMSSW_BASE'], 'src/TauAnalysis/TauIdEfficiency/test/'
    'commissioning', 'dataLumiMap.json')

_MC_LUMI_MAP_FILE = os.path.join(
    os.environ['CMSSW_BASE'], 'src/TauAnalysis/TauIdEfficiency/test/'
    'commissioning', 'mcLumiMap.json')

# Arugments: lumi map file, name of output collection, merge/add, list of samples
# to take from the JSON file
print "loading definition of Ztautau signal Monte Carlo samples..."
## CV: sample has not finished processing yet (11/12/10)
## EK: enabling it anyway!
ztautau_mc = build_sample(
    _MC_LUMI_MAP_FILE, "mc_ztautau",
    "merge", datasets = ["Ztautau"],
    alias_map=dijetSampleAliasMap)
#ztautau_mc = None
zttPU156bx_mc = build_sample(
    _MC_LUMI_MAP_FILE, "mc_zttPU156bx",
    "merge", datasets = ["ZtautauPU156bx"],
    alias_map=dijetSampleAliasMap)
zttPU156bxPFnoPileUp_mc = build_sample(
    _MC_LUMI_MAP_FILE, "mc_zttPU156bxPFnoPileUp",
    "merge", datasets = ["ZtautauPU156bxPFnoPileUp"],
    alias_map=dijetSampleAliasMap)

print "loading definition of QCD background Monte Carlo samples..."
qcddijet_mc = build_sample(
    _MC_LUMI_MAP_FILE, "mc_qcddijet", "merge",
    take_every=1, datasets = [
      "mcQCDdiJetPtHat15to30", "mcQCDdiJetPtHat30to50", "mcQCDdiJetPtHat50to80"
    ],
    alias_map = dijetSampleAliasMap)

## CV: sample has not finished processing yet (11/12/10)
ppmux_mc = build_sample(
   _MC_LUMI_MAP_FILE, "mc_ppmux", "merge",
   take_every=1, datasets = ["mcQCDppMuXPtHatGt20PtMuGt10"],
   alias_map = muEnrichedSampleAliasMap)
#ppmux_mc = None

print "loading definition of W + jets background Monte Carlo samples..."
## CV: sample has not finished processing yet (11/12/10)
wmunu_mc = build_sample(
   _MC_LUMI_MAP_FILE, "mc_wmunu", "merge",
   take_every=1, datasets = ["mcWtoMuNu"],
   alias_map = wJetsSampleAliasMap)
#wmunu_mc = None

# For data, we use the add mode, to concatenate data
print "loading definition of Data samples..."
data_dijet_runs132440to135802 = build_sample(
    _DATA_LUMI_MAP_FILE, "qcdDiJet_data_runs132440to135802", "add",
    take_every=1, datasets = ["qcdDiJet_data_runs132440_135802"],
    alias_map = dijetSampleAliasMap)
data_dijet_runs135821to141887 = build_sample(
    _DATA_LUMI_MAP_FILE, "qcdDiJet_data_runs135821to141887", "add",
    take_every=1, datasets = ["qcdDiJet_data_runs135821_141887"],
    alias_map = dijetSampleAliasMap)
data_dijet_runs141950to144114 = build_sample(
    _DATA_LUMI_MAP_FILE, "qcdDiJet_data_runs141950to144114", "add",
    take_every=1, datasets = ["qcdDiJet_data_runs141950_144114"],
    alias_map = dijetSampleAliasMap)
data_dijet = build_sample(
    _DATA_LUMI_MAP_FILE, "qcdDiJet_data", "add",
    take_every=1, datasets = [
      "qcdDiJet_data_runs132440_135802", "qcdDiJet_data_runs135821_141887", "qcdDiJet_data_runs141950_144114"
    ],
    alias_map = dijetSampleAliasMap)

data_ppmux_runs132440to145761 = build_sample(
    _DATA_LUMI_MAP_FILE, "qcdMuEnriched_data_runs132440to145761", "add",
    take_every=1, datasets = ["qcdMuEnriched_data_runs132440_145761"],
    alias_map = muEnrichedSampleAliasMap)
data_ppmux_runs145762_147116 = build_sample(
    _DATA_LUMI_MAP_FILE, "qcdMuEnriched_data_runs145762to147116", "add",
    take_every=1, datasets = ["qcdMuEnriched_data_runs145762_147116"],
    alias_map = muEnrichedSampleAliasMap)
data_ppmux_runs147117_149442 = build_sample(
    _DATA_LUMI_MAP_FILE, "qcdMuEnriched_data_runs147117to149442", "add",
    take_every=1, datasets = ["qcdMuEnriched_data_runs147117_149442"],
    alias_map = muEnrichedSampleAliasMap)
data_ppmux = build_sample(
    _DATA_LUMI_MAP_FILE, "qcdMuEnriched_data", "add",
    take_every=1, datasets = [
      "qcdMuEnriched_data_runs132440_145761", "qcdMuEnriched_data_runs145762_147116", "qcdMuEnriched_data_runs147117_149442"
    ],
    alias_map = muEnrichedSampleAliasMap)

data_wjets_runs132440to145761 = build_sample(
    _DATA_LUMI_MAP_FILE, "wJets_data_runs132440to145761", "add",
    take_every=1, datasets = ["wJets_data_runs132440_145761"],
    alias_map = wJetsSampleAliasMap)
data_wjets_runs145762_147116 = build_sample(
    _DATA_LUMI_MAP_FILE, "wJets_data_runs145762to147116", "add",
    take_every=1, datasets = ["wJets_data_runs145762_147116"],
    alias_map = wJetsSampleAliasMap)
data_wjets_runs147117_149442 = build_sample(
    _DATA_LUMI_MAP_FILE, "wJets_data_runs147117to149442", "add",
    take_every=1, datasets = ["wJets_data_runs147117_149442"],
    alias_map = wJetsSampleAliasMap)
data_wjets = build_sample(
    _DATA_LUMI_MAP_FILE, "wJets_data", "add",
    take_every=1, datasets = [
      "wJets_data_runs132440_145761", "wJets_data_runs145762_147116", "wJets_data_runs147117_149442"
    ],
    alias_map = wJetsSampleAliasMap)
