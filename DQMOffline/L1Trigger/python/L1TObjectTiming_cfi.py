import FWCore.ParameterSet.Config as cms
from DQMOffline.L1Trigger.L1TEtSumJetOffline_cfi import goodPFJetsForL1T

from DQMServices.Core.DQMEDAnalyzer import DQMEDAnalyzer
l1tObjectTimingDQM = DQMEDAnalyzer(
    "L1TObjectTiming",
    pfJetCollection=cms.InputTag("goodPFJetsForL1T"),
    L1TCaloLayer2JetCollection=cms.InputTag("caloStage2Digis", "Jet"),
    histFolder=cms.string('L1T/L1TObjects/Timing'),
    tcdsRecordLabel = cms.InputTag("tcdsDigis","tcdsRecord"),
)

l1tObjectTimingDQMEmu = l1tObjectTimingDQM.clone(
    theL1TJetCollection_=cms.InputTag("simCaloStage2Digis"),
    histFolder_=cms.string('L1TEMU/L1TObjects/Timing'),
)

l1tObjectTimingDQMSeq = cms.Sequence(
    cms.ignore(goodPFJetsForL1T)
    + l1tObjectTimingDQM
)

l1tObjectTimingDQMEmuSeq = cms.Sequence(
    cms.ignore(goodPFJetsForL1T)
    + l1tObjectTimingDQMEmu
)
