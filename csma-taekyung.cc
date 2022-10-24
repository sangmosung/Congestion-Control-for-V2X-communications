/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 */

//
// Example of the sending of a datagram to a broadcast address
//
// Network topology
//     ==============
//       |          |
//       n0    n1   n2
//       |     |
//     ==========
//
//   n0 originates UDP broadcast to 255.255.255.255/discard port, which 
//   is replicated and received on both n1 and n2

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/mobility-helper.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/ocb-wifi-mac.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/bsm-application.h"
#include "ns3/wave-bsm-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/aodv-module.h"
#include "ns3/olsr-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/dsr-module.h"
#include "ns3/applications-module.h"
#include "ns3/itu-r-1411-los-propagation-loss-model.h"
#include "ns3/ocb-wifi-mac.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/config-store-module.h"
#include "ns3/integer.h"
#include "ns3/wave-bsm-helper.h"
#include "ns3/wave-helper.h"
#include "ns3/yans-wifi-helper.h"
#include <random>

using namespace ns3;
using namespace dsr;


////////////////////////////////////////////////////////////////////// bsm message를 위한 코드 //////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



class WifiPhyStats : public Object
{
public:
  /**
   * \brief Gets the class TypeId
   * \return the class TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Constructor
   */
  WifiPhyStats ();

  /**
   * \brief Destructor
   */
  virtual ~WifiPhyStats ();

  /**
   * \brief Returns the number of bytes that have been transmitted
   * (this includes MAC/PHY overhead)
   * \return the number of bytes transmitted
   */
  uint32_t GetTxBytes ();

  /**
   * \brief Callback signiture for Phy/Tx trace
   * \param context this object
   * \param packet packet transmitted
   * \param mode wifi mode
   * \param preamble wifi preamble
   * \param txPower transmission power
   */
  void PhyTxTrace (std::string context, Ptr<const Packet> packet, WifiMode mode, WifiPreamble preamble, uint8_t txPower);

  /**
   * \brief Callback signiture for Phy/TxDrop
   * \param context this object
   * \param packet the tx packet being dropped
   */
  void PhyTxDrop (std::string context, Ptr<const Packet> packet);

  /**
   * \brief Callback signiture for Phy/RxDrop
   * \param context this object
   * \param packet the rx packet being dropped
   * \param reason the reason for the drop
   */
  void PhyRxDrop (std::string context, Ptr<const Packet> packet, WifiPhyRxfailureReason reason);

private:
  uint32_t m_phyTxPkts; ///< phy transmit packets
  uint32_t m_phyTxBytes; ///< phy transmit bytes
};



class RoutingStats
{
public:
  /**
   * \brief Constructor
   */
  RoutingStats ();

  /**
   * \brief Returns the number of bytes received
   * \return the number of bytes received
   */
  uint32_t GetRxBytes ();

  /**
   * \brief Returns the cumulative number of bytes received
   * \return the cumulative number of bytes received
   */
  uint32_t GetCumulativeRxBytes ();

  /**
   * \brief Returns the count of packets received
   * \return the count of packets received
   */
  uint32_t GetRxPkts ();

  /**
   * \brief Returns the cumulative count of packets received
   * \return the cumulative count of packets received
   */
  uint32_t GetCumulativeRxPkts ();

  /**
   * \brief Increments the number of (application-data)
   * bytes received, not including MAC/PHY overhead
   * \param rxBytes the number of bytes received
   */
  void IncRxBytes (uint32_t rxBytes);

  /**
   * \brief Increments the count of packets received
   */
  void IncRxPkts ();

  /**
   * \brief Sets the number of bytes received.
   * \param rxBytes the number of bytes received
   */
  void SetRxBytes (uint32_t rxBytes);

  /**
   * \brief Sets the number of packets received
   * \param rxPkts the number of packets received
   */
  void SetRxPkts (uint32_t rxPkts);

  /**
   * \brief Returns the number of bytes transmitted
   * \return the number of bytes transmitted
   */
  uint32_t GetTxBytes ();

