// $Id$
//
//    File: JEventProcessor_FCALPedestalPlugin.cc
// Created: Thu May 14 12:49:44 EDT 2015
// Creator: manlara (on Linux ifarm1401 2.6.32-431.el6.x86_64 x86_64)
//

#include "JEventProcessor_FCALPedestalPlugin.h"
using namespace jana;

#include <stdint.h>
// https://halldsvn.jlab.org/repos/branches/sim-recon-commissioning/src/libraries/DAQ/
#include <DAQ/Df250WindowRawData.h>
#include <DAQ/Df250PulseRawData.h>
#include <DAQ/Df250PulseIntegral.h>
#include <DAQ/Df250PulseTime.h>
#include <DAQ/Df250PulsePedestal.h>
#include <DAQ/Df250TriggerTime.h>



using namespace std;

// FCAL Hit Objects
// https://halldsvn.jlab.org/repos/branches/sim-recon-commissioning/src/libraries/FCAL/

// Other peoples plugins
// https://halldsvn.jlab.org/repos/branches/sim-recon-commissioning/src/programs/Analysis/plugins/


bool Df250WindowRawData_cmp(const Df250WindowRawData *a,const Df250WindowRawData *b){
	// sort by crate, then by slot, then by channel, then by trigger number
	if (a->rocid   != b->rocid)   return a->rocid < b->rocid;
	if (a->slot    != b->slot )   return a->slot < b->slot;
	if (a->channel != b->channel) return a->channel < b->channel;
	return a->itrigger < b->itrigger;
}
bool Df250PulseRawData_cmp(const Df250PulseRawData *a,const Df250PulseRawData *b){
	// sort by crate, then by slot, then by channel, then by trigger number
	if (a->rocid   != b->rocid)   return a->rocid < b->rocid;
	if (a->slot    != b->slot )   return a->slot < b->slot;
	if (a->channel != b->channel) return a->channel < b->channel;
	return a->itrigger < b->itrigger;
}
bool Df250PulseIntegral_cmp(const Df250PulseIntegral *a,const Df250PulseIntegral *b){
	// sort by crate, then by slot, then by channel, then by trigger number
	if (a->rocid   != b->rocid)   return a->rocid < b->rocid;
	if (a->slot    != b->slot )   return a->slot < b->slot;
	if (a->channel != b->channel) return a->channel < b->channel;
	return a->itrigger < b->itrigger;
}
bool Df250PulseTime_cmp(const Df250PulseTime *a,const Df250PulseTime *b){
	// sort by crate, then by slot, then by channel, then by trigger number
	if (a->rocid   != b->rocid)   return a->rocid < b->rocid;
	if (a->slot    != b->slot )   return a->slot < b->slot;
	if (a->channel != b->channel) return a->channel < b->channel;
	return a->itrigger < b->itrigger;
}
bool Df250PulsePedestal_cmp(const Df250PulsePedestal *a,const Df250PulsePedestal *b){
	// sort by crate, then by slot, then by channel, then by trigger number
	if (a->rocid   != b->rocid)   return a->rocid < b->rocid;
	if (a->slot    != b->slot )   return a->slot < b->slot;
	if (a->channel != b->channel) return a->channel < b->channel;
	return a->itrigger < b->itrigger;
}
bool Df250TriggerTime_cmp(const Df250TriggerTime *a,const Df250TriggerTime *b){
	// sort by crate, then by slot, then by channel, then by trigger number
	if (a->rocid   != b->rocid)   return a->rocid < b->rocid;
	if (a->slot    != b->slot )   return a->slot < b->slot;
	return a->itrigger < b->itrigger;
}

// Routine used to create our JEventProcessor
#include <JANA/JApplication.h>
#include <JANA/JFactory.h>
extern "C"{
void InitPlugin(JApplication *app){
	InitJANAPlugin(app);
	app->AddProcessor(new JEventProcessor_FCALPedestalPlugin());
}
} // "C"


//------------------
// JEventProcessor_FCALPedestalPlugin (Constructor)
//------------------
JEventProcessor_FCALPedestalPlugin::JEventProcessor_FCALPedestalPlugin()
{

}

//------------------
// ~JEventProcessor_FCALPedestalPlugin (Destructor)
//------------------
JEventProcessor_FCALPedestalPlugin::~JEventProcessor_FCALPedestalPlugin()
{

}

