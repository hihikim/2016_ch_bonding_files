
#ifndef MY_CHANNEL_MANAGER_H
#define MY_CHANNEL_MANAGER_H
namespace ns3 {

class ChannelManager : public Object
{
public:
  static TypeId GetTypeId (void);


  ChannelManager();
  virtual ~ChannelManager();



};

}
#endif
