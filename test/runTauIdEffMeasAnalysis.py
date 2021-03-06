#!/usr/bin/env python

from TauAnalysis.TauIdEfficiency.recoSampleDefinitionsTauIdEfficiency_7TeV_grid_cfi import recoSampleDefinitionsTauIdEfficiency_7TeV
from TauAnalysis.Configuration.userRegistry import getJobId
from TauAnalysis.TauIdEfficiency.tools.buildConfigFilesTauIdEffAnalysis import *

import os

channel = 'ZtoMuTau_tauIdEff'
#jobId = getJobId(channel)
jobId = '2012May12'

version = 'v1_8'
label = 'fitEWKbgSum_v1'

inputFilePath = '/data1/veelken/CMSSW_5_2_x/PATtuples/TauIdEffMeas/%s/%s/' % (jobId, version) \
               + 'user/v/veelken/CMSSW_5_2_x/PATtuples/TauIdEffMeas/%s/%s/' % (jobId, version)
outputFilePath = '/data1/veelken/tmp/muonPtGt17/%s_%s/' % (version, label)

samplesToAnalyze = [
    #
    # NOTE: data samples are added according to the runPeriod chosen
    #
    'Ztautau_pythia',
    'Zmumu_pythia',
    'DYmumuM10to20_pythia',
    'ZplusJets_madgraph2',
    'WplusJets_madgraph',
    'PPmuXptGt20Mu15v2',      
    'TTplusJets_madgraph2'
]

# define sample name and jobId of Ztautau sample
# used to compute preselection efficiencies and purities in C1p and C1f/D1p regions
sampleZtautau = 'Ztautau_pythia'

runPeriod = '2012RunAplusB'

runClosureTest = False
#runClosureTest = True

intLumiData = None
firstRunData = -1
lastRunData = -1
hltPaths = None
srcWeights = None
if runPeriod == '2012RunAplusB':
    samplesToAnalyze.extend([
        'data_TauPlusX_Run2012A_PromptReco_v1_runs190456to193621',
        'data_TauPlusX_Run2012B_PromptReco_v1_runs193752to194076v2',
        'data_TauPlusX_Run2012B_PromptReco_v1_runs194108to194479',
        'data_TauPlusX_Run2012B_PromptReco_v1_runs194790to195016',
        'data_TauPlusX_Run2012B_PromptReco_v1_runs195099to195947',
        'data_TauPlusX_Run2012B_PromptReco_v1_runs195948to196509'
    ])
    intLumiData = 5037 # runs 190456-196509
    hltPaths = {
        'Data' : [
            'HLT_IsoMu15_eta2p1_L1ETM20_v3',
            'HLT_IsoMu15_eta2p1_L1ETM20_v4',
            'HLT_IsoMu15_eta2p1_L1ETM20_v5'
        ],
        'smMC' : [
            'HLT_IsoMu15_eta2p1_L1ETM20_v1',
            'HLT_IsoMu15_eta2p1_L1ETM20_v3'
        ]
    }
    srcWeights = {
        'Data' : [],
        'smMC' : [ 'vertexMultiplicityReweight1d2012runs190456to196509' ]
    }
else:
    raise ValueError("Invalid runPeriod = %s !!" % runPeriod)

cut_muonPtMin         = 17.0
cut_tauLeadTrackPtMin =  5.0
cut_tauAbsIsoMax      =  5.0
cut_caloMEtPtMin      = 20.0
cut_pfMEtPtMin        = 20.0

plot_hltPaths = []

fitVariables = [
    'diTauVisMass',
    ##'diTauSVfitMass'
]    

mode = 'tauIdEfficiency'
#mode = 'tauChargeMisIdRate'

sysUncertainties = [
    "JetEn",          # needed for diTauMt/Pzeta
    "TauJetEn",       # needed for diTauVisMass
    "TauJetRes",      # needed for diTauVisMass
    "UnclusteredEn",  # needed for diTauMt/Pzeta
    "CaloMEtResponse" # needed for diTauMt/Pzeta and diTauVisMass
]

#templateMorphingMode = "none"
#templateMorphingMode = "horizontal" # WARNING: 'horizontal' template morphing runs **very** slow !!
templateMorphingMode = "vertical"

#fitIndividualProcesses = True
fitIndividualProcesses = False

# CV: only book histograms needed to run tau id. efficiency fit;
#     drop all control plots
fillControlPlots = True
#fillControlPlots = False

