#include "Pedestal.h"

Pedestal::~Pedestal(){
  
}
Pedestal::Pedestal(){
  
}
Pedestal::Pedestal( TString name )
{
  m_exists = false;
  //cout << "Initializing Constructor, by name" << endl;
  m_name = name;
  m_counter = 0;
  // crate/slot/channel_sampleAvg
  vector<TString> parse_key = StringUtilities::parseTString(m_name,"_");
  vector<TString> parse_name = StringUtilities::parseTString(parse_key[0],"/");
  for (int i=0; i<int(parse_name.size()); i++){
    if (i==0) m_rocid   = StringUtilities::TString2int(parse_name[i]);
    if (i==1) m_slot    = StringUtilities::TString2int(parse_name[i]);
    if (i==2) m_channel = StringUtilities::TString2int(parse_name[i]);
  }
  if (int(parse_key.size())==2){
    m_nsample = StringUtilities::TString2int(parse_key[1]);
  }
}
Pedestal::Pedestal( int rocid, int slot, int channel, int nsample )
{
  m_exists = false;
  m_counter = 0;
  //cout << "Initializing Constructor, by roc/slot/chan" << endl;
  char name[50];
  sprintf(name, "%02i/%02i/%02i", m_rocid, m_slot, m_channel);
  m_name = name;
  m_rocid = rocid;
  m_slot = slot;
  m_channel = channel;
  m_nsample = nsample;
}

TString Pedestal::getName( void ){ return m_name;    }

int Pedestal::getRocid( void )   { return m_rocid;   }
int Pedestal::getSlot( void )    { return m_slot;    }
int Pedestal::getChannel( void ) { return m_channel; }
int Pedestal::getCounter( void ) { return m_counter; }

void Pedestal::addPedestal( double ped ) { 
  m_exists = true;
  m_counter++;
  m_vec_ped.push_back(ped);
}
double Pedestal::getAvgPedestal( void ){ 
  int n = int(m_vec_ped.size());
  if (n==0) return 0.0;
  return TMath::Mean(n,&m_vec_ped[0]);
}
double Pedestal::getRmsPedestal( void ){ 
  int n = int(m_vec_ped.size());
  if (n==0) return 0.0;
  return TMath::RMS(n,&m_vec_ped[0]);
}
void Pedestal::addRms( double rms ) { 
  m_exists = true;
  m_vec_rms.push_back(rms);
}
double Pedestal::getAvgRms( void ){ 
  int n = int(m_vec_rms.size());
  if (n==0) return 0.0;
  return TMath::Mean(n,&m_vec_rms[0]);
}
double Pedestal::getRmsRms( void ){ 
  int n = int(m_vec_rms.size());
  if (n==0) return 0.0;
  return TMath::RMS(n,&m_vec_rms[0]);
}

double Pedestal::getRmsPedestalFit( void ){
  // convert vector into histogram
  int n = int(m_vec_ped.size());
  if (n==0) return 0.0;
  TH1::AddDirectory(kFALSE);
  TH1F* hist = new TH1F("hist","hist",4096, 0, 4096);
  for (int i=0; i<n; i++){
    hist->Fill(m_vec_ped[i]);
  }
  hist->Fit("gaus","0Q");
  TF1* fit = hist->GetFunction("gaus"); // par 1=mean, par 2=sigma
  double answer = fit->GetParameter(2);
  delete hist;
  return answer;
}

void Pedestal::addPedestalUInt( uint32_t ped ){
  m_vec_ped_uint.push_back(ped);
}
uint32_t Pedestal::getAvgPedestalUInt( void ){
  int n = int(m_vec_ped_uint.size());
  if (n==0) return 0;
  return TMath::Mean(n,&m_vec_ped_uint[0]);
}
double Pedestal::getRmsPedestalUInt( void ){
  int n = int(m_vec_rms.size());
  if (n==0) return 0.0;
  return TMath::RMS(n,&m_vec_ped_uint[0]);
}
double Pedestal::getRmsPedestalUIntFit( void ){
  // convert vector into histogram
  int n = int(m_vec_ped_uint.size());
  if (n==0) return 0.0;
  TH1::AddDirectory(kFALSE);
  TH1I* hist = new TH1I("hist","hist",4096, 0, 4096);
  for (int i=0; i<n; i++){
    hist->Fill(int(m_vec_ped_uint[i]));
  }
  hist->Fit("gaus","0Q");
  TF1* fit = hist->GetFunction("gaus"); // par 1=mean, par 2=sigma
  double answer = fit->GetParameter(2);
  delete hist;
  return answer;
}


vector<double> Pedestal::getVecPedestal( void ){
  return m_vec_ped;
}
void Pedestal::clearVecPedestal( void ){
  m_counter = 0;
  m_vec_ped.clear();
}
void Pedestal::clearVecPedestalUInt( void ){
  m_vec_ped_uint.clear();
}
int Pedestal::getEntries( void ){
  return int(m_vec_ped.size());
}

vector<double> Pedestal::getVecRms( void ){
  return m_vec_rms;
}
void Pedestal::clearVecRms( void ){
  m_vec_rms.clear();
}
int Pedestal::getRmsEntries( void ){
  return int(m_vec_rms.size());
}

void Pedestal::setEventnum( int eventnum ){ m_eventnum = eventnum;}
int Pedestal::getEventnum( void ){return m_eventnum;}

void Pedestal::setRun( int run ){m_run = run;}
int Pedestal::getRun( void ){return m_run;}

void Pedestal::setSampleAvg( int nsample ){m_nsample = nsample;}
int Pedestal::getSampleAvg( void ){return m_nsample;}

void Pedestal::setEventAvg( int nevent ){m_nevent = nevent;}
int Pedestal::getEventAvg( void ){return m_nevent;}

void Pedestal::setX( int x ){m_x = x;}
int Pedestal::getX( void ){return m_x;}

void Pedestal::setY( int y ){m_y = y;}
int Pedestal::getY( void ){return m_y;}

bool Pedestal::channelExists( void ){ return m_exists;}
