#include "DQM/CastorMonitor/interface/CastorPedestalMonitor.h"
#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/MonitorElement.h"

//***************************************************//
//********** CastorPedestalMonitor: *******************//
//********** pedestals of Castor r/o channels ********//
//********** Author: Dmytro Volyanskyy   ************//
//********** Date  : 29.08.2008          ************// 
//***************************************************//

CastorPedestalMonitor::CastorPedestalMonitor() { doPerChannel_ = false;  }

CastorPedestalMonitor::~CastorPedestalMonitor() {
}

void CastorPedestalMonitor::reset(){}

void CastorPedestalMonitor::setup(const edm::ParameterSet& ps, DQMStore* dbe){
  CastorBaseMonitor::setup(ps,dbe);
  baseFolder_ = rootFolder_+"CastorPedestalMonitor";

  doPerChannel_ = ps.getUntrackedParameter<bool>("PedestalsPerChannel", false);
  doFCpeds_ = ps.getUntrackedParameter<bool>("PedestalsInFC", true);

  ievt_=0;

  if ( m_dbe ) {
    m_dbe->setCurrentFolder(baseFolder_);
    meEVT_ = m_dbe->bookInt("Pedestal Task Event Number");
    meEVT_->Fill(ievt_);
        
    m_dbe->setCurrentFolder(baseFolder_);
  
    //========== book the following histograms ========//
    char* type = "Castor All Pedestal Values";
    castHists.ALLPEDS =  m_dbe->book1D(type,type,50,0,50);
    
    type = "Castor Pedestal Mean Reference Values";
    castHists.PEDESTAL_REFS = m_dbe->book1D(type,type,50,0,3); 
    
    type = "Castor Pedestal RMS Reference Values";
    castHists.WIDTH_REFS = m_dbe->book1D(type,type,50,0,3); 

    ///// castHists.PEDRMS  =  m_dbe->book1D("Castor Pedestal RMS Values","Castor Pedestal RMS Values",100,0,3);
    ///// castHists.SUBMEAN =  m_dbe->book1D("Castor Subtracted Mean Values","Castor Subtracted Mean Values",100,-2.5,2.5);
    /////  castHists.PEDMEAN =  m_dbe->book1D("Castor Pedestal Mean Values","Castor Pedestal Mean Values",100,0,9);
    ///// castHists.QIERMS  =  m_dbe->book1D("Castor QIE RMS Values","Castor QIE RMS Values",50,0,3);
    ///// castHists.QIEMEAN =  m_dbe->book1D("Castor QIE Mean Values","Castor QIE Mean Values",50,0,10);
 
}
 
  outputFile_ = ps.getUntrackedParameter<string>("PedestalFile", "");
  if ( outputFile_.size() != 0 ) { if(fVerbosity) cout << "Castor Pedestal Calibrations will be saved to " << outputFile_.c_str() << endl;}

  return;
}



//==========================================================//
//========================= processEvent ===================//
//==========================================================//


