#include "my_test.h"
#include "ns3/random-variable-stream.h"



NS_LOG_COMPONENT_DEFINE ("my-wifi-test");




int main (int argc, char *argv[])
{
	Parser parser;
	OutputGenerator* og = new OutputGenerator();
	unsigned int test_number = 0;

	CommandLine cmd;
	cmd.AddValue("test_number" , "insert number of input file", test_number);
	cmd.Parse(argc,argv);

	uint32_t payloadSize = 1472;
	//uint32_t payloadSize = 1448;
	//Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (payloadSize));
	ostringstream oss;
	oss.setf(ios::fixed,ios::floatfield);

	/*
	 * parse ap input
	 */
	oss<<AP_INPUT_PATH<<test_number;
	parser.SetupInputFile(oss.str(),true);
	oss.str("");oss.clear();
	parser.Parse();
	parser.CloseFile();

	og->SetupOutPutFile(test_number);
	/*
	 * parse sta input
	 */
	oss<<STA_INPUT_PATH<<test_number;
	parser.SetupInputFile(oss.str(),false);
	oss.str("");oss.clear();
	parser.Parse();
	parser.CloseFile();

	double shortest_sq_dist, sq_dist;
	unsigned int shortest_ap;

	Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
	//

	map<unsigned int, vector <unsigned int> > shortest_stas_of_ap; //shortest_stas_of_ap[a] = list of STAs which is allocated with AP a

	/*for(map<unsigned int,InApInfo>::iterator i = parser.GetApBegin();
		i != parser.GetApEnd();
		++i)
	{
		shortest_stas_of_ap[i->first].reserve(1);
	}*/

	for(map<unsigned int,InStaInfo>::iterator i = parser.GetStaBegin();
		i != parser.GetStaEnd();
		++i)
	{
		shortest_sq_dist = numeric_limits<double>::max();
		InStaInfo sta_info = i->second;
		for(map<unsigned int,InApInfo>::iterator j = parser.GetApBegin();
			j != parser.GetApEnd();
			++j)
		{
			InApInfo ap_info = j->second;
			sq_dist = (sta_info.x - ap_info.x) * (sta_info.x - ap_info.x);
			sq_dist += (sta_info.y - ap_info.y) * (sta_info.y - ap_info.y);

			if(shortest_sq_dist > sq_dist)
			{
				shortest_sq_dist = sq_dist;
				shortest_ap = j->first;
			}
		}
		og->RecordDistance(i->first,sqrt(shortest_sq_dist));
		shortest_stas_of_ap[shortest_ap].push_back(i->first);
	}

	og->PrintLinkInfo(shortest_stas_of_ap);
	og->PrintDistance();
	og->CleanDistMap();

	map<unsigned int, NodeContainer> ap_nodes;
	map<unsigned int, NodeContainer> sta_nodes;

	map<unsigned int, NetDeviceContainer> ap_devs;
	map<unsigned int, NetDeviceContainer> sta_devs;

	map<unsigned int, PeriodApThroughput* > ap_thr;
	map<unsigned int, PeriodStaThroughput* > sta_thr;


	YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
	//YansWifiChannelHelper channel;
	YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();

	//channel.AddPropagationLoss("ns3::FriisPropagationLossModel" );
	//channel.AddPropagationLoss("ns3::FixedRssLossModel", "Rss", DoubleValue(100.0));
	//channel.AddPropagationLoss("ns3::RangePropagationLossModel", "MaxRange", DoubleValue(250.0));

	//channel.AddPropagationLoss("ns3::LogDistancePropagationLossModel");
	//channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");

	phy.SetChannel (channel.Create ());
	phy.Set ("ShortGuardEnabled", BooleanValue (ENABLE_SHORT_GD));

	WifiHelper wifi;
	wifi.SetStandard (WIFI_PHY_STANDARD_80211ac);
	WifiMacHelper mac;


	oss.str("");oss.clear();
	oss << "VhtMcs" << MCS_NUMBER;
	/*wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", StringValue (oss.str ()),
								"ControlMode", StringValue (oss.str ()),
								"RtsCtsThreshold", UintegerValue(100));*/

	wifi.SetRemoteStationManager ("ns3::MinstrelHtWifiManager", "RtsCtsThreshold", UintegerValue(100));
	//wifi.SetRemoteStationManager ("ns3::OnoeWifiManager", "RtsCtsThreshold", UintegerValue(100));


	//Ssid ssid = Ssid ("ns3-80211ac");
	Ssid ssid;

	Ptr<RegularWifiMac> m_mac;

	MobilityHelper mobility;
	Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();


	for(map<unsigned int, vector <unsigned int>>::iterator i = shortest_stas_of_ap.begin();
		i != shortest_stas_of_ap.end();
		++i)
	{
		oss.str("");oss.clear();

		oss<<"ns3-80211ac-"<<i->first;
		ssid = oss.str();
		ap_nodes[i->first] = NodeContainer();
		ap_nodes[i->first].Create(1);  //create 1 node

		mac.SetType ("ns3::ApWifiMac",
					   "Ssid", SsidValue (ssid),
					   "EnableBeaconJitter", BooleanValue(true)
					   );

		ap_devs[i->first] = wifi.Install(phy, mac, ap_nodes[i->first]);


		m_mac = DynamicCast<RegularWifiMac> (DynamicCast<WifiNetDevice>(ap_devs[i->first].Get(0))->GetMac());
		Ptr<MacLow> m_low = m_mac->GetLow();
		m_low->EnableChannelBonding();


		InApInfo ap_info = parser.GetApInfo(i->first);
		m_low->SetChannelManager(phy, actual_ch[ap_info.channel], ap_info.width, WIFI_PHY_STANDARD_80211ac);


		ap_thr[i->first] = new PeriodApThroughput();

		LinkTrace(m_low->GetChannelManager()->GetPhys(), ap_thr[i->first]);

		positionAlloc->Add(Vector(ap_info.x, ap_info.y, 0) );
		double total = 0.0;
		for(vector <unsigned int>::iterator j = i->second.begin();
			j != i->second.end();
			j++)
		{
			sta_nodes[*j] = NodeContainer();
			sta_nodes[*j].Create(1);

			mac.SetType ("ns3::StaWifiMac",
						   "Ssid", SsidValue (ssid));

			sta_devs[*j] = wifi.Install(phy, mac, sta_nodes[*j]);


			m_mac = DynamicCast<RegularWifiMac> (DynamicCast<WifiNetDevice>(sta_devs[*j].Get(0))->GetMac());
			m_low = m_mac->GetLow();
			m_low->EnableChannelBonding();

			m_low->SetChannelManager(phy, actual_ch[ap_info.channel], ap_info.width, WIFI_PHY_STANDARD_80211ac);


			InStaInfo sta_info = parser.GetStaInfo(*j);

			positionAlloc->Add(Vector (sta_info.x, sta_info.y, 0));

			sta_thr[*j] = new PeriodStaThroughput(sta_info.traffic_demand);
			total += sta_info.traffic_demand;
		}

		ap_thr[i->first]->SetTotalDemand(total);
	}

	mobility.SetPositionAllocator (positionAlloc);
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

	InternetStackHelper stack;

	Ipv4AddressHelper address;
	address.SetBase (IP_BASE, SUBNET_MASK);

	map < unsigned int, vector <ApplicationContainer> > serverApp, clientApp;
	map < unsigned int, vector <ApplicationContainer> > echoserverApp, echoclientApp;
	//map < unsigned int, vector <ApplicationContainer> > App, sinkApp;

	for(map<unsigned int, vector <unsigned int>>::iterator i = shortest_stas_of_ap.begin();
		i != shortest_stas_of_ap.end();
		++i)
	{
		mobility.Install (ap_nodes[i->first]);
		stack.Install (ap_nodes[i->first]);
		Ipv4InterfaceContainer apNodeInterface = address.Assign(ap_devs[i->first]);

		//cout<<"ap "<<i->first<<" : address ("<<apNodeInterface.GetAddress(0)<<") | ";
		unsigned int port_num = 5000;
		unsigned int echo_port = 9;

		for(vector <unsigned int>::iterator j = i->second.begin();
			j != i->second.end();
			++j)
		{
			double diff = x->GetValue(0,1);
			InStaInfo stainfo = parser.GetStaInfo(*j);
			mobility.Install (sta_nodes[*j]);
			stack.Install (sta_nodes[*j]);

			Ipv4InterfaceContainer staNodeInterface = address.Assign(sta_devs[*j]);
			//cout<<"sta "<<*j<<" : address ("<<staNodeInterface.GetAddress(0)<<") | ";

			UdpServerHelper myServer (port_num);

			ApplicationContainer back;  //for find end

			back =  myServer.Install(sta_nodes[*j]);
			back.Start (Seconds(ARP_TIME + SERVER_START_TIME));
			back.Stop (Seconds(ARP_TIME + CLIENT_START_TIME + SIMULATION_TIME ));

			serverApp[*j].push_back(back) ;   //downlink ap -> stas


			UdpClientHelper myClient (staNodeInterface.GetAddress (0), port_num);
			myClient.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
			//myClient.SetAttribute ("Interval", TimeValue (Time ("0.00001"))); //packets/s

			double interval = (double) payloadSize * 8.0 / (stainfo.traffic_demand * MEGA);

			//cout<<interval<<endl;
			oss.str("");oss.clear();
			oss<<interval;
			myClient.SetAttribute("Interval", TimeValue (Time ( oss.str())));


			myClient.SetAttribute ("PacketSize", UintegerValue (payloadSize));
			//cout<<i->first<<endl;

			back = myClient.Install(ap_nodes[i->first]);

			back.Start( Seconds(ARP_TIME + CLIENT_START_TIME));
			back.Stop( Seconds (ARP_TIME + CLIENT_START_TIME + SIMULATION_TIME) );
			clientApp[i->first].push_back(back);


			UdpEchoServerHelper echoServer (echo_port);
			UdpEchoClientHelper echoClient (staNodeInterface.GetAddress(0), echo_port);
			echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
			echoClient.SetAttribute ("Interval", TimeValue (Seconds (0.001)));
			echoClient.SetAttribute ("PacketSize", UintegerValue (1024));


			back = echoServer.Install (sta_nodes[*j]) ;
			back.Start(Seconds(0.0));
			back.Stop(Seconds(ARP_TIME));

			echoserverApp[*j].push_back(back);

			back = echoClient.Install (ap_nodes[i->first]);
			back.Start(Seconds(diff + 1.0));
			//back.Start(Seconds(1.0));
			back.Stop(Seconds(ARP_TIME));

			echoclientApp[i->first].push_back(back);


			++port_num;
			++echo_port;
		}
		//cout<<endl;
	}

	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
	/*OlsrHelper olsr;

	Ipv4StaticRoutingHelper staticRouting;

	Ipv4ListRoutingHelper list;
	list.Add(staticRouting, 0);
	list.Add(olsr,10);
	stack.SetRoutingHelper(list);*/


	if(SIMULATION_TIME < PRINT_PERIOD)
	{

	}
	else
	{
		Simulator::Schedule(Seconds (ARP_TIME + CLIENT_START_TIME + PRINT_PERIOD), &PrintThroughputInPeriod, ap_thr, sta_thr, og, serverApp, shortest_stas_of_ap);
	}

	for(map<unsigned int, PeriodApThroughput* >::iterator i = ap_thr.begin() ;
		i != ap_thr.end();
		++i)
	{
		Simulator::Schedule(Seconds (ARP_TIME + CLIENT_START_TIME), &PeriodApThroughput::ResetIdle,i->second);
		i->second->ResetIdle();
	}



	Simulator::Stop (Seconds (ARP_TIME + CLIENT_START_TIME + SIMULATION_TIME));
	Simulator::Run ();
	Simulator::Destroy ();

	PrintThroughputInPeriod(ap_thr, sta_thr, og, serverApp, shortest_stas_of_ap);

	og->CloseFile();

	return 0;
}

