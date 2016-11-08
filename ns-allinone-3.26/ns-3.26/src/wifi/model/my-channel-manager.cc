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
ChannelBondingManager::~ChannelBondingManager()
{
	NS_LOG_FUNCTION (this);

}

/* static */
TypeId
ChannelBondingManager::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ChannelBondingManager")
    .SetParent<Object> ()
    .SetGroupName ("Wifi")
    .AddConstructor<ChannelBondingManager> ()
  ;
  return tid;
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

uint16_t ChannelBondingManager::GetUsableBondingChannel(uint16_t primary)
{
	std::pair<uint16_t,uint32_t> ch_info;

	ch_info = ch_map.at(primary);


	uint16_t usable_ch = primary;
	if(!CheckAllSubChannelReceived(primary))
		return 0;

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

	if(ch_num == primary_ch ||
		hdr.GetAddr1() == m_mac->GetAddress()	  //2nd channal don't have nav
	)
	{
		num_received++;
		received_channel[ch_num] = true;
		last_received_packet[ch_num] = Packet;

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
			if(receive_orther.IsRunning())
				receive_orther.Cancel();

			receive_orther = Simulator::Schedule (m_mac->GetSifs(),
														&ChannelBondingManager::ClearReceiveRecord, this
														);
		}



		if(hdr.IsRts() || hdr.IsCts() )
		{

			request_ch = GetUsableBondingChannel(primary_ch);

			if(request_ch == 0)
				request_width = 0;

			else
				request_width = ch_map.at(request_ch).second;


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
}



void ChannelBondingManager::ManageReceived (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble)
{
	WifiMacHeader hdr;
	last_received_packet[primary_ch]->PeekHeader(hdr);

	if(hdr.IsData())
	{
		std::ostringstream os;
		os.clear();
		Ptr<ns3::Packet> p;

		for(int i=0;i<8;i++)
		{
			if(received_channel[ch_numbers[i]]){
				p = last_received_packet[ch_numbers[i]];
				p->CopyData(&os, p->GetSize());
			}
		}

		uint8_t *b = new uint8_t[os.width()];
		const char* t_str = os.str().c_str();
		for(int i=0;i<os.width();i++)
		{
			b[i] = (uint8_t)t_str[i];
		}


		p = Create<ns3::Packet>(b, os.width());
		Packet = p;
	}


	m_mac->DeaggregateAmpduAndReceive(Packet, rxSnr,txVector, preamble);
}

void ChannelBondingManager::SendPacket (Ptr<const Packet> packet, WifiTxVector txVector, enum WifiPreamble preamble, enum mpduType mpdutype)
{
	WifiMacHeader hdr;
	packet->PeekHeader(hdr);

	if(hdr.IsData())
	{
		for(int i=0;i<8;i++)
		{
			if(packet_pieces[ch_numbers[i]] != 0)
				m_phys[ch_numbers[i]]->SendPacket(packet_pieces[ch_numbers[i]], txVector, preamble, mpdutype);

		}

		CleanPacketPieces();
	}

	else
	{
		for(int i=0;i<8;i++)
		{
			if(packet_pieces[ch_numbers[i]] != 0)
				m_phys[ch_numbers[i]]->SendPacket(packet, txVector, preamble, mpdutype);

		}
	}
}

void ChannelBondingManager::SendPacket (Ptr<const Packet> packet, WifiTxVector txVector, enum WifiPreamble preamble)
{
	SendPacket (packet, txVector, preamble, NORMAL_MPDU);
}


void ChannelBondingManager::ReceiveError(ns3::Ptr<ns3::Packet> packet, double rxSnr)
{
	m_mac->ReceiveError(packet, rxSnr);
}

Ptr<Packet> ChannelBondingManager::ConvertPacket(const Ptr<Packet> packet)
{
	CleanPacketPieces();
	WifiMacHeader hdr;
	packet->PeekHeader(hdr);

	if(hdr.IsRts())
	{
		request_ch = CheckChBonding(primary_ch);
		request_width = ch_map.at(request_ch).second;
	}

	CleanPacketPieces();

	std::vector<uint16_t> sub_chs = FindSubChannels(request_ch);
	Ptr<Packet> p;

	int using_channel = request_width / 20;

	std::ostringstream os;
	os.clear();
	packet->CopyData(&os, p->GetSize());

	int p_size = os.width();
	int unit = p_size / using_channel;

	uint8_t *b;
	const char* t_str = os.str().c_str();
	int point = 0;


	for(std::vector<uint16_t>::iterator i = sub_chs.begin() ;
		i != sub_chs.end() ;
		++i )
	{
		if(b != NULL)
			delete b;

		if(p_size - point < unit)
			b = new uint8_t[os.width()];

		else
			b = new uint8_t[unit];

		for(int j = 0; j < (int)(sizeof(b) / sizeof(b[0])) ; ++j)
		{
			b[j] = (uint8_t)t_str[point];
			++point;
		}

		p = Create<ns3::Packet>(b, sizeof(b) / sizeof(b[0]));

		packet_pieces[*i] = p;
	}


	return packet_pieces[primary_ch];
}

void ChannelBondingManager::CleanPacketPieces()
{
	for(std::map< uint16_t, Ptr<Packet> >::iterator i = packet_pieces.begin();
		i != packet_pieces.end();
		++i)
	{
		i->second = 0;
	}
}

std::vector<uint16_t> ChannelBondingManager::FindSubChannels(uint16_t ch_num)
{
	ChannelInfo ch_info = ch_map.at(ch_num);
	std::vector<uint16_t> result,temp;

	if(ch_info.second == 20)
	{
		result.push_back(ch_num);
	}

	else
	{
		uint16_t side_num = ch_info.second / 20;
		temp = FindSubChannels(ch_num - side_num);
		result.reserve(result.size() + temp.size());
		result.insert(result.end(),temp.begin(),temp.end());

		temp = FindSubChannels(ch_num + side_num);
		result.reserve(result.size() + temp.size());
		result.insert(result.end(),temp.begin(),temp.end());
	}

	return result;
}

}