void CastorPedestalMonitor::processEvent(const CastorDigiCollection& cast, const CastorDbService& cond)
{
  
  ievt_++;
  meEVT_->Fill(ievt_);
  
  if(!shape_) shape_ = cond.getCastorShape(); // this one is generic

  if(!m_dbe) { 
    if(fVerbosity) printf("CastorPedestalMonitor::processEvent DQMStore not instantiated!!!\n");  
    return; 
  }

  CaloSamples tool;  
 
   try{
    for (CastorDigiCollection::const_iterator j=cast.begin(); j!=cast.end(); j++){
      const CastorDataFrame digi = (const CastorDataFrame)(*j);	

       //======= get access to Castor Pedestal in the CONDITION DATABASE =====//
       /////// calibs_= cond.getCastorCalibrations(digi.id());  //in HCAL code 
       const CastorPedestal* ped = cond.getPedestal(digi.id()); 
       const CastorPedestalWidth* pedw = cond.getPedestalWidth(digi.id());

       detID_.clear(); capID_.clear(); pedVals_.clear();
      
       //===== if to convert ADC to fC 
      if(doFCpeds_){
	channelCoder_ = cond.getCastorCoder(digi.id());
	CastorCoderDb coderDB(*channelCoder_, *shape_);
	coderDB.adc2fC(digi,tool);
      }

      //====== fill Pedestal Mean and RMS values from the CONDITION DATABASE ======//
      for(int capID=0; capID<4; capID++){
           //------- Pedestal Mean from the Condition Database
	   float pedvalue=0; 	 
	   if(ped) pedvalue=ped->getValue(capID);
           castHists.PEDESTAL_REFS->Fill(pedvalue);
           PEDESTAL_REFS->Fill(pedvalue);
          ////////// castHists.PEDESTAL_REFS->Fill(calibs_.pedestal(capID)); //In HCAL code
	  /////////   PEDESTAL_REFS->Fill(calibs_.pedestal(capID));  // In HCAL code
          //-------- Pedestal RMS from the Condition Database 
           float width=0;
	   if(pedw) width = pedw->getWidth(capID);
           castHists.WIDTH_REFS->Fill(width);
           WIDTH_REFS->Fill(width);
     }
      
      //=========== fill ALL Pedestal Values ==================//
      for (int i=0; i<digi.size(); i++) {
	if(doFCpeds_) pedVals_.push_back(tool[i]);
	else pedVals_.push_back(digi.sample(i).adc());
	detID_.push_back(digi.id());
	capID_.push_back(digi.sample(i).capid());
	castHists.ALLPEDS->Fill(pedVals_[i]);
      }

      //do histograms for every channel
      if(doPerChannel_) perChanHists(detID_,capID_,pedVals_,castHists.PEDVALS, baseFolder_);

    }
  } 
   catch (...) {
    if(fVerbosity) cout << "CastorPedestalMonitor::processEvent  No Castor Digis." << endl;
  }

  return;
}

void CastorPedestalMonitor::done(){

  return;
}


//==========================================================//
//=============== do histograms for every channel ==========//
//==========================================================//

void CastorPedestalMonitor::perChanHists( vector<HcalCastorDetId> detID, vector<int> capID, vector<float> peds,
				          map<HcalCastorDetId, map<int, MonitorElement*> > &toolP,  
				          ////// map<HcalCastorDetId, map<int, MonitorElement*> > &toolS, 
                                          string baseFolder) 
 {
  
  if(m_dbe) m_dbe->setCurrentFolder(baseFolder);
  string type = "Castor";

  //================= loop over all channels ==============//
  for(unsigned int d=0; d<detID.size(); d++){
    HcalCastorDetId detid = detID[d];
    int capid = capID[d];
    float pedVal = peds[d];
    //-- outer iteration
    bool gotit=false;
    if(REG[detid]) gotit=true;
    
    if(gotit){
      //inner iteration
      map<int, MonitorElement*> _mei = toolP[detid];
      if(_mei[capid]==NULL){
	if(fVerbosity) printf("CastorPedestalAnalysis::perChanHists  This histo is NULL!!??\n");
      }
      else _mei[capid]->Fill(pedVal);
      
      ///////// _mei = toolS[detid];
      ////////  if(_mei[capid]==NULL){
      ////////	if(fVerbosity) printf("CastorPedestalAnalysis::perChanHists  This histo is NULL!!??\n");
      ////////  }
      //////// else _mei[capid]->Fill(pedVal-calibs_.pedestal(capid));
    }
    else{
      if(m_dbe){
	map<int,MonitorElement*> insertP; // Pedestal values in ADC
         //////// map<int,MonitorElement*> insertS; // Pedestal values (substracted) 
	
        //===== Loop over capID ====:
	for(int i=0; i<4; i++){
	  char name[1024];
	  sprintf(name,"%s Pedestal Value (ADC) zside=%d module=%d sector=%d CAPID=%d",
		  type.c_str(),detid.zside(),detid.module(),detid.sector(),i);      
	  insertP[i] =  m_dbe->book1D(name,name,10,-0.5,9.5);
	  
	  ////////// sprintf(name,"%s Pedestal Value (Subtracted) zside=%d module=%d sector=%d CAPID=%d",
	  /////////  type.c_str(),detid.zside(),detid.module(),detid.sector(),i);      
	  /////////  insertS[i] =  m_dbe->book1D(name,name,10,-5,5);	
	}
	
	insertP[capid]->Fill(pedVal);
	//////// insertS[capid]->Fill(pedVal-calibs_.pedestal(capid));
	toolP[detid] = insertP;
	//////// toolS[detid] = insertS;
      }
      REG[detid] = true;
    }
  }
}
