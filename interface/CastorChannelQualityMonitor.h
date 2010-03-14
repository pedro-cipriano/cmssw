#ifndef DQM_CASTORMONITOR_CASTORCHANNELQUALITYMONITOR_H
#define DQM_CASTORMONITOR_CASTORCHANNELQUALITYMONITOR_H

#include "DQM/CastorMonitor/interface/CastorBaseMonitor.h"
#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/MonitorElement.h"

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "DataFormats/HcalDigi/interface/CastorDataFrame.h"
#include "DataFormats/HcalRecHit/interface/HcalRecHitCollections.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <map>


class CastorChannelQualityMonitor: public CastorBaseMonitor{

  typedef std::map<uint32_t,float,std::less<uint32_t> >  MapType;

public:
  CastorChannelQualityMonitor();
  ~CastorChannelQualityMonitor();

  void setup(const edm::ParameterSet& ps, DQMStore* dbe);
  void processEvent(const CastorRecHitCollection& castorHits);
  void reset();


  struct{
    int module;
    int sector;
    vector<float> energy;
  } ChannelStatus;


private:

 int ievt_;
 int module ; int sector; double energy ;

 std::string histo;

 double nThreshold_;
 double dThreshold_;
 int aboveNoisyThreshold [14][16];
 int     aboveDThreshold [14][16];

 bool offline_;
 MonitorElement* reportSummary;
 MonitorElement* reportSummaryMap;  TH2F* h_reportSummaryMap;
 MonitorElement* overallStatus;



};

#endif