Parser::Parser()
{
	is_ap = false;
}

Parser::~Parser()
{
	if(input_file.is_open())
		input_file.close();
}

InApInfo Parser::GetApInfo(uint32_t index)
{
	return ap_info[index];
}

InStaInfo Parser::GetStaInfo(uint32_t index)
{
	return sta_info[index];
}

void Parser::SetupInputFile(string path, bool ap_sta)
{
	input_file.open(path.c_str(),ios::in);
	is_ap = ap_sta;
}

void Parser::Parse()
{
	string str;
	istringstream iss;
	char line[CHAR_MAX_LENGTH];
	unsigned int temp;
	for(;!input_file.eof();)
	{
		input_file.getline(line,CHAR_MAX_LENGTH);

		/*
		 * convert to string
		 */
		str = "";
		str += line;
		/*
		 * remove comment
		 */
		while(!str.empty() && str.at(0) == ' ')
			str = str.substr(1);
		temp = str.find('#');

		if (temp == 0)
			continue;

		if(temp < str.size())
		{
			str.resize(temp);
		}

		while(!str.empty() && str.at(0) == ' ')
			str = str.substr(1);


		unsigned int index;
		if(is_ap)
		{
			InApInfo input_info;

			for(int i =0;i<5;++i)
			{
				temp = str.find(',');
				iss.str("");iss.clear();
				iss.str(str.substr(0, temp));
				str = str.substr(temp+1);

				while(!str.empty() && str.at(0) == ' ')
					str = str.substr(1);


				switch(i)
				{
				case 0:
					iss>>index;
					break;
				case 1:
					iss>>input_info.x;
					break;
				case 2:
					iss>>input_info.y;
					break;
				case 3:
					iss>>input_info.channel;
					break;
				default:
					iss>>input_info.width;
				}
			}
			if(input_info.width != 0)
				ap_info[index] = input_info;
		}
		else
		{
			InStaInfo input_info;
			for(int i =0;i<4;++i)
			{
				temp = str.find(',');
				iss.str("");iss.clear();
				iss.str(str.substr(0, temp));
				str = str.substr(temp+1);
				while(!str.empty() && str.at(0) == ' ')
					str = str.substr(1);


				switch(i)
				{
				case 0:
					iss>>index;
					break;
				case 1:
					iss>>input_info.x;
					break;
				case 2:
					iss>>input_info.y;
					break;
				default:
					iss>>input_info.traffic_demand;
				}
			}
			sta_info[index] = input_info;
		}
	}
}

