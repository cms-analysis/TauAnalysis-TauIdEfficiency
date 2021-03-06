
/** \executable fitTauIdEff
 *
 * Perform simultaneous fit of Ztautau signal plus Zmumu, W + jets, QCD and TTbar background yields
 * in regions A, B, C1f, C1p, C2 and D, in order to determine tau identification efficiency
 *
 * \author Christian Veelken, UC Davis
 *
 * \version $Revision: 1.37 $
 *
 * $Id: fitTauIdEff.cc,v 1.37 2012/07/29 15:50:37 veelken Exp $
 *
 */

#include "FWCore/FWLite/interface/AutoLibraryLoader.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/PythonParameterSet/interface/MakeParameterSets.h"

#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "PhysicsTools/FWLite/interface/TFileService.h"
#include "DataFormats/FWLite/interface/InputSource.h"
#include "DataFormats/FWLite/interface/OutputFiles.h"

#include "HiggsAnalysis/CombinedLimit/interface/VerticalInterpPdf.h"

#include "TauAnalysis/TauIdEfficiency/bin/tauIdEffAuxFunctions.h"
#include "TauAnalysis/CandidateTools/interface/generalAuxFunctions.h"
#include "TauAnalysis/FittingTools/interface/templateFitAuxFunctions.h"

#include "RooAddPdf.h"
#include "RooCategory.h"
#include "RooCmdArg.h"
#include "RooConstVar.h"
#include "RooDataHist.h"
#include "RooFit.h"
#include "RooFormulaVar.h"
#include "RooGaussian.h"
#include "RooHistPdf.h"
#include "RooIntegralMorph.h"
#include "RooProduct.h"
#include "RooRealVar.h"
#include "RooSimultaneous.h"
#include "RooAddition.h"
#include "RooMinuit.h"
#include "RooFitResult.h"

#include <TROOT.h>
#include <TSystem.h>
#include <TFile.h>
#include <TString.h>
#include <TCanvas.h>
#include <TH1.h>
#include <THStack.h>
#include <TLegend.h>
#include <TObjArray.h>
#include <TBenchmark.h>
#include <TMatrixD.h>

#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

enum { kNoTemplateMorphing, kHorizontalTemplateMorphing, kVerticalTemplateMorphing };

//
//-------------------------------------------------------------------------------
//

struct fitVariableType
{
  fitVariableType()
    : xAxis_(0)
  {}
  fitVariableType(const std::string& name)
    : name_(name)
  {}
  fitVariableType(RooRealVar* xAxis)
    : name_(xAxis->GetName()),
      xAxis_(xAxis)
  {}
  std::string name_;
  RooRealVar* xAxis_;
};

struct fitParameterType
{
  fitParameterType()
    : fittedValue_(0)
  {}
  fitParameterType(RooRealVar* fittedValue, double expectedValue)
    : name_(fittedValue->GetName()),
      fittedValue_(fittedValue),
      expectedValue_(expectedValue)
  {
    // CV: expected values close to either zero or one indicate low event statistics in template histogram
    //    (potentially even empty template histogram)
    //    --> declare fit parameter to be constant, so that it does not 'confuse' RooFit
    const double pMin = 1.e-2;
    if ( expectedValue_ < pMin || expectedValue_ > (1. - pMin) ) fittedValue_->setConstant(true);
  }
  std::string name_;
  RooRealVar* fittedValue_;
  double expectedValue_;
};

struct alphaParameterType // nuissance parameter for template morphing
{
  alphaParameterType()
    : alpha_(0)
  {}
  alphaParameterType(RooRealVar* alpha, bool isBiDirectional)
    : name_(alpha->GetName()),
      alpha_(alpha),
      isBiDirectional_(isBiDirectional)
  {}
  std::string name_;
  RooRealVar* alpha_;
  bool isBiDirectional_; // set to true/false in case of vertical/horizontal morphing
};

//
//-------------------------------------------------------------------------------
//

void addToFormula(std::string& formula, const std::string& expression, TObjArray& arguments, RooAbsReal* p)
{
//-------------------------------------------------------------------------------
// Build formula and argument list for RooFormulaVar
//
// NOTE: auxiliary function for makeRooFormulaVar
//
//-------------------------------------------------------------------------------

  if ( expression != "" ) {
    if ( formula != "" ) formula.append("*");

    if      ( expression == "regular"  ) formula.append(p->GetName());
    else if ( expression == "inverted" ) formula.append("(1 - ").append(p->GetName()).append(")");
    else assert(0);
    
    arguments.Add(p);
  }
}

RooFormulaVar* makeRooFormulaVar_6regions(const std::string& process, 
					  const std::string& region, const std::string& region_passed, const std::string& region_failed, 
					  RooAbsReal* norm, RooAbsReal* fittedFraction, unsigned numCategories,
					  std::map<std::string, fitParameterType>& fitParameters)
{
//-------------------------------------------------------------------------------
// Make RooRealVar object representing normalization for MC process 
// passed as function argument in a given region.
//
// The regions are defined as follows:
//
//   /---------------\ /---------------\
//   |               | |               |
//   |               | |               |
//   |       A       | |       B       | 0.1 * muonPt < muonIso < 0.5 * muonPt
//   |               | |               |
//   |               | |               |
//   \---------------/ \---------------/
//
//   /---------------\ /---------------\
//   |               | |               |
//   |               | |               |
//   |       C       | |       D       |                muonIso < 0.1 * muonPt
//   |               | |               |
//   |               | |               |
//   \---------------/ \---------------/
//       
//           OS                SS
//
// C1p : Mt < 40 GeV && (Pzeta - 1.5 PzetaVis) > -20 GeV && tau id. passed
// C1f : Mt < 40 GeV && (Pzeta - 1.5 PzetaVis) > -20 GeV && tau id. failed
// C2p : Mt > 40 GeV || (Pzeta - 1.5 PzetaVis) < -20 GeV && tau id. passed
// C2f : Mt > 40 GeV || (Pzeta - 1.5 PzetaVis) < -20 GeV && tau id. failed
//
//-------------------------------------------------------------------------------

  //std::cout << "<makeRooFormulaVar_6regions>:" << std::endl;
  //std::cout << " building RooFormulaVar expression for process = " << process << ", region = " << region << "." << std::endl;

  assert(numCategories == 2 || numCategories == 4);

  RooFormulaVar* retVal = 0;

  std::string exprDiTauCharge = "";
  std::string exprDiTauKine   = "";
  std::string exprMuonIso     = "";
  std::string exprTauId       = "";

  if        ( region.find("A") != std::string::npos ) {
    exprDiTauCharge = "regular";
    exprMuonIso     = "inverted";
  } else if ( region.find("B") != std::string::npos ) {
    exprDiTauCharge = "inverted";
    exprMuonIso     = "inverted";
  } else if ( region.find("C") != std::string::npos ) {
    exprDiTauCharge = "regular";
    exprMuonIso     = "regular"; 
  } else if ( region.find("D") != std::string::npos ) {
    exprDiTauCharge = "inverted";
    exprMuonIso     = "regular";
  } else {
    throw cms::Exception("makeRooFormulaVar_6regions") 
      << "Region = " << region << " not defined !!\n";
  }
  
  if      ( region.find("1") != std::string::npos ) exprDiTauKine = "regular";
  else if ( region.find("2") != std::string::npos ) exprDiTauKine = "inverted";
  
  if      ( region.find("p") != std::string::npos ) exprTauId     = "regular";
  else if ( region.find("f") != std::string::npos ) exprTauId     = "inverted";

  // CV: fitted yields for all processes come-out factor N too high in closure test
  //    --> compensate by multiplying fittedFraction by fudge-factor N,
  //        N = number of categories used in simultaneous fit
  std::string nameFittedFraction_scaled = std::string(norm->GetName()).append("_").append(region).append("_fittedFraction_scaled");
  RooFormulaVar* fittedFraction_scaled = 
    new RooFormulaVar(nameFittedFraction_scaled.data(), 
		      nameFittedFraction_scaled.data(), Form("%u*@0", numCategories), RooArgList(*fittedFraction));

  std::string formula = "";
  TObjArray arguments; 
  addToFormula(formula, exprDiTauCharge, arguments, fitParameters["pDiTauCharge_OS_SS"].fittedValue_);
  addToFormula(formula, exprDiTauKine,   arguments, fitParameters["pDiTauKine_Sig_Bgr"].fittedValue_);
  addToFormula(formula, exprMuonIso,     arguments, fitParameters["pMuonIso_tight_loose"].fittedValue_);
  addToFormula(formula, exprTauId,       arguments, fitParameters["pTauId_passed_failed"].fittedValue_);
  addToFormula(formula, "regular",       arguments, norm);
  addToFormula(formula, "regular",       arguments, fittedFraction_scaled);

  std::string name = std::string("norm").append(process).append("_").append(region);
  retVal = new RooFormulaVar(name.data(), name.data(), formula.data(), RooArgSet(arguments));

  return retVal;
}

RooFormulaVar* makeRooFormulaVar_2regions(const std::string& process, 
					  const std::string& region, const std::string& region_passed, const std::string& region_failed, 
					  RooAbsReal* norm, RooAbsReal* fittedFraction, unsigned numCategories,
					  std::map<std::string, fitParameterType>& fitParameters)
{
  std::cout << "<makeRooFormulaVar_2regions>:" << std::endl;
  std::cout << " building RooFormulaVar expression for process = " << process << ", region = " << region << "." << std::endl;

  assert(numCategories == 2);

  RooFormulaVar* retVal = 0;

  std::string exprTauId = "";
  if      ( region == region_passed ) exprTauId = "regular";
  else if ( region == region_failed ) exprTauId = "inverted";
  else throw cms::Exception("makeRooFormulaVar_2regions") 
    << "Region = " << region << " matches neither 'passed' nor 'failed' region !!\n";
  
  // CV: fitted yields for all processes come-out factor 2 too high in closure test
  //    --> compensate by multiplying fittedFraction by fudge-factor 2
  std::string nameFittedFraction_scaled = std::string(norm->GetName()).append("_").append(region).append("_fittedFraction_scaled");
  RooFormulaVar* fittedFraction_scaled = 
    new RooFormulaVar(nameFittedFraction_scaled.data(), 
		      nameFittedFraction_scaled.data(), Form("%u*@0", numCategories), RooArgList(*fittedFraction));

  std::string formula = "";
  TObjArray arguments; 
  addToFormula(formula, exprTauId, arguments, fitParameters["pTauId_passed_failed"].fittedValue_);
  addToFormula(formula, "regular", arguments, norm);
  addToFormula(formula, "regular", arguments, fittedFraction_scaled);

  std::cout << " formula = " << formula << std::endl;
  std::cout << " arguments:" << std::endl;
  for ( int i = 0; i < arguments.GetEntries(); ++i ) {
    std::cout << "  " << dynamic_cast<TNamed*>(arguments.At(i))->GetName() << ":" 
	      << " value = " << dynamic_cast<RooAbsReal*>(arguments.At(i))->getVal() << std::endl;
  }

  std::string name = std::string("norm").append(process).append("_").append(region);
  retVal = new RooFormulaVar(name.data(), name.data(), formula.data(), RooArgSet(arguments));

  return retVal;
}

//
//-------------------------------------------------------------------------------
//

RooHistPdf* makeRooHistPdf(TH1* templateHistogram, RooAbsReal* fitVariable)
{
  std::string templateHistogramName_normalized = std::string(templateHistogram->GetName()).append("_normalized");
  TH1* templateHistogram_normalized = (TH1*)templateHistogram->Clone(templateHistogramName_normalized.data());

  std::string templateDataHistName = std::string(templateHistogram->GetName()).append("_dataHist");
  RooDataHist* templateDataHist = 
    new RooDataHist(templateDataHistName.data(), 
		    templateDataHistName.data(), *fitVariable, templateHistogram_normalized);
  std::string templatePdfName = std::string(templateHistogram->GetName()).append("_histPdf");
  RooHistPdf* templatePdf = 
    new RooHistPdf(templatePdfName.data(), 
		   templatePdfName.data(), *fitVariable, *templateDataHist);
  return templatePdf;
}

