#if !defined(PEDESTAL_H)
#define PEDESTAL_H

#include <stdint.h>
#include <stdlib.h>
#include <map>
#include <vector>

#include "StringUtilities.h"

#include "TROOT.h"
#include <TString.h>
#include "TMath.h"
#include <TH1F.h>
#include <TH1I.h>
#include <TF1.h>
#include "TMinuit.h"
#include <TVirtualFitter.h>
#include <TTree.h>
#include <TObjString.h>
#include <TObjArray.h>


class Pedestal{
  
  public:
    // Construct class roc/slot/channel
    Pedestal();
    Pedestal(TString name);
    Pedestal(int rocid, int slot, int channel, int nsample );
    ~Pedestal();
    
    bool channelExists( void );
    
    TString getName( void );
    int getRocid( void );
    int getSlot( void );
    int getChannel( void );
    
    int getCounter( void );
    
    int getEventnum( void );
    void setEventnum( int eventnum );
    
    int getRun( void );
    void setRun( int run );
    
    int getX( void );
    void setX( int x );
    
    int getY( void );
    void setY( int y );
    
    int getSampleAvg ( void );
    void setSampleAvg ( int nsample );
    
    int getEventAvg ( void );
    void setEventAvg ( int nevent );
    
    void addPedestal( double ped );
    double getAvgPedestal( void );
    double getRmsPedestal( void );
    double getRmsPedestalFit( void );
    
    void addPedestalUInt( uint32_t ped );
    uint32_t getAvgPedestalUInt( void );
    double getRmsPedestalUInt( void );
    double getRmsPedestalUIntFit( void );
    
    void addRms( double rms );
    double getAvgRms( void );
    double getRmsRms( void );
    
    vector<double> getVecPedestal( void );
    void clearVecPedestal( void );
    void clearVecPedestalUInt( void );
    int getEntries( void );
    
    vector<double> getVecRms( void );
    void clearVecRms( void );
    int getRmsEntries( void );
    
    
  private:
    TString m_name;
    int m_rocid, m_slot, m_channel, m_counter, m_eventnum, m_run, m_nsample, m_nevent;
    bool m_exists;
    int m_x, m_y;
    double m_ped;
    //TH1F* m_hist_ped;
    vector<double> m_vec_ped;
    vector<double> m_vec_rms;
    vector<uint32_t> m_vec_ped_uint;
    vector<double> m_vec_rms_uint;
  
};

#endif



