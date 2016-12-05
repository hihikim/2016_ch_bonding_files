#include "my_test.h"



using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("my-wifi-test");




int main (int argc, char *argv[])
{

	Parser parser;
	string test_number = 0;

	CommandLine cmd;
	cmd.AddValue("test_number" , "insert number of input file",test_number);

	uint32_t payloadSize = 1472;
	ostringstream oss;

	/*
	 * parse ap input
	 */
	oss<<"./input/ap/"<<test_number;
	parser.SetupInputFile(oss.str(),true);
	oss.clear();
	parser.Parse();
	parser.CloseFile();

	/*
	 * parse sta input
	 */
	oss<<"./input/sta/"<<test_number;
	parser.SetupInputFile(oss.str(),false);
	oss.clear();
	parser.Parse();
	parser.CloseFile();

	double shortest_sq_dist, sq_dist;
	shortest_sq_dist = numeric_limits<double>::max();
	unsigned int shortest_ap;


	map<unsigned int, vector <unsigned int> > shortest_stas_of_ap; //shortest_stas_of_ap[a] = list of STAs which is allocated with AP a

	for(map<unsigned int,InStaInfo>::iterator i = parser.GetStaBegin();
		i != parser.GetStaEnd();
		++i)
	{
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
		shortest_stas_of_ap[shortest_ap].push_back(i->first);
	}
	map<unsigned int, NodeContainer> ap_nodes;
	map<unsigned int, NodeContainer> sta_nodes;

	map<unsigned int, NetDeviceContainer> ap_devs;
	map<unsigned int, NetDeviceContainer> sta_devs;


	YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
	YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
	phy.SetChannel (channel.Create ());

	phy.Set ("ShortGuardEnabled", BooleanValue (ENABLE_SHORT_GD));

	WifiHelper wifi;
	wifi.SetStandard (WIFI_PHY_STANDARD_80211ac);
	WifiMacHelper mac;

	oss.clear();
	oss << "VhtMcs" << MCS_NUMBER;
	wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", StringValue (oss.str ()),
								"ControlMode", StringValue (oss.str ()),
								"RtsCtsThreshold", UintegerValue(100));

	Ssid ssid = Ssid ("ns3-80211ac");



	Ptr<RegularWifiMac> m_mac;

	MobilityHelper mobility;
	Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();


	for(map<unsigned int, vector <unsigned int>>::iterator i = shortest_stas_of_ap.begin();
		i != shortest_stas_of_ap.end();
		++i)
	{
		ap_nodes[i->first] = NodeContainer();
		ap_nodes[i->first].Create(1);  //create 1 node

		mac.SetType ("ns3::ApWifiMac",
					   "Ssid", SsidValue (ssid));

		ap_devs[i->first] = wifi.Install(phy, mac, ap_nodes[i->first]);

		m_mac = DynamicCast<RegularWifiMac> (DynamicCast<WifiNetDevice>(ap_devs[i->first].Get(0))->GetMac());
		Ptr<MacLow> m_low = m_mac->GetLow();
		m_low->EnableChannelBonding();

		InApInfo ap_info = parser.GetApInfo(i->first);
		m_low->SetChannelManager(phy, actual_ch[ap_info.channel], ap_info.width, WIFI_PHY_STANDARD_80211ac);

		positionAlloc->Add(Vector(ap_info.x, ap_info.y, 0) );
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
		}
	}

	mobility.SetPositionAllocator (positionAlloc);
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

	InternetStackHelper stack;

	Ipv4AddressHelper address;
	address.SetBase (IP_BASE, SUBNET_MASK);

	map < unsigned int, vector <ApplicationContainer> > serverApp , sinkApp, clientApp;

	for(map<unsigned int, vector <unsigned int>>::iterator i = shortest_stas_of_ap.begin();
		i != shortest_stas_of_ap.end();
		++i)
	{
		mobility.Install (ap_nodes[i->first]);
		stack.Install (ap_nodes[i->first]);
		Ipv4InterfaceContainer apNodeInterface = address.Assign(ap_devs[i->first]);

		unsigned int port_num = 1024;

		for(vector <unsigned int>::iterator j = i->second.begin();
			j != i->second.end();
			j++)
		{
			mobility.Install (sta_nodes[*j]);
			stack.Install (sta_nodes[*j]);

			Ipv4InterfaceContainer staNodeInterface = address.Assign(sta_devs[*j]);

			UdpServerHelper myServer (port_num);
			serverApp[*j].push_back( myServer.Install(sta_nodes[*j]) ) ;   //downlink ap -> stas

			vector<ApplicationContainer>::reverse_iterator end_iter;  //for find end

			end_iter = serverApp[*j].rbegin();
			end_iter->Start (Seconds(SERVER_START_TIME));
			end_iter->Stop (Seconds(SIMULATION_TIME + 1.0));

			UdpClientHelper myClient (staNodeInterface.GetAddress (0), port_num);
			myClient.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
			myClient.SetAttribute ("Interval", TimeValue (Time ("0.00001"))); //packets/s
			myClient.SetAttribute ("PacketSize", UintegerValue (payloadSize));

			clientApp[i->first].push_back( myClient.Install(ap_nodes[i->first]	) );

			end_iter = clientApp[i->first].rend();
			end_iter->Start( Seconds(CLIENT_START_TIME));
			end_iter->Stop( Seconds (SIMULATION_TIME + 1.0) );

			++port_num;
		}
	}

	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

	Simulator::Stop (Seconds (SIMULATION_TIME + 1));
	Simulator::Run ();
	Simulator::Destroy ();

	for(map < unsigned int, vector <ApplicationContainer> >::iterator i;
		i != serverApp.end();
		++i)
	{
		double throughput = 0;
		uint32_t totalPacketsThrough = DynamicCast<UdpServer> (i->second[0].Get (0))->GetReceived ();
		throughput = totalPacketsThrough * payloadSize * 8 / (SIMULATION_TIME * 1000000.0); //Mbit/s
		cout<<"sta index : "<<i->first<<" throughput : "<<throughput<<endl;
	}

	return 0;
}