RooAbsPdf* makePdfHorizontalMorphing(histogramMap3& histograms,
				     const std::string& region, fitVariableType& fitVariable, const vstring& sysUncertainties,
				     std::map<std::string, alphaParameterType>& alphaParameters)
{
  if ( sysUncertainties.size() != 1 )
    throw cms::Exception("makePdfHorizontalMorphing") 
      << "Number of sysUncertainties = " << sysUncertainties.size() << " not supported yet !!\n";

  const std::string& sysUncertainty = sysUncertainties.front();

  std::string templateHistName = histograms[region][fitVariable.name_][key_central_value]->GetName();

  TH1* templateHistUp = histograms[region][fitVariable.name_][std::string(sysUncertainty).append("Up")];
  RooHistPdf* pdfUp = makeRooHistPdf(templateHistUp, fitVariable.xAxis_);

  TH1* templateHistDown = histograms[region][fitVariable.name_][std::string(sysUncertainty).append("Down")];
  RooHistPdf* pdfDown = makeRooHistPdf(templateHistDown, fitVariable.xAxis_);
  
  RooRealVar* alpha = alphaParameters[sysUncertainty].alpha_;

  std::string pdfName = std::string(templateHistName).append("_pdf");
  RooIntegralMorph* pdf = new RooIntegralMorph(pdfName.data(), pdfName.data(), *pdfUp, *pdfDown, *fitVariable.xAxis_, *alpha);
  pdf->setCacheAlpha(true);
  return pdf;
}

RooAbsPdf* makePdfVerticalMorphing(histogramMap3& histograms,
				   const std::string& region, fitVariableType& fitVariable, const vstring& sysUncertainties,
				   std::map<std::string, alphaParameterType>& alphaParameters)
{
  // NOTE: VerticalInterpPdf requires "up"/"down" systematic shifts to be passed as pdfs
  //       The pdfs are assumed to be ordered in the following way, together with the  morphing parameters:
  //      o pdfs   = { histogram_central_value, histogram_sys1Up, histogram_sys1Down, histogram_sys2Up, histogram_sys2Down,... }
  //      o alphas = { alpha_sys1, alpha_sys2 }
  //
  //     --> number of pdfs = 2*(numbers of alphas) + 1)
  //

  //std::cout << "<makePdfVerticalMorphing>:" << std::endl;

  TObjArray pdfs;
  TObjArray alphas;

  std::string templateHistName = histograms[region][fitVariable.name_][key_central_value]->GetName();

  TH1* templateHist_central_value = histograms[region][fitVariable.name_][key_central_value];
  RooHistPdf* pdf_central_value = makeRooHistPdf(templateHist_central_value, fitVariable.xAxis_);
  pdfs.Add(pdf_central_value);

  for ( vstring::const_iterator sysUncertainty = sysUncertainties.begin();
	sysUncertainty != sysUncertainties.end(); ++sysUncertainty ) {
    TH1* templateHist_up = histograms[region][fitVariable.name_][std::string(*sysUncertainty).append("Up")];
    RooHistPdf* pdf_up = makeRooHistPdf(templateHist_up, fitVariable.xAxis_);
    pdfs.Add(pdf_up);

    TH1* templateHist_down = histograms[region][fitVariable.name_][std::string(*sysUncertainty).append("Down")];
    RooHistPdf* pdf_down = makeRooHistPdf(templateHist_down, fitVariable.xAxis_);
    pdfs.Add(pdf_down);

    RooRealVar* alpha = alphaParameters[*sysUncertainty].alpha_;
    alphas.Add(alpha);
  }

  if ( pdfs.GetEntries() != (2*alphas.GetEntries() + 1) ) 
    throw cms::Exception("makePdfVerticalMorphing") 
      << "Number of 'pdf' = " << pdfs.GetEntries() << " and 'alpha' parameter = " << alphas.GetEntries()
      << " arguments not compatible !!\n";

  std::string pdfName = std::string(templateHistName).append("_pdf");
  VerticalInterpPdf* pdf = new VerticalInterpPdf(pdfName.data(), pdfName.data(), RooArgList(pdfs), RooArgList(alphas));
  return pdf;
}

RooGaussian* makeFitConstraint(RooAbsReal* p, double value, double error)
{
  std::string pValueName = std::string(p->GetName()).append("_constValue");
  RooConstVar* pValue = new RooConstVar(pValueName.data(), pValueName.data(), value);
  std::string pErrorName = std::string(p->GetName()).append("_constError");
  RooConstVar* pError = new RooConstVar(pErrorName.data(), pErrorName.data(), error);

  std::string constraintName = std::string(p->GetName()).append("_constraint");
  RooGaussian* constraint = new RooGaussian(constraintName.data(), constraintName.data(), *p, *pValue, *pError);
  return constraint;
}

//
//-------------------------------------------------------------------------------
//