//------------------
// init
//------------------
jerror_t JEventProcessor_FCALPedestalPlugin::init(void)
{
	// This is called once at program startup. If you are creating
	// and filling historgrams in this plugin, you should lock the
	// ROOT mutex like this:
	//
	// japp->RootWriteLock();
	//  ... fill historgrams or trees ...
	// japp->RootUnLock();
	//
  
  japp->RootWriteLock();
  
  // Initialize FCAL Channels
  
  // The rocid is between rocid>=11 && rocid<=22
  int max_rocid = 22, min_rocid=11;
  // 3 to 19
  // skip 11 and 12
  int max_slot = 19, min_slot=3;
  // 16 channels starting from 0
  int n_channels = 16;
  for (int roc=min_rocid; roc<=max_rocid; roc++){
    for (int slot=min_slot; slot<=max_slot; slot++){
      for (int channel=0; channel<n_channels; channel++){
        for (int nsamples=1; nsamples<=maxSampleAvg; nsamples++){
          for (int nevents=1; nevents<=maxEvents; nevents++){
            char name[50];
            sprintf(name,"%02i/%02i/%02i",roc,slot,channel);
            TString key = TString(name)+"_"+StringUtilities::int2TString(nsamples)+"_"+StringUtilities::int2TString(nevents);
            Pedestal::Pedestal ped(key);
            //cout << "Initializing... " << key << endl;
            ped_map[key] = ped;
            ped_ana[key] = ped;
          }
        }
      }
    }
  }
  japp->RootUnLock();
  
  // Setup translation table between crate/slot/channel -> x,y
  // loop over DAQCoordinate_Ordered.txt where crate starts from 0
  japp->RootWriteLock();

  string daq_loc;
  int abs_num;
  string det_loc;

  string line;
  std::istringstream lin;
  ifstream inFile("DAQCoordinate_Ordered.txt");
  if(inFile.is_open())
  {
    while( getline (inFile,line) )
    {
      lin.clear();
      lin.str(line);
      std::istringstream iss(line);

      if (iss >> daq_loc >> abs_num >> det_loc){
        vector<TString> parseDAQ = StringUtilities::parseTString(TString(daq_loc),"/");
        vector<TString> parseLOC = StringUtilities::parseTString(TString(det_loc),"/");
        int rocid = parseDAQ[0].Atoi()+min_rocid, slot = parseDAQ[1].Atoi(), channel = parseDAQ[2].Atoi();
        int m_x = parseLOC[0].Atoi(), m_y = parseLOC[1].Atoi();
        char name[50];
        sprintf(name,"%02i/%02i/%02i",rocid,slot,channel);
        tranlation_map[TString(name)] = make_pair(m_x,m_y);
      }
    }
  }
  
  japp->RootUnLock();
	
	
	FCAL_Pedestal = new TTree("FCAL_Pedestal","Pedestal information");
  FCAL_Pedestal->Branch("rocid",    &rocid   , "rocid/i");    // vme crate number
  FCAL_Pedestal->Branch("slot",     &slot    , "slot/i");     // board number
  FCAL_Pedestal->Branch("channel",  &channel , "channel/i");  // board channel number
  FCAL_Pedestal->Branch("x",        &x   ,     "x/I");        // x coordinate where the center of (0,0)
  FCAL_Pedestal->Branch("y",        &y   ,     "y/I");        // y coordinate where the center of (0,0)
  FCAL_Pedestal->Branch("rocid",    &rocid   , "rocid/i");    // vme crate number
  FCAL_Pedestal->Branch("pedestal", &pedestal, "pedestal/D"); // pedestal calculated by averaging nEvents
  FCAL_Pedestal->Branch("pedestalUInt", &pedestal_uint, "pedestalUInt/i"); // pedestal calculated by averaging nEvents and saved as an unsigned int
  FCAL_Pedestal->Branch("rms",      &rms,      "rms/D");      // root-mean-square of pedestal average
  FCAL_Pedestal->Branch("rms_gaus", &rms_gaus,  "rms_gaus/D");      // root-mean-square of pedestal average
  FCAL_Pedestal->Branch("nSamples", &nSamples, "nSamples/i"); // the number of samples used in pedestal average
  FCAL_Pedestal->Branch("nEvents", &nEvents, "nEvents/i");    // the number of events to be averaged in the pedestal calc
  FCAL_Pedestal->Branch("eventnum", &eventnum, "eventnum/i"); // event number
  FCAL_Pedestal->Branch("run",      &run,      "run/i");      // run number  

  
  FCAL_Analysis = new TTree("FCAL_Analysis","Pedestal analysis");
  FCAL_Analysis->Branch("rocid",    &rocid   , "rocid/i");    // vme crate number
  FCAL_Analysis->Branch("slot",     &slot    , "slot/i");     // board number
  FCAL_Analysis->Branch("channel",  &channel , "channel/i");  // board channel number
  FCAL_Analysis->Branch("x",        &x   ,     "x/I");        // x coordinate where the center of (0,0)
  FCAL_Analysis->Branch("y",        &y   ,     "y/I");        // y coordinate where the center of (0,0)
  FCAL_Analysis->Branch("rocid",    &rocid   , "rocid/i");    // vme crate number
  FCAL_Analysis->Branch("nentries_pedestal",      &nentries_pedestal,      "nentries_pedestal/i");      // number of entries used in pedestal average
  FCAL_Analysis->Branch("nentries_rms",      &nentries_rms,      "nentries_rms/i");      // number of entries used in pedestal rms
  FCAL_Analysis->Branch("avg_pedestal", &avg_pedestal, "avg_pedestal/D"); // pedestal calculated by averaging nEvents
  FCAL_Analysis->Branch("avg_rms",      &avg_rms,      "avg_rms/D");      // average root-mean-square of pedestal
  FCAL_Analysis->Branch("rms_pedestal", &rms_pedestal, "rms_pedestal/D"); // rms of pedestal
  FCAL_Analysis->Branch("rms_pedestal_gaus", &rms_pedestal_gaus, "rms_pedestal_gaus/D"); // rms of pedestal
  FCAL_Analysis->Branch("rms_rms",      &rms_rms,      "rms_rms/D");      // root-mean-square of pedestal rms
  FCAL_Analysis->Branch("avg_pedestalUInt", &avg_pedestal_uint, "avg_pedestalUInt/D"); // pedestal calculated by averaging nEvents
  FCAL_Analysis->Branch("rms_pedestalUInt",      &rms_pedestal_uint,      "rms_pedestalUInt/D");      // root-mean-square of pedestal average
  FCAL_Analysis->Branch("rms_pedestalUInt_gaus",      &rms_pedestal_uint_gaus,      "rms_pedestalUInt_gaus/D");      // root-mean-square of pedestal average
  FCAL_Analysis->Branch("nSamples", &nSamples, "nSamples/i"); // the number of samples used in pedestal average
  FCAL_Analysis->Branch("nEvents", &nEvents, "nEvents/i");    // the number of events to be averaged in the pedestal calc
  FCAL_Analysis->Branch("eventnum", &eventnum, "eventnum/i"); // event number
  FCAL_Analysis->Branch("run",      &run,      "run/i");      // run number

	return NOERROR;
}

