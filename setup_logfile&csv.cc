#include <fstream>
#include <iostream>
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

using namespace ns3;
using namespace dsr;


  std::ofstream m_os; ///< output stream
  WaveBsmHelper m_waveBsmHelper; ///< helper
  Ptr<RoutingHelper> m_routingHelper; ///< routing helper

  
  //blank out the last output file and write the column headers
  std::ofstream out (m_CSVfileName.c_str ());
  out << "SimulationSecond," <<
    "ReceiveRate," <<
    "PacketsReceived," <<
    "NumberOfSinks," <<
    "RoutingProtocol," <<
    "TransmissionPower," <<
    "WavePktsSent," <<
    "WavePtksReceived," <<
    "WavePktsPpr," <<
    "ExpectedWavePktsReceived," <<
    "ExpectedWavePktsInCoverageReceived," <<
    "MacPhyOverhead" <<
    std::endl;
  out.close ();


  // open log file for output
  m_os.open (m_logFile.c_str ());

  // Enable logging from the ns2 helper
  LogComponentEnable ("Ns2MobilityHelper",LOG_LEVEL_DEBUG);

  Packet::EnablePrinting ();

  NS_LOG_INFO ("Run Simulation.");

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

  Simulator::Stop (Seconds (m_TotalSimTime));
  Simulator::Run ();
  Simulator::Destroy ();

  m_os.close (); // close log file