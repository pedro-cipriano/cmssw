#include <DQM/CastorMonitor/interface/CastorMonitorModule.h>
#include "DataFormats/FEDRawData/interface/FEDNumbering.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Event.h"

//**************************************************************//
//***************** CastorMonitorModule       ******************//
//***************** Author: Dmytro Volyanskyy ******************//
//***************** Date  : 22.11.2008 (first version) *********// 
//**************************************************************//
////---- simple event filter which directs events to monitoring tasks: 
////---- access unpacked data from each event and pass them to monitoring tasks 
////---- last revision: 06.10.2010 

//==================================================================//
//======================= Constructor ==============================//
//==================================================================//
CastorMonitorModule::CastorMonitorModule(const edm::ParameterSet& ps){
 
   ////---- get steerable variables
  inputLabelRaw_             = ps.getParameter<edm::InputTag>("rawLabel");
  inputLabelReport_          = ps.getParameter<edm::InputTag>("unpackerReportLabel");
  inputLabelDigi_            = ps.getParameter<edm::InputTag>("digiLabel");
  inputLabelRecHitCASTOR_    = ps.getParameter<edm::InputTag>("CastorRecHitLabel"); 
  inputLabelCastorTowers_    = ps.getParameter<edm::InputTag>("CastorTowerLabel"); 
  inputLabelCastorBasicJets_ = ps.getParameter<edm::InputTag>("CastorBasicJetsLabel"); 
  inputLabelCastorJetIDs_    = ps.getParameter<edm::InputTag>("CastorJetIDLabel"); 
  

  fVerbosity = ps.getUntrackedParameter<int>("debug", 0);                        //-- show debug 
  showTiming_ = ps.getUntrackedParameter<bool>("showTiming", false);         //-- show CPU time 
  dump2database_   = ps.getUntrackedParameter<bool>("dump2database",false);  //-- dumps output to database file
  EDMonOn_=  ps.getUntrackedParameter<bool>("EDMonitor", false);

 if(fVerbosity>0) std::cout << "CastorMonitorModule Constructor (start)" << std::endl;

  ////---- initialize Run, LS, Event number and other parameters
  irun_=0; 
  ilumisec_=0; 
  ievent_=0; 
  itime_=0;
  ibunch_=0;
  actonLS_=false;

  meStatus_=0;  meRunType_=0;
  meEvtMask_=0; meFEDS_=0;
  meLatency_=0; meQuality_=0;
  fedsListed_ = false;

  RecHitMon_ = NULL; 
  DataIntMon_ = NULL; 
  DigiMon_ = NULL; 
  LedMon_ = NULL;    
  PSMon_ = NULL;    
  CQMon_ = NULL;
  EDMon_ = NULL;  
  HIMon_ = NULL; 
 
  /////---- set false these here
  rawOK_    = false;
  reportOK_ = false;
  digiOK_   = false;
  rechitOK_ = false;
  towerOK_  = false;
  jetOK_    = false;
  jetIdOK_  = false;

  nRaw=0;
  nDigi=0;
  nRechit=0;
  nTower=0;
  nJet=0;
  nJetId=0;
   


 ////---- get DQMStore service  
  dbe_ = edm::Service<DQMStore>().operator->();

  ////---- initialise CastorMonitorSelector
  evtSel_ = new CastorMonitorSelector(ps);

  //---------------------- DataIntMonitor ----------------------// 
  if ( ps.getUntrackedParameter<bool>("DataIntMonitor", false) ) {
    if(fVerbosity>0) std::cout << "CastorMonitorModule: Data Integrity monitor flag is on...." << std::endl;
    DataIntMon_ = new CastorDataIntegrityMonitor();
    DataIntMon_->setup(ps, dbe_);
  }
 //------------------------------------------------------------//

 //---------------------- DigiMonitor ----------------------// 
  if ( ps.getUntrackedParameter<bool>("DigiMonitor", false) ) {
    if(fVerbosity>0) std::cout << "CastorMonitorModule: Digi monitor flag is on...." << std::endl;
    DigiMon_ = new CastorDigiMonitor();
    DigiMon_->setup(ps, dbe_);
  }
 //------------------------------------------------------------//

 ////-------------------- RecHitMonitor ------------------------// 
  if ( ps.getUntrackedParameter<bool>("RecHitMonitor", false) ) {
    if(fVerbosity>0) std::cout << "CastorMonitorModule: RecHit monitor flag is on...." << std::endl;
    RecHitMon_ = new CastorRecHitMonitor();
    RecHitMon_->setup(ps, dbe_);
  }
 //-------------------------------------------------------------//
 

 ////-------------------- HIMonitor ------------------------// 
  if ( ps.getUntrackedParameter<bool>("HIMonitor", false) ) {
    if(fVerbosity>0) std::cout << "CastorMonitorModule: HI monitor flag is on...." << std::endl;
    HIMon_ = new CastorHIMonitor();
    HIMon_->setup(ps, dbe_);
  }
 //-------------------------------------------------------------//

////-------------------- ChannelQualityMonitor ------------------------// 
  if ( ps.getUntrackedParameter<bool>("ChannelQualityMonitor", false) ) {
    if(fVerbosity>0) std::cout << "CastorChannelQualityMonitor: CQ monitor flag is on...." << std::endl;
    CQMon_ = new CastorChannelQualityMonitor();
    CQMon_->setup(ps, dbe_);
  }
 //-------------------------------------------------------------//
  /* // take it away for the time being 
  ////-------------------- LEDMonitor ------------------------// 
  if ( ps.getUntrackedParameter<bool>("LEDMonitor", false) ) {
    if(fVerbosity>0) std::cout << "CastorMonitorModule: LED monitor flag is on...." << std::endl;
    LedMon_ = new CastorLEDMonitor();
    LedMon_->setup(ps, dbe_);
  }
 //-------------------------------------------------------------//
 */
 //---------------------- PSMonitor ----------------------// 
  if ( ps.getUntrackedParameter<bool>("PSMonitor", false) ) {
    if(fVerbosity>0) std::cout << "CastorMonitorModule: PS monitor flag is on...." << std::endl;
    PSMon_ = new CastorPSMonitor();
    PSMon_->setup(ps, dbe_);
  }
 //------------------------------------------------------------//

 //---------------------- EDMonitor ----------------------// 
  if ( ps.getUntrackedParameter<bool>("EDMonitor", false) ) {
    if(fVerbosity>0) std::cout << "CastorMonitorModule: ED monitor flag is on...." << std::endl;
    EDMon_ = new CastorEventDisplay();
    EDMon_->setup(ps, dbe_);
  }
 //------------------------------------------------------------//


 //---------------------- Tower Jet Monitor --------------------// 
  if ( ps.getUntrackedParameter<bool>("TowerJetMonitor", false) ) {
    if(fVerbosity>0) std::cout << "CastorMonitorModule: Tower Jet monitor flag is on...." << std::endl;
    TowerJetMon_ = new CastorTowerJetMonitor();
    TowerJetMon_->setup(ps, dbe_);
  }
 //------------------------------------------------------------//



   ////---- ADD OTHER MONITORS HERE !!!
  
  ////---- get steerable variables
  prescaleEvt_ = ps.getUntrackedParameter<int>("diagnosticPrescaleEvt", -1);
  if(fVerbosity>0) std::cout << "===>CastorMonitor event prescale = " << prescaleEvt_ << " event(s)"<< std::endl;

  prescaleLS_ = ps.getUntrackedParameter<int>("diagnosticPrescaleLS", -1);
  if(fVerbosity>0) std::cout << "===>CastorMonitor lumi section prescale = " << prescaleLS_ << " lumi section(s)"<< std::endl;
  if (prescaleLS_>0) actonLS_=true;

  prescaleUpdate_ = ps.getUntrackedParameter<int>("diagnosticPrescaleUpdate", -1);
  if(fVerbosity>0) std::cout << "===>CastorMonitor update prescale = " << prescaleUpdate_ << " update(s)"<< std::endl;

  prescaleTime_ = ps.getUntrackedParameter<int>("diagnosticPrescaleTime", -1);
  if(fVerbosity>1) std::cout << "===>CastorMonitor time prescale = " << prescaleTime_ << " minute(s)"<< std::endl;

  ////---- base folder for the contents of this job
  std::string subsystemname = ps.getUntrackedParameter<std::string>("subSystemFolder", "Castor") ;
  if(fVerbosity>0) std::cout << "===>CastorMonitor name = " << subsystemname << std::endl;
  rootFolder_ = subsystemname + "/";
  
 if ( dbe_ != NULL ){
  dbe_->setCurrentFolder(rootFolder_);
  }


  gettimeofday(&psTime_.updateTV,NULL);
  ////---- get time in milliseconds, convert to minutes
  psTime_.updateTime = (psTime_.updateTV.tv_sec*1000.0+psTime_.updateTV.tv_usec/1000.0);
  psTime_.updateTime /= 1000.0;
  psTime_.elapsedTime=0;
  psTime_.vetoTime=psTime_.updateTime;

 if(fVerbosity>0) std::cout << "CastorMonitorModule Constructor (end)" << std::endl;

}


