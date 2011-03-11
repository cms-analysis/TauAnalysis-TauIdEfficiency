import FWCore.ParameterSet.Config as cms

#--------------------------------------------------------------------------------
# define variables specific to PFTaus
# reconstructed by fixed signal cone algorithm
#--------------------------------------------------------------------------------

pfTausFixedCone_recInfo = cms.PSet(
    # Select multiplicy of object(s) to store
    vector = cms.bool(True), # Store a value for all objects in this collection
    #indices = cms.vuint_32([0, 1, 2]) # Store values for first, second, third objects
    
    # Extractor plugin
    pluginType = cms.string("PATTauVectorValExtractor"),
    
    # Collection to extract from
    src = cms.InputTag("patPFTausDijetTagAndProbeFixedCone"),
    
    # Variables to compute for this source
    columns = cms.PSet(
        # kinematic variables for PFTau
        pt = cms.string("pt()"),
        eta = cms.string("eta()"),
        phi = cms.string("phi()"),
        mass = cms.string("mass()"),

        # charge of PFTau
        # (sum of charges of charged particles within signal cone)
        charge = cms.string("charge()"),

        # transverse momentum of leading charged/neutral particle within signal cone
        leadChargedParticlePt = cms.string("? leadPFChargedHadrCand().isNonnull() ? leadPFChargedHadrCand().pt() : 0."),
        leadNeutralParticlePt = cms.string("? leadPFNeutralCand().isNonnull() ? leadPFNeutralCand().pt() : 0."),
        leadParticlePt = cms.string("? leadPFCand().isNonnull() ? leadPFCand().pt() : 0."),

        # multiplicity and Pt sum of charged/neutral particles within signal and isolation cones
        numChargedParticlesSignalCone = cms.string("signalPFChargedHadrCands().size()"),
        numNeutralHadronsSignalCone = cms.string("signalPFNeutrHadrCands().size()"),
        numPhotonsSignalCone = cms.string("signalPFGammaCands().size()"),
        numParticlesSignalCone = cms.string("signalPFCands().size()"),

        numChargedParticlesIsoCone = cms.string("isolationPFChargedHadrCands().size()"),
        numNeutralHadronsIsoCone = cms.string("isolationPFNeutrHadrCands().size()"),
        numPhotonsIsoCone = cms.string("isolationPFGammaCands().size()"),
        numParticlesIsoCone = cms.string("isolationPFCands().size()"),

        numChargedParticles = cms.string("signalPFChargedHadrCands().size() + isolationPFChargedHadrCands().size()"),
        numNeutralHadrons = cms.string("signalPFNeutrHadrCands().size() + isolationPFNeutrHadrCands().size()"),
        numPhotons = cms.string("signalPFGammaCands().size() + isolationPFGammaCands().size()"),
        numParticles = cms.string("signalPFCands().size() + isolationPFCands().size()"),

        ptSumChargedParticlesIsoCone = cms.string("isolationPFChargedHadrCandsPtSum()"),
        ptSumPhotonsIsoCone = cms.string("isolationPFGammaCandsEtSum()"),

        # kinematic variables for PFJet associated to PFTau
        jetPt = cms.string("pfJetRef().pt()"),
        jetEta = cms.string("pfJetRef().eta()"),
        jetPhi = cms.string("pfJetRef().phi()"),

        # jet width
	jetWidth = cms.string("? (etaetaMoment() + phiphiMoment()) > 0. ? sqrt(etaetaMoment() + phiphiMoment()) : 0."),

        # loose PFIsolation Pt sum
        # (computed by summing PFChargedHadrons of Pt > 1.0 GeV + PFGammas of Pt > 1.5 GeV
        #  in pfNoPileUp collection within region 0.15 < dR < 0.4/0.6 centered on PFTau direction)
        ptSumLooseIsolation04 = cms.string("userFloat('pfLooseIsoPt04')"),
        ptSumLooseIsolation06 = cms.string("userFloat('pfLooseIsoPt06')"),
        
        # flags for tag/probe and Pt_index for distinguishing between
        # highest Pt, second highest Pt and third highest Pt jet
        tagJet15U = cms.string("userFloat('tagJet15U')"),
        probeJet15U = cms.string("userFloat('probeJet15U')"),
        tagJet30U = cms.string("userFloat('tagJet30U')"),
        probeJet30U = cms.string("userFloat('probeJet30U')"),
        tagJet50U = cms.string("userFloat('tagJet50U')"),
        probeJet50U = cms.string("userFloat('probeJet50U')"),
        tagJet70U = cms.string("userFloat('tagJet70U')"),
        probeJet70U = cms.string("userFloat('probeJet70U')"),
        tagJet100U = cms.string("userFloat('tagJet100U')"),
        probeJet100U = cms.string("userFloat('probeJet100U')"),
        ptIndex = cms.string("userFloat('pt_index')"),
                        
        # tau id. discriminators based on leading track/PFChargedHadron                                                 
        byLeadTrackFinding = cms.string("tauID('leadingTrackFinding')"),
        byLeadTrackPtCut = cms.string("tauID('leadingTrackPtCut')"),
        byTrackIsolation = cms.string("tauID('trackIsolation')"),
        byEcalIsolation = cms.string("tauID('ecalIsolation')"),
        byIsolation = cms.string("tauID('byIsolation')"),
        
        # tau id. discriminators based on leading pion (PFChargedHadron or PFGamma)                                    
        byLeadPionPtCut = cms.string("tauID('leadingPionPtCut')"),
        byTrackIsolationUsingLeadingPion = cms.string("tauID('trackIsolationUsingLeadingPion')"),
        byEcalIsolationUsingLeadingPion = cms.string("tauID('ecalIsolationUsingLeadingPion')"),
        byIsolationUsingLeadingPion = cms.string("tauID('byIsolationUsingLeadingPion')"),
        
        # discriminators against electrons/muons
        againstElectron = cms.string("tauID('againstElectron')"),
        pfElectronMVA = cms.string("? leadPFCand().isNonnull() ? leadPFCand().mva_e_pi() : +1."),
        againstMuon = cms.string("tauID('againstMuon')")                             
    )
)

