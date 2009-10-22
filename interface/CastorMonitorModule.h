#ifndef CastorMonitorModule_H
#define CastorMonitorModule_H

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/MonitorElement.h"
#include "Geometry/Records/interface/IdealGeometryRecord.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "FWCore/Utilities/interface/CPUTimer.h"
#include "DataFormats/Provenance/interface/EventID.h"  

#include "DataFormats/L1GlobalTrigger/interface/L1GlobalTriggerReadoutSetup.h"
#include "DataFormats/L1GlobalTrigger/interface/L1GlobalTriggerReadoutRecord.h"
#include "DataFormats/L1GlobalTrigger/interface/L1GlobalTriggerReadoutSetupFwd.h"

#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"
#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/HcalDigi/interface/HcalUnpackerReport.h" //-- no CastorUnpackerReport at the moment !
#include "DataFormats/HcalDetId/interface/HcalCastorDetId.h" //-- HcalCastorDetId
#include "DQM/CastorMonitor/interface/CastorMonitorSelector.h"
#include "DQM/CastorMonitor/interface/CastorPedestalMonitor.h"
#include "DQM/CastorMonitor/interface/CastorRecHitMonitor.h"
#include "DQM/CastorMonitor/interface/CastorRecHitsValidation.h"
#include "DQM/CastorMonitor/interface/CastorLEDMonitor.h"

#include "CalibCalorimetry/CastorCalib/interface/CastorDbASCIIIO.h" //-- use to get/dump Calib to DB 
#include "CondFormats/CastorObjects/interface/CastorChannelQuality.h" //-- use to get/hold channel status
#include "CondFormats/DataRecord/interface/CastorChannelQualityRcd.h"

//// #include "CondFormats/HcalObjects/interface/HcalCondObjectContainer.h" //-- 

#include <memory>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sys/time.h>

using namespace std;
using namespace edm;

class CastorMonitorModule : public EDAnalyzer{

public:
  
  ////---- constructor
  CastorMonitorModule(const edm::ParameterSet& ps);

  ////---- destructor
  ~CastorMonitorModule();
  
 protected:
  
  ////---- analyze
  void analyze(const edm::Event& e, const edm::EventSetup& c);
  
  ////---- beginJob
  void beginJob(const edm::EventSetup& c);
  
  ////---- beginRun
  void beginRun(const edm::Run& run, const edm::EventSetup& c);

  ////---- begin LumiBlock
  void beginLuminosityBlock(const edm::LuminosityBlock& lumiSeg, 
                            const edm::EventSetup& c) ;

  ////---- end LumiBlock
  void endLuminosityBlock(const edm::LuminosityBlock& lumiSeg, 
                          const edm::EventSetup& c);

  ////---- endJob
  void endJob(void);
  
  ////---- endRun
  void endRun(const edm::Run& run, const edm::EventSetup& c);

  ////---- reset
  void reset(void);

  ////---- boolean prescale test for event
  bool prescale();

  ////---- check whether Castor has FED data
  void CheckCastorStatus     (const FEDRawDataCollection& rawraw, 
			      const HcalUnpackerReport& report, 
			      const CastorElectronicsMap& emap,
			      const CastorDigiCollection& castordigi
			      );
    
 private:
 
  ////----
  ////---- steerable variables that can be specified in the configuration 
  ////---- input file for the process.       
  ////----
  ////---- prescale variables for restricting the frequency of analyzer
  ////---- behavior.  The base class does not implement prescales.
  ////---- set to -1 to be ignored.
  int prescaleEvt_;    //-- units of events
  int prescaleLS_;     //-- units of lumi sections
  int prescaleTime_;   //-- units of minutes
  int prescaleUpdate_; //-- units of "updates", TBD

  ////---- name of the monitoring process which derives from this
  ////---- class, used to standardize filename and file structure
  std::string monitorName_;

  ////---- verbosity switch used for debugging or informational output
  int fVerbosity;  

  ////---- counters and flags
  int nevt_;
  int nlumisecs_;
  bool saved_;

     


  ////---- control whether or not to display time used by each module
  bool showTiming_; 
  edm::CPUTimer cpu_timer; 

  ////---- psTime
  struct{
    timeval startTV,updateTV;
    double elapsedTime; 
    double vetoTime; 
    double updateTime;
  } psTime_;    


  ////---- define the DQMStore 
  DQMStore* dbe_;  
  
  ////---- define environment variables
  int irun_,ilumisec_,ievent_,itime_;
  bool actonLS_ ;
  std::string rootFolder_;

  int ievt_;
  int ievt_pre_; //-- copy of counter used for prescale purposes
  bool fedsListed_;
  
  //edm::InputTag inputLabelGT_;
  edm::InputTag inputLabelDigi_;
  edm::InputTag inputLabelRecHitCASTOR_;
  //edm::InputTag inputLabelCaloTower_;
  //edm::InputTag inputLabelLaser_;

  ////---- Maps of readout hardware unit to calorimeter channel
  std::map<uint32_t, std::vector<HcalCastorDetId> > DCCtoCell;
  std::map<uint32_t, std::vector<HcalCastorDetId> > ::iterator thisDCC;
  std::map<pair <int,int> , std::vector<HcalCastorDetId> > HTRtoCell;
  std::map<pair <int,int> , std::vector<HcalCastorDetId> > ::iterator thisHTR;

  ////---- define ME used to display the DQM Job Status
  MonitorElement* meFEDS_;
  MonitorElement* meStatus_;
  MonitorElement* meRunType_;
  MonitorElement* meEvtMask_;
  MonitorElement* meTrigger_;
  MonitorElement* meLatency_;
  MonitorElement* meQuality_;
  
  ////---- define monitors
  CastorMonitorSelector*    evtSel_;
  CastorRecHitMonitor*      RecHitMon_;
  CastorRecHitsValidation*  RecHitMonValid_;
  CastorPedestalMonitor*    PedMon_;
  CastorLEDMonitor*         LedMon_;
  
  edm::ESHandle<CastorDbService> conditions_;
  const CastorElectronicsMap*    readoutMap_;

  ofstream m_logFile;

  ////---- decide whether the Castor status should be checked
  bool checkCASTOR_;

  ////----- define this ME to check whether the Castor is present 
  ////----- in the run (using FED info)  
  ////----- 1 is present , 0 - no Digis , -1 no within FED 
  MonitorElement* meCASTOR_;


  /////---- myquality_ will store status values for each det ID I find
  bool dump2database_;
  std::map<HcalCastorDetId, unsigned int> myquality_;
  CastorChannelQuality* chanquality_;
};

#endif