void Parser::CloseFile()
{
	if(input_file.is_open())
		input_file.close();
}

void Parser::Clean()
{
	CloseFile();

	ap_info.clear();
	sta_info.clear();
}

map<unsigned int, InApInfo>::iterator Parser::GetApBegin()
{
	return ap_info.begin();
}
map<unsigned int, InStaInfo>::iterator Parser::GetStaBegin()
{
	return sta_info.begin();
}

map<unsigned int, InApInfo>::iterator Parser::GetApEnd()
{
	return ap_info.end();
}
map<unsigned int, InStaInfo>::iterator Parser::GetStaEnd()
{
	return sta_info.end();
}

OutputGenerator::OutputGenerator()
{
}

OutputGenerator::~OutputGenerator()
{
}

void OutputGenerator::CloseFile()
{
	if(ap_output_file.is_open())
		ap_output_file.close();

	if(sta_output_file.is_open())
		sta_output_file.close();
}

void OutputGenerator::Print()
{
	Time now = Simulator::Now();
	if(now == Time("0.0"))
		now = Seconds (ARP_TIME + CLIENT_START_TIME + SIMULATION_TIME);

	ap_output_file<<"Time : "<<now.GetSeconds()<<endl;
	sta_output_file<<"Time : "<<now.GetSeconds()<<endl;

	for(map<unsigned int, OutApInfo>::iterator i = ap_info.begin();
		i != ap_info.end();
		++i)
	{
		ap_output_file<<"index : "<<i->first<<endl;
		ap_output_file<<"throughput : "<<i->second.now_through_packets<<endl;
		ap_output_file<<"average throughput : "<<i->second.avg_throughput<<endl;
		ap_output_file<<"minimum throughput : "<<i->second.min_throughput<<endl;
		ap_output_file<<"maximum throughput : "<<i->second.max_throughput<<endl;
		ap_output_file<<"total throughput / demand(%) : "<<i->second.throughput_demand_ratio<<endl;

		ap_output_file<<"% of the idle time over transmission time :"<<endl;

		for(map<unsigned int, double>::iterator j = i->second.idle_ratio.begin();
			j != i->second.idle_ratio.end();
			++j)
		{
			 ap_output_file<<"Channel "<<j->first<<" : "<<j->second<<endl;
		}
		ap_output_file<<endl;
	}

	for(map<unsigned int, OutStaInfo>::iterator i = sta_info.begin(); i != sta_info.end(); ++i)
	{
		sta_output_file<<"index : "<<i->first<<endl;
		sta_output_file<<"throughput : "<<i->second.now_through_packets<<endl;
		sta_output_file<<"average throughput : "<<i->second.avg_throughput<<endl;
		sta_output_file<<"minimum throughput : "<<i->second.min_throughput<<endl;
		sta_output_file<<"maximum throughput : "<<i->second.max_throughput<<endl;
		sta_output_file<<"throughput/demand(%) : "<<i->second.served_demand_ratio<<endl;
		sta_output_file<<endl;
	}

	ap_output_file<<"----------------------------------------"<<endl;
	sta_output_file<<"----------------------------------------"<<endl;
}

