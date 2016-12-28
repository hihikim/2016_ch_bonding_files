#include "my_test.h"
#include "ns3/random-variable-stream.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("my-wifi-test");




int main (int argc, char *argv[])
{

	Parser parser;
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

	for(map<unsigned int,InApInfo>::iterator i = parser.GetApBegin();
		i != parser.GetApEnd();
		++i)
	{
		shortest_stas_of_ap[i->first].reserve(1);
	}

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
		cout<<"sta "<<i->first<<" : shortest ap "<<shortest_ap<<" distance "<<sqrt(shortest_sq_dist)<<endl;
		shortest_stas_of_ap[shortest_ap].push_back(i->first);
	}



	for(map<unsigned int,vector <unsigned int>>::iterator i = shortest_stas_of_ap.begin();
		i != shortest_stas_of_ap.end();
		++i)
	{
		cout<<"AP "<<i->first<<":";
		for(vector <unsigned int>::iterator j = i->second.begin(); j != i->second.end();j++)
		{
			cout<<"STA "<<*j<<" | ";
		}
		cout<<endl;
	}


	map<unsigned int, NodeContainer> ap_nodes;
	map<unsigned int, NodeContainer> sta_nodes;

	map<unsigned int, NetDeviceContainer> ap_devs;
	map<unsigned int, NetDeviceContainer> sta_devs;


	YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
	YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();

	//channel.AddPropagationLoss("ns3::FixedRssLossModel", "Rss", DoubleValue(100.0));
	//channel.AddPropagationLoss("ns3::RangePropagationLossModel", "MaxRange", DoubleValue(250.0));

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

		cout<<"ap "<<i->first<<" : address ("<<apNodeInterface.GetAddress(0)<<") | ";
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
			cout<<"sta "<<*j<<" : address ("<<staNodeInterface.GetAddress(0)<<") | ";

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
		cout<<endl;
	}

	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
	/*OlsrHelper olsr;

	Ipv4StaticRoutingHelper staticRouting;

	Ipv4ListRoutingHelper list;
	list.Add(staticRouting, 0);
	list.Add(olsr,10);
	stack.SetRoutingHelper(list);*/

	Simulator::Stop (Seconds (ARP_TIME + CLIENT_START_TIME + SIMULATION_TIME));
	Simulator::Run ();
	Simulator::Destroy ();

	for(map < unsigned int, vector <ApplicationContainer> >::iterator i = serverApp.begin();
		i != serverApp.end();
		++i)
	{
		double throughput = 0;
		uint32_t totalPacketsThrough = DynamicCast<UdpServer> ((i->second)[0].Get (0))->GetReceived ();
		throughput = totalPacketsThrough * payloadSize * 8 / (SIMULATION_TIME * 1000000.0); //Mbit/s
		cout<<"sta index : "<<i->first<<" throughput : "<<throughput<<endl;
	}

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