//==================================================================//
//======================= Destructor ===============================//
//==================================================================//
CastorMonitorModule::~CastorMonitorModule(){
  
// if (dbe_){    
// if(DataIntMon_!=NULL)     {  DigiMon_->clearME();}
//   if(DigiMon_!=NULL)     {  DigiMon_->clearME();}
//   if(RecHitMon_!=NULL)  {  RecHitMon_->clearME();}
//   if(LedMon_!=NULL)     {  LedMon_->clearME();}
//   if(PSMon_!=NULL)     {  LedMon_->clearME();}
//   if(HIMon_!=NULL)     {  HIMon_->clearME();}
//   if(TowerJetMon_!=NULL)     {  HIMon_->clearME();}
//   dbe_->setCurrentFolder(rootFolder_);
//   dbe_->removeContents();
// }
//
//  if(DigiMon_!=NULL)    { delete DigiMon_;   DigiMon_=NULL;     }
//  if(RecHitMon_!=NULL) { delete RecHitMon_; RecHitMon_=NULL; }
//  if(HIMon_!=NULL) { delete HIMon_; HIMon_=NULL; }
//  if(TowerJetMon_!=NULL) { delete TowerJetMon_; TowerJetMon_=NULL; }
//  if(LedMon_!=NULL)    { delete LedMon_;   LedMon_=NULL;     }
//  delete evtSel_; evtSel_ = NULL;

} 