void OutputGenerator::SetupOutPutFile(unsigned int test_number)
{
	ostringstream oss;
	oss<<"./output/ap/"<<test_number;
	ap_output_file.open(oss.str().c_str());
	oss.str("");oss.clear();
	oss<<"./output/sta/"<<test_number;
	sta_output_file.open(oss.str().c_str());
}

void OutputGenerator::RecordApData(unsigned int index, OutApInfo outinfo)
{
	ap_info[index] = outinfo;
}

void OutputGenerator::RecordStaData(unsigned int index, OutStaInfo outinfo)
{
	sta_info[index] = outinfo;
}

void OutputGenerator::PrintLinkInfo(map<unsigned int, vector <unsigned int> > shortest_stas_of_ap)
{
	ap_output_file<<"-------------Link Information----------------------"<<endl;
	for(map<unsigned int,vector <unsigned int>>::iterator i = shortest_stas_of_ap.begin();
		i != shortest_stas_of_ap.end();
		++i)
	{
		ap_output_file<<"# AP "<<i->first<<" : ";
		for(vector <unsigned int>::iterator j = i->second.begin(); j != i->second.end();j++)
		{
			ap_output_file<<"STA "<<*j<<" | ";
		}
		ap_output_file<<endl;
	}
	ap_output_file<<"-----------------------------------------------\n";
}

