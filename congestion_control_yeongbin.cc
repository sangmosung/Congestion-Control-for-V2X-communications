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

#define OBU_NODE 200
#define RSU_NODE 1
#define ROW_LINE 10

NS_LOG_COMPONENT_DEFINE ("WifiSimpleOcb");

/*
 * In WAVE module, there is no net device class named like "Wifi80211pNetDevice",
 * instead, we need to use Wifi80211pHelper to create an object of
 * WifiNetDevice class.
 *
 * usage:
 *  NodeContainer nodes;
 *  NetDeviceContainer devices;
 *  nodes.Create (2);
 *  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
 *  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
 *  wifiPhy.SetChannel (wifiChannel.Create ());
 *  NqosWaveMacHelper wifi80211pMac = NqosWave80211pMacHelper::Default();
 *  Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();
 *  devices = wifi80211p.Install (wifiPhy, wifi80211pMac, nodes);
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

static void GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize,
                             uint32_t pktCount, Time pktInterval )
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
  std::string phyMode ("OfdmRate6MbpsBW10MHz");
  uint32_t packetSize = 1000; // bytes
  uint32_t numPackets = 1;
  std::string animFile = "Congestion_control.xml" ;  // Name of file for animation output
  double interval = 1.0; // seconds
  bool verbose = false;

  CommandLine cmd (__FILE__);

  cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
  cmd.AddValue ("packetSize", "size of application packet sent", packetSize);
  cmd.AddValue ("numPackets", "number of packets generated", numPackets);
  cmd.AddValue ("interval", "interval (seconds) between packets", interval);
  cmd.AddValue ("verbose", "turn on all WifiNetDevice log components", verbose);
  cmd.AddValue ("animFile",  "File Name for Animation Output", animFile);
  cmd.Parse (argc, argv);
  // Convert to time object
  Time interPacketInterval = Seconds (interval);


  NodeContainer c;
  c.Create (OBU_NODE + RSU_NODE);

  // The below set of helpers will help us to put together the wifi NICs we want
  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
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
  NetDeviceContainer devices = wifi80211p.Install (wifiPhy, wifi80211pMac, c);

  // CSMA node setting
  NodeContainer c1 = NodeContainer (c,0,OBU_NODE + RSU_NODE);
  NS_LOG_INFO ("Build Topology.");
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate (5000000)));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (5)));
  NetDeviceContainer n1 = csma.Install (c1);

  NetDeviceContainer devices_mix = NetDeviceContainer (devices, n1);



  // Tracing
  wifiPhy.EnablePcap ("wave-simple-80211p", false);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (OBU_NODE/(2*ROW_LINE), 0.0, 0.0));

  for(int i =0; i<ROW_LINE; i++)
  {
    for(int j=0;j<OBU_NODE/ROW_LINE;j++)
      positionAlloc->Add (Vector (1*j, 1*(i+1), 0.0));
  }
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (c);

  NS_LOG_INFO ("Enabling OLSR Routing");
  
  InternetStackHelper internet;
  internet.Install (c);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.0.0.0", "255.0.0.0");
  ipv4.Assign (devices_mix);

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
    for(int i=1; i<OBU_NODE+RSU_NODE; i++)
      {
        InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), i);
        Ptr<Socket> recvSink = Socket::CreateSocket (c.Get (0), tid);
        InetSocketAddress local = InetSocketAddress (Ipv4Address("255.255.255.255"), i);
        recvSink->Bind (local);
        recvSink->SetRecvCallback (MakeCallback(&ReceivePacket_PVD));
        Ptr<Socket> source = Socket::CreateSocket (c.Get (i), tid);
        source->SetAllowBroadcast (true);
        source->Connect (remote);
        // NS_LOG_UNCOND (source->GetNode ()->GetId ());   
        Simulator::ScheduleWithContext (source->GetNode ()->GetId (),
                                Seconds (m+0.02*i), &GenerateTraffic,
                                source, packetSize, numPackets, interPacketInterval);
      }
      m++;
    }

  // WSA Broadcast
  // Every 1sec, One RSU sends the WSA message to all OBUs  
  int n = 0;
  while(n<5)
  {
    for(int j =1; j<OBU_NODE+RSU_NODE; j++)
      {
        Ptr<Socket> recvSink = Socket::CreateSocket (c.Get (j), tid);
        InetSocketAddress local = InetSocketAddress (Ipv4Address("255.255.255.255"), 80);
        recvSink->Bind (local);

        // recvSink->SetRecvCallback (MakeCallback (&BsmApplication::ReceiveWavePacket, recvSink));
        recvSink->SetRecvCallback (MakeCallback (&ReceivePacket_BSM));                                     
      
      }
    InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), 80);
    Ptr<Socket> source = Socket::CreateSocket (c.Get (0), tid);
    source->SetAllowBroadcast (true);
    source->Connect (remote);
    Simulator::ScheduleWithContext (source->GetNode ()->GetId (),
                                    Seconds (n), &GenerateTraffic,
                                    source, packetSize, numPackets, interPacketInterval);
    string BSM = to_string(source->GetNode()->GetId()) + "RSU send WSA message as broadcast";
    NS_LOG_UNCOND (BSM);
    n++;    
  }

  // BSM Broadcast
  // All OBU nodes send the BSM message to RSU
  uint16_t port = 9;
  NS_LOG_INFO ("Create Applications.");
  for(int i = 1; i <OBU_NODE + RSU_NODE ;i++)
  {
    OnOffHelper onoff ("ns3::UdpSocketFactory", 
                      Address (InetSocketAddress (Ipv4Address ("255.255.255.255"), port)));
    // 10Kb/s , 125 => 0.1s
    // 1Kb = 1000bit = 125 byte
    // 16Kb = 16000bit = 2000 byte
    
    onoff.SetConstantRate (DataRate ("16Kb/s"),200);
    
    ApplicationContainer app = onoff.Install (c1.Get (i));

    app.Start (Seconds (0.0));
    app.Stop (Seconds (5.0));
  }

  AsciiTraceHelper ascii;
  csma.EnableAsciiAll (ascii.CreateFileStream ("CSMA_estimation.tr"));

  // Also configure some tcpdump traces; each interface will be traced
  // The output files will be named
  // csma-broadcast-<nodeId>-<interfaceId>.pcap
  // and can be read by the "tcpdump -tt -r" command 

  NS_LOG_INFO ("Run Simulation.");

  AnimationInterface anim (animFile);
  anim.SetMaxPktsPerTraceFile(500000);
  Simulator::Run ();
  std::cout << "Animation Trace file created:" << animFile.c_str ()<< std::endl;
  Simulator::Destroy ();

  return 0;
}
