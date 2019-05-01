#include "DQMOffline/L1Trigger/interface/L1TPrefiringDQMOffline.h"
#include "DataFormats/Math/interface/deltaR.h"
#include <sstream>
#include <utility>

bool IsUnprefirableEvent(const edm::Event&, const edm::EDGetTokenT<TCDSRecord>&);

// Constructor
L1TPrefiringDQMOffline::L1TPrefiringDQMOffline(const edm::ParameterSet& params):
  tcdsRecordToken_(consumes<TCDSRecord>(params.getParameter<edm::InputTag>("tcdsRecordLabel"))),
  offlineJetSource_(consumes<reco::PFJetCollection>(params.getParameter<edm::InputTag>("offlineJetSource"))),
  stage2CaloLayer2JetSource_(consumes<l1t::JetBxCollection>(params.getParameter<edm::InputTag>("stage2CaloLayer2JetSource"))),
  monitorDir_(params.getUntrackedParameter<std::string> ("monitorDir", "")),
  verbose_(params.getUntrackedParameter<bool>("verbose", false))
{
}

// Destructor
L1TPrefiringDQMOffline::~L1TPrefiringDQMOffline() {}

void L1TPrefiringDQMOffline::dqmBeginRun(edm::Run const& iRun, edm::EventSetup const& evtSetup) {
}

void L1TPrefiringDQMOffline::bookHistograms(DQMStore::IBooker &ibooker, edm::Run const&, edm::EventSetup const& evtSetup) {
  ibooker.setCurrentFolder(monitorDir_);

  offline_vs_online_njets = ibooker.book2D("offline_vs_online_njets", "nJets in offline vs online", 100, 0, 100, 100, 0, 100);

  for(int ibx = kMinBx; ibx < kMaxBx; ++ibx){
    std::stringstream name;
    name << "offline_pt_eta_bx" <<ibx;
    std::stringstream title;
    title << "Offline Pt and Eta for matched jets in BX = "<<ibx<<" ; Offline Pt (GeV); Offline #eta (rad)";
    offline_pt_eta[ibx] = ibooker.book2D(name.str(), title.str(), 100, 0, 500, 5, -2.5, 2.5);
    title.str("");
    title.flush();
    title << "Online Pt and Eta for matched jets in BX = "<<ibx<<" ; Online Pt (GeV); Online #eta (rad)";
    online_pt_eta[ibx] = ibooker.book2D(name.str(), title.str(), 100, 0, 500, 5, -2.5, 2.5);
  }
}

typedef std::vector<std::pair<const reco::PFJetCollection&, 
                              const l1t::JetBxCollection::value_type&>> MatchedOfflineOnlineJetCollection;

std::pair<l1t::JetBxCollection::const_iterator, int> find_matched_jet(
  const reco::PFJet& offlineJet, const l1t::JetBxCollection& allOnlineJets){
    // find corresponding L1 jet
    double minDeltaR = 0.3;

    auto matchedOnlineJet = allOnlineJets.end();
    int matched_bx = -999;

    for (int bunchCrossing = allOnlineJets.getFirstBX(); bunchCrossing <= allOnlineJets.getLastBX(); ++bunchCrossing) {
        for (auto jet = allOnlineJets.begin(bunchCrossing); jet != allOnlineJets.end(bunchCrossing); ++jet) {
            double currentDeltaR = deltaR(jet->eta(), jet->phi(), offlineJet.eta(), offlineJet.phi());
            if (currentDeltaR >= minDeltaR) {
                continue;
            } else {
                minDeltaR = currentDeltaR;
                matchedOnlineJet = jet;
                matched_bx = bunchCrossing;
                break;
            }
        }
    }
    return std::make_pair(matchedOnlineJet, matched_bx);
}