//=================================================================//
//========================== beginJob =============================//
//================================================================//
void CastorMonitorModule::beginJob(){
  ievt_ = 0;
  ievt_pre_=0;
  
  rawOK_    = false;
  reportOK_ = false;
  digiOK_   = false;
  rechitOK_ = false;
  towerOK_  = false;
  jetOK_    = false;
  jetIdOK_  = false;
  
  nRaw=0;
  nDigi=0;
  nRechit=0;
  nTower=0;
  nJet=0;
  nJetId=0;

  if(fVerbosity>0) std::cout << "CastorMonitorModule::beginJob (start)" << std::endl;

  if ( dbe_ != NULL ){
  

     ////---- create EventProduct histogram
    dbe_->setCurrentFolder(rootFolder_+"CastorEventProducts");
    meEVT_ = dbe_->bookInt("Event Number"); 
    CastorEventProduct =dbe_->book2D("CastorEventProduct","CastorEventProduct",6,0,6,1,0,1);
    /*
    CastorEventProduct->getTH2F()->GetXaxis()->SetBinLabel(1,"RawData");
    CastorEventProduct->getTH2F()->GetXaxis()->SetBinLabel(2,"CastorDigi");
    CastorEventProduct->getTH2F()->GetXaxis()->SetBinLabel(3,"CastorRecHits");
    CastorEventProduct->getTH2F()->GetXaxis()->SetBinLabel(4,"CastorTowers");
    CastorEventProduct->getTH2F()->GetXaxis()->SetBinLabel(4,"CastorBasicJets");
    CastorEventProduct->getTH2F()->GetXaxis()->SetBinLabel(5,"CastorJetIDs");
    CastorEventProduct->getTH2F()->GetYaxis()->SetBinLabel(1,"Status");
    CastorEventProduct->getTH2F()->SetBinContent(1,1,-1);
    CastorEventProduct->getTH2F()->SetBinContent(2,1,-1);
    CastorEventProduct->getTH2F()->SetBinContent(3,1,-1);
    CastorEventProduct->getTH2F()->SetOption("textcolz");
    */


    dbe_->setCurrentFolder(rootFolder_+"DQM Job Status" );
    meStatus_  = dbe_->bookInt("STATUS");
    meRunType_ = dbe_->bookInt("RUN TYPE");
    meEvtMask_ = dbe_->bookInt("EVT MASK");
    meFEDS_    = dbe_->book1D("FEDs Unpacked","FEDs Unpacked",100,660,759);
    meCASTOR_ = dbe_->bookInt("CASTORpresent");
    ////---- process latency 
    meLatency_ = dbe_->book1D("Process Latency","Process Latency",2000,0,10);
    meQuality_ = dbe_->book1D("Quality Status","Quality Status",100,0,1);
    meStatus_->Fill(0);
    meRunType_->Fill(-1);
    meEvtMask_->Fill(-1);
    ////---- should fill with 0 to start
    meCASTOR_->Fill(0); 
    }
  else{
    if(fVerbosity>0) std::cout << "CastorMonitorModule::beginJob - NO DQMStore service" << std::endl; 
  }
 
 if(fVerbosity>0) std::cout << "CastorMonitorModule::beginJob (end)" << std::endl;

  return;
} 