Parser::Parser()
{
	input_file = NULL;
	is_ap = false;
}

Parser::~Parser()
{
	if(input_file->is_open())
		input_file->close();

	if(input_file != NULL)
	{
		delete input_file;
		input_file = NULL;
	}
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
	input_file = new ifstream(path,ios::in);
	is_ap = ap_sta;
}

void Parser::Parse()
{
	string str;
	istringstream iss;
	char line[CHAR_MAX_LENGTH];
	unsigned int temp;
	for(;!input_file->eof();)
	{
		input_file->getline(line,CHAR_MAX_LENGTH);

		/*
		 * convert to string
		 */
		str.clear();
		str += line;

		/*
		 * remove comment
		 */
		temp = str.find('#');
		if(temp < str.size())
		{
			str.resize(temp);
		}

		while(str.at(0) == ' ')
			str.erase(str.begin());

		unsigned int index;
		if(is_ap)
		{
			InApInfo input_info;

			for(int i =0;i<5;++i)
			{
				temp = str.find(' ');
				iss.clear();
				iss.str(str.substr(0, temp));
				str = str.substr(temp+1);

				while(str.at(0) == ' ')
					str.erase(str.begin());


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
			ap_info[index] = input_info;
		}
		else
		{
			InStaInfo input_info;
			for(int i =0;i<4;++i)
			{
				temp = str.find(' ');
				iss.clear();
				iss.str(str.substr(0, temp));
				str = str.substr(temp+1);
				while(str.at(0) == ' ')
					str.erase(str.begin());


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
	if(input_file->is_open())
		input_file->close();

	if(input_file != NULL)
	{
		delete input_file;
		input_file = NULL;
	}
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

