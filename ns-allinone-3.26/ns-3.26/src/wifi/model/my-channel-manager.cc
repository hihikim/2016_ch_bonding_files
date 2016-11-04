#include "my-channel-manager.h"
#include "ns3/log.h"


namespace ns3 {


NS_LOG_COMPONENT_DEFINE ("ChannelManager");

ChannelManager::ChannelManager()
: ch_map()
{
	max_width = 160;
	primary_ch = 36;

	ChannelMapping();
}

void ChannelManager::ChannelMapping() const
{
	//20Hz
	ch_map[36] = std::make_pair(40, 20);
	ch_map[40] = std::make_pair(36, 20);
	ch_map[44] = std::make_pair(48, 20);
	ch_map[48] = std::make_pair(44, 20);
	ch_map[52] = std::make_pair(56, 20);
	ch_map[56] = std::make_pair(52, 20);
	ch_map[60] = std::make_pair(64, 20);
	ch_map[64] = std::make_pair(60, 20);

	//40Hz
	ch_map[38] = std::make_pair(46, 40);
	ch_map[46] = std::make_pair(38, 40);
	ch_map[54] = std::make_pair(62, 40);
	ch_map[62] = std::make_pair(54, 40);

	//80Hz
	ch_map[42] = std::make_pair(58, 80);
	ch_map[58] = std::make_pair(42, 80);

	//160Hz
	ch_map[50] = std::make_pair(50, 160);
}

}