struct processEntryType
{
  processEntryType(const std::string& name, 
		   histogramMap3& histograms, 
		   std::map<std::string, fitVariableType>& fitVariables, 
		   const vstring& regionsToFit, const std::string& region_passed, const std::string& region_failed,
		   const vstring& sysUncertainties, int templateMorphingMode, 
		   const std::string& legendEntry, int fillColor)
    : name_(name),
      histograms_(histograms),
      fitVariables_(fitVariables),
      regionsToFit_(regionsToFit),
      region_passed_(region_passed),
      region_failed_(region_failed),
      sysUncertainties_(sysUncertainties),
      templateMorphingMode_(templateMorphingMode), 
      legendEntry_(legendEntry),
      fillColor_(fillColor)
  {
//--- initialize fit variable in case it does not yet exist
//
//    NOTE: fit variables need to be shared among all processes and data
//
    for ( std::map<std::string, fitVariableType>::iterator fitVariable = fitVariables_.begin();
	  fitVariable != fitVariables_.end(); ++fitVariable ) {
      const std::string& region = fitVariable->first;
      const std::string& fitVariableName = fitVariable->second.name_;

      TAxis* xAxis = histograms_[region][fitVariableName][key_central_value]->GetXaxis();
      double fitMin = xAxis->GetXmin();
      double fitMax = xAxis->GetXmax();

      if ( gFitVariableMap.find(fitVariableName) != gFitVariableMap.end() ) {
	RooRealVar* fitVariable_xAxis = gFitVariableMap[fitVariableName];
	if ( !(fitVariable_xAxis->getMin() == fitMin &&
	       fitVariable_xAxis->getMax() == fitMax) )
	  throw cms::Exception("processEntryType") 
	    << "Incompatible binning for regions '" << region << "' and '" << gFitVariableRefRegion[fitVariableName] << "' !!\n";
	fitVariable->second.xAxis_ = fitVariable_xAxis;
      } else { 
	RooRealVar* fitVariable_xAxis = new RooRealVar(fitVariableName.data(), fitVariableName.data(), fitMin, fitMax);
	fitVariable_xAxis->setBins(100, "cache");
	fitVariable->second.xAxis_ = fitVariable_xAxis;
	gFitVariableMap[fitVariableName] = fitVariable_xAxis;
	gFitVariableRefRegion[fitVariableName] = region;
      }
    }

//--- check that fit variable definition for 'passed' and 'failed' regions is identical
    RooRealVar* fitVariable_passed = fitVariables_[region_passed].xAxis_;
    RooRealVar* fitVariable_failed = fitVariables_[region_failed].xAxis_;
    if ( !(fitVariable_passed->getMin() == fitVariable_failed->getMin() &&
	   fitVariable_passed->getMax() == fitVariable_failed->getMax()) ) {
      throw cms::Exception("processEntryType") 
	<< "Incompatible binning for 'passed' and 'failed' regions !!\n";
    }
    
    vstring sysUncertainties_expanded;
    sysUncertainties_expanded.push_back(key_central_value);
    for ( vstring::const_iterator sysUncertainty = sysUncertainties_.begin();
	  sysUncertainty != sysUncertainties_.end(); ++sysUncertainty ) {
      if ( (*sysUncertainty) != key_central_value ) {
	sysUncertainties_expanded.push_back(std::string(*sysUncertainty).append("Up"));
	sysUncertainties_expanded.push_back(std::string(*sysUncertainty).append("Down"));
      }
    }
    
    for ( vstring::const_iterator sysUncertainty = sysUncertainties_expanded.begin();
	  sysUncertainty != sysUncertainties_expanded.end(); ++sysUncertainty ) {
      double numEventsABCD = 0.;
      for ( histogramMap3::const_iterator region = histograms_.begin();
	    region != histograms_.end(); ++region ) {
	if ( histograms[region->first]["EventCounter"].find(*sysUncertainty) != histograms[region->first]["EventCounter"].end() ) {
	  double numEvents_region = getIntegral(histograms[region->first]["EventCounter"][*sysUncertainty]);
	  numEvents_[region->first][*sysUncertainty] = numEvents_region;
	  std::cout << "numEvents[" << region->first << "][" << (*sysUncertainty) << "] = " << numEvents_[region->first][*sysUncertainty] << std::endl;
	  if ( contains_string(regionsToFit_, region->first) ) numEventsABCD += numEvents_region;
	} else {
	  edm::LogWarning ("processEntryType") 
	    << "Failed to compute numEvents for process = " << name_ << "," 
	    << " region = " << region->first << ", central_or_shift = " << (*sysUncertainty) << " !!";
	}
      }
      numEvents_["ABCD"][*sysUncertainty] = numEventsABCD;
      std::cout << "numEvents[ABCD][" << (*sysUncertainty) << "] = " << numEvents_["ABCD"][*sysUncertainty] << std::endl;
    }
    
    norm_.name_ = std::string(name).append("_norm");
    norm_.expectedValue_ = numEvents_["ABCD"][key_central_value];
    norm_.fittedValue_ = new RooRealVar(norm_.name_.data(), norm_.name_.data(), norm_.expectedValue_, 0., 1.e+6);
    
    for ( vstring::const_iterator sysUncertainty = sysUncertainties_.begin();
	  sysUncertainty != sysUncertainties_.end(); ++sysUncertainty ) {
      std::string alphaName = std::string(name).append("_alpha_").append(*sysUncertainty);
      if ( templateMorphingMode == kHorizontalTemplateMorphing ) {
	RooRealVar* alpha = new RooRealVar(alphaName.data(), alphaName.data(), 0.5, 0., 1.);
	alphaParameters_[*sysUncertainty] = alphaParameterType(alpha, false);
      } else if ( templateMorphingMode == kVerticalTemplateMorphing ) {
	RooRealVar* alpha = new RooRealVar(alphaName.data(), alphaName.data(), 0., -1., +1.);
	alphaParameters_[*sysUncertainty] = alphaParameterType(alpha, true);
      }
    }
    
    for ( histogramMap3::const_iterator region = histograms_.begin();
	  region != histograms_.end(); ++region ) {
      for ( histogramMap2::const_iterator observable = region->second.begin();
	    observable != region->second.end(); ++observable ) {
	TObjArray fittedFraction_parameters;
	for ( histogramMap1::const_iterator sysUncertainty = observable->second.begin();
	      sysUncertainty != observable->second.end(); ++sysUncertainty ) {
	  double fittedFraction_value = 1.;
	  if ( numEvents_[region->first].find(sysUncertainty->first) != numEvents_[region->first].end() &&
	       histograms_[region->first][observable->first].find(sysUncertainty->first) != histograms_[region->first][observable->first].end() ) {
	    TH1* histogram = histograms_[region->first][observable->first][sysUncertainty->first];
	    assert(histogram);
	    double numEvents_region = numEvents_[region->first][sysUncertainty->first];
	    double integral = getIntegral(histograms_[region->first][observable->first][sysUncertainty->first], false, false);
	    if ( numEvents_region > 0. ) fittedFraction_value = integral/numEvents_region;
	  } else {
	    edm::LogWarning ("processEntryType") 
	      << "Failed to compute fittedFraction for process = " << name_ << "," 
	      << " region = " << region->first << ", central_or_shift = " << sysUncertainty->first << " !!";
	  }
	  std::string nameFittedFraction = Form("%s_%s_%s_%s_fittedFraction", name.data(), region->first.data(), observable->first.data(), sysUncertainty->first.data());
	  RooConstVar* fittedFraction = 
  	    new RooConstVar(nameFittedFraction.data(), 
			    nameFittedFraction.data(), fittedFraction_value);
	  fittedFractionParameters_[region->first][observable->first][sysUncertainty->first] = fittedFraction;
	  std::cout << "fittedFractionParameters_[" << region->first << "][" << observable->first << "][" << sysUncertainty->first << "] = " << fittedFraction 
		    << " (name = " << fittedFraction->GetName() << ", value = " << fittedFraction->getVal() << ")" << std::endl;
	  fittedFraction_parameters.Add(fittedFraction);
	}	
	std::string nameFittedFraction_central_value = fittedFractionParameters_[region->first][observable->first][key_central_value]->GetName();
	std::string fittedFraction_formula = nameFittedFraction_central_value;
	for ( vstring::const_iterator sysUncertainty = sysUncertainties_.begin();
	      sysUncertainty != sysUncertainties_.end(); ++sysUncertainty ) {
	  RooAbsReal* alpha = alphaParameters_[*sysUncertainty].alpha_;
	  if ( alpha ) {
	    fittedFraction_parameters.Add(alpha);
	    RooAbsReal* fittedFraction_up = fittedFractionParameters_[region->first][observable->first][std::string(*sysUncertainty).append("Up")];
	    if ( fittedFraction_up ) {
    	      fittedFraction_formula.append(
	        Form(" + TMath::Max(0., %s)*(%s - %s)", alpha->GetName(), fittedFraction_up->GetName(), nameFittedFraction_central_value.data()));
	      fittedFraction_parameters.Add(fittedFraction_up);
	    }
	    RooAbsReal* fittedFraction_down = fittedFractionParameters_[region->first][observable->first][std::string(*sysUncertainty).append("Down")];
	    if ( fittedFraction_down ) {
  	      fittedFraction_formula.append(
                Form(" + TMath::Min(0., %s)*(%s - %s)", alpha->GetName(), nameFittedFraction_central_value.data(), fittedFraction_down->GetName()));
	      fittedFraction_parameters.Add(fittedFraction_down);
	    }  
	  }
	}
	std::string nameFittedFraction = Form("%s_%s_%s_fittedFraction", name.data(), region->first.data(), observable->first.data());
	RooFormulaVar* fittedFraction = 
  	    new RooFormulaVar(nameFittedFraction.data(), 
			      nameFittedFraction.data(), fittedFraction_formula.data(), RooArgList(fittedFraction_parameters));
	fittedFractions_[region->first][observable->first] = fittedFraction;
      }
    } 

    RooFormulaVar* (*makeRooFormulaVar)(const std::string&, 
					const std::string&, const std::string&, const std::string&, 
					RooAbsReal*, RooAbsReal*, unsigned, 
					std::map<std::string, fitParameterType>&) = 0;

    if ( regionsToFit.size() == 6 &&
	 contains_string(regionsToFit, "A")   &&
	 contains_string(regionsToFit, "B")   &&	 
	 contains_string(regionsToFit, region_passed) &&
	 contains_string(regionsToFit, region_failed) &&
	 contains_string(regionsToFit, "C2")  &&
	 contains_string(regionsToFit, "D")   ) {
      std::string nameDiTauCharge_OS_SS = std::string(name).append("_").append("pDiTauCharge_OS_SS");
      double pDiTauCharge_OS_SS_value = 
	(numEvents_["A"][key_central_value] + numEvents_["C"][key_central_value])/numEvents_["ABCD"][key_central_value];
      RooRealVar* pDiTauCharge_OS_SS = 
	new RooRealVar(nameDiTauCharge_OS_SS.data(), 
		       nameDiTauCharge_OS_SS.data(), pDiTauCharge_OS_SS_value, 0., 1.);
      fitParameters_["pDiTauCharge_OS_SS"] = fitParameterType(pDiTauCharge_OS_SS, pDiTauCharge_OS_SS_value);

      std::string nameMuonIso_tight_loose = std::string(name).append("_").append("pMuonIso_tight_loose");
      double pMuonIso_tight_loose_value = 
	(numEvents_["C"][key_central_value] + numEvents_["D"][key_central_value])/numEvents_["ABCD"][key_central_value];
      RooRealVar* pMuonIso_tight_loose =
	new RooRealVar(nameMuonIso_tight_loose.data(), 
		       nameMuonIso_tight_loose.data(), pMuonIso_tight_loose_value, 0., 1.);
      fitParameters_["pMuonIso_tight_loose"] = fitParameterType(pMuonIso_tight_loose, pMuonIso_tight_loose_value);
      
      std::string nameDiTauKine_Sig_Bgr = std::string(name).append("_").append("pDiTauKine_Sig_Bgr");
      double pDiTauKine_Sig_Bgr_value = 
	numEvents_["C1"][key_central_value]/numEvents_["C"][key_central_value];
      RooRealVar* pDiTauKine_Sig_Bgr =
	new RooRealVar(nameDiTauKine_Sig_Bgr.data(), 
		       nameDiTauKine_Sig_Bgr.data(), pDiTauKine_Sig_Bgr_value, 0., 1.);
      fitParameters_["pDiTauKine_Sig_Bgr"] = fitParameterType(pDiTauKine_Sig_Bgr, pDiTauKine_Sig_Bgr_value);
      
      std::string nameTauId_passed_failed = std::string(name).append("_").append("pTauId_passed_failed");
      double pTauId_passed_failed_value = 
	numEvents_[region_passed][key_central_value]/(numEvents_[region_passed][key_central_value] + numEvents_[region_failed][key_central_value]);
      RooRealVar* pTauId_passed_failed =
	new RooRealVar(nameTauId_passed_failed.data(),
		       nameTauId_passed_failed.data(), pTauId_passed_failed_value, 0., 1.);
      fitParameters_["pTauId_passed_failed"] = fitParameterType(pTauId_passed_failed, pTauId_passed_failed_value);

      makeRooFormulaVar = &makeRooFormulaVar_6regions;
    } else if ( regionsToFit.size() == 2 &&
		contains_string(regionsToFit, region_passed) &&
		contains_string(regionsToFit, region_failed) ) {
      std::string nameTauId_passed_failed = std::string(name).append("_").append("pTauId_passed_failed");
      double pTauId_passed_failed_value = 
	numEvents_[region_passed][key_central_value]/(numEvents_[region_passed][key_central_value] + numEvents_[region_failed][key_central_value]);
      RooRealVar* pTauId_passed_failed =
	new RooRealVar(nameTauId_passed_failed.data(),
		       nameTauId_passed_failed.data(), pTauId_passed_failed_value, 0., 1.);
      fitParameters_["pTauId_passed_failed"] = fitParameterType(pTauId_passed_failed, pTauId_passed_failed_value);

      makeRooFormulaVar = &makeRooFormulaVar_2regions;
    } else throw cms::Exception("processEntryType")  
	<< "Invalid parameter 'regionsToFit' = " << format_vstring(regionsToFit) << "!!\n";

//--- fix relative contribution of Zmumu/mu --> tau fake background to Monte Carlo prediction
//    and take template histogram for 'failed' region from 'passed' region
//   (to work-around problem with low MC statistics)
    if ( //name_ == "Zmumu"     ||
	 name_ == "EWKmuFake" ) {
      fitParameters_["pTauId_passed_failed"].fittedValue_->setConstant(true);

      const std::string& fitVariableName_passed = fitVariables[region_passed].name_;
      histogramMap1& histograms_passed = histograms_[region_passed][fitVariableName_passed];
      const std::string& fitVariableName_failed = fitVariables[region_failed].name_;
      histogramMap1& histograms_failed = histograms_[region_failed][fitVariableName_failed];
      for ( histogramMap1::const_iterator sysUncertainty = histograms_[region_passed][fitVariableName_passed].begin();
	    sysUncertainty != histograms_[region_passed][fitVariableName_passed].end(); ++sysUncertainty ) {
	histograms_failed[sysUncertainty->first] = histograms_passed[sysUncertainty->first];
	fittedFractions_[region_failed][fitVariableName_failed] = fittedFractions_[region_passed][fitVariableName_passed];
      }    
    }
    
    assert(makeRooFormulaVar);

    numCategories_["A"]           = 4;
    numCategories_["B"]           = 4;
    numCategories_["C2"]          = 4;
    numCategories_["D"]           = 4;
    numCategories_[region_passed] = 2;
    numCategories_[region_failed] = 2;
    
    for ( vstring::const_iterator region = regionsToFit.begin();
	  region != regionsToFit.end(); ++region ) {
      const std::string& fitVariableName = fitVariables_[*region].name_;

      assert(numCategories_.find(*region) != numCategories_.end());
      normFactors_[*region] =
	(*makeRooFormulaVar)(name, *region, region_passed, region_failed, 
			     norm_.fittedValue_, fittedFractions_[*region][fitVariableName], numCategories_[*region], fitParameters_);

      if ( templateMorphingMode == kNoTemplateMorphing ) 
	pdfs_[*region] = 
	  makeRooHistPdf(histograms_[*region][fitVariableName][key_central_value], fitVariables_[*region].xAxis_);
      else if ( templateMorphingMode == kHorizontalTemplateMorphing ) 
	pdfs_[*region] = 
	  makePdfHorizontalMorphing(histograms_, *region, fitVariables_[*region], 
				    sysUncertainties_, alphaParameters_);
      else if ( templateMorphingMode == kVerticalTemplateMorphing ) 
	pdfs_[*region] = 
	  makePdfVerticalMorphing(histograms_, *region, fitVariables_[*region], 
				  sysUncertainties_, alphaParameters_);
      else assert(0);
    }
  }
  std::string name_; 
  histogramMap3 histograms_;                              // key = (region, observable, central value/systematic uncertainty)  
  valueMap2 numEvents_;                                   // key = (region, central value/systematic uncertainty)
  std::map<std::string, fitVariableType> fitVariables_;   // key =  region
  static std::map<std::string, RooRealVar*> gFitVariableMap;       // key = fit variable name
  static std::map<std::string, std::string> gFitVariableRefRegion; // key = fit variable name
  vstring regionsToFit_;
  std::string region_passed_;
  std::string region_failed_;
  vstring sysUncertainties_;
  int templateMorphingMode_;
  std::string legendEntry_;
  int fillColor_;
  rooAbsRealMap3 fittedFractionParameters_;               // key = (region, observable, central value/systematic uncertainty)
  rooAbsRealMap2 fittedFractions_;                        // key = (region, observable)
  std::map<std::string, unsigned> numCategories_;         // key = region
  fitParameterType norm_;
  std::map<std::string, RooFormulaVar*> normFactors_;     // key = region
  std::map<std::string, fitParameterType> fitParameters_; // key = fit parameter name:
                                                          //      o 'pDiTauCharge_OS_SS'
                                                          //      o 'pMuonIso_tight_loose'
                                                          //      o 'pDiTauKine_Sig_Bgr'
                                                          //      o 'pTauId_passed_failed'
  std::map<std::string, alphaParameterType> alphaParameters_; // key = systematic uncertainty
  histogramMap2 fittedTemplateShapes_; // key = (region, observable)
  std::map<std::string, RooAbsPdf*> pdfs_; // key = region
};

// define static variables shared by all instances of processEntryType
std::map<std::string, RooRealVar*> processEntryType::gFitVariableMap;  
std::map<std::string, std::string> processEntryType::gFitVariableRefRegion;

//
//-------------------------------------------------------------------------------
//

double getTemplateNorm_fitted(processEntryType& processEntry, const std::string& region, const std::string& fitVariable)
{
  double retVal = processEntry.norm_.fittedValue_->getVal()*processEntry.fittedFractions_[region][fitVariable]->getVal();
  return retVal;
}

TH1* compFittedTemplateShape(processEntryType& processEntry, const std::string& region, const std::string& fitVariableName)
{
  //std::cout << "<compFittedTemplateShape>:" << std::endl;
  TH1* histogram_central_value = processEntry.histograms_[region][fitVariableName][key_central_value];
  //std::cout << " histogram_central_value = " << histogram_central_value << "," 
  //	      << " name = " << histogram_central_value->GetName() << std::endl;

  RooAbsPdf* pdf = processEntry.pdfs_[region];

  std::string histogramName_fitted = std::string(histogram_central_value->GetName()).append("_fittedShape");
  TH1* histogram_fitted = (TH1*)histogram_central_value->Clone(histogramName_fitted.data());

  TAxis* xAxis = histogram_central_value->GetXaxis();
  for ( int iBin = 1; iBin <= (histogram_fitted->GetNbinsX() + 1); ++iBin ) {
    double xMin = xAxis->GetBinLowEdge(iBin);
    double xMax = xAxis->GetBinUpEdge(iBin);
    if ( dynamic_cast<VerticalInterpPdf*>(pdf) ) { // special handling for Giovanni's vertical morphing pdf
      double binContent = histogram_central_value->GetBinContent(iBin);
      for ( vstring::const_iterator sysUncertainty = processEntry.sysUncertainties_.begin();
	    sysUncertainty != processEntry.sysUncertainties_.end(); ++sysUncertainty ) {
	std::string key_up = std::string(*sysUncertainty).append("Up");
	TH1* histogram_up = processEntry.histograms_[region][fitVariableName][key_up];
	//std::cout << " histogram_up = " << histogram_up << std::endl;

	std::string key_down = std::string(*sysUncertainty).append("Down");
	TH1* histogram_down = processEntry.histograms_[region][fitVariableName][key_down];
	//std::cout << " histogram_down = " << histogram_down << std::endl;

	double coeff = processEntry.alphaParameters_[*sysUncertainty].alpha_->getVal();

	if      ( coeff > 0. ) binContent += coeff*(histogram_up->GetBinContent(iBin) - histogram_central_value->GetBinContent(iBin));
	else if ( coeff < 0. ) binContent += coeff*(histogram_central_value->GetBinContent(iBin) - histogram_down->GetBinContent(iBin));
      }
      histogram_fitted->SetBinContent(iBin, binContent);
    } else { // all other pdfs
      RooRealVar* fitVariable = processEntry.fitVariables_[region].xAxis_;
      TString binLabel = Form("bin%i", iBin);
      fitVariable->getBinning(binLabel.Data(), false, true);
      fitVariable->setRange(binLabel.Data(), xMin, xMax);
      RooAbsReal* pdfIntegral_bin = pdf->createIntegral(*fitVariable, RooFit::NormSet(*fitVariable), RooFit::Range(binLabel.Data()));    
      double integral = pdfIntegral_bin->getVal();
      //std::cout << "integral(" << xMin << ".." << xMax << ") = " << integral << std::endl;
      histogram_fitted->SetBinContent(iBin, integral);
      delete pdfIntegral_bin;
    }
  }
  return histogram_fitted;
}

