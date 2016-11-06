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
	received_channel[36] = false; received_channel[40] = false; received_channel[44] = false; received_channel[48] = false;
	received_channel[52] = false; received_channel[56] = false; received_channel[60] = false; received_channel[64] = false;
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

void ChannelManager::SetChannelOption(uint32_t primary_ch,uint32_t max_width){
	this->max_width = max_width;
	this->primary_ch = primary_ch;
}

void ChannelManager::MakePhys(YansWifiPhyHelper phy){

}

void ChannelManager::ClearReceiveRecord(){
	received_channel[36] = false; received_channel[40] = false; received_channel[44] = false; received_channel[48] = false;
	received_channel[52] = false; received_channel[56] = false; received_channel[60] = false; received_channel[64] = false;
}


bool ChannelManager::CheckChBonding(uint32_t primary){
	std::pair<uint32_t,uint32_t> ch_info;

	ch_info = ch_map[primary];


	if(CheckAllSubChannelIdle(ch_info.first))   //if second channel is idle return true
		return true;

	else                                //if second channel is not idle return false
		return false;

}

bool ChannelManager::CheckAllSubChannelIdle(uint32_t ch_num){
	std::pair<uint32_t,uint32_t> ch_info;
	ch_info = ch_map[ch_num];

	uint32_t ch_length = ch_info.second;
	uint32_t side_num = ch_length / 20;

	if(ch_length == 20){
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



}
