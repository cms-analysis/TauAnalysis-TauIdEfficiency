import FWCore.ParameterSet.Config as cms
import copy

from PhysicsTools.PatAlgos.tools.tauTools import *
from PhysicsTools.PatAlgos.tools.jetTools import *

from TauAnalysis.Configuration.tools.metTools import *

from TauAnalysis.TauIdEfficiency.tools.sequenceBuilder import buildDijetTauSequence

def configurePatTupleProduction(process):

    #--------------------------------------------------------------------------------
    # produce PAT objects
    #--------------------------------------------------------------------------------

    process.load("PhysicsTools.PatAlgos.patSequences_cff")
    process.load("PhysicsTools.PatAlgos.producersLayer1.tauProducer_cff")
    process.load("PhysicsTools.PatAlgos.producersLayer1.muonProducer_cff")
    process.load("PhysicsTools.PatAlgos.producersLayer1.metProducer_cff")
    process.load("TauAnalysis.CandidateTools.muTauPairProduction_cff")

    process.load("TauAnalysis.TauIdEfficiency.patConfiguration.matchingPrototypes")
    patCaloTauMatchProtoType = copy.deepcopy(process.dijetCleanerPrototype)
    patCaloTauMatchProtoType.checkOverlaps.TagJet.src = cms.InputTag("caloJetsTagAndProbes", "tagObject")
    patCaloTauMatchProtoType.checkOverlaps.HighestPtProbeJet.src = cms.InputTag("caloJetsTagAndProbes", "highestPtProbe")
    patCaloTauMatchProtoType.checkOverlaps.SecondHighestPtProbe.src = cms.InputTag("caloJetsTagAndProbes", "secondHighestPtProbe")

    patPFTauMatchProtoType = copy.deepcopy(process.dijetCleanerPrototype)
    patPFTauMatchProtoType.checkOverlaps.TagJet.src = cms.InputTag("pfJetsTagAndProbes", "tagObject")
    patPFTauMatchProtoType.checkOverlaps.HighestPtProbeJet.src = cms.InputTag("pfJetsTagAndProbes", "highestPtProbe")
    patPFTauMatchProtoType.checkOverlaps.SecondHighestPtProbe.src = cms.InputTag("pfJetsTagAndProbes", "secondHighestPtProbe")

    #-------------------------------------------------------------------------------- 
    #
    # produce combinations of muon + tau-jet pairs
    # for collection of pat::Tau objects representing CaloTaus 
    #
    process.caloTauMatch = process.tauMatch.clone(
        src = cms.InputTag("caloRecoTauProducer")
    )
    process.caloTauGenJetMatch = process.tauGenJetMatch.clone(
        src = cms.InputTag("caloRecoTauProducer")
    )
    switchToCaloTau(process)
    patCaloTauProducerProtoType = process.patTaus.clone(
        genParticleMatch = cms.InputTag("caloTauMatch"),
        genJetMatch      = cms.InputTag("caloTauGenJetMatch")
    )
    process.buildCaloTaus = buildDijetTauSequence(
        process,
        collectionName = "caloTau",
        producerProtoType = patCaloTauProducerProtoType,
        cleanerProtoType = patCaloTauMatchProtoType,
        cleanerOverlapCheckerSource = "pfJetsTagAndProbe"
    )
    process.caloTauSequence = cms.Sequence(
        process.caloTauMatch + process.caloTauGenJetMatch
       + process.buildCaloTaus
    )

    process.patMuonCaloTauPairs = process.allMuTauPairs.clone(
        srcLeg1 = cms.InputTag('patMuons'),
        srcLeg2 = cms.InputTag('patCaloTaus'),
        srcMET = cms.InputTag('patMETs')
    )
    #--------------------------------------------------------------------------------

    #--------------------------------------------------------------------------------
    #
    # produce collection of pat::Tau objects representing PFTaus
    # reconstructed by fixed signal cone algorithm
    # (plus combinations of muon + tau-jet pairs)
    #
    process.pfTauMatchFixedCone = process.tauMatch.clone(
        src = cms.InputTag("fixedConePFTauProducer")
    )
    process.pfTauGenJetMatchFixedCone = process.tauGenJetMatch.clone(
        src = cms.InputTag("fixedConePFTauProducer")
    )
    switchToPFTauFixedCone(process)
    patPFTauProducerProtoTypeFixedCone = process.patTaus.clone(
        genParticleMatch = cms.InputTag("pfTauMatchFixedCone"),
        genJetMatch      = cms.InputTag("pfTauGenJetMatchFixedCone")
    )
    process.buildPFTausFixedCone = buildDijetTauSequence(
        process,
        collectionName = "pfTauFixedCone",
        producerProtoType = patPFTauProducerProtoTypeFixedCone,
        cleanerProtoType = patPFTauMatchProtoType,
        cleanerOverlapCheckerSource = "pfJetsTagAndProbe"
    )
    process.pfTauSequenceFixedCone = cms.Sequence(
        process.pfTauMatchFixedCone + process.pfTauGenJetMatchFixedCone
       + process.buildPFTausFixedCone
    )
    
    process.patMuonPFTauPairsFixedCone = process.allMuTauPairs.clone(
        srcLeg1 = cms.InputTag('patMuons'),
        srcLeg2 = cms.InputTag('patPFTausFixedCone'),
        srcMET = cms.InputTag('patPFMETs')
    )
    #--------------------------------------------------------------------------------

    #--------------------------------------------------------------------------------
    #
    # produce collection of pat::Tau objects representing PFTaus
    # reconstructed by shrinking signal cone algorithm
    # (plus combinations of muon + tau-jet pairs) 
    #
    process.pfTauMatchShrinkingCone = process.tauMatch.clone(
        src = cms.InputTag("shrinkingConePFTauProducer")
    )
    process.pfTauGenJetMatchShrinkingCone = process.tauGenJetMatch.clone(
        src = cms.InputTag("shrinkingConePFTauProducer")
    )
    switchToPFTauShrinkingCone(process)
    patPFTauProducerProtoTypeShrinkingCone = process.patTaus.clone(
        genParticleMatch = cms.InputTag("pfTauMatchShrinkingCone"),
        genJetMatch      = cms.InputTag("pfTauGenJetMatchShrinkingCone")
    )
    process.buildPFTausShrinkingCone = buildDijetTauSequence(
        process,
        collectionName = "pfTauShrinkingCone",
        producerProtoType = patPFTauProducerProtoTypeShrinkingCone,
        cleanerProtoType = patPFTauMatchProtoType,
        cleanerOverlapCheckerSource = "pfJetsTagAndProbe"
    )
    process.pfTauSequenceShrinkingCone = cms.Sequence(
        process.pfTauMatchShrinkingCone + process.pfTauGenJetMatchShrinkingCone
       + process.buildPFTausShrinkingCone
    )

    process.patMuonPFTauPairsShrinkingCone = process.allMuTauPairs.clone(
        srcLeg1 = cms.InputTag('patMuons'),
        srcLeg2 = cms.InputTag('patPFTausShrinkingCone'),
        srcMET = cms.InputTag('patPFMETs')
    )
    #--------------------------------------------------------------------------------

    #--------------------------------------------------------------------------------
    #
    # produce collection of pat::Tau objects representing PFTaus
    # reconstructed by hadron + strips (HPS) algorithm
    # (plus combinations of muon + tau-jet pairs) 
    #
    process.pfTauMatchHPS = process.tauMatch.clone(
        src = cms.InputTag("hpsPFTauProducer")
    )
    process.pfTauGenJetMatchHPS = process.tauGenJetMatch.clone(
        src = cms.InputTag("hpsPFTauProducer")
    )
    switchToPFTauHPS(process)
    patPFTauProducerProtoTypeHPS = process.patTaus.clone(
        genParticleMatch = cms.InputTag("pfTauMatchHPS"),
        genJetMatch      = cms.InputTag("pfTauGenJetMatchHPS")
    )
    process.buildPFTausHPS = buildDijetTauSequence(
        process,
        collectionName = "pfTauHPS",
        producerProtoType = patPFTauProducerProtoTypeHPS,
        cleanerProtoType = patPFTauMatchProtoType,
        cleanerOverlapCheckerSource = "pfJetsTagAndProbe"
    )
    process.pfTauSequenceHPS = cms.Sequence(
        process.pfTauMatchHPS + process.pfTauGenJetMatchHPS
       + process.buildPFTausHPS
    )

    process.patMuonPFTauPairsHPS = process.allMuTauPairs.clone(
        srcLeg1 = cms.InputTag('patMuons'),
        srcLeg2 = cms.InputTag('patPFTausHPS'),
        srcMET = cms.InputTag('patPFMETs')
    )
    #--------------------------------------------------------------------------------

    #--------------------------------------------------------------------------------
    # replace caloJets by pfJets
    switchJetCollection(process, cms.InputTag("iterativeCone5PFJets"))
    #--------------------------------------------------------------------------------

    #--------------------------------------------------------------------------------
    # add pfMET
    # set Boolean swich to true in order to apply type-1 corrections
    addPFMet(process, correct = False)
    #--------------------------------------------------------------------------------

    process.patTupleProductionSequence = cms.Sequence(
        process.patDefaultSequence
       ##+ process.caloTauSequence
       + process.pfTauSequenceFixedCone + process.pfTauSequenceShrinkingCone ##+ process.pfTauSequenceHPS
       ##+ process.patMuonCaloTauPairs
       ##+ process.patMuonPFTauPairsFixedCone + process.patMuonPFTauPairsShrinkingCone ##+ process.patMuonPFTauPairsHPS
    )
