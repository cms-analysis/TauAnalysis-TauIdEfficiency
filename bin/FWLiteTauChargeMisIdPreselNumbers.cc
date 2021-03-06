
/** \executable FWLiteTauChargeMisIdPreselNumbers
 *
 * Determine purities in C1p and D1p regions,
 * needed for jet --> tau fake impurity correction applied in measurement of tau charge misidentification rate
 *
 * \author Christian Veelken, UC Davis
 *
 * \version $Revision: 1.6 $
 *
 * $Id: FWLiteTauChargeMisIdPreselNumbers.cc,v 1.6 2012/02/02 09:03:32 veelken Exp $
 *
 */

#include "FWCore/FWLite/interface/AutoLibraryLoader.h"

#include "DataFormats/FWLite/interface/Event.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/PythonParameterSet/interface/MakeParameterSets.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "FWCore/Utilities/interface/Exception.h"

#include "PhysicsTools/FWLite/interface/TFileService.h"
#include "DataFormats/FWLite/interface/InputSource.h"
#include "DataFormats/FWLite/interface/OutputFiles.h"

#include "DataFormats/PatCandidates/interface/Tau.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/MET.h"
#include "DataFormats/L1GlobalTrigger/interface/L1GlobalTriggerReadoutRecord.h"
#include "DataFormats/L1GlobalTrigger/interface/L1GlobalTriggerObjectMapRecord.h"
#include "DataFormats/L1GlobalTrigger/interface/L1GlobalTriggerObjectMapFwd.h"
#include "DataFormats/L1GlobalTrigger/interface/L1GlobalTriggerObjectMap.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/HepMCCandidate/interface/GenParticleFwd.h"
#include "DataFormats/Common/interface/Handle.h"

#include "PhysicsTools/SelectorUtils/interface/PFJetIDSelectionFunctor.h"

#include "TauAnalysis/TauIdEfficiency/interface/TauIdEffEventSelector.h"
#include "TauAnalysis/TauIdEfficiency/interface/TauIdEffCutFlowTable.h"
#include "TauAnalysis/CandidateTools/interface/candidateAuxFunctions.h"
#include "TauAnalysis/CandidateTools/interface/generalAuxFunctions.h"

#include "AnalysisDataFormats/TauAnalysis/interface/CompositePtrCandidateT1T2MEt.h"
#include "AnalysisDataFormats/TauAnalysis/interface/CompositePtrCandidateT1T2MEtFwd.h"

#include <TFile.h>
#include <TTree.h>
#include <TSystem.h>
#include <TROOT.h>
#include <TBenchmark.h>

typedef std::vector<std::string> vstring;
typedef std::vector<bool> vbool;

enum { kUnmatched, kTauHadMatched, kFakeTauMatched };

