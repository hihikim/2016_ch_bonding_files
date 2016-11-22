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

typedef std::pair<uint16_t, uint32_t> ChannelInfo;

class ChannelBondingManager : public Object
{
public:
  static TypeId GetTypeId (void);


  ChannelBondingManager();
  virtual ~ChannelBondingManager();
  uint16_t GetPrimaryCh();
  uint32_t GetMaxWidth();
  
  void SetChannelOption(uint16_t Primary_Ch,uint32_t Max_Width);
  void MakePhys(const WifiPhyHelper &phy, Ptr<WifiPhy> primary, uint16_t ch_num, uint32_t channel_width, enum WifiPhyStandard standard);

  void CheckChannelBeforeSend(void);

  void ResetPhys();

  void Receive36Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble);
  void Receive40Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble);
  void Receive44Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble);
  void Receive48Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble);
  void Receive52Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble);
  void Receive56Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble);
  void Receive60Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble);
  void Receive64Channel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble);
  void ReceiveSubChannel (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble, uint16_t ch_num);

  void ReceiveOk (Ptr<Packet> packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble, bool ampduSubframe);
  void ReceiveError (Ptr<Packet> packet, double rxSnr);

  void SendPacket (Ptr<const Packet> packet, WifiTxVector txVector, enum WifiPreamble preamble, enum mpduType mpdutype);
  void SendPacket(Ptr<const Packet> packet, WifiTxVector txVector, enum WifiPreamble preamble);

  std::map<uint16_t, ChannelInfo> ChannelMapping();

  void ClearReceiveRecord();

  Ptr<Packet> ConvertPacket(Ptr<const Packet> packet);


  void SetPhysCallback();
  void ManageReceived (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble);
  void SetMyMac(Ptr<MacLow> mac);
  void NeedRtsCts(bool need);

  uint32_t GetConvertedSize(Ptr<const Packet> packet);


private:
  uint32_t max_width;

  uint32_t request_width;
  uint16_t request_ch;


  uint16_t primary_ch;
  uint16_t ch_numbers[8] = {36 , 40, 44, 48, 52, 56, 60, 64};

  Ptr<MacLow> m_mac;
  std::map< uint16_t, Ptr<Packet> > last_received_packet;
  std::map< uint16_t, Ptr<Packet> > packet_pieces;
  
  bool alloc_last_primary_hdr;
  WifiMacHeader last_primary_hdr;


  std::map<uint16_t, Ptr<WifiPhy> > m_phys;

  std::map<int,bool> received_channel;

  EventId clean_timer;
  bool need_rts_cts;

  uint16_t CheckChBonding(uint16_t primary);  //

  bool CheckAllSubChannelIdle(uint16_t ch_num);

  uint16_t GetUsableBondingChannel(uint16_t primary);

  bool CheckAllSubChannelReceived(uint16_t ch_num);

  uint16_t GetChannelWithWidth(uint32_t width);

  void CleanPacketPieces();
  std::vector<uint16_t> FindSubChannels(uint16_t ch_num);

  uint8_t GetNumberOfReceive();





  const std::map < uint16_t, ChannelInfo > ch_map;
  
  
};

}
#endif
