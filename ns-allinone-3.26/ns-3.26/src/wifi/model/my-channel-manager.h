#ifndef MY_CHANNEL_MANAGER_H
#define MY_CHANNEL_MANAGER_H

#include "wifi-mac-header.h"
#include "wifi-mode.h"
#include "wifi-phy.h"
#include "wifi-preamble.h"
#include "wifi-remote-station-manager.h"
#include "ctrl-headers.h"
#include "mgt-headers.h"
#include "block-ack-agreement.h"
#include "ns3/mac48-address.h"
#include "ns3/callback.h"
#include "ns3/event-id.h"
#include "ns3/packet.h"
#include "ns3/nstime.h"
#include "qos-utils.h"
#include "block-ack-cache.h"
#include "wifi-tx-vector.h"
#include "mpdu-aggregator.h"
#include "msdu-aggregator.h"
#include "mac-low.h"
#include "ns3/wifi-helper.h"
#include "ampdu-tag.h"



namespace ns3 {
class MacLow;

//typedef std::pair<uint16_t, uint32_t> ChannelInfo;

typedef struct
{
	uint16_t Secondary_Ch;	//secondary channel
	uint32_t Width;
	uint16_t L_CHD, R_CHD;	//children channels
	uint16_t Parent;	 //parrent nodes
}ChannelInfo;

typedef struct
{
	Time ErrorTime;
	std::vector< Ptr<Packet> > ErrorPacket;
	double rxSnr;
}error_packet_info;


class ChannelBondingManager : public Object
{
public:
	static TypeId GetTypeId (void);


	ChannelBondingManager();
	virtual ~ChannelBondingManager();
	uint16_t GetPrimaryCh();           // return primary channel number
	uint32_t GetMaxWidth();            // return max channel width
	uint32_t GetRequestWidth();        // return current channel width
	
	void SetChannelOption(uint16_t Primary_Ch,uint32_t Max_Width);  // set primary channel & max channel width
	void ChangeMaxWidth(uint32_t Max_Width);                        // change max channel width
	void MakePhys(const WifiPhyHelper &phy, Ptr<WifiPhy> primary, uint16_t ch_num, uint32_t channel_width, enum WifiPhyStandard standard);   // make subchannels phy

	void CheckChannelBeforeSend(void);                 // find widest available channel 

	void ResetPhys();                                 // remove all subchannels

	void SendPacket (Ptr<const Packet> packet, WifiTxVector txVector, enum WifiPreamble preamble, enum mpduType mpdutype); // send packet
	void SendPacket(Ptr<const Packet> packet, WifiTxVector txVector, enum WifiPreamble preamble);

	static std::map<uint16_t, ChannelInfo> ChannelMapping();             // channal bonding map

	void ClearReceiveRecord();                   // clear last_received_packet

	Ptr<Packet> ConvertPacket(Ptr<const Packet> packet);


	void SetPhysCallback();
	void ManageReceived (Ptr<Packet> packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble);
	void SetMyMac(Ptr<MacLow> mac);
	void NeedRtsCts(bool need);

	uint32_t GetConvertedSize(Ptr<const Packet> packet);

	std::map<uint16_t, Ptr<WifiPhy>> GetPhys();


private:
	Time last_receive_or_error_time;  //last time of error/receive packet(used in duplicated packet check)
	uint16_t RECount; //receive or error count
	uint16_t RECountLimit;  //required number of receive or error (0: unknown)
	double MinSnr; //minimum snr packets
	bool isErr, ErrReport; //true false of error occur, report

	Ptr<Packet> last_packet; //latest errored/received packet
	uint32_t max_width;  // maximum channel width parameter

	uint32_t request_width;  // channel width parameter of sending packet 
	uint16_t request_ch;  // channel number parameter of sending packet 


	uint16_t primary_ch;   //primary channel parameter
	std::vector<uint16_t> ch_numbers;  // all channel number of primary and sub channels
	error_packet_info error_packets;   // recevied error packets

	Ptr<MacLow> m_mac;   // maclow pointer
	std::map< uint16_t, Ptr<Packet> > last_received_packet;  //last receive packets
	std::map< uint16_t, Ptr<Packet> > packet_pieces;   // modified packet for send
	
	std::map<uint16_t, Ptr<WifiPhy> > m_phys;  // phy classes of sub channels & primary channel

	std::map<int,bool> received_channel;  // receive duplicated packet flag

	bool need_rts_cts;  // rts/cts flag

	uint16_t CheckChBonding(uint16_t primary);	// find suitable bonding channel (idle condition / no consider Rts-Cts width

	bool CheckAllSubChannelIdle(uint16_t ch_num);   // check all sub channels in ch_num(merged channel number ex: 36+40 = 38) are idle

	uint16_t GetUsableBondingChannel(uint16_t primary);  // get suitable bonding channel in rts-cts environment

	bool CheckAllSubChannelReceived(uint16_t ch_num);  //check all sub channels of ch_num(merged channel number ex: 36+40 = 38) receive rts-cts packet

	uint16_t GetChannelWithWidth(uint32_t width);    // width -> merged channel number

	void CleanPacketPieces();                                // clear storage of duplicated packets for sending
	std::vector<uint16_t> FindSubChannels(uint16_t ch_num);  // return all sub channels constituting merged channel(ch_num)

	uint8_t GetNumberOfReceive();                   // the number of recevied deplicated packet

	void SetUpChannelNumbers();                           // make sub channels using maximum channel width

	int CheckError(Ptr<const Packet> Packet);            // check error is occured in receiving time

	// when phy receive packet, this function operate
	void Receive1Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble);
	void Receive2Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble);
	void Receive3Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble);
	void Receive4Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble);
	void Receive5Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble);
	void Receive6Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble);
	void Receive7Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble);
	void Receive8Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble);
	void ReceiveSubChannel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble, uint16_t ch_num);

	void ReceiveOk (Ptr<Packet> packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble, bool ampduSubframe);

	// when phy receive error, this function operate
	void Error1Channel(Ptr<Packet> packet, double rxSnr);
	void Error2Channel(Ptr<Packet> packet, double rxSnr);
	void Error3Channel(Ptr<Packet> packet, double rxSnr);
	void Error4Channel(Ptr<Packet> packet, double rxSnr);
	void Error5Channel(Ptr<Packet> packet, double rxSnr);
	void Error6Channel(Ptr<Packet> packet, double rxSnr);
	void Error7Channel(Ptr<Packet> packet, double rxSnr);
	void Error8Channel(Ptr<Packet> packet, double rxSnr);

	void Error (Ptr<Packet> packet, double rxSnr, uint16_t ch_num);

	bool CheckItFirst(Ptr<Packet> packet);   // this function check the packet is same with previously received packet

	void ReceivePrimaryError (Ptr<Packet> packet, double rxSnr);   // error is occured in primary channel


	const std::map < uint16_t, ChannelInfo > ch_map;	// channel map for merged channel
};

}
#endif