  /**
   * \brief Returns the cumulative number of bytes transmitted
   * \return the cumulative number of bytes transmitted
   */
  uint32_t GetCumulativeTxBytes ();

  /**
   * \brief Returns the number of packets transmitted
   * \return the number of packets transmitted
   */
  uint32_t GetTxPkts ();

  /**
   * \brief Returns the cumulative number of packets transmitted
   * \return the cumulative number of packets transmitted
   */
  uint32_t GetCumulativeTxPkts ();

  /**
   * \brief Increment the number of bytes transmitted
   * \param txBytes the number of additional bytes transmitted
   */
  void IncTxBytes (uint32_t txBytes);

  /**
   * \brief Increment the count of packets transmitted
   */
  void IncTxPkts ();

  /**
   * \brief Sets the number of bytes transmitted
   * \param txBytes the number of bytes transmitted
   */
  void SetTxBytes (uint32_t txBytes);

  /**
   * \brief Sets the number of packets transmitted
   * \param txPkts the number of packets transmitted
   */
  void SetTxPkts (uint32_t txPkts);

private:
  uint32_t m_RxBytes; ///< reeive bytes
  uint32_t m_cumulativeRxBytes; ///< cumulative receive bytes
  uint32_t m_RxPkts; ///< receive packets
  uint32_t m_cumulativeRxPkts; ///< cumulative receive packets
  uint32_t m_TxBytes; ///< transmit bytes
  uint32_t m_cumulativeTxBytes; ///< cumulative transmit bytes
  uint32_t m_TxPkts; ///< transmit packets
  uint32_t m_cumulativeTxPkts; ///< cumulative transmit packets
};



class WifiApp
{
public:
  /**
   * \brief Constructor
   */
  WifiApp ();

  /**
   * \brief Destructor
   */
  virtual ~WifiApp ();

  /**
   * \brief Enacts simulation of an ns-3 wifi application
   * \param argc program arguments count
   * \param argv program arguments
   */
  void Simulate (int argc, char **argv);

protected:
  /**
   * \brief Sets default attribute values
   */
  virtual void SetDefaultAttributeValues ();

  /**
   * \brief Process command line arguments
   * \param argc program arguments count
   * \param argv program arguments
   */
  virtual void ParseCommandLineArguments (int argc, char **argv);

  /**
   * \brief Configure nodes
   */
  virtual void ConfigureNodes ();

  /**
   * \brief Configure channels
   */
  virtual void ConfigureChannels ();

  /**
   * \brief Configure devices
   */
  virtual void ConfigureDevices ();

  /**
   * \brief Configure mobility
   */
  virtual void ConfigureMobility ();

  /**
   * \brief Configure applications
   */
  virtual void ConfigureApplications ();

  /**
   * \brief Configure tracing
   */
  virtual void ConfigureTracing ();

  /**
   * \brief Run the simulation
   */
  virtual void RunSimulation ();

  /**
   * \brief Process outputs
   */
  virtual void ProcessOutputs ();
};



class RoutingHelper : public Object
{
public:
  /**
   * \brief Get class TypeId
   * \return the TypeId for the class
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Constructor
   */
  RoutingHelper ();

  /**
   * \brief Destructor
   */
  virtual ~RoutingHelper ();

  /**
   * \brief Installs routing functionality on nodes and their
   * devices and interfaces.
   * \param c node container
   * \param d net device container
   * \param i IPv4 interface container
   * \param totalTime the total time that nodes should attempt to
   * route data
   * \param protocol the routing protocol (1=OLSR;2=AODV;3=DSDV;4=DSR)
   * \param nSinks the number of nodes which will act as data sinks
   * \param routingTables dump routing tables at t=5 seconds (0=no;1=yes)
   */
  void Install (NodeContainer & c,
                NetDeviceContainer & d,
                Ipv4InterfaceContainer & i,
                double totalTime,
                int protocol,
                uint32_t nSinks,
                int routingTables);

  /**
   * \brief Trace the receipt of an on-off-application generated packet
   * \param context this object
   * \param packet a received packet
   */
  void OnOffTrace (std::string context, Ptr<const Packet> packet);

