/**
 * \class L1TStage2uGTTimingUnprefireable
 *
 * Description: DQM for L1 Micro Global Trigger timing.
 *
 */

#include "DQM/L1TMonitor/interface/L1TStage2uGTTimingUnprefireable.h"

bool IsUnprefirableEvent(const edm::Event&, const edm::EDGetTokenT<TCDSRecord>&);

// Constructor
L1TStage2uGTTimingUnprefireable::L1TStage2uGTTimingUnprefireable(const edm::ParameterSet& params):
  l1tStage2uGtSource_(consumes<GlobalAlgBlkBxCollection>(params.getParameter<edm::InputTag>("l1tStage2uGtSource"))),
  tcdsRecordToken_(consumes<TCDSRecord>(params.getParameter<edm::InputTag>("tcdsRecordLabel"))),
  monitorDir_(params.getUntrackedParameter<std::string> ("monitorDir", "")),
  verbose_(params.getUntrackedParameter<bool>("verbose", false)),
  gtUtil_(new l1t::L1TGlobalUtil(params, consumesCollector(), *this, params.getParameter<edm::InputTag>("l1tStage2uGtSource"), params.getParameter<edm::InputTag>("l1tStage2uGtSource"))),
  numAlgs_(0),
  unprescaledAlgoShortList_(params.getUntrackedParameter<std::vector<std::string>> ("unprescaledAlgoShortList")),
  prescaledAlgoShortList_(params.getUntrackedParameter<std::vector<std::string>> ("prescaledAlgoShortList"))
{
}

// Destructor
L1TStage2uGTTimingUnprefireable::~L1TStage2uGTTimingUnprefireable() {}

void L1TStage2uGTTimingUnprefireable::dqmBeginRun(edm::Run const& iRun, edm::EventSetup const& evtSetup) {
  // Get the trigger menu information
  gtUtil_->retrieveL1Setup(evtSetup);

  // Find the number of algos defined
  numAlgs_ = static_cast<int>(gtUtil_->decisionsInitial().size());

  int algoBitUnpre_=-1; 
  for(unsigned int i=0;i<unprescaledAlgoShortList_.size();i++){
    if (gtUtil_->getAlgBitFromName(unprescaledAlgoShortList_.at(i), algoBitUnpre_)) {
       unprescaledAlgoBitName_.emplace_back(unprescaledAlgoShortList_.at(i), algoBitUnpre_);
    }
    else {
      edm::LogWarning("L1TStage2uGTTimingUnprefireable") << "Algo \"" << unprescaledAlgoShortList_.at(i) << "\" not found in the trigger menu " << gtUtil_->gtTriggerMenuName() << ". Could not retrieve algo bit number.";
    }
  }

  int algoBitPre_=-1; 
  for(unsigned int i=0;i<prescaledAlgoShortList_.size();i++){
    if ((gtUtil_->getAlgBitFromName(prescaledAlgoShortList_.at(i), algoBitPre_))) {
       prescaledAlgoBitName_.emplace_back(prescaledAlgoShortList_.at(i), algoBitPre_);
    }
    else {
      edm::LogWarning("L1TStage2uGTTimingUnprefireable") << "Algo \"" << prescaledAlgoShortList_.at(i) << "\" not found in the trigger menu " << gtUtil_->gtTriggerMenuName() << ". Could not retrieve algo bit number.";
    }
  }
}