struct cutFlowEntryType
{
  cutFlowEntryType(const edm::ParameterSet& cfg)
  {
    binVariable_ = cfg.getParameter<std::string>("binVariable");

    std::string label = cfg.getParameter<std::string>("label");

    edm::ParameterSet cfgTauHadMatched = cfg;
    std::string labelTauHadMatched = std::string(label).append("TauHadMatched");
    cfgTauHadMatched.addParameter<std::string>("label", labelTauHadMatched);
    cutFlowTauHadMatched_ = new TauIdEffCutFlowTable(cfgTauHadMatched);

    edm::ParameterSet cfgTauHadMatchedCorrectCharge = cfg;
    std::string labelTauHadMatchedCorrectCharge = std::string(label).append("TauHadMatchedCorrectCharge");
    cfgTauHadMatchedCorrectCharge.addParameter<std::string>("label", labelTauHadMatchedCorrectCharge);
    cutFlowTauHadMatchedCorrectCharge_ = new TauIdEffCutFlowTable(cfgTauHadMatchedCorrectCharge);
    edm::ParameterSet cfgOneProngMatchedCorrectCharge = cfg;
    std::string labelOneProngMatchedCorrectCharge = std::string(label).append("OneProngMatchedCorrectCharge");
    cfgOneProngMatchedCorrectCharge.addParameter<std::string>("label", labelOneProngMatchedCorrectCharge);
    cutFlowOneProngMatchedCorrectCharge_ = new TauIdEffCutFlowTable(cfgOneProngMatchedCorrectCharge);
    edm::ParameterSet cfgThreeProngMatchedCorrectCharge = cfg;
    std::string labelThreeProngMatchedCorrectCharge = std::string(label).append("ThreeProngMatchedCorrectCharge");
    cfgThreeProngMatchedCorrectCharge.addParameter<std::string>("label", labelThreeProngMatchedCorrectCharge);
    cutFlowThreeProngMatchedCorrectCharge_ = new TauIdEffCutFlowTable(cfgThreeProngMatchedCorrectCharge);

    edm::ParameterSet cfgTauHadMatchedWrongCharge = cfg;
    std::string labelTauHadMatchedWrongCharge = std::string(label).append("TauHadMatchedWrongCharge");
    cfgTauHadMatchedWrongCharge.addParameter<std::string>("label", labelTauHadMatchedWrongCharge);
    cutFlowTauHadMatchedWrongCharge_ = new TauIdEffCutFlowTable(cfgTauHadMatchedWrongCharge);
    edm::ParameterSet cfgOneProngMatchedWrongCharge = cfg;
    std::string labelOneProngMatchedWrongCharge = std::string(label).append("OneProngMatchedWrongCharge");
    cfgOneProngMatchedWrongCharge.addParameter<std::string>("label", labelOneProngMatchedWrongCharge);
    cutFlowOneProngMatchedWrongCharge_ = new TauIdEffCutFlowTable(cfgOneProngMatchedWrongCharge);
    edm::ParameterSet cfgThreeProngMatchedWrongCharge = cfg;
    std::string labelThreeProngMatchedWrongCharge = std::string(label).append("ThreeProngMatchedWrongCharge");
    cfgThreeProngMatchedWrongCharge.addParameter<std::string>("label", labelThreeProngMatchedWrongCharge);
    cutFlowThreeProngMatchedWrongCharge_ = new TauIdEffCutFlowTable(cfgThreeProngMatchedWrongCharge);

    edm::ParameterSet cfgFakeTauMatched = cfg;
    std::string labelFakeTauMatched = std::string(label).append("FakeTauMatched");
    cfgFakeTauMatched.addParameter<std::string>("label", labelFakeTauMatched);
    cutFlowFakeTauMatched_ = new TauIdEffCutFlowTable(cfgFakeTauMatched);

    edm::ParameterSet cfgNoMatchingApplied = cfg;
    std::string labelNoMatchingApplied = std::string(label).append("NoMatchingApplied");
    cfgNoMatchingApplied.addParameter<std::string>("label", labelNoMatchingApplied);
    cutFlowNoMatchingApplied_ = new TauIdEffCutFlowTable(cfgNoMatchingApplied);
  }
  ~cutFlowEntryType()
  {
    delete cutFlowTauHadMatched_;
    delete cutFlowTauHadMatchedCorrectCharge_;
    delete cutFlowOneProngMatchedCorrectCharge_;
    delete cutFlowThreeProngMatchedCorrectCharge_;
    delete cutFlowTauHadMatchedWrongCharge_;
    delete cutFlowOneProngMatchedWrongCharge_;
    delete cutFlowThreeProngMatchedWrongCharge_;
    delete cutFlowFakeTauMatched_;
    delete cutFlowNoMatchingApplied_;
  }
  void bookCutFlowTables(TFileDirectory& dir)
  {
    cutFlowTauHadMatched_->bookCutFlowTable(dir);
    cutFlowTauHadMatchedCorrectCharge_->bookCutFlowTable(dir);
    cutFlowOneProngMatchedCorrectCharge_->bookCutFlowTable(dir);
    cutFlowThreeProngMatchedCorrectCharge_->bookCutFlowTable(dir);
    cutFlowTauHadMatchedWrongCharge_->bookCutFlowTable(dir);
    cutFlowOneProngMatchedWrongCharge_->bookCutFlowTable(dir);
    cutFlowThreeProngMatchedWrongCharge_->bookCutFlowTable(dir);
    cutFlowFakeTauMatched_->bookCutFlowTable(dir);
    cutFlowNoMatchingApplied_->bookCutFlowTable(dir);
  }
  void fillCutFlowTables(double x, 
			 const vbool& selectionFlags, 
			 int genMatchType, double genTauCharge, const std::string& genTauDecayMode, double recTauCharge,
			 double evtWeight)
  {
    if        ( genMatchType == kTauHadMatched  ) {
      cutFlowTauHadMatched_->fillCutFlowTable(x, selectionFlags, evtWeight);
      
      bool isOneProngMatched = 
	( genTauDecayMode == "oneProng0Pi0"    ||
	  genTauDecayMode == "oneProng1Pi0"    ||
	  genTauDecayMode == "oneProng2Pi0"    ||
	  genTauDecayMode == "oneProngOther"   );
      bool isThreeProngMatched = 
	( genTauDecayMode == "threeProng0Pi0"  ||
	  genTauDecayMode == "threeProngOther" ||
	  genTauDecayMode == "threeProngOther" );

      if      ( (genTauCharge*recTauCharge) > +0.5 ) {
	cutFlowTauHadMatchedCorrectCharge_->fillCutFlowTable(x, selectionFlags, evtWeight);
	if ( isOneProngMatched   ) cutFlowOneProngMatchedCorrectCharge_->fillCutFlowTable(x, selectionFlags, evtWeight);
	if ( isThreeProngMatched ) cutFlowThreeProngMatchedCorrectCharge_->fillCutFlowTable(x, selectionFlags, evtWeight);
      }
      else if ( (genTauCharge*recTauCharge) < -0.5 ) {
	cutFlowTauHadMatchedWrongCharge_->fillCutFlowTable(x, selectionFlags, evtWeight);
	if ( isOneProngMatched   ) cutFlowOneProngMatchedWrongCharge_->fillCutFlowTable(x, selectionFlags, evtWeight);
	if ( isThreeProngMatched ) cutFlowThreeProngMatchedWrongCharge_->fillCutFlowTable(x, selectionFlags, evtWeight);
      }
    } else if ( genMatchType == kFakeTauMatched ) {
      cutFlowFakeTauMatched_->fillCutFlowTable(x, selectionFlags, evtWeight);
    }

    cutFlowNoMatchingApplied_->fillCutFlowTable(x, selectionFlags, evtWeight);
  }

