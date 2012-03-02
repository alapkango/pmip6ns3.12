/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *
 * Proxy Mobile IPv6 (RFC5213) Implementation in NS3 
 *
 * Korea Univerity of Technology and Education (KUT)
 * Electronics and Telecommunications Research Institute (ETRI)
 *
 * Author: Hyon-Young Choi <commani@gmail.com>
 *
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/pmip6-module.h"
#include "ns3/wimax-module.h"
#include "ns3/csma-module.h"
#include "ns3/bridge-module.h"

#include "ns3/ipv6-static-routing.h"
#include "ns3/ipv6-static-source-routing.h"
#include "ns3/ipv6-routing-table-entry.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>

NS_LOG_COMPONENT_DEFINE ("Pmip6Wimax");

using namespace ns3;

Ipv6InterfaceContainer AssignIpv6Address(Ptr<NetDevice> device, Ipv6Address addr, Ipv6Prefix prefix);
Ipv6InterfaceContainer AssignWithoutAddress(Ptr<NetDevice> device);

int main (int argc, char *argv[])
{

  NodeContainer sta;
  NodeContainer cn;
  NodeContainer backbone;
  NodeContainer aps;

  //ref nodes
  NodeContainer lma;
  NodeContainer mags;
  NodeContainer outerNet;
  NodeContainer mag1Net;
  NodeContainer mag2Net;
  
  NetDeviceContainer backboneDevs;
  NetDeviceContainer outerDevs;
  NetDeviceContainer mag1Devs;
  NetDeviceContainer mag2Devs;
  NetDeviceContainer mag1ApDev;
  NetDeviceContainer mag2ApDev;
  NetDeviceContainer mag1BrDev;
  NetDeviceContainer mag2BrDev;
  NetDeviceContainer staDevs;
  
  Ipv6InterfaceContainer backboneIfs;
  Ipv6InterfaceContainer outerIfs;
  Ipv6InterfaceContainer mag1Ifs;
  Ipv6InterfaceContainer mag2Ifs;
  Ipv6InterfaceContainer staIfs;
  
  CommandLine cmd;
  cmd.Parse (argc, argv);
  
  SeedManager::SetSeed (123456);

//  LogLevel logAll = static_cast<LogLevel>(LOG_PREFIX_TIME | LOG_PREFIX_NODE | LOG_LEVEL_ALL);
//  LogLevel logLogic = static_cast<LogLevel>(LOG_PREFIX_TIME | LOG_PREFIX_NODE | LOG_LEVEL_LOGIC);
  LogLevel logInfo = static_cast<LogLevel>(LOG_PREFIX_TIME | LOG_PREFIX_NODE | LOG_LEVEL_INFO);

  LogComponentEnable ("Udp6Server", logInfo);
 
  backbone.Create(3);
  aps.Create(2);
  cn.Create(1);
  sta.Create(1);

  InternetStackHelper internet;
  internet.Install (backbone);
  internet.Install (aps);
  internet.Install (cn);
  internet.Install (sta);

  lma.Add(backbone.Get(0));
  
  mags.Add(backbone.Get(1));
  mags.Add(backbone.Get(2));
  
  outerNet.Add(lma);
  outerNet.Add(cn);
  
  mag1Net.Add(mags.Get(0));
  mag1Net.Add(aps.Get(0));

  mag2Net.Add(mags.Get(1));
  mag2Net.Add(aps.Get(1));

  CsmaHelper csma, csma1;
  
  //MAG's MAC Address (for unify default gateway of MN)
  Mac48Address magMacAddr("00:00:AA:BB:CC:DD");

  Ipv6InterfaceContainer iifc;
  
  //Link between CN and LMA is 50Mbps and 1ms delay
  csma1.SetChannelAttribute ("DataRate", DataRateValue (DataRate(50000000)));
  csma1.SetChannelAttribute ("Delay", TimeValue (MilliSeconds(1)));
  csma1.SetDeviceAttribute ("Mtu", UintegerValue (1400));
  
  outerDevs = csma1.Install(outerNet);
  iifc = AssignIpv6Address(outerDevs.Get(0), Ipv6Address("3ffe:2::1"), 64);
  outerIfs.Add(iifc);
  iifc = AssignIpv6Address(outerDevs.Get(1), Ipv6Address("3ffe:2::2"), 64);
  outerIfs.Add(iifc);
  outerIfs.SetRouter(0, true);

  //All Link is 50Mbps and 1ms delay
  csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate(50000000)));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds(1)));
  csma.SetDeviceAttribute ("Mtu", UintegerValue (1400));

  backboneDevs = csma.Install(backbone);
  iifc = AssignIpv6Address(backboneDevs.Get(0), Ipv6Address("3ffe:1::1"), 64);
  backboneIfs.Add(iifc);
  iifc = AssignIpv6Address(backboneDevs.Get(1), Ipv6Address("3ffe:1::2"), 64);
  backboneIfs.Add(iifc);
  iifc = AssignIpv6Address(backboneDevs.Get(2), Ipv6Address("3ffe:1::3"), 64);
  backboneIfs.Add(iifc);
  backboneIfs.SetRouter(0, true);
  
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc;
  
  positionAlloc = CreateObject<ListPositionAllocator> ();
  
  positionAlloc->Add (Vector (0.0, -20.0, 0.0));   //LMA
  positionAlloc->Add (Vector (-50.0, 20.0, 0.0)); //MAG1
  positionAlloc->Add (Vector (50.0, 20.0, 0.0));  //MAG2
  
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  
  mobility.Install (backbone);
  
  positionAlloc = CreateObject<ListPositionAllocator> ();
  
  positionAlloc->Add (Vector (75.0, -20.0, 0.0));   //CN
  
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  
  mobility.Install (cn);
  
  positionAlloc = CreateObject<ListPositionAllocator> ();
  
  positionAlloc->Add (Vector (-50.0, 40.0, 0.0)); //MAG1AP
  positionAlloc->Add (Vector (50.0, 40.0, 0.0));  //MAG2AP
  
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  
  mobility.Install (aps);
  
  positionAlloc = CreateObject<ListPositionAllocator> ();
  
  positionAlloc->Add (Vector (-50.0, 60.0, 0.0)); //STA
  
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");  
  mobility.Install(sta);
  
  Ptr<ConstantVelocityMobilityModel> cvm = sta.Get(0)->GetObject<ConstantVelocityMobilityModel>();
  cvm->SetVelocity(Vector (10.0, 0, 0)); //move to left to right 10.0m/s
  
  ////////////////////
  WimaxHelper wimax;
  BridgeHelper bridge;
  
  Ptr<WimaxChannel> wimaxChannel = CreateObject<SimpleOfdmWimaxChannel> (SimpleOfdmWimaxChannel::COST231_PROPAGATION);
  
  //Setting MAG1 and Wimax-BS
  mag1Devs = csma.Install(mag1Net);
  mag1Devs.Get(0)->SetAddress(magMacAddr);
  
  mag1Ifs = AssignIpv6Address(mag1Devs.Get(0), Ipv6Address("3ffe:1:1::1"), 64);
  
  mag1ApDev.Add (wimax.Install (mag1Net.Get (1),
                             WimaxHelper::DEVICE_TYPE_BASE_STATION,
							 WimaxHelper::SIMPLE_PHY_TYPE_OFDM,
							 wimaxChannel,
							 WimaxHelper::SCHED_TYPE_SIMPLE));

  mag1BrDev = bridge.Install (aps.Get (0), NetDeviceContainer (mag1ApDev, mag1Devs.Get (1)));
  
  iifc = AssignWithoutAddress(mag1Devs.Get(1));
  mag1Ifs.Add(iifc);
  mag1Ifs.SetRouter(0, true);
  
  //Setting MAG2
  mag2Devs = csma.Install(mag2Net);
  mag2Devs.Get(0)->SetAddress(magMacAddr);
  
  mag2Ifs = AssignIpv6Address(mag2Devs.Get(0), Ipv6Address("3ffe:1:2::1"), 64);
  
  mag2ApDev.Add (wimax.Install (mag2Net.Get (1),
                             WimaxHelper::DEVICE_TYPE_BASE_STATION,
							 WimaxHelper::SIMPLE_PHY_TYPE_OFDM,
							 CreateObject<SimpleOfdmWimaxChannel> (SimpleOfdmWimaxChannel::COST231_PROPAGATION),
							 WimaxHelper::SCHED_TYPE_SIMPLE));
  
  mag2BrDev = bridge.Install (aps.Get (1), NetDeviceContainer (mag2ApDev, mag2Devs.Get (1)));
  
  iifc = AssignWithoutAddress(mag2Devs.Get(1));
  mag2Ifs.Add(iifc);
  mag2Ifs.SetRouter(0, true);
  
  //setting station
  staDevs.Add (wimax.Install (sta.Get (0),
                              WimaxHelper::DEVICE_TYPE_SUBSCRIBER_STATION,
             				  WimaxHelper::SIMPLE_PHY_TYPE_OFDM,
							  wimaxChannel,
							  WimaxHelper::SCHED_TYPE_SIMPLE));

  Ptr<SubscriberStationNetDevice> ssDev;
  ssDev = staDevs.Get (0)->GetObject<SubscriberStationNetDevice> ();
  ssDev->SetModulationType (WimaxPhy::MODULATION_TYPE_QAM64_34);
  
  // Create one BE Downlink service flow between the ss and the bs
  ServiceFlow * DlServiceFlowBe = new ServiceFlow (ServiceFlow::SF_DIRECTION_DOWN);
  DlServiceFlowBe->SetCsSpecification (ServiceFlow::IPV6);
  DlServiceFlowBe->SetServiceSchedulingType (ServiceFlow::SF_TYPE_BE);
  DlServiceFlowBe->SetMaxSustainedTrafficRate (50000000);
  DlServiceFlowBe->SetMinReservedTrafficRate (50000000);
  DlServiceFlowBe->SetMinTolerableTrafficRate (50000000);
  DlServiceFlowBe->SetMaximumLatency (10);
  DlServiceFlowBe->SetMaxTrafficBurst (1000);
  DlServiceFlowBe->SetTrafficPriority (1);

  // Create one BE Uplink service flow between the ss and the bs
  ServiceFlow * UlServiceFlowBe = new ServiceFlow (ServiceFlow::SF_DIRECTION_UP);
  UlServiceFlowBe->SetCsSpecification (ServiceFlow::IPV6);
  UlServiceFlowBe->SetServiceSchedulingType (ServiceFlow::SF_TYPE_BE);
  UlServiceFlowBe->SetMaxSustainedTrafficRate (50000000);
  UlServiceFlowBe->SetMinReservedTrafficRate (50000000);
  UlServiceFlowBe->SetMinTolerableTrafficRate (50000000);
  UlServiceFlowBe->SetMaximumLatency (10);
  UlServiceFlowBe->SetMaxTrafficBurst (1000);
  UlServiceFlowBe->SetTrafficPriority (1);  

  ssDev->AddServiceFlow (DlServiceFlowBe);
  ssDev->AddServiceFlow (UlServiceFlowBe);  
  
  iifc = AssignWithoutAddress(staDevs.Get(0)); 
  staIfs.Add(iifc);
  
  //attach PMIPv6 agents
  Pmip6ProfileHelper *profile = new Pmip6ProfileHelper();

  //adding profile for each station  
  profile->AddProfile(Identifier("pmip1@example.com"), Identifier(Mac48Address::ConvertFrom(staDevs.Get(0)->GetAddress())), backboneIfs.GetAddress(0, 1), std::list<Ipv6Address>());

  Pmip6LmaHelper lmahelper;
  lmahelper.SetPrefixPoolBase(Ipv6Address("3ffe:1:4::"), 48);
  lmahelper.SetProfileHelper(profile);
  
  lmahelper.Install(lma.Get(0));
  
  Pmip6MagHelper maghelper;

  maghelper.SetProfileHelper(profile);  
  
  maghelper.Install (mags.Get(0), mag1Ifs.GetAddress(0, 0), aps.Get(0));
  maghelper.Install (mags.Get(1), mag2Ifs.GetAddress(0, 0), aps.Get(1));
  
  AsciiTraceHelper ascii;
  csma.EnableAsciiAll (ascii.CreateFileStream ("pmip6-wimax.tr"));
  csma.EnablePcapAll (std::string ("pmip6-wimax"), false);
  
  wimax.EnablePcap ("pmip6-wimax", mag1ApDev.Get(0));
  wimax.EnablePcap ("pmip6-wimax", mag2ApDev.Get(0));
  wimax.EnablePcap ("pmip6-wimax", staDevs.Get(0));

  /* Create a Ping6 application to send ICMPv6 echo request from node zero to
   */
  Udp6ServerHelper udpServer(6000);

  ApplicationContainer apps = udpServer.Install (sta.Get (0));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (10.0));

  uint32_t packetSize = 1024;
  uint32_t maxPacketCount = 0xffffffff;
  Time interPacketInterval = MilliSeconds(10);
  Udp6ClientHelper udpClient(Ipv6Address("3ffe:1:4:1:200:ff:fe00:c"), 6000);
  