void OutputGenerator::RecordDistance(unsigned int index,double distance)
{
	dist_map[index] = distance;
}

void OutputGenerator::PrintDistance()
{
	sta_output_file<<"---------------------distance information----------------------\n";
	for(map<unsigned int, double>::iterator i = dist_map.begin();
		i != dist_map.end();
		++i)
	{
		sta_output_file<<"# STA "<<i->first<<" : "<<i->second<<" m\n";
	}
	sta_output_file<<"----------------------------------------------------------\n";
}

void OutputGenerator::CleanDistMap()
{
	dist_map.clear();
}


void LinkTrace(map<uint16_t, Ptr<WifiPhy> > sub_phys,PeriodApThroughput* ap_thr)
{
	int num=0;
	void (PeriodApThroughput::*func)(Time, Time, WifiPhy::State);

	for(map<uint16_t, Ptr<WifiPhy> >::iterator sub_phy = sub_phys.begin();
		sub_phy != sub_phys.end();
		++sub_phy)
	{
		++num;
		switch(num)
		{
		case 1:
			func = &PeriodApThroughput::StateChange1;
			break;

		case 2:
			func = &PeriodApThroughput::StateChange2;
			break;

		case 3:
			func = &PeriodApThroughput::StateChange3;
			break;

		case 4:
			func = &PeriodApThroughput::StateChange4;
			break;

		case 5:
			func = &PeriodApThroughput::StateChange5;
			break;

		case 6:
			func = &PeriodApThroughput::StateChange6;
			break;

		case 7:
			func = &PeriodApThroughput::StateChange7;
			break;

		default:
			func = &PeriodApThroughput::StateChange8;
		}

		ap_thr->AddCh(sub_phy->first);


		PointerValue testing;

		sub_phy->second->GetAttribute("State", testing);
		testing.GetObject()->TraceConnectWithoutContext("State",MakeCallback(func, ap_thr));
	}
}

void PrintThroughputInPeriod(map<unsigned int, PeriodApThroughput* > ap_thr,
								 map<unsigned int, PeriodStaThroughput* > sta_thr,
								 OutputGenerator* og,
								 map < unsigned int, vector <ApplicationContainer> > serverApp,
								 map<unsigned int, vector <unsigned int> > shortest_stas_of_ap
								 )
{
	for(map<unsigned int, vector <unsigned int> >::iterator i = shortest_stas_of_ap.begin();
		i != shortest_stas_of_ap.end();
		++i)
	{
		uint32_t total_through_packet = 0;

		for(vector <unsigned int>::iterator j = i->second.begin();
			j != i->second.end();
			++j)
		{
			uint32_t through_packet = DynamicCast<UdpServer> (serverApp[*j][0].Get(0))->GetReceived();
			og->RecordStaData(*j,sta_thr[*j]->GetThroughput(through_packet));
			total_through_packet += through_packet;
		}

		og->RecordApData(i->first, ap_thr[i->first]->GetThroughput(total_through_packet));
	}

	og->Print();

	if(Simulator::Now() + Seconds(PRINT_PERIOD) < Seconds (ARP_TIME + CLIENT_START_TIME + SIMULATION_TIME))
		Simulator::Schedule(Seconds(PRINT_PERIOD), &PrintThroughputInPeriod, ap_thr, sta_thr, og, serverApp, shortest_stas_of_ap);
}

