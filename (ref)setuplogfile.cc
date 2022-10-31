void
VanetRoutingExperiment::SetupLogFile ()
{
  // open log file for output
  m_os.open (m_logFile.c_str ());
}

void VanetRoutingExperiment::SetupLogging ()
{

  // Enable logging from the ns2 helper
  LogComponentEnable ("Ns2MobilityHelper",LOG_LEVEL_DEBUG);

  Packet::EnablePrinting ();
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
VanetRoutingExperiment::ProcessOutputs ()
{
  // calculate and output final results
  double bsm_pdr1 = m_waveBsmHelper.GetWaveBsmStats ()->GetCumulativeBsmPdr (1);
  double bsm_pdr2 = m_waveBsmHelper.GetWaveBsmStats ()->GetCumulativeBsmPdr (2);
  double bsm_pdr3 = m_waveBsmHelper.GetWaveBsmStats ()->GetCumulativeBsmPdr (3);
  double bsm_pdr4 = m_waveBsmHelper.GetWaveBsmStats ()->GetCumulativeBsmPdr (4);
  double bsm_pdr5 = m_waveBsmHelper.GetWaveBsmStats ()->GetCumulativeBsmPdr (5);
  double bsm_pdr6 = m_waveBsmHelper.GetWaveBsmStats ()->GetCumulativeBsmPdr (6);
  double bsm_pdr7 = m_waveBsmHelper.GetWaveBsmStats ()->GetCumulativeBsmPdr (7);
  double bsm_pdr8 = m_waveBsmHelper.GetWaveBsmStats ()->GetCumulativeBsmPdr (8);
  double bsm_pdr9 = m_waveBsmHelper.GetWaveBsmStats ()->GetCumulativeBsmPdr (9);
  double bsm_pdr10 = m_waveBsmHelper.GetWaveBsmStats ()->GetCumulativeBsmPdr (10);

  double averageRoutingGoodputKbps = 0.0;
  uint32_t totalBytesTotal = m_routingHelper->GetRoutingStats ().GetCumulativeRxBytes ();
  averageRoutingGoodputKbps = (((double) totalBytesTotal * 8.0) / m_TotalSimTime) / 1000.0;

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

  if (m_log != 0)
    {
      NS_LOG_UNCOND ("BSM_PDR1=" << bsm_pdr1 << " BSM_PDR2=" << bsm_pdr2 << " BSM_PDR3=" << bsm_pdr3 << " BSM_PDR4=" << bsm_pdr4 << " BSM_PDR5=" << bsm_pdr5 << " BSM_PDR6=" << bsm_pdr6 << " BSM_PDR7=" << bsm_pdr7 << " BSM_PDR8=" << bsm_pdr8 << " BSM_PDR9=" << bsm_pdr9 << " BSM_PDR10=" << bsm_pdr10 << " Goodput=" << averageRoutingGoodputKbps << "Kbps MAC/PHY-oh=" << mac_phy_oh);

    }

  std::ofstream out (m_CSVfileName2.c_str (), std::ios::app);

  out << bsm_pdr1 << ","
      << bsm_pdr2 << ","
      << bsm_pdr3 << ","
      << bsm_pdr4 << ","
      << bsm_pdr5 << ","
      << bsm_pdr6 << ","
      << bsm_pdr7 << ","
      << bsm_pdr8 << ","
      << bsm_pdr9 << ","
      << bsm_pdr10 << ","
      << averageRoutingGoodputKbps << ","
      << mac_phy_oh << ""
      << std::endl;

  out.close ();

  m_os.close (); // close log file
}