//
//-------------------------------------------------------------------------------
//

void fitUsingRooFit(processEntryType& data, double intLumiData, 
		    std::map<std::string, processEntryType*>& processEntries, // key = process name
		    double sysVariedByNsigma, const std::string& processName_signal,
		    const std::string& tauId, double& effValue, double& effError, bool& hasFitConverged,
		    int verbosity = 0)
{
  if ( verbosity ) {
    std::cout << "<fitUsingRooFit>:" << std::endl;
    std::cout << " performing Fit of variable = " << data.fitVariables_[data.region_passed_].name_ 
	      << " for Tau id. = " << tauId << std::endl;
  }

  std::map<std::string, RooAddPdf*> pdfsSum;
  
  for ( vstring::const_iterator region = data.regionsToFit_.begin();
	region != data.regionsToFit_.end(); ++region ) {
    TObjArray pdfs_region;
    TObjArray fitParameters_region;
    for ( std::map<std::string, processEntryType*>::const_iterator processEntry = processEntries.begin();
	  processEntry != processEntries.end(); ++processEntry ) {
      if ( processEntry->first == "mcSum" ) continue;
      pdfs_region.Add(processEntry->second->pdfs_[*region]);
      fitParameters_region.Add(processEntry->second->normFactors_[*region]);
    }
    
    std::string pdfSumName = std::string("pdfSum").append(*region);
    pdfsSum[*region] = 
      new RooAddPdf(pdfSumName.data(),
		    pdfSumName.data(), RooArgList(pdfs_region), RooArgList(fitParameters_region));
  }
//
// CV: due to limitation in RooFit
//    (cf. http://root.cern.ch/phpBB3/viewtopic.php?f=15&t=9518)
//     need to construct log-likelihood functions separately for regions { A, B, D } and { C1p, C1f }
//
  RooCategory* fitCategoriesABC2D = new RooCategory("categoriesABC2D", "categoriesABC2D");
  RooSimultaneous* pdfSimultaneousFitABC2D = 
    new RooSimultaneous("pdfSimultaneousFitABC2D", 
			"pdfSimultaneousFitABC2D", *fitCategoriesABC2D);
  histogramMap1 histogramDataMapABC2D; // key = region
  TObjArray fitConstraintsABC2D;
  bool doFitABC2D = false;

  RooCategory* fitCategoriesC1 = new RooCategory("categoriesC1", "categoriesC1");
  RooSimultaneous* pdfSimultaneousFitC1 = 
    new RooSimultaneous("pdfSimultaneousFitC1", 
			"pdfSimultaneousFitC1", *fitCategoriesC1);
  histogramMap1 histogramDataMapC1; // key = region
  TObjArray fitConstraintsC1;
  bool doFitC1 = false;

  for ( vstring::const_iterator region = data.regionsToFit_.begin();
	region != data.regionsToFit_.end(); ++region ) {
    TObjArray* fitConstraints = 0;
    if ( (*region) == "A"  ||
	 (*region) == "B"  ||
	 (*region) == "C2" ||
	 (*region) == "D"  ) {
      fitCategoriesABC2D->defineType(region->data());
      pdfSimultaneousFitABC2D->addPdf(*pdfsSum[*region], region->data());
      histogramDataMapABC2D[*region] = data.histograms_[*region][data.fitVariables_[*region].name_][key_central_value];      
      fitConstraints = &fitConstraintsABC2D;
      doFitABC2D = true;
    } else if ( (*region) == "C1p" ||
		(*region) == "C1f" || (*region) == "D1p" ) {
      fitCategoriesC1->defineType(region->data());
      pdfSimultaneousFitC1->addPdf(*pdfsSum[*region], region->data());
      histogramDataMapC1[*region] = data.histograms_[*region][data.fitVariables_[*region].name_][key_central_value];
      fitConstraints = &fitConstraintsC1;
      doFitC1 = true;
    } 

    assert(fitConstraints);

    for ( std::map<std::string, processEntryType*>::const_iterator processEntry = processEntries.begin();
	  processEntry != processEntries.end(); ++processEntry ) {
      if ( processEntry->first == "mcSum" ) continue;
      for ( vstring::const_iterator sysUncertainty = processEntry->second->sysUncertainties_.begin();
	    sysUncertainty != processEntry->second->sysUncertainties_.end(); ++sysUncertainty ) {
	alphaParameterType& alphaParameter = processEntry->second->alphaParameters_[*sysUncertainty];
	if ( processEntry->first == "TTplusJets" ||
	     processEntry->first == "Zmumu"      ||
	     processEntry->first == "EWKmuFake"  ||   
	     processEntry->first == "QCD"        ) {
	  alphaParameter.alpha_->setConstant(true);
	} else {
	  bool isLowStatistics = false;
	  for ( vstring::const_iterator region = processEntry->second->regionsToFit_.begin(); 
		region != processEntry->second->regionsToFit_.end(); ++region ) {
	    if ( processEntry->second->histograms_[*region][processEntry->second->fitVariables_[*region].name_][key_central_value]->GetEntries() < 10 ) isLowStatistics = true;
	  }
	  if ( isLowStatistics ) {
	    alphaParameter.alpha_->setConstant(true);
	  } else {
	    bool isAlreadyIncluded = false;
	    for ( int iFitConstraint = 0; iFitConstraint < fitConstraints->GetEntries(); ++iFitConstraint ) {
	      if ( dynamic_cast<RooGaussian*>(fitConstraints->At(iFitConstraint)) ) {
		RooGaussian* fitConstraint = dynamic_cast<RooGaussian*>(fitConstraints->At(iFitConstraint));
		if ( fitConstraint->GetName() == std::string(alphaParameter.alpha_->GetName()).append("_constraint") ) 
		  isAlreadyIncluded = true;
	      }
	    }
	    if ( !isAlreadyIncluded ) {
	      std::cout << "process = " << processEntry->first << ": adding alpha-parameter = " << alphaParameter.name_ << std::endl;
	      if ( !alphaParameter.isBiDirectional_ ) // horizontal morphing
		fitConstraints->Add(makeFitConstraint(alphaParameter.alpha_, 0.5, 0.5/sysVariedByNsigma));
	      else                                    // vertical morphing
		fitConstraints->Add(makeFitConstraint(alphaParameter.alpha_, 0.0, 1.0/sysVariedByNsigma));
	    } 
	  }
	}
      }
    }
  }

  if ( !doFitC1 )
    throw cms::Exception("fitTauIdEff") 
      << "No constraints defined for region 'C1' !!\n";

  for ( std::map<std::string, processEntryType*>::const_iterator processEntry = processEntries.begin();
	processEntry != processEntries.end(); ++processEntry ) {
    std::cout << "process = " << processEntry->first << std::endl;
    if ( processEntry->first == "mcSum" ) continue;

    if ( processEntry->first == "Ztautau"    ||
	 processEntry->first == "WplusJets"  ||
	 processEntry->first == "EWKjetFake" ||
	 processEntry->first == "TTplusJets" )
      fitConstraintsC1.Add(makeFitConstraint(processEntry->second->norm_.fittedValue_, 
					     processEntry->second->norm_.expectedValue_, TMath::Max(1.e+1, 0.5*processEntry->second->norm_.expectedValue_)));
    else if ( processEntry->first == "QCD" ) {
      //fitConstraintsC1.Add(makeFitConstraint(processEntry->second->norm_.fittedValue_, 
      //                                       processEntry->second->norm_.expectedValue_, TMath::Max(1.e+1, 0.5*processEntry->second->norm_.expectedValue_)));
      processEntry->second->norm_.fittedValue_->setVal(0.33*processEntry->second->norm_.fittedValue_->getVal());
      processEntry->second->norm_.fittedValue_->setConstant(true);
    } else if ( processEntry->first == "Zmumu"     || 
	        processEntry->first == "EWKmuFake" )
      fitConstraintsC1.Add(makeFitConstraint(processEntry->second->norm_.fittedValue_, 
					     processEntry->second->norm_.expectedValue_, TMath::Max(1.e+1, 1.0*processEntry->second->norm_.expectedValue_)));
    processEntry->second->norm_.fittedValue_->setMin(1.);
    if ( processEntry->first == "EWKmuFake" ) processEntry->second->norm_.fittedValue_->setMax(5.*processEntry->second->norm_.expectedValue_);
    
    std::map<std::string, bool> isLowStatistics; // key = region
    for ( vstring::const_iterator region = processEntry->second->regionsToFit_.begin(); 
	  region != processEntry->second->regionsToFit_.end(); ++region ) {
      if ( processEntry->second->histograms_[*region][processEntry->second->fitVariables_[*region].name_][key_central_value]->GetEntries() < 10 ) isLowStatistics[*region] = true;
      else isLowStatistics[*region] = false;
    }

    for ( std::map<std::string, fitParameterType>::const_iterator fitParameter = processEntry->second->fitParameters_.begin();
	  fitParameter != processEntry->second->fitParameters_.end(); ++fitParameter ) {           
      if ( fitParameter->first == "pDiTauCharge_OS_SS" ) {
	//if ( isLowStatistics["A"] && 
	//     isLowStatistics["B"] && 
	//     isLowStatistics["D"] ) {
	//  fitParameter->second.fittedValue_->setConstant(true);
	//} else {
	  if ( processEntry->first == "Ztautau" )
	    //fitParameter->second.fittedValue_->setConstant(true);
	    fitConstraintsABC2D.Add(makeFitConstraint(fitParameter->second.fittedValue_, 
						      fitParameter->second.expectedValue_, 0.025));
	  else if ( processEntry->first == "Zmumu"      ||
		    processEntry->first == "WplusJets"  ||
		    processEntry->first == "EWKjetFake" )
	    fitConstraintsABC2D.Add(makeFitConstraint(fitParameter->second.fittedValue_,
						      fitParameter->second.expectedValue_, 0.10));
	  else 
	    fitConstraintsABC2D.Add(makeFitConstraint(fitParameter->second.fittedValue_, 
						      fitParameter->second.expectedValue_, 0.05));
	//}
      } else if ( fitParameter->first == "pMuonIso_tight_loose" ) {
	if ( isLowStatistics["A"] && 
	     isLowStatistics["B"] && 
	     isLowStatistics["D"] ) {
	  fitParameter->second.fittedValue_->setConstant(true);
	} else {
	  //if ( processEntry->first == "Ztautau" )
	  //  fitParameter->second.fittedValue_->setConstant(true);
	  //else if ( processEntry->first == "Zmumu"      ||
   	  if ( processEntry->first == "Ztautau"    ||
	       processEntry->first == "Zmumu"      ||
	       processEntry->first == "WplusJets"  ||
	       processEntry->first == "EWKmuFake"  ||
	       processEntry->first == "EWKjetFake" )
	    fitConstraintsABC2D.Add(makeFitConstraint(fitParameter->second.fittedValue_, 
						      fitParameter->second.expectedValue_, 0.01));
	  else if ( processEntry->first == "TTplusJets" )
	    fitConstraintsABC2D.Add(makeFitConstraint(fitParameter->second.fittedValue_, 
						      fitParameter->second.expectedValue_, 0.02));
	  //else if ( !(processEntry->first == "QCD") )
          else
	    fitConstraintsABC2D.Add(makeFitConstraint(fitParameter->second.fittedValue_, 
						      fitParameter->second.expectedValue_, TMath::Max(0.05, 0.5*fitParameter->second.expectedValue_)));
	}
      } else if ( fitParameter->first == "pDiTauKine_Sig_Bgr" ) {
	if ( isLowStatistics["C1p"] && 
	     isLowStatistics["C1f"] && 
	     isLowStatistics["C2"]  ) {
	  fitParameter->second.fittedValue_->setConstant(true);
	} else {
	  fitConstraintsABC2D.Add(makeFitConstraint(fitParameter->second.fittedValue_, 
						    fitParameter->second.expectedValue_, TMath::Max(0.01, 0.10*fitParameter->second.expectedValue_)));
	}
      } else if ( fitParameter->first == "pTauId_passed_failed" ) {
	if ( isLowStatistics["C1p"] && 
	     isLowStatistics["C1f"] ) { 
	  fitParameter->second.fittedValue_->setConstant(true);
	} else {
	  if ( processEntry->first == "WplusJets"  ||
	       processEntry->first == "EWKjetFake" ||
	       processEntry->first == "QCD"        )
	    fitConstraintsC1.Add(makeFitConstraint(fitParameter->second.fittedValue_, 
						   fitParameter->second.expectedValue_, TMath::Max(0.01, 0.2*fitParameter->second.expectedValue_)));
	  else if ( processEntry->first == "Zmumu"     ||
		    processEntry->first == "EWKmuFake" )
	    fitConstraintsC1.Add(makeFitConstraint(fitParameter->second.fittedValue_, 
						   fitParameter->second.expectedValue_, TMath::Max(0.05, 0.5*fitParameter->second.expectedValue_)));
	}
      }
    }
  }

  TObjArray nlls;
  if ( doFitABC2D ) {
    RooRealVar* fitVariableABC2D = data.fitVariables_["A"].xAxis_;
    RooDataHist* dataABC2D = 
      new RooDataHist("dataABC2D", 
		      "dataABC2D", *fitVariableABC2D, *fitCategoriesABC2D, histogramDataMapABC2D);
    RooLinkedList fitOptionsABC2D;
    fitOptionsABC2D.Add(new RooCmdArg(RooFit::Extended()));
    std::cout << "#fitConstraintsABC2D = " << fitConstraintsABC2D.GetEntries() << std::endl;
    if ( fitConstraintsABC2D.GetEntries() > 0 ) 
      fitOptionsABC2D.Add(new RooCmdArg(RooFit::ExternalConstraints(RooArgSet(fitConstraintsABC2D))));
    pdfSimultaneousFitABC2D->printCompactTree();
    RooAbsReal* nllABC2D = pdfSimultaneousFitABC2D->createNLL(*dataABC2D, fitOptionsABC2D); 
    nlls.Add(nllABC2D);
  }

  RooRealVar* fitVariableC1 = data.fitVariables_[data.region_passed_].xAxis_;
  RooDataHist* dataC1 = 
    new RooDataHist("dataC1", 
		    "dataC1", *fitVariableC1, *fitCategoriesC1, histogramDataMapC1);
  RooLinkedList fitOptionsC1;
  fitOptionsC1.Add(new RooCmdArg(RooFit::Extended()));
  std::cout << "#fitConstraintsC1 = " << fitConstraintsC1.GetEntries() << std::endl;
  if ( fitConstraintsC1.GetEntries() > 0 ) 
    fitOptionsC1.Add(new RooCmdArg(RooFit::ExternalConstraints(RooArgSet(fitConstraintsC1))));
  pdfSimultaneousFitC1->printCompactTree();
  RooAbsReal* nllC1 = pdfSimultaneousFitC1->createNLL(*dataC1, fitOptionsC1); 
  nlls.Add(nllC1);

//--- set tau id. efficiency to "random" value
  processEntries[processName_signal]->fitParameters_["pTauId_passed_failed"].fittedValue_->setVal(0.55);

  RooAddition nll("nll", "nll", RooArgSet(nlls));
  RooMinuit minuit(nll); 
  minuit.setErrorLevel(1);
  minuit.setNoWarn();
  minuit.setPrintEvalErrors(1);
  minuit.setPrintLevel(0);
  //minuit.setWarnLevel(1);
  minuit.migrad(); 
  minuit.hesse(); 

//--- unpack covariance matrix of fit parameters
  std::string fitResultName = std::string("fitResult").append("_").append(tauId);
  RooFitResult*	fitResult = minuit.save(fitResultName.data(), fitResultName.data());
   
  effValue = processEntries[processName_signal]->fitParameters_["pTauId_passed_failed"].fittedValue_->getVal();
  effError = processEntries[processName_signal]->fitParameters_["pTauId_passed_failed"].fittedValue_->getError();
  hasFitConverged = (fitResult->status() == 0) ? true : false;
  
//--- store fitted/morphed template shapes
  for ( std::map<std::string, processEntryType*>::const_iterator processEntry = processEntries.begin();
	processEntry != processEntries.end(); ++processEntry ) {
    std::cout << "storing fitted template shapes for process = " << processEntry->first << "..." << std::endl;
    for ( vstring::const_iterator region = data.regionsToFit_.begin();
	  region != data.regionsToFit_.end(); ++region ) {
      if ( processEntry->first == "mcSum" ) continue;
      processEntry->second->fittedTemplateShapes_[*region][data.fitVariables_[*region].name_] =
	compFittedTemplateShape(*processEntry->second, *region, data.fitVariables_[*region].name_);
      std::cout << " histogram[" << (*region) << "][" << data.fitVariables_[*region].name_ << "] = "
		<< processEntry->second->fittedTemplateShapes_[*region][data.fitVariables_[*region].name_] << std::endl;
    }												 
  }

  if ( verbosity ) {
    std::cout << tauId << ":";
    if ( hasFitConverged ) std::cout << " fit converged."          << std::endl; 
    else                   std::cout << " fit failed to converge." << std::endl;
    
    const RooArgList& fitParameter = fitResult->floatParsFinal();
    
    int numFitParameter = fitParameter.getSize();
    
    TMatrixD cov(numFitParameter, numFitParameter);
    for ( int iParameter = 0; iParameter < numFitParameter; ++iParameter ) {
      const RooAbsArg* paramI_arg = fitParameter.at(iParameter);
      const RooRealVar* paramI = dynamic_cast<const RooRealVar*>(paramI_arg);    
      double sigmaI = paramI->getError();
      
      std::cout << " parameter #" << iParameter << ": " << paramI_arg->GetName() 
		<< " = " << paramI->getVal() << " +/- " << paramI->getError() << std::endl;
      
      for ( int jParameter = 0; jParameter < numFitParameter; ++jParameter ) {
	const RooAbsArg* paramJ_arg = fitParameter.at(jParameter);
	const RooRealVar* paramJ = dynamic_cast<const RooRealVar*>(paramJ_arg);
	double sigmaJ = paramJ->getError();
	
	double corrIJ = fitResult->correlation(*paramI_arg, *paramJ_arg);
	
	cov(iParameter, jParameter) = sigmaI*sigmaJ*corrIJ;
      }
    }
    
    cov.Print();
    
    std::cout << std::endl;

    std::cout << "Results of fitting variable = " << fitVariableC1->GetName() << " for Tau id. = " << tauId << std::endl;
    for ( std::map<std::string, processEntryType*>::const_iterator processEntry = processEntries.begin();
	  processEntry != processEntries.end(); ++processEntry ) {
      std::cout << " " << processEntry->second->name_ << ":" << std::endl;
      std::cout << "  normalization = " << processEntry->second->norm_.fittedValue_->getVal()
		<< " +/- " << processEntry->second->norm_.fittedValue_->getError()
		<< " (MC exp. = " << processEntry->second->norm_.expectedValue_ << ")" << std::endl;

      for ( std::map<std::string, fitParameterType>::const_iterator fitParameter = processEntry->second->fitParameters_.begin();
	    fitParameter != processEntry->second->fitParameters_.end(); ++fitParameter ) {
	std::cout << "  " << fitParameter->first << " = " << fitParameter->second.fittedValue_->getVal()
		  << " +/- " << fitParameter->second.fittedValue_->getError()
		  << " (MC exp. = " << fitParameter->second.expectedValue_ << ")" << std::endl;
      }

      if ( processEntry->second->alphaParameters_.begin() != processEntry->second->alphaParameters_.end() ) {
	std::cout << "  nuissance parameters:" << std::endl;
	for ( vstring::const_iterator sysUncertainty = processEntry->second->sysUncertainties_.begin();
	      sysUncertainty != processEntry->second->sysUncertainties_.end(); ++sysUncertainty ) {
	  alphaParameterType& alphaParameter = processEntry->second->alphaParameters_[*sysUncertainty];
	  std::cout << "   " << (*sysUncertainty) << " = " << alphaParameter.alpha_->getVal() 
		    << " +/- " << alphaParameter.alpha_->getError() << std::endl;
	}
      }

      for ( vstring::const_iterator region = data.regionsToFit_.begin();
	    region != data.regionsToFit_.end(); ++region ) {
	std::cout << "  region " << (*region) << " = " 
		  << processEntry->second->normFactors_[*region]->getVal()/
   	            (processEntry->second->fittedFractions_[*region][data.fitVariables_[*region].name_]->getVal()*processEntry->second->numCategories_[*region])
		  << " (MC exp. = " << processEntry->second->numEvents_[*region][key_central_value] << ","
		  << " fitted fraction = " << processEntry->second->fittedFractions_[*region][data.fitVariables_[*region].name_]->getVal() << ")" << std::endl;
      }
    }
    std::cout << " Data:" << std::endl;
    std::cout << "  events = " << data.numEvents_["ABCD"][key_central_value] 
	      << " +/- " << TMath::Sqrt(data.numEvents_["ABCD"][key_central_value]) << std::endl;
    for ( vstring::const_iterator region = data.regionsToFit_.begin();
	  region != data.regionsToFit_.end(); ++region ) {
      std::cout << "  region " << (*region) << " = " << data.numEvents_[*region][key_central_value] 
		<< " +/- " << TMath::Sqrt(data.numEvents_[*region][key_central_value]) << std::endl;
    }
  }
}