tauIds = {
    # HPS combined isolation discriminators
    # (based on isolation sumPt of PFChargedHadrons + PFGammas)
    'tauDiscrHPScombLooseDBcorr'  : {
        'discriminators' : [
            'decayModeFinding',
            'byLooseCombinedIsolationDeltaBetaCorr'
        ],
        'legendEntry' : "HPS comb. Loose",
        'markerStyleData' : 20,
        'markerStyleSim' : 24,
        'color' : 418
    },
    'tauDiscrHPScombLooseDBcorrAndElectronVetoMVA'  : {
        'discriminators' : [
            'decayModeFinding',
            'byLooseCombinedIsolationDeltaBetaCorr',
            'againstElectronMVA'
            
        ],
        'legendEntry' : "HPS comb. Loose & e-Veto MVA",
        'markerStyleData' : 20,
        'markerStyleSim' : 24,
        'color' : 418
    },
    ##'tauDiscrHPScombLooseDBcorrAndElectronVetoMVA2'  : {
    ##    'discriminators' : [
    ##        'decayModeFinding',
    ##        'byLooseCombinedIsolationDeltaBetaCorr',
    ##        'againstElectronTightMVA2'
    ##        
    ##    ],
    ##    'legendEntry' : "HPS comb. Loose & e-Veto MVA2",
    ##    'markerStyleData' : 20,
    ##    'markerStyleSim' : 24,
    ##    'color' : 418
    ##},
    'tauDiscrHPScombLooseDBcorrAndMuonVeto'  : {
        'discriminators' : [
            'decayModeFinding',
            'byLooseCombinedIsolationDeltaBetaCorr',
            'againstMuonTight'
        ],
        'legendEntry' : "HPS comb. Loose & #mu-Veto",
        'markerStyleData' : 20,
        'markerStyleSim' : 24,
        'color' : 418
    },
    'tauDiscrHPScombMediumDBcorr' : {
        'discriminators' : [
            'decayModeFinding',
            'byMediumCombinedIsolationDeltaBetaCorr'
        ],
        'legendEntry' : "HPS comb. Medium",
        'markerStyleData' : 21,
        'markerStyleSim' : 25,
        'color' : 807
    },
    'tauDiscrHPScombTightDBcorr' : {
        'discriminators' : [
            'decayModeFinding',
            'byTightCombinedIsolationDeltaBetaCorr'
        ],
        'legendEntry' : "HPS comb. Tight",
        'markerStyleData' : 22,
        'markerStyleSim' : 26,
        'color' : 618
    },
    # HPS ring isolation discriminators
    # (based on MVA)
    'tauDiscrHPSlooseMVA'  : {
        'discriminators' : [
            'decayModeFinding',
            'byLooseIsolationMVA'
        ],
        'legendEntry' : "HPS Loose MVA",
        'markerStyleData' : 20,
        'markerStyleSim' : 24,
        'color' : 418
    },
    'tauDiscrHPSlooseMVAandElectronVetoMVA'  : {
        'discriminators' : [
            'decayModeFinding',
            'byLooseIsolationMVA',
            'againstElectronMVA'
            
        ],
        'legendEntry' : "HPS Loose MVA & e-Veto MVA",
        'markerStyleData' : 20,
        'markerStyleSim' : 24,
        'color' : 418
    },
    ##'tauDiscrHPSlooseMVAandElectronVetoMVA2'  : {
    ##    'discriminators' : [
    ##        'decayModeFinding',
    ##        'byLooseIsolationMVA',
    ##        'againstElectronTightMVA2'
    ##        
    ##    ],
    ##    'legendEntry' : "HPS Loose MVA & e-Veto MVA2",
    ##    'markerStyleData' : 20,
    ##    'markerStyleSim' : 24,
    ##    'color' : 418
    ##},
    'tauDiscrHPSlooseMVAandMuonVeto'  : {
        'discriminators' : [
            'decayModeFinding',
            'byLooseIsolationMVA',
            'againstMuonTight'
        ],
        'legendEntry' : "HPS Loose MVA & #mu-Veto",
        'markerStyleData' : 20,
        'markerStyleSim' : 24,
        'color' : 418
    },
    'tauDiscrHPSmediumMVA' : {
        'discriminators' : [
            'decayModeFinding',
            'byMediumIsolationMVA'
        ],
        'legendEntry' : "HPS Medium MVA",
        'markerStyleData' : 21,
        'markerStyleSim' : 25,
        'color' : 807
    },
    'tauDiscrHPStightMVA' : {
        'discriminators' : [
            'decayModeFinding',
            'byTightIsolationMVA'
        ],
        'legendEntry' : "HPS Tight MVA",
        'markerStyleData' : 22,
        'markerStyleSim' : 26,
        'color' : 618
    }
}

binning = {
    # NOTE: these strings need to match what is hard-coded in regionEntryType::regionEntryType constructor,
    #       defined in TauAnalysis/TauIdEfficiency/bin/FWLiteTauIdEffAnalyzer.cc 
    ##'tauPt' : {
    ##    'tauPtLt30' : {
    ##        'min' :  20.0,
    ##        'max' :  30.0
    ##    },
    ##    'tauPt30to40' : {
    ##        'min' :  30.0,
    ##        'max' :  40.0
    ##    },
    ##    'tauPtGt40' : {
    ##        'min' :  40.0,
    ##        'max' : 100.0
    ##    },
    ##    'xAxisTitle' : "P_{T}^{#tau}"
    ##},
    ##'tauAbsEta' : {
    ##    'tauAbsEtaLt14' : {
    ##        'min' :  0.0,
    ##        'max' :  1.4
    ##    },
    ##    'tauAbsEta14to19' : {
    ##        'min' :  1.4,
    ##        'max' :  1.9
    ##    },
    ##    'tauAbsEta19to23' : {
    ##        'min' :  1.9,
    ##        'max' :  2.3
    ##    },
    ##    'xAxisTitle' : "|#eta_{#tau}|"
    ##},
    ##'numVertices' : {
    ##    'numVerticesLe5' : {
    ##        'min' :  -0.5,
    ##        'max' :   5.5
    ##    },
    ##    'numVertices6to10' : {
    ##        'min' :   5.5,
    ##        'max' :  10.5
    ##    },
    ##    'numVertices11to15' : {
    ##        'min' :  10.5,
    ##        'max' :  15.5
    ##    },
    ##    'numVertices16to20' : {
    ##        'min' :  15.5,
    ##        'max' :  20.5
    ##    },
    ##    'numVerticesGe21' : {
    ##        'min' :  20.5,
    ##        'max' :  29.5
    ##    },
    ##    'xAxisTitle' : "Num. Vertices"
    ##},
    ##'sumEt' : {
    ##    'sumEtLt250' : {
    ##        'min' :    0.0,
    ##        'max' :  250.0
    ##    },
    ##    'sumEt250to350' : {
    ##        'min' :  250.0,
    ##        'max' :  350.0
    ##    },
    ##    'sumEt350to450' : {
    ##        'min' :  350.0,
    ##        'max' :  450.0
    ##    },
    ##    'sumEtGt450' : {
    ##        'min' :  450.0,
    ##        'max' : 1000.0
    ##    },
    ##    'xAxisTitle' : "#Sigma E_{T}"
    ##}
}

