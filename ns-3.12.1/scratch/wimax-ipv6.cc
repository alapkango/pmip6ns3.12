/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 *  Copyright (c) 2007,2008, 2009 INRIA, UDcast
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
 * Author: Mohamed Amine Ismail <amine.ismail@sophia.inria.fr>
 *                              <amine.ismail@udcast.com>
 */

//
// Default network topology includes a base station (BS), a subscriber station (SS),
// and a wiredNode
//      +-----+
//      |  SS |
//      +-----+
//    3ffe:100::2
//      -------
//        ((*))
//
//       
//                +------------+                +------------+
//         ((*))==|Base Station|=============== |  wiredNode |
//                +------------+                +------------+
//        3ffe:100::1       3ffe:e::1       3ffe:e::2

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wimax-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include <iostream>
#include <iomanip>

NS_LOG_COMPONENT_DEFINE ("wimaxIpv6Simulation");

using namespace ns3;

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

void PrintRoutingTable (Ptr<Node> n)
{
  Ptr<Ipv6> ipv6 = n->GetObject<Ipv6> ();

  uint32_t nbRoutes = 0;
  Ipv6RoutingTableEntry route;

  Ptr<Ipv6StaticRouting> routing = 0;
  Ipv6StaticRoutingHelper routingHelper;

  routing = routingHelper.GetStaticRouting (ipv6);

  std::cout << "Routing table of " << n << " : " << std::endl;
  std::cout << std::setw(41) << std::left << "Destination" 
      << std::setw(41) << std::left << "Gateway" 
	  << std::setw(5)  << std::left << "If#" 
	  << std::setw(41) << std::left << "Prefix to use" 
	  << std::endl;

  nbRoutes = routing->GetNRoutes ();
  for (uint32_t i = 0 ; i < nbRoutes ; i++)
    {
      route = routing->GetRoute (i);
      std::cout << std::setw(41) << std::left << route.GetDest () << "  "
          << std::setw(41) << std::left << route.GetGateway () << "  "
          << std::setw(5)  << std::left << route.GetInterface () << "  "
          << std::setw(41) << std::left << route.GetPrefixToUse ()
          << std::endl;
    }
}