//
//-------------------------------------------------------------------------------
//

histogramMap3 sumHistograms(histogramMap4& histograms,
			    const vstring& mcNamesToSum, const std::string& mcNameSum,
			    double mcToDataScaleFactor = 1.)
{
  histogramMap3 retVal;
  for ( std::vector<std::string>::const_iterator mcName = mcNamesToSum.begin();
	mcName != mcNamesToSum.end(); ++mcName ) {
    for ( histogramMap3::const_iterator region = histograms[*mcName].begin();
	  region != histograms[*mcName].end(); ++region ) {
      for ( histogramMap2::const_iterator observable = region->second.begin();
	    observable != region->second.end(); ++observable ) {
	for ( histogramMap1::const_iterator sysUncertainty = observable->second.begin();
	      sysUncertainty != observable->second.end(); ++sysUncertainty ) {

	  TH1* histogram = sysUncertainty->second;
	  
	  TH1* histogramSum = retVal[region->first][observable->first][sysUncertainty->first];
	  if ( !histogramSum ) {
	    std::string histogramName = histogram->GetName();
	    std::string histogramSumName = std::string(mcNameSum).append(std::string(histogramName, histogramName.find("_")));
	    histogramSum = (TH1*)histogram->Clone(histogramSumName.data());
	    histogramSum->Scale(mcToDataScaleFactor);
	    retVal[region->first][observable->first][sysUncertainty->first] = histogramSum;
	  } else {	    	      
	    histogramSum->Add(histogram, mcToDataScaleFactor);
	  }
	}
      }
    }
  }
  return retVal;
}

//
//-------------------------------------------------------------------------------
//

