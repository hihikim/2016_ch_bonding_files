#include "my-channel-manager.h"
#include "ns3/log.h"


namespace ns3 {


NS_LOG_COMPONENT_DEFINE ("ChannelManager");

ChannelManager::ChannelManager()
: ch_map()
{
	max_width = 160;
	primary_ch = 36;
	num_received = 0;

	ChannelMapping();
	received_channel[36] = false; received_channel[40] = false; received_channel[44] = false; received_channel[48] = false;
	received_channel[52] = false; received_channel[56] = false; received_channel[60] = false; received_channel[64] = false;
}
void ChannelManager::SetMyMac(Ptr<MacLow> mac)
{
	m_mac = mac;
}

void ChannelManager::ChannelMapping() const
{
	//20Hz
	ch_map[36] = std::make_pair(40, 20);
	ch_map[40] = std::make_pair(36, 20);
	ch_map[44] = std::make_pair(48, 20);
	ch_map[48] = std::make_pair(44, 20);
	ch_map[52] = std::make_pair(56, 20);
	ch_map[56] = std::make_pair(52, 20);
	ch_map[60] = std::make_pair(64, 20);
	ch_map[64] = std::make_pair(60, 20);

	//40Hz
	ch_map[38] = std::make_pair(46, 40);
	ch_map[46] = std::make_pair(38, 40);
	ch_map[54] = std::make_pair(62, 40);
	ch_map[62] = std::make_pair(54, 40);

	//80Hz
	ch_map[42] = std::make_pair(58, 80);
	ch_map[58] = std::make_pair(42, 80);

	//160Hz
	ch_map[50] = std::make_pair(50, 160);
}

void ChannelManager::SetChannelOption(uint16_t primary_ch,uint32_t max_width){
	this->max_width = max_width;
	this->primary_ch = primary_ch;
}

void ChannelManager::MakePhys(YansWifiPhyHelper phy, Ptr<WifiPhy> primary, uint16_t ch_num, uint32_t channel_width, enum WifiPhyStandard standard){
   SetChannelOption(ch_num, channel_width);

   Ptr<NetDevice> device = primary->GetDevice();
   Ptr<Node> node = device->GetNode();


   num_received = 0;
   for(int i =0;i<8;++i){
	   if(ch_numbers[i] == ch_num)
	   {
		   m_phys[ch_num] = primary;
	   }
	   else
	   {
		   m_phys[ch_numbers[i]] = phy.Create (node, device);
		   m_phys[ch_numbers[i]]->ConfigureStandard (standard);
	   }

	   m_phys[ch_numbers[i]]->SetChannelNumber(ch_numbers[i]);
   }
}

void ChannelManager::ResetPhys(){
	for(int i =0;i<8;++i){
		m_phys[ch_numbers[i]]->SetReceiveOkCallback (MakeNullCallback<void, Ptr<Packet>, double, WifiTxVector, enum WifiPreamble> ());
		m_phys[ch_numbers[i]]->SetReceiveErrorCallback (MakeNullCallback<void, Ptr<Packet>, double> ());
		m_phys[ch_numbers[i]] = 0;
	}
}

void ChannelManager::ClearReceiveRecord(){
	/*
	 * received_channel[36] = false; received_channel[40] = false; received_channel[44] = false; received_channel[48] = false;
	 * received_channel[52] = false; received_channel[56] = false; received_channel[60] = false; received_channel[64] = false;
	 */

	for(int i =0;i<8;++i){
		received_channel[ch_numbers[i]] = false;
	}
}


uint16_t ChannelManager::CheckChBonding(uint16_t primary)
{
	std::pair<uint16_t,uint32_t> ch_info;

	ch_info = ch_map[primary];


	uint32_t usable_ch = primary;

	while(true)
	{
		ch_info = ch_map[usable_ch];
		if( ch_info.second == max_width)
			break;

		else if( CheckAllSubChannelIdle( ch_info.first ))
		{
			usable_ch = (usable_ch + ch_info.first) / 2;
		}

		else
			break;
	}

	return usable_ch;

}

bool ChannelManager::CheckAllSubChannelIdle(uint16_t ch_num){
	std::pair<uint16_t,uint32_t> ch_info;
	ch_info = ch_map[ch_num];

	uint32_t ch_width = ch_info.second;
	uint16_t side_num = ch_width / 20;

	if(ch_width == 20){
		if(m_phys[ch_num]->IsStateIdle ())
			return true;

		else
			return false;
	}


	else
	{
		if(CheckAllSubChannelIdle(ch_num - side_num) && CheckAllSubChannelIdle (ch_num + side_num) )
			return true;

		else
			return false;
	}


}

uint16_t ChannelManager::GetUsableChannelBonding(uint16_t primary)
{
	std::pair<uint16_t,uint32_t> ch_info;

	ch_info = ch_map[primary];


	uint16_t usable_ch = primary;

	while(true)
	{
		ch_info = ch_map[usable_ch];
		if( ch_info.second >= max_width)
			break;

		else if( CheckAllSubChannelReceived( ch_info.first ))
		{
			usable_ch = (usable_ch + ch_info.first) / 2;
		}

		else
			break;
	}

	return usable_ch;
}

bool ChannelManager::CheckAllSubChannelReceived(uint16_t ch_num)
{
	std::pair<uint16_t,uint32_t> ch_info;
	ch_info = ch_map[ch_num];

	uint32_t ch_width = ch_info.second;
	uint16_t side_num = ch_width / 20;


	if(ch_width == 20){
		return received_channel[ch_num];
	}

	else
	{
		if(CheckAllSubChannelReceived(ch_num - side_num) && CheckAllSubChannelReceived (ch_num + side_num) )
			return true;

		else
			return false;
	}

}
uint16_t ChannelManager::GetPrimaryCh()
{
	return primary_ch;
}
uint32_t ChannelManager::GetMaxWidth()
{
	return max_width;
}
void ChannelManager::SetPhysCallback()
{
	for(int i =0;i<8;++i){
		m_phys[ch_numbers[i]]->SetReceiveErrorCallback (MakeCallback (&ChannelManager::ReceiveError, this));
	}
	m_phys[ch_numbers[36]]->SetReceiveOkCallback (MakeCallback (&ChannelManager::Receive36Channel, this));
	m_phys[ch_numbers[40]]->SetReceiveOkCallback (MakeCallback (&ChannelManager::Receive40Channel, this));
	m_phys[ch_numbers[44]]->SetReceiveOkCallback (MakeCallback (&ChannelManager::Receive44Channel, this));
	m_phys[ch_numbers[48]]->SetReceiveOkCallback (MakeCallback (&ChannelManager::Receive48Channel, this));
	m_phys[ch_numbers[52]]->SetReceiveOkCallback (MakeCallback (&ChannelManager::Receive52Channel, this));
	m_phys[ch_numbers[56]]->SetReceiveOkCallback (MakeCallback (&ChannelManager::Receive56Channel, this));
	m_phys[ch_numbers[60]]->SetReceiveOkCallback (MakeCallback (&ChannelManager::Receive60Channel, this));
	m_phys[ch_numbers[64]]->SetReceiveOkCallback (MakeCallback (&ChannelManager::Receive64Channel, this));
}

void ChannelManager::Receive36Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble)
{
	ReceiveSubChannel(Packet,rxSnr,txVector,preamble,36);
}