  /**
   * \brief Returns the RoutingStats instance
   * \return the RoutingStats instance
   */
  RoutingStats & GetRoutingStats ();

  /**
   * \brief Enable/disable logging
   * \param log non-zero to enable logging
   */
  void SetLogging (int log);

private:
  /**
   * \brief Sets up the protocol protocol on the nodes
   * \param c node container
   */
  void SetupRoutingProtocol (NodeContainer & c);

  /**
   * \brief Assigns IPv4 addresses to net devices and their interfaces
   * \param d net device container
   * \param adhocTxInterfaces IPv4 interface container
   */
  void AssignIpAddresses (NetDeviceContainer & d,
                          Ipv4InterfaceContainer & adhocTxInterfaces);

  /**
   * \brief Sets up routing messages on the nodes and their interfaces
   * \param c node container
   * \param adhocTxInterfaces IPv4 interface container
   */
  void SetupRoutingMessages (NodeContainer & c,
                             Ipv4InterfaceContainer & adhocTxInterfaces);

  /**
   * \brief Sets up a routing packet for tranmission
   * \param addr destination address
   * \param node source node
   * \return Socket to be used for sending/receiving a routed data packet
   */
  Ptr<Socket> SetupRoutingPacketReceive (Ipv4Address addr, Ptr<Node> node);

  /**
   * \brief Process a received routing packet
   * \param socket the receiving socket
   */
  void ReceiveRoutingPacket (Ptr<Socket> socket);

  double m_TotalSimTime;        ///< seconds
  uint32_t m_protocol;       ///< routing protocol; 0=NONE, 1=OLSR, 2=AODV, 3=DSDV, 4=DSR
  uint32_t m_port;           ///< port
  uint32_t m_nSinks;              ///< number of sink nodes (< all nodes)
  int m_routingTables;      ///< dump routing table (at t=5 sec).  0=No, 1=Yes
  RoutingStats routingStats; ///< routing statistics
  std::string m_protocolName; ///< protocol name
  int m_log; ///< log
};



class VanetRoutingExperiment : public WifiApp
{
public:
  /**
   * \brief Constructor
   */
  VanetRoutingExperiment ();

protected:
  /**
   * \brief Sets default attribute values
   */
  virtual void SetDefaultAttributeValues ();

  /**
   * \brief Process command line arguments
   * \param argc program arguments count
   * \param argv program arguments
   */
  virtual void ParseCommandLineArguments (int argc, char **argv);

  /**
   * \brief Configure nodes
   */
  virtual void ConfigureNodes ();

  /**
   * \brief Configure channels
   */
  virtual void ConfigureChannels ();

  /**
   * \brief Configure devices
   */
  virtual void ConfigureDevices ();

  /**
   * \brief Configure mobility
   */
  virtual void ConfigureMobility ();

  /**
   * \brief Configure applications
   */
  virtual void ConfigureApplications ();

  /**
   * \brief Configure tracing
   */
  virtual void ConfigureTracing ();

  /**
   * \brief Run the simulation
   */
  virtual void RunSimulation ();

  /**
   * \brief Process outputs
   */
  virtual void ProcessOutputs ();

private:
  /**
   * \brief Run the simulation
   */
  void Run ();

  /**
   * \brief Run the simulation
   * \param argc command line argument count
   * \param argv command line parameters
   */
  void CommandSetup (int argc, char **argv);

  /**
   * \brief Checks the throughput and outputs summary to CSV file1.
   * This is scheduled and called once per second
   */
  void CheckThroughput ();

  /**
   * \brief Set up log file
   */
  void SetupLogFile ();

  /**
   * \brief Set up logging
   */
  void SetupLogging ();

  /**
   * \brief Configure default attributes
   */
  void ConfigureDefaults ();

  /**
   * \brief Set up the adhoc mobility nodes
   */
  void SetupAdhocMobilityNodes ();

  /**
   * \brief Set up the adhoc devices
   */
  void SetupAdhocDevices ();

  /**
   * \brief Set up generation of IEEE 1609 WAVE messages,
   * as a Basic Safety Message (BSM).  The BSM is typically
   * a ~200-byte packets broadcast by all vehicles at a nominal
   * rate of 10 Hz
   */
  void SetupWaveMessages ();

