#ifndef L1TObjectTiming_H
#define L1TObjectTiming_H

//Framework
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

//event
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"

//DQM
#include "DQMServices/Core/interface/DQMEDAnalyzer.h"
#include "DQMServices/Core/interface/DQMStore.h"

// PFJets
#include "DataFormats/JetReco/interface/PFJet.h"

// L1T objects
#include "DataFormats/L1Trigger/interface/Jet.h"
#include "DataFormats/TCDS/interface/TCDSRecord.h"

class L1TObjectTiming : public DQMEDAnalyzer {
public:
  L1TObjectTiming(const edm::ParameterSet& ps);
  ~L1TObjectTiming() override;

protected:
  void dqmBeginRun(edm::Run const&, edm::EventSetup const&) override;
  void bookHistograms(DQMStore::IBooker&, edm::Run const&, edm::EventSetup const&) override;
  void analyze(edm::Event const& e, edm::EventSetup const& eSetup) override;

private:
  void fillWithMatchedJetPair(const l1t::Jet&, const reco::PFJet&, int l1tBX);
  // TODO: Move functin out of this class and then use this in the original filter plugin
  bool IsUnprefirableEvent(const edm::Event&, const edm::EDGetTokenT<TCDSRecord>&);

  // Config file parameters TO CLEAN
  edm::EDGetTokenT<reco::PFJetCollection> thePFJetCollection_;
  edm::EDGetTokenT<l1t::JetBxCollection> theL1TJetCollection_;
  edm::EDGetTokenT<TCDSRecord> tcdsRecordToken_;
  std::string histFolder_;

  enum {kCentralBX=2, kNumBx=5};
  MonitorElement* h_resolutionJetEta_;
  MonitorElement* h_nMatched_;
  MonitorElement* h_pt_eta_[kNumBx];
  MonitorElement* h_phi_eta_[kNumBx];

};

#endif // L1TObjectTiming_H
