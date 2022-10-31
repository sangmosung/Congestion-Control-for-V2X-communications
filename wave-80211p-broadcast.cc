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

typedef struct {
  float time = 0;
  float prev_time = 0;
  int num = 0;
  int rep_num = 0;
}RSU;

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
unsigned char recv_buffer[100];
unsigned char recv_buffer2[100];
unsigned char send_buffer[100];
Ptr<Packet> packet_0;
Ptr<Packet> packet_1;
float f= 0;
float a;
int number = 0;

// PVD
void ReceivePacket_PVD (Ptr<Socket> socket)
{
  
  while (socket->Recv (recv_buffer2,12,0))
    {
      // NS_LOG_UNCOND ("RSU received PVD from OBU");
      // printf("출력: %s \n", recv_buffer2);
      string st = "";
      for(int i = 0 ; i<12 ; i++)
        st += recv_buffer2[i];
       
      // std::cout << "PVD로부터 전달받은내용: " << st << std::endl;
    }
}

void ReceivePacket_From_WSA (Ptr<Socket> socket)
{
  // printf("출력: %s \n", recv_buffer);
  
  while (socket->Recv (recv_buffer,10,0))
    {
      // NS_LOG_UNCOND ("RSU received PVD from OBU");
      // printf("출력: %s \n", recv_buffer);
      string st = "";
      for(int i = 0 ; i<10 ; i++)
        st += recv_buffer[i];
      f = std::stof(st); 
      // std::cout << "WSA로부터 전달받은 주기" << f << std::endl;
    }
}
RSU rsu;
// BSM
void ReceivePacket_BSM (Ptr<Socket> socket)
{
  while (socket->Recv ())
    {
      // string BSM = "RSU received broadcasting BSM from OBU";
      // NS_LOG_UNCOND (BSM);
      // std::cout << Simulator::Now ().GetSeconds () << std::endl;
      if(rsu.num==0)
      {
          rsu.prev_time = Simulator::Now ().GetSeconds ();
          printf("hello\n");
      }
      else if (rsu.num == OBU_NODE-1)
      {
          rsu.time = Simulator::Now ().GetSeconds ();
          // printf("RSU에서 측정한 전송주기는?: ");
          std::cout << Simulator::Now ().GetSeconds () << ": RSU에서 측정한 전송주기는?: "<< rsu.time - rsu.prev_time << std::endl;
          a = rsu.time - rsu.prev_time;
          uint8_t* packet_buffer = (uint8_t*)(&a);
          packet_0 = Create<Packet> (packet_buffer,200);
      }
      rsu.num++;
      if(Simulator::Now ().GetSeconds () >1 && rsu.rep_num ==0)
      {
        rsu.num=1;
        rsu.prev_time = Simulator::Now ().GetSeconds ();
        rsu.rep_num += 1;
      }
      for(int i =0; i <10 ; i++)
      {
        if(Simulator::Now ().GetSeconds () >1+i && rsu.rep_num == i)
        { 
          rsu.num=1;
          rsu.prev_time = Simulator::Now ().GetSeconds ();  
          rsu.rep_num += 1;
        }
      }
      // printf("%d\n", rsu.num);
    }
  }
