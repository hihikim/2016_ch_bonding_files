#include <map>
#include <utility>

#ifndef MY_CHANNEL_MANAGER_H
#define MY_CHANNEL_MANAGER_H
namespace ns3 {

class ChannelManager : public Object
{
public:
  static TypeId GetTypeId (void);


  ChannelManager();
  virtual ~ChannelManager();
  uint32_t GetPrimaryCh();
  uint32_t GetMaxWidth();
  
  void SetChannelOption(uint32_t Primary_Ch,uint32_t Max_Width);
  void MakePhys(YansWifiPhyHelper phy, Ptr<WifiPhy> primary, uint32_t ch_num, uint32_t channel_width, enum WifiPhyStandard standard);

  void ResetPhys();
  
  void ReceiveOk();
  void ReceiveError();

  void SendPacket (Ptr<const Packet> packet, WifiTxVector txVector, enum WifiPreamble preamble, enum mpduType mpdutype);
  void SendPacket(Ptr<const Packet> packet, WifiTxVector txVector, enum WifiPreamble preamble);

  void ChannelMapping() const;

  void ClearReceiveRecord();

  void SetPhysCallback();



private:
  uint32_t max_width;
  uint32_t primary_ch;
  uint32_t ch_numbers[] = {36 , 40, 44, 48, 52, 56, 60, 64};
  
  std::map<uint32_t, Ptr<WifiPhy> > m_phys;

  std::map<int,bool> received_channel;

  EventId primary_receive_rts;
  EventId primary_receive_cts;

  uint32_t CheckChBonding(uint32_t primary);

  bool CheckAllSubChannelIdle(uint32_t ch_num);

  uint32_t GetUsableChannelBonding(uint32_t primary);

  bool CheckAllSubChannelReceived(uint32_t ch_num);

  const std::map < uint32_t, std::pair<uint32_t, uint32_t> > ch_map;
  
  
};

}
#endif
