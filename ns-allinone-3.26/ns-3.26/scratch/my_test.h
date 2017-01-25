#ifndef MY_TEST__H
#define MY_TEST__H

#define CHAR_MAX_LENGTH 1024
#define ENABLE_SHORT_GD false
#define MCS_NUMBER 0
#define IP_BASE "192.168.0.0"
#define SUBNET_MASK "255.255.0.0"
#define ARP_TIME 2.0
#define SERVER_START_TIME 0.0
#define CLIENT_START_TIME 1.0
#define SIMULATION_TIME 10.0
//#define SIMULATION_TIME 3.0
#define AP_INPUT_PATH "./input/ap/"
#define STA_INPUT_PATH "./input/sta/"
#define DEFF_UNIT 0
#define MEGA 1000000.0
#define PRINT_PERIOD 1.0
#define PAYLOADSIZE 1472   //udp
#define WINDOW_SIZE_BEGIN 32.0
#define WINDOW_SIZE_MAX 5.0
#define LOOPCOUNT 100
#define CCATHRESHOLD -60.0
//#define EDTHRESHOLD -60.0
#define EDTHRESHOLD -60.0
#define TXPOWER 30.0

#define SLOT_TIME 10


#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/propagation-loss-model.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <cmath>

using namespace std;
using namespace ns3;


class PeriodApThroughput;
class PeriodStaThroughput;
class OutputGenerator;

ofstream trace_file;

void printcw(const uint32_t cw);
void printbackoff(const uint32_t cw);

void LinkTrace(map<uint16_t, Ptr<WifiPhy> > sub_phys,PeriodApThroughput* ap_thr);



struct InApInfo    //input information of ap
{
	double x;  //position x
	double y;  //position y
	uint16_t channel; //allocated channel
	uint32_t width;   //width   1~8
};

struct InStaInfo   //input information of station
{
	double x;  //position x
	double y;  //position y
	double traffic_demand; //traffic demand
};

struct OutApInfo    //output information of ap
{
	double now_through_packets;
	double avg_throughput; //average throughput
	double min_throughput;
	double max_throughput;
	double throughput_demand_ratio;
	map<unsigned int, double> idle_ratio;
};

struct OutStaInfo   //output information of station
{
	double now_through_packets;
	double avg_throughput;
	double min_throughput;
	double max_throughput;
	double served_demand_ratio;
};


class Parser
{
public:
	Parser();
	~Parser();

	void SetupInputFile(string path, bool ap_sta);  //ap_sta: ap(true) sta(false)
	void Parse();
	void CloseFile();
	void Clean();

	map<unsigned int, InApInfo> GetApInfos();
	map<unsigned int, InStaInfo> GetStaInfos();

private:
	ifstream input_file;
	map<unsigned int, InApInfo> ap_info;
	map<unsigned int, InStaInfo> sta_info;
	bool is_ap;
};


class OutputGenerator
{
public:
	OutputGenerator();
	~OutputGenerator();
	void CloseFile();
	void Print();
	void SetupOutPutFile(unsigned int test_number);
	void RecordApData(unsigned int index, OutApInfo outinfo);
	void RecordStaData(unsigned int index, OutStaInfo outinfo);
	void PrintLinkInfo(map<unsigned int, vector <unsigned int> > shortest_stas_of_ap);
	void RecordDistance(unsigned int index,double distance);
	void PrintDistance();
	void CleanDistMap();

private:
	map<unsigned int, double> dist_map;
	ofstream ap_output_file, sta_output_file;
	map<uint32_t,OutStaInfo> sta_info;
	map<uint32_t,OutApInfo> ap_info;
};

class PeriodApThroughput
{
public:
	PeriodApThroughput();
	~PeriodApThroughput();
	OutApInfo GetThroughput(uint32_t through_packets);

