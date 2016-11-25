#include "my-channel-manager.h"

#include "ns3/assert.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/tag.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/double.h"
#include "mpdu-aggregator.h"
#include "msdu-aggregator.h"



namespace ns3 {


NS_LOG_COMPONENT_DEFINE ("ChannelBondingManager");

ChannelBondingManager::ChannelBondingManager()
: ch_map(ChannelMapping())
{
	need_rts_cts = false;
	max_width = 160;
	primary_ch = 36;
	request_width = 160;

	alloc_last_primary_hdr = false;

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

std::map<uint16_t, ChannelInfo >  ChannelBondingManager::ChannelMapping()
{
	 std::map<uint16_t, ChannelInfo >  result;                            //make channel map
	 ChannelInfo ch_info;

	 /*
	  *                              5.17-5.33 GHz
	  *
	  *             160                        50
	  *                                    /        \
	  *             80                   42         58
	  *                                /   \      /    \
	  *             40                38   46    54    62
	  *                              / \   / \   / \   / \
	  *             20              36 40 44 48 52 56 60 64
	  */
     //20Hz
	 ch_info.Width = 20;

	 ch_info.Parent = 38;
	 ch_info.Secondary_Ch = 40; ch_info.L_CHD = 36; ch_info.R_CHD = 36; result[36] = ch_info;
	 ch_info.Secondary_Ch = 36; ch_info.L_CHD = 40; ch_info.R_CHD = 40; result[40] = ch_info;

	 ch_info.Parent = 46;
	 ch_info.Secondary_Ch = 48; ch_info.L_CHD = 44; ch_info.R_CHD = 44; result[44] = ch_info;
	 ch_info.Secondary_Ch = 44; ch_info.L_CHD = 48; ch_info.R_CHD = 48; result[48] = ch_info;

	 ch_info.Parent = 54;
	 ch_info.Secondary_Ch = 56; ch_info.L_CHD = 52; ch_info.R_CHD = 52; result[52] = ch_info;
	 ch_info.Secondary_Ch = 52; ch_info.L_CHD = 56; ch_info.R_CHD = 56; result[56] = ch_info;

	 ch_info.Parent = 62;
	 ch_info.Secondary_Ch = 64; ch_info.L_CHD = 60; ch_info.R_CHD = 60; result[60] = ch_info;
	 ch_info.Secondary_Ch = 60; ch_info.L_CHD = 64; ch_info.R_CHD = 64; result[64] = ch_info;

	//40Hz
	 ch_info.Width = 40;

	 ch_info.Parent = 42;
	 ch_info.Secondary_Ch = 46; ch_info.L_CHD = 36; ch_info.R_CHD = 40; result[38] = ch_info;
	 ch_info.Secondary_Ch = 38; ch_info.L_CHD = 44; ch_info.R_CHD = 48; result[46] = ch_info;

	 ch_info.Parent = 58;
	 ch_info.Secondary_Ch = 62; ch_info.L_CHD = 52; ch_info.R_CHD = 56; result[54] = ch_info;
	 ch_info.Secondary_Ch = 54; ch_info.L_CHD = 60; ch_info.R_CHD = 64; result[62] = ch_info;

	//80Hz
	 ch_info.Width = 80;

	 ch_info.Parent = 50;
	 ch_info.Secondary_Ch = 58; ch_info.L_CHD = 38; ch_info.R_CHD = 46; result[42] = ch_info;
	 ch_info.Secondary_Ch = 42; ch_info.L_CHD = 54; ch_info.R_CHD = 62; result[58] = ch_info;

	//160Hz
	 ch_info.Width = 160;

	 ch_info.Parent = 50;
	 ch_info.Secondary_Ch = 50; ch_info.L_CHD = 42; ch_info.R_CHD = 58; result[50] = ch_info;

	 /*
	  *                 5.49-5.725GHz
	  *             160                         114
	  *                                    /            \
	  *             80                   106            122             138
	  *                                /     \         /    \         /      \
	  *             40                102    110      118   126     134      142
	  *                              /  \   /  \     /  \   /  \    /  \     /  \
	  *             20             100 104 108 112 116 120 124 128 132 136 140 144
	  */
	 //20Hz
	 ch_info.Width = 20;

	 ch_info.Parent = 102;
	 ch_info.Secondary_Ch = 104; ch_info.L_CHD = 100; ch_info.R_CHD = 100; result[100] = ch_info;
	 ch_info.Secondary_Ch = 100; ch_info.L_CHD = 104; ch_info.R_CHD = 104; result[104] = ch_info;

	 ch_info.Parent = 110;
	 ch_info.Secondary_Ch = 112; ch_info.L_CHD = 108; ch_info.R_CHD = 108; result[108] = ch_info;
	 ch_info.Secondary_Ch = 108; ch_info.L_CHD = 112; ch_info.R_CHD = 112; result[112] = ch_info;

	 ch_info.Parent = 118;
	 ch_info.Secondary_Ch = 120; ch_info.L_CHD = 116; ch_info.R_CHD = 116; result[116] = ch_info;
	 ch_info.Secondary_Ch = 116; ch_info.L_CHD = 120; ch_info.R_CHD = 120; result[120] = ch_info;

	 ch_info.Parent = 126;
	 ch_info.Secondary_Ch = 128; ch_info.L_CHD = 124; ch_info.R_CHD = 124; result[124] = ch_info;
	 ch_info.Secondary_Ch = 124; ch_info.L_CHD = 128; ch_info.R_CHD = 128; result[128] = ch_info;

	 ch_info.Parent = 134;
	 ch_info.Secondary_Ch = 136; ch_info.L_CHD = 132; ch_info.R_CHD = 132; result[132] = ch_info;
	 ch_info.Secondary_Ch = 132; ch_info.L_CHD = 136; ch_info.R_CHD = 136; result[136] = ch_info;

	 ch_info.Parent = 142;
	 ch_info.Secondary_Ch = 144; ch_info.L_CHD = 140; ch_info.R_CHD = 140; result[140] = ch_info;
	 ch_info.Secondary_Ch = 140; ch_info.L_CHD = 144; ch_info.R_CHD = 144; result[144] = ch_info;

	//40Hz
	 ch_info.Width = 40;

	 ch_info.Parent = 106;
	 ch_info.Secondary_Ch = 110; ch_info.L_CHD = 100; ch_info.R_CHD = 104; result[102] = ch_info;
	 ch_info.Secondary_Ch = 102; ch_info.L_CHD = 108; ch_info.R_CHD = 112; result[110] = ch_info;

	 ch_info.Parent = 122;
	 ch_info.Secondary_Ch = 126; ch_info.L_CHD = 116; ch_info.R_CHD = 120; result[118] = ch_info;
	 ch_info.Secondary_Ch = 118; ch_info.L_CHD = 124; ch_info.R_CHD = 128; result[126] = ch_info;

	 ch_info.Parent = 138;
	 ch_info.Secondary_Ch = 142; ch_info.L_CHD = 132; ch_info.R_CHD = 136; result[134] = ch_info;
	 ch_info.Secondary_Ch = 134; ch_info.L_CHD = 140; ch_info.R_CHD = 144; result[142] = ch_info;

	//80Hz
	 ch_info.Width = 80;

	 ch_info.Parent = 114;
	 ch_info.Secondary_Ch = 122; ch_info.L_CHD = 102; ch_info.R_CHD = 110; result[106] = ch_info;
	 ch_info.Secondary_Ch = 106; ch_info.L_CHD = 118; ch_info.R_CHD = 126; result[122] = ch_info;

	 ch_info.Parent = 138;
	 ch_info.Secondary_Ch = 138; ch_info.L_CHD = 134; ch_info.R_CHD = 142; result[138] = ch_info;

	//160Hz
	 ch_info.Width = 160;

	 ch_info.Parent = 114;
	 ch_info.Secondary_Ch = 114; ch_info.L_CHD = 106; ch_info.R_CHD = 122; result[114] = ch_info;

	 /*
	  *                            5.745-5.825GHz
	  *             80                   155
	  *                                /     \
	  *             40                151    159
	  *                              /  \    /  \
	  *             20             149 153 157 161 165
	  */
	 //20Hz
	 ch_info.Width = 20;

	 ch_info.Parent = 151;
	 ch_info.Secondary_Ch = 153; ch_info.L_CHD = 149; ch_info.R_CHD = 149; result[149] = ch_info;
	 ch_info.Secondary_Ch = 149; ch_info.L_CHD = 153; ch_info.R_CHD = 153; result[153] = ch_info;

	 ch_info.Parent = 159;
	 ch_info.Secondary_Ch = 161; ch_info.L_CHD = 157; ch_info.R_CHD = 157; result[157] = ch_info;
	 ch_info.Secondary_Ch = 157; ch_info.L_CHD = 161; ch_info.R_CHD = 161; result[161] = ch_info;

	 ch_info.Parent = 165;
	 ch_info.Secondary_Ch = 165; ch_info.L_CHD = 165; ch_info.R_CHD = 165; result[165] = ch_info;

	//40Hz
	 ch_info.Width = 40;

	 ch_info.Parent = 155;
	 ch_info.Secondary_Ch = 159; ch_info.L_CHD = 149; ch_info.R_CHD = 153; result[151] = ch_info;
	 ch_info.Secondary_Ch = 151; ch_info.L_CHD = 157; ch_info.R_CHD = 161; result[159] = ch_info;

	//80Hz
	 ch_info.Width = 80;

	 ch_info.Parent = 155;
	 ch_info.Secondary_Ch = 155; ch_info.L_CHD = 151; ch_info.R_CHD = 159; result[155] = ch_info;



	 return result;
}

void ChannelBondingManager::SetChannelOption(uint16_t primary_ch,uint32_t max_width){
	this->max_width = max_width;                        //set max width and primary_ch
	this->primary_ch = primary_ch;
}

void ChannelBondingManager::MakePhys(const WifiPhyHelper &phy, Ptr<WifiPhy> primary, uint16_t ch_num, uint32_t channel_width, enum WifiPhyStandard standard){
   SetChannelOption(ch_num, channel_width);                          //make each phys


   Ptr<NetDevice> device = primary->GetDevice();
   Ptr<Node> node = device->GetNode();


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

	   //m_phys[ch_numbers[i]]->EnableChannelBonding(true);
	   m_phys[ch_numbers[i]]->SetChannelNumber(ch_numbers[i]);
   }
}

void ChannelBondingManager::ResetPhys(){
	for(int i =0;i<8;++i){                                                                    //reset phys
		m_phys[ch_numbers[i]]->SetReceiveOkCallback (MakeNullCallback<void, Ptr<Packet>, double, WifiTxVector, enum WifiPreamble> ());
		m_phys[ch_numbers[i]]->SetReceiveErrorCallback (MakeNullCallback<void, Ptr<Packet>, double> ());
		m_phys[ch_numbers[i]] = 0;
	}
}

void ChannelBondingManager::ClearReceiveRecord(){
	for(int i =0;i<8;++i){                               //clear received record
		received_channel[ch_numbers[i]] = false;
		last_received_packet[ch_numbers[i]] = 0;
	}

}


uint16_t ChannelBondingManager::CheckChBonding(uint16_t primary)
{
	if (primary == 0)                                      //find suitable bonding channel (idle condition / no consider Rts-Cts width
		NS_FATAL_ERROR("Wrong Channel Number");

	ChannelInfo ch_info;
	std::map<uint16_t, ChannelInfo>::const_iterator ch_i;

	uint32_t usable_ch = primary;

	while(true)
	{
		ch_i = ch_map.find(usable_ch);

		if(ch_i == ch_map.end())
		{
			std::ostringstream oss;
			oss << usable_ch<<" is Wrong Channel Number";
			NS_FATAL_ERROR(oss.str());
		}

		ch_info = ch_i->second;


		if( ch_info.Width == max_width || ch_info.Parent == usable_ch)
			break;

		else if( CheckAllSubChannelIdle( ch_info.Secondary_Ch ))
		{
			usable_ch = ch_info.Parent;
		}

		else
			break;
	}

	return usable_ch;
}

bool ChannelBondingManager::CheckAllSubChannelIdle(uint16_t ch_num){
	ChannelInfo ch_info;                     //find all subchannels are idle
	std::map<uint16_t, ChannelInfo>::const_iterator ch_i = ch_map.find(ch_num);

	if(ch_i == ch_map.end())
	{
		std::ostringstream oss;
		oss << ch_num<<" is Wrong Channel Number";
		NS_FATAL_ERROR(oss.str());
	}

	ch_info = ch_i->second;

	uint32_t ch_width = ch_info.Width;

	if(ch_width == 20){
		if(m_phys[ch_num]->IsStateIdle ())
			return true;

		else
			return false;
	}

	else
	{
		if(CheckAllSubChannelIdle(ch_info.L_CHD) && CheckAllSubChannelIdle (ch_info.R_CHD) )
			return true;

		else
			return false;
	}
}

uint16_t ChannelBondingManager::GetUsableBondingChannel(uint16_t primary)                //get bonding channel in rts-cts environment
{
	ChannelInfo ch_info;
	std::map<uint16_t, ChannelInfo>::const_iterator ch_i = ch_map.find(primary);

	std::ostringstream oss;
	if(ch_i == ch_map.end())
	{
		oss << primary<<" is Wrong Channel Number";
		NS_FATAL_ERROR(oss.str());
	}

	ch_info = ch_i->second;


	uint16_t usable_ch = primary;

	if(!need_rts_cts)
	{
		CheckChannelBeforeSend();
		return request_ch;
	}

	if(!CheckAllSubChannelReceived(primary))
		return 0;

	while(true)
	{
		//ch_info = ch_map.at(usable_ch);
		ch_i = ch_map.find(usable_ch);

		if(ch_i == ch_map.end())
		{
			oss.clear();
			oss << usable_ch<<" is Wrong Channel Number";
			NS_FATAL_ERROR(oss.str());
		}


		if( ch_info.Width >= max_width || ch_info.Parent == usable_ch)
			break;

		else if( CheckAllSubChannelReceived( ch_info.Secondary_Ch ))
		{
			usable_ch = ch_info.Parent;
		}

		else
			break;
	}

	return usable_ch;
}

bool ChannelBondingManager::CheckAllSubChannelReceived(uint16_t ch_num)                   //check all sub channel receive rts-cts
{
	ChannelInfo ch_info;
	std::map<uint16_t, ChannelInfo>::const_iterator ch_i = ch_map.find(ch_num);

	if(ch_i == ch_map.end())
	{
		std::ostringstream oss;
		oss << ch_num<<" is Wrong Channel Number";
		NS_FATAL_ERROR(oss.str());
	}

	ch_info = ch_i->second;

	uint32_t ch_width = ch_info.Width;

	if(ch_width == 20){
		if(received_channel[ch_num])
		{
			WifiMacHeader hdr1,hdr2;
			if(received_channel[primary_ch])
			{
				last_received_packet[ch_num]->PeekHeader(hdr1);
				last_received_packet[primary_ch]->PeekHeader(hdr2);
				if(hdr1.GetAddr1() == hdr2.GetAddr1() &&
					hdr1.GetType() == hdr2.GetType()
				   )
					return true;

				else
					return false;
			}
			else
				return false;
		}
		else
			return false;
	}

	else
	{
		if(CheckAllSubChannelReceived(ch_info.L_CHD) && CheckAllSubChannelReceived (ch_info.R_CHD) )
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

uint32_t ChannelBondingManager::GetRequestWidth()
{
	return request_width;
}

void ChannelBondingManager::SetPhysCallback()
{

	for(int i =0;i<8;++i){
		m_phys[ch_numbers[i]]->SetReceiveErrorCallback (MakeCallback (&ChannelBondingManager::ReceiveError, this));
	}

	m_phys[ch_numbers[0]]->SetReceiveOkCallback (MakeCallback (&ChannelBondingManager::Receive36Channel, this));
	m_phys[ch_numbers[1]]->SetReceiveOkCallback (MakeCallback (&ChannelBondingManager::Receive40Channel, this));
	m_phys[ch_numbers[2]]->SetReceiveOkCallback (MakeCallback (&ChannelBondingManager::Receive44Channel, this));
	m_phys[ch_numbers[3]]->SetReceiveOkCallback (MakeCallback (&ChannelBondingManager::Receive48Channel, this));
	m_phys[ch_numbers[4]]->SetReceiveOkCallback (MakeCallback (&ChannelBondingManager::Receive52Channel, this));
	m_phys[ch_numbers[5]]->SetReceiveOkCallback (MakeCallback (&ChannelBondingManager::Receive56Channel, this));
	m_phys[ch_numbers[6]]->SetReceiveOkCallback (MakeCallback (&ChannelBondingManager::Receive60Channel, this));
	m_phys[ch_numbers[7]]->SetReceiveOkCallback (MakeCallback (&ChannelBondingManager::Receive64Channel, this));
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
	bool isampdu = false;                              //manage 1 subchannel receive packet
	WifiMacHeader hdr;
	AmpduTag ampdu;
	AmpduSubframeHeader ampduhdr;

	if(Packet->PeekPacketTag(ampdu))
	{
		isampdu = true;
		Packet->RemoveHeader(ampduhdr);
		Packet->PeekHeader(hdr);
		Packet->AddHeader(ampduhdr);
		alloc_last_primary_hdr = false;

		if(ch_num == primary_ch){
			alloc_last_primary_hdr = true;

			last_primary_hdr = hdr;
		}
	}

	else
	{
		isampdu = false;
		Packet->PeekHeader(hdr);

		if(ch_num == primary_ch){
			alloc_last_primary_hdr = true;

			last_primary_hdr = hdr;
		}
	}

	received_channel[ch_num] = true;
	last_received_packet[ch_num] = Packet;


	//start timer
	if(ch_num == primary_ch)
	{
		if(clean_timer.IsRunning())
			clean_timer.Cancel();

		clean_timer = Simulator::Schedule(m_mac->GetSifs(),
												&ChannelBondingManager::ClearReceiveRecord, this
												);
	}

	if(hdr.GetAddr1() == m_mac->GetAddress())
	{
		if(isampdu)
		{

			//need_rts_cts = false;
			if(!need_rts_cts &&
				ch_num == primary_ch)
			{
				request_width = txVector.GetChannelWidth();
				request_ch = GetChannelWithWidth(request_width);
			}

			uint32_t recevied_num = GetNumberOfReceive();

			if(recevied_num != 0 &&
				recevied_num == (request_width/20) ){
				//std::cout<<"ho~\n";
				ManageReceived(Packet, rxSnr, txVector, preamble);
			}
		}

		else
		{
		   if(hdr.IsRts() || hdr.IsCts() )
			{
				if(primary_ch == ch_num)
				{
					NeedRtsCts(true);
				}

				request_ch = GetUsableBondingChannel(primary_ch);

				if(request_ch == 0)
					request_width = 0;

				else
					request_width = ch_map.find(request_ch)->second.Width;
			}

		   if(ch_num == primary_ch){
				ManageReceived(Packet, rxSnr, txVector, preamble);
			}
		}
	}

	else
	{
		if(ch_num == primary_ch){
			ManageReceived(Packet, rxSnr, txVector, preamble);
		}
	}
}



void ChannelBondingManager::ManageReceived (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble)
{
	bool isampdu = false;                   //marge receive packet and forwardup to mac-low
	AmpduTag ampdu;
	WifiMacHeader hdr;
	AmpduSubframeHeader ampduhdr;


	if(Packet->PeekPacketTag(ampdu))
	{
		Packet->PeekHeader(ampduhdr);
		isampdu = true;
	}
	else
	{
		Packet->PeekHeader(hdr);
		isampdu = false;
	}

	Ptr<ns3::Packet> p;


	/*if(isampdu)
	{
		for(int i=0;i<8;i++)
		{
			if(received_channel[ch_numbers[i]])
			{
				temp_p = last_received_packet[ch_numbers[i]]->Copy();

				if(ch_numbers[i] == primary_ch)
				{
					temp_p->RemoveHeader(ampduhdr);
					temp_p->RemoveHeader(hdr);
				}
				else
				{
					temp_p->RemoveHeader(etc_ampdu);
					temp_p->RemoveHeader(etc);
				}

				if(p == 0)
					p = temp_p;

				else
					p->AddAtEnd(temp_p);
			}
		}
		p->AddHeader(hdr);
		p->AddHeader(ampduhdr);
	}

	else*/
	//{
		p = Packet->Copy();
	//}

	if(isampdu || (!hdr.IsRts() && !hdr.IsCts()))
		ClearReceiveRecord();

	m_mac->DeaggregateAmpduAndReceive(p, rxSnr,txVector, preamble);
}

void ChannelBondingManager::SendPacket (Ptr<const Packet> packet, WifiTxVector txVector, enum WifiPreamble preamble, enum mpduType mpdutype)
{                                               //send packet
	ConvertPacket(packet);                       //split packet

	ClearReceiveRecord();


	//txVector.SetChannelWidth(request_width);

	for(int i=0;i<8;i++)
	{
		if(packet_pieces[ch_numbers[i]] != 0)
		{
			m_phys[ch_numbers[i]]->SendPacket(packet_pieces[ch_numbers[i]], txVector, preamble, mpdutype);
		}
	}


	CleanPacketPieces();

}

void ChannelBondingManager::SendPacket (Ptr<const Packet> packet, WifiTxVector txVector, enum WifiPreamble preamble)
{
	SendPacket (packet, txVector, preamble, NORMAL_MPDU);
}


void ChannelBondingManager::ReceiveError(ns3::Ptr<ns3::Packet> packet, double rxSnr)
{
	m_mac->ReceiveError(packet, rxSnr);
}

void ChannelBondingManager::CheckChannelBeforeSend()   //set bonding channel in no rts-cts environment
{
	request_ch = CheckChBonding(primary_ch);
	request_width = ch_map.find(request_ch)->second.Width;
}

Ptr<Packet> ChannelBondingManager::ConvertPacket(Ptr<const Packet> packet)   //split the packet
{
	CleanPacketPieces();

	/*bool isampdu = false;
	AmpduTag ampdu;
	WifiMacHeader hdr;
	AmpduSubframeHeader ampduhdr;
	Ptr<Packet> origin_p = packet->Copy();

	if(origin_p->PeekPacketTag(ampdu))
	{
		isampdu = true;
		origin_p->RemoveHeader(ampduhdr);
		origin_p->RemoveHeader(hdr);
	}

	else
	{
		isampdu = false;
		origin_p->PeekHeader(hdr);
	}


	if(request_ch == 0){  //이상
		if(!need_rts_cts)
			CheckChannelBeforeSend();
		else
			return packet->Copy();
	}


	std::vector<uint16_t> sub_chs = FindSubChannels(request_ch);

	int using_channel = request_width / 20;

	if(isampdu)
	{
		Ptr<Packet> p;

		int size_p = origin_p->GetSize();

		int point = 0;
		int unit = size_p / using_channel;


		for(std::vector<uint16_t>::iterator i = sub_chs.begin() ;
			i != sub_chs.end() ;
			++i )
		{

			if((size_p - point)%using_channel ==0 )
			{
				p = origin_p->CreateFragment(point, unit);
				point += unit;
				--using_channel;
			}

			else
			{
				p = origin_p->CreateFragment(point,unit+1);
				point += (1+unit);
				--using_channel;
			}
			p->AddHeader(hdr);
			p->AddHeader(ampduhdr);
			packet_pieces[*i] = p;
		}
	}

	else*/
	{
		/*std::cout<<"------------------channel num : "<<primary_ch<<"---------------------------\n";
		std::cout<<"rts/cts : "<<(need_rts_cts ? "true" : "false")<<std::endl;
		std::cout<<"width : "<<request_width<<std::endl;
		std::cout<<"max width : "<<request_width<<std::endl;*/
		WifiMacHeader hdr;
		packet->PeekHeader(hdr);
		//std::cout<<hdr.GetTypeString()<<std::endl;
		Ptr<Packet> origin_p = packet->Copy();
		std::vector<uint16_t> sub_chs = FindSubChannels(request_ch);
		for(std::vector<uint16_t>::iterator i = sub_chs.begin() ;
			i != sub_chs.end() ;
			++i )
		{
			packet_pieces[*i] = origin_p;
		}
	}


	return packet_pieces[primary_ch];
}

void ChannelBondingManager::CleanPacketPieces()           //clear the storage for splited packet
{
	for(std::map< uint16_t, Ptr<Packet> >::iterator i = packet_pieces.begin();
		i != packet_pieces.end();
		++i)
	{
		i->second = 0;
	}
}

std::vector<uint16_t> ChannelBondingManager::FindSubChannels(uint16_t ch_num)    //find all subchannel composing the bonding channel
{
	std::map<uint16_t, ChannelInfo>::const_iterator ch_i = ch_map.find(ch_num);

	if(ch_i == ch_map.end())
	{
		std::ostringstream oss;
		oss<<ch_num<<" is wrong channel number";
		NS_FATAL_ERROR(oss.str());
	}

	ChannelInfo ch_info = ch_i->second;
	std::vector<uint16_t> result,temp;

	if(ch_info.Width == 20)
	{
		result.push_back(ch_num);
	}

	else
	{
		temp = FindSubChannels(ch_info.L_CHD);
		result.reserve(result.size() + temp.size());
		result.insert(result.end(),temp.begin(),temp.end());

		temp = FindSubChannels(ch_info.R_CHD);
		result.reserve(result.size() + temp.size());
		result.insert(result.end(),temp.begin(),temp.end());
	}

	return result;
}

uint16_t ChannelBondingManager::GetChannelWithWidth(uint32_t width)      //find bonding channel number using width
{
	uint16_t result = primary_ch;
	ChannelInfo ch_info;
	//std::map<uint16_t, ChannelInfo>::iterator ch_i;
	//ChannelInfo ch_info = *ch_i;

	while(true)
	{
		ch_info = ch_map.find(result)->second;

		if(ch_info.Width == width)
			return result;

		else if(ch_info.Parent == result)  //searched top
			return 0;

		else
			result = ch_info.Parent;
	}
}

void ChannelBondingManager::NeedRtsCts(bool need){   //set using rts-cts
	need_rts_cts = need;
}

uint8_t ChannelBondingManager::GetNumberOfReceive()       //find number of received packet in each subchannel
{
	uint8_t result = 0;

	if(!received_channel[primary_ch])
		return result;

	else
	{
		Ptr<Packet> p;
		p = last_received_packet[primary_ch]->Copy();
		AmpduTag ampdu;
		WifiMacHeader hdr;
		AmpduSubframeHeader ampduhdr;
		bool primary_mpdu_tf, mpdu_tf;
		Mac48Address primary_sender_addr;
		WifiMacType primary_hdr_type;

		if(p->PeekPacketTag(ampdu))
		{
			p->RemoveHeader(ampduhdr);
			p->RemoveHeader(hdr);
			primary_mpdu_tf = true;
		}

		else
		{
			p->RemoveHeader(hdr);
			primary_mpdu_tf = false;
		}

		p = 0;
		primary_sender_addr = hdr.GetAddr2();
		primary_hdr_type = hdr.GetType();

		for(int i=0;i<8;i++)
		{
			if(received_channel[ch_numbers[i]])
			{
				p = last_received_packet[ch_numbers[i]]->Copy();
				if(p->PeekPacketTag(ampdu))
				{
					p->RemoveHeader(ampduhdr);
					p->RemoveHeader(hdr);
					mpdu_tf = true;
				}

				else
				{
					p->RemoveHeader(hdr);
					mpdu_tf = false;
				}

				p = 0;

				if(primary_mpdu_tf == mpdu_tf)
				{
					if(primary_sender_addr == hdr.GetAddr2() &&
							primary_hdr_type == hdr.GetType())
					{
						++result;
					}

					else
					{
						received_channel[ch_numbers[i]] = false;
						last_received_packet[ch_numbers[i]] = 0;
					}
				}
				else
				{
					received_channel[ch_numbers[i]] = false;
					last_received_packet[ch_numbers[i]] = 0;
				}
			}
		}
		return result;
	}
}

uint32_t ChannelBondingManager::GetConvertedSize(Ptr<const Packet> packet)    //get converted packet size
{


  WifiMacHeader hdr;
  AmpduSubframeHeader ampduhdr;
  Ptr<Packet> origin_p = packet->Copy();


  origin_p->RemoveHeader(ampduhdr);
  origin_p->RemoveHeader(hdr);



  int using_channel = request_width / 20;


  Ptr<Packet> p;

  int size_p = origin_p->GetSize();
  int unit = size_p / using_channel;


  if(size_p % using_channel ==0 )
  {
    p = origin_p->CreateFragment(0, unit);
  }

  else
  {
    p = origin_p->CreateFragment(0,unit+1);
  }
  p->AddHeader(hdr);
  p->AddHeader(ampduhdr);

  return p->GetSize();

}


}
