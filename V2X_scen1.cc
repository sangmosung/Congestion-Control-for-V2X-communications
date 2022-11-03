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

/**
 * @brief RSU struct to find the transmission period in RSU
 * @param prev_time parameter to store the time when the first BSM arrived in one cycle
 * @param current_time parameter to store the time when the last BSM arrived in one cycle
 * @param arrival_num parameter to store number of arrived BSM and to be reset every 1 sec
 * @param start_time_num parameter to evaluate the BSM start number
 */
typedef struct {
  float prev_time = 0;
  float current_time = 0;
  int arrival_num = 0;
  int start_time_num = 0;
}RSU;
/**
 * @brief WSA struct to store the time receiving WSA
 * @param time_wsa parameter to evaluate the time of arrival of WSA
 */
typedef struct {
  float time_wsa = 0;
}WSA;

#define OBU_NODE 500
#define RSU_NODE 1
#define ROW_LINE 10
#define TOTAL_TIME 100
#define BSM_PACKET_SIZE 200
#define BYTE_SIZE 8
NS_LOG_COMPONENT_DEFINE ("WifiSimpleOcb");

unsigned char recv_wsa_packet[100]; // buffer to save the WSA packet message
unsigned char recv_pvd_packet[100]; // buffer to save the PVD packet message
float time_diff; // the time difference between the time the first node received the bsm and the time the last node gave the bsm
int j_copy =0; // the number that equals the time the simulater runs and is updated every second.
float ITT; // transmission time to send BSM
float init_itt = 0.080; // initial transmission time
Ptr<Packet> wsa_packet; // WSA Packet
Ptr<Packet> pvd_packet; // PVD Packet
std::string recv_itt_data; // string to save the ITT data
std::string send_itt_data; // string to save the ITT data


RSU rsu;
WSA wsa;

/**
 * @brief the function that RSU receives the PVD message from each OBU
 * @param socket input socket
 **/
void ReceivePacket_PVD (Ptr<Socket> socket)
{
  while (socket->Recv (recv_pvd_packet,12,0))
    {
      string st = "";
      for(int i = 0 ; i<12 ; i++)
        st += recv_pvd_packet[i];
    }
}

/**
 * @brief the function that each OBU receives the WSA from RSU
 * @details OBU receives the packet from RSU and stores the message to recv_itt_data
 * @param socket input socket
 */
void ReceivePacket_WSA (Ptr<Socket> socket)
{
  while (socket->Recv (recv_wsa_packet,7,0))
    {
      recv_itt_data = "";
      for(int i = 0 ; i<7 ; i++)
        recv_itt_data += recv_wsa_packet[i];
    }
    wsa.time_wsa = Simulator::Now ().GetSeconds (); 
}

/**
 * @brief the function that RSU receives the BSM from all OBUs and caculated ITT
 * @details RSU receives the BSM from OBUs and calculates the CBR using struct RSU
 * @details RSU stores the ITT in wsa_packet using CBR and in csv file
 * @param socket input socket
 */
void ReceivePacket_BSM (Ptr<Socket> socket)
{
  while (socket->Recv ())
    {
      if(rsu.arrival_num==0)
      {
          rsu.prev_time = Simulator::Now ().GetSeconds ();
          printf("Simulation Start\n");
      }
      else if (rsu.arrival_num == OBU_NODE-1)
      {
          float cbr;
          char itt_char[2];
          rsu.current_time = Simulator::Now ().GetSeconds ();
          time_diff = rsu.current_time - rsu.prev_time;
          std::cout << Simulator::Now ().GetSeconds () << "s>> Time taken from BSM transmission to arrival: "<< time_diff <<  "[s]" << std::endl;
          itt_char[0] = recv_itt_data[0];
          itt_char[1] = recv_itt_data[1];
          int prev_itt = (itt_char[0]-'0')*10+(itt_char[1]-'0');
          if(j_copy ==0)
            cbr = time_diff/init_itt*100;
          else
            cbr = (time_diff)*(prev_itt*1000)/(BSM_PACKET_SIZE*BYTE_SIZE)*100;
          
          std::cout << Simulator::Now ().GetSeconds () << "s>> Channel Busy Ratio: "<< cbr  << "[%]"<< std::endl;
          
          float m_simulationTime = rsu.current_time;
          float m_cbr = cbr;
          std::ofstream out;
          out.open("V2X_variables2.csv", std::ios::app);
          out << m_simulationTime << ","
              << m_cbr << ",";
          out.close();
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
          wsa_packet = Create<Packet> (packet_buffer,200);
      }
      rsu.arrival_num++;

      for(int i =0; i <TOTAL_TIME ; i++)
      {
        if(j_copy ==i && rsu.start_time_num == i)
        { 
          rsu.arrival_num=1;
          rsu.prev_time = Simulator::Now ().GetSeconds ();  
          rsu.start_time_num += 1;
        }
      }
    }
  }