	void StateChange1(Time Start, Time duration, WifiPhy::State state);
	void StateChange2(Time Start, Time duration, WifiPhy::State state);
	void StateChange3(Time Start, Time duration, WifiPhy::State state);
	void StateChange4(Time Start, Time duration, WifiPhy::State state);
	void StateChange5(Time Start, Time duration, WifiPhy::State state);
	void StateChange6(Time Start, Time duration, WifiPhy::State state);
	void StateChange7(Time Start, Time duration, WifiPhy::State state);
	void StateChange8(Time Start, Time duration, WifiPhy::State state);

	void AddCh(uint16_t ch_num);
	void SetTotalDemand(double total);
	double GetTotalDemand();

	void ResetIdle();




private:
	uint32_t ex_through_packets;
	vector<uint16_t> ch_numbers;
	//map<uint16_t, ns3::Time> busy_time, latest_tx_begin, latest_rx_begin;
	map<uint16_t, ns3::Time> idle_time;
	ns3::Time last_print_time;
	uint32_t min_through_packets;
	uint32_t max_through_packets;
	uint32_t now_through_packets;
	uint32_t total_through_packets;
	double total_demand;

	void IdleTimeOccur(uint16_t ch_num,Time duration);
};

class PeriodStaThroughput
{
public:
	PeriodStaThroughput(double demand);
	~PeriodStaThroughput();
	OutStaInfo GetThroughput(uint32_t through_packets);
private:
	ns3::Time last_print_time;
	uint32_t ex_through_packets;
	uint32_t min_through_packets;
	uint32_t max_through_packets;
	uint32_t now_through_packets;
	uint32_t total_through_packets;
	double demand;
};


uint16_t actual_ch[] = {36, 40, 44, 48,
			52, 56, 60, 64,
			100, 104, 108, 112,
			116, 120, 124, 128,
			132, 136, 140, 144,
			149, 153, 157, 161, 165};



unsigned int WidestWidth(unsigned int primary_ch);

class NodeManager
{
public:
	enum ChannelManagerMapType
	{
		AP = 0,
		STA = 1,
	};

	enum LinkType
	{
		Up = false,
		Down = true,
	};

	NodeManager(uint32_t payloadSize,OutputGenerator* og);
	~NodeManager();
	void InputInfo(map<uint32_t, InApInfo> ap_info, map<uint32_t, InStaInfo> sta_info);
	void CalculateShortestAp();
	void SetTestEnv();
	void MakeAddressMap();
	void PrintThroughputInPeriod();
	void SetLinkType(LinkType type);


protected:

	OutputGenerator* og;
	unsigned int payloadSize;
	map<unsigned int, vector <unsigned int> > shortest_stas_of_ap;
	//map<int, vector <unsigned int> > using_ch_map;

	vector <unsigned int> all_aps;
	map<uint32_t, InStaInfo> sta_infos;
	map<uint32_t, InApInfo> ap_infos;

	map<unsigned int, NodeContainer> ap_nodes;
	map<unsigned int, Ptr<ChannelBondingManager> > ch_manager_map[2];
	map<unsigned int, NodeContainer> sta_nodes;

	map<unsigned int, NetDeviceContainer> ap_devs;
	map<unsigned int, NetDeviceContainer> sta_devs;

	map<unsigned int, PeriodApThroughput* > ap_thr;
	map<unsigned int, PeriodStaThroughput* > sta_thr;

	map< unsigned int, map <unsigned int,ApplicationContainer> > serverApp, clientApp;
	map< unsigned int, vector <ApplicationContainer> > echoserverApp, echoclientApp;

	Ptr<PropagationLossModel> PLM;

	map<Mac48Address,pair<unsigned int,bool>> address_map;  //pair : (index, ap) true-> ap false->sta



	LinkType link_type;
	Ptr<UniformRandomVariable> x;
private:
	void SetAppDownLink();
	void SetAppUpLink();


};


