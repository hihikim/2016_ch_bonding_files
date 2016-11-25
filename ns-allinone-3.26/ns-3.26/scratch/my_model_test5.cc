/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 SEBASTIEN DERONNE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Sebastien Deronne <sebastien.deronne@gmail.com>
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/olsr-helper.h"

// This is a simple example in order to show how to configure an IEEE 802.11ac Wi-Fi network.
//
// It ouputs the UDP or TCP goodput for every VHT bitrate value, which depends on the MCS value (0 to 9, where 9 is
// forbidden when the channel width is 20 MHz), the channel width (20, 40, 80 or 160 MHz) and the guard interval (long
// or short). The PHY bitrate is constant over all the simulation run. The user can also specify the distance between
// the access point and the station: the larger the distance the smaller the goodput.
//
// The simulation assumes a single station in an infrastructure network:
//
//  STA     AP
//    *     *
//    |     |
//   n1     n2
//
//Packets in this simulation aren't marked with a QosTag so they are considered
//belonging to BestEffort Access Class (AC_BE).

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("vht-wifi-network");

int main (int argc, char *argv[])
{
        bool udp = true;
        double simulationTime = 10; //seconds
        double distance = 1.0; //meters

        CommandLine cmd;
        cmd.AddValue ("distance", "Distance in meters between the station and the access point", distance);
        cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
        cmd.AddValue ("udp", "UDP if set to 1, TCP otherwise", udp);
        cmd.Parse (argc,argv);

        std::cout << "MCS value" << "\t\t" << "Channel width" << "\t\t" << "short GI" << "\t\t" << "Throughput" << '\n';


          
        uint32_t payloadSize; //1500 byte IP packet
        if (udp)
        {
          payloadSize = 1472; //bytes
        }
        else
        {
          payloadSize = 1448; //bytes
          Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (payloadSize));
        }

        NodeContainer wifiStaNode;
        wifiStaNode.Create (1);
        NodeContainer wifiApNode;
        wifiApNode.Create (1);

        YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
        YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
        phy.SetChannel (channel.Create ());

        // Set guard interv58374 Mbit/s

        phy.Set ("ShortGuardEnabled", BooleanValue (0));

        WifiHelper wifi;
        wifi.SetStandard (WIFI_PHY_STANDARD_80211ac);
        WifiMacHelper mac;

        std::ostringstream oss;
        oss << "VhtMcs" << 0;
        wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", StringValue (oss.str ()),
                                    "ControlMode", StringValue (oss.str ()),
                                    "RtsCtsThreshold", UintegerValue(100));

        Ssid ssid = Ssid ("ns3-80211ac");

        mac.SetType ("ns3::StaWifiMac",
                   "Ssid", SsidValue (ssid));

        NetDeviceContainer staDevice, staDevice2;
        staDevice = wifi.Install (phy, mac, wifiStaNode);

        mac.SetType ("ns3::ApWifiMac",
                   "Ssid", SsidValue (ssid));

        NetDeviceContainer apDevice, apDevice2;
        apDevice = wifi.Install (phy, mac, wifiApNode);

        // Set channel width
        //Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/ChannelWidth", UintegerValue (j));   //전부 다걸림
        Ptr<RegularWifiMac> m_mac = DynamicCast<RegularWifiMac> (DynamicCast<WifiNetDevice>(apDevice.Get(0))->GetMac());

        Ptr<MacLow> m_low = m_mac->GetLow();
        m_low->EnableChannelBonding();
        m_low->SetChannelManager(phy, 36, 40, WIFI_PHY_STANDARD_80211ac);


        m_mac = DynamicCast<RegularWifiMac> (DynamicCast<WifiNetDevice>(staDevice.Get(0))->GetMac());

        m_low = m_mac->GetLow();

        m_low->EnableChannelBonding();
        m_low->SetChannelManager(phy, 36, 40, WIFI_PHY_STANDARD_80211ac);

        // mobility.
        MobilityHelper mobility;
        Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

        positionAlloc->Add (Vector (0.0, 0.0, 0.0));
        positionAlloc->Add (Vector (distance, 0.0, 0.0));
        mobility.SetPositionAllocator (positionAlloc);

        mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

        mobility.Install (wifiApNode);
        mobility.Install (wifiStaNode);

        //추가

        /* Internet stack*/
        InternetStackHelper stack;
        stack.Install (wifiApNode);
        stack.Install (wifiStaNode);


        Ipv4AddressHelper address;

        address.SetBase ("192.168.1.0", "255.255.255.0");
        Ipv4InterfaceContainer staNodeInterface;   //2로 선언했는데 배열이나 vector도 가능함
        Ipv4InterfaceContainer apNodeInterface;

        staNodeInterface = address.Assign (staDevice);
        apNodeInterface = address.Assign (apDevice);


        /* Setting applications */
        ApplicationContainer serverApp, sinkApp;
        
          //UDP flow
          UdpServerHelper myServer (9);
          serverApp = myServer.Install (wifiStaNode.Get (0));
          serverApp.Start (Seconds (0.0));
          serverApp.Stop (Seconds (simulationTime + 1));

          UdpClientHelper myClient (staNodeInterface.GetAddress (0), 9);
          myClient.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
          myClient.SetAttribute ("Interval", TimeValue (Time ("0.00001"))); //packets/s
          myClient.SetAttribute ("PacketSize", UintegerValue (payloadSize));

          ApplicationContainer clientApp = myClient.Install (wifiApNode.Get (0));
          clientApp.Start (Seconds (1.0));
          clientApp.Stop (Seconds (simulationTime + 1));



        /*OlsrHelper olsr;

        Ipv4StaticRoutingHelper staticRouting;

        Ipv4ListRoutingHelper list;
        list.Add(staticRouting, 0);
        list.Add(olsr,10);
        stack.SetRoutingHelper(list);*/
        Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

        Simulator::Stop (Seconds (simulationTime + 1));
        Simulator::Run ();
        Simulator::Destroy ();

        double throughput = 0;
    
          uint32_t totalPacketsThrough = DynamicCast<UdpServer> (serverApp.Get (0))->GetReceived ();
          throughput = totalPacketsThrough * payloadSize * 8 / (simulationTime * 1000000.0); //Mbit/s
       
       
        std::cout << 0 << "\t\t\t" << 40 << " MHz\t\t\t" << 0 << "\t\t\t" << throughput << " Mbit/s" << std::endl;
       
    
  return 0;
}
