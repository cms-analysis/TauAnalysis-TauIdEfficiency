
/** \executable FWLiteTauIdEffAnalyzer
 *
 * Apply event selections for ABCD regions 
 * and fill histograms for tau id. efficiency measurement
 *
 * \author Christian Veelken, UC Davis
 *
 * \version $Revision: 1.12 $
 *
 * $Id: FWLiteTauIdEffAnalyzer.cc,v 1.12 2011/07/10 15:47:27 veelken Exp $
 *
 */

#include "FWCore/FWLite/interface/AutoLibraryLoader.h"

#include "DataFormats/FWLite/interface/Event.h"
#include "DataFormats/FWLite/interface/LuminosityBlock.h"
#include "DataFormats/FWLite/interface/Run.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/PythonParameterSet/interface/MakeParameterSets.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "FWCore/Utilities/interface/Exception.h"

#include "PhysicsTools/FWLite/interface/TFileService.h"
#include "DataFormats/FWLite/interface/InputSource.h"
#include "DataFormats/FWLite/interface/OutputFiles.h"

#include "DataFormats/PatCandidates/interface/Tau.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/TriggerEvent.h"
#include "DataFormats/PatCandidates/interface/TriggerAlgorithm.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/Common/interface/MergeableCounter.h"
#include "DataFormats/Luminosity/interface/LumiSummary.h"
#include "DataFormats/Common/interface/Handle.h"

#include "TauAnalysis/TauIdEfficiency/interface/TauIdEffEventSelector.h"
#include "TauAnalysis/TauIdEfficiency/interface/TauIdEffHistManager.h"

#include "AnalysisDataFormats/TauAnalysis/interface/CompositePtrCandidateT1T2MEt.h"
#include "AnalysisDataFormats/TauAnalysis/interface/CompositePtrCandidateT1T2MEtFwd.h"

#include <TFile.h>
#include <TTree.h>
#include <TSystem.h>
#include <TROOT.h>
#include <TBenchmark.h>

typedef std::vector<std::string> vstring;