//------------------
// brun
//------------------
jerror_t JEventProcessor_FCALPedestalPlugin::brun(JEventLoop *eventLoop, int runnumber)
{
	// This is called whenever the run number changes
	// FreeClear();
	return NOERROR;
}

//------------------
// evnt
//------------------
jerror_t JEventProcessor_FCALPedestalPlugin::evnt(JEventLoop *loop, int eventnumber)
{
	// This is called for every event. Use of common resources like writing
	// to a file or filling a histogram should be mutex protected. Using
	// loop->Get(...) to get reconstructed objects (and thereby activating the
	// reconstruction algorithm) should be done outside of any mutex lock
	// since multiple threads may call this method at the same time.
	// Here's an example:
	//
	// vector<const MyDataClass*> mydataclasses;
	// loop->Get(mydataclasses);
	//
	// japp->RootWriteLock();
	//  ... fill historgrams or trees ...
	// japp->RootUnLock();
	
	vector<const Df250WindowRawData*> f250WindowRawData_vec;
  loop->Get(f250WindowRawData_vec);
  sort(f250WindowRawData_vec.begin(), f250WindowRawData_vec.end(), Df250WindowRawData_cmp);
	
	vector<const Df250PulsePedestal*> f250PulsePedestal_vec;
  loop->Get(f250PulsePedestal_vec);
  sort(f250PulsePedestal_vec.begin(), f250PulsePedestal_vec.end(), Df250PulsePedestal_cmp);

  bool get_windowraw_pedestals = f250WindowRawData_vec.size()>0 ? true : false;
  
  if (get_windowraw_pedestals){
    for (int i=0; i<int(f250WindowRawData_vec.size()); i++){
      
      int ROCID   = f250WindowRawData_vec[i]->rocid; 
      int SLOT    = f250WindowRawData_vec[i]->slot; 
      int CHANNEL = f250WindowRawData_vec[i]->channel;
      
      if (ROCID>=11 && ROCID<=23){
        japp->RootWriteLock();
        
        const vector<uint16_t> window = f250WindowRawData_vec[i]->samples;
        
        char name[50];
        sprintf(name,"%02i/%02i/%02i",ROCID,SLOT,CHANNEL);
        
        int m_x = 0;
        int m_y = 0;
        if (tranlation_map.count(name)>0){
          m_x = tranlation_map[TString(name)].first;
          m_y = tranlation_map[TString(name)].second;
        }
        
        for (int nsamples=1; nsamples<=maxSampleAvg; nsamples++){
          
          for (int nevents=1; nevents<=maxEvents; nevents++){
            
            // Calculate pedestal based on window raw data
            int nwindow = window.size();
            if (nsamples>nwindow) {
              cerr << "Problem!! Number of sample points must be less than window" << endl;
              continue;
            }
            double avg = 0;
            vector<double> m_ped_vec;
            for (int ii=0; ii<nsamples; ii++){
              avg += window[ii];
              m_ped_vec.push_back(window[ii]);
            }

            double m_pedestal = avg/double(nsamples);
            double m_rms      = TMath::RMS(m_ped_vec.size(),&m_ped_vec[0]);
            m_ped_vec.clear();

            TString key = TString(name)+"_"+StringUtilities::int2TString(nsamples)+"_"+StringUtilities::int2TString(nevents);
            
            ped_map[key].addPedestal(m_pedestal);
            ped_map[key].addPedestalUInt(m_pedestal);
            ped_map[key].setRun(loop->GetJEvent().GetRunNumber());
            ped_map[key].setEventnum(eventnumber);
            ped_map[key].setX(m_x);
            ped_map[key].setY(m_y);
            ped_map[key].setSampleAvg(nsamples);
            ped_map[key].setEventAvg(nevents);

            Pedestal::Pedestal ped = ped_map[key];
            
            if (ped.getCounter()==nevents) {  
              rocid = ped.getRocid();
              slot = ped.getSlot();
              channel = ped.getChannel();
              x = ped.getX();
              y = ped.getY();
              pedestal = ped.getAvgPedestal();
              pedestal_uint = ped.getAvgPedestalUInt();
              if (nevents==1) {
                rms = m_rms;
                rms_gaus = ped.getRmsPedestalFit();
              }
              else {
                rms = ped.getRmsPedestal();
                rms_gaus = ped.getRmsPedestalFit();
              }
              nSamples = ped.getSampleAvg();
              nEvents = ped.getEventAvg();
              eventnum = ped.getEventnum();
              run = ped.getRun();
              
              FCAL_Pedestal->Fill();
              // reset
              ped_map[key].clearVecPedestal();
              ped_map[key].clearVecPedestalUInt();
              
              // Fill for analysis of pedestal
              ped_ana[key].addPedestal(pedestal);
              ped_ana[key].addPedestalUInt(pedestal_uint);
              ped_ana[key].addRms(rms);
              ped_ana[key].setRun(run);
              ped_ana[key].setEventnum(eventnum);
              ped_ana[key].setX(x);
              ped_ana[key].setY(y);
              ped_ana[key].setSampleAvg(nSamples);
              ped_ana[key].setEventAvg(nEvents);
              
            }
            
          } // loop over nevents
        } // loop over nsamples
        
        
        japp->RootUnLock();
      }// rocid>=11 && rocid<=23 conditional
    } // End loop over f250WindowRawData_vec
  } // End get_windowraw_pedestals conditional

  // Additional Conditional, f250PulsePedestal
  
	return NOERROR;
}

