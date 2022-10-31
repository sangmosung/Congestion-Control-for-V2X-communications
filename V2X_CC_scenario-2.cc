/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006,2007 INRIA
 * Copyright (c) 2013 Dalian University of Technology
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 * Author: Junling Bu <linlinjavaer@gmail.com>
 *
 */
/**
 * This example shows basic construction of an 802.11p node.  Two nodes
 * are constructed with 802.11p devices, and by default, one node sends a single
 * packet to another node (the number of packets and interval between
 * them can be configured by command-line arguments).  The example shows
 * typical usage of the helper classes for this mode of WiFi (where "OCB" refers
 * to "Outside the Context of a BSS")."
 */

#include "ns3/vector.h"
#include "ns3/string.h"
#include "ns3/socket.h"
#include "ns3/double.h"
#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/command-line.h"
#include "ns3/mobility-model.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/position-allocator.h"
#include "ns3/mobility-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-interface-container.h"
#include "ns3/wave-bsm-helper.h"
#include "ns3/bsm-application.h"
#include <iostream>
#include <string>
#include <vector>

#include "ns3/ocb-wifi-mac.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"
#include "ns3/netanim-module.h"
#include <random>
#include "ns3/csma-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include <random>

using namespace ns3;
using std::string;
using std::to_string;


NS_LOG_COMPONENT_DEFINE ("V2X_Congestion-Control");

/*
 * In WAVE module, there is no net device class named like "Wifi80211pNetDevice",
 * instead, we need to use Wifi80211pHelper to create an object of
 * WifiNetDevice class.
 *
 * The reason of not providing a 802.11p class is that most of modeling
 * 802.11p standard has been done in wifi module, so we only need a high
 * MAC class that enables OCB mode.
 */

// PVD
void ReceivePacket_PVD (Ptr<Socket> socket)
{
  while (socket->Recv ())
    {
      NS_LOG_UNCOND ("RSU received PVD from OBU");
    }
}
// BSM
void ReceivePacket_BSM (Ptr<Socket> socket)
{

  while (socket->Recv ())
    {
      // string BSM = to_string(socket->GetNode()->GetId()) + "th OBU received broadcasting BSM from OBU";
      // NS_LOG_UNCOND (BSM);
    }
}
// Traffic generation
static void GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize, uint32_t pktCount, Time pktInterval )
{
  if (pktCount > 0)
    {
      socket->Send (Create<Packet> (pktSize));
      Simulator::Schedule (pktInterval, &GenerateTraffic,
                           socket, pktSize,pktCount - 1, pktInterval);
    }
  else
    {
      socket->Close ();
    }
}