//=================================================================//
//========================== beginRun =============================//
//================================================================//
void CastorMonitorModule::beginRun(const edm::Run& iRun, const edm::EventSetup& iSetup) {

  fedsListed_ = false;
  reset();

  ////---- get Castor Conditions and eMap at the beginning of each new run
  iSetup.get<CastorDbRecord>().get(conditions_);
  CastorReadoutMap_=conditions_->getCastorMapping();
  
  ////---- get Castor Pedestal Values from the DB
  iSetup.get<CastorPedestalsRcd>().get(dbPedestals);
  if(!dbPedestals.isValid() && fVerbosity>0)    std::cout << "CASTOR  has no CastorPedestals in the CondDB !!!" << std::endl;
 
  ////----------------- fill fPedestalSigmaAverage ----------////
  float        sigma_averaged;
  unsigned int iChannel  = 0;
  std::vector<DetId> channels = dbPedestals->getAllChannels();
  
  ////---- loop over channels
  for (std::vector<DetId>::iterator ch=channels.begin(); ch!=channels.end(); ch++) {
    const CastorPedestal * pedestals_mean  = dbPedestals->getValues(*ch);
    sigma_averaged = 0.;

    ////---- loop over CapIDs
    for (short unsigned int iCapId = 0; iCapId < 4; iCapId++){
      sigma_averaged += sqrt(pedestals_mean->getWidth(iCapId));
    };

    ////--- fill the array
    fPedestalNSigmaAverage[HcalCastorDetId(*ch).module()-1][HcalCastorDetId(*ch).sector()-1] = sigma_averaged/4;
    iChannel++;
  };

  if(iChannel<224 && fVerbosity>0)  std::cout << "There are less that 224 channels in CastorPedestalsRcd record !!!" << std::endl;



  return;
}

//=================================================================//
//========================== beginLuminosityBlock ================//
//================================================================//
void CastorMonitorModule::beginLuminosityBlock(const edm::LuminosityBlock& lumiSeg, 
					       const edm::EventSetup& context) {
  
  if(actonLS_ && !prescale()){
    ////---- do scheduled tasks...
  }
}


//================================================================//
//========================== endLuminosityBlock ==================//
//================================================================//
void CastorMonitorModule::endLuminosityBlock(const edm::LuminosityBlock& lumiSeg, 
					     const edm::EventSetup& context) {
  if(actonLS_ && !prescale()){
    ////--- do scheduled tasks...
  }
}

//=================================================================//
//========================== endRun ===============================//
//=================================================================//
void CastorMonitorModule::endRun(const edm::Run& r, const edm::EventSetup& context)
{
  if (fVerbosity>0)  std::cout <<"CastorMonitorModule::endRun(...) "<<std::endl;
  ////--- do final pedestal histogram filling
  if (DigiMon_!=NULL) //************ DigiMon_->fillPedestalHistos(); //FIX
  return;
}

//=================================================================//
//========================== endJob ===============================//
//=================================================================//
void CastorMonitorModule::endJob(void) {
 
  //if ( meStatus_ ) meStatus_->Fill(2);
 
  if(RecHitMon_!=NULL) RecHitMon_->done();
  if(DataIntMon_!=NULL) DataIntMon_->done();
  if(DigiMon_!=NULL) DigiMon_->done();
  if(LedMon_!=NULL) LedMon_->done();
  if(CQMon_!=NULL) CQMon_->done();
  if(PSMon_!=NULL) PSMon_->done();
  if(EDMon_!=NULL) EDMon_->done();
  if(HIMon_!=NULL) RecHitMon_->done();
  if(TowerJetMon_!=NULL) TowerJetMon_->done();

  /* LEAVE IT OUT FOR THE MOMENT
  // TO DUMP THE OUTPUT TO DATABASE FILE
  if (dump2database_){
    
    } 
  */
  return;
}