execDir = "%s/bin/%s/" % (os.environ['CMSSW_BASE'], os.environ['SCRAM_ARCH'])

executable_compTauIdEffPreselNumbers = None
suffix_noTauSel                      = None
regions                              = None
regionsToFit                         = None
keyword_compTauIdEffPreselNumbers    = None
passed_region                        = None
failed_region                        = None
regionQCDtemplateFromData_passed     = None
regionQCDtemplateFromData_failed     = None
regionQCDtemplateFromData_D          = None
regionWplusJetsSideband_passed       = None
regionWplusJetsSideband_failed       = None
regionWplusJetsSideband_D            = None
histQCDtemplateFromData_passed       = None
histQCDtemplateFromData_failed       = None
histQCDtemplateFromData_D            = None
fitMethod                            = None
tauChargeMode                        = None
disableTauCandPreselCuts             = None
executable_compTauIdEffFinalNumbers  = None
keyword_compTauIdEffFinalNumbers     = None
expEff_label                         = None                
measEff_label                        = None  
if mode == 'tauIdEfficiency':
    executable_compTauIdEffPreselNumbers = execDir + 'FWLiteTauIdEffPreselNumbers'
    keyword_compTauIdEffPreselNumbers    = 'compTauIdEffPreselNumbers'
    suffix_noTauSel                      = 'noTauSel'
    regions                              = [
        'ABCD',
        'A',
        'A_mW', 
        'A1',  # QCD enriched control region (OS, loose muon isolation, Mt && Pzeta cuts applied)
        'A1_mW',
        #'A1_mW_tW',
        'A1p',
        'A1f',
        'AWj',
        'AWj_mW',
        #'AWj_mW_tW',
        'B',
        'B_mW',
        'B1',  # QCD enriched control region (SS, loose muon isolation, Mt && Pzeta cuts applied)
        'B1_mW',
        #'B1_mW_tW',
        'B1p',
        'B1f',
        'BWj',
        'BWj_mW',
        #'BWj_mW_tW',
        'C',
        'C1',
        'C1p',
        'C1f',
        'C1f_DEBUG1',
        'C1f_DEBUG2',
        'C2',
        'C2p',
        'C2f',
        'CWj',
        'CWj_mW',
        'D',   # generic background control region (SS, tight muon isolation)
        'D1',
        'D1p',
        'D1f',
        'DWj',
        'DWj_mW'
    ]
    ##regionsToFit                         = [ 'A', 'B', 'C1p', 'C1f', 'C2', 'D' ]
    regionsToFit                         = [ 'C1p', 'C1f' ]
    passed_region                        = 'C1p'
    failed_region                        = 'C1f'
    regionQCDtemplateFromData_passed     = 'A1'
    regionQCDtemplateFromData_failed     = 'A1'
    regionQCDtemplateFromData_D          = 'A'
    #regionWplusJetsSideband_passed       = 'CWj'
    #regionWplusJetsSideband_failed       = 'CWj'
    regionWplusJetsSideband_passed       = 'AWj'
    regionWplusJetsSideband_failed       = 'AWj'
    regionWplusJetsSideband_D            = 'AWj'
    histQCDtemplateFromData_passed       = ''.join([ passed_region, "_qcd" ])
    histQCDtemplateFromData_failed       = ''.join([ failed_region, "_qcd" ])
    histQCDtemplateFromData_D            = ''.join([ "D",           "_qcd" ])
    fitMethod                            = 'fitTauIdEff'
    tauChargeMode                        = 'tauLeadTrackCharge'
    disableTauCandPreselCuts             = False
    executable_compTauIdEffFinalNumbers  = execDir + 'compTauIdEffFinalNumbers'
    keyword_compTauIdEffFinalNumbers     = 'compTauIdEffFinalNumbers'
    expEff_label                         = 'expEff'
    measEff_label                        = 'measEff'   
elif mode == 'tauChargeMisIdRate':
    executable_compTauIdEffPreselNumbers = execDir + 'FWLiteTauChargeMisIdPreselNumbers'
    keyword_compTauIdEffPreselNumbers    = 'compTauChargeMisIdPreselNumbers'
    suffix_noTauSel                      = 'noTauSel'
    regions                              = [
        'A1_mW',
        'B1',  # control region used to obtain QCD template from Data
        'B1_mW',
        'B1p',
        'B1f',
        'B2',
        'C1',
        'C1p',
        'C1f',
        'C2',
        'CWj',
        'D1',
        'D1p',
        'D1f',
        'D2',
        'DWj'
    ]
    regionsToFit                         = [ 'C1p', 'D1p' ]
    passed_region                        = 'C1p'
    failed_region                        = 'D1p'
    regionQCDtemplateFromData_passed     = 'A1_mW'
    regionQCDtemplateFromData_failed     = 'B1_mW'
    regionQCDtemplateFromData_D          = ''
    regionWplusJetsSideband_passed       = 'CWj'
    regionWplusJetsSideband_failed       = 'DWj'
    regionWplusJetsSideband_D            = ''
    histQCDtemplateFromData_passed       = ''.join([ passed_region, "_qcd" ])
    histQCDtemplateFromData_failed       = ''.join([ failed_region, "_qcd" ])
    histQCDtemplateFromData_D            = ''
    fitMethod                            = 'fitTauIdEff'
    tauChargeMode                        = 'tauSignalChargedHadronSum'
    disableTauCandPreselCuts             = True
    executable_compTauIdEffFinalNumbers  = execDir + 'compTauChargeMisIdFinalNumbers'
    keyword_compTauIdEffFinalNumbers     = 'compTauChargeMisIdFinalNumbers'
    expEff_label                         = 'expRate'
    measEff_label                        = 'measRate'   
else:
    raise ValueError("Invalid mode = %s !!" % mode)

