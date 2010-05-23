import FWCore.ParameterSet.Config as cms

process = cms.Process("skimQCDdiJet")

# import of standard configurations for RECOnstruction
# of electrons, muons and tau-jets with non-standard isolation cones
process.load('Configuration/StandardSequences/Services_cff')
process.load('FWCore/MessageService/MessageLogger_cfi')
process.MessageLogger.cerr.FwkReport.reportEvery = 100
#process.MessageLogger.cerr.threshold = cms.untracked.string('INFO')
process.load('Configuration/StandardSequences/GeometryIdeal_cff')
process.load('Configuration/StandardSequences/MagneticField_cff')
process.load('Configuration/StandardSequences/Reconstruction_cff')
process.load('Configuration/StandardSequences/FrontierConditions_GlobalTag_cff')
process.GlobalTag.globaltag = cms.string('MC_36Y_V7A::All')

#--------------------------------------------------------------------------------
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        '/store/relval/CMSSW_3_6_1/RelValZTT/GEN-SIM-RECO/START36_V7-v1/0021/F405BC9A-525D-DF11-AB96-002618943811.root',
        '/store/relval/CMSSW_3_6_1/RelValZTT/GEN-SIM-RECO/START36_V7-v1/0020/EE3E8F74-365D-DF11-AE3D-002618FDA211.root'
    ),
    skipEvents = cms.untracked.uint32(0)            
)

# print event content 
process.printEventContent = cms.EDAnalyzer("EventContentAnalyzer")

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(1000)
)
#--------------------------------------------------------------------------------

#--------------------------------------------------------------------------------
# define skimming criteria
# (HLT single jet trigger passed && either two CaloJets or two PFJets of Pt > 10 GeV within |eta| < 2.5)
process.load('TauAnalysis.TauIdEfficiency.filterQCDdiJet_cfi')
#--------------------------------------------------------------------------------

#--------------------------------------------------------------------------------
# define information to be written to output file
# for events passing skimming
process.load('Configuration.EventContent.EventContent_cff')
#--------------------------------------------------------------------------------

#--------------------------------------------------------------------------------
# save events passing either the CaloJet or PFJet skimming criteria
process.qcdDiJetSkimOutputModule = cms.OutputModule("PoolOutputModule",                                 
    process.RECOSIMEventContent,                                               
    process.qcdDiJetEventSelection,
    fileName = cms.untracked.string('qcdDiJetSkim.root')
)
#--------------------------------------------------------------------------------

process.options = cms.untracked.PSet(
    wantSummary = cms.untracked.bool(True)
)

process.o = cms.EndPath(process.qcdDiJetSkimOutputModule)

