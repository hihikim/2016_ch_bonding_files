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
  void MakePhys(YansWifiPhyHelper phy);

  void ResetPhys();
  
  void ReceiveOk();
  void ReceiveError();


  void SendPacket();

  void ChannelMapping() const;

  void ClearReceiveRecord();



private:
  uint32_t max_width;
  uint32_t primary_ch;
  
  
  std::map<uint32_t, Ptr<WifiPhy> > m_phys;

  std::map<int,bool> received_channel;

  EventId primary_receive_rts;
  EventId primary_receive_cts;

  bool CheckChBonding(uint32_t primary);

  bool CheckAllSubChannelIdle(uint32_t ch_num);

  const std::map < uint32_t, std::pair<uint32_t, uint32_t> > ch_map;
  
  
};

}
#endif