struct regionEntryType
{
  regionEntryType(fwlite::TFileService& fs,
		  const std::string& process, const std::string& region, 
		  const vstring& tauIdDiscriminators, const std::string& tauIdName, const std::string& sysShift)
    : process_(process),
      region_(region),
      tauIdDiscriminators_(tauIdDiscriminators),
      tauIdName_(tauIdName),
      sysShift_(sysShift),
      selector_(0),
      histManager_(0),
      histManagerTauEtaLt15_(0),
      histManagerTauEta15to19_(0),
      histManagerTauEta19to23_(0),
      histManagerTauPtLt25_(0),
      histManagerTauPt25to30_(0),
      histManagerTauPt30to40_(0),
      histManagerTauPtGt40_(0),
      histManagerSumEtLt250_(0),
      histManagerSumEt250to350_(0),
      histManagerSumEt350to450_(0),
      histManagerSumEtGt450_(0),
      histManagerNumVerticesLeq4_(0),
      histManagerNumVertices5to6_(0),
      histManagerNumVertices7to8_(0),
      histManagerNumVerticesGt8_(0),
      numMuTauPairs_selected_(0),
      numMuTauPairsWeighted_selected_(0.)
  {
    edm::ParameterSet cfgSelector;
    cfgSelector.addParameter<vstring>("tauIdDiscriminators", tauIdDiscriminators_);
    cfgSelector.addParameter<std::string>("region", region_);

    selector_ = new TauIdEffEventSelector(cfgSelector);

    edm::ParameterSet cfgHistManager;
    cfgHistManager.addParameter<std::string>("process", process_);
    cfgHistManager.addParameter<std::string>("region", region_);
    cfgHistManager.addParameter<std::string>("tauIdDiscriminator", tauIdName_);
    if      ( region.find("p") != std::string::npos ) label_ = "passed";
    else if ( region.find("f") != std::string::npos ) label_ = "failed";
    else                                              label_ = "all";
    if ( sysShift_ != "CENTRAL_VALUE" ) label_.append("_").append(sysShift_);
    cfgHistManager.addParameter<std::string>("label", label_);

    histManager_                = addHistManager(fs, "",                cfgHistManager);

    histManagerTauEtaLt15_      = addHistManager(fs, "tauEtaLt15",      cfgHistManager);
    histManagerTauEta15to19_    = addHistManager(fs, "tauEta15to19",    cfgHistManager);
    histManagerTauEta19to23_    = addHistManager(fs, "tauEta19to23",    cfgHistManager);

    histManagerTauPtLt25_       = addHistManager(fs, "tauPtLt25",       cfgHistManager);
    histManagerTauPt25to30_     = addHistManager(fs, "tauPt25to30",     cfgHistManager);
    histManagerTauPt30to40_     = addHistManager(fs, "tauPt30to40",     cfgHistManager);
    histManagerTauPtGt40_       = addHistManager(fs, "tauPtGt40",       cfgHistManager);

    histManagerSumEtLt250_      = addHistManager(fs, "sumEtLt250",      cfgHistManager);
    histManagerSumEt250to350_   = addHistManager(fs, "sumEt250to350",   cfgHistManager);
    histManagerSumEt350to450_   = addHistManager(fs, "sumEt350to450",   cfgHistManager);
    histManagerSumEtGt450_      = addHistManager(fs, "sumEtGt450",      cfgHistManager);

    histManagerNumVerticesLeq4_ = addHistManager(fs, "numVerticesLeq4", cfgHistManager);
    histManagerNumVertices5to6_ = addHistManager(fs, "numVertices5to6", cfgHistManager);
    histManagerNumVertices7to8_ = addHistManager(fs, "numVertices7to8", cfgHistManager);
    histManagerNumVerticesGt8_  = addHistManager(fs, "numVerticesGt8",  cfgHistManager);
  }
  ~regionEntryType()
  {
    delete selector_;

    delete histManager_;

    delete histManagerTauEtaLt15_;
    delete histManagerTauEta15to19_;
    delete histManagerTauEta19to23_;

    delete histManagerTauPtLt25_;
    delete histManagerTauPt25to30_;
    delete histManagerTauPt30to40_;
    delete histManagerTauPtGt40_;

    delete histManagerSumEtLt250_;
    delete histManagerSumEt250to350_;
    delete histManagerSumEt350to450_;
    delete histManagerSumEtGt450_;

    delete histManagerNumVerticesLeq4_;
    delete histManagerNumVertices5to6_;
    delete histManagerNumVertices7to8_;
    delete histManagerNumVerticesGt8_;
  }
  TauIdEffHistManager* addHistManager(fwlite::TFileService& fs, const std::string& dirName, const edm::ParameterSet& cfg)
  {
    TauIdEffHistManager* retVal = new TauIdEffHistManager(cfg);

    if ( dirName != "" ) {
      TFileDirectory dir = fs.mkdir(dirName.data());
      retVal->bookHistograms(dir);
    } else {
      retVal->bookHistograms(fs);
    }

    return retVal;
  }
  void analyze(const PATMuTauPair& muTauPair, size_t numVertices, double evtWeight)
  {
    pat::strbitset evtSelFlags;
    if ( selector_->operator()(muTauPair, evtSelFlags) ) {
//--- fill histograms for "inclusive" tau id. efficiency measurement
      histManager_->fillHistograms(muTauPair, numVertices, evtWeight);

//--- fill histograms for tau id. efficiency measurement as function of tau-jet pseudo-rapidity
      double tauAbsEta = TMath::Abs(muTauPair.leg1()->eta());
      if      ( tauAbsEta < 1.5 ) histManagerTauEtaLt15_->fillHistograms(muTauPair, numVertices, evtWeight);
      else if ( tauAbsEta < 1.9 ) histManagerTauEta15to19_->fillHistograms(muTauPair, numVertices, evtWeight);
      else if ( tauAbsEta < 2.3 ) histManagerTauEta19to23_->fillHistograms(muTauPair, numVertices, evtWeight);

//--- fill histograms for tau id. efficiency measurement as function of tau-jet transverse momentum
      double tauPt = muTauPair.leg1()->pt();
      if      ( tauPt > 20.0 && tauPt <  25.0 ) histManagerTauPtLt25_->fillHistograms(muTauPair, numVertices, evtWeight);
      else if ( tauPt > 25.0 && tauPt <  30.0 ) histManagerTauPt25to30_->fillHistograms(muTauPair, numVertices, evtWeight);
      else if ( tauPt > 30.0 && tauPt <  40.0 ) histManagerTauPt30to40_->fillHistograms(muTauPair, numVertices, evtWeight);
      else if ( tauPt > 40.0 && tauPt < 100.0 ) histManagerTauPtGt40_->fillHistograms(muTauPair, numVertices, evtWeight);
      
//--- fill histograms for tau id. efficiency measurement as function of sumEt
      double sumEt = muTauPair.met()->sumEt();
      if      ( sumEt <  250.0 ) histManagerSumEtLt250_->fillHistograms(muTauPair, numVertices, evtWeight);
      else if ( sumEt <  350.0 ) histManagerSumEt250to350_->fillHistograms(muTauPair, numVertices, evtWeight);
      else if ( sumEt <  450.0 ) histManagerSumEt350to450_->fillHistograms(muTauPair, numVertices, evtWeight);
      else if ( sumEt < 1000.0 ) histManagerSumEtGt450_->fillHistograms(muTauPair, numVertices, evtWeight);
      
//--- fill histograms for tau id. efficiency measurement as function of reconstructed vertex multiplicity
      if      ( numVertices <=  4 ) histManagerNumVerticesLeq4_->fillHistograms(muTauPair, numVertices, evtWeight);
      else if ( numVertices <=  6 ) histManagerNumVertices5to6_->fillHistograms(muTauPair, numVertices, evtWeight);
      else if ( numVertices <=  8 ) histManagerNumVertices7to8_->fillHistograms(muTauPair, numVertices, evtWeight);
      else if ( numVertices <= 20 ) histManagerNumVerticesGt8_->fillHistograms(muTauPair, numVertices, evtWeight);
 
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

  TauIdEffEventSelector* selector_;

  TauIdEffHistManager* histManager_;

  TauIdEffHistManager* histManagerTauEtaLt15_;
  TauIdEffHistManager* histManagerTauEta15to19_;
  TauIdEffHistManager* histManagerTauEta19to23_;

  TauIdEffHistManager* histManagerTauPtLt25_;
  TauIdEffHistManager* histManagerTauPt25to30_;
  TauIdEffHistManager* histManagerTauPt30to40_;
  TauIdEffHistManager* histManagerTauPtGt40_;

  TauIdEffHistManager* histManagerSumEtLt250_;
  TauIdEffHistManager* histManagerSumEt250to350_;
  TauIdEffHistManager* histManagerSumEt350to450_;
  TauIdEffHistManager* histManagerSumEtGt450_;

  TauIdEffHistManager* histManagerNumVerticesLeq4_;
  TauIdEffHistManager* histManagerNumVertices5to6_;
  TauIdEffHistManager* histManagerNumVertices7to8_;
  TauIdEffHistManager* histManagerNumVerticesGt8_;

  int numMuTauPairs_selected_;
  double numMuTauPairsWeighted_selected_;
};

int main(int argc, char* argv[]) 
{
//--- parse command-line arguments
  if ( argc < 2 ) {
    std::cout << "Usage: " << argv[0] << " [parameters.py]" << std::endl;
    return 0;
  }

  std::cout << "<FWLiteTauIdEffAnalyzer>:" << std::endl;

//--- load framework libraries
  gSystem->Load("libFWCoreFWLite");
  AutoLibraryLoader::enable();

//--- keep track of time it takes the macro to execute
  TBenchmark clock;
  clock.Start("FWLiteTauIdEffAnalyzer");

//--- read python configuration parameters
  if ( !edm::readPSetsFrom(argv[1])->existsAs<edm::ParameterSet>("process") ) 
    throw cms::Exception("FWLiteTauIdEffAnalyzer") 
      << "No ParameterSet 'process' found in configuration file = " << argv[1] << " !!\n";

  edm::ParameterSet cfg = edm::readPSetsFrom(argv[1])->getParameter<edm::ParameterSet>("process");

  edm::ParameterSet cfgTauIdEffAnalyzer = cfg.getParameter<edm::ParameterSet>("tauIdEffAnalyzer");

  edm::InputTag srcMuTauPairs = cfgTauIdEffAnalyzer.getParameter<edm::InputTag>("srcMuTauPairs");
  edm::InputTag srcTrigger = cfgTauIdEffAnalyzer.getParameter<edm::InputTag>("srcTrigger");
  vstring hltPaths = cfgTauIdEffAnalyzer.getParameter<vstring>("hltPaths");
  edm::InputTag srcGoodMuons = cfgTauIdEffAnalyzer.getParameter<edm::InputTag>("srcGoodMuons");
  edm::InputTag srcVertices = cfgTauIdEffAnalyzer.getParameter<edm::InputTag>("srcVertices");
  typedef std::vector<edm::InputTag> vInputTag;
  vInputTag srcWeights = cfgTauIdEffAnalyzer.getParameter<vInputTag>("weights");
  std::string sysShift = cfgTauIdEffAnalyzer.exists("sysShift") ?
    cfgTauIdEffAnalyzer.getParameter<std::string>("sysShift") : "CENTRAL_VALUE";
  edm::InputTag srcEventCounter = cfgTauIdEffAnalyzer.getParameter<edm::InputTag>("srcEventCounter");

  fwlite::InputSource inputFiles(cfg); 
  int maxEvents = inputFiles.maxEvents();

  fwlite::OutputFiles outputFile(cfg);
  fwlite::TFileService fs = fwlite::TFileService(outputFile.file().data());

//--- initialize selections and histograms
//    for different ABCD regions
  std::vector<regionEntryType*> regionEntries;

  std::string process = cfgTauIdEffAnalyzer.getParameter<std::string>("process");
  std::cout << " process = " << process << std::endl;
  std::string processType = cfgTauIdEffAnalyzer.getParameter<std::string>("type");
  std::cout << " type = " << processType << std::endl;
  bool isData = (processType == "Data");
  vstring regions = cfgTauIdEffAnalyzer.getParameter<vstring>("regions");
  typedef std::vector<edm::ParameterSet> vParameterSet;
  vParameterSet cfgTauIdDiscriminators = cfgTauIdEffAnalyzer.getParameter<vParameterSet>("tauIds");
  for ( vParameterSet::const_iterator cfgTauIdDiscriminator = cfgTauIdDiscriminators.begin();
	cfgTauIdDiscriminator != cfgTauIdDiscriminators.end(); ++cfgTauIdDiscriminator ) {
    for ( vstring::const_iterator region = regions.begin();
	  region != regions.end(); ++region ) {
      vstring tauIdDiscriminators = cfgTauIdDiscriminator->getParameter<vstring>("discriminators");
      std::string tauIdName = cfgTauIdDiscriminator->getParameter<std::string>("name");
      regionEntryType* regionEntry = new regionEntryType(fs, process, *region, tauIdDiscriminators, tauIdName, sysShift);
      regionEntries.push_back(regionEntry);
    }
  }

  edm::ParameterSet cfgSelectorABCD;
  cfgSelectorABCD.addParameter<vstring>("tauIdDiscriminators", vstring());
  cfgSelectorABCD.addParameter<std::string>("region", "ABCD");
  TauIdEffEventSelector* selectorABCD = new TauIdEffEventSelector(cfgSelectorABCD);

//--- book "dummy" histogram counting number of processed events
  TH1* histogramEventCounter = fs.make<TH1F>("numEventsProcessed", "Number of processed Events", 3, -0.5, +2.5);
  histogramEventCounter->GetXaxis()->SetBinLabel(1, "all Events (DBS)");      // CV: bin numbers start at 1 (not 0) !!
  histogramEventCounter->GetXaxis()->SetBinLabel(2, "processed by Skimming");
  histogramEventCounter->GetXaxis()->SetBinLabel(3, "analyzed in PAT-tuple");
  
  int allEvents_DBS = cfgTauIdEffAnalyzer.getParameter<int>("allEvents_DBS");
  if ( allEvents_DBS > 0 ) {
    histogramEventCounter->SetBinContent(1, cfgTauIdEffAnalyzer.getParameter<int>("allEvents_DBS"));
  } else {
    histogramEventCounter->SetBinContent(1, -1.);
  }
  
  double xSection = cfgTauIdEffAnalyzer.getParameter<double>("xSection");
  double intLumiData = cfgTauIdEffAnalyzer.getParameter<double>("intLumiData");

  int    numEvents_processed                     = 0; 
  double numEventsWeighted_processed             = 0.;
  int    numEvents_passedTrigger                 = 0;
  double numEventsWeighted_passedTrigger         = 0.;
  int    numEvents_passedDiMuonVeto              = 0;
  double numEventsWeighted_passedDiMuonVeto      = 0.;
  int    numEvents_passedDiMuTauPairVeto         = 0;
  double numEventsWeighted_passedDiMuTauPairVeto = 0.;

  edm::RunNumber_t lastLumiBlock_run = -1;
  edm::LuminosityBlockNumber_t lastLumiBlock_ls = -1;

  double intLumiData_analyzed = 0.;
  edm::InputTag srcLumiProducer = cfgTauIdEffAnalyzer.getParameter<edm::InputTag>("srcLumiProducer");

  bool maxEvents_processed = false;
  for ( vstring::const_iterator inputFileName = inputFiles.files().begin();
	inputFileName != inputFiles.files().end() && !maxEvents_processed; ++inputFileName ) {

//--- open input file
    TFile* inputFile = TFile::Open(inputFileName->data());
    if ( !inputFile ) 
      throw cms::Exception("FWLiteTauIdEffAnalyzer") 
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

//--- check if new luminosity section has started;
//    if so, retrieve number of events contained in this luminosity section before skimming
      if ( !(evt.id().run() == lastLumiBlock_run && evt.luminosityBlock() == lastLumiBlock_ls) ) {
	const fwlite::LuminosityBlock& ls = evt.getLuminosityBlock();
	edm::Handle<edm::MergeableCounter> numEvents_skimmed;
	ls.getByLabel(srcEventCounter, numEvents_skimmed);
	if ( numEvents_skimmed.isValid() ) histogramEventCounter->Fill(1, numEvents_skimmed->value);
	lastLumiBlock_run = evt.id().run();
	lastLumiBlock_ls = evt.luminosityBlock();

	if ( isData ) {
	  edm::Handle<LumiSummary> lumiSummary;
	  edm::InputTag srcLumiProducer("lumiProducer");
	  ls.getByLabel(srcLumiProducer, lumiSummary);
	  intLumiData_analyzed += lumiSummary->intgRecLumi();
	}
      }

//--- fill "dummy" histogram counting number of processed events
      histogramEventCounter->Fill(2);

//--- quit event loop if maximal number of events to be processed is reached 
      ++numEvents_processed;
      numEventsWeighted_processed += evtWeight;
      if ( maxEvents > 0 && numEvents_processed >= maxEvents ) maxEvents_processed = true;

//--- check that event has passed triggers
      edm::Handle<pat::TriggerEvent> hltEvent;
      evt.getByLabel(srcTrigger, hltEvent);
  
      bool isTriggered = false;
      for ( vstring::const_iterator hltPathName = hltPaths.begin();
	    hltPathName != hltPaths.end() && !isTriggered; ++hltPathName ) {
	if ( (*hltPathName) == "*" ) { // check for wildcard character "*" that accepts all events
	  isTriggered = true;
	} else {
	  const pat::TriggerPath* hltPath = hltEvent->path(*hltPathName);
	  if ( hltPath && hltPath->wasAccept() ) isTriggered = true;
	}
      }

      if ( !isTriggered ) continue;
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

      unsigned numMuTauPairsABCD = 0;
      for ( PATMuTauPairCollection::const_iterator muTauPair = muTauPairs->begin();
	    muTauPair != muTauPairs->end(); ++muTauPair ) {
	pat::strbitset evtSelFlags;
	if ( selectorABCD->operator()(*muTauPair, evtSelFlags) ) ++numMuTauPairsABCD;
      }
      
      if ( !(numMuTauPairsABCD <= 1) ) continue;
      ++numEvents_passedDiMuTauPairVeto;
      numEventsWeighted_passedDiMuTauPairVeto += evtWeight;

//--- determine number of vertices reconstructed in the event
//   (needed to parametrize dependency of tau id. efficiency on number of pile-up interactions)
      edm::Handle<reco::VertexCollection> vertices;
      evt.getByLabel(srcVertices, vertices);
      size_t numVertices = vertices->size();

//--- iterate over collection of muon + tau-jet pairs:
//    check which region muon + tau-jet pair is selected in,
//    fill histograms for that region
      for ( PATMuTauPairCollection::const_iterator muTauPair = muTauPairs->begin();
	    muTauPair != muTauPairs->end(); ++muTauPair ) {
	for ( std::vector<regionEntryType*>::iterator regionEntry = regionEntries.begin();
	      regionEntry != regionEntries.end(); ++regionEntry ) {
	  (*regionEntry)->analyze(*muTauPair, numVertices, evtWeight);
	}
      }
    }

//--- close input file
    delete inputFile;
  }

//--- scale histograms taken from Monte Carlo simulation
//    according to cross-section times luminosity
  if ( !isData ) {
    double mcScaleFactor = (intLumiData*xSection)/(double)allEvents_DBS;
    std::cout << " intLumiData = " << intLumiData << std::endl;
    std::cout << " xSection = " << xSection << std::endl;
    std::cout << " allEvents_DBS = " << allEvents_DBS << std::endl;
    std::cout << "--> scaling histograms by factor = " << mcScaleFactor
	      << " according to cross-section times luminosity." << std::endl;

//--- apply correction to scale-factor in order to account for events lost, 
//    due to aborted skimming/crab or PAT-tuple production/lxbatch jobs
    double lostStatCorrFactor = 1.;
    if ( histogramEventCounter->GetBinContent(1) > histogramEventCounter->GetBinContent(2) && 
	 histogramEventCounter->GetBinContent(2) > 0.                                      ) {
      lostStatCorrFactor = histogramEventCounter->GetBinContent(1)/histogramEventCounter->GetBinContent(2);
      std::cout << "--> scaling histograms by additional factor = " << lostStatCorrFactor
		<< " to account for events lost," << std::endl; 
      std::cout << "    due to aborted skimming/crab or PAT-tuple production/lxbatch jobs." << std::endl;
    }

    for ( std::vector<regionEntryType*>::iterator regionEntry = regionEntries.begin();
	  regionEntry != regionEntries.end(); ++regionEntry ) {
      (*regionEntry)->histManager_->scaleHistograms(mcScaleFactor*lostStatCorrFactor);
    }
  }

  std::cout << "<FWLiteTauIdEffAnalyzer>:" << std::endl;
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
    std::cout << "  region " << (*regionEntry)->region_ << ":" 
	      << " " << (*regionEntry)->numMuTauPairs_selected_ 
	      << " (weighted = " << (*regionEntry)->numMuTauPairsWeighted_selected_ << ")" << std::endl;
    lastTauIdName = (*regionEntry)->tauIdName_;
  }
  
  if ( isData ) {
    std::cout << " intLumiData (recorded, IsoMu17 prescale corr.) = " << intLumiData << " pb" << std::endl;
    std::cout << " intLumiData_analyzed (recorded) = " << intLumiData_analyzed*1.e-6 << " pb" << std::endl;
  }

  clock.Show("FWLiteTauIdEffAnalyzer");

  return 0;
}
