/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 University of Pennsylvania
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
 */

//include(s)

#include "chord-ipv4-helper.h"
#include "ns3/chord-ipv4.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/names.h"

namespace ns3 {

ChordIpv4Helper::ChordIpv4Helper (Ipv4Address bootStrapIp, uint16_t bootStrapPort, Ipv4Address localIpAddress,uint16_t listeningPort, uint16_t applicationPort)
{
  m_factory.SetTypeId (ChordIpv4::GetTypeId ());
  SetAttribute ("BootStrapIp", Ipv4AddressValue(bootStrapIp));
  SetAttribute ("BootStrapPort", UintegerValue(bootStrapPort));
  SetAttribute ("LocalIpAddress", Ipv4AddressValue(localIpAddress));
  SetAttribute ("ListeningPort", UintegerValue(listeningPort));
  SetAttribute ("ApplicationPort", UintegerValue(applicationPort));
  SetAttribute ("DHashEnable", BooleanValue(false));
} //Constructor

ChordIpv4Helper::ChordIpv4Helper (Ipv4Address bootStrapIp, uint16_t bootStrapPort, Ipv4Address localIpAddress,uint16_t listeningPort, uint16_t applicationPort, uint16_t dHashPort)
{
  m_factory.SetTypeId (ChordIpv4::GetTypeId ());
  SetAttribute ("BootStrapIp", Ipv4AddressValue(bootStrapIp));
  SetAttribute ("BootStrapPort", UintegerValue(bootStrapPort));
  SetAttribute ("LocalIpAddress", Ipv4AddressValue(localIpAddress));
  SetAttribute ("ListeningPort", UintegerValue(listeningPort));
  SetAttribute ("ApplicationPort", UintegerValue(applicationPort));
  SetAttribute ("DHashPort", UintegerValue(dHashPort));
  SetAttribute ("DHashEnable", BooleanValue(true));
} //Constructor 

void 
ChordIpv4Helper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
ChordIpv4Helper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
ChordIpv4Helper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
ChordIpv4Helper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
ChordIpv4Helper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<ChordIpv4> ();
  node->AddApplication (app);
  
  return app;
}

/* TODO: Add interface functions to chord running on a node  */

} //namespace ns3
