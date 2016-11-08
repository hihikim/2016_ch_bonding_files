#include "my-channel-manager.h"

#include "ns3/assert.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/tag.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/double.h"



namespace ns3 {
class MacLow;

NS_LOG_COMPONENT_DEFINE ("ChannelBondingManager");

ChannelBondingManager::ChannelBondingManager()
: ch_map(ChannelMapping())
{
	max_width = 160;
	primary_ch = 36;
	num_received = 0;
	request_width = 160;

	ChannelMapping();
	received_channel[36] = false; received_channel[40] = false; received_channel[44] = false; received_channel[48] = false;
	received_channel[52] = false; received_channel[56] = false; received_channel[60] = false; received_channel[64] = false;
}
void ChannelBondingManager::SetMyMac(Ptr<MacLow> mac)
{
	m_mac = mac;
}

std::map<uint16_t, std::pair<uint16_t,uint32_t> >  ChannelBondingManager::ChannelMapping()
{
	 std::map<uint16_t, std::pair<uint16_t,uint32_t> >  result;
	//20Hz
	 result[36] = std::make_pair(40, 20);
	 result[40] = std::make_pair(36, 20);
	 result[44] = std::make_pair(48, 20);
	 result[48] = std::make_pair(44, 20);
	 result[52] = std::make_pair(56, 20);
	 result[56] = std::make_pair(52, 20);
	 result[60] = std::make_pair(64, 20);
	 result[64] = std::make_pair(60, 20);

	//40Hz
	 result[38] = std::make_pair(46, 40);
	 result[46] = std::make_pair(38, 40);
	 result[54] = std::make_pair(62, 40);
	 result[62] = std::make_pair(54, 40);

	//80Hz
	 result[42] = std::make_pair(58, 80);
	 result[58] = std::make_pair(42, 80);

	//160Hz
	 result[50] = std::make_pair(50, 160);
	 return result;
}

void ChannelBondingManager::SetChannelOption(uint16_t primary_ch,uint32_t max_width){
	this->max_width = max_width;
	this->primary_ch = primary_ch;
}

void ChannelBondingManager::MakePhys(const WifiPhyHelper &phy, Ptr<WifiPhy> primary, uint16_t ch_num, uint32_t channel_width, enum WifiPhyStandard standard){
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

void ChannelBondingManager::ResetPhys(){
	for(int i =0;i<8;++i){
		m_phys[ch_numbers[i]]->SetReceiveOkCallback (MakeNullCallback<void, Ptr<Packet>, double, WifiTxVector, enum WifiPreamble> ());
		m_phys[ch_numbers[i]]->SetReceiveErrorCallback (MakeNullCallback<void, Ptr<Packet>, double> ());
		m_phys[ch_numbers[i]] = 0;
	}
}

void ChannelBondingManager::ClearReceiveRecord(){
	/*
	 * received_channel[36] = false; received_channel[40] = false; received_channel[44] = false; received_channel[48] = false;
	 * received_channel[52] = false; received_channel[56] = false; received_channel[60] = false; received_channel[64] = false;
	 */

	for(int i =0;i<8;++i){
		received_channel[ch_numbers[i]] = false;
	}

	num_received = 0;
}


uint16_t ChannelBondingManager::CheckChBonding(uint16_t primary)
{
	std::pair<uint16_t,uint32_t> ch_info;

	uint32_t usable_ch = primary;

	while(true)
	{
		ch_info = ch_map.at(usable_ch);

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

bool ChannelBondingManager::CheckAllSubChannelIdle(uint16_t ch_num){
	std::pair<uint16_t,uint32_t> ch_info;
	ch_info = ch_map.at(ch_num);

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

uint16_t ChannelBondingManager::GetUsableBondedChannel(uint16_t primary)
{
	std::pair<uint16_t,uint32_t> ch_info;

	ch_info = ch_map.at(primary);


	uint16_t usable_ch = primary;

	while(true)
	{
		ch_info = ch_map.at(usable_ch);
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

bool ChannelBondingManager::CheckAllSubChannelReceived(uint16_t ch_num)
{
	std::pair<uint16_t,uint32_t> ch_info;
	ch_info = ch_map.at(ch_num);

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
uint16_t ChannelBondingManager::GetPrimaryCh()
{
	return primary_ch;
}
uint32_t ChannelBondingManager::GetMaxWidth()
{
	return max_width;
}
void ChannelBondingManager::SetPhysCallback()
{

	for(int i =0;i<8;++i){
		m_phys[ch_numbers[i]]->SetReceiveErrorCallback (MakeCallback (&ChannelBondingManager::ReceiveError, this));
	}
	m_phys[ch_numbers[36]]->SetReceiveOkCallback (MakeCallback (&ChannelBondingManager::Receive36Channel, this));
	m_phys[ch_numbers[40]]->SetReceiveOkCallback (MakeCallback (&ChannelBondingManager::Receive40Channel, this));
	m_phys[ch_numbers[44]]->SetReceiveOkCallback (MakeCallback (&ChannelBondingManager::Receive44Channel, this));
	m_phys[ch_numbers[48]]->SetReceiveOkCallback (MakeCallback (&ChannelBondingManager::Receive48Channel, this));
	m_phys[ch_numbers[52]]->SetReceiveOkCallback (MakeCallback (&ChannelBondingManager::Receive52Channel, this));
	m_phys[ch_numbers[56]]->SetReceiveOkCallback (MakeCallback (&ChannelBondingManager::Receive56Channel, this));
	m_phys[ch_numbers[60]]->SetReceiveOkCallback (MakeCallback (&ChannelBondingManager::Receive60Channel, this));
	m_phys[ch_numbers[64]]->SetReceiveOkCallback (MakeCallback (&ChannelBondingManager::Receive64Channel, this));
}

void ChannelBondingManager::Receive36Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble)
{
	ReceiveSubChannel(Packet,rxSnr,txVector,preamble,36);
}

void ChannelBondingManager::Receive40Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble)
{
	ReceiveSubChannel(Packet,rxSnr,txVector,preamble,40);
}

void ChannelBondingManager::Receive44Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble)
{
	ReceiveSubChannel(Packet,rxSnr,txVector,preamble,44);
}

void ChannelBondingManager::Receive48Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble)
{
	ReceiveSubChannel(Packet,rxSnr,txVector,preamble,48);
}

void ChannelBondingManager::Receive52Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble)
{
	ReceiveSubChannel(Packet,rxSnr,txVector,preamble,52);
}

void ChannelBondingManager::Receive56Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble)
{
	ReceiveSubChannel(Packet,rxSnr,txVector,preamble,56);
}

void ChannelBondingManager::Receive60Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble)
{
	ReceiveSubChannel(Packet,rxSnr,txVector,preamble,60);
}

void ChannelBondingManager::Receive64Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble)
{
	ReceiveSubChannel(Packet,rxSnr,txVector,preamble,64);
}