/**
 * @brief the function that sends the PVD packets
 * @param socket input socket
 * @param packet packet including the PVD
 * @param pktCount number of times to send
 * @param pktInterval time interval at which packets are sent
 */
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
/**
 * @brief the function that generates the WSA and sends the WSA packets
 * @details RSU generates the WSA and sends the WSA to all OBUs
 * @param socket input socket
 * @param packet packet to include the WSA
 * @param pktCount number of times to send
 * @param pktInterval time interval at which packets are sent
 */
static void GenerateTraffic_WSA (Ptr<Socket> socket, Ptr<Packet> packet,
                             uint32_t pktCount, Time pktInterval )
{
  if (pktCount > 0)
    {
      uint8_t packet_buffer[7]; 
      std::copy(send_itt_data.begin(), send_itt_data.end(), std::begin(packet_buffer));
      packet = Create<Packet> (packet_buffer,7);
      socket->Send(packet);
      std::cout << Simulator::Now ().GetSeconds () << "s>> ITT(" << ITT << ")를 담은 WSA 메시지가 전송되었습니다." << std::endl;
      printf("\n");
      Simulator::Schedule (pktInterval, &GenerateTraffic_WSA,
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
  uint32_t packetSize = 1000; 
  uint32_t numPackets = 1;
  std::string animFile = "wave-80211p.xml" ;
  double interval = 1.0;
  bool verbose = false;

  CommandLine cmd (__FILE__);

  cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
  cmd.AddValue ("packetSize", "size of application packet sent", packetSize);
  cmd.AddValue ("numPackets", "number of packets generated", numPackets);
  cmd.AddValue ("interval", "interval (seconds) between packets", interval);
  cmd.AddValue ("verbose", "turn on all WifiNetDevice log components", verbose);
  cmd.AddValue ("animFile",  "File Name for Animation Output", animFile);
  cmd.Parse (argc, argv);
  Time interPacketInterval = Seconds (interval);

  /**
   * @brief Simulate every sec from 0 sec to TOTAL_TIME
   */
  for(int j = 0 ; j<TOTAL_TIME; j++)
  {
    j_copy = j;
    NodeContainer c;
    c.Create (OBU_NODE + RSU_NODE);
    

    /**
     * @brief install wifi device to nodes 
     */
    YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
    Ptr<YansWifiChannel> channel = wifiChannel.Create ();
    wifiPhy.SetChannel (channel);
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
    NetDeviceContainer wave_devices = wifi80211p.Install (wifiPhy, wifi80211pMac, c);
    NS_LOG_INFO ("Build Topology.");

    /**
     * @brief install the csma to nodes
     */
    CsmaHelper csma;
    csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate (5000000)));
    csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (50)));
    NetDeviceContainer csma_devices = csma.Install (c);

    NetDeviceContainer total_devices = NetDeviceContainer (wave_devices, csma_devices);
    wifiPhy.EnablePcap ("wave-simple-80211p", false);

    /**
     * @brief assign positions to each nodes
     */
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

    /**
     * @brief assign the ip address to toal_devices
     */
    NS_LOG_INFO ("Enabling OLSR Routing");
    InternetStackHelper internet;
    internet.Install (c);

    Ipv4AddressHelper ipv4;
    NS_LOG_INFO ("Assign IP Addresses.");
    ipv4.SetBase ("10.0.0.0", "255.0.0.0");
    ipv4.Assign (total_devices);

    TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
    
    /**
     * @brief this step is the RSU sends the WSA to OBUs
     * @details RSU sends the WSA packet every sec using GenerateTraffic_WSA function and
     * @details OBUs receive the WSA packet using ReceivePacket_WSA function.
     * @details Finally, we make the csv file using m_itt and m_wsaReceiveTime
     */
    for(int k =1; k<OBU_NODE+RSU_NODE; k++)
      {
        Ptr<Socket> recvSink = Socket::CreateSocket (c.Get (k), tid);
        InetSocketAddress local = InetSocketAddress (Ipv4Address("255.255.255.255"), 80);
        recvSink->Bind (local);
        recvSink->SetRecvCallback (MakeCallback (&ReceivePacket_WSA));  // RSU receives the BSM according to ReceivePacket_WSA function                                   
      }
    float m_wsaReceiveTime = wsa.time_wsa;
    float m_itt = ITT;
    std::ofstream out;
    out.open("V2X_variables2.csv", std::ios::app);  // generates the csv file
    out << m_wsaReceiveTime << "," 
        << m_itt
        << std::endl;
    out.close();
    InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), 80);
    Ptr<Socket> source = Socket::CreateSocket (c.Get (0), tid);
    source->SetAllowBroadcast (true);
    source->Connect (remote);
    Simulator::ScheduleWithContext (source->GetNode ()->GetId (),                     // OBUs send the WSA using GenerateTraffic_WSA function
                                Seconds (j+1), &GenerateTraffic_WSA,                  // inputs of GenerateTraffic_PVD are source, wsa_packet
                                source, wsa_packet, numPackets, interPacketInterval); // numPackets, and interPacketInterval
    /**
     * @brief this step is all OBUs send the BSM to RSU
     * @details OBUs send the BSM packet according to itt based on csma and
     * @details RSU receives the BSM packet using ReceivePacket_BSM function 
     */
    uint16_t port = 9;
    NS_LOG_INFO ("Create Applications.");
    for(int i = 1; i <OBU_NODE + RSU_NODE ;i++)
    {
      OnOffHelper onoff ("ns3::UdpSocketFactory", 
                        Address (InetSocketAddress (Ipv4Address ("255.255.255.255"), port)));
      char itt[7];
      strcpy(itt,recv_itt_data.c_str());
      if(j!=0)
        onoff.SetConstantRate (DataRate (itt),BSM_PACKET_SIZE); // transmission varies according to itt and BSM_PACKET_SIZE.
      else
        onoff.SetConstantRate (DataRate ("20Kb/s"),BSM_PACKET_SIZE); // initial transmission time
      ApplicationContainer app = onoff.Install (c.Get (i)); // OBUs send the BSM using csma
      Ptr<Socket> recvSink = Socket::CreateSocket (c.Get (0), tid); // RSU is recv_socket
      InetSocketAddress local = InetSocketAddress (Ipv4Address("255.255.255.255"), port);
      recvSink->Bind (local);
      recvSink->SetRecvCallback (MakeCallback(&ReceivePacket_BSM)); // RSU receives the BSM according to ReceivePacket_BSM function
      app.Start (Seconds (0.0001+j));
      app.Stop (Seconds (1.0001+j));
    }

    /**
     * @brief Construct a new ns3::Packet Metadata::Enable object
     * @details Enable() must be set when sending real data in the packet.
     */
    ns3::PacketMetadata::Enable ();
    /**
     * @brief this step is the OBUs send the PVD to RSU
     * @details OBUs send the PVD data every sec using GenerateTraffic_PVD function and 
     * @details RSU receives the PVD using ReceivePacket_PVD function from OBUs
     */
    for(int i=1; i<OBU_NODE+RSU_NODE; i++)
      {
        string PVD_message = "car_info";
        uint8_t PVD_buffer[15];
        std::copy(PVD_message.begin(), PVD_message.end(), std::begin(PVD_buffer));
        pvd_packet = Create<Packet> (PVD_buffer,8); // packet to send the PVD from OBUs to RSU
        InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), i);
        Ptr<Socket> recvSink = Socket::CreateSocket (c.Get (0), tid);
        InetSocketAddress local = InetSocketAddress (Ipv4Address("255.255.255.255"), i);
        recvSink->Bind (local);
        recvSink->SetRecvCallback (MakeCallback(&ReceivePacket_PVD)); // RSU receives the PVD according to ReceivePacket_PVD function
        Ptr<Socket> source = Socket::CreateSocket (c.Get (i), tid);
        source->SetAllowBroadcast (true);
        source->Connect (remote);
        Simulator::ScheduleWithContext (source->GetNode ()->GetId (),                 // OBUs send the PVD using GenerateTraffic_PVD function
                                Seconds (j+1+i/(OBU_NODE)), &GenerateTraffic_PVD,     // inputs of GenerateTraffic_PVD are source, pvd_packet
                                source, pvd_packet, numPackets, interPacketInterval); // numPackets, and interPacketInterval
      }
    /**
     * @brief Create trace file for WSA, PVD, and BSM
     */
    AsciiTraceHelper ascii;
    csma.EnableAsciiAll (ascii.CreateFileStream ("V2X_congestion_control.tr"));

    /**
     * @brief Create animation file for WSA, PVD, and BSM
     */
    NS_LOG_INFO ("Run Simulation.");
    AnimationInterface anim (animFile);
    uint32_t rsu_icon = anim.AddResource("/home/smsung/Pictures/Base.png");
    uint32_t bluecar_icon = anim.AddResource("/home/smsung/Pictures/bluecar.png");
    Ptr<Node> rsu = c.Get(0);
    anim.UpdateNodeImage(rsu->GetId(),rsu_icon);
    anim.UpdateNodeSize(0,3,3);

    for (int i=1; i<OBU_NODE+1; i++)
    {
      Ptr<Node> greencar = c.Get(i);
      anim.UpdateNodeImage(greencar->GetId(),bluecar_icon);
    }
    anim.SetMaxPktsPerTraceFile(500000);
    /**
     * @brief Construct a new Simulator:: Run object
     * @details simulates the application sending BSM, WSA, and PVD
     */
    Simulator::Run ();
    std::cout << "Animation Trace file created:" << animFile.c_str ()<< std::endl;
    Simulator::Destroy ();

  }
  return 0;
}