void L1TPrefiringDQMOffline::analyze(const edm::Event& evt, const edm::EventSetup& evtSetup) {
  if (verbose_) {
    edm::LogInfo("L1TPrefiringDQMOffline") << "L1TPrefiringDQMOffline DQM: Analyzing.." << std::endl;
  }

  bool is_unprefireable = IsUnprefirableEvent(evt, tcdsRecordToken_);
  
  if (not is_unprefireable){
    return;
  }

  // Get the offline jet collection
  edm::Handle<reco::PFJetCollection> offlineJets;
  evt.getByToken(offlineJetSource_, offlineJets);

  if (!offlineJets.isValid()) {
    edm::LogInfo("L1TPrefiringDQMOffline") << "Cannot find offline jet record";
    return;
  }

  // Get the online L1 jet collection
  edm::Handle<l1t::JetBxCollection> onlineJets;
  evt.getByToken(stage2CaloLayer2JetSource_, onlineJets);

  if (!onlineJets.isValid()) {
    edm::LogInfo("L1TPrefiringDQMOffline") << "Cannot find online jet record";
    return;
  }

  offline_vs_online_njets->Fill(onlineJets->size(), offlineJets->size());

  for (const auto jet: *offlineJets){
      auto match_jet_bx = find_matched_jet(jet, *onlineJets);
      auto onlineJet = match_jet_bx.first;
      auto bunchCrossing = match_jet_bx.second;
      if (onlineJet == onlineJets->end()){
          continue;
      }
      offline_pt_eta[bunchCrossing]->Fill(jet.pt(), jet.eta());
      online_pt_eta[bunchCrossing]->Fill(onlineJet->pt(), onlineJet->eta());
  }
}

//define this as a plug-in
DEFINE_FWK_MODULE (L1TPrefiringDQMOffline);

/// This code is shamelessly ripped from:
/// EventFilter/L1TRawToDigi/plugins/TriggerRulePrefireVetoFilter.cc
/// which ideally will become an "include"able bit of C++ code when this work is finally merged
bool
IsUnprefirableEvent(const edm::Event& iEvent, const edm::EDGetTokenT<TCDSRecord>& tcdsRecordToken_)
{
  edm::Handle<TCDSRecord> tcdsRecordH;
  iEvent.getByToken(tcdsRecordToken_, tcdsRecordH);
  const auto& tcdsRecord = *tcdsRecordH.product();

  uint64_t thisEvent = (tcdsRecord.getBXID()-1) + tcdsRecord.getOrbitNr()*3564ull;

  std::vector<uint64_t> eventHistory;
  for (auto&& l1a : tcdsRecord.getFullL1aHistory()) {
    eventHistory.push_back(thisEvent - ((l1a.getBXID()-1) + l1a.getOrbitNr()*3564ull));
  }

  // should be 16 according to TCDSRecord.h, we only care about the last 4
  if ( eventHistory.size() < 4 ) {
    edm::LogError("TriggerRulePrefireVetoFilter") << "Unexpectedly small L1A history from TCDSRecord";
  }

  // No more than 1 L1A in 3 BX
  if ( eventHistory[0] < 3ull ) {
    edm::LogError("TriggerRulePrefireVetoFilter") << "Found an L1A in an impossible location?! (1 in 3)";
  }
  if ( eventHistory[0] == 3ull ) return true;

  // No more than 2 L1As in 25 BX
  if ( eventHistory[0] < 25ull and eventHistory[1] < 25ull ) {
    edm::LogError("TriggerRulePrefireVetoFilter") << "Found an L1A in an impossible location?! (2 in 25)";
  }
  if ( eventHistory[0] < 25ull and eventHistory[1] == 25ull ) return true;

  // No more than 3 L1As in 100 BX
  if ( eventHistory[0] < 100ull and eventHistory[1] < 100ull and eventHistory[2] < 100ull ) {
    edm::LogError("TriggerRulePrefireVetoFilter") << "Found an L1A in an impossible location?! (3 in 100)";
  }
  if ( eventHistory[0] < 100ull and eventHistory[1] < 100ull and eventHistory[2] == 100ull ) return true;

  // No more than 4 L1As in 240 BX
  if ( eventHistory[0] < 240ull and eventHistory[1] < 240ull and eventHistory[2] < 240ull and eventHistory[3] < 240ull ) {
    edm::LogError("TriggerRulePrefireVetoFilter") << "Found an L1A in an impossible location?! (4 in 240)";
  }
  if ( eventHistory[0] < 240ull and eventHistory[1] < 240ull and eventHistory[2] < 240ull and eventHistory[3] == 240ull ) return true;

  return false;
}