void ChannelBondingManager::ReceiveSubChannel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble, uint16_t ch_num)
{
	WifiMacHeader hdr;
	Packet->PeekHeader(hdr);

	num_received++;
	last_received_packet[ch_num] = Packet;

	if(ch_num == primary_ch ||
		hdr.GetAddr1() == m_mac->GetAddress()	  //2nd channal don't have nav
	)
		received_channel[ch_num] = true;


	//start timer
	if(hdr.IsRts())
	{
		if(receive_rts.IsRunning())
			receive_rts.Cancel();

		receive_rts = Simulator::Schedule(m_mac->GetSifs(),
												&ChannelBondingManager::ClearReceiveRecord, this
												);
	}

	else if(hdr.IsCts())
	{
		if(receive_cts.IsRunning())
			receive_cts.Cancel();

		receive_cts = Simulator::Schedule(m_mac->GetSifs(),
												&ChannelBondingManager::ClearReceiveRecord, this
												);
	}

	else
	{
		receive_orther = Simulator::Schedule (m_mac->GetSifs(),
													&ChannelBondingManager::ClearReceiveRecord, this
													);
	}



	if(hdr.IsRts() || hdr.IsCts() )
	{

		request_width = GetUsableWidth();


		if (request_width != 0)  //received primary
		{
			ManageReceived(Packet, rxSnr, txVector, preamble);
		}
	}

	else if(hdr.IsData())
	{
		if(num_received == (request_width/20) ){
			ManageReceived(Packet, rxSnr, txVector, preamble);
		}
	}

	else
	{
		ManageReceived(Packet, rxSnr, txVector, preamble);
		ClearReceiveRecord();
	}
}



bool ChannelBondingManager::CheckWidthUsable(uint32_t width)
{

	uint16_t check_ch = primary_ch;
	std::pair<uint16_t,uint32_t> ch_info;
	ch_info = ch_map.at(check_ch);

	if(width == 20)
	{
		return received_channel[check_ch];
	}

	for(uint32_t w = ch_info.second; w < width;)
	{
		if(!received_channel[ch_info.first])  //if second channel not received
			return false;

		check_ch = (check_ch + ch_info.first)/2;  //go to bigger channel

		ch_info = ch_map.at(check_ch);
	}

	return true;

}

uint32_t ChannelBondingManager::GetUsableWidth(void)
{

	uint32_t width = 0;

	for(uint32_t i = 20; i <= max_width; i *= 2)
	{
		if( CheckWidthUsable (i) )
			width = i;

		else
			break;
	}

	return width;
}

void ChannelBondingManager::ManageReceived (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble)
{


	m_mac->DeaggregateAmpduAndReceive(Packet, rxSnr,txVector, preamble);
}

}