  /**
   * \brief Set up generation of packets to be routed
   * through the vehicular network
   */
  void SetupRoutingMessages ();

  /**
   * \brief Set up a prescribed scenario
   */
  void SetupScenario ();

  /**
   * \brief Write the header line to the CSV file1
   */
  void WriteCsvHeader ();

  /**
   * \brief Set up configuration parameter from the global variables
   */
  void SetConfigFromGlobals ();

  /**
   * \brief Set up the global variables from the configuration parameters
   */
  void SetGlobalsFromConfig ();

  /**
   * Course change function
   * \param os the output stream
   * \param context trace source context (unused)
   * \param mobility the mobility model
   */
  static void
  CourseChange (std::ostream *os, std::string context, Ptr<const MobilityModel> mobility);

  uint32_t m_port; ///< port
  std::string m_CSVfileName; ///< CSV file name
  std::string m_CSVfileName2; ///< CSV file name
  uint32_t m_nSinks; ///< number of sinks
  std::string m_protocolName; ///< protocol name
  double m_txp; ///< distance
  bool m_traceMobility; ///< trace mobility
  uint32_t m_protocol; ///< protocol

  uint32_t m_lossModel; ///< loss model
  uint32_t m_fading; ///< fading
  std::string m_lossModelName; ///< loss model name

  std::string m_phyMode; ///< phy mode
  uint32_t m_80211mode; ///< 80211 mode

  std::string m_traceFile; ///< trace file 
  std::string m_logFile; ///< log file
  uint32_t m_mobility; ///< mobility
  uint32_t m_nNodes; ///< number of nodes
  double m_TotalSimTime; ///< total sim time
  std::string m_rate; ///< rate
  std::string m_phyModeB; ///< phy mode
  std::string m_trName; ///< trace file name
  int m_nodeSpeed; ///< in m/s
  int m_nodePause; ///< in s
  uint32_t m_wavePacketSize; ///< bytes
  double m_waveInterval; ///< seconds
  int m_verbose; ///< verbose
  std::ofstream m_os; ///< output stream
  NetDeviceContainer m_adhocTxDevices; ///< adhoc transmit devices
  Ipv4InterfaceContainer m_adhocTxInterfaces; ///< adhoc transmit interfaces
  uint32_t m_scenario; ///< scenario
  double m_gpsAccuracyNs; ///< GPS accuracy
  double m_txMaxDelayMs; ///< transmit maximum delay
  int m_routingTables; ///< routing tables
  int m_asciiTrace; ///< ascii trace
  int m_pcap; ///< PCAP
  std::string m_loadConfigFilename; ///< load config file name
  std::string m_saveConfigFilename; ///< save configi file name

  WaveBsmHelper m_waveBsmHelper; ///< helper
  Ptr<RoutingHelper> m_routingHelper; ///< routing helper
  Ptr<WifiPhyStats> m_wifiPhyStats; ///< wifi phy statistics
  int m_log; ///< log
  /// used to get consistent random numbers across scenarios
  int64_t m_streamIndex;
  NodeContainer m_adhocTxNodes; ///< adhoc transmit nodes
  double m_txSafetyRange1; ///< range 1
  double m_txSafetyRange2; ///< range 2
  double m_txSafetyRange3; ///< range 3
  double m_txSafetyRange4; ///< range 4
  double m_txSafetyRange5; ///< range 5
  double m_txSafetyRange6; ///< range 6
  double m_txSafetyRange7; ///< range 7
  double m_txSafetyRange8; ///< range 8
  double m_txSafetyRange9; ///< range 9
  double m_txSafetyRange10; ///< range 10
  std::vector <double> m_txSafetyRanges; ///< list of ranges
  std::string m_exp; ///< exp
  Time m_cumulativeBsmCaptureStart; ///< capture start
};