executable_FWLiteTauIdEffAnalyzer = execDir + 'FWLiteTauIdEffAnalyzer'
executable_hadd = 'hadd'
executable_makeTauIdEffQCDtemplate = execDir + 'makeTauIdEffQCDtemplate'
executable_smoothTauIdEffTemplates = execDir + 'smoothTauIdEffTemplates'
executable_fitTauIdEff = execDir + fitMethod
executable_makeTauIdEffFinalPlots = execDir + 'makeTauIdEffFinalPlots'
executable_shell = '/bin/csh'

nice = 'nice '

if len(samplesToAnalyze) == 0:
    samplesToAnalyze = recoSampleDefinitionsTauIdEfficiency_7TeV['RECO_SAMPLES'].keys()

if not os.path.exists(outputFilePath):
    os.mkdir(outputFilePath)

outputFilePath = os.path.join(outputFilePath, runPeriod)
if not os.path.exists(outputFilePath):
    os.mkdir(outputFilePath)
    
outputFilePath = os.path.join(outputFilePath, mode)
if not os.path.exists(outputFilePath):
    os.mkdir(outputFilePath)

outputFilePath_plots = os.path.join(outputFilePath, "plots")
if not os.path.exists(outputFilePath_plots):
    os.mkdir(outputFilePath_plots)

#--------------------------------------------------------------------------------
#
# build config files for running FWLiteTauIdEffAnalyzer macro
#
configFileNames_FWLiteTauIdEffAnalyzer = []
outputFileNames_FWLiteTauIdEffAnalyzer = []
logFileNames_FWLiteTauIdEffAnalyzer    = []
for sampleToAnalyze in samplesToAnalyze:
    fwliteInput_firstRun = -1
    fwliteInput_lastRun  = -1
    processType = recoSampleDefinitionsTauIdEfficiency_7TeV['RECO_SAMPLES'][sampleToAnalyze]['type']
    if processType == 'Data':
        fwliteInput_firstRun = firstRunData
        fwliteInput_lastRun = lastRunData
    retVal_FWLiteTauIdEffAnalyzer = \
      buildConfigFile_FWLiteTauIdEffAnalyzer(sampleToAnalyze, version, inputFilePath, fwliteInput_firstRun, fwliteInput_lastRun, tauIds,
                                             binning, sysUncertainties, outputFilePath,
                                             recoSampleDefinitionsTauIdEfficiency_7TeV,
                                             regions, intLumiData, hltPaths, srcWeights,
                                             tauChargeMode, disableTauCandPreselCuts,
                                             cut_muonPtMin, cut_tauLeadTrackPtMin, cut_tauAbsIsoMax, cut_caloMEtPtMin, cut_pfMEtPtMin,
                                             plot_hltPaths,
                                             fillControlPlots)

    if retVal_FWLiteTauIdEffAnalyzer is None:
        continue
    
    configFileNames_FWLiteTauIdEffAnalyzer.extend(retVal_FWLiteTauIdEffAnalyzer['configFileNames'])
    outputFileNames_FWLiteTauIdEffAnalyzer.extend(retVal_FWLiteTauIdEffAnalyzer['outputFileNames'])
    logFileNames_FWLiteTauIdEffAnalyzer.extend(retVal_FWLiteTauIdEffAnalyzer['logFileNames'])
#--------------------------------------------------------------------------------

#--------------------------------------------------------------------------------
#
# build shell script for running 'hadd' in order to "harvest" histograms
# produced by FWLiteTauIdEffAnalyzer macro
#
haddShellFileName_stage1 = os.path.join(outputFilePath, 'harvestTauIdEffHistograms_stage1_%s.csh' % "".join([ jobId, version ]))
haddInputFileNames_stage1 = outputFileNames_FWLiteTauIdEffAnalyzer
haddOutputFileName_stage1 = os.path.join(outputFilePath, 'analyzeTauIdEffHistograms_all_%s.root' % "".join([ jobId, version ]))
retVal_hadd_stage1 = \
  buildConfigFile_hadd(executable_hadd, haddShellFileName_stage1, haddInputFileNames_stage1, haddOutputFileName_stage1)
haddLogFileName_stage1 = retVal_hadd_stage1['logFileName']
#--------------------------------------------------------------------------------

#--------------------------------------------------------------------------------
#
# build config files for running smoothTauIdEffTemplates macro
#
configFileNames_smoothTauIdEffTemplates = []
outputFileNames_smoothTauIdEffTemplates = []
logFileNames_smoothTauIdEffTemplates   = []
retVal_smoothTauIdEffTemplates = \
  buildConfigFile_smoothTauIdEffTemplates("".join([ jobId, version ]), '', haddOutputFileName_stage1, tauIds.keys(),
                                          fitVariables, sysUncertainties, outputFilePath,
                                          regions, fitIndividualProcesses, True, outputFilePath_plots)
configFileNames_smoothTauIdEffTemplates.append(retVal_smoothTauIdEffTemplates['configFileName'])
outputFileNames_smoothTauIdEffTemplates.append(retVal_smoothTauIdEffTemplates['outputFileName'])
logFileNames_smoothTauIdEffTemplates.append(retVal_smoothTauIdEffTemplates['logFileName'])
for binVariable in binning.keys():
    for binName, binOptions in binning[binVariable].items():
        if isinstance(binOptions, dict) and binOptions.get('min') is not None and binOptions.get('max') is not None:
            retVal_smoothTauIdEffTemplates = \
              buildConfigFile_smoothTauIdEffTemplates("".join([ jobId, version ]), binName, haddOutputFileName_stage1, tauIds.keys(),
                                                      fitVariables, sysUncertainties, outputFilePath,
                                                      regions, fitIndividualProcesses, True, outputFilePath_plots)
            configFileNames_smoothTauIdEffTemplates.append(retVal_smoothTauIdEffTemplates['configFileName'])
            outputFileNames_smoothTauIdEffTemplates.append(retVal_smoothTauIdEffTemplates['outputFileName'])
            logFileNames_smoothTauIdEffTemplates.append(retVal_smoothTauIdEffTemplates['logFileName'])