void L1TStage2uGTTimingUnprefireable::bookHistograms(DQMStore::IBooker &ibooker, edm::Run const&, edm::EventSetup const& evtSetup) {

  ibooker.setCurrentFolder(monitorDir_);

  size_t n_selected_algs = numAlgs_;
  std::string title = "All Algorithms vs. BX Number in Event for Unprefirable Events;Bunch Crossing Number In Event;Algorithm Trigger Bits";
  all_algos_vs_bx[true] = ibooker.book2D("all_algos_vs_bx_unpref", title , 5, -2.5, 2.5, n_selected_algs, -0.5, n_selected_algs - 0.5);
  title = "All Algorithms vs. BX Number in Event for Prefirable Events;Bunch Crossing Number In Event;Algorithm Trigger Bits";
  all_algos_vs_bx[false] = ibooker.book2D("all_algos_vs_bx_pref", title , 5, -2.5, 2.5, n_selected_algs, -0.5, n_selected_algs - 0.5);

  n_selected_algs = prescaledAlgoBitName_.size();
  title = "uGT: Prescaled Algorithm Trigger Bits vs. BX Number In Event for Unprefirable Events;Bunch Crossing Number In Event;Algorithm Trigger Names + Bits";
  prescaled_algos_vs_bx[true] = ibooker.book2D("prescaled_algos_vs_bx_unpref", title, 5, -2.5, 2.5, n_selected_algs, -0.5, n_selected_algs-0.5);
  title = "uGT: Prescaled Algorithm Trigger Bits vs. BX Number In Event for Prefirable Events;Bunch Crossing Number In Event;Algorithm Trigger Names + Bits";
  prescaled_algos_vs_bx[false] = ibooker.book2D("prescaled_algos_vs_bx_pref", title, 5, -2.5, 2.5, n_selected_algs, -0.5, n_selected_algs-0.5);
  for(unsigned int algo=0; algo<n_selected_algs; ++algo) {
    const auto bit_name_pair = prescaledAlgoBitName_.at(algo);
    std::string name = bit_name_pair.first+" ("+std::to_string(bit_name_pair.second)+")";
    prescaled_algos_vs_bx[true]->setBinLabel(algo+1, name, 2);
    prescaled_algos_vs_bx[false]  ->setBinLabel(algo+1, name, 2);
  }

  n_selected_algs = unprescaledAlgoBitName_.size();
  title = "uGT: Unprescaled Algorithm Trigger Bits vs. BX Number In Event for Unprefirable Events;Bunch Crossing Number In Event;Algorithm Trigger Names + Bits";
  unprescaled_algos_vs_bx[true] = ibooker.book2D("unprescaled_algos_vs_bx_unpref", title, 5, -2.5, 2.5, n_selected_algs, -0.5, n_selected_algs-0.5);
  title = "uGT: Unprescaled Algorithm Trigger Bits vs. BX Number In Event for Prefirable Events;Bunch Crossing Number In Event;Algorithm Trigger Names + Bits";
  unprescaled_algos_vs_bx[false] = ibooker.book2D("unprescaled_algos_vs_bx_pref", title, 5, -2.5, 2.5, n_selected_algs, -0.5, n_selected_algs-0.5);
  for(unsigned int algo=0; algo<n_selected_algs; ++algo) {
    const auto bit_name_pair = unprescaledAlgoBitName_.at(algo);
    std::string name = bit_name_pair.first+" ("+std::to_string(bit_name_pair.second)+")";
    unprescaled_algos_vs_bx[true]->setBinLabel(algo+1, name, 2);
    unprescaled_algos_vs_bx[false]->setBinLabel(algo+1, name, 2);
  }

}

void L1TStage2uGTTimingUnprefireable::analyze(const edm::Event& evt, const edm::EventSetup& evtSetup) {
  if (verbose_) {
    edm::LogInfo("L1TStage2uGTTimingUnprefireable") << "L1TStage2uGTTimingUnprefireable DQM: Analyzing.." << std::endl;
  }

  bool is_unprefireable = IsUnprefirableEvent(evt, tcdsRecordToken_);
  
  // Open uGT readout record
  edm::Handle<GlobalAlgBlkBxCollection> uGtAlgs;
  evt.getByToken(l1tStage2uGtSource_, uGtAlgs);
  
  if (!uGtAlgs.isValid()) {
    edm::LogInfo("L1TStage2uGTTimingUnprefireable") << "Cannot find uGT readout record.";
    return;
  }

  for (int ibx = uGtAlgs->getFirstBX(); ibx != uGtAlgs->getLastBX(); ++ibx) {
    for (auto algoBlk = uGtAlgs->begin(ibx); algoBlk != uGtAlgs->end(ibx); ++algoBlk) {
      const auto algoInitial = algoBlk->getAlgoDecisionInitial(); 
      for (size_t algo = 0; algo < algoInitial.size(); ++algo) { 
        if (algoInitial.at(algo)) {
          all_algos_vs_bx[is_unprefireable]->Fill(ibx, algo);
        }
      }
      for (unsigned int algo = 0; algo < unprescaledAlgoBitName_.size(); ++algo) {
        if (algoBlk->getAlgoDecisionInitial(unprescaledAlgoBitName_.at(algo).second)) {
          unprescaled_algos_vs_bx[is_unprefireable]->Fill(ibx, algo);
        }
      }
      for (unsigned int algo = 0; algo < prescaledAlgoBitName_.size(); ++algo) {
        if (algoBlk->getAlgoDecisionInitial(prescaledAlgoBitName_.at(algo).second)) {
          prescaled_algos_vs_bx[is_unprefireable]->Fill(ibx, algo);
        }
      }
    }
  }

}


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
