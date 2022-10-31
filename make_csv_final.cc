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
 * You should have received time_diff copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 * Author: Junling Bu <linlinjavaer@gmail.com>
 *
 */
/**
 * This example shows basic construction of an 802.11p node.  Two nodes
 * are constructed with 802.11p devices, and by default, one node sends time_diff single
 * packet to another node (the number of packets and interval between
 * them can be configured by command-line arguments).  The example shows
 * typical usage of the helper classes for this mode of WiFi (where "OCB" refers
 * to "Outside the Context of time_diff BSS")."
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
  int cycle_num = 0;
  int start_time_num = 0;
}RSU;

typedef struct {
  float time = 0;
}WSA;

#define OBU_NODE 100
#define RSU_NODE 1
#define ROW_LINE 10
#define TOTAL_TIME 100
#define BSM_PACKET_SIZE 200
#define BYTE_SIZE 8
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
 * The reason of not providing time_diff 802.11p class is that most of modeling
 * 802.11p standard has been done in wifi module, so we only need time_diff high
 * MAC class that enables OCB mode.
 */
unsigned char recv_wsa_packet[100];
unsigned char recv_pvd_packet[100];
Ptr<Packet> send_wsa_packet;
Ptr<Packet> send_pvd_packet;
std::string recv_itt_data;
std::string send_itt_data;
float time_diff;
int number = 0;
int j_num =0;
float ITT;

// PVD
void ReceivePacket_PVD (Ptr<Socket> socket)
{
  
  while (socket->Recv (recv_pvd_packet,12,0))
    {
      string st = "";
      for(int i = 0 ; i<12 ; i++)
        st += recv_pvd_packet[i];
    }
}