#--------------------------------------------------------------------------------

#--------------------------------------------------------------------------------
#
# build shell script for running 'hadd' in order to merge smoothed shape templates
# produced by smoothTauIdEffTemplates macro with output of FWLiteTauIdEffAnalyzer macro 
#
haddShellFileName_stage2 = os.path.join(outputFilePath, 'harvestTauIdEffHistograms_stage2_%s.csh' % "".join([ jobId, version ]))
haddInputFileNames_stage2 = []
haddInputFileNames_stage2.append(haddOutputFileName_stage1)
haddInputFileNames_stage2.extend(outputFileNames_smoothTauIdEffTemplates)
haddOutputFileName_stage2 = \
  os.path.join(outputFilePath, 'analyzeTauIdEffHistograms_all_smoothedTemplates_%s.root' % "".join([ jobId, version ]))
retVal_hadd_stage2 = \
  buildConfigFile_hadd(executable_hadd, haddShellFileName_stage2, haddInputFileNames_stage2, haddOutputFileName_stage2)
haddLogFileName_stage2 = retVal_hadd_stage2['logFileName']
#--------------------------------------------------------------------------------

#--------------------------------------------------------------------------------
#
# build config files for running makeTauIdEffQCDtemplate macro
#
configFileNames_makeTauIdEffQCDtemplate = []
outputFileNames_makeTauIdEffQCDtemplate = []
logFileNames_makeTauIdEffQCDtemplate    = []
retVal_makeTauIdEffQCDtemplate = \
  buildConfigFile_makeTauIdEffQCDtemplate("".join([ jobId, version ]), '', haddOutputFileName_stage2, tauIds.keys(),
                                          fitVariables, sysUncertainties, outputFilePath,
                                          regionQCDtemplateFromData_passed, regionQCDtemplateFromData_failed, regionQCDtemplateFromData_D,
                                          regionWplusJetsSideband_passed, regionWplusJetsSideband_failed, regionWplusJetsSideband_D, 
                                          histQCDtemplateFromData_passed, histQCDtemplateFromData_failed, histQCDtemplateFromData_D)
configFileNames_makeTauIdEffQCDtemplate.append(retVal_makeTauIdEffQCDtemplate['configFileName'])
outputFileNames_makeTauIdEffQCDtemplate.append(retVal_makeTauIdEffQCDtemplate['outputFileName'])
logFileNames_makeTauIdEffQCDtemplate.append(retVal_makeTauIdEffQCDtemplate['logFileName'])
for binVariable in binning.keys():
    for binName, binOptions in binning[binVariable].items():
        if isinstance(binOptions, dict) and binOptions.get('min') is not None and binOptions.get('max') is not None:
            retVal_makeTauIdEffQCDtemplate = \
              buildConfigFile_makeTauIdEffQCDtemplate("".join([ jobId, version ]), binName, haddOutputFileName_stage2, tauIds.keys(),
                                                      fitVariables, sysUncertainties, outputFilePath,
                                                      regionQCDtemplateFromData_passed, regionQCDtemplateFromData_failed, regionQCDtemplateFromData_D,
                                                      regionWplusJetsSideband_passed, regionWplusJetsSideband_failed, regionWplusJetsSideband_D, 
                                                      histQCDtemplateFromData_passed, histQCDtemplateFromData_failed, histQCDtemplateFromData_D)
            configFileNames_makeTauIdEffQCDtemplate.append(retVal_makeTauIdEffQCDtemplate['configFileName'])
            outputFileNames_makeTauIdEffQCDtemplate.append(retVal_makeTauIdEffQCDtemplate['outputFileName'])
            logFileNames_makeTauIdEffQCDtemplate.append(retVal_makeTauIdEffQCDtemplate['logFileName'])
#--------------------------------------------------------------------------------

#--------------------------------------------------------------------------------
#
# build shell script for running 'hadd' in order to merge QCD background templates
# produced by makeTauIdEffQCDtemplate macro with output of FWLiteTauIdEffAnalyzer macro 
#
haddShellFileName_stage3 = os.path.join(outputFilePath, 'harvestTauIdEffHistograms_stage3_%s.csh' % "".join([ jobId, version ]))
haddInputFileNames_stage3 = []
haddInputFileNames_stage3.append(haddOutputFileName_stage2)
haddInputFileNames_stage3.extend(outputFileNames_makeTauIdEffQCDtemplate)
haddOutputFileName_stage3 = \
  os.path.join(outputFilePath, 'analyzeTauIdEffHistograms_all_corrQCDtemplates_%s.root' % "".join([ jobId, version ]))
retVal_hadd_stage3 = \
  buildConfigFile_hadd(executable_hadd, haddShellFileName_stage3, haddInputFileNames_stage3, haddOutputFileName_stage3)
haddLogFileName_stage3 = retVal_hadd_stage3['logFileName']
#--------------------------------------------------------------------------------

