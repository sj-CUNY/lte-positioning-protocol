#include "scratch/locationCertificate.h"
double rxDelay = 0.0;
int numRxDelay = 0;
void
ServerConnectionEstablished (Ptr<const ThreeGppHttpServer>, Ptr<Socket>)
{
  NS_LOG_INFO ("Client has established a connection to the server.");
}

void
MainObjectGenerated (uint32_t size)
{
  NS_LOG_INFO ("Server generated a main object of " << size << " bytes.");
}

void
EmbeddedObjectGenerated (uint32_t size)
{
  NS_LOG_INFO ("Server generated an embedded object of " << size << " bytes.");
}

void
ServerTx (Ptr<const Packet> packet)
{
  NS_LOG_INFO ("Server sent a packet of " << packet->GetSize () << " bytes.");
  tx_3gpp += packet->GetSize();
}

void
ClientRx (Ptr<const Packet> packet, const Address &address)
{
  NS_LOG_INFO ("Client received a packet of " << packet->GetSize () << " bytes from " << address);
  rx_3gpp += packet->GetSize();
}

void
ClientMainObjectReceived (Ptr<const ThreeGppHttpClient>, Ptr<const Packet> packet)
{
  Ptr<Packet> p = packet->Copy ();
  ThreeGppHttpHeader header;
  p->RemoveHeader (header);
  if (header.GetContentLength () == p->GetSize ()
      && header.GetContentType () == ThreeGppHttpHeader::MAIN_OBJECT)
    {
      NS_LOG_INFO ("Client has successfully received a main object of "
                   << p->GetSize () << " bytes.");
    }
  else
    {
      NS_LOG_INFO ("Client failed to parse a main object. ");
    }
}

void
ClientEmbeddedObjectReceived (Ptr<const ThreeGppHttpClient>, Ptr<const Packet> packet)
{
  Ptr<Packet> p = packet->Copy ();
  ThreeGppHttpHeader header;
  p->RemoveHeader (header);
  if (header.GetContentLength () == p->GetSize ()
      && header.GetContentType () == ThreeGppHttpHeader::EMBEDDED_OBJECT)
    {
      NS_LOG_INFO ("Client has successfully received an embedded object of "
                   << p->GetSize () << " bytes.");
    }
  else
    {
      NS_LOG_INFO ("Client failed to parse an embedded object. ");
    }
}
void
ClientRxDelay(const Time &t, const uint32_t &addr)
{
   httpDelay[addr] += t.GetMilliSeconds();
   num_httpDelay[addr]++;

   std::cout << "Average delay so far " << httpDelay[addr]/double(num_httpDelay[addr]) << std::endl;
} 
void
ClientWebpageCompleted(const Time &t, const uint32_t &size, const uint32_t &addr)
{
   double rT = t.GetMilliSeconds();
   throughput[addr] += size/rT;   
   num_pages[addr]++;
   responseTime[addr] += rT;
   web_size[addr] += size;
   std::cout << "Average throughput " << size/rT << " from " << addr <<  std::endl;
   std::cout << "Average responseTime " << rT << " size " << size << " from " << addr <<  std::endl;
} 
void
UdpPacketTransmitted(Ptr<const Packet> p)
{
     udp_sent++;
     std::cout << "udp sent  " <<  udp_sent <<  std::endl;
}

