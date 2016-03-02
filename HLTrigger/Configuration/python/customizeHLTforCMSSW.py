import FWCore.ParameterSet.Config as cms

#
# reusable functions
def producers_by_type(process, *types):
    return (module for module in process._Process__producers.values() if module._TypedParameterizable__type in types)
def filters_by_type(process, *types):
    return (filter for filter in process._Process__filters.values() if filter._TypedParameterizable__type in types)
def analyzers_by_type(process, *types):
    return (analyzer for analyzer in process._Process__analyzers.values() if analyzer._TypedParameterizable__type in types)

def esproducers_by_type(process, *types):
    return (module for module in process._Process__esproducers.values() if module._TypedParameterizable__type in types)

#
# one action function per PR - put the PR number into the name of the function

# example:
# def customiseFor12718(process):
#     for pset in process._Process__psets.values():
#         if hasattr(pset,'ComponentType'):
#             if (pset.ComponentType == 'CkfBaseTrajectoryFilter'):
#                 if not hasattr(pset,'minGoodStripCharge'):
#                     pset.minGoodStripCharge = cms.PSet(refToPSet_ = cms.string('HLTSiStripClusterChargeCutNone'))
#     return process

#
# CMSSW version specific customizations
def customizeHLTforCMSSW(process, menuType="GRun"):

    import os
    cmsswVersion = os.environ['CMSSW_VERSION']

    if cmsswVersion >= "CMSSW_8_0":
#       process = customiseFor12718(process)
        pass

#   stage-2 changes if needed
    if ( (menuType == "Fake") or (menuType == "FULL") ):
        return process

#   set non-default stage-2 L1T menus (as long as o2o is missing)
    if (menuType == "HIon"):
        from HLTrigger.Configuration.CustomConfigs import L1THIon
        process = L1THIon(process)
    elif (menuType == "PRef"):
        from HLTrigger.Configuration.CustomConfigs import L1TPRef
        process = L1TPRef(process)
    else:
        from HLTrigger.Configuration.CustomConfigs import L1TGRun
        process = L1TGRun(process)

#   replace converted l1extra=>l1t plugins which are not yet in ConfDB
    replaceList = {
        'EDAnalyzer' : { },
        'EDFilter'   : {
            'HLTMuonL1Filter' : 'HLTMuonL1TFilter',
            },
        'EDProducer' : {
            'CaloTowerCreatorForTauHLT' : 'CaloTowerFromL1TCreatorForTauHLT',
            'L1HLTTauMatching' : 'L1THLTTauMatching',
            'HLTCaloJetL1MatchProducer' : 'HLTCaloJetL1TMatchProducer',
            'HLTPFJetL1MatchProducer' : 'HLTPFJetL1TMatchProducer',
            }
        }
    print " "
    print "# Replacing plugins: "
    for type,list in replaceList.iteritems():
        if (type=="EDAnalyzer"):
            print "# Replacing EDAnalyzers:"
            for old,new in list.iteritems():
                print '# EDAnalyzer plugin type: ',old,' -> ',new
                for module in analyzers_by_type(process,old):
                    label = module._Labelable__label
                    print '# Instance: ',label
                    setattr(process,label,cms.EDAnalyzer(new,**module.parameters_()))
        elif (type=="EDFilter"):
            print "# Replacing EDFilters  :"
            for old,new in list.iteritems():
                print '# EDFilter plugin type  : ',old,' -> ',new
                for module in filters_by_type(process,old):
                    label = module._Labelable__label
                    print '# Instance: ',label
                    setattr(process,label,cms.EDFilter(new,**module.parameters_()))
        elif (type=="EDProducer"):
            print "# Replacing EDProducers:"
            for old,new in list.iteritems():
                print '# EDProducer plugin type: ',old,' -> ',new
                for module in producers_by_type(process,old):
                    label = module._Labelable__label
                    print '# Instance: ',label
                    setattr(process,label,cms.EDProducer(new,**module.parameters_()))
                    if (new == 'CaloTowerFromL1TCreatorForTauHLT'):
                        setattr(getattr(process,label),'TauTrigger',cms.InputTag('hltCaloStage2Digis'))
                    if ((new == 'HLTCaloJetL1TMatchProducer') or (new == 'HLTPFJetL1TMatchProducer')):
                        setattr(getattr(process,label),'L1Jets',cms.InputTag('hltCaloStage2Digis'))
                        if hasattr(getattr(process,label),'L1CenJets'):
                            delattr(getattr(process,label),'L1CenJets')
                        if hasattr(getattr(process,label),'L1ForJets'):
                            delattr(getattr(process,label),'L1ForJets')
                        if hasattr(getattr(process,label),'L1TauJets'):
                            delattr(getattr(process,label),'L1TauJets')
        else:
            print "# Error - Type ',type,' not recognised!"

#   replace remaining l1extra modules with filter returning 'false'
    badTypes = (
                'HLTEgammaL1MatchFilterRegional',
                'HLTEcalRecHitInAllL1RegionsProducer',
                'EgammaHLTCaloTowerProducer',
                'HLTMuonL1Filter',
                'HLTL1MuonSelector',
                'L2MuonSeedGenerator',
                'CaloTowerCreatorForTauHLT',
                'HLTPFJetL1MatchProducer',
                'IsolatedPixelTrackCandidateProducer',
                'HLTLevel1Activity',
                )
#   Both of the HLTEcalRecHitInAllL1RegionsProducer instances need InputTag fixes
#    for module in producers_by_type(process,'HLTEcalRecHitInAllL1RegionsProducer'):
#        label = module._Labelable__label
#        setattr(getattr(process,label).l1InputRegions[0],'inputColl',cms.InputTag('hltCaloStage2Digis'))
#        setattr(getattr(process,label).l1InputRegions[1],'inputColl',cms.InputTag('hltCaloStage2Digis'))
#        setattr(getattr(process,label).l1InputRegions[2],'inputColl',cms.InputTag('hltCaloStage2Digis'))
#   One of the EgammaHLTCaloTowerProducer instances need InputTag fixes
#    if hasattr(process,'hltRegionalTowerForEgamma'):
#        setattr(getattr(process,'hltRegionalTowerForEgamma'),'L1NonIsoCand',cms.InputTag('hltCaloStage2Digis'))
#        setattr(getattr(process,'hltRegionalTowerForEgamma'),'L1IsoCand',cms.InputTag('hltCaloStage2Digis'))

    print " "
    print "# Unconverted module types: ",badTypes
    badModules = [ ]
    for badType in badTypes:
        print " "
        print '## Unconverted module type: ',badType
        for module in analyzers_by_type(process,badType):
            label = module._Labelable__label
            badModules += [label]
            print '### analyzer label: ',label
        for module in filters_by_type(process,badType):
            label = module._Labelable__label
            badModules += [label]
            print '### filter   label: ',label
        for module in producers_by_type(process,badType):
            label = module._Labelable__label
            badModules += [label]
            print '### producer label: ',label
    print " "
    for label in badModules:
        setattr(process,label,cms.EDFilter("HLTBool",result=cms.bool(False)))

    return process
