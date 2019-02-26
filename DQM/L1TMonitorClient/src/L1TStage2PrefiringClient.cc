#include "DQM/L1TMonitorClient/interface/L1TStage2PrefiringClient.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include <iostream>
using std::cout;
using std::endl;

L1TStage2PrefiringClient::DoubleRatio::DoubleRatio(std::string monitorDir, std::string inputNum, std::string inputDen, 
                                                               std::string name, std::string title):
    monitorDir_(monitorDir), inputNum_(inputNum), inputDen_(inputDen),
    ratioName_("ratio_" + name), ratioTitle_("Ratio of L1A in Unprefirable Events to Prefirable Events for" + title),
    doubleRatioName_("relative_ratio_" + name), doubleRatioTitle_("Fraction of L1A for Unprefirable Events compared to Prefirable Events for" + title),
    ratioME_(nullptr), doubleRatioME_(nullptr){
}

L1TStage2PrefiringClient::DoubleRatio::~DoubleRatio(){}

void L1TStage2PrefiringClient::DoubleRatio::book(DQMStore::IBooker& ibooker, DQMStore::IGetter& igetter)
{
  // Book when called the first time. Otherwise reset the ratio histogram.
  if (ratioME_ == nullptr) {
    ibooker.setCurrentFolder(monitorDir_);

    // get the axis range from the numerator histogram
    const MonitorElement* numME_ = igetter.get(inputNum_);
    if (numME_) {
      TH2F *hNum = numME_->getTH2F();

      ratioME_ = ibooker.book2D(ratioName_, ratioTitle_, 
          hNum->GetNbinsX(), hNum->GetXaxis()->GetXmin(), hNum->GetXaxis()->GetXmax(),
          hNum->GetNbinsY(), hNum->GetYaxis()->GetXmin(), hNum->GetYaxis()->GetXmax());
      ratioME_->setEfficiencyFlag();
      ratioME_->setAxisTitle("Relative Bunch Crossing ", 1);
      ratioME_->setAxisTitle("Algorithm", 2);
      ratioME_->setAxisTitle("Ratio of L1A, Unprefirable / Prefirable events", 3);

      doubleRatioME_ = ibooker.book2D(doubleRatioName_, doubleRatioTitle_, 
          hNum->GetNbinsX(), hNum->GetXaxis()->GetXmin(), hNum->GetXaxis()->GetXmax(),
          hNum->GetNbinsY(), hNum->GetYaxis()->GetXmin(), hNum->GetYaxis()->GetXmax());
      doubleRatioME_->setEfficiencyFlag();
      doubleRatioME_->setAxisTitle("Relative Bunch Crossing ", 1);
      doubleRatioME_->setAxisTitle("Algorithm", 2);
      doubleRatioME_->setAxisTitle("Ratio of L1A, (Unprefirable / Prefirable) normalised to BX = 0", 3);
    }
  } else {
    ratioME_->Reset();
  }
}

void L1TStage2PrefiringClient::DoubleRatio::FirstRatio(DQMStore::IGetter& igetter)
{
  const MonitorElement* numME_ = igetter.get(inputNum_);
  const MonitorElement* denME_ = igetter.get(inputDen_);

  if (!numME_ || !denME_) {
    return;
  }

  TH2F *hNum = numME_->getTH2F();
  TH2F *hDen = dynamic_cast<TH2F*>(denME_->getTH2F()->Clone("den"));

  TH2F *hRatio = ratioME_->getTH2F();
  TH2F *hDoubleRatio = doubleRatioME_->getTH2F();

  // Set the axis labels the same as the numerator histogram to be able to divide
  if (hNum->GetXaxis()->IsAlphanumeric()) {
    for (int i = 1; i <= hNum->GetNbinsX(); ++i) {
      hDen->GetXaxis()->SetBinLabel(i, hNum->GetXaxis()->GetBinLabel(i));
      hRatio->GetXaxis()->SetBinLabel(i, hNum->GetXaxis()->GetBinLabel(i));
      hDoubleRatio->GetXaxis()->SetBinLabel(i, hNum->GetXaxis()->GetBinLabel(i));
    }
  }
  if (hNum->GetYaxis()->IsAlphanumeric()) {
    for (int i = 1; i <= hNum->GetNbinsY(); ++i) {
      hDen->GetYaxis()->SetBinLabel(i, hNum->GetYaxis()->GetBinLabel(i));
      hRatio->GetYaxis()->SetBinLabel(i, hNum->GetYaxis()->GetBinLabel(i));
      hDoubleRatio->GetYaxis()->SetBinLabel(i, hNum->GetYaxis()->GetBinLabel(i));
    }
  }

  hRatio->Divide(hNum, hDen, 1, 1, "B");

  delete hDen;
}

