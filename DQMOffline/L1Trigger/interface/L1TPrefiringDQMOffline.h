#ifndef L1TPREFIRINGDQMOFFLINE_H
#define L1TPREFIRINGDQMOFFLINE_H


/**
 * \class L1TPrefiringDQMOffline
 *
 * Description: DQM for L1 Micro Global Trigger timing.
 *
 */

// System include files
#include <memory>
#include <vector>
#include <utility>
// User include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Framework/interface/EventSetup.h"

// L1 trigger include files
#include "DataFormats/L1TGlobal/interface/GlobalAlgBlk.h"
#include "DataFormats/L1Trigger/interface/BXVector.h"
#include "DataFormats/TCDS/interface/TCDSRecord.h"
#include "DataFormats/JetReco/interface/PFJetCollection.h"
#include "DataFormats/L1Trigger/interface/Jet.h"
#include "L1Trigger/L1TGlobal/interface/L1TGlobalUtil.h"

// DQM include files
#include "DQMServices/Core/interface/DQMEDAnalyzer.h"
#include "DQMServices/Core/interface/MonitorElement.h"

//
// Class declaration
//

class L1TPrefiringDQMOffline: public DQMEDAnalyzer {

public:
  L1TPrefiringDQMOffline(const edm::ParameterSet& ps); // constructor
  ~L1TPrefiringDQMOffline() override; // destructor

protected:
  void dqmBeginRun(const edm::Run&, const edm::EventSetup&) override;
  void bookHistograms(DQMStore::IBooker &ibooker, edm::Run const&, edm::EventSetup const&) override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;

private:
   
  // Input parameters
  edm::EDGetTokenT<TCDSRecord> tcdsRecordToken_; // TMP: Needed for marking events as (Un)Prefireable
  edm::EDGetTokenT<reco::PFJetCollection> offlineJetSource_;
  edm::EDGetTokenT<l1t::JetBxCollection> stage2CaloLayer2JetSource_;

  std::string monitorDir_; // histogram folder for L1 uGT plots

  bool verbose_; // verbosity switch

  enum {kNumBx=5, kMinBx=-2, kMaxBx=2};
  MonitorElement* offline_pt_eta[kNumBx];
  MonitorElement* online_pt_eta[kNumBx];
  MonitorElement* offline_vs_online_njets;
};

#endif
