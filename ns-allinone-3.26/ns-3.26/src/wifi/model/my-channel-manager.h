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

namespace ns3 {

class ChannelManager : public Object
{
public:
  static TypeId GetTypeId (void);


  ChannelManager();
  virtual ~ChannelManager();
  uint16_t GetPrimaryCh();
  uint32_t GetMaxWidth();
  
  void SetChannelOption(uint16_t Primary_Ch,uint32_t Max_Width);
  void MakePhys(YansWifiPhyHelper phy, Ptr<WifiPhy> primary, uint16_t ch_num, uint32_t channel_width, enum WifiPhyStandard standard);

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

  void ChannelMapping() const;

  void ClearReceiveRecord();

  void SetPhysCallback();
  void ManageReceived  (Ptr<Packet> Packet, double rxSnr, WifiTxVector txVector, WifiPreamble preamble);



private:
  uint32_t max_width;
  uint32_t requist_width;
  uint16_t primary_ch;
  uint16_t ch_numbers[] = {36 , 40, 44, 48, 52, 56, 60, 64};
  Ptr<MacLow> m_mac;
  Ptr<Packet> last_primary_packet;
  
  std::map<uint16_t, Ptr<WifiPhy> > m_phys;

  std::map<int,bool> received_channel;

  EventId primary_receive_rts;
  EventId primary_receive_cts;

  uint16_t CheckChBonding(uint16_t primary);

  void SetMyMac(Ptr<MacLow> mac);

  bool CheckAllSubChannelIdle(uint16_t ch_num);

  uint16_t GetUsableChannelBonding(uint16_t primary);

  bool CheckAllSubChannelReceived(uint16_t ch_num);


  const std::map < uint16_t, std::pair<uint16_t, uint32_t> > ch_map;
  
  
};

}
#endif