//  udpClient.SetIfIndex (outerIfs.GetInterfaceIndex (1));
  udpClient.SetAttribute ("Interval", TimeValue (interPacketInterval));
  udpClient.SetAttribute ("PacketSize", UintegerValue (packetSize));
  udpClient.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));

  apps = udpClient.Install (cn.Get (0));
  apps.Start (Seconds (1.5));
  apps.Stop (Seconds (10.0));  
  
  Simulator::Stop (Seconds (10.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}

Ipv6InterfaceContainer AssignIpv6Address(Ptr<NetDevice> device, Ipv6Address addr, Ipv6Prefix prefix)
{
  Ipv6InterfaceContainer retval;

  Ptr<Node> node = device->GetNode ();
  NS_ASSERT_MSG (node, "Ipv6AddressHelper::Allocate (): Bad node");

  Ptr<Ipv6> ipv6 = node->GetObject<Ipv6> ();
  NS_ASSERT_MSG (ipv6, "Ipv6AddressHelper::Allocate (): Bad ipv6");
  int32_t ifIndex = 0;

  ifIndex = ipv6->GetInterfaceForDevice (device);
  if (ifIndex == -1)
    {
      ifIndex = ipv6->AddInterface (device);
    }
  NS_ASSERT_MSG (ifIndex >= 0, "Ipv6AddressHelper::Allocate (): "
                 "Interface index not found");

  Ipv6InterfaceAddress ipv6Addr = Ipv6InterfaceAddress (addr, prefix);
  ipv6->SetMetric (ifIndex, 1);
  ipv6->SetUp (ifIndex);
  ipv6->AddAddress (ifIndex, ipv6Addr);

  retval.Add (ipv6, ifIndex);

  return retval;
}

Ipv6InterfaceContainer AssignWithoutAddress(Ptr<NetDevice> device)
{
  Ipv6InterfaceContainer retval;

  Ptr<Node> node = device->GetNode ();
  NS_ASSERT_MSG (node, "Ipv6AddressHelper::Allocate (): Bad node");

  Ptr<Ipv6> ipv6 = node->GetObject<Ipv6> ();
  NS_ASSERT_MSG (ipv6, "Ipv6AddressHelper::Allocate (): Bad ipv6");
  int32_t ifIndex = 0;

  ifIndex = ipv6->GetInterfaceForDevice (device);
  if (ifIndex == -1)
    {
      ifIndex = ipv6->AddInterface (device);
    }
  NS_ASSERT_MSG (ifIndex >= 0, "Ipv6AddressHelper::Allocate (): "
                 "Interface index not found");

  ipv6->SetMetric (ifIndex, 1);
  ipv6->SetUp (ifIndex);

  retval.Add (ipv6, ifIndex);

  return retval;
}
