// $Id$
//
//    File: JEventProcessor_FCALPedestalPlugin.h
// Created: Thu May 14 12:49:44 EDT 2015
// Creator: manlara (on Linux ifarm1401 2.6.32-431.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_FCALPedestalPlugin_
#define _JEventProcessor_FCALPedestalPlugin_

#include <JANA/JEventProcessor.h>

#include <stdint.h>
#include <stdlib.h>
#include <map>
#include <utility>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>

#include "StringUtilities.h"

#include "TROOT.h"
#include <TString.h>
#include <TH1F.h>
#include <TF1.h>
#include "TMinuit.h"
#include <TVirtualFitter.h>
#include <TTree.h>
#include <TObjString.h>
#include <TObjArray.h>

#include "Pedestal.h"



class JEventProcessor_FCALPedestalPlugin:public jana::JEventProcessor{
	public:
		JEventProcessor_FCALPedestalPlugin();
		~JEventProcessor_FCALPedestalPlugin();
		const char* className(void){return "JEventProcessor_FCALPedestalPlugin";}
		
		TTree* FCAL_Pedestal;
    TTree* FCAL_Analysis;
		
		const static int maxEvents = 20;
    const static int maxSampleAvg = 20;
    //int nsamples[20];
    std::map<TString, Pedestal> ped_map;
    std::map<TString, Pedestal> ped_ana;
    
    // key = rocid/slot/channel
    // value = (x,y)
    std::map<TString, pair<int,int> > tranlation_map;
		
		uint32_t rocid;
    uint32_t slot;
    uint32_t channel;
    int x;
    int y;
    double pedestal;
    uint32_t pedestal_uint;
    double rms;
    double rms_gaus;
    uint32_t nSamples;
    uint32_t nEvents;
    uint32_t eventnum;
    uint32_t run;
    
    double avg_pedestal;
    double avg_rms;
    double rms_pedestal;
    double rms_rms;
    double avg_pedestal_uint;
    double rms_pedestal_uint;
    double rms_pedestal_uint_gaus;
    double rms_pedestal_gaus;
    uint32_t nentries_rms;
    uint32_t nentries_pedestal;
    
    

	private:
		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, int eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
		
};

#endif // _JEventProcessor_FCALPedestalPlugin_