#--------------------------------------------------------------------------------
#
# build config files for running fitTauIdEff macro
#
configFileNames_fitTauIdEff = []
outputFileNames_fitTauIdEff = []
logFileNames_fitTauIdEff    = []
for tauId in tauIds.keys():
    for fitVariable in fitVariables:
        retVal_fitTauIdEff = \
          buildConfigFile_fitTauIdEff(fitMethod, "".join([ jobId, version ]), '', haddOutputFileName_stage3, tauId,
                                      fitVariable, templateMorphingMode, sysUncertainties, outputFilePath,
                                      regionsToFit, passed_region, failed_region,                              
                                      histQCDtemplateFromData_passed, histQCDtemplateFromData_failed, histQCDtemplateFromData_D,
                                      fitIndividualProcesses, intLumiData, runClosureTest, True, outputFilePath_plots)
        configFileNames_fitTauIdEff.append(retVal_fitTauIdEff['configFileName'])
        outputFileNames_fitTauIdEff.append(retVal_fitTauIdEff['outputFileName'])
        logFileNames_fitTauIdEff.append(retVal_fitTauIdEff['logFileName'])
        for binVariable in binning.keys():
            for binName, binOptions in binning[binVariable].items():
                if isinstance(binOptions, dict) and binOptions.get('min') is not None and binOptions.get('max') is not None:
                    retVal_fitTauIdEff = \
                      buildConfigFile_fitTauIdEff(fitMethod, "".join([ jobId, version ]), binName, haddOutputFileName_stage3, tauId,
                                                  fitVariable, templateMorphingMode, sysUncertainties, outputFilePath,
                                                  regionsToFit, passed_region, failed_region,
                                                  histQCDtemplateFromData_passed, histQCDtemplateFromData_failed, histQCDtemplateFromData_D,
                                                  fitIndividualProcesses, intLumiData, runClosureTest, False, outputFilePath_plots)
                    configFileNames_fitTauIdEff.append(retVal_fitTauIdEff['configFileName'])
                    outputFileNames_fitTauIdEff.append(retVal_fitTauIdEff['outputFileName'])
                    logFileNames_fitTauIdEff.append(retVal_fitTauIdEff['logFileName'])
#--------------------------------------------------------------------------------

#--------------------------------------------------------------------------------
#
# build config files for running FWLiteTauIdEffPreselNumbers macro
#
retVal_FWLiteTauIdEffPreselNumbers = \
  buildConfigFile_FWLiteTauIdEffPreselNumbers(inputFilePath, sampleZtautau, "_".join([ suffix_noTauSel, version ]), tauIds,
                                              binning, outputFilePath, hltPaths, srcWeights, keyword_compTauIdEffPreselNumbers,
                                              cut_muonPtMin, cut_tauLeadTrackPtMin, cut_tauAbsIsoMax, cut_caloMEtPtMin, cut_pfMEtPtMin)
configFileName_FWLiteTauIdEffPreselNumbers = retVal_FWLiteTauIdEffPreselNumbers['configFileName']
outputFileName_FWLiteTauIdEffPreselNumbers = retVal_FWLiteTauIdEffPreselNumbers['outputFileName']
logFileName_FWLiteTauIdEffPreselNumbers = retVal_FWLiteTauIdEffPreselNumbers['logFileName']
#--------------------------------------------------------------------------------

#--------------------------------------------------------------------------------
#
# build shell script for running 'hadd' in order to "harvest" fit results
# with preselection efficiencies and purities in regions C1p/C1f
#
haddShellFileName_stage4 = os.path.join(outputFilePath, 'harvestTauIdEffHistograms_stage4_%s.csh' % "".join([ jobId, version ]))
haddInputFileNames_stage4 = []
haddInputFileNames_stage4.extend(outputFileNames_fitTauIdEff)
haddInputFileNames_stage4.append(outputFileName_FWLiteTauIdEffPreselNumbers)
haddOutputFileName_stage4 = os.path.join(outputFilePath, 'compTauIdEffFinalNumbers_input_%s.root' % "".join([ jobId, version ]))
retVal_hadd_stage4 = \
  buildConfigFile_hadd(executable_hadd, haddShellFileName_stage4, haddInputFileNames_stage4, haddOutputFileName_stage4)
haddLogFileName_stage4 = retVal_hadd_stage4['logFileName']
#--------------------------------------------------------------------------------

#--------------------------------------------------------------------------------
#
# build config files for running compTauIdEffFinalNumbers macro
#
configFileNames_compTauIdEffFinalNumbers = []
outputFileNames_compTauIdEffFinalNumbers = []
logFileNames_compTauIdEffFinalNumbers = []
retVal_compTauIdEffFinalNumbers = \
  buildConfigFile_compTauIdEffFinalNumbers(haddOutputFileName_stage4, '', "".join([ jobId, version ]), tauIds,
                                           fitVariables, outputFilePath,
                                           keyword_compTauIdEffFinalNumbers, passed_region, failed_region,
                                           fitIndividualProcesses)
configFileNames_compTauIdEffFinalNumbers.append(retVal_compTauIdEffFinalNumbers['configFileName'])
outputFileNames_compTauIdEffFinalNumbers.append(retVal_compTauIdEffFinalNumbers['outputFileName'])
logFileNames_compTauIdEffFinalNumbers.append(retVal_compTauIdEffFinalNumbers['logFileName'])
for binVariable in binning.keys():
    for binName, binOptions in binning[binVariable].items():
        if isinstance(binOptions, dict) and binOptions.get('min') is not None and binOptions.get('max') is not None:
            retVal_compTauIdEffFinalNumbers = \
              buildConfigFile_compTauIdEffFinalNumbers(haddOutputFileName_stage4, binName, "".join([ jobId, version ]), tauIds,
                                                       fitVariables, outputFilePath,
                                                       keyword_compTauIdEffFinalNumbers, passed_region, failed_region,
                                                       fitIndividualProcesses)
            configFileNames_compTauIdEffFinalNumbers.append(retVal_compTauIdEffFinalNumbers['configFileName'])
            outputFileNames_compTauIdEffFinalNumbers.append(retVal_compTauIdEffFinalNumbers['outputFileName'])
            logFileNames_compTauIdEffFinalNumbers.append(retVal_compTauIdEffFinalNumbers['logFileName'])
#--------------------------------------------------------------------------------

#--------------------------------------------------------------------------------
#
# build shell script for running 'hadd' in order to "harvest" final tau id. efficiency numbers
# for different tauPt, tauEta,... bins
#
haddShellFileName_stage5 = os.path.join(outputFilePath, 'harvestTauIdEffHistograms_stage5_%s.csh' % "".join([ jobId, version ]))
haddInputFileNames_stage5 = outputFileNames_compTauIdEffFinalNumbers
haddOutputFileName_stage5 = os.path.join(outputFilePath, 'compTauIdEffFinalNumbers_all_%s.root' % "".join([ jobId, version ]))
retVal_hadd_stage5 = \
  buildConfigFile_hadd(executable_hadd, haddShellFileName_stage5, haddInputFileNames_stage5, haddOutputFileName_stage5)