int main (int argc, char *argv[])
{
  NodeContainer ssNodes;
  NodeContainer bsNodes;
  
  NodeContainer wiredNodes;

  ssNodes.Create (1);
  bsNodes.Create (1);
  wiredNodes.Create (1);

  WimaxHelper wimax;

  NetDeviceContainer ssDevs, bsDevs;
  NetDeviceContainer wiredDevs;
  
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate("100Mbps")));
  csma.SetChannelAttribute ("Delay", TimeValue (Seconds(0.000002)));
  
  NodeContainer wiredNet;
  
  wiredNet.Add(bsNodes);
  wiredNet.Add(wiredNodes);
  
  wiredDevs = csma.Install(wiredNet);
  
  ssDevs = wimax.Install (ssNodes,
                          WimaxHelper::DEVICE_TYPE_SUBSCRIBER_STATION,
                          WimaxHelper::SIMPLE_PHY_TYPE_OFDM,
                          WimaxHelper::SCHED_TYPE_SIMPLE);
						  
  bsDevs = wimax.Install (bsNodes, 
						  WimaxHelper::DEVICE_TYPE_BASE_STATION, 
						  WimaxHelper::SIMPLE_PHY_TYPE_OFDM, 
						  WimaxHelper::SCHED_TYPE_SIMPLE);

  wimax.EnableAscii ("bs-devices", bsDevs);
  wimax.EnableAscii ("ss-devices", ssDevs);

  Ptr<SubscriberStationNetDevice> ss;

  ss = ssDevs.Get (0)->GetObject<SubscriberStationNetDevice> ();
  ss->SetModulationType (WimaxPhy::MODULATION_TYPE_QAM64_34);

  Ptr<BaseStationNetDevice> bs;

  bs = bsDevs.Get (0)->GetObject<BaseStationNetDevice> ();

  InternetStackHelper stack;
  stack.Install (bsNodes);
  stack.Install (ssNodes);
  stack.Install (wiredNodes);
  
  Ipv6InterfaceContainer ssIfs;
  Ipv6InterfaceContainer bsIfs;
  Ipv6InterfaceContainer wiredIfs;
  Ipv6InterfaceContainer iifc;

  iifc = AssignIpv6Address(bsDevs.Get(0), Ipv6Address("3ffe:100::1"), 64);
  bsIfs.Add(iifc);
  bsIfs.SetRouter(0, true);
  
  iifc = AssignIpv6Address(ssDevs.Get(0), Ipv6Address("3ffe:100::2"), 64);
  ssIfs.Add(iifc);
  
  iifc = AssignIpv6Address(wiredDevs.Get(0), Ipv6Address("3ffe:e::1"), 64);
  wiredIfs.Add(iifc);

  iifc = AssignIpv6Address(wiredDevs.Get(1), Ipv6Address("3ffe:e::2"), 64);
  wiredIfs.Add(iifc);  
  
  wiredIfs.SetRouter(0, true);

  
  //wimax.EnableLogComponents ();  // Turn on all wimax logging
  //LogLevel logAll = static_cast<LogLevel>(LOG_LEVEL_ALL|LOG_PREFIX_TIME|LOG_PREFIX_NODE);
  LogLevel logInfo = static_cast<LogLevel>(LOG_LEVEL_INFO|LOG_PREFIX_TIME|LOG_PREFIX_NODE);

  //LogComponentEnable ("Udp6Client", logAll);
  LogComponentEnable ("Udp6Server", logInfo);

  /*------------------------------*/
  /* Create a Udp6 application to send ICMPv6 echo request from node zero to
   */
  Udp6ServerHelper udpServer(6000);

  ApplicationContainer apps = udpServer.Install (ssNodes.Get (0));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (10.0));

  uint32_t packetSize = 1024;
  uint32_t maxPacketCount = 0xffffffff;
  Time interPacketInterval = Seconds(0.0002);
  Udp6ClientHelper udpClient(Ipv6Address("3ffe:100::2"), 6000);
  
  udpClient.SetAttribute ("Interval", TimeValue (interPacketInterval));
  udpClient.SetAttribute ("PacketSize", UintegerValue (packetSize));
  udpClient.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));

  apps = udpClient.Install (wiredNodes.Get (0));
  apps.Start (Seconds (1.5));
  apps.Stop (Seconds (10.0));  

  AsciiTraceHelper ascii;
  wimax.EnableAsciiAll (ascii.CreateFileStream ("wimax-ipv6.tr"));
  csma.EnableAsciiAll (ascii.CreateFileStream ("wimax-ipv6.tr"));

  wimax.EnablePcap ("wimax-ipv6", ssNodes.Get (0)->GetId (), ss->GetIfIndex ());
  wimax.EnablePcap ("wimax-ipv6", bsNodes.Get (0)->GetId (), bs->GetIfIndex ());
  
  csma.EnablePcap("wimax-ipv6", wiredDevs);
  
  // Create one Be Downlink service flow between the ss and the bs
  ServiceFlow * DlServiceFlowBe = new ServiceFlow (ServiceFlow::SF_DIRECTION_DOWN);

  DlServiceFlowBe->SetCsSpecification (ServiceFlow::IPV6);
  DlServiceFlowBe->SetServiceSchedulingType (ServiceFlow::SF_TYPE_BE);
  DlServiceFlowBe->SetMaxSustainedTrafficRate (100000000);
  DlServiceFlowBe->SetMinReservedTrafficRate (100000000);
  DlServiceFlowBe->SetMinTolerableTrafficRate (100000000);
  DlServiceFlowBe->SetMaximumLatency (10);
  DlServiceFlowBe->SetMaxTrafficBurst (1000);
  DlServiceFlowBe->SetTrafficPriority (1);

  ss->AddServiceFlow (DlServiceFlowBe);

  ss = 0;
  bs = 0;

  PrintRoutingTable(bsNodes.Get(0));
  
  NS_LOG_INFO ("Starting simulation.....");
  Simulator::Stop(Seconds(10.1));
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

  return 0;
}
