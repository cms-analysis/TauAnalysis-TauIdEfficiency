
/** \executable FWLiteTauFakeRateAnalyzer
 *
 * Apply event selections for 
 *  o QCD multi-jet
 *  o QCD muon enriched
 *  o W + jets enriched
 *  o Zmumu enriched
 * event samples and fill histograms for jet --> tau fake-rate measurement
 *
 * \author Christian Veelken, UC Davis
 *
 * \version $Revision: 1.2 $
 *
 * $Id: FWLiteTauFakeRateAnalyzer.cc,v 1.2 2011/07/21 16:37:13 veelken Exp $
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
#include "DataFormats/PatCandidates/interface/MET.h"
#include "DataFormats/PatCandidates/interface/TriggerEvent.h"
#include "DataFormats/PatCandidates/interface/TriggerAlgorithm.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/Common/interface/MergeableCounter.h"
#include "DataFormats/Luminosity/interface/LumiSummary.h"
#include "DataFormats/Common/interface/Handle.h"

#include "TauAnalysis/TauIdEfficiency/interface/TauFakeRateEventSelector.h"
#include "TauAnalysis/TauIdEfficiency/interface/TauFakeRateHistManager.h"

#include <TFile.h>
#include <TTree.h>
#include <TSystem.h>
#include <TROOT.h>
#include <TBenchmark.h>

typedef std::vector<std::string> vstring;

struct regionEntryType
{
  regionEntryType(TFileDirectory& dir,
		  const std::string& process, const std::string& region, 
		  const vstring& tauIdDiscriminators, const std::string& tauIdName)
    : process_(process),
      region_(region),
      tauIdDiscriminators_(tauIdDiscriminators),
      tauIdName_(tauIdName),
      selector_(0),
      histograms_(0),
      numTauJetCands_processed_(0),
      numTauJetCandsWeighted_processed_(0.),
      numTauJetCands_selected_(0),
      numTauJetCandsWeighted_selected_(0.)
  {
    //std::cout << "<regionEntryType>:" << std::endl;
    //std::cout << " region = " << region << std::endl;

    edm::ParameterSet cfgSelector;
    cfgSelector.addParameter<vstring>("tauIdDiscriminators", tauIdDiscriminators_);
    cfgSelector.addParameter<std::string>("region", region_);

    selector_ = new TauFakeRateEventSelector(cfgSelector);

    edm::ParameterSet cfgHistManager;
    cfgHistManager.addParameter<std::string>("process", process_);
    cfgHistManager.addParameter<std::string>("region", region_);
    cfgHistManager.addParameter<std::string>("tauIdDiscriminator", tauIdName_);

    histograms_ = new TauFakeRateHistManager(cfgHistManager);
    histograms_->bookHistograms(dir);
  }
  ~regionEntryType()
  {
    delete selector_;

    delete histograms_;
  }
  void analyze(const pat::Tau& tauJetCand, size_t numVertices, double sumEt, double evtWeight)
  {
    ++numTauJetCands_processed_;
    numTauJetCandsWeighted_processed_ += evtWeight;

    pat::strbitset evtSelFlags;
    if ( selector_->operator()(tauJetCand, evtSelFlags) ) {
      histograms_->fillHistograms(tauJetCand, numVertices, sumEt, evtWeight);

      ++numTauJetCands_selected_;
      numTauJetCandsWeighted_selected_ += evtWeight;
    }
  }

  std::string process_;
  std::string region_;
  vstring tauIdDiscriminators_;
  std::string tauIdName_;

  TauFakeRateEventSelector* selector_;

  TauFakeRateHistManager* histograms_;

  double numTauJetCands_processed_;
  double numTauJetCandsWeighted_processed_;
  double numTauJetCands_selected_;
  double numTauJetCandsWeighted_selected_;
};

int main(int argc, char* argv[]) 
{
//--- parse command-line arguments
  if ( argc < 2 ) {
    std::cout << "Usage: " << argv[0] << " [parameters.py]" << std::endl;
    return 0;
  }

  std::cout << "<FWLiteTauFakeRateAnalyzer>:" << std::endl;

//--- load framework libraries
  gSystem->Load("libFWCoreFWLite");
  AutoLibraryLoader::enable();

//--- keep track of time it takes the macro to execute
  TBenchmark clock;
  clock.Start("FWLiteTauFakeRateAnalyzer");

//--- read python configuration parameters
  if ( !edm::readPSetsFrom(argv[1])->existsAs<edm::ParameterSet>("process") ) 
    throw cms::Exception("FWLiteTauFakeRateAnalyzer") 
      << "No ParameterSet 'process' found in configuration file = " << argv[1] << " !!\n";

  edm::ParameterSet cfg = edm::readPSetsFrom(argv[1])->getParameter<edm::ParameterSet>("process");

  edm::ParameterSet cfgTauFakeRateAnalyzer = cfg.getParameter<edm::ParameterSet>("tauFakeRateAnalyzer");

  edm::InputTag srcTauJetCandidates = cfgTauFakeRateAnalyzer.getParameter<edm::InputTag>("srcTauJetCandidates");
  edm::InputTag srcVertices = cfgTauFakeRateAnalyzer.getParameter<edm::InputTag>("srcVertices");
  edm::InputTag srcMET = cfgTauFakeRateAnalyzer.getParameter<edm::InputTag>("srcMET");
  typedef std::vector<edm::InputTag> vInputTag;
  vInputTag srcWeights = cfgTauFakeRateAnalyzer.getParameter<vInputTag>("weights");
  edm::InputTag srcEventCounter = cfgTauFakeRateAnalyzer.getParameter<edm::InputTag>("srcEventCounter");

  fwlite::InputSource inputFiles(cfg); 
  int maxEvents = inputFiles.maxEvents();

  fwlite::OutputFiles outputFile(cfg);
  fwlite::TFileService fs = fwlite::TFileService(outputFile.file().data());

//--- initialize selections and histograms
//    for P(assed)/F(ailed) and A(ll) regions
  std::vector<regionEntryType*> regionEntries;

  std::string process = cfgTauFakeRateAnalyzer.getParameter<std::string>("process");
  std::cout << " process = " << process << std::endl;
  std::string processType = cfgTauFakeRateAnalyzer.getParameter<std::string>("type");
  std::cout << " type = " << processType << std::endl;
  bool isData = (processType == "Data");
  std::string evtSel = cfgTauFakeRateAnalyzer.getParameter<std::string>("evtSel");
  TFileDirectory dir = fs.mkdir(evtSel);
  vstring regions = cfgTauFakeRateAnalyzer.getParameter<vstring>("regions");
  typedef std::vector<edm::ParameterSet> vParameterSet;
  vParameterSet cfgTauIdDiscriminators = cfgTauFakeRateAnalyzer.getParameter<vParameterSet>("tauIds");
  for ( vParameterSet::const_iterator cfgTauIdDiscriminator = cfgTauIdDiscriminators.begin();
	cfgTauIdDiscriminator != cfgTauIdDiscriminators.end(); ++cfgTauIdDiscriminator ) {
    for ( vstring::const_iterator region = regions.begin();
	  region != regions.end(); ++region ) {
      vstring tauIdDiscriminators = cfgTauIdDiscriminator->getParameter<vstring>("discriminators");
      std::string tauIdName = cfgTauIdDiscriminator->getParameter<std::string>("name");
      regionEntryType* regionEntry = new regionEntryType(dir, process, *region, tauIdDiscriminators, tauIdName);
      regionEntries.push_back(regionEntry);
    }
  }

//--- book "dummy" histogram counting number of processed events
  TH1* histogramEventCounter = fs.make<TH1F>("numEventsProcessed", "Number of processed Events", 3, -0.5, +2.5);
  histogramEventCounter->GetXaxis()->SetBinLabel(1, "all Events (DBS)");      // CV: bin numbers start at 1 (not 0) !!
  histogramEventCounter->GetXaxis()->SetBinLabel(2, "processed by Skimming");
  histogramEventCounter->GetXaxis()->SetBinLabel(3, "analyzed in PAT-tuple");
  
  int allEvents_DBS = cfgTauFakeRateAnalyzer.getParameter<int>("allEvents_DBS");
  if ( allEvents_DBS > 0 ) {
    histogramEventCounter->SetBinContent(1, cfgTauFakeRateAnalyzer.getParameter<int>("allEvents_DBS"));
  } else {
    histogramEventCounter->SetBinContent(1, -1.);
  }
  
  double xSection = cfgTauFakeRateAnalyzer.getParameter<double>("xSection");
  double intLumiData = cfgTauFakeRateAnalyzer.getParameter<double>("intLumiData");

  int    numEvents_processed         = 0; 
  double numEventsWeighted_processed = 0.;

  edm::RunNumber_t lastLumiBlock_run = -1;
  edm::LuminosityBlockNumber_t lastLumiBlock_ls = -1;

  double intLumiData_analyzed = 0.;
  edm::InputTag srcLumiProducer = cfgTauFakeRateAnalyzer.getParameter<edm::InputTag>("srcLumiProducer");

  bool maxEvents_processed = false;
  for ( vstring::const_iterator inputFileName = inputFiles.files().begin();
	inputFileName != inputFiles.files().end() && !maxEvents_processed; ++inputFileName ) {

//--- open input file
    TFile* inputFile = TFile::Open(inputFileName->data());
    if ( !inputFile ) 
      throw cms::Exception("FWLiteTauFakeRateAnalyzer") 
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

//--- determine number of vertices reconstructed in the event
//   (needed to parametrize dependency of jet --> tau fake-rate on number of pile-up interactions)
      edm::Handle<reco::VertexCollection> vertices;
      evt.getByLabel(srcVertices, vertices);
      size_t numVertices = vertices->size();

//--- determine sumEt of all particles in the event
//   (needed to parametrize dependency of jet --> tau fake-rate on level of hadronic activity)
      edm::Handle<pat::METCollection> METs;
      evt.getByLabel(srcMET, METs);
      if ( !(METs->size() == 1) )
	throw cms::Exception("FWLiteTauFakeRateAnalyzer") 
	  << "Failed to find unique MET object in the event !!\n";
      double sumEt = METs->begin()->sumEt();

//--- iterate over collection of tau-jet candidates:
//    check if tau-jet candidate passed/fails tau id. criteria,
//    fill corresponding histograms
      edm::Handle<pat::TauCollection> tauJetCandidates;
      evt.getByLabel(srcTauJetCandidates, tauJetCandidates);
      for ( pat::TauCollection::const_iterator tauJetCand = tauJetCandidates->begin();
	    tauJetCand != tauJetCandidates->end(); ++tauJetCand ) {
	for ( std::vector<regionEntryType*>::iterator regionEntry = regionEntries.begin();
	      regionEntry != regionEntries.end(); ++regionEntry ) {
	  (*regionEntry)->analyze(*tauJetCand, numVertices, sumEt, evtWeight);
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
      (*regionEntry)->histograms_->scaleHistograms(mcScaleFactor*lostStatCorrFactor);
    }
  }

  std::cout << "<FWLiteTauFakeRateAnalyzer>:" << std::endl;
  std::cout << " numEvents_processed: " << numEvents_processed 
	    << " (weighted = " << numEventsWeighted_processed << ")" << std::endl;
  std::string lastTauIdName = "";
  for ( std::vector<regionEntryType*>::iterator regionEntry = regionEntries.begin();
	regionEntry != regionEntries.end(); ++regionEntry ) {
    if ( (*regionEntry)->tauIdName_ != lastTauIdName ) 
      std::cout << " numTauJetCands_processed/numTauJetCands_selected," 
		<< " " << (*regionEntry)->tauIdName_ << std::endl;
    std::cout << "  region " << (*regionEntry)->region_ << ":" 
	      << " " << (*regionEntry)->numTauJetCands_processed_ 
	      << "/" << (*regionEntry)->numTauJetCands_selected_ 
	      << " (weighted = " << (*regionEntry)->numTauJetCandsWeighted_processed_ 
	      << "/" << (*regionEntry)->numTauJetCandsWeighted_selected_ << ")" << std::endl;
    lastTauIdName = (*regionEntry)->tauIdName_;
  }
  
  if ( isData ) {
    std::cout << " intLumiData (recorded, Trigger prescale corr.) = " << intLumiData << " pb" << std::endl;
    // CV: luminosity is recorded in some 'weird' units,
    //     needs to be multiplied by factor 0.10 in order to be in units of pb^-1
    std::cout << " intLumiData_analyzed (recorded) = " << intLumiData_analyzed*1.e-6*0.10 << " pb" << std::endl;
  }

  clock.Show("FWLiteTauFakeRateAnalyzer");

  return 0;
}