void drawHistograms(const std::string& region, const std::string& observable, 
		    processEntryType& data, double intLumiData, 
		    std::map<std::string, processEntryType*>& processEntries, // key = processEntry.name
		    const vstring& mcNamesToDraw, 
		    bool normalizeTemplatesToFit, // false: normalize to MC expectation
		                                  // true:  normalize according to scale-factors determined by fit
		    const std::string& histogramTitle, const std::string& xAxisTitle, 
		    const std::string& outputFileName, 
		    bool runStatTest = false)
{
//--------------------------------------------------------------------------------
// Make control plots of sum(MC) versus Data.
// If normalization factors are passed as function argument,
// normalize all MC distributions accordingly;
// else assume MC distributions passed as function arguments
// are already properly normalized (by cross-section)
//--------------------------------------------------------------------------------

  //std::cout << "<drawHistograms>:" << std::endl;
  //std::cout << " region = " << region << std::endl;
  //std::cout << " observable = " << observable << std::endl;
  //std::cout << " mcNamesToDraw = " << format_vstring(mcNamesToDraw) << std::endl;

  TCanvas* canvas = new TCanvas("canvas", "canvas", 800, 640);
  canvas->SetFillColor(10);
  canvas->SetBorderSize(2);

  canvas->SetLeftMargin(0.14);
  canvas->SetBottomMargin(0.12);

  TH1* histogramData = data.histograms_[region][observable][key_central_value];
  applyStyleOption(histogramData, histogramTitle, xAxisTitle);
  histogramData->SetLineColor(1);
  histogramData->SetMarkerColor(1);
  histogramData->SetMarkerStyle(20);

  TLegend* legend = new TLegend(0.64, 0.59, 0.89, 0.89, "", "brNDC"); 
  legend->SetBorderSize(0);
  legend->SetFillColor(0);
  legend->AddEntry(histogramData, "Data", "p");
  
  THStack* mcStack = new THStack("mcSum", "mcSum");
  TH1* mcSum = 0;
  std::vector<TH1*> histogramsToDelete;

  for ( vstring::const_iterator mcName = mcNamesToDraw.begin();
	mcName != mcNamesToDraw.end(); ++mcName ) {
    TH1* histogramToDraw = 0;
    if ( normalizeTemplatesToFit ) {      
      if ( processEntries[*mcName]->fittedTemplateShapes_[region][observable] ) {
	histogramToDraw = 
	  normalize(processEntries[*mcName]->fittedTemplateShapes_[region][observable], 
		    processEntries[*mcName]->normFactors_[region]->getVal()/processEntries[*mcName]->numCategories_[region]);
      } else {
	histogramToDraw = 
	  normalize(processEntries[*mcName]->histograms_[region][observable][key_central_value],
		    processEntries[*mcName]->normFactors_[region]->getVal()/processEntries[*mcName]->numCategories_[region]);
      }
      //std::cout << "process = " << (*mcName) << ": norm-factor = " << processEntries[*mcName]->normFactors_[region]->getVal() << ","
      //	  << " #categories = " << processEntries[*mcName]->numCategories_[region] 
      //	  << " --> integral = " << histogramToDraw->Integral() << std::endl;
      histogramsToDelete.push_back(histogramToDraw);
    } else {
      histogramToDraw = processEntries[*mcName]->histograms_[region][observable][key_central_value];
    }

    if ( mcSum ) {
      mcSum->Add(histogramToDraw);
    } else {
      mcSum = (TH1*)histogramToDraw->Clone("mcSum");
    }

    applyStyleOption(histogramToDraw, histogramTitle, xAxisTitle);
    histogramToDraw->SetFillStyle(1001);
    histogramToDraw->SetFillColor(processEntries[*mcName]->fillColor_);

    legend->AddEntry(histogramToDraw, processEntries[*mcName]->legendEntry_.data(), "f");

    mcStack->Add(histogramToDraw);
  }

  mcStack->SetMaximum(1.5*TMath::Max(mcSum->GetMaximum(), histogramData->GetMaximum()));
  mcStack->Draw("hist");
  applyStyleOption(mcStack, histogramTitle, xAxisTitle);
  
  histogramData->SetStats(false);
  histogramData->Draw("ep1same");

  legend->Draw();

  drawCMSprelimaryLabels(intLumiData);

  canvas->Update();

  size_t idx = outputFileName.find_last_of('.');
  std::string outputFileName_plot = std::string(outputFileName, 0, idx);
  if ( idx != std::string::npos ) canvas->Print(std::string(outputFileName_plot).append(std::string(outputFileName, idx)).data());
  canvas->Print(std::string(outputFileName_plot).append(".eps").data());
  canvas->Print(std::string(outputFileName_plot).append(".png").data());
  canvas->Print(std::string(outputFileName_plot).append(".pdf").data());

  if ( runStatTest ) {
    std::cout << "<runStatTest>:" << std::endl;
    std::cout << " histogram = " << histogramData->GetName() << std::endl;
    std::cout << " Data: entries = " << getIntegral(histogramData, false, false)
	      << " (" << getIntegral(histogramData, true, true) << " incl. underflow/overflow bins)" << std::endl;
    std::cout << " MC: entries  = " << getIntegral(mcSum, false, false)
	      << " (" << getIntegral(mcSum, true, true) << " incl. underflow/overflow bins)" << std::endl;
    std::cout << " p(chi^2) = " << histogramData->Chi2Test(mcSum, "UW") << std::endl;
    std::cout << " p(KS) = " << histogramData->KolmogorovTest(mcSum, "") << std::endl;
  }
  
  for ( std::vector<TH1*>::iterator it = histogramsToDelete.begin();
	it != histogramsToDelete.end(); ++it ) {
    delete (*it);
  }

  delete mcStack;
  delete mcSum;
  delete legend;
  delete canvas;
}

//
//-------------------------------------------------------------------------------
//

void compSysHistograms(histogramMap4& histograms, const std::string& sysUncertainty)
{
  for ( histogramMap4::iterator process = histograms.begin();
	process != histograms.end(); ++process ) {
    for ( histogramMap3::iterator region = process->second.begin();
	  region != process->second.end(); ++region ) {
      for ( histogramMap2::iterator observable = region->second.begin();
	    observable != region->second.end(); ++observable ) {
	for ( histogramMap1::iterator sysUncertainty = observable->second.begin();
	      sysUncertainty != observable->second.end(); ++sysUncertainty ) {
	  std::string key_up = std::string(sysUncertainty->first).append("Up");
	  TH1* histogram_up = observable->second[key_up];
	  assert(histogram_up);
	  if ( !histogram_up->GetSumw2N() ) histogram_up->Sumw2();

	  std::string key_down = std::string(sysUncertainty->first).append("Down");
	  TH1* histogram_down = observable->second[key_down];
	  assert(histogram_down);
	  if ( !histogram_down->GetSumw2N() ) histogram_down->Sumw2();

	  assert(isCompatibleBinning(histogram_up, histogram_down));
  
	  std::string histogramName_diff = std::string(histogram_up->GetName()).append("_diff");
	  TH1* histogram_diff = (TH1*)histogram_up->Clone(histogramName_diff.data());
	  
	  unsigned numBins = histogram_up->GetNbinsX();
	  for ( unsigned iBin = 0; iBin <= (numBins + 1); ++iBin ) {
	    double binContent_diff = 0.5*(histogram_up->GetBinContent(iBin) - histogram_down->GetBinContent(iBin));
	    histogram_diff->SetBinContent(iBin, binContent_diff);
	    
	    double binError_up   = histogram_up->GetBinError(iBin);
	    double binError_down = histogram_down->GetBinError(iBin);
	    double binError_diff = TMath::Sqrt(binError_up*binError_up + binError_down*binError_down);
	    histogram_diff->SetBinError(iBin, binError_diff);
	  }
	  
	  std::string key_diff =  std::string(sysUncertainty->first).append("Diff");
	  observable->second[key_diff] = histogram_diff;
	}
      }	    
    }
  }
} 

void savePseudoExperimentHistograms(TH1* histogram, const std::string& xAxisLabel, const std::string& outputFileName)
{
  TCanvas* canvas = new TCanvas("canvas", "canvas", 800, 640);
  canvas->SetFillColor(10);
  canvas->SetBorderSize(2);

  canvas->SetLeftMargin(0.12);
  canvas->SetBottomMargin(0.12);

  TH1* histogram_normalized = normalize(histogram);
      
  applyStyleOption(histogram_normalized, histogram->GetTitle(), xAxisLabel, "a.u");

  canvas->Update();

  size_t idx = outputFileName.find_last_of('.');
  std::string outputFileName_plot = std::string(outputFileName, 0, idx);
  outputFileName_plot.append("_").append(histogram->GetName());
  if ( idx != std::string::npos ) canvas->Print(std::string(outputFileName_plot).append(std::string(outputFileName, idx)).data());
  canvas->Print(std::string(outputFileName_plot).append(".png").data());
  canvas->Print(std::string(outputFileName_plot).append(".pdf").data());

  delete canvas;
}

void saveValueAsHistogram(TFileDirectory& histogramOutputDirectory, double value, double error,
			  const std::string& histogramName, const std::string& histogramTitle, 
			  const std::string& fitVariable, const std::string& tauId)
{
  TString histogramName_expanded = Form(histogramName.data(), fitVariable.data(), tauId.data());
  TString histogramTitle_expanded = Form(histogramTitle.data(), tauId.data(), fitVariable.data());
  TH1* histogram = histogramOutputDirectory.make<TH1F>(histogramName_expanded.Data(), histogramTitle_expanded.Data(), 1, -0.5, +0.5);
  int bin = histogram->FindBin(0.);
  histogram->SetBinContent(bin, value);
  histogram->SetBinError(bin, error);
}

//
//-------------------------------------------------------------------------------
//