VanetRoutingExperiment::VanetRoutingExperiment ()
  : m_port (9),
    m_CSVfileName ("vanet-routing.output.csv"),
    m_CSVfileName2 ("vanet-routing.output2.csv"),
    m_nSinks (10),
    m_protocolName ("protocol"),
    m_txp (20),
    m_traceMobility (false),
    // AODV
    m_protocol (2),
    // Two-Ray ground
    m_lossModel (3),
    m_fading (0),
    m_lossModelName (""),
    m_phyMode ("OfdmRate6MbpsBW10MHz"),
    // 1=802.11p
    m_80211mode (1),
    m_traceFile (""),
    m_logFile ("low99-ct-unterstrass-1day.filt.7.adj.log"),
    m_mobility (1),
    m_nNodes (156),
    m_TotalSimTime (300.01),
    m_rate ("2048bps"),
    m_phyModeB ("DsssRate11Mbps"),
    m_trName ("vanet-routing-compare"),
    m_nodeSpeed (20),
    m_nodePause (0),
    m_wavePacketSize (200),
    m_waveInterval (0.1),
    m_verbose (0),
    m_scenario (1),
    m_gpsAccuracyNs (40),
    m_txMaxDelayMs (10),
    m_routingTables (0),
    m_asciiTrace (0),
    m_pcap (0),
    m_loadConfigFilename ("load-config.txt"),
    m_saveConfigFilename (""),
    m_log (0),
    m_streamIndex (0),
    m_adhocTxNodes (),
    m_txSafetyRange1 (50.0),
    m_txSafetyRange2 (100.0),
    m_txSafetyRange3 (150.0),
    m_txSafetyRange4 (200.0),
    m_txSafetyRange5 (250.0),
    m_txSafetyRange6 (300.0),
    m_txSafetyRange7 (350.0),
    m_txSafetyRange8 (400.0),
    m_txSafetyRange9 (450.0),
    m_txSafetyRange10 (500.0),
    m_txSafetyRanges (),
    m_exp (""),
    m_cumulativeBsmCaptureStart (0)
{
  m_wifiPhyStats = CreateObject<WifiPhyStats> ();
  m_routingHelper = CreateObject<RoutingHelper> ();

  // set to non-zero value to enable
  // simply uncond logging during simulation run
  m_log = 1;
}

void
VanetRoutingExperiment::SetupWaveMessages ()
{
  // WAVE PHY mode
  // 0=continuous channel; 1=channel-switching
  int chAccessMode = 0;
  if (m_80211mode == 3)
    {
      chAccessMode = 1;
    }

  m_waveBsmHelper.Install (m_adhocTxInterfaces,
                           Seconds (m_TotalSimTime),
                           m_wavePacketSize,
                           Seconds (m_waveInterval),
                           // GPS accuracy (i.e, clock drift), in number of ns
                           m_gpsAccuracyNs,
                           m_txSafetyRanges,
                           chAccessMode,
                           // tx max delay before transmit, in ms
                           MilliSeconds (m_txMaxDelayMs));

  // fix random number streams
  m_streamIndex += m_waveBsmHelper.AssignStreams (m_adhocTxNodes, m_streamIndex);
}



void
VanetRoutingExperiment::ConfigureApplications ()
{
  // Traffic mix consists of:
  // 2. Broadcasting of Basic Safety Message (BSM)
  SetupWaveMessages ();

  // config trace to capture app-data (bytes) for
  // routing data, subtracted and used for
  // routing overhead
  std::ostringstream oss;
  oss.str ("");
  oss << "/NodeList/*/ApplicationList/*/$ns3::OnOffApplication/Tx";
  Config::Connect (oss.str (), MakeCallback (&RoutingHelper::OnOffTrace, m_routingHelper));
}