void
UdpPacketReceived(Ptr<const Packet> p, const Address &addr)
{
     udp_recvd++;
     udp_rxsize += p->GetSize();
     std::cout << "udp received  " << udp_recvd << " fraction delivered " << udp_recvd/double(udp_sent) << " at " << addr <<  std::endl;
}
void
start3gpp(NodeContainer ueNodes, Ptr<Node> remoteHost, Ipv4Address remoteHostAddr)
{

  // Install and start applications on UEs and remote host
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;
 // Create HTTP server helper
      ThreeGppHttpServerHelper serverHelper (remoteHostAddr);
  // Install HTTP server
      serverApps = serverHelper.Install (remoteHost);
      Ptr<ThreeGppHttpServer> httpServer = serverApps.Get (0)->GetObject<ThreeGppHttpServer> ();

  // Example of connecting to the trace sources
      httpServer->TraceConnectWithoutContext ("ConnectionEstablished",
                                          MakeCallback (&ServerConnectionEstablished));
      httpServer->TraceConnectWithoutContext ("MainObject", MakeCallback (&MainObjectGenerated));
      httpServer->TraceConnectWithoutContext ("EmbeddedObject", MakeCallback (&EmbeddedObjectGenerated));
      httpServer->TraceConnectWithoutContext ("Tx", MakeCallback (&ServerTx));

  // Setup HTTP variables for the server
  PointerValue varPtr;
  httpServer->GetAttribute ("Variables", varPtr);
  Ptr<ThreeGppHttpVariables> httpVariables = varPtr.Get<ThreeGppHttpVariables> ();
  httpVariables->SetMainObjectSizeMean (102400); // 100kB
  httpVariables->SetMainObjectSizeStdDev (40960); // 40kB



  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
  // Create HTTP client helper
  ThreeGppHttpClientHelper clientHelper (remoteHostAddr);

  // Install HTTP client
  ApplicationContainer clientApps = clientHelper.Install (ueNodes.Get (u));
  Ptr<ThreeGppHttpClient> httpClient = clientApps.Get (0)->GetObject<ThreeGppHttpClient> ();

  // Example of connecting to the trace sources
  httpClient->TraceConnectWithoutContext ("RxMainObject", MakeCallback (&ClientMainObjectReceived));
  httpClient->TraceConnectWithoutContext ("RxEmbeddedObject", MakeCallback (&ClientEmbeddedObjectReceived));
  httpClient->TraceConnectWithoutContext ("Rx", MakeCallback (&ClientRx));
  httpClient->TraceConnectWithoutContext ("RxRtt", MakeCallback (&ClientRxDelay));
  httpClient->TraceConnectWithoutContext ("Response", MakeCallback (&ClientWebpageCompleted));


    }
  serverApps.Start (Seconds (0.01));
  clientApps.Start (Seconds (0.1));
  // Uncomment to enable PCAP tracing
  //p2ph.EnablePcapAll("lena-epc-first");
}

void
startUdp(NodeContainer ueNodes, Ptr<Node> udpHost)
{
    uint16_t dlPort = 1234;
   PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory",
                                   InetSocketAddress (Ipv4Address::GetAny (), dlPort));
  Ptr<UniformRandomVariable> startTimeSeconds = CreateObject<UniformRandomVariable> ();
   for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
   {   
	ApplicationContainer serverApps = packetSinkHelper.Install (ueNodes.Get(u));
        Ptr<PacketSink> psink = serverApps.Get(0)->GetObject<PacketSink>();
         psink->TraceConnectWithoutContext("Rx", MakeCallback(&UdpPacketReceived));
   	serverApps.Start (MilliSeconds (0.01));
   	UdpClientHelper client (ueNodes.Get(u)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), dlPort);

  	ApplicationContainer clientApps = client.Install (udpHost);
        Ptr<UdpClient> udpClient = clientApps.Get(0)->GetObject<UdpClient>();
  	udpClient->TraceConnectWithoutContext ("TxDone", MakeCallback (&UdpPacketTransmitted));
        clientApps.Start (Seconds (1+startTimeSeconds->GetValue()));
//        ++dlPort;
   }

}

void
startLpp(NodeContainer ueNodes, Ptr<Node> lppHost)
{
  // Install and start applications on UEs and remote host
  uint16_t otherPort = 9;
  ApplicationContainer serverApps;
  Ptr<UniformRandomVariable> startTimeSeconds = CreateObject<UniformRandomVariable> ();
  Ptr<Ipv4> lpp_ip = lppHost->GetObject<Ipv4>();
  Ipv4Address lpp_addr = lpp_ip->GetAddress(1,0).GetLocal();
   
  LppServerHelper serverHelper (lpp_addr);
  serverApps = serverHelper.Install (lppHost);
  //random_data(s, rand() % MAX_DATA);  
  //serverHelper.SetFill(serverApps.Get(0), s);
   
  serverApps.Start (MilliSeconds (0.01));

  LppClientHelper lppClientHelper (lpp_addr, otherPort);
  lppClientHelper.SetAttribute("Interval", TimeValue(Seconds(1)));
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      ApplicationContainer clientApps;
      Ptr<Node> n = ueNodes.Get(u);
      clientApps = lppClientHelper.Install (n);
      Location *lc = new Location(); 
      lppClientHelper.CertificateRxTrace(clientApps.Get(0), MakeCallback(&Location::CertificateReceived, lc));
      lppClientHelper.CertificateReqTrace(clientApps.Get(0), MakeCallback(&Location::CertificateRequested, lc));
      clientApps.Start (MilliSeconds (startTimeSeconds->GetValue()*120));
    }
}