//=================================================================//
//========================== reset  ===============================//
//=================================================================//
void CastorMonitorModule::reset(){

  if(DataIntMon_!=NULL)  DataIntMon_->reset();
  if(DigiMon_!=NULL)     DigiMon_->reset();
  if(RecHitMon_!=NULL)   RecHitMon_->reset();
  if(LedMon_!=NULL)      LedMon_->reset();
  if(CQMon_!=NULL)       CQMon_->reset();
  if(PSMon_!=NULL)       PSMon_->reset();
  if(EDMon_!=NULL)       EDMon_->reset();
  if(HIMon_!=NULL)       HIMon_->reset();
  if(TowerJetMon_!=NULL)       HIMon_->reset();
}


//=================================================================//
//========================== analyze  ===============================//
//=================================================================//
void CastorMonitorModule::analyze(const edm::Event& iEvent, const edm::EventSetup& eventSetup){


  using namespace edm;


  ////---- environment datamembers
  irun_     = iEvent.id().run();
  ilumisec_ = iEvent.luminosityBlock();
  ievent_   = iEvent.id().event();
  itime_    = iEvent.time().value();
  ibunch_   = iEvent.bunchCrossing();


  if (fVerbosity>0) { 
  std::cout << "==> CastorMonitorModule: evts: "<< nevt_ << ", run: " << irun_ << ", LS: " << ilumisec_ << std::endl;
  std::cout << " evt: " << ievent_ << ", time: " << itime_  <<"\t counter = "<< ievt_pre_<< "\t total count = "<<ievt_<<std::endl; 
  }

  ////---- event counter
  ievt_++;

  ////---- skip this event if we're prescaling...
  ievt_pre_++; // need to increment counter before calling prescale
  if(prescale()) return;
  meLatency_->Fill(psTime_.elapsedTime);
  if ( dbe_ ) meStatus_->Fill(1);
    

  //---------------------------------------------------------------//
  //----------- try to get raw data and unpacker report -----------//
  //---------------------------------------------------------------//

  
  ////---- try to get raw data and unpacker report
  edm::Handle<FEDRawDataCollection> RawData;   // We do not want to look at Abort Gap events
  iEvent.getByLabel(inputLabelRaw_,RawData);
  if (RawData.failedToGet()|| !RawData.isValid()){
  rawOK_=false;
  if (fVerbosity>0)  std::cout << "==> CastorMonitorModule: RawData with label: "<<inputLabelRaw_<<" not available !!!";
  }
   else {rawOK_=true; nRaw++;}
 
  edm::Handle<HcalUnpackerReport> unpackReport;
  iEvent.getByLabel(inputLabelReport_,unpackReport);
  if (unpackReport.failedToGet() || !unpackReport.isValid() ) {
       reportOK_=false;
        if (fVerbosity>0)  std::cout << "==> CastorMonitorModule: UnpackerReport with label: "<<inputLabelReport_<<" not available !!!";
        //if (fVerbosity>0) edm::LogWarning("CastorMonitorModule")<<" UnpackerReport with label "<<inputLabelReport_<<" \not available";
     }
   else {reportOK_=true; }

  /*
  edm::Handle<HcalUnpackerReport> report; 
  iEvent.getByType(report);  
  if (!report.isValid()) {
    rawOK_=false;
  }
  else 
    {
      if(!fedsListed_){
	const std::vector<int> feds =  (*report).getFedsUnpacked();    
	for(unsigned int f=0; f<feds.size(); f++){
	  meFEDS_->Fill(feds[f]);    
	}
	fedsListed_ = true;
      }
    }
  */
  //---------------------------------------------------------------//
  //-------------------  try to get digis ------------------------//
  //---------------------------------------------------------------//
  edm::Handle<CastorDigiCollection> CastorDigi;
  iEvent.getByLabel(inputLabelDigi_,CastorDigi);
  if( CastorDigi.failedToGet() || !CastorDigi.isValid() ) digiOK_=false;
  else { digiOK_=true; nDigi++;}
 
  
  ////---- LEAVE IT OUT FOR THE MOMENT
  ////---- check that Castor is on by seeing which are reading out FED data
  //if ( checkCASTOR_ )
  //  CheckCastorStatus(*RawData,*report,*readoutMap_,*CastorDigi);
  

  //---------------------------------------------------------------//
  //------------------- try to get RecHits ------------------------//
  //---------------------------------------------------------------//
  edm::Handle<CastorRecHitCollection> CastorHits;
  iEvent.getByLabel(inputLabelRecHitCASTOR_, CastorHits);
  if( CastorHits.failedToGet() || !CastorHits.isValid() ) rechitOK_ = false;
  else {rechitOK_ = true;  nRechit++;}
  
  //---------------------------------------------------------------//
  //------------------- try to get Towers ------------------------//
  //---------------------------------------------------------------//
  edm::Handle<reco::CastorTowerCollection> CastorTowers;
  iEvent.getByLabel(inputLabelCastorTowers_, CastorTowers);
  if(CastorTowers.failedToGet() || !CastorTowers.isValid() ) towerOK_=false;  
  else {towerOK_=true; nTower++;}
 
  //---------------------------------------------------------------//
  //------------------- try to get BasicJets ----------------------//
  //---------------------------------------------------------------//
  edm::Handle<reco::BasicJet> CastorBasicJets;
  iEvent.getByLabel(inputLabelCastorBasicJets_, CastorBasicJets);  
  if(CastorBasicJets.failedToGet() || !CastorBasicJets.isValid() ) jetOK_=false;
  else {jetOK_=true; nJet++;}

  //---------------------------------------------------------------//
  //------------------- try to get JetIDs    ----------------------//
  //---------------------------------------------------------------//
  edm::Handle<reco::CastorJetIDValueMap> CastorJetIDs;
  iEvent.getByLabel(inputLabelCastorJetIDs_, CastorJetIDs); 
  if(CastorJetIDs.failedToGet() || !CastorJetIDs.isValid() ) jetIdOK_=false;
  else {jetIdOK_=true; nJetId++;}


  if(fVerbosity>0) {
   std::cout << "    RAW Data   ==> " << rawOK_<< std::endl;
   std::cout << "    Digis      ==> " << digiOK_<< std::endl;
   std::cout << "    RecHits    ==> " << rechitOK_<< std::endl;
   std::cout << "    CaloTowers ==> " << towerOK_<< std::endl;
   std::cout << "    BasicJets  ==> " << jetOK_<< std::endl;
   std::cout << "    CastorJetIDs     ==> " << jetIdOK_<< std::endl;
  }
 
  ////---- fill CastorEventProduct histogram
  if(double(nRaw)/double(ievt_)>0.85) CastorEventProduct->getTH2F()->SetBinContent(1,1,1);
  else CastorEventProduct->getTH2F()->SetBinContent(1,1,-1);
  
  if(double(nDigi)/double(ievt_)>0.85) CastorEventProduct->getTH2F()->SetBinContent(2,1,1);
  else CastorEventProduct->getTH2F()->SetBinContent(2,1,-1);

  if(double(nRechit)/double(ievt_)>0.85) CastorEventProduct->getTH2F()->SetBinContent(3,1,1);
  else CastorEventProduct->getTH2F()->SetBinContent(3,1,-1);

  if(double(nTower)/double(ievt_)>0.85) CastorEventProduct->getTH2F()->SetBinContent(4,1,1);
  else CastorEventProduct->getTH2F()->SetBinContent(4,1,-1);

  if(double(nJet)/double(ievt_)>0.85) CastorEventProduct->getTH2F()->SetBinContent(5,1,1);
  else CastorEventProduct->getTH2F()->SetBinContent(5,1,-1);

  if(double(nJetId)/double(ievt_)>0.85) CastorEventProduct->getTH2F()->SetBinContent(6,1,1);
  else CastorEventProduct->getTH2F()->SetBinContent(6,1,-1);
 
  

  //------------------------------------------------------------//
  //---------------- Run the configured tasks ------------------//
  //-------------- protect against missing products -----------//
  //-----------------------------------------------------------//

 if (showTiming_){
      cpu_timer.reset(); cpu_timer.start();
  }
 

  //----------------- Data Integrity monitor task ------------------//
 if(rawOK_ && reportOK_) DataIntMon_->processEvent(*RawData,*unpackReport,*CastorReadoutMap_ ); 
  if (showTiming_){
      cpu_timer.stop();
      if (DataIntMon_!=NULL) std::cout <<"TIMER:: DATA INTEGRITY MONITOR ->"<<cpu_timer.cpuTime()<<std::endl;
      cpu_timer.reset(); cpu_timer.start();
    }

 
  //----------------- Digi monitor task ------------------//
  if(digiOK_) DigiMon_->processEvent(*CastorDigi,*conditions_);
  if (showTiming_){
      cpu_timer.stop();
      if (DigiMon_!=NULL) std::cout <<"TIMER:: DIGI MONITOR ->"<<cpu_timer.cpuTime()<<std::endl;
      cpu_timer.reset(); cpu_timer.start();
    }
 
 
 //----------------- Rec Hit monitor task -------------------------//
 if(rechitOK_) RecHitMon_->processEvent(*CastorHits);
 if (showTiming_){
      cpu_timer.stop();
      if (RecHitMon_!=NULL) std::cout <<"TIMER:: RECHIT MONITOR ->"<<cpu_timer.cpuTime()<<std::endl;
      cpu_timer.reset(); cpu_timer.start();
    } 
 
   //----------------- Channel Quality Monitor task -------------------------//
 if(rechitOK_) CQMon_->processEvent(*CastorHits);
 if (showTiming_){
      cpu_timer.stop();
      if (CQMon_!=NULL) std::cout <<"TIMER:: CHANNELQUALITY MONITOR ->"<<cpu_timer.cpuTime()<<std::endl;
      cpu_timer.reset(); cpu_timer.start();
    }
 
 ////--- TAKE IT OUT FOR THE TIME BEING
 /*
  //---------------- LED monitor task ------------------------//
  if(digiOK_) LedMon_->processEvent(*CastorDigi,*conditions_);
   if (showTiming_){
       cpu_timer.stop();
       if (LedMon_!=NULL) std::cout <<"TIMER:: LED MONITOR ->"<<cpu_timer.cpuTime()<<std::endl;
       cpu_timer.reset(); cpu_timer.start();
     }
 */
 //---------------- Pulse Shape monitor task ------------------------//
 ////---- get electronics map
 
  edm::ESHandle<CastorElectronicsMap> refEMap;
  eventSetup.get<CastorElectronicsMapRcd>().get(refEMap);
  const CastorElectronicsMap* myRefEMap = refEMap.product();
  listEMap = myRefEMap->allPrecisionId();
  if(digiOK_) PSMon_->processEvent(*CastorDigi,*conditions_, listEMap, ibunch_, fPedestalNSigmaAverage);  
  if (showTiming_) {
      cpu_timer.stop();
      if (PSMon_!=NULL) std::cout <<"TIMER:: PULSE SHAPE  ->"<<cpu_timer.cpuTime()<<std::endl;
      cpu_timer.reset(); cpu_timer.start();
    }



  //---------------- EventDisplay monitor task ------------------------//
  ////---- get calo geometry
  edm::ESHandle<CaloGeometry> caloGeometry;
  eventSetup.get<CaloGeometryRecord>().get(caloGeometry);
  
 if(rechitOK_ && EDMonOn_) EDMon_->processEvent(*CastorHits, *caloGeometry);
 if (showTiming_){
      cpu_timer.stop();
      if (EDMon_!=NULL) std::cout <<"TIMER:: EVENTDISPLAY MONITOR ->"<<cpu_timer.cpuTime()<<std::endl;
      cpu_timer.reset(); cpu_timer.start();
    }   

//----------------- Heavy Ion monitor task -------------------------//
 if(rechitOK_ && digiOK_ ) HIMon_->processEvent(*CastorHits, *CastorDigi, *conditions_);
 if (showTiming_){
      cpu_timer.stop();
      if (HIMon_!=NULL) std::cout <<"TIMER:: HI MONITOR ->"<<cpu_timer.cpuTime()<<std::endl;
      cpu_timer.reset(); cpu_timer.start();
    }
 
 

//----------------- TowerJet monitor task ------------------//
 if( towerOK_ ) TowerJetMon_->processEventTowers(*CastorTowers);
 if( jetOK_   ) TowerJetMon_->processEventJets(*CastorBasicJets);
 if( jetIdOK_ ) TowerJetMon_->processEventJetIDs(*CastorJetIDs);
 
  if (showTiming_){
      cpu_timer.stop();
      if (TowerJetMon_!=NULL) std::cout <<"TIMER:: TOWER JET MONITOR ->"<<cpu_timer.cpuTime()<<std::endl;
      cpu_timer.reset(); cpu_timer.start();
    }


  if(fVerbosity>0 && ievt_%100 == 0)
    std::cout << "CastorMonitorModule: processed " << ievt_ << " events" << std::endl;
  

 ////---- fill the event number
  meEVT_->Fill(ievt_);

  return;
}




