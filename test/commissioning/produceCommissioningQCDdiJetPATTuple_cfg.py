import FWCore.ParameterSet.Config as cms

process = cms.Process("prodCommissioningQDCdiJetPATtuple")

# import of standard configurations for RECOnstruction
# of electrons, muons and tau-jets with non-standard isolation cones
process.load('Configuration/StandardSequences/Services_cff')
process.load('FWCore/MessageService/MessageLogger_cfi')
process.MessageLogger.cerr.FwkReport.reportEvery = 100
#process.MessageLogger.cerr.threshold = cms.untracked.string('INFO')
#process.MessageLogger.suppressInfo = cms.untracked.vstring()
#process.MessageLogger.suppressWarning = cms.untracked.vstring("PATTriggerProducer",)
process.load('Configuration/StandardSequences/GeometryIdeal_cff')
process.load('Configuration/StandardSequences/MagneticField_cff')
process.load('Configuration/StandardSequences/FrontierConditions_GlobalTag_cff')

#--------------------------------------------------------------------------------
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        'file:/data2/friis/CMSSW_4_2_X/skims/06-27-MatthewsZTTEvents/crab_0_110627_082505/ZTTCands_merged_v1.root'
    ),
    skipEvents = cms.untracked.uint32(0)
)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(-1)
)

#--------------------------------------------------------------------------------
# define configuration parameter default values

isMC = True # use for MC
##isMC = False # use for Data
##HLTprocessName = "HLT" # use for 2011 Data
HLTprocessName = "HLT" # use for Summer'11 MC
pfCandidateCollection = "particleFlow" # pile-up removal disabled
##pfCandidateCollection = "pfNoPileUp" # pile-up removal enabled
applyEventSelection = True 
#--------------------------------------------------------------------------------

#--------------------------------------------------------------------------------
# define "hooks" for replacing configuration parameters
# in case running jobs on the CERN batch system/grid
#
#__isMC = #isMC#
#__HLTprocessName = #HLTprocessName#
#__pfCandidateCollection = #pfCandidateCollection#
#__applyEventSelection = #applyEventSelection#
#
#--------------------------------------------------------------------------------

#--------------------------------------------------------------------------------
# define GlobalTag to be used for event reconstruction
if isMC:
    process.GlobalTag.globaltag = cms.string('START52_V9::All')
else:
    process.GlobalTag.globaltag = cms.string('GR_R_52_V7::All')
#--------------------------------------------------------------------------------

#--------------------------------------------------------------------------------
# define skimming criteria
# (in order to be able to produce Tau Ntuple directly from unskimmed Monte Carlo/datasets;
#  HLT single jet trigger passed && either two CaloJets or two PFJets of Pt > 10 GeV within |eta| < 2.5)
process.load('TauAnalysis.TauIdEfficiency.filterQCDdiJet_cfi')
if isMC:
    process.dataQualityFilters.remove(process.hltPhysicsDeclared)
    process.dataQualityFilters.remove(process.dcsstatus)
#--------------------------------------------------------------------------------

#--------------------------------------------------------------------------------
#
# produce collections of objects needed as input for PAT-tuple production
# (e.g. rerun reco::Tau identification algorithms with latest tags)
#
from TauAnalysis.TauIdEfficiency.tools.configurePrePatProduction import configurePrePatProduction

configurePrePatProduction(process, pfCandidateCollection = pfCandidateCollection, isMC = isMC)
#--------------------------------------------------------------------------------

#--------------------------------------------------------------------------------
#
# produce PAT objects
#
from TauAnalysis.TauIdEfficiency.tools.configurePatTupleProduction import configurePatTupleProduction
from TauAnalysis.TauIdEfficiency.tools.sequenceBuilder import buildQCDdiJetTauSequence

process.load("PhysicsTools.PatAlgos.cleaningLayer1.tauCleaner_cfi")

# remove jets outside kinematic range Pt > 10 GeV && |eta| < 2.5 from Tau Ntuple
# (in order to speed-up plotting macros)
patCaloTauCleanerPrototype = process.cleanPatTaus.clone(
    preselection = cms.string(''),
    checkOverlaps = cms.PSet(),
    finalCut = cms.string(
        'caloTauTagInfoRef().jetRef().pt() > 8.0 & abs(caloTauTagInfoRef().jetRef().eta()) < 2.5'
    )
)

patPFTauCleanerPrototype = process.cleanPatTaus.clone(
    preselection = cms.string(''),
    checkOverlaps = cms.PSet(),
    finalCut = cms.string(
        'pfJetRef().pt() > 8.0 & abs(pfJetRef().eta()) < 2.5'
    )
)

retVal = configurePatTupleProduction(
    process, patSequenceBuilder = buildQCDdiJetTauSequence,
    patPFTauCleanerPrototype = patPFTauCleanerPrototype,
    patCaloTauCleanerPrototype = patCaloTauCleanerPrototype,
    hltProcess = HLTprocessName,
    isMC = isMC
)

# add event counter for Mauro's "self baby-sitting" technology
process.totalEventsProcessed = cms.EDProducer("EventCountProducer")
process.eventCounterPath = cms.Path(process.totalEventsProcessed)
#--------------------------------------------------------------------------------