  std::string binVariable_;

  TauIdEffCutFlowTable* cutFlowTauHadMatched_;
  TauIdEffCutFlowTable* cutFlowTauHadMatchedCorrectCharge_;
  TauIdEffCutFlowTable* cutFlowOneProngMatchedCorrectCharge_;
  TauIdEffCutFlowTable* cutFlowThreeProngMatchedCorrectCharge_;
  TauIdEffCutFlowTable* cutFlowTauHadMatchedWrongCharge_;
  TauIdEffCutFlowTable* cutFlowOneProngMatchedWrongCharge_;
  TauIdEffCutFlowTable* cutFlowThreeProngMatchedWrongCharge_;
  TauIdEffCutFlowTable* cutFlowFakeTauMatched_;
  TauIdEffCutFlowTable* cutFlowNoMatchingApplied_;
};

struct regionEntryType
{
  regionEntryType(fwlite::TFileService& fs,
		  const std::string& process, const std::string& region, 
		  const vstring& tauIdDiscriminators, const std::string& tauIdName, const std::string& sysShift,
                  const edm::ParameterSet& cfgBinning)
    : process_(process),
      region_(region),
      tauIdDiscriminators_(tauIdDiscriminators),
      tauIdName_(tauIdName),      
      sysShift_(sysShift),
      numPreselCuts_(3),
      numTauIdDiscriminators_(tauIdDiscriminators.size()),      
      selector_(0),
      cutFlowUnbinned_(0),
      numMuTauPairs_selected_(0),
      numMuTauPairsWeighted_selected_(0.)
  {
    edm::ParameterSet cfgSelector;
    cfgSelector.addParameter<std::string>("region", region_);
    cfgSelector.addParameter<vstring>("tauIdDiscriminators", tauIdDiscriminators_);
    cfgSelector.addParameter<std::string>("tauChargeMode", "tauSignalChargedHadronSum");
    cfgSelector.addParameter<bool>("disableTauCandPreselCuts", true);

    selector_ = new TauIdEffEventSelector(cfgSelector);

    edm::ParameterSet cfgCutFlowTable = cfgBinning;
    cfgCutFlowTable.addParameter<std::string>("process", process_);
    cfgCutFlowTable.addParameter<std::string>("region", region_);
    cfgCutFlowTable.addParameter<std::string>("tauIdDiscriminator", tauIdName_);
    label_ = "";
    if ( sysShift != "CENTRAL_VALUE" ) label_.append("_").append(sysShift_);
    cfgCutFlowTable.addParameter<std::string>("label", label_);

    selectionNames_.resize(numPreselCuts_ + numTauIdDiscriminators_);
    selectionNames_[0] = std::string(region);
    selectionNames_[1] = "eVeto";
    selectionNames_[2] = "muVeto";
    for ( int iTauIdDiscriminator = 0; iTauIdDiscriminator < numTauIdDiscriminators_; ++iTauIdDiscriminator ) {
      selectionNames_[numPreselCuts_ + iTauIdDiscriminator] = tauIdDiscriminators_[iTauIdDiscriminator];
    }
    cfgCutFlowTable.addParameter<vstring>("selectionNames", selectionNames_);

    tauIdFlags_.resize(numPreselCuts_ + numTauIdDiscriminators_);

    TFileDirectory dir = fs.mkdir("presel");

    edm::ParameterSet cfgUnbinned_bin0;
    cfgUnbinned_bin0.addParameter<std::string>("subdir", "");
    cfgUnbinned_bin0.addParameter<double>("min", -0.5);
    cfgUnbinned_bin0.addParameter<double>("max", +0.5);
    typedef std::vector<edm::ParameterSet> vParameterSet;
    vParameterSet cfgUnbinned_bins;
    cfgUnbinned_bins.push_back(cfgUnbinned_bin0);
    edm::ParameterSet cfgUnbinned = cfgCutFlowTable;
    cfgUnbinned.addParameter<vParameterSet>("binning", cfgUnbinned_bins);
    cfgUnbinned.addParameter<std::string>("binVariable", "");
    cutFlowUnbinned_ = new cutFlowEntryType(cfgUnbinned);
    cutFlowUnbinned_->bookCutFlowTables(dir);

    vstring binVariableNames = cfgBinning.getParameterNamesForType<vParameterSet>();
    for ( vstring::const_iterator binVariableName = binVariableNames.begin();
	  binVariableName != binVariableNames.end(); ++binVariableName ) {
      vParameterSet cfgBinVariableBins = cfgBinning.getParameter<vParameterSet>(*binVariableName);
      edm::ParameterSet cfgBinVariable = cfgCutFlowTable;
      cfgBinVariable.addParameter<vParameterSet>("binning", cfgBinVariableBins);
      cfgBinVariable.addParameter<std::string>("binVariable", *binVariableName);
      cutFlowEntryType* cutFlowEntry = new cutFlowEntryType(cfgBinVariable);
      cutFlowEntry->bookCutFlowTables(dir);
      cutFlowEntriesBinned_.push_back(cutFlowEntry);
    }
  }
  ~regionEntryType()
  {
    delete selector_;

    delete cutFlowUnbinned_;
    for ( std::vector<cutFlowEntryType*>::iterator it = cutFlowEntriesBinned_.begin();
	  it != cutFlowEntriesBinned_.end(); ++it ) {
      delete (*it);
    }
  }
  void analyze(const PATMuTauPair& muTauPair, 
	       int genMatchType, double genTauCharge, const std::string& genTauDecayMode, double recTauCharge,
	       const pat::MET& caloMEt, 
	       size_t numJets_bTagged,
	       size_t numVertices, 
	       double evtWeight)
  {
    //std::cout << "<cutFlowEntryType::analyze>:" << std::endl;

    pat::strbitset evtSelFlags;
    if ( selector_->operator()(muTauPair, caloMEt, numJets_bTagged, evtSelFlags) ) {

//--- set flags indicating whether tau-jet candidate passes 
//    "leading" track finding, leading track Pt and loose (PF)isolation requirements 
//    plus tau id. discriminators
      tauIdFlags_[0] = true;
      tauIdFlags_[1] = (muTauPair.leg2()->userFloat("PFElectronMVA") < 0.6);
      tauIdFlags_[2] = (muTauPair.leg2()->userFloat("dRnearestMuon") > 0.5);
      for ( int iTauIdDiscriminator = 0; iTauIdDiscriminator < numTauIdDiscriminators_; ++iTauIdDiscriminator ) {
	const std::string tauIdDiscriminator = tauIdDiscriminators_[iTauIdDiscriminator];
	//std::cout << " tauIdDiscriminator = " << tauIdDiscriminator << ":" 
	//	    << " " << muTauPair.leg2()->tauID(tauIdDiscriminator.data()) << std::endl;
	tauIdFlags_[numPreselCuts_ + iTauIdDiscriminator] = (muTauPair.leg2()->tauID(tauIdDiscriminator.data()) > 0.5);
      }
      //std::cout << "tauIdFlags = " << format_vbool(tauIdFlags_) << std::endl;

//--- fill histograms for "inclusive" tau id. efficiency measurement
      cutFlowUnbinned_->fillCutFlowTables(0., 
					  tauIdFlags_, 
					  genMatchType, genTauCharge, genTauDecayMode, recTauCharge,
					  evtWeight);

//--- fill histograms for tau id. efficiency measurement as function of:
//   o tau-jet transverse momentum
//   o tau-jet pseudo-rapidity
//   o reconstructed vertex multiplicity
//   o sumEt
//   o ...
      for ( std::vector<cutFlowEntryType*>::iterator cutFlowEntry = cutFlowEntriesBinned_.begin();
	    cutFlowEntry != cutFlowEntriesBinned_.end(); ++cutFlowEntry ) {
	double x = 0.;
	if      ( (*cutFlowEntry)->binVariable_ == "tauPt"       ) x = muTauPair.leg2()->pt();
	else if ( (*cutFlowEntry)->binVariable_ == "tauAbsEta"   ) x = TMath::Abs(muTauPair.leg2()->eta());
	else if ( (*cutFlowEntry)->binVariable_ == "numVertices" ) x = numVertices;
	else if ( (*cutFlowEntry)->binVariable_ == "sumEt"       ) x = muTauPair.met()->sumEt();
	else throw cms::Exception("regionEntryType::analyze")
	  << "Invalid binVariable = " << (*cutFlowEntry)->binVariable_ << " !!\n";
	(*cutFlowEntry)->fillCutFlowTables(x, 
					   tauIdFlags_, 
					   genMatchType, genTauCharge, genTauDecayMode, recTauCharge,
					   evtWeight);
      }

      ++numMuTauPairs_selected_;
      numMuTauPairsWeighted_selected_ += evtWeight;
    }
  }