WSA wsa;
void ReceivePacket_From_WSA (Ptr<Socket> socket)
{
  // printf("출력: %s \n", recv_wsa_packet);
  
  while (socket->Recv (recv_wsa_packet,7,0))
    {
      recv_itt_data = "";
      for(int i = 0 ; i<7 ; i++)
        recv_itt_data += recv_wsa_packet[i];
    }
    wsa.time = Simulator::Now ().GetSeconds (); 
}
RSU rsu;
// BSM
void ReceivePacket_BSM (Ptr<Socket> socket)
{
  while (socket->Recv ())
    {

      if(rsu.cycle_num==0)
      {
          rsu.prev_time = Simulator::Now ().GetSeconds ();
          printf("Simulation Start\n");
      }
      else if (rsu.cycle_num == OBU_NODE-1)
      {
          rsu.time = Simulator::Now ().GetSeconds ();
          time_diff = rsu.time - rsu.prev_time;
          std::cout << Simulator::Now ().GetSeconds () << "s>> Time taken from BSM transmission to arrival: "<< time_diff <<  "[s]" << std::endl;

          // std::cout << recv_itt_data << std::endl;
          char itt_char[2];
          itt_char[0] = recv_itt_data[0];
          itt_char[1] = recv_itt_data[1];
          int prev_itt = (itt_char[0]-'0')*10+(itt_char[1]-'0');
          // std::cout << itt_char << std::endl;
          // std::cout << "hi:"<<prev_itt << std::endl;
          float cbr;
          if(j_num ==0)
            cbr = time_diff/0.1*100;
          else
            cbr = (time_diff)*(prev_itt*1000)/(BSM_PACKET_SIZE*BYTE_SIZE)*100;
          
          std::cout << Simulator::Now ().GetSeconds () << "s>> Channel Busy Ratio: "<< cbr  << "[%]"<< std::endl;

          // extract simulation time, cbr as csv file
          float m_simulationTime = rsu.time;
          float m_cbr = cbr;
          std::ofstream out;
          out.open("V2X variables.csv", std::ios::app);
          out << m_simulationTime << ","
              << m_cbr << ",";
          out.close();

          // printf("CBR: %.2f %\n",cbr);
          if(cbr!= 0 && cbr >100 && cbr<110) // 0.107
            {
              send_itt_data = "15Kb/s";
              ITT = 0.107;
            }
          else if(cbr!=0 && cbr>110 && cbr<120) // 0.114
            {
              send_itt_data = "14Kb/s";
              ITT = 0.114;
            }
          else if(cbr!=0 && cbr>120 && cbr<130) // 0.123
            {
              send_itt_data = "13Kb/s";
              ITT = 0.123;
            }
          else if(cbr!=0 && cbr>130 && cbr<140) // 0.133
            {
              send_itt_data = "12Kb/s";
              ITT = 0.133;
            }
          else if(cbr!=0 && cbr>140 && cbr<150) // 0.145
            {
              send_itt_data = "11Kb/s";
              ITT = 0.145;
            }
          else if(cbr!=0 && cbr>150) // 0.16
            {
              send_itt_data = "10Kb/s";
              ITT = 0.160;
            }
          else if(cbr!=0 && cbr>90 && cbr<100) // 0.1
            {
              send_itt_data = "16Kb/s";
              ITT = 0.100;
            }
          else if(cbr!=0 && cbr>80 && cbr<90) // 0.094
            {
              send_itt_data = "17Kb/s";
              ITT = 0.094;
            }
          else if(cbr!=0 && cbr>70 && cbr<80) // 0.089
            {
              send_itt_data = "18Kb/s";
              ITT = 0.089;
            }
          else if(cbr!=0 && cbr>60 && cbr<70) // 0.084
            {
              send_itt_data = "19Kb/s";
              ITT = 0.084;
            }
          else if(cbr!=0 && cbr<60) // 0.08
            {
              send_itt_data = "20Kb/s";
              ITT = 0.080;
            }

          uint8_t* packet_buffer = (uint8_t*)(&send_itt_data);
          send_wsa_packet = Create<Packet> (packet_buffer,200);
      }
      rsu.cycle_num++;

      for(int i =0; i <TOTAL_TIME ; i++)
      {
        if(j_num ==i && rsu.start_time_num == i)
        { 
          rsu.cycle_num=1;
          rsu.prev_time = Simulator::Now ().GetSeconds ();  
          rsu.start_time_num += 1;
        }
      }
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
      
      // std::cout << time_diff << std::endl;
      // string a_string(std::to_string(time_diff));
      // std::cout << a_string << std::endl;
      uint8_t packet_buffer[7];
      
      std::copy(send_itt_data.begin(), send_itt_data.end(), std::begin(packet_buffer));

      packet = Create<Packet> (packet_buffer,7);
      
      // socket->Send (Create<Packet> (pktSize));
      socket->Send(packet);
      string WSA = "RSU send WSA message as broadcast";
      NS_LOG_UNCOND (WSA);
      std::cout << Simulator::Now ().GetSeconds () << "s>> ITT(" << ITT << ")를 담은 WSA 메시지가 전송되었습니다." << std::endl;
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

  for(int j = 0 ; j<TOTAL_TIME; j++)
  {
  j_num = j;
  NodeContainer c;
  c.Create (OBU_NODE + RSU_NODE);

  // The below set of helpers will help us to put together the wifi NICs we want
  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  Ptr<YansWifiChannel> channel = wifiChannel.Create ();
  wifiPhy.SetChannel (channel);
  // ns-3 supports generate time_diff pcap trace
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
  // NodeContainer c1 = NodeContainer (c,0,OBU_NODE + RSU_NODE);
  NS_LOG_INFO ("Build Topology.");
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate (5000000)));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (50)));
  NetDeviceContainer n1 = csma.Install (c);

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
      
      // extract wsa recieve time, itt as csv file
      float m_wsaRecieveTime = wsa.time;
      float m_itt = ITT;
      std::ofstream out;
      out.open("V2X variables.csv", std::ios::app);
      out << m_wsaRecieveTime << "," 
          << m_itt
          << std::endl;
      out.close();

      InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), 80);
      Ptr<Socket> source = Socket::CreateSocket (c.Get (0), tid);
      source->SetAllowBroadcast (true);
      source->Connect (remote);
        
      Simulator::ScheduleWithContext (source->GetNode ()->GetId (),
                                      Seconds (j+1), &GenerateTraffic,
                                      source, send_wsa_packet, numPackets, interPacketInterval);
  
  uint16_t port = 9;
  NS_LOG_INFO ("Create Applications.");
  for(int i = 1; i <OBU_NODE + RSU_NODE ;i++)
  {
    OnOffHelper onoff ("ns3::UdpSocketFactory", 
                      Address (InetSocketAddress (Ipv4Address ("255.255.255.255"), port)));
    // 10Kb/s , 125 => 0.1s
    // 1Kb = 1000bit = 125 byte
    // 16Kb = 16000bit = 2000 byte
    char itt[7];
    strcpy(itt,recv_itt_data.c_str());
    if(j!=0)
      onoff.SetConstantRate (DataRate (itt),BSM_PACKET_SIZE);

    else
      onoff.SetConstantRate (DataRate ("16Kb/s"),BSM_PACKET_SIZE);

    ApplicationContainer app = onoff.Install (c.Get (i));
    // RSU가 측정하는 부분
    Ptr<Socket> recvSink = Socket::CreateSocket (c.Get (0), tid);
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
      string PVD_message = "car_info";
      uint8_t PVD_buffer[15];
      
      std::copy(PVD_message.begin(), PVD_message.end(), std::begin(PVD_buffer));

      send_pvd_packet = Create<Packet> (PVD_buffer,8);
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
                              source, send_pvd_packet, numPackets, interPacketInterval);
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
  // std::cout << "What is the recv_itt_data: " << recv_itt_data << std::endl;
  

  return 0;
}