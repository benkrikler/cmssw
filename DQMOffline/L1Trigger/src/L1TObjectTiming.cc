#include "DQMOffline/L1Trigger/interface/L1TObjectTiming.h"
#include "DQMOffline/L1Trigger/interface/L1TFillWithinLimits.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"                           
#include "FWCore/Framework/interface/MakerMacros.h"

using dqmoffline::l1t::fillWithinLimits;
using dqmoffline::l1t::fill2DWithinLimits;

L1TObjectTiming::L1TObjectTiming(const edm::ParameterSet& ps):
  thePFJetCollection_(consumes<reco::PFJetCollection>(ps.getParameter<edm::InputTag>("pfJetCollection"))),
  theL1TJetCollection_(consumes<l1t::JetBxCollection>(ps.getParameter<edm::InputTag>("L1TCaloLayer2JetCollection"))),
  tcdsRecordToken_(consumes<TCDSRecord>(ps.getParameter<edm::InputTag>("tcdsRecordLabel"))),
  histFolder_(ps.getParameter<std::string>("histFolder"))
{
}

L1TObjectTiming::~L1TObjectTiming(){
  edm::LogInfo("L1TObjectTiming") << "Destructor L1TObjectTiming::~L1TObjectTiming " << std::endl;
}

void L1TObjectTiming::dqmBeginRun(edm::Run const&, edm::EventSetup const&){
  edm::LogInfo("L1TObjectTiming") << "L1TObjectTiming::beginRun" << std::endl;
}

void L1TObjectTiming::bookHistograms(DQMStore::IBooker& ibooker, edm::Run const&, edm::EventSetup const&){
  edm::LogInfo("L1TObjectTiming") << "L1TObjectTiming::bookHistograms" << std::endl;

  ibooker.cd();
  ibooker.setCurrentFolder(histFolder_);

  h_prefirable_ = 
      ibooker.book1D("Prefirable", "Was the event prefirable; Was event prefirable; No. of events", 2, -0.5, 1.5);

  plots[false].bookHistograms(ibooker, false);
  plots[true].bookHistograms(ibooker, true);
}
void L1TObjectTiming::PlotCollect::bookHistograms(DQMStore::IBooker& ibooker, bool unprefirable){
  std::string prefire_title = unprefirable? "Unprefirable": "Prefirable";
  h_nMatched_ = 
      ibooker.book1D("BXOfMatchedJets_" + prefire_title,
                     "Relative bunch crossing for matched L1 jets in " + prefire_title +
                     " events; Relative Bunch Crossing; No. of events", 5, -2.5, 2.5);

  for (int i=0; i<kNumBx; ++i){
      int bx = i - kCentralBX;
      std::string bx_name = "_";
      if (bx > 0) { bx_name += "p";}
      else if (bx < 0){ bx_name += "m"; }
      else{ bx_name += "O"; }
      bx_name += to_string(abs(bx));

      std::string title = "E_{T} vs #eta for PFJets matched to L1Jets in " + prefire_title;
      title += " Events for BX=" + to_string(bx);
      title += "; PF Jet #eta; PF Jet E_{T} (GeV)";
      h_pt_eta_[i]=ibooker.book2D("PFJetEtVsEta_" + prefire_title + bx_name,
                                  title, 50, -5, 5, 35, 0, 700);

      title = "#phi vs #eta for PFJets matched to L1Jet in " + prefire_title;
      title += " Events for BX=" + to_string(bx);
      title += "; PF Jet #eta; PF Jet #phi";
      h_phi_eta_[i]=ibooker.book2D("PFJetPhiVsEta_" + prefire_title + bx_name,
                                   title, 50, -5, 5, 35, -3.2, 3.2);
  }
}

void L1TObjectTiming::analyze(edm::Event const& e, edm::EventSetup const& eSetup){
  edm::LogInfo("L1TObjectTiming") << "L1TObjectTiming::analyze" << std::endl;

  bool is_unprefireable = IsUnprefirableEvent(e, tcdsRecordToken_);
  fillWithinLimits(h_prefirable_, is_unprefireable);

  // Get objects from event
  edm::Handle<l1t::JetBxCollection> l1Jets;
  e.getByToken(theL1TJetCollection_, l1Jets);

  edm::Handle<reco::PFJetCollection> pfJets;
  e.getByToken(thePFJetCollection_, pfJets);

  if (!pfJets.isValid()) {
    edm::LogWarning("L1TObjectTiming") << "invalid collection: PF jets " << std::endl;
    return;
  }
  if (!l1Jets.isValid()) {
    edm::LogWarning("L1TObjectTiming") << "invalid collection: L1 jets " << std::endl;
    return;
  }

  if (pfJets->empty()) {
    LogDebug("L1TObjectTiming") << "no PF jets found" << std::endl;
    return;
  }

  for (auto pfjet = pfJets->begin(); pfjet != pfJets->end(); ++pfjet){
    double minDeltaR = 0.3;
    l1t::JetBxCollection::const_iterator matchedL1Jet;
    bool foundMatch = false;
    int bxWithMatch = -999;
    for (int bunchCrossing = l1Jets->getFirstBX(); bunchCrossing <= l1Jets->getLastBX(); ++bunchCrossing) {

      for (auto l1jet = l1Jets->begin(bunchCrossing); l1jet != l1Jets->end(bunchCrossing); ++l1jet) {
        const double currentDeltaR = deltaR(l1jet->eta(), l1jet->phi(), pfjet->eta(), pfjet->phi());

        if (currentDeltaR < minDeltaR) {
          minDeltaR = currentDeltaR;
          foundMatch = true;
          matchedL1Jet = l1jet;
          bxWithMatch = bunchCrossing;
          break;
        }
      }
    }

    if (foundMatch){
      plots[is_unprefireable].fillWithMatchedJetPair(*matchedL1Jet, *pfjet, bxWithMatch);
    }
  }

}

void L1TObjectTiming::PlotCollect::fillWithMatchedJetPair(const l1t::Jet& l1jet, const reco::PFJet& pfjet, int l1tBX){

  // Plot number in each BX
  fillWithinLimits(h_nMatched_, l1tBX);

  // Plot 2D: Pt vs eta
  fill2DWithinLimits(h_pt_eta_[l1tBX + kCentralBX], pfjet.eta(), pfjet.pt());

  // Plot 2D: phi vs eta
  fill2DWithinLimits(h_phi_eta_[l1tBX + kCentralBX], pfjet.eta(), pfjet.phi());
}

//define this as a plug-in
DEFINE_FWK_MODULE(L1TObjectTiming);

/// This code is shamelessly ripped from:
/// EventFilter/L1TRawToDigi/plugins/TriggerRulePrefireVetoFilter.cc
/// which ideally will become an "include"able bit of C++ code when this work is finally merged
bool
L1TObjectTiming::IsUnprefirableEvent(const edm::Event& iEvent, const edm::EDGetTokenT<TCDSRecord>& tcdsRecordToken_)
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