//=====================================================================//
//========================== prescale  ===============================//
//===================================================================//
////// It returns true if this event should be skipped according to 
////// the prescale condition.
bool CastorMonitorModule::prescale()
{
  if (fVerbosity>0) std::cout <<"CastorMonitorModule::prescale"<<std::endl;
  
  gettimeofday(&psTime_.updateTV,NULL);
  double time = (psTime_.updateTV.tv_sec*1000.0+psTime_.updateTV.tv_usec/1000.0);
  time/= (1000.0); ///in seconds
  psTime_.elapsedTime = time - psTime_.updateTime;
  psTime_.updateTime = time;
  ////---- determine if we care...
  bool evtPS =    prescaleEvt_>0;
  bool lsPS =     prescaleLS_>0;
  bool timePS =   prescaleTime_>0;
  bool updatePS = prescaleUpdate_>0;

  ////---- if no prescales are set, keep the event
  if(!evtPS && !lsPS && !timePS && !updatePS) return false;
    
  ////---- check each instance
  if(lsPS && (ilumisec_%prescaleLS_)!=0) lsPS = false; //-- LS veto
  //if(evtPS && (ievent_%prescaleEvt_)!=0) evtPS = false; //evt # veto
  if (evtPS && (ievt_pre_%prescaleEvt_)!=0) evtPS = false;
  if(timePS)
    {
      double elapsed = (psTime_.updateTime - psTime_.vetoTime)/60.0;
      if(elapsed<prescaleTime_){
	timePS = false;  //-- timestamp veto
	psTime_.vetoTime = psTime_.updateTime;
      }
    } 

  //  if(prescaleUpdate_>0 && (nupdates_%prescaleUpdate_)==0) updatePS=false; ///need to define what "updates" means
  
  if (fVerbosity>0) 
    {
      std::cout<<"CastorMonitorModule::prescale  evt: "<<ievent_<<"/"<<evtPS<<", ";
      std::cout <<"ls: "<<ilumisec_<<"/"<<lsPS<<",";
      std::cout <<"time: "<<psTime_.updateTime - psTime_.vetoTime<<"/"<<timePS<<std::endl;
    }  
  ////---- if any criteria wants to keep the event, do so
  if(evtPS || lsPS || timePS) return false;
  return true;
} 