pfTausFixedCone_recJetIdInfo = cms.PSet(
    # Select multiplicy of object(s) to store
    vector = cms.bool(True), # Store a value for all objects in this collection
    #indices = cms.vuint_32([0, 1, 2]) # Store values for first, second, third objects
    
    # Extractor plugin
    pluginType = cms.string("PATTauVectorPFJetIdValExtractor"),
    
    # Collection to extract from
    src = cms.InputTag("patPFTausDijetTagAndProbeFixedCone"),
    srcJet = cms.InputTag("patJetsAK5PF"),
    
    # Variables to compute for this source
    columns = cms.PSet(
        # jetId bits
        jetIdLoose = cms.string("loose"),
        jetIdTight = cms.string("tight")
    )
)   

pfTausFixedCone_genInfo = pfTausFixedCone_recInfo.clone(
    pluginType = cms.string("PATTauVectorGenJetValExtractor"),

    columns = cms.PSet(
        # generator level information
        genMatch = cms.string("genMatch"),
        
        genPt = cms.string("genPt"),
        genEta = cms.string("genEta"),
        genPhi = cms.string("genPhi"),
        
        genDecayMode = cms.string("genDecayMode"),

        genPdgId = cms.string("genPdgId")
    ),
    
    srcGenParticles = cms.InputTag('genParticles'),
    skipPdgIdsGenParticleMatch = cms.vint32( 12, 14, 16 )
)    

pfTausFixedCone_mcEmbeddingInfo = pfTausFixedCone_recInfo.clone(
    pluginType = cms.string("PATTauVectorValExtractor"),

    columns = cms.PSet(
        # flags for tag/probe
        tag = cms.string("userFloat('tag')"),
        probe = cms.string("userFloat('probe')"),
    )
)    