PeriodApThroughput::PeriodApThroughput()
{
	ex_through_packets = 0;
	ch_numbers.reserve(8);
	last_print_time = Seconds(ARP_TIME + CLIENT_START_TIME);

	min_through_packets = numeric_limits<uint32_t>::max();
	max_through_packets = numeric_limits<uint32_t>::min();
	now_through_packets = 0;
	total_through_packets = 0;
	total_demand = 0.0;
}

PeriodApThroughput::~PeriodApThroughput()
{
}

OutApInfo PeriodApThroughput::GetThroughput(uint32_t through_packets)
{
	OutApInfo result;

	Time now = Simulator::Now();
	if(now == Time("0.0"))
		now = Seconds (ARP_TIME + CLIENT_START_TIME + SIMULATION_TIME);

	Time period = now - last_print_time;
	last_print_time = now;

	total_through_packets = through_packets;
	now_through_packets = total_through_packets - ex_through_packets;
	ex_through_packets = through_packets;

	if(now_through_packets < min_through_packets)
		min_through_packets = now_through_packets;

	if(now_through_packets > max_through_packets)
		max_through_packets = now_through_packets;

	result.now_through_packets = now_through_packets * PAYLOADSIZE * 8 / (MEGA * period.GetSeconds());
	result.avg_throughput = total_through_packets * PAYLOADSIZE * 8 / (MEGA * (now - Seconds(ARP_TIME + CLIENT_START_TIME)).GetSeconds()); //Mbit/s
	result.min_throughput = min_through_packets * PAYLOADSIZE * 8 / (MEGA * period.GetSeconds());
	result.max_throughput = max_through_packets * PAYLOADSIZE * 8 / (MEGA * period.GetSeconds());

	//cout<<"period : "<<period.GetNanoSeconds()<<endl;
	for(map<uint16_t, ns3::Time>::iterator i = idle_time.begin();
		i != idle_time.end();
		++i)
	{
		//cout<<"ch "<<i->first<<" : time : "<<i->second.GetNanoSeconds()<<endl;
		result.idle_ratio[i->first] = (1.0 - (double) i->second.GetNanoSeconds() / (double) period.GetNanoSeconds() )  * 100.0;
		i->second = Time("0.0");
	}

	result.throughput_demand_ratio = result.avg_throughput / total_demand * 100.0;

	return result;
}

void PeriodApThroughput::StateChange1(Time Start, Time duration, WifiPhy::State state)
{
	//if(state == WifiPhy::State::IDLE)
	if(state == WifiPhy::State::TX ||
		state == WifiPhy::State::RX)
	{
		IdleTimeOccur(ch_numbers[0], duration);
	}
}
void PeriodApThroughput::StateChange2(Time Start, Time duration, WifiPhy::State state)
{
	//if(state == WifiPhy::State::IDLE)
	if(state == WifiPhy::State::TX ||
		state == WifiPhy::State::RX)
	{
		IdleTimeOccur(ch_numbers[1], duration);
	}
}
void PeriodApThroughput::StateChange3(Time Start, Time duration, WifiPhy::State state)
{
	//if(state == WifiPhy::State::IDLE)
	if(state == WifiPhy::State::TX ||
		state == WifiPhy::State::RX)
	{
		IdleTimeOccur(ch_numbers[2], duration);
	}
}
void PeriodApThroughput::StateChange4(Time Start, Time duration, WifiPhy::State state)
{
	//if(state == WifiPhy::State::IDLE)
	if(state == WifiPhy::State::TX ||
		state == WifiPhy::State::RX)
	{
		IdleTimeOccur(ch_numbers[3], duration);
	}
}
void PeriodApThroughput::StateChange5(Time Start, Time duration, WifiPhy::State state)
{
	//if(state == WifiPhy::State::IDLE)
	if(state == WifiPhy::State::TX ||
		state == WifiPhy::State::RX)
	{
		IdleTimeOccur(ch_numbers[4], duration);
	}
}
void PeriodApThroughput::StateChange6(Time Start, Time duration, WifiPhy::State state)
{
	//if(state == WifiPhy::State::IDLE)
	if(state == WifiPhy::State::TX ||
		state == WifiPhy::State::RX)
	{
		IdleTimeOccur(ch_numbers[5], duration);
	}
}
void PeriodApThroughput::StateChange7(Time Start, Time duration, WifiPhy::State state)
{
	//if(state == WifiPhy::State::IDLE)
	if(state == WifiPhy::State::TX ||
		state == WifiPhy::State::RX)
	{
		IdleTimeOccur(ch_numbers[6], duration);
	}
}
void PeriodApThroughput::StateChange8(Time Start, Time duration, WifiPhy::State state)
{
	//if(state == WifiPhy::State::IDLE)
	if(state == WifiPhy::State::TX ||
		state == WifiPhy::State::RX)
	{
		IdleTimeOccur(ch_numbers[7], duration);
	}
}