//====================================================================//
//=================== CheckCastorStatus  =============================//
//====================================================================//
////---- This function provides a check whether the Castor is on 
////---- by seeing which are reading out FED data  

void CastorMonitorModule::CheckCastorStatus(const FEDRawDataCollection& RawData, 
					       const HcalUnpackerReport& report, 
					       const CastorElectronicsMap& emap,
					       const CastorDigiCollection& CastorDigi
		         		     )
{
  
  ////---- comment this out, since it is anyway out of use  now
  //vector<int> fedUnpackList;
  ////---- NO getCastorFEDIds() at the moment
  // for (int i=FEDNumbering::getHcalFEDIds().first; i<=FEDNumbering::getHcalFEDIds().second; i++) 
  //   {
  //     fedUnpackList.push_back(i);
  //   }
  // for (std::vector<int>::const_iterator i=fedUnpackList.begin(); i!=fedUnpackList.end();++i) 
  //   {
  //     const FEDRawData& fed = RawData.FEDData(*i);
  //     if (fed.size()<12) continue; //-- Was 16 !      
      ////---- get the DCC header - NO CastorDCCHeader at the moment
  //     const HcalDCCHeader* dccHeader=(const HcalDCCHeader*)(fed.data());
  //    if (!dccHeader) return;
  //    int dccid=dccHeader->getSourceId();
    
   ////---- check for CASTOR
  //     ////---- Castor FED numbering of DCCs= [690 -693]  
  //    if (dccid >= 690 && dccid <=693){
  //	if ( CastorDigi.size()>0){
  //	  meCASTOR_->Fill(1); 
  //	 }
  //	else {meCASTOR_->Fill(0);  }  
  //     }
  //    else{ meCASTOR_->Fill(-1); }
  //  }
  return;
}

#include "FWCore/Framework/interface/MakerMacros.h"
#include <DQM/CastorMonitor/interface/CastorMonitorModule.h>
#include "DQMServices/Core/interface/DQMStore.h"

DEFINE_FWK_MODULE(CastorMonitorModule);