static void GenerateTraffic_PVD (Ptr<Socket> socket, Ptr<Packet> packet,
                             uint32_t pktCount, Time pktInterval )
{
  if (pktCount > 0)
    {
      socket->Send (packet);
      Simulator::Schedule (pktInterval, &GenerateTraffic_PVD,
                           socket, packet,pktCount - 1, pktInterval);
    }
  else
    {
      socket->Close ();
    }
}
static void GenerateTraffic (Ptr<Socket> socket, Ptr<Packet> packet,
                             uint32_t pktCount, Time pktInterval )
{
  
  if (pktCount > 0)
    {
      
      // std::cout << a << std::endl;
      string a_string(std::to_string(a));
      // std::cout << a_string << std::endl;
      uint8_t packet_buffer[10];
      
      std::copy(a_string.begin(), a_string.end(), std::begin(packet_buffer));

      // for(int i=0 ; i< 10;i++)
      // {
      //   printf("%c", packet_buffer[i]);
      // }
      packet = Create<Packet> (packet_buffer,10);
      
      // socket->Send (Create<Packet> (pktSize));
      socket->Send(packet);
      string WSA = "RSU send WSA message as broadcast";
      NS_LOG_UNCOND (WSA);
      std::cout << Simulator::Now ().GetSeconds () << "초에 전송주기" << a_string << "을 담은 패킷이 전송되었습니다." << std::endl;
      printf("\n");
      Simulator::Schedule (pktInterval, &GenerateTraffic,
                           socket, packet,pktCount - 1, pktInterval);
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
  std::string animFile = "wave-80211p.xml" ;  // Name of file for animation output
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

  for(int j = 0 ; j<10; j++)
  {
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
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (50)));
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



  // ns3::PacketMetadata::Enable (); // 이거 넣어줘야 실제 패킷 보내고 받을 때 오류 안생김
  // WSA
    for(int k =1; k<OBU_NODE+RSU_NODE; k++)
      {
        Ptr<Socket> recvSink = Socket::CreateSocket (c.Get (k), tid);
        InetSocketAddress local = InetSocketAddress (Ipv4Address("255.255.255.255"), 80);
        recvSink->Bind (local);
        recvSink->SetRecvCallback (MakeCallback (&ReceivePacket_From_WSA));                                     
      }
      InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), 80);
      Ptr<Socket> source = Socket::CreateSocket (c.Get (0), tid);
      source->SetAllowBroadcast (true);
      source->Connect (remote);
        
      Simulator::ScheduleWithContext (source->GetNode ()->GetId (),
                                      Seconds (j+1), &GenerateTraffic,
                                      source, packet_0, numPackets, interPacketInterval);
  
  uint16_t port = 9;
  NS_LOG_INFO ("Create Applications.");
  for(int i = 1; i <OBU_NODE + RSU_NODE ;i++)
  {
    OnOffHelper onoff ("ns3::UdpSocketFactory", 
                      Address (InetSocketAddress (Ipv4Address ("255.255.255.255"), port)));
    // 10Kb/s , 125 => 0.1s
    // 1Kb = 1000bit = 125 byte
    // 16Kb = 16000bit = 2000 byte
    if(f!=0 && f<0.1)
      onoff.SetConstantRate (DataRate ("20Kb/s"),200);
    else
      onoff.SetConstantRate (DataRate ("16Kb/s"),200);

    ApplicationContainer app = onoff.Install (c1.Get (i));
    // RSU가 측정하는 부분
    Ptr<Socket> recvSink = Socket::CreateSocket (c1.Get (0), tid);
    InetSocketAddress local = InetSocketAddress (Ipv4Address("255.255.255.255"), port);
    recvSink->Bind (local);
    recvSink->SetRecvCallback (MakeCallback(&ReceivePacket_BSM));
    app.Start (Seconds (0.0001+j));
    app.Stop (Seconds (1.0001+j));
  }
// PVD Unicast code  
  ns3::PacketMetadata::Enable (); // 이거 넣어줘야 실제 패킷 보내고 받을 때 오류 안생김
  
  for(int i=1; i<OBU_NODE+RSU_NODE; i++)
    {
      string PVD_message = "buffer_size_12";
      uint8_t PVD_buffer[15];
      
      std::copy(PVD_message.begin(), PVD_message.end(), std::begin(PVD_buffer));

      packet_1 = Create<Packet> (PVD_buffer,8);
      InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), i+100);
      Ptr<Socket> recvSink = Socket::CreateSocket (c.Get (0), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address("255.255.255.255"), i+100);
      recvSink->Bind (local);
      recvSink->SetRecvCallback (MakeCallback(&ReceivePacket_PVD));
      Ptr<Socket> source = Socket::CreateSocket (c.Get (i), tid);
      source->SetAllowBroadcast (true);
      source->Connect (remote);
      // NS_LOG_UNCOND (source->GetNode ()->GetId ());   
      Simulator::ScheduleWithContext (source->GetNode ()->GetId (),
                              Seconds (j+1+i/(OBU_NODE)), &GenerateTraffic_PVD,
                              source, packet_1, numPackets, interPacketInterval);
    }


  AsciiTraceHelper ascii;
  csma.EnableAsciiAll (ascii.CreateFileStream ("WSA_example.tr"));


  // Also configure some tcpdump traces; each interface will be traced
  // The output files will be named
  // csma-broadcast-<nodeId>-<interfaceId>.pcap
  // and can be read by the "tcpdump -tt -r" command 

  NS_LOG_INFO ("Run Simulation.");

  AnimationInterface anim (animFile);
  anim.SetMaxPktsPerTraceFile(500000);
  Simulator::Run ();
  // std::cout << "Animation Trace file created:" << animFile.c_str ()<< std::endl;
  Simulator::Destroy ();
  }
  std::cout << "What is the f: " << f << std::endl;
  

  return 0;
}
