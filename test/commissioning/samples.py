from TauAnalysis.TauIdEfficiency.ntauples.sample_builder import build_sample
import os

'''

samples.py

Central defintion of data sources for commissioning.

$Id: samples.py,v 1.30 2011/01/05 10:16:56 friis Exp $

'''

# Map the complicated PFTau EDProducer names to something more managable.  These
# are the strings that can/should now be used to retrieve the ntuple from the
# ntuple manager.  Other samples (W+jets etc) can rename their tau collections
# to match these.
dijetSampleAliasMap = {
    'patPFTausDijetTagAndProbeHPS': 'hps',
    'patPFTausDijetTagAndProbeShrinkingCone': 'shrinking',
    'patPFTausDijetTagAndProbeFixedCone': 'fixed',
    'patPFTausDijetTagAndProbeHPSpTaNC06': 'hpstanc',
    'patCaloTausDijetTagAndProbe': 'calo'
}

muEnrichedSampleAliasMap = {
    'patPFTausCleanedHPS': 'hps',
    'patPFTausCleanedHPSpTaNC06': 'hpstanc',
    'patPFTausCleanedShrinkingCone': 'shrinking',
    'patPFTausCleanedFixedCone': 'fixed',
    'patCaloTausCleaned': 'calo'
}

#wJetsSampleAliasMap = {
    #'patPFTausCleanedHPS': 'hps',
    #'patPFTausCleanedHPSpTaNC': 'hpstanc',
    #'patPFTausCleanedShrinkingCone': 'shrinking',
    #'patPFTausCleanedFixedCone': 'fixed',
    #'patCaloTausCleaned': 'calo'
#}

wJetsSampleAliasMap = {
    'patPFTausLoosePFIsoEmbeddedHPS': 'hps',
    'patPFTausLoosePFIsoEmbeddedHPSpTaNC06': 'hpstanc',
    'patPFTausLoosePFIsoEmbeddedShrinkingCone': 'shrinking',
    'patPFTausLoosePFIsoEmbeddedFixedCone': 'fixed',
    'patCaloTausLoosePFIsoEmbedded': 'calo'
}

muEnrichedSampleAliasMap = wJetsSampleAliasMap

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

# Build the hacked sample
ztautau_test = build_sample(
    _MC_LUMI_MAP_FILE, "mc_ztt_test",
    "merge", datasets = ["ZtautauTEST"],
    alias_map=dijetSampleAliasMap
)

print "loading definition of QCD background Monte Carlo samples..."
qcddijet_mc = build_sample(
    _MC_LUMI_MAP_FILE, "mc_qcddijet", "merge",
    take_every=1, datasets = [
      "mcQCDdiJetPtHat15to30", "mcQCDdiJetPtHat30to50", "mcQCDdiJetPtHat50to80"
    ],
    alias_map = dijetSampleAliasMap)

ppmux_mc = build_sample(
   _MC_LUMI_MAP_FILE, "mc_ppmux", "merge",
   take_every=1, datasets = ["mcQCDppMuXptHatGt20PtMuGt10"],
   alias_map = muEnrichedSampleAliasMap)
ppmuxPU156bx_mc = build_sample(
   _MC_LUMI_MAP_FILE, "mc_ppmuxPU156bx", "merge",
   take_every=1, datasets = ["mcQCDppMuXptHatGt20PtMuGt10PU156bx"],
   alias_map = muEnrichedSampleAliasMap)

print "loading definition of W + jets background Monte Carlo samples..."
wmunu_mc = build_sample(
   _MC_LUMI_MAP_FILE, "mc_wmunu", "merge",
   take_every=1, datasets = ["mcWtoMuNu"],
   alias_map = wJetsSampleAliasMap)
wmunuPU156bx_mc = build_sample(
   _MC_LUMI_MAP_FILE, "mc_wmunuPU156bx", "merge",
   take_every=1, datasets = ["mcWtoMuNuPU156bx"],
   alias_map = wJetsSampleAliasMap)

# For data, we use the add mode, to concatenate data
print "loading definition of Data samples..."
data_dijet = build_sample(
    _DATA_LUMI_MAP_FILE, "qcdDiJet_data", "add",
    take_every=1, datasets = [
        "qcdDiJet_data_MinBiasCommissioning_Jun14ReReco",
        "qcdDiJet_data_JetMETTau_Run2010Ai_Sep17ReReco",
        "qcdDiJet_data_JetMET_Run2010Aii_Sep17ReReco"
    ],
    alias_map = dijetSampleAliasMap)

data_ppmux = build_sample(
    _DATA_LUMI_MAP_FILE, "qcdMuEnriched_data", "add",
    take_every=1, datasets = [
        "qcdMuEnriched_data_Mu_Run2010A_Nov4ReReco",
        "qcdMuEnriched_data_Mu_Run2010B_Nov4ReReco"
    ],
    alias_map = muEnrichedSampleAliasMap)

data_wjets = build_sample(
    _DATA_LUMI_MAP_FILE, "wJets_data", "add",
    take_every=1, datasets = [
        "wJets_data_Mu_Run2010A_Nov4ReReco",
        "wJets_data_Mu_Run2010B_Nov4ReReco",
    ],
    alias_map = wJetsSampleAliasMap)
