
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
  void MakePhy(YansWifiPhyHelper phy);

  void ResetPhy();
  
  void ReceiveOk();
  void ReceiveError();


  void SendPacket();



protect:
  uint32_t Max_Width;
  uint32_t Primary_Ch;
  
  
  Ptr<WifiPhy> m_phys[8];

  
  
  
  
  
};

}
#endif