// 이부분이 bsm message를 보내는 부분인것으로 추정
void
VanetRoutingExperiment::CheckThroughput ()
{
  uint32_t bytesTotal = m_routingHelper->GetRoutingStats ().GetRxBytes ();
  uint32_t packetsReceived = m_routingHelper->GetRoutingStats ().GetRxPkts ();
  double kbps = (bytesTotal * 8.0) / 1000;
  double wavePDR = 0.0;
  int wavePktsSent = m_waveBsmHelper.GetWaveBsmStats ()->GetTxPktCount ();
  int wavePktsReceived = m_waveBsmHelper.GetWaveBsmStats ()->GetRxPktCount ();
  if (wavePktsSent > 0)
    {
      int wavePktsReceived = m_waveBsmHelper.GetWaveBsmStats ()->GetRxPktCount ();
      wavePDR = (double) wavePktsReceived / (double) wavePktsSent;
    }

  int waveExpectedRxPktCount = m_waveBsmHelper.GetWaveBsmStats ()->GetExpectedRxPktCount (1);
  int waveRxPktInRangeCount = m_waveBsmHelper.GetWaveBsmStats ()->GetRxPktInRangeCount (1);
  double wavePDR1_2 = m_waveBsmHelper.GetWaveBsmStats ()->GetBsmPdr (1);
  double wavePDR2_2 = m_waveBsmHelper.GetWaveBsmStats ()->GetBsmPdr (2);
  double wavePDR3_2 = m_waveBsmHelper.GetWaveBsmStats ()->GetBsmPdr (3);
  double wavePDR4_2 = m_waveBsmHelper.GetWaveBsmStats ()->GetBsmPdr (4);
  double wavePDR5_2 = m_waveBsmHelper.GetWaveBsmStats ()->GetBsmPdr (5);
  double wavePDR6_2 = m_waveBsmHelper.GetWaveBsmStats ()->GetBsmPdr (6);
  double wavePDR7_2 = m_waveBsmHelper.GetWaveBsmStats ()->GetBsmPdr (7);
  double wavePDR8_2 = m_waveBsmHelper.GetWaveBsmStats ()->GetBsmPdr (8);
  double wavePDR9_2 = m_waveBsmHelper.GetWaveBsmStats ()->GetBsmPdr (9);
  double wavePDR10_2 = m_waveBsmHelper.GetWaveBsmStats ()->GetBsmPdr (10);

  // calculate MAC/PHY overhead (mac-phy-oh)
  // total WAVE BSM bytes sent
  uint32_t cumulativeWaveBsmBytes = m_waveBsmHelper.GetWaveBsmStats ()->GetTxByteCount ();
  uint32_t cumulativeRoutingBytes = m_routingHelper->GetRoutingStats ().GetCumulativeTxBytes ();
  uint32_t totalAppBytes = cumulativeWaveBsmBytes + cumulativeRoutingBytes;
  uint32_t totalPhyBytes = m_wifiPhyStats->GetTxBytes ();
  // mac-phy-oh = (total-phy-bytes - total-app-bytes) / total-phy-bytes
  double mac_phy_oh = 0.0;
  if (totalPhyBytes > 0)
    {
      mac_phy_oh = (double) (totalPhyBytes - totalAppBytes) / (double) totalPhyBytes;
    }

  std::ofstream out (m_CSVfileName.c_str (), std::ios::app);

  if (m_log != 0 )
    {
      NS_LOG_UNCOND ("At t=" << (Simulator::Now ()).As (Time::S) << " BSM_PDR1=" << wavePDR1_2 << " BSM_PDR1=" << wavePDR2_2 << " BSM_PDR3=" << wavePDR3_2 << " BSM_PDR4=" << wavePDR4_2 << " BSM_PDR5=" << wavePDR5_2 << " BSM_PDR6=" << wavePDR6_2 << " BSM_PDR7=" << wavePDR7_2 << " BSM_PDR8=" << wavePDR8_2 << " BSM_PDR9=" << wavePDR9_2 << " BSM_PDR10=" << wavePDR10_2 << " Goodput=" << kbps << "Kbps" /*<< " MAC/PHY-OH=" << mac_phy_oh*/);
    }

  out << (Simulator::Now ()).As (Time::S) << ","
      << kbps << ","
      << packetsReceived << ","
      << m_nSinks << ","
      << m_protocolName << ","
      << m_txp << ","
      << wavePktsSent << ","
      << wavePktsReceived << ","
      << wavePDR << ","
      << waveExpectedRxPktCount << ","
      << waveRxPktInRangeCount << ","
      << wavePDR1_2 << ","
      << wavePDR2_2 << ","
      << wavePDR3_2 << ","
      << wavePDR4_2 << ","
      << wavePDR5_2 << ","
      << wavePDR6_2 << ","
      << wavePDR7_2 << ","
      << wavePDR8_2 << ","
      << wavePDR9_2 << ","
      << wavePDR10_2 << ","
      << mac_phy_oh << ""
      << std::endl;

  out.close ();

  m_routingHelper->GetRoutingStats ().SetRxBytes (0);
  m_routingHelper->GetRoutingStats ().SetRxPkts (0);
  m_waveBsmHelper.GetWaveBsmStats ()->SetRxPktCount (0);
  m_waveBsmHelper.GetWaveBsmStats ()->SetTxPktCount (0);
  for (int index = 1; index <= 10; index++)
    {
      m_waveBsmHelper.GetWaveBsmStats ()->SetExpectedRxPktCount (index, 0);
      m_waveBsmHelper.GetWaveBsmStats ()->SetRxPktInRangeCount (index, 0);
    }

  Time currentTime = Simulator::Now ();
  if (currentTime <= m_cumulativeBsmCaptureStart)
    {
      for (int index = 1; index <= 10; index++)
        {
          m_waveBsmHelper.GetWaveBsmStats ()->ResetTotalRxPktCounts (index);
        }
    }

  Simulator::Schedule (Seconds (1.0), &VanetRoutingExperiment::CheckThroughput, this);
}

