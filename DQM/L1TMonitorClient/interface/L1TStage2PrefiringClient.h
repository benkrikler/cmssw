#ifndef DQM_L1TMonitorClient_L1TStage2PrefiringClient_H
#define DQM_L1TMonitorClient_L1TStage2PrefiringClient_H

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "DQMServices/Core/interface/MonitorElement.h"
#include "DQMServices/Core/interface/DQMEDHarvester.h"

class L1TStage2PrefiringClient: public DQMEDHarvester
{
  public:
    class DoubleRatio{
      public:

        DoubleRatio(std::string monitorDir, std::string inputNum, std::string inputDem, 
                    std::string name, std::string title);
        virtual ~DoubleRatio();

        void run(DQMStore::IBooker& ibooker, DQMStore::IGetter& igetter){
                book(ibooker, igetter);
                FirstRatio(igetter);
                NormToBX0();
            }

      private:
        void book(DQMStore::IBooker& ibooker, DQMStore::IGetter& igetter);
        void FirstRatio(DQMStore::IGetter& igetter);
        void NormToBX0();

      private:
        std::string monitorDir_;
        std::string inputNum_;
        std::string inputDen_;
        std::string ratioName_, ratioTitle_;
	std::string doubleRatioName_, doubleRatioTitle_;

        MonitorElement* ratioME_;
        MonitorElement* doubleRatioME_;
    };

  public:

    L1TStage2PrefiringClient(const edm::ParameterSet&);
    ~L1TStage2PrefiringClient() override;
    static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

  protected:

    void dqmEndJob(DQMStore::IBooker& ibooker, DQMStore::IGetter& igetter)override;
    void dqmEndLuminosityBlock(DQMStore::IBooker& ibooker,DQMStore::IGetter& igetter,const edm::LuminosityBlock& lumiSeg, const edm::EventSetup& c) override;

  private:
    std::string monitorDir_;
    std::string inputDir_;
    std::string allAlgsName_;
    std::string prescaleAlgsName_;
    std::string unprescaleAlgsName_;
    std::string prefirableSuffix_;
    std::string unprefirableSuffix_;

    DoubleRatio ratioAll_;
    DoubleRatio ratioPrescale_;
    DoubleRatio ratioUnprescale_;
};

#endif