//------------------
// erun
//------------------
jerror_t JEventProcessor_FCALPedestalPlugin::erun(void)
{
	// This is called whenever the run number changes, before it is
	// changed to give you a chance to clean up before processing
	// events from the next run number.
	
	return NOERROR;
}

//------------------
// fini
//------------------
jerror_t JEventProcessor_FCALPedestalPlugin::fini(void)
{
	// Called before program exit after event processing is finished.
	
	
	japp->RootWriteLock();

  ped_map.clear();
  
  for (std::map<TString, Pedestal>::const_iterator iter=ped_ana.begin(); iter!=ped_ana.end(); ++iter){
    TString key = iter->first;
    Pedestal::Pedestal ped = ped_ana[key];
    
    if (!ped.channelExists()) continue;
    
    rocid = ped.getRocid();
    slot = ped.getSlot();
    channel = ped.getChannel();
    x = ped.getX();
    y = ped.getY();
    avg_pedestal = ped.getAvgPedestal();
    rms_pedestal = ped.getRmsPedestal();
    rms_pedestal_gaus = ped.getRmsPedestalFit();
    nentries_pedestal = ped.getEntries();
    
    avg_pedestal_uint = ped.getAvgPedestalUInt();
    rms_pedestal_uint = ped.getRmsPedestalUInt();
    rms_pedestal_uint_gaus = ped.getRmsPedestalUIntFit();
    
    avg_rms = ped.getAvgRms();
    rms_rms = ped.getRmsRms();
    nentries_rms = ped.getRmsEntries();
    nSamples = ped.getSampleAvg();
    nEvents = ped.getEventAvg();
    eventnum = ped.getEventnum();
    run = ped.getRun();
    
    FCAL_Analysis->Fill();
    // reset
    ped_ana[key].clearVecPedestal();
    ped_ana[key].clearVecRms();
    
  }
	ped_ana.clear();
  japp->RootUnLock();
	
	
	return NOERROR;
}