void PeriodApThroughput::AddCh(uint16_t ch_num)
{
	ch_numbers.push_back(ch_num);
	idle_time[ch_num] = Time("0.0");
}
void PeriodApThroughput::SetTotalDemand(double total)
{
	total_demand = total;
}

void PeriodApThroughput::ResetIdle()
{
	last_print_time = Simulator::Now();
	for(map<uint16_t, ns3::Time>::iterator i = idle_time.begin();
		i != idle_time.end();
		++i)
	{
		i->second = Time("0.0");
	}
}

void PeriodApThroughput::IdleTimeOccur(uint16_t ch_num,Time duration)
{
	idle_time[ch_num] += duration;
}

PeriodStaThroughput::PeriodStaThroughput(double demand)
{
	ex_through_packets = 0;
	min_through_packets = numeric_limits<uint32_t>::max();
	max_through_packets = numeric_limits<uint32_t>::min();
	now_through_packets = 0;
	total_through_packets = 0;
	last_print_time = Seconds(ARP_TIME + CLIENT_START_TIME);
	this->demand = demand;
}

PeriodStaThroughput::~PeriodStaThroughput()
{

}

OutStaInfo PeriodStaThroughput::GetThroughput(uint32_t through_packets)
{
	OutStaInfo result;

	Time now = Simulator::Now();
	if(now == Time("0.0"))
		now = Seconds (ARP_TIME + CLIENT_START_TIME + SIMULATION_TIME);

	Time period = now - last_print_time;
	last_print_time = now;

	total_through_packets = through_packets;
	now_through_packets = total_through_packets - ex_through_packets;
	ex_through_packets = through_packets;

	if(now_through_packets < min_through_packets)
		min_through_packets = now_through_packets;

	if(now_through_packets > max_through_packets)
		max_through_packets = now_through_packets;

	result.now_through_packets = now_through_packets * PAYLOADSIZE * 8 / (MEGA * period.GetSeconds());
	result.avg_throughput = total_through_packets * PAYLOADSIZE * 8 / (MEGA * (now - Seconds(ARP_TIME + CLIENT_START_TIME)).GetSeconds()); //Mbit/s
	result.min_throughput = min_through_packets * PAYLOADSIZE * 8 / (MEGA * period.GetSeconds());
	result.max_throughput = max_through_packets * PAYLOADSIZE * 8 / (MEGA * period.GetSeconds());

	result.served_demand_ratio =  result.avg_throughput / demand * 100.0;


	return result;
}

DynamicChannelBonding::DynamicChannelBonding()
{

}

DynamicChannelBonding::~DynamicChannelBonding()
{

}

void DynamicChannelBonding::clean_map()
{

}

void DynamicChannelBonding::insert_mac(unsigned int ap_index, unsigned int ch_num, Mac48Address address)
{

}