void
VanetRoutingExperiment::Run ()
{
  NS_LOG_INFO ("Run Simulation.");

  CheckThroughput ();

  Simulator::Stop (Seconds (m_TotalSimTime));
  Simulator::Run ();
  Simulator::Destroy ();
}

void
VanetRoutingExperiment::RunSimulation ()
{
  Run ();
}

void
WifiApp::Simulate (int argc, char **argv)
{
  ConfigureApplications ();
  RunSimulation ();
  // ProcessOutputs ();  이건 포함을 시켜야 하는걸까?
}


////////////////////////////////////////////////////////////////////// 여기서부터 기존 코드 시작 //////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


NS_LOG_COMPONENT_DEFINE ("CsmaBroadcastExample");

int main (int argc, char *argv[])
{

  // Users may find it convenient to turn on explicit debugging
  // for selected modules; the below lines suggest how to do this

#if 0
  LogComponentEnable ("CsmaBroadcastExample", LOG_LEVEL_INFO);
#endif
  LogComponentEnable ("CsmaBroadcastExample", LOG_PREFIX_TIME);

  // Allow the user to override any of the defaults and the above
  // Bind()s at run-time, via command-line arguments
  CommandLine cmd (__FILE__);
  std::string animFile = "CSMA.xml" ;  // Name of file for animation output

  cmd.AddValue ("animFile",  "File Name for Animation Output", animFile);

  cmd.Parse (argc, argv);
  NS_LOG_INFO ("Create nodes.");
  NodeContainer c;
  c.Create (100);
  NodeContainer c1 = NodeContainer (c,0,25);
  NodeContainer c2 = NodeContainer(c, 25, 50);
  NodeContainer c3 = NodeContainer(c, 50, 75);
  NodeContainer c4 = NodeContainer(c, 75, 100);
  NodeContainer c12 = NodeContainer(c, 0, 50);
  NodeContainer c34 = NodeContainer(c, 50, 100);
  NodeContainer c1234 = NodeContainer(c, 0, 100);

  NS_LOG_INFO ("Build Topology.");
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate (5000000)));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (5)));
  NetDeviceContainer n2 = csma.Install (c2);
  NetDeviceContainer n1 = csma.Install (c1);
  NetDeviceContainer n3 = csma.Install (c3);
  NetDeviceContainer n4 = csma.Install (c4);
  NetDeviceContainer n12 = csma.Install (c12);
  NetDeviceContainer n34 = csma.Install (c34);
  NetDeviceContainer n1234 = csma.Install (c1234);


  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  for(int i =0; i<10; i++)
  {
    for(int j=0;j<10;j++)
      positionAlloc->Add (Vector (1*j, 1*(i+1), 0.0));
  }
  mobility.SetPositionAllocator (positionAlloc);  
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (c);


  InternetStackHelper internet;
  internet.Install (c);


  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  // ipv4.SetBase ("192.168.1.0", "255.255.255.0");
  // ipv4.Assign (n1);
  // ipv4.SetBase ("10.1.0.0", "255.255.255.0");
  // ipv4.Assign (n2);
  // ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  // ipv4.Assign (n3);
  // ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  // ipv4.Assign (n4);
  // ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  // ipv4.Assign (n12);
  // ipv4.SetBase ("10.1.4.0", "255.255.255.0");
  // ipv4.Assign (n34);
  ipv4.SetBase ("10.1.5.0", "255.255.255.0");
  ipv4.Assign (n1234);
  

  // RFC 863 discard port ("9") indicates packet should be thrown away
  // by the system.  We allow this silent discard to be overridden
  // by the PacketSink application.
  uint16_t port = 9;

  // Create the OnOff application to send UDP datagrams of size
  // 512 bytes (default) at a rate of 500 Kb/s (default) from n0
  // NS_LOG_INFO ("Create Applications.");
  // for(int i =1; i <10 ; i++)
  // {
  //   if(i < 50 && i >1)
  //   {
  //     OnOffHelper onoff ("ns3::UdpSocketFactory", 
  //                       Address (InetSocketAddress (Ipv4Address ("255.255.255.255"), port+i)));
  //     onoff.SetConstantRate (DataRate ("500kb/s"));

  //     // ApplicationContainer app = onoff.Install (c0.Get (0));
  //     ApplicationContainer app = onoff.Install (c2.Get (i));
  //     // Start the application
  //     app.Start (Seconds (1.0));
  //     app.Stop (Seconds (5.0));

  //     // Create an optional packet sink to receive these packets
  //     PacketSinkHelper sink ("ns3::UdpSocketFactory",
  //                           Address (InetSocketAddress (Ipv4Address::GetAny (), port+i)));
  //     // app = sink.Install (c0.Get (1));
  //     app = sink.Install (c2.Get (1));
  //     // app.Add (sink.Install (c1.Get (1)));
  //     app.Start (Seconds (1.0));
  //     app.Stop (Seconds (5.0));
  //   }
  // }
  NS_LOG_INFO ("Create Applications.");
  
  for(int i = 0; i <100; i++)
  {
    OnOffHelper onoff1 ("ns3::UdpSocketFactory", 
                      Address (InetSocketAddress (Ipv4Address ("255.255.255.255"), port)));
    onoff1.SetConstantRate (DataRate ("500Kb/s"));



    ApplicationContainer app1 = onoff1.Install (c1234.Get (i));


    app1.Start (Seconds (5.0));
    app1.Stop (Seconds (10.0));
    // Start the application

    // // Create an optional packet sink to receive these packets
    // PacketSinkHelper sink1 ("ns3::UdpSocketFactory",
    //                       Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
    // // app = sink.Install (c0.Get (1));
    // app1 = sink1.Install (c1234.Get (i));
    // // app1.Add (sink1.Install (c1.Get (1)));
    // app1.Start (Seconds (5.0));
    // app1.Stop (Seconds (10.0));
  }

  // Configure ascii tracing of all enqueue, dequeue, and NetDevice receive 
  // events on all devices.  Trace output will be sent to the file 
  // "csma-one-subnet.tr"
  AsciiTraceHelper ascii;
  csma.EnableAsciiAll (ascii.CreateFileStream ("csma-broadcast.tr"));

  // Also configure some tcpdump traces; each interface will be traced
  // The output files will be named 
  // csma-broadcast-<nodeId>-<interfaceId>.pcap
  // and can be read by the "tcpdump -tt -r" command 
  csma.EnablePcapAll ("csma-broadcast", false);

  NS_LOG_INFO ("Run Simulation.");
  
  AnimationInterface anim (animFile);
  Simulator::Run ();
  std::cout << "Animation Trace file created:" << animFile.c_str ()<< std::endl;
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}