haddLogFileName_stage5 = retVal_hadd_stage5['logFileName']
#--------------------------------------------------------------------------------

#--------------------------------------------------------------------------------
#
# make final tau id. efficiency plots as function of tauPt, tauEta,...
#
configFileNames_makeTauIdEffFinalPlots = []
outputFileNames_makeTauIdEffFinalPlots = []
logFileNames_makeTauIdEffFinalPlots    = []
for binVariable in binning.keys():

    # make plots for HPS combined isolation discriminators
    discriminators_HPScombined = [
        'tauDiscrHPScombLooseDBcorr',
        'tauDiscrHPScombMediumDBcorr',
        'tauDiscrHPScombTightDBcorr'
    ]
    outputFileName_HPScombined = 'makeTauIdEffFinalPlots_HPScombined_%s.eps' % binVariable
    retVal_makeTauIdEffFinalPlots = \
      buildConfigFile_makeTauIdEffFinalPlots(haddOutputFileName_stage5, tauIds, discriminators_HPScombined,
                                             binning[binVariable], fitVariables, outputFilePath, outputFileName_HPScombined,
                                             expEff_label, measEff_label, intLumiData)
    configFileNames_makeTauIdEffFinalPlots.append(retVal_makeTauIdEffFinalPlots['configFileName'])
    outputFileNames_makeTauIdEffFinalPlots.append(retVal_makeTauIdEffFinalPlots['outputFileName'])
    logFileNames_makeTauIdEffFinalPlots.append(retVal_makeTauIdEffFinalPlots['logFileName'])

    # make plots for HPS isolation with MVA-based isolation discriminators
    discriminators_HPSmva = [
        'tauDiscrHPSlooseMVA',
        'tauDiscrHPSmediumMVA',
        'tauDiscrHPStightMVA'
    ]
    outputFileName_HPSmva = 'makeTauIdEffFinalPlots_HPSmva_%s.eps' % binVariable
    retVal_makeTauIdEffFinalPlots = \
      buildConfigFile_makeTauIdEffFinalPlots(haddOutputFileName_stage5, tauIds, discriminators_HPSmva,
                                             binning[binVariable], fitVariables, outputFilePath, outputFileName_HPSmva,
                                             expEff_label, measEff_label, intLumiData)
    configFileNames_makeTauIdEffFinalPlots.append(retVal_makeTauIdEffFinalPlots['configFileName'])
    outputFileNames_makeTauIdEffFinalPlots.append(retVal_makeTauIdEffFinalPlots['outputFileName'])
    logFileNames_makeTauIdEffFinalPlots.append(retVal_makeTauIdEffFinalPlots['logFileName'])
#--------------------------------------------------------------------------------

def make_MakeFile_vstring(list_of_strings):
    retVal = ""
    for i, string_i in enumerate(list_of_strings):
        if i > 0:
            retVal += " "
        retVal += string_i
    return retVal

# done building config files, now build Makefile...
makeFileName = "Makefile_TauIdEffMeasAnalysis_%s_%s_%s" % (jobId, version, label)
##if runClosureTest:
##    makeFileName += "_mcClosureTest"
makeFile = open(makeFileName, "w")
makeFile.write("\n")
makeFile.write("all: %s %s\n" %
  (haddOutputFileName_stage5,
   make_MakeFile_vstring(outputFileNames_makeTauIdEffFinalPlots)))
makeFile.write("\techo 'Finished running TauIdEffMeasAnalysis.'\n")
makeFile.write("\n")
for i, outputFileName in enumerate(outputFileNames_FWLiteTauIdEffAnalyzer):
    makeFile.write("%s: %s\n" %
      (outputFileName,
       #executable_FWLiteTauIdEffAnalyzer,
       ""))
    makeFile.write("\t%s%s %s &> %s\n" %
      (nice, executable_FWLiteTauIdEffAnalyzer,
       configFileNames_FWLiteTauIdEffAnalyzer[i],
       logFileNames_FWLiteTauIdEffAnalyzer[i]))
makeFile.write("\n")
makeFile.write("%s: %s\n" %
  (haddOutputFileName_stage1,
   make_MakeFile_vstring(haddInputFileNames_stage1)))
makeFile.write("\t%s%s %s &> %s\n" %
  (nice, executable_shell,
   haddShellFileName_stage1,
   haddLogFileName_stage1))
makeFile.write("\n")
for i, outputFileName in enumerate(outputFileNames_smoothTauIdEffTemplates):
    makeFile.write("%s: %s %s\n" %
      (outputFileName,
       executable_smoothTauIdEffTemplates,
       haddOutputFileName_stage1))
    makeFile.write("\t%s%s %s &> %s\n" %
      (nice, executable_smoothTauIdEffTemplates,
       configFileNames_smoothTauIdEffTemplates[i],
       logFileNames_smoothTauIdEffTemplates[i]))
makeFile.write("\n")
makeFile.write("%s: %s\n" %
  (haddOutputFileName_stage2,
   make_MakeFile_vstring(haddInputFileNames_stage2)))
makeFile.write("\t%s%s %s &> %s\n" %
  (nice, executable_shell,
   haddShellFileName_stage2,
   haddLogFileName_stage2))
makeFile.write("\n")
for i, outputFileName in enumerate(outputFileNames_makeTauIdEffQCDtemplate):
    makeFile.write("%s: %s %s\n" %
      (outputFileName,
       executable_makeTauIdEffQCDtemplate,
       haddOutputFileName_stage2))
    makeFile.write("\t%s%s %s &> %s\n" %
      (nice, executable_makeTauIdEffQCDtemplate,
       configFileNames_makeTauIdEffQCDtemplate[i],
       logFileNames_makeTauIdEffQCDtemplate[i]))