int main (int argc, char *argv[])
{

    double distance = 1000;
    //LogComponentEnable ("LppSim", LOG_LEVEL_ALL);
    //ConfigParams(argc, argv);
    CommandLine cmd;
//  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("RawText"));
  
    ConfigStore inputConfig;
    inputConfig.ConfigureDefaults ();
	// parse again so you can override default values from the command line
    cmd.AddValue ("numUe", "number of UEs", numberOfNodes);
    cmd.AddValue ("numEnb", "number of enodeBs", numEnb);
    cmd.AddValue ("pmodel", "Propagation Loss Model to use", p_model);
    cmd.AddValue ("time", "Simulation time", simTime);
    cmd.AddValue ("output", "filename", fname);
    cmd.AddValue ("lpp", "using lpp", lpp);
    cmd.AddValue ("udp", "using udp", udp);
    cmd.AddValue ("threeGpp", "using 3gpp", threeGpp);
    cmd.AddValue ("fading", "using fading", fading);
    cmd.AddValue ("distance", "Distance between enbNodes", distance);
    
    cmd.Parse (argc, argv);
    Box macroUeBox = Box (-distance * 0.5, distance * 1.5, -distance * 0.5, distance * 1.5, 1.5, 1.5);
    lpp_out.open(fname+"lpp.plt");
    http_out.open(fname+"http.plt");
    Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
    setScheduler(lteHelper);
    if (fading == true)
	    setFadingModel(lteHelper);
    setPathLossModel(lteHelper, p_model);
//"ns3::FriisPropagationLossModel");

    Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
    lteHelper->SetEpcHelper(epcHelper); 
    
    Ptr<Node> pgw = epcHelper->GetPgwNode();
    NodeContainer enbNodes;
    enbNodes.Create(numEnb);
    NodeContainer ueNodes;
    ueNodes.Create(numberOfNodes);

     InternetStackHelper internet;
    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    NodeContainer remoteHostContainer;
    remoteHostContainer.Create (3);
 
// we install the IP stack on the remote hosts
    internet.Install (remoteHostContainer);

  // Create the Internet and connect remote hosts to the pgw
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
 
  //Configure ip addresses on each remote host separately

  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
 for (uint32_t i = 0; i < remoteHostContainer.GetN(); ++i)
  {	
  NetDeviceContainer internetDevices;
  internetDevices = p2ph.Install (pgw, remoteHostContainer.Get(i));
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  ipv4h.NewNetwork(); 
  // interface 0 is localhost, 1 is the p2p device

  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHostContainer.Get(i)->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
  }

  setMobility2(distance, ueNodes, enbNodes, macroUeBox);

  Config::SetDefault ("ns3::LteEnbRrc::DefaultTransmissionMode", UintegerValue (4)); // MIMO Multiuser


    NetDeviceContainer enbDevs;
    enbDevs = lteHelper->InstallEnbDevice(enbNodes);
    NetDeviceContainer ueDevs;
    ueDevs = lteHelper->InstallUeDevice(ueNodes);
   

// we install the IP stack on the UEs
    internet.Install (ueNodes);

    Ipv4InterfaceContainer ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer(ueDevs));
// assign IP address to UEs
    for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
  {
    Ptr<Node> ue = ueNodes.Get (u);
    // set the default gateway for the UE
    Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4> ());
    ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
  }
    lteHelper->Attach(ueDevs);

  delays = new uint32_t[numberOfNodes+enbNodes.GetN()];
  num_delays = new uint32_t[numberOfNodes+enbNodes.GetN()];
  responseTime = new uint32_t[numberOfNodes+enbNodes.GetN()];
  web_size = new uint32_t[numberOfNodes+enbNodes.GetN()];
  httpDelay = new uint32_t[numberOfNodes+enbNodes.GetN()];
  num_httpDelay = new uint32_t[numberOfNodes+enbNodes.GetN()];
  throughput = new double[numberOfNodes+enbNodes.GetN()];
  num_pages = new uint32_t[numberOfNodes+enbNodes.GetN()];

  for (uint32_t i = 0; i < numberOfNodes+enbNodes.GetN(); ++i)
  {
	delays[i] = 0;
	num_delays[i] = 0;
	responseTime[i] = 0;
        web_size[i] = 0;
	throughput[i] = 0;
	num_pages[i] = 0;
	num_httpDelay[i] = 0;
	httpDelay[i] = 0;
  } 

  if (lpp == true)
	startLpp(ueNodes, remoteHostContainer.Get(1));
  
  if (threeGpp == true)
	start3gpp(ueNodes, remoteHostContainer.Get(0), remoteHostContainer.Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal());
  if (udp == true)
        startUdp(ueNodes,remoteHostContainer.Get(2)); 
 // lteHelper->EnableTraces();
   // lteHelper->EnableLogComponents();
 // AsciiTraceHelper ascii;
 // p2ph.EnableAsciiAll(ascii.CreateFileStream("lpp-udp.tr"));


    Simulator::Schedule(Seconds(simTime), &PlotLppDelay, "lc_delay_plot.pdf", ueNodes);
    Simulator::Schedule(Seconds(simTime), &PlotHttpParams, "http_params_plot.pdf", ueNodes);
    Simulator::Stop(Seconds(simTime+1.0));

    Simulator::Run();

    Simulator::Destroy();
    return 0;
}



