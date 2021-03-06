#ifndef TauAnalysis_TauIdEfficiency_PATPFTauSelectorForTauIdEff_h
#define TauAnalysis_TauIdEfficiency_PATPFTauSelectorForTauIdEff_h

/** \class PATPFTauSelectorForTauIdEff
 *
 * Preselect (PF)Tau-jet candidates for tau id. efficiency measurement
 *
 * NOTE: Class produces collection of pat::Tau objects the four-vectors of which 
 *       are set to (jet-energy corrected) PFJet four-vectors.
 *       The "original" components of the PFTau four-vector are stored in userFloat variables
 *       'tauPt', 'tauEta', 'tauPhi', 'tauMass'
 * 
 * \author Christian Veelken, UC Davis
 *
 * \version $Revision: 1.5 $
 *
 * $Id: PATPFTauSelectorForTauIdEff.h,v 1.5 2011/11/23 10:32:14 veelken Exp $
 *
 */

#include "FWCore/Framework/interface/EDFilter.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "CommonTools/Utils/interface/StringCutObjectSelector.h"
#include "CommonTools/Utils/interface/PtComparator.h"

#include "RecoTauTag/RecoTau/interface/RecoTauQualityCuts.h"

#include "TauAnalysis/RecoTools/interface/ParticlePFIsolationExtractor.h"

#include <string>

class PATPFTauSelectorForTauIdEff : public edm::EDFilter
{  
 public:

  explicit PATPFTauSelectorForTauIdEff(const edm::ParameterSet&);
  virtual ~PATPFTauSelectorForTauIdEff();

  bool filter(edm::Event&, const edm::EventSetup&);
  
private:

  std::string moduleLabel_;

  bool filter_;

  edm::InputTag src_;

  std::string jetEnergyCorrection_;

  double minJetPt_;
  double maxJetEta_;

  reco::tau::RecoTauQualityCuts* trackQualityCuts_;
  double minLeadTrackPt_;
  double maxDzLeadTrack_;
  double maxLeadTrackPFElectronMVA_;
  bool applyECALcrackVeto_;

  double minDeltaRtoNearestMuon_;
  StringCutObjectSelector<pat::Muon>* muonSelection_;
  edm::InputTag srcMuon_;

  ParticlePFIsolationExtractor<pat::Tau>* pfIsolationExtractor_;
  double maxPFIsoPt_;
  edm::InputTag srcPFIsoCandidates_;
  edm::InputTag srcPFNoPileUpCandidates_;
  edm::InputTag srcVertex_;
  edm::InputTag srcRhoFastJet_;

  // special flag to save pat::Taus failing selection cuts,
  // but passing tau id. discriminators
  // (for measurement of tau charge misidentification rate)
  StringCutObjectSelector<pat::Tau>* save_;

  // special flag to add userFloats to all pat::Taus
  // without applying any selection cuts
  bool produceAll_;

  // utility to sort (PF)tau-jet candidates in order of decreasing Pt
  // (taken from PhysicsTools/PatAlgos/plugins/PATTauProducer.h)
  GreaterByPt<pat::Tau> pfTauPtComparator_;
  
  int verbosity_;
};

#endif