int main(int argc, const char* argv[])
{
//--- parse command-line arguments
  if ( argc < 2 ) {
    std::cout << "Usage: " << argv[0] << " [parameters.py]" << std::endl;
    return 0;
  }

  std::cout << "<fitTauIdEff>:" << std::endl;

//--- disable pop-up windows showing graphics output
  gROOT->SetBatch(true);

//--- load framework libraries
  gSystem->Load("libFWCoreFWLite");
  AutoLibraryLoader::enable();

//--- keep track of time it takes the macro to execute
  TBenchmark clock;
  clock.Start("fitTauIdEff");

//--- read python configuration parameters
  if ( !edm::readPSetsFrom(argv[1])->existsAs<edm::ParameterSet>("process") ) 
    throw cms::Exception("fitTauIdEff") 
      << "No ParameterSet 'process' found in configuration file = " << argv[1] << " !!\n";

  edm::ParameterSet cfg = edm::readPSetsFrom(argv[1])->getParameter<edm::ParameterSet>("process");

  edm::ParameterSet cfgFitTauIdEff = cfg.getParameter<edm::ParameterSet>("fitTauIdEff");

  std::string tauId = cfgFitTauIdEff.getParameter<std::string>("tauId");
  vstring tauIds;
  tauIds.push_back(tauId);

  double intLumiData = cfgFitTauIdEff.getParameter<double>("intLumiData"); // in units of fb^-1

  bool runClosureTest = cfgFitTauIdEff.getParameter<bool>("runClosureTest");

  std::string fitVariable = cfgFitTauIdEff.getParameter<std::string>("fitVariable");
  std::map<std::string, fitVariableType> fitVariables;
  vstring observables;  
  observables.push_back("EventCounter");
  vstring regionsToFit = cfgFitTauIdEff.getParameter<vstring>("regions");
  vstring regionsToLoad = regionsToFit;
  std::string region_passed = cfgFitTauIdEff.getParameter<std::string>("region_passed");
  std::string region_failed = cfgFitTauIdEff.getParameter<std::string>("region_failed");
  if ( regionsToFit.size() == 6 &&
       contains_string(regionsToFit, "A")           &&
       contains_string(regionsToFit, "B")           &&	 
       contains_string(regionsToFit, region_passed) &&
       contains_string(regionsToFit, region_failed) &&
       contains_string(regionsToFit, "C2")          &&
       contains_string(regionsToFit, "D")         ) {
    fitVariables["A"]           = fitVariableType("diTauMt");
    fitVariables["B"]           = fitVariableType("diTauMt");
    fitVariables[region_passed] = fitVariableType(fitVariable);
    fitVariables[region_failed] = fitVariableType(fitVariable);
    fitVariables["C2"]          = fitVariableType("diTauMt");
    fitVariables["D"]           = fitVariableType("diTauMt");
    observables.push_back(fitVariable);
    observables.push_back(std::string("diTauMt"));
    add_string_uniquely(regionsToLoad, "C");    // needed to initialize probabilities 
    add_string_uniquely(regionsToLoad, "C1");   //  'pDiTauCharge_OS_SS', 'pMuonIso_tight_loose' and 'pDiTauKine_Sig_Bgr'
    if ( runClosureTest ) 
      add_string_uniquely(regionsToLoad, "A1"); // needed for QCD template 
  } else if ( regionsToFit.size() == 2 &&
	      contains_string(regionsToFit, region_passed) &&
	      contains_string(regionsToFit, region_failed) ) {
    fitVariables[region_passed] = fitVariableType(fitVariable);
    fitVariables[region_failed] = fitVariableType(fitVariable);
    observables.push_back(fitVariable);
  } else throw cms::Exception("fitTauIdEff") 
      << "Invalid configuration parameter 'regions' = " << format_vstring(regionsToFit) << "!!\n";

  std::string regionQCDtemplate_passed = cfgFitTauIdEff.getParameter<std::string>("regionQCDtemplate_passed");
  std::string regionQCDtemplate_failed = cfgFitTauIdEff.getParameter<std::string>("regionQCDtemplate_failed");
  std::string regionQCDtemplate_D      = cfgFitTauIdEff.getParameter<std::string>("regionQCDtemplate_D");
  vstring regionsQCDtemplate;
  add_string_uniquely(regionsQCDtemplate, regionQCDtemplate_passed);
  add_string_uniquely(regionsQCDtemplate, regionQCDtemplate_failed);
  if ( regionQCDtemplate_D != "" ) add_string_uniquely(regionsQCDtemplate, regionQCDtemplate_D);

  std::string templateMorphingMode_string = cfgFitTauIdEff.getParameter<std::string>("templateMorphingMode");
  int templateMorphingMode = -1;
  if      ( templateMorphingMode_string == "none"       ) templateMorphingMode = kNoTemplateMorphing;
  else if ( templateMorphingMode_string == "horizontal" ) templateMorphingMode = kHorizontalTemplateMorphing;
  else if ( templateMorphingMode_string == "vertical"   ) templateMorphingMode = kVerticalTemplateMorphing;
  else throw cms::Exception("fitTauIdEff") 
    << "Invalid configuration parameter 'templateMorphingMode' = " << templateMorphingMode_string << "!!\n";

  vstring sysUncertainties = cfgFitTauIdEff.getParameter<vstring>("sysUncertainties");
  vstring sysUncertainties_expanded;
  sysUncertainties_expanded.push_back(key_central_value);
  for ( vstring::const_iterator sysUncertainty = sysUncertainties.begin();
	sysUncertainty != sysUncertainties.end(); ++sysUncertainty ) {
    sysUncertainties_expanded.push_back(std::string(*sysUncertainty).append("Up"));
    sysUncertainties_expanded.push_back(std::string(*sysUncertainty).append("Down"));
  }

  double sysVariedByNsigma = cfgFitTauIdEff.getParameter<double>("sysVariedByNsigma");

  fwlite::InputSource inputFiles(cfg); 
  if ( inputFiles.files().size() != 1 ) 
    throw cms::Exception("fitTauIdEff") 
      << "Input file must be unique, got = " << format_vstring(inputFiles.files()) << " !!\n";
  std::string histogramFileName = (*inputFiles.files().begin());
  
  TFile* histogramInputFile = new TFile(histogramFileName.data());
  std::string directory = cfgFitTauIdEff.getParameter<std::string>("directory");
  TDirectory* histogramInputDirectory = ( directory != "" ) ?
    dynamic_cast<TDirectory*>(histogramInputFile->Get(directory.data())) : histogramInputFile;
  if ( !histogramInputDirectory ) 
    throw cms::Exception("fitTauIdEff") 
      << "Directory = " << directory << " does not exists in input file = " << histogramFileName << " !!\n";

  std::map<std::string, processEntryType*> processEntries;
  histogramMap4 histograms_mc; // key = (process, region, observable, central value/systematic uncertainty)

  bool fitIndividualProcesses = cfgFitTauIdEff.getParameter<bool>("fitIndividualProcesses");
  vstring processes;
  std::string processName_signal;
  std::map<std::string, std::string> legendEntries;
  legendEntries["Ztautau"] = "Z/#gamma^{*} #rightarrow #tau^{+} #tau^{-}";
  legendEntries["Zmumu"] = "Z/#gamma^{*} #rightarrow #mu^{+} #mu^{-}";
  legendEntries["WplusJets"] = "W + jets";
  legendEntries["EWKmuFake"] = "EWK, #mu #rightarrow #tau_{had}";
  legendEntries["EWKjetFake"] = "EWK, jet #rightarrow #tau_{had}";
  legendEntries["QCD"] = "QCD";
  legendEntries["TTplusJets"] = "t#bar{t} + jets";
  std::map<std::string, int> fillColors;
  fillColors["Ztautau"] = 628;
  fillColors["Zmumu"] = 596;
  fillColors["WplusJets"] = 856;
  fillColors["EWKmuFake"] = 596;
  fillColors["EWKjetFake"] = 856;
  fillColors["QCD"] = 797;
  fillColors["TTplusJets"] = 618;
  // CV: need to add processes in reverse order in which they drawn,
  //     i.e. the process which is to be drawn on the bottom (top) needs to be added first (last)
  if ( fitIndividualProcesses ) {
    processes.push_back(std::string("TTplusJets"));    
    processes.push_back(std::string("Zmumu"));
    processes.push_back(std::string("WplusJets"));
    processes.push_back(std::string("QCD"));
    processName_signal = "Ztautau";
    processes.push_back(processName_signal);

    for ( vstring::const_iterator process = processes.begin();
	  process != processes.end(); ++process ) {
      histogramMap3 histograms_process;
      loadHistograms(histograms_process, histogramInputDirectory, 
		     *process, regionsToLoad, tauId, observables, sysUncertainties_expanded, true, true);
      processEntries[*process] = 
	new processEntryType(*process, histograms_process, fitVariables, regionsToFit, region_passed, region_failed, 
			     sysUncertainties, templateMorphingMode, legendEntries[*process], fillColors[*process]);
      histograms_mc[*process] = histograms_process;
    }
  } else {
    processes.push_back(std::string("TTplusJets"));
    processes.push_back(std::string("EWKmuFake"));
    processes.push_back(std::string("EWKjetFake"));
    processes.push_back(std::string("QCD"));
    processName_signal = "Ztautau";
    processes.push_back(processName_signal);

    histogramMap3 histograms_Ztautau_genTau;
    //loadHistograms(histograms_Ztautau_genTau, histogramInputDirectory, 
    //	             "Ztautau", regionsToLoad, tauId, observables, sysUncertainties_expanded, true, true, "GenTau");
    loadHistograms(histograms_Ztautau_genTau, histogramInputDirectory, 
		   "ZplusJets", regionsToLoad, tauId, observables, sysUncertainties_expanded, true, true, "GenTau");
    processEntries["Ztautau"] =
      new processEntryType("Ztautau", histograms_Ztautau_genTau, fitVariables, regionsToFit, region_passed, region_failed, 
			   sysUncertainties, templateMorphingMode, legendEntries["Ztautau"], fillColors["Ztautau"]);
    histograms_mc["Ztautau"] = histograms_Ztautau_genTau;
    
    histogramMap4 histograms_EWK_muFake;
    histogramMap4 histograms_EWK_jetFake;
    vstring processes_EWK;
    //processes_EWK.push_back(std::string("Ztautau"));
    //processes_EWK.push_back(std::string("Zmumu"));
    processes_EWK.push_back(std::string("ZplusJets"));
    processes_EWK.push_back(std::string("WplusJets"));
    for ( vstring::const_iterator process = processes_EWK.begin();
	  process != processes_EWK.end(); ++process ) {
      loadHistograms(histograms_EWK_muFake[*process], histogramInputDirectory, 
		     *process, regionsToLoad, tauId, observables, sysUncertainties_expanded, true, true, "MuToTauFake");
      loadHistograms(histograms_EWK_jetFake[*process], histogramInputDirectory, 
		     *process, regionsToLoad, tauId, observables, sysUncertainties_expanded, true, true, "JetToTauFake");
    }

    histogramMap3 histograms_EWKsum_muFake = sumHistograms(histograms_EWK_muFake, processes_EWK, "EWKmuFake");
    processEntries["EWKmuFake"] = 
      new processEntryType("EWKmuFake", histograms_EWKsum_muFake, fitVariables, regionsToFit, region_passed, region_failed, 
			   sysUncertainties, templateMorphingMode, legendEntries["EWKmuFake"], fillColors["EWKmuFake"]);
    histograms_mc["EWKmuFake"] = histograms_EWKsum_muFake;

    histogramMap3 histograms_EWKsum_jetFake = sumHistograms(histograms_EWK_jetFake, processes_EWK, "EWKjetFake");
    processEntries["EWKjetFake"] = 
      new processEntryType("EWKjetFake", histograms_EWKsum_jetFake, fitVariables, regionsToFit, region_passed, region_failed, 
			   sysUncertainties, templateMorphingMode, legendEntries["EWKjetFake"], fillColors["EWKjetFake"]);
    histograms_mc["EWKjetFake"] = histograms_EWKsum_jetFake;

    histogramMap3 histograms_QCD;
    loadHistograms(histograms_QCD, histogramInputDirectory, 
		   "QCD", regionsToLoad, tauId, observables, sysUncertainties_expanded, true, true);
    processEntries["QCD"] = 
      new processEntryType("QCD", histograms_QCD, fitVariables, regionsToFit, region_passed, region_failed, 
			   sysUncertainties, templateMorphingMode, legendEntries["QCD"], fillColors["QCD"]);
    histograms_mc["QCD"] = histograms_QCD;
    
    histogramMap3 histograms_TTplusJets;
    loadHistograms(histograms_TTplusJets, histogramInputDirectory, 
		   "TTplusJets", regionsToLoad, tauId, observables, sysUncertainties_expanded, true, true);
    processEntries["TTplusJets"] = 
      new processEntryType("TTplusJets", histograms_TTplusJets, fitVariables, regionsToFit, region_passed, region_failed, 
			   sysUncertainties, templateMorphingMode, legendEntries["TTplusJets"], fillColors["TTplusJets"]);
    histograms_mc["TTplusJets"] = histograms_TTplusJets;
  }

  histogramMap3 histograms_mcSum = sumHistograms(histograms_mc, processes, "mcSum");
  processEntries["mcSum"] = 
    new processEntryType("mcSum", histograms_mcSum, fitVariables, regionsToFit, region_passed, region_failed, 
			 sysUncertainties, templateMorphingMode, "Simulation", 10);
  histograms_mc["mcSum"] = histograms_mcSum;
  
  vstring sysUncertainties_data;
  sysUncertainties_data.push_back(key_central_value);
  histogramMap3 histograms_data; // key = (region, observable, key_central_value)
//--- closure test: fit sum(MC) instead of Data
  if ( runClosureTest ) {
    std::cout << "<fitTauIdEfficiency>:" << std::endl;
    std::cout << " running closure-test...fluctuating histograms..." << std::endl; 
    for ( vstring::const_iterator region = regionsToFit.begin();
	  region != regionsToFit.end(); ++region ) {
      for ( vstring::const_iterator observable = observables.begin();  
	    observable != observables.end(); ++observable ) {
	TH1* origHistogram = histograms_mc["mcSum"][*region][*observable][key_central_value];
	    
	TH1* fluctHistogram = (TH1*)origHistogram->Clone(TString(origHistogram->GetName()).Append("_fluctuated"));
	fluctHistogram->Reset();
	sampleHistogram_stat(origHistogram, fluctHistogram, -1., true);
	
	std::cout << "histograms_data[region = " << (*region) << "][observable = " << (*observable) << "]" 
		  << " = " << fluctHistogram << " (original = " << origHistogram << ")" << std::endl;
	std::cout << " (name = " << fluctHistogram->GetName() << ", integral = " << getIntegral(fluctHistogram, false, false) << ")" << std::endl;
	histograms_data[*region][*observable][key_central_value] = fluctHistogram;
      }
    }
  } else {
    loadHistograms(histograms_data, histogramInputDirectory, 
		   "Data", regionsToLoad, tauId, observables, sysUncertainties_data, true, false);
    loadHistograms(histograms_data, histogramInputDirectory, 
		   "Data", regionsQCDtemplate, tauId, observables, sysUncertainties_expanded, true, false);  
  }
  processEntryType* data = 
    new processEntryType("Data", histograms_data, fitVariables, regionsToFit, region_passed, region_failed, 
			 sysUncertainties_data, kNoTemplateMorphing, "Data", 1);
  
  bool runPseudoExperiments = cfgFitTauIdEff.getParameter<bool>("runPseudoExperiments");
  unsigned numPseudoExperiments = cfgFitTauIdEff.getParameter<unsigned>("numPseudoExperiments");

  bool makeControlPlots = cfgFitTauIdEff.getParameter<bool>("makeControlPlots");
  std::string controlPlotFilePath = cfgFitTauIdEff.getParameter<std::string>("controlPlotFilePath");

//--- scale initial values for normalization factors of all processes
//    by ratio to Data to MC event yields in region 'ABCD', 
//    in order to speed-up convergence of fit
  double scaleFactorMCtoData = data->numEvents_["ABCD"][key_central_value]/processEntries["mcSum"]->numEvents_["ABCD"][key_central_value];
  std::cout << "number of events in region ABCD: data = " << data->numEvents_["ABCD"][key_central_value] << "," 
	    << " sum(MC) = " << processEntries["mcSum"]->numEvents_["ABCD"][key_central_value] << std::endl;
  std::cout << "--> MC-to-Data scale-factor = " << scaleFactorMCtoData << std::endl;
  for ( std::map<std::string, processEntryType*>::iterator processEntry = processEntries.begin();
	processEntry != processEntries.end(); ++processEntry ) {
    processEntry->second->norm_.expectedValue_ = scaleFactorMCtoData*processEntry->second->numEvents_["ABCD"][key_central_value];
    processEntry->second->norm_.fittedValue_->setVal(processEntry->second->norm_.expectedValue_);
  }

//--- obtain template for QCD background from data,
//    from SS && Mt < 40 GeV && (Pzeta - 1.5 PzetaVis) > -20 GeV sideband
  if ( !runClosureTest ) {
    for ( vstring::const_iterator observable = observables.begin();  
	  observable != observables.end(); ++observable ) {
      for ( vstring::const_iterator sysUncertainty = sysUncertainties_expanded.begin();
	    sysUncertainty != sysUncertainties_expanded.end(); ++sysUncertainty ) {
	processEntries["QCD"]->histograms_[region_passed][*observable][*sysUncertainty] =
	  data->histograms_[regionQCDtemplate_passed][*observable][*sysUncertainty];
	processEntries["QCD"]->histograms_[region_failed][*observable][*sysUncertainty] =
	  data->histograms_[regionQCDtemplate_failed][*observable][*sysUncertainty];
	if ( regionQCDtemplate_D != "" )
	  processEntries["QCD"]->histograms_["D"][*observable][*sysUncertainty] =
	    data->histograms_[regionQCDtemplate_D][*observable][*sysUncertainty];
      }
    }

    std::cout << "<fitTauIdEfficiency>:" << std::endl;
    std::cout << " taking QCD template from data...updating histogramMap..." << std::endl; 
    for ( vstring::const_iterator region = regionsToFit.begin();
	  region != regionsToFit.end(); ++region ) {
      for ( vstring::const_iterator observable = observables.begin();
	    observable != observables.end(); ++observable ) {
	for ( vstring::const_iterator sysUncertainty = sysUncertainties_expanded.begin();
	      sysUncertainty != sysUncertainties_expanded.end(); ++sysUncertainty ) {
	  TH1* histogram = processEntries["QCD"]->histograms_[*region][*observable][*sysUncertainty];
	  std::cout << "histogramMap[region = " << (*region) << "][observable = " << (*observable) << "]" 
		    << "[sysUncertainty = " << (*sysUncertainty) << "] = " << histogram << std::endl;
	  std::cout << " (name = " << histogram->GetName() << ", integral = " << histogram->Integral() << ")" << std::endl;
	}
      }
    }
  }

//--- define x-axis titles
  std::map<std::string, std::string> xAxisTitles;
  xAxisTitles["diTauMt"]      = "M_{T}^{#muMET} / GeV";
  xAxisTitles["diTauVisMass"] = "M_{vis}^{#mu#tau} / GeV";

//--- make control plots plots of Data compared to sum(MC) scaled by cross-sections
//    for muonPt, tauPt, Mt, visMass,... distributions in different regions
  if ( makeControlPlots ) {
    for ( vstring::const_iterator region = regionsToFit.begin();
	  region != regionsToFit.end(); ++region ) {
      for ( vstring::const_iterator observable = observables.begin();  
	    observable != observables.end(); ++observable ) {
	if ( (*observable) == "EventCounter" ) continue;
        std::string outputFileName = controlPlotFilePath;
	if ( outputFileName.find_last_of("/") != (outputFileName.length() - 1) ) outputFileName.append("/");
	outputFileName.append("controlPlotsTauIdEff_");
	outputFileName.append(tauId).append("_");
	outputFileName.append(*region).append("_").append(*observable).append("_prefit.pdf");
	drawHistograms(*region, *observable, *data, intLumiData, processEntries, processes, false, 
		       "", xAxisTitles[*observable], outputFileName, false);
      }
    }
  }

  std::cout << "running fit for central values..." << std::endl;
  
  double effValue = 0.;
  double effError = 1.;
  bool hasFitConverged = false;  
  fitUsingRooFit(*data, intLumiData, processEntries, sysVariedByNsigma, processName_signal, 
		 tauId, effValue, effError, hasFitConverged, 1);
  
//--- make control plots of Data compared to sum(MC) scaled by normalization factors determined by fit
//    for muonPt, tauPt, Mt, visMass,... distributions in different regions
  if ( makeControlPlots ) {
    for ( vstring::const_iterator region = regionsToFit.begin();
	  region != regionsToFit.end(); ++region ) {
      for ( vstring::const_iterator observable = observables.begin();  
	    observable != observables.end(); ++observable ) {
	if ( (*observable) == "EventCounter" ) continue;
	std::string outputFileName = controlPlotFilePath;
	if ( outputFileName.find_last_of("/") != (outputFileName.length() - 1) ) outputFileName.append("/");
	outputFileName.append("controlPlotsTauIdEff_");
	outputFileName.append(tauId).append("_");
	outputFileName.append(*region).append("_").append(*observable).append("_postfit_");
	outputFileName.append(fitVariable).append(".pdf");
	drawHistograms(*region, *observable, *data, intLumiData, processEntries, processes, true, 
		       "", xAxisTitles[*observable], outputFileName, false);
      }
    }
  }

  std::cout << "Efficiency of Tau id. = " << tauId << ":" << std::endl;  
  std::cout << " fitVariable = " << fitVariable << ":" 
	    << " result = " << effValue*100. << " +/- " << effError*100. << "%";
  if  ( hasFitConverged ) std::cout << " (fit converged successfully)";
  else std::cout << " (fit failed to converge)";
  std::cout << std::endl;   
  double tauIdEffMCexp = processEntries[processName_signal]->fitParameters_["pTauId_passed_failed"].expectedValue_;
  std::cout << "(Monte Carlo prediction = " << tauIdEffMCexp*100. << "%)" << std::endl; 

  if ( runPseudoExperiments ) {

//--- compute difference between template histograms obtained for "up"/"down" shifts
    for ( vstring::const_iterator sysUncertainty = sysUncertainties.begin();
	  sysUncertainty != sysUncertainties.end(); ++sysUncertainty ) {
      compSysHistograms(histograms_mc, *sysUncertainty);
    }

    TString effDistributionName = Form("effDistribution_%s_%s", tauId.data(), fitVariable.data());
    TString effDistributionTitle = Form("ToyMC: %s Efficiency for %s", tauId.data(), fitVariable.data());
    TH1* effDistribution = 
      new TH1D(effDistributionName.Data(), 
	       effDistributionTitle.Data(), 101, -0.005, +1.005);

    TString fitConvergenceDistributionName  = Form("fitConvergenceDistribution_%s_%s", tauId.data(), fitVariable.data());
    TString fitConvergenceDistributionTitle = Form("ToyMC: %s Efficiency for %s", tauId.data(), fitVariable.data());
    TH1* fitConvergenceDistribution = 
      new TH1D(fitConvergenceDistributionName.Data(), 
	       fitConvergenceDistributionTitle.Data(), 2, -0.005, +1.005);
    TAxis* xAxis = fitConvergenceDistribution->GetXaxis();
    xAxis->SetBinLabel(1, "Failure");
    xAxis->SetBinLabel(2, "Success");

    histogramMap4 histograms_fluctuated; // key = (process, region, observable, central value/systematic uncertainty) 

    for ( unsigned i = 0; i < numPseudoExperiments; ++i ) {
      for ( vstring::const_iterator process = processes.begin();
	    process != processes.end(); ++process ) {
	for ( vstring::const_iterator region = regionsToFit.begin();
	      region != regionsToFit.end(); ++region ) {
	  for ( vstring::const_iterator observable = observables.begin();  
		observable != observables.end(); ++observable ) {
	    TH1* origHistogram = histograms_mc[*process][*region][*observable][key_central_value];
	    
	    TH1* fluctHistogram = histograms_fluctuated[*process][*region][*observable][key_central_value];
	    if ( !fluctHistogram ) {
	      fluctHistogram = (TH1*)origHistogram->Clone(TString(origHistogram->GetName()).Append("_fluctuated"));
	      histograms_fluctuated[*process][*region][*observable][key_central_value] = fluctHistogram;
	    }
	      
	    sampleHistogram_stat(origHistogram, fluctHistogram);
	      
	    for ( vstring::const_iterator sysUncertainty = sysUncertainties.begin();
		  sysUncertainty != sysUncertainties.end(); ++sysUncertainty ) {
	      std::string key_systematic = std::string(*sysUncertainty).append("Diff");
	      TH1* sysHistogram = histograms_mc[*process][*region][*observable][key_systematic];
	      assert(sysHistogram);
		
	      sampleHistogram_sys(fluctHistogram, sysHistogram, 1.0/sysVariedByNsigma, -1.0, +1.0, kCoherent);
	    }

	    processEntries[*process]->histograms_[*region][*observable][key_central_value] = fluctHistogram;
	  }
	}
      }
    
      double effValue_i = 0.;
      double effError_i = 1.;
      bool hasFitConverged_i = false;
      fitUsingRooFit(*data, intLumiData, processEntries, sysVariedByNsigma, processName_signal, 
		     tauId, effValue_i, effError_i, hasFitConverged_i, 0);

      effDistribution->Fill(effValue_i);
      fitConvergenceDistribution->Fill(hasFitConverged_i);
    }

    std::string outputFileName = controlPlotFilePath;
    if ( outputFileName.find_last_of("/") != (outputFileName.length() - 1) ) outputFileName.append("/");
    outputFileName.append("runPseudoExperiments_");
    savePseudoExperimentHistograms(effDistribution, "#varepsilon", outputFileName);
    savePseudoExperimentHistograms(fitConvergenceDistribution, "Fit status", outputFileName);
  }

  delete histogramInputFile;

//-- save fit results
  fwlite::OutputFiles outputFile(cfg);
  fwlite::TFileService fs = fwlite::TFileService(outputFile.file().data());

  TFileDirectory fitResultOutputDirectory = ( directory != "" ) ? 
    fs.mkdir(directory.data()) : fs;
  
  saveValueAsHistogram(fitResultOutputDirectory, 
		       effValue, effError, 
		       "fitResult_%s_%s", 
		       "%s Efficiency, obtained by fitting %s", 
		       fitVariable, tauId);
  saveValueAsHistogram(fitResultOutputDirectory, 
		       processEntries[processName_signal]->normFactors_[region_passed]->getVal() 
		      + processEntries[processName_signal]->normFactors_[region_failed]->getVal(), 0.,
		       "fitNorm_%s_%s", 
		       "Fitted Number of Z #rightarrow #tau^{+} #tau^{-} Events in 'passed' + 'failed' regions", 
		       fitVariable, tauId);
  saveValueAsHistogram(fitResultOutputDirectory, 
		       processEntries[processName_signal]->fitParameters_["pTauId_passed_failed"].expectedValue_, 0.,
		       "expResult_%s_%s", 
		       "Expected %s Efficiency",
		       fitVariable, tauId);
  saveValueAsHistogram(fitResultOutputDirectory, 
		       processEntries[processName_signal]->numEvents_[region_passed][key_central_value] 
		      + processEntries[processName_signal]->numEvents_[region_failed][key_central_value], 0.,
		       "expNorm_%s_%s", 
		       "Expected Number of Z #rightarrow #tau^{+} #tau^{-} Events in 'passed' + 'failed' regions",
		       fitVariable, tauId);
 
//--print time that it took macro to run
  std::cout << "finished executing fitTauIdEff macro:" << std::endl;
  std::cout << " tauId  = " << tauId << std::endl;
  std::cout << " fitVariable = " << fitVariable << std::endl;
  if ( runPseudoExperiments ) {
    std::cout << " #sysUncertainties    = " << sysUncertainties.size() 
	      << " (numPseudoExperiments = " << numPseudoExperiments << ")" << std::endl;
  }
  clock.Show("fitTauIdEff");

  return 0;
}
