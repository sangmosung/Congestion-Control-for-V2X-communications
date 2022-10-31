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
    // "ReceiveRate," <<
    // "PacketsReceived," <<
    "NumberOfSinks," <<
    "WavePktsSent," <<
    "WavePtksReceived," <<
    "WavePktsPpr," <<
    "ExpectedWavePktsReceived," <<
    "ExpectedWavePktsInCoverageReceived" <<
    std::endl;
  out.close ();


  // open log file for output
  m_os.open (m_logFile.c_str ());

  // Enable logging from the ns2 helper
  LogComponentEnable ("Ns2MobilityHelper",LOG_LEVEL_DEBUG);

  Packet::EnablePrinting ();

  NS_LOG_INFO ("Run Simulation.");

  // 이 부분부터 로그를 만드는 부분
  // CBR, VD 등의 로그는 기존 코드에서 받아와서 여기서 구성을 다시 해주어도 상관없다. 
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

  std::ofstream out (m_CSVfileName.c_str (), std::ios::app);

  out << (Simulator::Now ()).As (Time::S) << ","
      << m_nSinks << ","
      << wavePktsSent << ","
      << wavePktsReceived << ","
      << wavePDR << ","
      << waveExpectedRxPktCount << ","
      << waveRxPktInRangeCount << ""
      << std::endl;

  out.close ();

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