void L1TStage2PrefiringClient::DoubleRatio::NormToBX0(){
  if (!ratioME_) {
    return;
  }

  TH2F *hRatio = ratioME_->getTH2F();
  TH2F *hDoubleRatio = doubleRatioME_->getTH2F();
  const int bx_zero_bin = hRatio->GetXaxis()->FindFixBin(0.);
  for (int iy = 1; iy <= hRatio->GetNbinsY(); ++iy) {
    const float norm = hRatio->GetBinContent(bx_zero_bin, iy);
    for (int ix = 1; ix <= hRatio->GetNbinsX(); ++ix) {
      const float content = hRatio->GetBinContent(ix, iy);
      if (norm){
        hDoubleRatio->SetBinContent(ix, iy, content / norm);
      }else{
        hDoubleRatio->SetBinContent(ix, iy, 0);
      }
    }
  }
}

L1TStage2PrefiringClient::L1TStage2PrefiringClient(const edm::ParameterSet& ps):
  monitorDir_(ps.getUntrackedParameter<std::string>("monitorDir")),
  inputDir_(ps.getUntrackedParameter<std::string>("inputDir")),
  allAlgsName_(ps.getUntrackedParameter<std::string>("allAlgsHist", "all_algos_vs_bx")),
  prescaleAlgsName_(ps.getUntrackedParameter<std::string>("prescaleAlgsHist", "prescaled_algos_vs_bx")),
  unprescaleAlgsName_(ps.getUntrackedParameter<std::string>("unprescaleAlgsHist", "unprescaled_algos_vs_bx")),
  prefirableSuffix_(ps.getUntrackedParameter<std::string>("prefirableSuffix", "_pref")),
  unprefirableSuffix_(ps.getUntrackedParameter<std::string>("unprefirableSuffix", "_unpref")),
  ratioAll_(monitorDir_, inputDir_+"/"+allAlgsName_+unprefirableSuffix_, inputDir_+"/"+allAlgsName_+prefirableSuffix_, 
      allAlgsName_, "All Algorithms"),
  ratioPrescale_(monitorDir_, inputDir_+"/"+prescaleAlgsName_+unprefirableSuffix_, inputDir_+"/"+prescaleAlgsName_+prefirableSuffix_, 
      prescaleAlgsName_, "Algorithms with Prescaling"),
  ratioUnprescale_(monitorDir_, inputDir_+"/"+unprescaleAlgsName_+unprefirableSuffix_, inputDir_+"/"+unprescaleAlgsName_+prefirableSuffix_, 
      unprescaleAlgsName_, "Algorithms without Prescaling")
{
}

L1TStage2PrefiringClient::~L1TStage2PrefiringClient(){}

void L1TStage2PrefiringClient::fillDescriptions(edm::ConfigurationDescriptions& descriptions)
{
  edm::ParameterSetDescription desc;
  desc.addUntracked<std::string>("monitorDir", "")->setComment("Target directory in the DQM file. Will be created if not existing.");
  desc.addUntracked<std::string>("inputDir", "")->setComment("Path to input histogram directory");
  descriptions.add("l1TStage2PrefiringClient", desc);
}

void L1TStage2PrefiringClient::dqmEndLuminosityBlock(DQMStore::IBooker& ibooker, DQMStore::IGetter& igetter, const edm::LuminosityBlock& lumiSeg, const edm::EventSetup& c)
{
  ratioAll_.run(ibooker, igetter);
  ratioPrescale_.run(ibooker, igetter);
  ratioUnprescale_.run(ibooker, igetter);
}

void L1TStage2PrefiringClient::dqmEndJob(DQMStore::IBooker& ibooker, DQMStore::IGetter& igetter) {
  ratioAll_.run(ibooker, igetter);
  ratioPrescale_.run(ibooker, igetter);
  ratioUnprescale_.run(ibooker, igetter);
}