int main (int argc, char *argv[])
{

  int obu_g1_num = 200;
  int obu_g2_num = 100;
  int obu_g3_num = 100;
  int total_obu_num = obu_g1_num + obu_g2_num + obu_g3_num; // 400
  int RSU_NODE = 1;
  int ROW_LINE = 10;

  std::string phyMode ("OfdmRate6MbpsBW10MHz");
  uint32_t PVD_pktSize = 1000; // bytes
  uint32_t WSA_pktSize = 1000;
  uint32_t numPackets = 1;
  std::string animFile = "Congestion_control.xml" ;  // Name of file for animation output
  double interval = 1.0; // seconds
  bool verbose = false;

  CommandLine cmd (__FILE__);

  cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
  cmd.AddValue ("PVD_pktSize", "size of application packet sent", PVD_pktSize);
  cmd.AddValue ("numPackets", "number of packets generated", numPackets);
  cmd.AddValue ("interval", "interval (seconds) between packets", interval);
  cmd.AddValue ("verbose", "turn on all WifiNetDevice log components", verbose);
  cmd.AddValue ("animFile",  "File Name for Animation Output", animFile);
  cmd.Parse (argc, argv);

  // Convert to time object
  Time interPacketInterval = Seconds (interval);

  // Node Container
  NodeContainer rsuNode;
  rsuNode.Create (RSU_NODE);  // 1

  NodeContainer obu_grp1;
  obu_grp1.Create (obu_g1_num); // 200

  NodeContainer obu_grp2;
  obu_grp2.Create (obu_g2_num); // 100

  NodeContainer obu_grp3;
  obu_grp3.Create (obu_g3_num); // 100

  NodeContainer total_obugrp = NodeContainer(obu_grp1, obu_grp2, obu_grp3); // 400
  NodeContainer total_grp = NodeContainer(rsuNode, obu_grp1, obu_grp2, obu_grp3); // 401


  // The below set of helpers will help us to put together the wifi NICs we want
  YansWifiPhyHelper wifiPhy;
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  Ptr<YansWifiChannel> channel = wifiChannel.Create ();
  wifiPhy.SetChannel (channel);
  // ns-3 supports generate a pcap trace
  wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11);
  NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default ();
  Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();
  if (verbose)
    {
      wifi80211p.EnableLogComponents ();      // Turn on all Wifi 802.11p logging
    }

  wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                      "DataMode",StringValue (phyMode),
                                      "ControlMode",StringValue (phyMode));

  // NetDevice Installation

  NetDeviceContainer NDrsu = wifi80211p.Install (wifiPhy, wifi80211pMac, rsuNode);
  NetDeviceContainer NDgrp1 = wifi80211p.Install (wifiPhy, wifi80211pMac, obu_grp1);
  NetDeviceContainer NDgrp2 = wifi80211p.Install (wifiPhy, wifi80211pMac, obu_grp2);
  NetDeviceContainer NDgrp3 = wifi80211p.Install (wifiPhy, wifi80211pMac, obu_grp3);
  NetDeviceContainer NDgrp_total = wifi80211p.Install (wifiPhy, wifi80211pMac, total_grp);

  // CSMA node setting
  // NodeContainer c1 = NodeContainer (c,0,OBU_grp1 + RSU_NODE);

  NS_LOG_INFO ("Build Topology.");
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate (5000000)));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (5)));
  NetDeviceContainer csma_total = csma.Install (total_grp);

  NetDeviceContainer devices_mix = NetDeviceContainer (NDgrp_total, csma_total);

  // Tracing
  wifiPhy.EnablePcap ("wave-simple-80211p", false);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (total_obu_num/(2*ROW_LINE), -2, 0.0));

  for(int i =0; i<ROW_LINE; i++)
  {
    for(int j=1;j<(total_obu_num/ROW_LINE)+0.5;j++)
      positionAlloc->Add (Vector (1*j, 1*(i+1), 0.0));
  }
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (total_grp);

  NS_LOG_INFO ("Enabling OLSR Routing");
  
  InternetStackHelper internet;
  internet.Install (total_grp);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");

  ipv4.SetBase ("10.0.0.0", "255.0.0.0"); // RSU IP address 10.0.0.1 (PVD Unicast Dest)
  ipv4.Assign (NDrsu);

  ipv4.SetBase ("10.0.1.0", "255.0.1.0");
  ipv4.Assign (NDgrp1);

  ipv4.SetBase ("10.0.2.0", "255.0.2.0");
  ipv4.Assign (NDgrp2);

  ipv4.SetBase ("10.0.3.0", "255.0.3.0");
  ipv4.Assign (NDgrp3);

  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  /// RSU가 OBU에게 broadcast message를 보낼 때!!
  // 1. 다중 node의 동시 broadcast 구현
  // 2. broadcast Interval을 부여
  //   - ns3 내에 timer = 100ms 


  // PVD Unicast
  // All OBU nodes send the PVD message to RSU during 1sec
  int m = 0;                           
  while(m<5)
  {
    for(int i=1; i<total_obu_num + RSU_NODE; i++)
      {
        InetSocketAddress remote = InetSocketAddress (Ipv4Address ("10.0.0.1"), i);
        Ptr<Socket> recvSink = Socket::CreateSocket (total_grp.Get (0), tid);
        InetSocketAddress local = InetSocketAddress (Ipv4Address("255.255.255.255"), i);
        recvSink->Bind (local);
        recvSink->SetRecvCallback (MakeCallback(&ReceivePacket_PVD));
        Ptr<Socket> source = Socket::CreateSocket (total_grp.Get (i), tid);
        source->SetAllowBroadcast (true);
        source->Connect (remote);
        // NS_LOG_UNCOND (source->GetNode ()->GetId ());   
        Simulator::ScheduleWithContext (source->GetNode ()->GetId (),
                                Seconds (m+0.0025*i), &GenerateTraffic, //Seconds (m+0.02*i)
                                source, PVD_pktSize, numPackets, interPacketInterval);
      }
    m++;
  }

    // WSA Broadcast
    // Every 1sec, One RSU sends the WSA message to all OBUs  
    int n = 0;
    while(n<5)
    {
      for(int j =1; j<total_obu_num + RSU_NODE; j++)
      {
        Ptr<Socket> recvSink = Socket::CreateSocket (total_grp.Get (j), tid);
        InetSocketAddress local = InetSocketAddress (Ipv4Address("255.255.255.255"), 80);
        recvSink->Bind (local);

        // recvSink->SetRecvCallback (MakeCallback (&BsmApplication::ReceiveWavePacket, recvSink));
        recvSink->SetRecvCallback (MakeCallback (&ReceivePacket_BSM));                                     
      
      }
      InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), 80);
      Ptr<Socket> source = Socket::CreateSocket (total_grp.Get (0), tid);
      source->SetAllowBroadcast (true);
      source->Connect (remote);
      Simulator::ScheduleWithContext (source->GetNode ()->GetId (),
                                      Seconds (n), &GenerateTraffic,
                                      source, WSA_pktSize, numPackets, interPacketInterval);
      string BSM = to_string(source->GetNode()->GetId()) + "RSU send WSA message as broadcast";
      NS_LOG_UNCOND (BSM);
      n++;    
    }

  // BSM Broadcast
  // All OBU nodes send the BSM message to RSU
  uint16_t port = 9;
 
  NS_LOG_INFO ("BSM Transfer Scenario");

  // Scenario #1: 200 BSM transmit
  for(int i=0; i <obu_g1_num; i++)
  {
    OnOffHelper onoff ("ns3::UdpSocketFactory", 
                      Address (InetSocketAddress (Ipv4Address ("255.255.255.255"), port)));

    onoff.SetConstantRate (DataRate ("16Kb/s"),200);
    
    ApplicationContainer app = onoff.Install (obu_grp1.Get (i));

    app.Start (Seconds (1.0));
    app.Stop (Seconds (2.0));
  }

  // Scenario #2: 400 BSM transmit
  for(int i=0; i <total_obu_num; i++)
  {
    OnOffHelper onoff ("ns3::UdpSocketFactory", 
                      Address (InetSocketAddress (Ipv4Address ("255.255.255.255"), port)));
    
    onoff.SetConstantRate (DataRate ("16Kb/s"),200);
    
    ApplicationContainer app = onoff.Install (total_obugrp.Get (i));

    app.Start (Seconds (2.00001));
    app.Stop (Seconds (3.0));
  }


  // Scenario #3: 200 BSM transmit
  for(int i=0; i <obu_g1_num; i++)
  {
    OnOffHelper onoff ("ns3::UdpSocketFactory", 
                      Address (InetSocketAddress (Ipv4Address ("255.255.255.255"), port)));
    
    onoff.SetConstantRate (DataRate ("16Kb/s"),200);
    
    ApplicationContainer app = onoff.Install (obu_grp1.Get (i));

    app.Start (Seconds (3.00001));
    app.Stop (Seconds (4.0));
  }

  // Also configure some tcpdump traces; each interface will be traced
  // The output files will be named
  // csma-broadcast-<nodeId>-<interfaceId>.pcap
  // and can be read by the "tcpdump -tt -r" command 
  
  NS_LOG_INFO ("Configure Animation.");

  AnimationInterface anim (animFile);

  uint32_t rsu_icon = anim.AddResource("/home/smsung/Pictures/Base.png");
  uint32_t greencar_icon = anim.AddResource("/home/smsung/Pictures/greencar.png");
  uint32_t redcar_icon = anim.AddResource("/home/smsung/Pictures/redcar.png");
  uint32_t bluecar_icon = anim.AddResource("/home/smsung/Pictures/bluecar.png");

  Ptr<Node> rsu = total_grp.Get(0);
  anim.UpdateNodeImage(rsu->GetId(),rsu_icon);
  anim.UpdateNodeSize(0,3,3);
  
  for (int i=1; i<201; i++)
  {
    Ptr<Node> greencar = total_grp.Get(i);
    anim.UpdateNodeImage(greencar->GetId(),greencar_icon);
  }
  for (int i=201; i<301; i++)
  {
    Ptr<Node> redcar = total_grp.Get(i);
    anim.UpdateNodeImage(redcar->GetId(),redcar_icon);
  }
  for (int i=301; i<401; i++)
  {
    Ptr<Node> bluecar = total_grp.Get(i);
    anim.UpdateNodeImage(bluecar->GetId(),bluecar_icon);
  }
  
  AsciiTraceHelper ascii;
  csma.EnableAsciiAll (ascii.CreateFileStream ("CSMA_estimation.tr"));

  NS_LOG_INFO ("Run Simulation.");

  anim.SetMaxPktsPerTraceFile(500000);
  Simulator::Run ();
  std::cout << "Animation Trace file created:" << animFile.c_str ()<< std::endl;
  Simulator::Destroy ();

  return 0;
}