makeFile.write("\n")
makeFile.write("%s: %s\n" %
  (haddOutputFileName_stage3,
   make_MakeFile_vstring(haddInputFileNames_stage3)))
makeFile.write("\t%s%s %s &> %s\n" %
  (nice, executable_shell,
   haddShellFileName_stage3,
   haddLogFileName_stage3))
makeFile.write("\n")
for i, outputFileName in enumerate(outputFileNames_fitTauIdEff):
    makeFile.write("%s: %s %s %s\n" %
      (outputFileName,
       executable_fitTauIdEff,
       configFileNames_fitTauIdEff[i],
       haddOutputFileName_stage3))
    makeFile.write("\t%s%s %s &> %s\n" %
      (nice, executable_fitTauIdEff,
       configFileNames_fitTauIdEff[i],
       logFileNames_fitTauIdEff[i]))
makeFile.write("\n")
makeFile.write("%s: %s\n" %
  (outputFileName_FWLiteTauIdEffPreselNumbers,
   #executable_compTauIdEffPreselNumbers,
   ""))
makeFile.write("\t%s%s %s &> %s\n" %
  (nice, executable_compTauIdEffPreselNumbers,
   configFileName_FWLiteTauIdEffPreselNumbers,
   logFileName_FWLiteTauIdEffPreselNumbers))
makeFile.write("\n")
makeFile.write("%s: %s\n" %
  (haddOutputFileName_stage4,
   make_MakeFile_vstring(haddInputFileNames_stage4)))
makeFile.write("\t%s%s %s &> %s\n" %
  (nice, executable_shell,
   haddShellFileName_stage4,
   haddLogFileName_stage4))
makeFile.write("\n")
for i, outputFileName in enumerate(outputFileNames_compTauIdEffFinalNumbers):
    makeFile.write("%s: %s %s %s\n" %
      (outputFileName,
       executable_compTauIdEffFinalNumbers,
       configFileNames_compTauIdEffFinalNumbers[i],
       haddOutputFileName_stage4))
    makeFile.write("\t%s%s %s &> %s\n" %
      (nice, executable_compTauIdEffFinalNumbers,
       configFileNames_compTauIdEffFinalNumbers[i],
       logFileNames_compTauIdEffFinalNumbers[i]))
makeFile.write("\n")
makeFile.write("%s: %s\n" %
  (haddOutputFileName_stage5,
   make_MakeFile_vstring(haddInputFileNames_stage5)))
makeFile.write("\t%s%s %s &> %s\n" %
  (nice, executable_shell,
   haddShellFileName_stage5,
   haddLogFileName_stage5))
makeFile.write("\n")
for i, outputFileName in enumerate(outputFileNames_makeTauIdEffFinalPlots):
    makeFile.write("%s: %s %s\n" %
      (outputFileName,
       executable_makeTauIdEffFinalPlots,
       haddOutputFileName_stage5))
    makeFile.write("\t%s%s %s &> %s\n" %
      (nice, executable_makeTauIdEffFinalPlots,
       configFileNames_makeTauIdEffFinalPlots[i],
       logFileNames_makeTauIdEffFinalPlots[i]))
makeFile.write("\n")
makeFile.write(".PHONY: clean\n")
makeFile.write("clean:\n")
makeFile.write("\trm -f %s\n" % make_MakeFile_vstring(outputFileNames_FWLiteTauIdEffAnalyzer))
makeFile.write("\trm -f %s\n" % make_MakeFile_vstring(haddInputFileNames_stage1))
makeFile.write("\trm -f %s\n" % haddShellFileName_stage1)
makeFile.write("\trm -f %s\n" % haddOutputFileName_stage1)
makeFile.write("\trm -f %s\n" % make_MakeFile_vstring(outputFileNames_smoothTauIdEffTemplates))
makeFile.write("\trm -f %s\n" % make_MakeFile_vstring(haddInputFileNames_stage2))
makeFile.write("\trm -f %s\n" % haddShellFileName_stage2)
makeFile.write("\trm -f %s\n" % haddOutputFileName_stage2)
makeFile.write("\trm -f %s\n" % make_MakeFile_vstring(outputFileNames_makeTauIdEffQCDtemplate))
makeFile.write("\trm -f %s\n" % make_MakeFile_vstring(haddInputFileNames_stage3))
makeFile.write("\trm -f %s\n" % haddShellFileName_stage3)
makeFile.write("\trm -f %s\n" % haddOutputFileName_stage3)
makeFile.write("\trm -f %s\n" % make_MakeFile_vstring(outputFileNames_fitTauIdEff))
makeFile.write("\trm -f %s\n" % outputFileName_FWLiteTauIdEffPreselNumbers)
makeFile.write("\trm -f %s\n" % make_MakeFile_vstring(haddInputFileNames_stage4))
makeFile.write("\trm -f %s\n" % haddShellFileName_stage4)
makeFile.write("\trm -f %s\n" % haddOutputFileName_stage4)
makeFile.write("\trm -f %s\n" % make_MakeFile_vstring(outputFileNames_compTauIdEffFinalNumbers))
makeFile.write("\trm -f %s\n" % make_MakeFile_vstring(haddInputFileNames_stage5))
makeFile.write("\trm -f %s\n" % haddShellFileName_stage5)
makeFile.write("\trm -f %s\n" % haddOutputFileName_stage5)
makeFile.write("\trm -f %s\n" % make_MakeFile_vstring(outputFileNames_makeTauIdEffFinalPlots))
makeFile.write("\techo 'Finished deleting old files.'\n")
makeFile.write("\n")
makeFile.close()

print("Finished building Makefile. Now execute 'make -j 8 -f %s'." % makeFileName)