void DynamicChannelBonding::calculate_tau()
{
	double tau_i;
	double p_i;
	vector<unsigned int> aps = GetApIndexs();
	map<unsigned int, map<unsigned int, double> > new_tau;
	unsigned int primary_ch;

	for(vector<unsigned int>::iterator i = aps.begin();
		i != aps.end();
		++i)
	{
		p_i = p[*i];
		tau_i = ( 2.0 * ( 1.0 - p_i) ) / ( ( 1.0 - 2 * p_i ) * ( WINDOW_SIZE_BEGIN + 1.0 ) + ( p_i * WINDOW_SIZE_BEGIN) * ( 1 - pow(2*p_i, WINDOW_SIZE_MAX) ) );

		vector<unsigned int> chs = GetApChannels(*i);
		primary_ch = GetPrimaryChannel(*i);
		for(vector<unsigned int>::iterator c = chs.begin();
			c != chs.end();
			++c)
		{
			if(*c == primary_ch)
			{
				new_tau[*i][*c] = tau_i;
			}

			else
			{
				new_tau[*i][*c] = 1 - pow( (1.0 - tau[*i][*c]) , GetNI(*i) - 1);

				for(vector<unsigned int>::iterator j = aps.begin();
					j != aps.end();
					++j)
				{
					if(*i != *j)
					{
						new_tau[*i][*c] = new_tau[*i][*c] * pow(1.0 - tau[*j][*c] , GetNIJ(*i,*j));
					}
				}
			}
		}
	}

	tau = new_tau;
}

void DynamicChannelBonding::calculate_p()
{
	vector<unsigned int> aps = GetApIndexs();
	unsigned int primary_ch;
	unsigned int i, j;
	double temp_val;
	for(map<unsigned int, double>::iterator p_i = p.begin();
		p_i != p.end();
		++p_i)
	{
		i = p_i->first;
		temp_val = 1.0;

		primary_ch = GetPrimaryChannel(i);

		for(vector<unsigned int>::iterator iter_j = aps.begin();
			iter_j != aps.end();
			++iter_j)
		{
			j = *iter_j;

			temp_val = temp_val * pow( (1.0 - tau[j][primary_ch] ) , GetNIJ(i,j) );
		}
		temp_val = temp_val / (1.0 - tau[i][primary_ch]);
		temp_val = 1 - temp_val;
	}
}

void DynamicChannelBonding::calculate_variables()
{
	for(unsigned int i = 0; i < LOOPCOUNT; ++i)
	{
		calculate_p();
		calculate_tau();
	}
}

unsigned int DynamicChannelBonding::get_width(unsigned int index)
{
	double largest_gamma = 0.0;
	double gamma, temp;
	unsigned int proper_width = 20;
	unsigned int widest = WidestWidth(GetPrimaryChannel(index));
	vector<unsigned int> aps = GetInterfereApIndexs(index);

	for(unsigned int width = 20; width <= widest; width *= 2)
	{
		gamma = 0.0;
		temp = GetThroughput_demand_ratio(index);

		if(temp > 1.0)
			temp = 1.0;

		gamma = temp;

		for(vector<unsigned int>::iterator ap = aps.begin();
			ap != aps.end();
			++ap)
		{
			temp = GetThroughput_demand_ratio(*ap);
			if(temp > 1.0)
				temp = 1.0;

			gamma += temp;
		}

		if(gamma > largest_gamma)
		{
			proper_width = width;
			largest_gamma = gamma;
		}
	}

	return proper_width;
}
/*
 *need to do
 */


double DynamicChannelBonding::GetNI(unsigned int i)
{
	double result = 0.0;
	return result;
}

double DynamicChannelBonding::GetNIJ(unsigned int i, unsigned int j)
{
	double result = 0.0;

	if(i == j)
	{
		result = GetNI(i);
	}

	else
	{

	}

	return result;
}

vector<unsigned int> DynamicChannelBonding::GetApIndexs()
{
	vector<unsigned int> result;
	return result;
}

vector<unsigned int> DynamicChannelBonding::GetInterfereApIndexs(unsigned int index)
{
	vector<unsigned int> result;
	return result;
}

vector<unsigned int> DynamicChannelBonding::GetApChannels(unsigned int index)
{
	vector<unsigned int> result;
	return result;
}

unsigned int DynamicChannelBonding::GetPrimaryChannel(unsigned int index)
{
	unsigned int result = 0;
	return result;
}

double DynamicChannelBonding::GetThroughputHat(unsigned int index)
{
	double result = 0.0;
	return result;
}

double DynamicChannelBonding::GetThroughput_demand_ratio(unsigned int index)
{
	double result = 0.0;
	return result;
}

unsigned int WidestWidth(unsigned int primary_ch)
{
	unsigned int result = 0;
	return result;
}