#--------------------------------------------------------------------------------
#
# produce Ntuple
#
process.load("TauAnalysis.TauIdEfficiency.ntupleConfigGenPhaseSpaceEventInfo_cfi")
process.load("TauAnalysis.TauIdEfficiency.ntupleConfigGenPileUpEventInfo_cfi")

process.ntupleProducer = cms.EDProducer("ObjValEDNtupleProducer",

    ntupleName = cms.string("tauIdEffNtuple"),

    sources = cms.PSet()
)

if isMC:
    setattr(process.ntupleProducer.sources, "genPhaseSpaceEventInfo", process.genPhaseSpaceEventInfo_template)
    setattr(process.ntupleProducer.sources, "genPileUpEventInfo", process.genPileUpEventInfo_template)
#--------------------------------------------------------------------------------

#--------------------------------------------------------------------------------
#
# update InputTags for HLT trigger result object
# in case running on reprocessed Monte Carlo samples
#
if HLTprocessName != "HLT":
    process.hltSingleJet.selector.src = cms.InputTag('TriggerResults::' + HLTprocessName)
    process.patTrigger.processName = HLTprocessName
    process.patTriggerEvent.processName = HLTprocessName
    process.patCaloTausTriggerEvent.processName = cms.string(HLTprocessName)
    process.patPFTausTriggerEventFixedCone.processName = cms.string(HLTprocessName)
    process.patPFTausTriggerEventShrinkingCone.processName = cms.string(HLTprocessName)
    process.patPFTausTriggerEventHPS.processName = cms.string(HLTprocessName)
    process.patPFTausTriggerEventHPSpTaNC.processName = cms.string(HLTprocessName)
#--------------------------------------------------------------------------------

#--------------------------------------------------------------------------------
#
# Save PAT-tuple
#
process.patTupleOutputModule = cms.OutputModule("PoolOutputModule",
    cms.PSet(
        outputCommands = cms.untracked.vstring(
            'drop *',
            'keep EventAux_*_*_*',
            'keep LumiSummary_*_*_*',                       
            'keep edmMergeableCounter_*_*_*',
            ##'keep *_%s_*_*' % retVal['caloTauCollection'],
            ##'keep *_%s_*_*' % retVal['pfTauCollectionFixedCone'],
            ##'keep *_%s_*_*' % retVal['pfTauCollectionShrinkingCone'],
            'keep *_%s_*_*' % retVal['pfTauCollectionHPS'],
            'keep *_%s_*_*' % retVal['pfTauCollectionHPSpTaNC'],
            'keep *_offlinePrimaryVertices_*_*', 
            'keep *_selectedPrimaryVertexPosition_*_*', 
            'keep *_selectedPrimaryVertexHighestPtTrackSum_*_*', 
            'keep *_offlinePrimaryVerticesWithBS_*_*',
            'keep *_patPFMet_*_*',
            ##'keep *_patMETs_*_*',                                            
            'keep *_patJetsAK5PFnotOverlappingWithLeptonsForMEtUncertainty_*_*',
            'keep patJets_patJetsAK5PF_*_*',                                            
            ##'keep patJets_patJetsAK5Calo_*_*',                                            
            'keep *_*ntupleProducer*_*_*'
        )
    ),
    process.qcdDiJetEventSelection, # comment-out to disable filtering of events written to PAT-tuple
    fileName = cms.untracked.string("tauCommissioningQCDdiJetPATtuple.root")
)

from PhysicsTools.PatAlgos.patEventContent_cff import patTriggerEventContent
process.patTupleOutputModule.outputCommands.extend(patTriggerEventContent)

if isMC:
    process.patTupleOutputModule.outputCommands.extend(
        cms.untracked.vstring(
            'keep *_smearedPatJetsAK5PF_*_*',
            'keep *_vertexMultiplicityReweight3dRunA_*_*',
            'keep *_vertexMultiplicityReweight3dRunB_*_*',
            'keep *_vertexMultiplicityVsRhoPFNeutralReweight_*_*',
            'keep *_addPileupInfo_*_*',
            'keep *_genPhaseSpaceEventInfo_*_*',
            'keep *_genParticles_*_*'
        )
    )
#--------------------------------------------------------------------------------

process.printEventContent = cms.EDAnalyzer("EventContentAnalyzer")

process.p = cms.Path(
    process.prePatProductionSequence
   + process.patTupleProductionSequence
   + process.ntupleProducer
   ##+ process.printEventContent
)

process.options = cms.untracked.PSet(
    wantSummary = cms.untracked.bool(True)
)

process.o = cms.EndPath(process.patTupleOutputModule)

# define order in which different paths are run
if applyEventSelection:
    process.schedule = cms.Schedule(
        process.eventCounterPath,
        process.p,
        ##process.caloTauSkimPath,
        process.pfTauSkimPath,
        process.o
    )
else:
    delattr(process.patTupleOutputModule, "SelectEvents")
    process.schedule = cms.Schedule(
        process.eventCounterPath,
        process.p,
        process.o
    )

processDumpFile = open('produceCommissioningQCDdiJetPATTuple.dump' , 'w')
print >> processDumpFile, process.dumpPython()


