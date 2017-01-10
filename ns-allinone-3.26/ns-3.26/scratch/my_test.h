#ifndef MY_TEST__H
#define MY_TEST__H

#define CHAR_MAX_LENGTH 1024
#define ENABLE_SHORT_GD false
#define MCS_NUMBER 0
#define IP_BASE "192.168.0.0"
#define SUBNET_MASK "255.255.0.0"
#define ARP_TIME 1.0
#define SERVER_START_TIME 0.0
#define CLIENT_START_TIME 1.0
#define SIMULATION_TIME 10.0
#define AP_INPUT_PATH "./input/ap/"
#define STA_INPUT_PATH "./input/sta/"
#define DEFF_UNIT 0
#define MEGA 1000000
#define PRINT_PERIOD 1.0
#define PAYLOADSIZE 1472   //udp



#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/olsr-helper.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

using namespace std;
using namespace ns3;


class PeriodApThroughput;
class PeriodStaThroughput;
class OutputGenerator;

void LinkTrace(map<uint16_t, Ptr<WifiPhy> > sub_phys,PeriodApThroughput* ap_thr);

void PrintThroughputInPeriod(map<unsigned int, PeriodApThroughput* > ap_thr,
								 map<unsigned int, PeriodStaThroughput* > sta_thr,
								 OutputGenerator* og,
								 map < unsigned int, vector <ApplicationContainer> > serverApp,
								 map<unsigned int, vector <unsigned int> > shortest_stas_of_ap
								 );

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
	InApInfo GetApInfo(uint32_t index);
	InStaInfo GetStaInfo(uint32_t index);
	void SetupInputFile(string path, bool ap_sta);  //ap_sta: ap(true) sta(false)
	void Parse();
	void CloseFile();
	void Clean();
	map<unsigned int, InApInfo>::iterator GetApBegin();
	map<unsigned int, InStaInfo>::iterator GetStaBegin();
	map<unsigned int, InApInfo>::iterator GetApEnd();
	map<unsigned int, InStaInfo>::iterator GetStaEnd();

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

#endif
