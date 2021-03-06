import FWCore.ParameterSet.Config as cms

from TauAnalysis.TauIdEfficiency.filterDataQuality_cfi import *
from TauAnalysis.RecoTools.recoVertexSelection_cff import *

#--------------------------------------------------------------------------------
# select muon enriched QCD events
# from which to determine tau fake-rates
#--------------------------------------------------------------------------------

#--------------------------------------------------------------------------------
#
# define HLT trigger path
#
hltMu = cms.EDFilter("EventSelPluginFilter",
    selector = cms.PSet(
        pluginName = cms.string('hltMu'),             
        pluginType = cms.string('TriggerResultEventSelector'),
        src = cms.InputTag('TriggerResults::HLT'),
        triggerPaths = cms.vstring(
            'HLT_Mu17_v1', # Summer'12 MC, produced with CMSSW_5_2_x
            'HLT_Mu17_v3'  # 2012 Run A Data
        )
    )
)
#--------------------------------------------------------------------------------

#--------------------------------------------------------------------------------
#
# define selection of "loose" Muons
#
globalMuons = cms.EDFilter("PATMuonSelector",
    src = cms.InputTag('patMuons'),
    cut = cms.string("isGlobalMuon"),
    filter = cms.bool(False)
)

diMuonVeto = cms.EDFilter("PATCandViewCountFilter",
    src = cms.InputTag('globalMuons'),
    minNumber = cms.uint32(1),
    maxNumber = cms.uint32(1)
)
#
# define selection of "tight" Muons
#
selectedMuons = cms.EDFilter("PATMuonSelector",
    src = cms.InputTag('patMuons'),
    cut = cms.string("isGlobalMuon & pt > 18. & abs(eta) < 2.1"),
    filter = cms.bool(True)
)
#--------------------------------------------------------------------------------

#--------------------------------------------------------------------------------
#
# define loose CaloTau candidate/CaloJet selection
#
selectedCaloTaus = cms.EDFilter("CaloTauSelector",
    src = cms.InputTag('caloRecoTauProducer'),
    discriminators = cms.VPSet(),
    cut = cms.string("abs(caloTauTagInfoRef().jetRef().eta) < 2.5 & caloTauTagInfoRef().jetRef().pt > 10."),
    filter = cms.bool(True)
)

muonCaloTauPairs = cms.EDProducer("DiCandidatePairProducer",
    useLeadingTausOnly = cms.bool(False),
    srcLeg1 = cms.InputTag('selectedMuons'),
    srcLeg2 = cms.InputTag('selectedCaloTaus'),
    dRmin12 = cms.double(0.),
    srcMET = cms.InputTag('metJESCorAK5CaloJetMuons'),
    recoMode = cms.string(""),
    verbosity = cms.untracked.int32(0)                                       
)

selectedMuonCaloTauPairs = cms.EDFilter("DiCandidatePairSelector",
    src = cms.InputTag('muonCaloTauPairs'),
    cut = cms.string("dR12 > 0.7 & mt1MET < 40."),
    filter = cms.bool(True)                                     
)

muonCaloTauSkimPath = cms.Path(
    hltMu
   + globalMuons + diMuonVeto + selectedMuons
   + selectedCaloTaus + muonCaloTauPairs + selectedMuonCaloTauPairs
   + dataQualityFilters
)
#--------------------------------------------------------------------------------

#--------------------------------------------------------------------------------
#
# define loose PFTau candidate/PFJet selection
#
selectedPFTaus = cms.EDFilter("PFTauSelector",
    src = cms.InputTag('hpsPFTauProducer'),
    discriminators = cms.VPSet(),                          
    cut = cms.string("abs(jetRef().eta) < 2.5 & jetRef().pt > 10."),
    filter = cms.bool(True)
)

muonPFTauPairs = cms.EDProducer("DiCandidatePairProducer",
    useLeadingTausOnly = cms.bool(False),
    srcLeg1 = cms.InputTag('selectedMuons'),
    srcLeg2 = cms.InputTag('selectedPFTaus'),
    dRmin12 = cms.double(0.),
    srcMET = cms.InputTag('patType1CorrectedPFMet'),
    recoMode = cms.string(""),
    verbosity = cms.untracked.int32(0)                                       
)

selectedMuonPFTauPairs = cms.EDFilter("DiCandidatePairSelector",
    src = cms.InputTag('muonPFTauPairs'),
    cut = cms.string("dR12 > 0.7 & mt1MET < 40."),
    filter = cms.bool(True)                                     
)

muonPFTauSkimPath = cms.Path(    
    hltMu
   + globalMuons + diMuonVeto + selectedMuons
   + selectedPFTaus + muonPFTauPairs + selectedMuonPFTauPairs
   + dataQualityFilters
)
#--------------------------------------------------------------------------------

qcdMuEnrichedEventSelection = cms.untracked.PSet(
    SelectEvents = cms.untracked.PSet(
        SelectEvents = cms.vstring(
            ##'muonCaloTauSkimPath',
            'muonPFTauSkimPath'
        )
    )
)
