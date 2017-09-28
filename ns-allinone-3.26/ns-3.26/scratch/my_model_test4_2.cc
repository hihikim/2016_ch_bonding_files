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

        NodeContainer wifiStaNode2;
        wifiStaNode2.Create (1);
        NodeContainer wifiApNode2;
        wifiApNode2.Create (1);

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

        // 2번째 ap-sta만들기

        ssid = Ssid ("ns3-80211ac2");

        mac.SetType ("ns3::StaWifiMac",
                   "Ssid", SsidValue (ssid));
        staDevice2 = wifi.Install (phy, mac, wifiStaNode2);


        mac.SetType ("ns3::ApWifiMac",
                   "Ssid", SsidValue (ssid));              
        apDevice2 = wifi.Install (phy, mac, wifiApNode2);

        // Set channel width
        //Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/ChannelWidth", UintegerValue (j));   //전부 다걸림
        uint32_t ch_36_width = 40;
        uint32_t ch_44_width = 80;
        Ptr<RegularWifiMac> m_mac = DynamicCast<RegularWifiMac> (DynamicCast<WifiNetDevice>(apDevice.Get(0))->GetMac());

        Ptr<MacLow> m_low = m_mac->GetLow();
        m_low->EnableChannelBonding();
        m_low->SetChannelManager(phy, 36, ch_36_width, WIFI_PHY_STANDARD_80211ac);


        m_mac = DynamicCast<RegularWifiMac> (DynamicCast<WifiNetDevice>(staDevice.Get(0))->GetMac());

        m_low = m_mac->GetLow();

        m_low->EnableChannelBonding();
        m_low->SetChannelManager(phy, 36, ch_36_width, WIFI_PHY_STANDARD_80211ac);




        m_mac = DynamicCast<RegularWifiMac> (DynamicCast<WifiNetDevice>(apDevice2.Get(0))->GetMac());

        m_low = m_mac->GetLow();
        m_low->EnableChannelBonding();
        m_low->SetChannelManager(phy, 44, ch_44_width, WIFI_PHY_STANDARD_80211ac);


        m_mac = DynamicCast<RegularWifiMac> (DynamicCast<WifiNetDevice>(staDevice2.Get(0))->GetMac());

        m_low = m_mac->GetLow();

        m_low->EnableChannelBonding();
        m_low->SetChannelManager(phy, 44, ch_44_width, WIFI_PHY_STANDARD_80211ac);




        // mobility.
        MobilityHelper mobility;
        Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

        positionAlloc->Add (Vector (0.0, 0.0, 0.0));
        positionAlloc->Add (Vector (distance, 0.0, 0.0));
        positionAlloc->Add (Vector (distance * 2, 0.0, 0.0));  //위치 추가
        positionAlloc->Add (Vector (distance * 3, 0.0, 0.0));
        mobility.SetPositionAllocator (positionAlloc);

        mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

        mobility.Install (wifiApNode);
        mobility.Install (wifiStaNode);

        //추가
        mobility.Install (wifiApNode2);
        mobility.Install (wifiStaNode2);

        /* Internet stack*/
        InternetStackHelper stack;
        stack.Install (wifiApNode);
        stack.Install (wifiStaNode);

        //추가
        stack.Install (wifiApNode2);
        stack.Install (wifiStaNode2);

        Ipv4AddressHelper address;

        address.SetBase ("192.168.1.0", "255.255.255.0");
        Ipv4InterfaceContainer staNodeInterface, staNodeInterface2;   //2로 선언했는데 배열이나 vector도 가능함
        Ipv4InterfaceContainer apNodeInterface, apNodeInterface2;

        staNodeInterface = address.Assign (staDevice);
        apNodeInterface = address.Assign (apDevice);

        staNodeInterface2 = address.Assign (staDevice2);
        apNodeInterface2 = address.Assign (apDevice2);

        /* Setting applications */
        ApplicationContainer serverApp, sinkApp;
        ApplicationContainer serverApp2, sinkApp2;
        if (udp)
        {
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


          /*
           *--------------추가-----------------
          */

          UdpServerHelper myServer2 (9);
          serverApp2 = myServer2.Install (wifiStaNode2.Get (0));
          serverApp2.Start (Seconds (0.0));
          serverApp2.Stop (Seconds (simulationTime + 1));

          UdpClientHelper myClient2 (staNodeInterface2.GetAddress (0), 9);
          myClient2.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
          myClient2.SetAttribute ("Interval", TimeValue (Time ("0.00001"))); //packets/s
          myClient2.SetAttribute ("PacketSize", UintegerValue (payloadSize));

          ApplicationContainer clientApp2 = myClient2.Install (wifiApNode2.Get (0));
          clientApp2.Start (Seconds (1.0));
          clientApp2.Stop (Seconds (simulationTime + 1));
        }
        else
        {
          //TCP flow
          uint16_t port = 50000;
          Address apLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
          PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", apLocalAddress);
          sinkApp = packetSinkHelper.Install (wifiStaNode.Get (0));

          sinkApp.Start (Seconds (0.0));
          sinkApp.Stop (Seconds (simulationTime + 1));

          OnOffHelper onoff ("ns3::TcpSocketFactory",Ipv4Address::GetAny ());
          onoff.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
          onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
          onoff.SetAttribute ("PacketSize", UintegerValue (payloadSize));
          onoff.SetAttribute ("DataRate", DataRateValue (1000000000)); //bit/s
          ApplicationContainer apps;

          AddressValue remoteAddress (InetSocketAddress (staNodeInterface.GetAddress (0), port));
          onoff.SetAttribute ("Remote", remoteAddress);
          apps.Add (onoff.Install (wifiApNode.Get (0)));
          apps.Start (Seconds (1.0));
          apps.Stop (Seconds (simulationTime + 1));

          /*
           *------------------------추가------------------------------
          */
          uint16_t port2 = 50000;
          Address apLocalAddress2 (InetSocketAddress (Ipv4Address::GetAny (), port2));
          PacketSinkHelper packetSinkHelper2 ("ns3::TcpSocketFactory", apLocalAddress2);
          sinkApp2 = packetSinkHelper2.Install (wifiStaNode2.Get (0));

          sinkApp2.Start (Seconds (0.0));
          sinkApp2.Stop (Seconds (simulationTime + 1));

          OnOffHelper onoff2 ("ns3::TcpSocketFactory",Ipv4Address::GetAny ());
          onoff2.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
          onoff2.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
          onoff2.SetAttribute ("PacketSize", UintegerValue (payloadSize));
          onoff2.SetAttribute ("DataRate", DataRateValue (1000000000)); //bit/s
          ApplicationContainer apps2;

          AddressValue remoteAddress2 (InetSocketAddress (staNodeInterface2.GetAddress (0), port2));
          onoff2.SetAttribute ("Remote", remoteAddress2);
          apps2.Add (onoff.Install (wifiApNode2.Get (0)));
          apps2.Start (Seconds (1.0));
          apps2.Stop (Seconds (simulationTime + 1));
        }

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
        double throughput2 = 0;
        if (udp)
        {
          //UDP
          uint32_t totalPacketsThrough = DynamicCast<UdpServer> (serverApp.Get (0))->GetReceived ();
          throughput = totalPacketsThrough * payloadSize * 8 / (simulationTime * 1000000.0); //Mbit/s
          uint32_t totalPacketsThrough2 = DynamicCast<UdpServer> (serverApp2.Get (0))->GetReceived ();
          throughput2 = totalPacketsThrough2 * payloadSize * 8 / (simulationTime * 1000000.0); //Mbit/s
        }
        else
        {
          //TCP
          uint32_t totalPacketsThrough = DynamicCast<PacketSink> (sinkApp.Get (0))->GetTotalRx ();
          throughput = totalPacketsThrough * 8 / (simulationTime * 1000000.0); //Mbit/s

          uint32_t totalPacketsThrough2 = DynamicCast<PacketSink> (sinkApp2.Get (0))->GetTotalRx ();
          throughput2 = totalPacketsThrough2 * 8 / (simulationTime * 1000000.0); //Mbit/s

        }
        std::cout << 0 << "\t\t\t" << ch_36_width << " MHz\t\t\t" << 0 << "\t\t\t" << throughput << " Mbit/s" << std::endl;
        std::cout << 0 << "\t\t\t" << ch_44_width << " MHz\t\t\t" << 0 << "\t\t\t" << throughput2 << " Mbit/s" << std::endl;


    
  return 0;
}