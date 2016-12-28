#ifndef MY_TEST__H
#define MY_TEST__H

#define CHAR_MAX_LENGTH 1024
#define ENABLE_SHORT_GD false
#define MCS_NUMBER 0
#define IP_BASE "192.168.0.0"
#define SUBNET_MASK "255.255.0.0"
#define ARP_TIME 10.0
#define SERVER_START_TIME 0.0
#define CLIENT_START_TIME 1.0
#define SIMULATION_TIME 1.0
#define AP_INPUT_PATH "./input/ap/"
#define STA_INPUT_PATH "./input/sta/"
#define DEFF_UNIT 0
#define MEGA 1000000


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
	double avg_throughput; //average throughput
	double min_throughput;
	double max_throughput;
	vector<double> tx_idle_ratio;
};

struct OutStaInfo   //output information of station
{
	double avg_throughput;
	double min_throughput;
	double max_throughput;
	vector<double> served_demand_ratio;
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
	void Clean();
	void Print();
	void SetupOutPutFile(string path);
	void RecordData();

private:
	ofstream output_file;
	map<uint32_t,OutStaInfo> sta_info;
	map<uint32_t,OutApInfo> ap_info;
};

uint16_t actual_ch[] = {36, 40, 44, 48,
							52, 56, 60, 64,
							100, 104, 108, 112,
							116, 120, 124, 128,
							132, 136, 140, 144,
							149, 153, 157, 161, 165};


#endif