  std::string process_;
  std::string region_;
  vstring tauIdDiscriminators_;
  std::string tauIdName_;
  std::string sysShift_;
  std::string label_;
  vstring selectionNames_;
  
  int numPreselCuts_;
  int numTauIdDiscriminators_;

  TauIdEffEventSelector* selector_;

  vbool tauIdFlags_;
  
  cutFlowEntryType* cutFlowUnbinned_;
  std::vector<cutFlowEntryType*> cutFlowEntriesBinned_;

  int numMuTauPairs_selected_;
  double numMuTauPairsWeighted_selected_;
};

int getGenMatchType(const PATMuTauPair& muTauPair, const reco::GenParticleCollection& genParticles,
		    double& genTauCharge, std::string& genTauDecayMode, double& recTauCharge)
{
//--- check if reconstructed tau-jet candidate matches "true" hadronic tau decay on generator level,
//    is a "fake" tau (i.e. matches a quark/gluon/e/mu/photon on generator level)
//    or fails to be matched to any generator level object
//
//    NOTE: code to perform matching taken from TauAnalysis/Core/plugins/TauHistManager.cc
//
  //std::cout << "<getGenMatchType>:" << std::endl;

  const reco::GenParticle* matchingGenParticle = findGenParticle(muTauPair.leg2()->p4(), genParticles);
  int matchingGenParticleAbsPdgId = ( matchingGenParticle ) ?
    TMath::Abs(matchingGenParticle->pdgId()) : 0;
  //std::cout << " matchingGenParticleAbsPdgId = " << matchingGenParticleAbsPdgId << std::endl;
  
  genTauCharge = ( matchingGenParticle ) ?
    matchingGenParticle->charge() : 0.;
  recTauCharge = muTauPair.leg2()->charge();

  genTauDecayMode = ( matchingGenParticle && matchingGenParticleAbsPdgId == 15 ) ?
    getGenTauDecayMode(matchingGenParticle) : "";
  //std::cout << " genTauDecayMode = " << genTauDecayMode << std::endl;

  if ( matchingGenParticleAbsPdgId == 15 &&
       (genTauDecayMode == "oneProng0Pi0"    ||
	genTauDecayMode == "oneProng1Pi0"    ||
	genTauDecayMode == "oneProng2Pi0"    ||
	genTauDecayMode == "oneProngOther"   ||
	genTauDecayMode == "threeProng0Pi0"  ||
	genTauDecayMode == "threeProngOther" ||
	genTauDecayMode == "threeProngOther" ||
	genTauDecayMode == "rare"            ) ) return kTauHadMatched;    
  else if ( (matchingGenParticleAbsPdgId >=  1 && matchingGenParticleAbsPdgId <=  6) ||
	     matchingGenParticleAbsPdgId == 11 || matchingGenParticleAbsPdgId == 13  ||
	    (matchingGenParticleAbsPdgId == 15 && (genTauDecayMode == "electron" || genTauDecayMode == "muon")) ||
	     matchingGenParticleAbsPdgId == 21 ||
	     matchingGenParticleAbsPdgId == 22 ) return kFakeTauMatched;
  else                                           return kUnmatched;
}

