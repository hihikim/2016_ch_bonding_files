#include "my_test.h"
#include "ns3/random-variable-stream.h"



NS_LOG_COMPONENT_DEFINE ("my-wifi-test");




int main (int argc, char *argv[])
{
	Parser parser;
	OutputGenerator* og = new OutputGenerator();
	unsigned int test_number = 0;
	unsigned int payloadSize = 1472;
	CommandLine cmd;
	cmd.AddValue("test_number" , "insert number of input file", test_number);

	cmd.Parse(argc,argv);


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

	DynamicChannelBonding dcb(payloadSize,og);

	dcb.InputInfo(parser.GetApInfos(),parser.GetStaInfos());

	dcb.CalculateShortestAp();

	dcb.SetTestEnv();

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

map<unsigned int, InApInfo> Parser::GetApInfos()
{
	return ap_info;
}
map<unsigned int, InStaInfo> Parser::GetStaInfos()
{
	return sta_info;
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
double PeriodApThroughput::GetTotalDemand()
{
	return total_demand;
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

DynamicChannelBonding::DynamicChannelBonding(uint32_t payloadSize,OutputGenerator* og) : NodeManager(payloadSize,og)
{
	channel_map = ChannelBondingManager::ChannelMapping();
	make_bit_map();
}

DynamicChannelBonding::~DynamicChannelBonding()
{

}

void DynamicChannelBonding::clean_map()
{
	//link_map.clear();
	total_link_map.clear();
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

				vector<unsigned int> aps_of_ch = GetApsUseChannel(*c);
				for(vector<unsigned int>::iterator j = aps_of_ch.begin();
					j != aps_of_ch.end();
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
	vector<unsigned int> aps;
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

		aps = GetApsUseChannel(primary_ch);

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
	vector<unsigned int> aps;

	for(unsigned int width = 20; width <= widest; width *= 2)
	{
		gamma = 0.0;
		temp = GetThroughput_demand_ratio(index);

		if(temp > 1.0)
			temp = 1.0;

		gamma = temp;
		aps = GetInterfereApIndexs(index,width);

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

double DynamicChannelBonding::GetNI(unsigned int i)
{
	double result = 0.0;
	map<Mac48Address, bool> my_link_state = total_link_map[i];

	for(map<Mac48Address, bool>::iterator link = my_link_state.begin();
		link != my_link_state.end();
		++link)
	{
		if(link->second)
			result += 1.0;
	}

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
		for(map<Mac48Address, bool>::iterator addr = total_link_map[j].begin();
			addr != total_link_map[j].end();
			++addr)
		{
			if(addr->second)
			{
				if(total_link_map[i][addr->first])
					result += 1.0;
			}
		}
	}

	return result;
}

vector<unsigned int> DynamicChannelBonding::GetApIndexs()
{
	return all_aps;
}

vector<unsigned int> DynamicChannelBonding::GetApsUseChannel(unsigned int channel_number)
{
	/*
	 * need to do
	 */
	vector<unsigned int> result;

	for(map<unsigned int, vector <unsigned int> >::iterator ap = shortest_stas_of_ap.begin();
		ap != shortest_stas_of_ap.end();
		++ap)
	{
		//ap_infos[ap->first].channel
	}

	return result;
}

vector<unsigned int> DynamicChannelBonding::GetInterfereApIndexs(unsigned int index, unsigned int width)
{
	vector<unsigned int> result;

	for(vector <unsigned int>::iterator other = all_aps.begin();
		other != all_aps.end();
		++other)
	{
		if(index != *other)
		{
			if( (UsingChannelsToBit(index, width) & UsingChannelsToBit(*other,ap_infos[*other].width)) != 0 )
			{
				result.push_back(*other);
			}
		}
	}

	return result;
}

vector<unsigned int> DynamicChannelBonding::GetApChannels(unsigned int index)
{

	return using_ch_map[index];
}

int DynamicChannelBonding::UsingChannelsToBit(unsigned int index, unsigned int width)
{
	unsigned int ch_num = GetPrimaryChannel(index);
	unsigned int widest = WidestWidth(ch_num);


	if(width > widest)
		width = widest;

	int loop_count = 0;
	while(true)
	{
		if(channel_map[ch_num].Width == width)
			break;
		else
		{
			ch_num = channel_map[ch_num].Parent;
		}

		++loop_count;

		if(loop_count > 10)
			NS_FATAL_ERROR("infinite loop\n");
	}

	return channel_bit_map[ch_num];
}

unsigned int DynamicChannelBonding::GetPrimaryChannel(unsigned int index)
{
	return ap_infos[index].channel;
}

double DynamicChannelBonding::GetThroughputHat(unsigned int index)
{
	/*
	 *need to do
	 */
	double result = 0.0;
	return result;
}

double DynamicChannelBonding::GetThroughput_demand_ratio(unsigned int index)
{
	double result = 0.0;
	result = GetThroughputHat(index);
	result /= ap_thr[index]->GetTotalDemand();
	return result;
}

void DynamicChannelBonding::InitTauAndP()
{
	/*
	 *need to do
	 */
}

void DynamicChannelBonding::make_bit_map()
{
	channel_bit_map[36] = CH_36; channel_bit_map[40] = CH_40; channel_bit_map[44] = CH_44; channel_bit_map[48] = CH_48;
	channel_bit_map[52] = CH_52; channel_bit_map[56] = CH_56; channel_bit_map[60] = CH_60; channel_bit_map[64] = CH_64;

	channel_bit_map[100] = CH_100; channel_bit_map[104] = CH_104; channel_bit_map[108] = CH_108; channel_bit_map[112] = CH_112;
	channel_bit_map[116] = CH_116; channel_bit_map[120] = CH_120; channel_bit_map[124] = CH_124; channel_bit_map[128] = CH_128;

	channel_bit_map[132] = CH_132; channel_bit_map[136] = CH_136; channel_bit_map[140] = CH_140; channel_bit_map[144] = CH_144;

	channel_bit_map[149] = CH_149; channel_bit_map[153] = CH_153; channel_bit_map[157] = CH_157; channel_bit_map[161] = CH_161;

	channel_bit_map[165] = CH_165;

	for(std::map<uint16_t, ChannelInfo >::iterator i = channel_map.begin();
		i != channel_map.end();
		++i)
	{
		fill_bit_map(i->first);
	}
}

int DynamicChannelBonding::fill_bit_map(uint16_t ch_num)
{
	int result;

	if(channel_bit_map.find(ch_num) == channel_bit_map.end())
	{
		/*
		 * not filled
		 */

		if(channel_map[ch_num].Width == 20)
		{
			ostringstream oss;
			oss<<"Channel "<<ch_num<<" not filled \n";
			NS_FATAL_ERROR(oss.str());
		}

		result = fill_bit_map(channel_map[ch_num].L_CHD);
		result = result | fill_bit_map(channel_map[ch_num].R_CHD);
		channel_bit_map[ch_num] = result;
	}

	else
	{
		/*
		 *  filled
		 */
		result = channel_bit_map[ch_num];
	}
	return result;
}

unsigned int WidestWidth(unsigned int primary_ch)
{
	unsigned int result = 0;
	switch(primary_ch)
	{
	case 36: case 40: case 44: case 48: case 52: case 56: case 60: case 64:
	case 100: case 104: case 108: case 112: case 116: case 120: case 124: case 128:
		result = 160;
		break;

	case 132: case 136: case 140: case 144: case 149: case 153: case 157: case 161:
		result = 80;
		break;
	case 165:
		result = 20;
		break;
	default:
		std::ostringstream oss;
		oss<<primary_ch<<" is wrong prime number";
		NS_FATAL_ERROR(oss.str());
	}
	return result;
}

NodeManager::NodeManager(uint32_t payloadSize,OutputGenerator* og)
{
	this->payloadSize = payloadSize;
	this->og = og;
}

NodeManager::~NodeManager()
{

}

void NodeManager::InputInfo(map<uint32_t, InApInfo> ap_info, map<uint32_t, InStaInfo> sta_info)
{
	this->ap_infos = ap_info;
	this->sta_infos = sta_info;
}

void NodeManager::CalculateShortestAp()
{
	double shortest_sq_dist,sq_dist;
	unsigned int shortest_ap;

	for(map<unsigned int,InStaInfo>::iterator i = sta_infos.begin();
		i != sta_infos.end();
		++i)
	{
		shortest_sq_dist = numeric_limits<double>::max();
		InStaInfo sta_info = i->second;
		for(map<unsigned int,InApInfo>::iterator j = ap_infos.begin();
			j != ap_infos.end();
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

	for(map<unsigned int, vector <unsigned int>>::iterator i = shortest_stas_of_ap.begin();
		i != shortest_stas_of_ap.end();
		++i)
	{
		all_aps.push_back(i->first);
	}

	og->PrintLinkInfo(shortest_stas_of_ap);
	og->PrintDistance();
	og->CleanDistMap();
}

void NodeManager::SetTestEnv()
{
	Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
	ostringstream oss;
	oss.setf(ios::fixed,ios::floatfield);

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

	wifi.SetRemoteStationManager ("ns3::MinstrelHtWifiManager", "RtsCtsThreshold", UintegerValue(100));



	//Ssid ssid = Ssid ("ns3-80211ac");
	Ssid ssid;

	Ptr<RegularWifiMac> m_mac;

	MobilityHelper mobility;
	Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

	InApInfo ap_info;
	InStaInfo sta_info;

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


		ap_info = ap_infos[i->first];
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


			sta_info = sta_infos[*j];

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
			sta_info = sta_infos[*j];
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

			double interval = (double) payloadSize * 8.0 / (sta_info.traffic_demand * MEGA);

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
	}

	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
	/*OlsrHelper olsr;

	Ipv4StaticRoutingHelper staticRouting;

	Ipv4ListRoutingHelper list;
	list.Add(staticRouting, 0);
	list.Add(olsr,10);
	stack.SetRoutingHelper(list);*/

	MakeAddressMap();


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

}


void NodeManager::MakeAddressMap()
{
	Mac48Address addr;
	for(map<unsigned int, NetDeviceContainer>::iterator ap = ap_devs.begin();
		ap != ap_devs.end();
		++ap)
	{
		addr = Mac48Address::ConvertFrom((ap->second.Get(0))->GetAddress());
		address_map[ addr ] = make_pair(ap->first,true);
	}

	for(map<unsigned int, NetDeviceContainer>::iterator sta = sta_devs.begin();
		sta != sta_devs.end();
		++sta)
	{
		addr = Mac48Address::ConvertFrom((sta->second.Get(0))->GetAddress());
		address_map[addr] = make_pair(sta->first,false);
	}
}