void ChannelManager::Receive40Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble)
{
	ReceiveSubChannel(Packet,rxSnr,txVector,preamble,40);
}

void ChannelManager::Receive44Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble)
{
	ReceiveSubChannel(Packet,rxSnr,txVector,preamble,44);
}

void ChannelManager::Receive48Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble)
{
	ReceiveSubChannel(Packet,rxSnr,txVector,preamble,48);
}

void ChannelManager::Receive52Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble)
{
	ReceiveSubChannel(Packet,rxSnr,txVector,preamble,52);
}

void ChannelManager::Receive56Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble)
{
	ReceiveSubChannel(Packet,rxSnr,txVector,preamble,56);
}

void ChannelManager::Receive60Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble)
{
	ReceiveSubChannel(Packet,rxSnr,txVector,preamble,60);
}

void ChannelManager::Receive64Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble)
{
	ReceiveSubChannel(Packet,rxSnr,txVector,preamble,64);
}

void ChannelManager::ReceiveSubChannel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble, uint16_t ch_num)
{
	WifiMacHeader hdr;
	Packet->PeekHeader(hdr);

	num_received++;
	last_received_packet[ch_num] = Packet;
	received_channel[ch_num] = true;

	if(num_received == 1)  //initial
	{
		//start timer
		if(hdr.IsRts())
		{
			receive_rts = Simulator::Schedule(m_mac->GetSifs(),
													&ChannelManager::ClearReceiveRecord, this
													);
		}
		else if(hdr.IsCts())
		{
			receive_rts = Simulator::Schedule(m_mac->GetSifs(),
													&ChannelManager::ClearReceiveRecord, this
													);
		}
	}


	if(hdr.IsRts() || hdr.IsCts() )
	{
		std::pair<uint16_t,uint32_t> ch_info = ch_map[GetUsableChannelBonding(primary_ch)];
		request_width = ch_info.second;

		if (ch_num == primary_ch)
		{
			if(hdr.IsRts())
				receive_rts.Cancel();

			else
				receive_cts.Cancel();
		}
	}

	if (ch_num == primary_ch){
		ManageReceived(Packet, rxSnr, txVector, preamble);
	}
}




void ChannelManager::ManageReceived  (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble)
{
	WifiMacHeader hdr;
	Packet->PeekHeader(hdr);
	if(hdr.IsData())
	{
		//data merge

	}

	m_mac->DeaggregateAmpduAndReceive(Packet, rxSnr,txVector, preamble);
}

bool ChannelManager::CheckWidthUsable()
{
	uint16_t check_ch = primary_ch;
	std::pair<uint16_t,uint32_t> ch_info;
	ch_info = ch_map[check_ch];

	if(request_width == 20)
	{
		return received_channel[check_ch];
	}

	for(uint32_t width = ch_info.second; width < request_width;)
	{
		if(!received_channel[ch_info.first])  //if second channel not received
			return false;

		check_ch = (check_ch + ch_info.first)/2;  //go to bigger channel

		ch_info = ch_map[check_ch];
	}

	return true;

}

}