int main(int argc, char* argv[]) 
{
//--- parse command-line arguments
  if ( argc < 2 ) {
    std::cout << "Usage: " << argv[0] << " [parameters.py]" << std::endl;
    return 0;
  }

  std::cout << "<FWLiteTauChargeMisIdPreselNumbers>:" << std::endl;

//--- load framework libraries
  gSystem->Load("libFWCoreFWLite");
  AutoLibraryLoader::enable();

//--- keep track of time it takes the macro to execute
  TBenchmark clock;
  clock.Start("FWLiteTauChargeMisIdPreselNumbers");

//--- read python configuration parameters
  if ( !edm::readPSetsFrom(argv[1])->existsAs<edm::ParameterSet>("process") ) 
    throw cms::Exception("FWLiteTauChargeMisIdPreselNumbers") 
      << "No ParameterSet 'process' found in configuration file = " << argv[1] << " !!\n";

  edm::ParameterSet cfg = edm::readPSetsFrom(argv[1])->getParameter<edm::ParameterSet>("process");

  edm::ParameterSet cfgTauChargeMisIdPreselNumbers = cfg.getParameter<edm::ParameterSet>("compTauChargeMisIdPreselNumbers");

  edm::InputTag srcMuTauPairs = cfgTauChargeMisIdPreselNumbers.getParameter<edm::InputTag>("srcMuTauPairs");
  edm::InputTag srcGenParticles = cfgTauChargeMisIdPreselNumbers.getParameter<edm::InputTag>("srcGenParticles");
  edm::InputTag srcHLTresults = cfgTauChargeMisIdPreselNumbers.getParameter<edm::InputTag>("srcHLTresults");
  edm::InputTag srcL1GtReadoutRecord = cfgTauChargeMisIdPreselNumbers.getParameter<edm::InputTag>("srcL1GtReadoutRecord"); 
  edm::InputTag srcL1GtObjectMapRecord = cfgTauChargeMisIdPreselNumbers.getParameter<edm::InputTag>("srcL1GtObjectMapRecord");
  vstring hltPaths = cfgTauChargeMisIdPreselNumbers.getParameter<vstring>("hltPaths");
  vstring l1Bits = cfgTauChargeMisIdPreselNumbers.getParameter<vstring>("l1Bits");
  edm::InputTag srcCaloMEt = cfgTauChargeMisIdPreselNumbers.getParameter<edm::InputTag>("srcCaloMEt");
  edm::InputTag srcGoodMuons = cfgTauChargeMisIdPreselNumbers.getParameter<edm::InputTag>("srcGoodMuons");
  edm::InputTag srcJets = cfgTauChargeMisIdPreselNumbers.getParameter<edm::InputTag>("srcJets");
  edm::ParameterSet cfgJetId;
  cfgJetId.addParameter<std::string>("version", "FIRSTDATA");
  cfgJetId.addParameter<std::string>("quality", "LOOSE");
  PFJetIDSelectionFunctor jetId(cfgJetId);
  edm::InputTag srcVertices = cfgTauChargeMisIdPreselNumbers.getParameter<edm::InputTag>("srcVertices");
  typedef std::vector<edm::InputTag> vInputTag;
  vInputTag srcWeights = cfgTauChargeMisIdPreselNumbers.getParameter<vInputTag>("weights");
  std::string sysShift = cfgTauChargeMisIdPreselNumbers.exists("sysShift") ?
    cfgTauChargeMisIdPreselNumbers.getParameter<std::string>("sysShift") : "CENTRAL_VALUE";

  fwlite::InputSource inputFiles(cfg); 
  int maxEvents = inputFiles.maxEvents();

  fwlite::OutputFiles outputFile(cfg);
  fwlite::TFileService fs = fwlite::TFileService(outputFile.file().data());

//--- initialize selections and histograms
//    for different ABCD regions
  std::vector<regionEntryType*> regionEntries;

  std::string process = cfgTauChargeMisIdPreselNumbers.getParameter<std::string>("process");
  vstring regions = cfgTauChargeMisIdPreselNumbers.getParameter<vstring>("regions");
  edm::ParameterSet cfgBinning = cfgTauChargeMisIdPreselNumbers.getParameter<edm::ParameterSet>("binning");
  typedef std::vector<edm::ParameterSet> vParameterSet;
  vParameterSet cfgTauIdDiscriminators = cfgTauChargeMisIdPreselNumbers.getParameter<vParameterSet>("tauIds");
  for ( vParameterSet::const_iterator cfgTauIdDiscriminator = cfgTauIdDiscriminators.begin();
	cfgTauIdDiscriminator != cfgTauIdDiscriminators.end(); ++cfgTauIdDiscriminator ) {
    for ( vstring::const_iterator region = regions.begin();
	  region != regions.end(); ++region ) {
      vstring tauIdDiscriminators = cfgTauIdDiscriminator->getParameter<vstring>("discriminators");
      std::string tauIdName = cfgTauIdDiscriminator->getParameter<std::string>("name");
      regionEntryType* regionEntry = 
	new regionEntryType(fs, process, *region, tauIdDiscriminators, tauIdName, sysShift, cfgBinning);
      regionEntries.push_back(regionEntry);
    }
  }

  edm::ParameterSet cfgSelectorABCD;
  cfgSelectorABCD.addParameter<std::string>("region", "ABCD");
  cfgSelectorABCD.addParameter<vstring>("tauIdDiscriminators", vstring());
  cfgSelectorABCD.addParameter<std::string>("tauChargeMode", "tauSignalChargedHadronSum");
  cfgSelectorABCD.addParameter<bool>("disableTauCandPreselCuts", true);
  TauIdEffEventSelector* selectorABCD = new TauIdEffEventSelector(cfgSelectorABCD);

  int    numEvents_processed                     = 0; 
  double numEventsWeighted_processed             = 0.;
  int    numEvents_passedTrigger                 = 0;
  double numEventsWeighted_passedTrigger         = 0.;
  int    numEvents_passedDiMuonVeto              = 0;
  double numEventsWeighted_passedDiMuonVeto      = 0.;
  int    numEvents_passedDiMuTauPairVeto         = 0;
  double numEventsWeighted_passedDiMuTauPairVeto = 0.;
  
  bool maxEvents_processed = false;
  for ( vstring::const_iterator inputFileName = inputFiles.files().begin();
	inputFileName != inputFiles.files().end() && !maxEvents_processed; ++inputFileName ) {

//--- open input file
    TFile* inputFile = TFile::Open(inputFileName->data());
    if ( !inputFile ) 
      throw cms::Exception("FWLiteTauChargeMisIdPreselNumbers") 
	<< "Failed to open inputFile = " << (*inputFileName) << " !!\n";

    std::cout << "opening inputFile = " << (*inputFileName);
    TTree* tree = dynamic_cast<TTree*>(inputFile->Get("Events"));
    if ( tree ) std::cout << " (" << tree->GetEntries() << " Events)";
    std::cout << std::endl;

    fwlite::Event evt(inputFile);
    for ( evt.toBegin(); !(evt.atEnd() || maxEvents_processed); ++evt ) {

      //std::cout << "processing run = " << evt.id().run() << ":" 
      //	  << " ls = " << evt.luminosityBlock() << ", event = " << evt.id().event() << std::endl;

//--- compute event weight
//   (pile-up reweighting, Data/MC correction factors,...)
      double evtWeight = 1.0;
      for ( vInputTag::const_iterator srcWeight = srcWeights.begin();
	    srcWeight != srcWeights.end(); ++srcWeight ) {
	edm::Handle<double> weight;
	evt.getByLabel(*srcWeight, weight);
	evtWeight *= (*weight);
      }

//--- quit event loop if maximal number of events to be processed is reached 
      ++numEvents_processed;
      numEventsWeighted_processed += evtWeight;
      if ( maxEvents > 0 && numEvents_processed >= maxEvents ) maxEvents_processed = true;

//--- check that event has passed triggers
      bool anyHLTpath_passed = false;
      if ( hltPaths.size() == 0 ) {
	anyHLTpath_passed = true;
      } else {
	edm::Handle<edm::TriggerResults> hltResults;
	evt.getByLabel(srcHLTresults, hltResults);
	
	const edm::TriggerNames& triggerNames = evt.triggerNames(*hltResults);
	
	for ( vstring::const_iterator hltPath = hltPaths.begin();
	      hltPath != hltPaths.end(); ++hltPath ) {
	  bool isHLTpath_passed = false;
	  unsigned int idx = triggerNames.triggerIndex(*hltPath);
	  if ( idx < triggerNames.size() ) isHLTpath_passed = hltResults->accept(idx);
	  if ( isHLTpath_passed ) anyHLTpath_passed = true;
	}
      }

      if ( !anyHLTpath_passed ) continue;

      bool anyL1bit_passed = false;
      if ( l1Bits.size() == 0 ) {
	anyL1bit_passed = true;
      } else {
	edm::Handle<L1GlobalTriggerReadoutRecord> l1GtReadoutRecord;
	evt.getByLabel(srcL1GtReadoutRecord, l1GtReadoutRecord);
	edm::Handle<L1GlobalTriggerObjectMapRecord> l1GtObjectMapRecord;
	evt.getByLabel(srcL1GtObjectMapRecord, l1GtObjectMapRecord);

	DecisionWord l1GtDecision = l1GtReadoutRecord->decisionWord();
	const std::vector<L1GlobalTriggerObjectMap>& l1GtObjectMaps = l1GtObjectMapRecord->gtObjectMap();

	for ( vstring::const_iterator l1Bit = l1Bits.begin();
	      l1Bit != l1Bits.end(); ++l1Bit ) {	  
	  for ( std::vector<L1GlobalTriggerObjectMap>::const_iterator l1GtObjectMap = l1GtObjectMaps.begin();
		l1GtObjectMap != l1GtObjectMaps.end(); ++l1GtObjectMap ) {
	    bool isL1bit_passed = false;
	    std::string l1Bit_idx = (*l1GtObjectMap).algoName();
	    int idx = (*l1GtObjectMap).algoBitNumber();	    
	    if ( l1Bit_idx == (*l1Bit) ) isL1bit_passed = l1GtDecision[idx];
	    if ( isL1bit_passed ) anyL1bit_passed = true;
	  }
	}
      }

      if ( !anyL1bit_passed ) continue;

      ++numEvents_passedTrigger;
      numEventsWeighted_passedTrigger += evtWeight;

//--- require event to contain only one "good quality" muon
      typedef std::vector<pat::Muon> PATMuonCollection;
      edm::Handle<PATMuonCollection> goodMuons;
      evt.getByLabel(srcGoodMuons, goodMuons);
      size_t numGoodMuons = goodMuons->size();
	
      if ( !(numGoodMuons <= 1) ) continue;
      ++numEvents_passedDiMuonVeto;
      numEventsWeighted_passedDiMuonVeto += evtWeight;

//--- require event to contain exactly one muon + tau-jet pair
//    passing the selection criteria for region "ABCD"
      edm::Handle<PATMuTauPairCollection> muTauPairs;
      evt.getByLabel(srcMuTauPairs, muTauPairs);

      typedef std::vector<pat::MET> PATMETCollection;
      edm::Handle<PATMETCollection> caloMEt;
      evt.getByLabel(srcCaloMEt, caloMEt);
      if ( caloMEt->size() != 1 )
	throw cms::Exception("FWLiteTauChargeMisIdPreselNumbers")
	  << "Failed to find unique CaloMEt object !!\n";

      unsigned numMuTauPairsABCD = 0; // Note: no b-jet veto applied
      for ( PATMuTauPairCollection::const_iterator muTauPair = muTauPairs->begin();
	    muTauPair != muTauPairs->end(); ++muTauPair ) {
	pat::strbitset evtSelFlags;
	if ( selectorABCD->operator()(*muTauPair, caloMEt->front(), 0, evtSelFlags) ) ++numMuTauPairsABCD;
      }
      
      if ( !(numMuTauPairsABCD <= 1) ) continue;
      ++numEvents_passedDiMuTauPairVeto;
      numEventsWeighted_passedDiMuTauPairVeto += evtWeight;

      edm::Handle<pat::JetCollection> jets;
      evt.getByLabel(srcJets, jets);      

//--- determine number of vertices reconstructed in the event
//   (needed to parametrize dependency of tau id. efficiency on number of pile-up interactions)
      edm::Handle<reco::VertexCollection> vertices;
      evt.getByLabel(srcVertices, vertices);
      size_t numVertices = vertices->size();

//--- iterate over collection of muon + tau-jet pairs:
//    check which region muon + tau-jet pair is selected in
//    and whether reconstructed tau-jet matches "true" hadronic tau decay on generator level or is fake,
//    count number of "true" and fake taus selected in all regions
      edm::Handle<reco::GenParticleCollection> genParticles;
      evt.getByLabel(srcGenParticles, genParticles);

      int muTauPairIdx = 0;
      for ( PATMuTauPairCollection::const_iterator muTauPair = muTauPairs->begin();
	    muTauPair != muTauPairs->end(); ++muTauPair, ++muTauPairIdx ) {

//--- require event to contain to b-jets
//   (not overlapping with muon or tau-jet candidate)
	size_t numJets_bTagged = 0;
	for ( pat::JetCollection::const_iterator jet = jets->begin();
	      jet != jets->end(); ++jet ) {
	  if ( jet->pt() > 30. && TMath::Abs(jet->eta()) < 2.4 && jetId(*jet) &&
	       jet->bDiscriminator("combinedSecondaryVertexBJetTags") > 0.679 ) ++numJets_bTagged; // "medium" WP
	}
	
	double genTauCharge, recTauCharge;
	std::string genTauDecayMode;
	int genMatchType = getGenMatchType(*muTauPair, *genParticles, genTauCharge, genTauDecayMode, recTauCharge);
	for ( std::vector<regionEntryType*>::iterator regionEntry = regionEntries.begin();
	      regionEntry != regionEntries.end(); ++regionEntry ) {   
	  (*regionEntry)->analyze(*muTauPair, 
				  genMatchType, genTauCharge, genTauDecayMode, recTauCharge, 
				  caloMEt->front(), 
				  numJets_bTagged,
				  numVertices, 
				  evtWeight);
        }
      }
    }

//--- close input file
    delete inputFile;
  }

  std::cout << "<FWLiteTauChargeMisIdPreselNumbers>:" << std::endl;
  std::cout << " numEvents_processed: " << numEvents_processed 
	    << " (weighted = " << numEventsWeighted_processed << ")" << std::endl;
  std::cout << " numEvents_passedTrigger: " << numEvents_passedTrigger 
	    << " (weighted = " << numEventsWeighted_passedTrigger << ")" << std::endl;
  std::cout << " numEvents_passedDiMuonVeto: " << numEvents_passedDiMuonVeto 
	    << " (weighted = " << numEventsWeighted_passedDiMuonVeto << ")" << std::endl;
  std::cout << " numEvents_passedDiMuTauPairVeto: " << numEvents_passedDiMuTauPairVeto
	    << " (weighted = " << numEventsWeighted_passedDiMuTauPairVeto << ")" << std::endl;
  std::string lastTauIdName = "";
  for ( std::vector<regionEntryType*>::iterator regionEntry = regionEntries.begin();
	regionEntry != regionEntries.end(); ++regionEntry ) {
    if ( (*regionEntry)->tauIdName_ != lastTauIdName ) 
      std::cout << " numMuTauPairs_selected, " << (*regionEntry)->tauIdName_ << std::endl;
    std::cout << "  region " << (*regionEntry)->region_ << ":" << std::endl;
    std::cout << "   entries = " << (*regionEntry)->numMuTauPairs_selected_ 
	      << " (weighted = " << (*regionEntry)->numMuTauPairsWeighted_selected_ << ")" << std::endl;
    TauIdEffCutFlowTable* cutFlowTableTauHadMatched = (*regionEntry)->cutFlowUnbinned_->cutFlowTauHadMatched_;
    std::cout << "   tauHadMatched (weighted) = " << cutFlowTableTauHadMatched->getCutFlowNumber(0, 0) << std::endl;
    TauIdEffCutFlowTable* cutFlowTableFakeTauMatched = (*regionEntry)->cutFlowUnbinned_->cutFlowFakeTauMatched_;
    std::cout << "   fakeTauMatched (weighted) = " << cutFlowTableFakeTauMatched->getCutFlowNumber(0, 0) << std::endl;
    TauIdEffCutFlowTable* cutFlowNoMatchingApplied = (*regionEntry)->cutFlowUnbinned_->cutFlowNoMatchingApplied_;
    std::cout << "   noMatchingApplied (weighted) = " << cutFlowNoMatchingApplied->getCutFlowNumber(0, 0) << std::endl;
    double purity_high = cutFlowTableTauHadMatched->getCutFlowNumber(0, 4)/
      (cutFlowTableTauHadMatched->getCutFlowNumber(0, 4) + cutFlowTableFakeTauMatched->getCutFlowNumber(0, 4));
    std::cout << "   purity (tauHadMatched/(tauHadMatched + fakeTauMatched) = " << purity_high << std::endl;
    double purity_low = cutFlowTableTauHadMatched->getCutFlowNumber(0, 4)/
      cutFlowNoMatchingApplied->getCutFlowNumber(0, 4);
    std::cout << "   purity (tauHadMatched/(noMatchingApplied) = " << purity_low << std::endl;
    lastTauIdName = (*regionEntry)->tauIdName_;
  }

  clock.Show("FWLiteTauChargeMisIdPreselNumbers");

  return 0;
}