class DynamicChannelBonding : public NodeManager
{
public:
	DynamicChannelBonding(uint32_t payloadSize,OutputGenerator* og);
	~DynamicChannelBonding();
	void clean_map();
	void calculate_tau();
	void calculate_p();
	void calculate_variables();
	unsigned int get_width(unsigned int index);
	double GetNI(unsigned int i);
	double GetNIJ(unsigned int i, unsigned int j);
	vector<unsigned int> GetApIndexs();
	vector<unsigned int> GetApsUseChannel(unsigned int channel_number);
	vector<unsigned int> GetInterfereApIndexs(unsigned int index,unsigned int width);
	vector<unsigned int> GetSubChannels(unsigned int primary, unsigned int width);
	int UsingChannelsToBit(unsigned int index, unsigned int width);

	unsigned int GetPrimaryChannel(unsigned int index);
	double GetThroughputHat(unsigned int index, unsigned int width);
	double GetThroughput_demand_ratio(unsigned int index, unsigned int width);
	void InitTauAndP();
	void make_bit_map();
	double Get_ti(unsigned int index, unsigned int width);
	double Get_tsi(unsigned int index, unsigned int width);
	double Get_tfi(unsigned int index);
	vector<unsigned int> FindSubChannels(unsigned int ch_num);
	vector<double> Calculate_p_c(unsigned int index, unsigned int width);
	double GetAvgPathLoss(unsigned int index);
	double GetWhiteNoise(unsigned int width);
	double GetTauValue(unsigned int index, unsigned int channel, unsigned int width);


private:
	map<unsigned int, map<unsigned int, map<Mac48Address, bool> > > link_map; //link_map[ap_index][channel_number][macaddress] = true/false my_sta_station
	map<unsigned int, map<Mac48Address, bool> > total_link_map;  //total_link_map[ap_index][macaddress] = my_station; (sum of all signal in channels)
	map<unsigned int, map<unsigned int, double> > tau;
	map<unsigned int, double> p;
	map<unsigned int, map<unsigned int, double>> matrix_F;  //unit(mwatt)
	std::map<uint16_t, ChannelInfo > channel_map;
	std::map<uint16_t, unsigned int> channel_bit_map;
	map<unsigned int, Time>  SIFS, RTS_Time, CTS_Time, ACK_Time;  //map[width] = time;
	Time DIFS, Slot;
	double min_sinr;

	enum ChannelToBit
	{
		CH_36 = 0b1000000000000000000000000,
		CH_40 = 0b0100000000000000000000000,
		CH_44 = 0b0010000000000000000000000,
		CH_48 = 0b0001000000000000000000000,
		CH_52 = 0b0000100000000000000000000,
		CH_56 = 0b0000010000000000000000000,
		CH_60 = 0b0000001000000000000000000,
		CH_64 = 0b0000000100000000000000000,

		CH_100 = 0b0000000010000000000000000,
		CH_104 = 0b0000000001000000000000000,
		CH_108 = 0b0000000000100000000000000,
		CH_112 = 0b0000000000010000000000000,
		CH_116 = 0b0000000000001000000000000,
		CH_120 = 0b0000000000000100000000000,
		CH_124 = 0b0000000000000010000000000,
		CH_128 = 0b0000000000000001000000000,

		CH_132 = 0b0000000000000000100000000,
		CH_136 = 0b0000000000000000010000000,
		CH_140 = 0b0000000000000000001000000,
		CH_144 = 0b0000000000000000000100000,

		CH_149 = 0b0000000000000000000010000,
		CH_153 = 0b0000000000000000000001000,
		CH_157 = 0b0000000000000000000000100,
		CH_161 = 0b0000000000000000000000010,

		CH_165 = 0b0000000000000000000000001,
	};

	int fill_bit_map(uint16_t ch_num);  //filling and search bit
	void SetApWidth(unsigned int ap_index, unsigned int width);
	void InitTimeValues();
	void InitMatrix();

};

#endif
