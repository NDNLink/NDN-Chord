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

#include "chord-node.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ChordNode");  

ChordNode::ChordNode (Ptr<ChordIdentifier> identifier, Ipv4Address address, uint16_t port, uint16_t applicationPort, uint16_t dHashPort)
{
  NS_LOG_FUNCTION_NOARGS();
  m_identifier = Create<ChordIdentifier> (identifier);
  m_address = address;
  m_port = port;
  m_applicationPort = applicationPort;
  m_dHashPort = dHashPort;
  m_name = "";
  m_routable = true;
}
ChordNode::ChordNode (Ptr<ChordIdentifier> identifier, std::string name, Ipv4Address address, uint16_t port, uint16_t applicationPort, uint16_t dHashPort)
{
  NS_LOG_FUNCTION_NOARGS();
  m_identifier = Create<ChordIdentifier>(identifier);
  m_name = name;
  m_address = address;
  m_port = port;
  m_applicationPort = applicationPort;
  m_dHashPort = dHashPort;
  m_routable = true;
}

ChordNode::ChordNode (const Ptr<ChordNode> chordNode)
{
  NS_LOG_FUNCTION_NOARGS();
  m_identifier = Create<ChordIdentifier>(chordNode->GetChordIdentifier());
  m_name = chordNode->GetName();
  m_address = chordNode->GetIpAddress();
  m_port = chordNode->GetPort();
  m_applicationPort = chordNode->GetApplicationPort();
  m_dHashPort = chordNode->GetDHashPort();
  m_routable = true;
}

ChordNode::ChordNode ()
{
  NS_LOG_FUNCTION_NOARGS();
  m_name = "";
}

ChordNode::~ChordNode ()
{
  NS_LOG_FUNCTION_NOARGS();
}

void
ChordNode::DoDispose (void)
{
  NS_LOG_FUNCTION_NOARGS();
  m_identifier->DoDispose();
}

void
ChordNode::SetTimestamp (Time timestamp)
{
  NS_LOG_FUNCTION_NOARGS();
  m_timestamp = timestamp;
}

void
ChordNode::SetRoutable (bool routable)
{
  m_routable = routable;
} 

bool
ChordNode::GetRoutable ()
{
  return m_routable;
}

Ipv4Address
ChordNode::GetIpAddress (void)
{
  NS_LOG_FUNCTION_NOARGS();
  return m_address;
}

uint16_t
ChordNode::GetPort (void)
{
  NS_LOG_FUNCTION_NOARGS();
  return m_port;
}

uint16_t
ChordNode::GetApplicationPort ()
{
  NS_LOG_FUNCTION_NOARGS();
  return m_applicationPort;
}

uint16_t
ChordNode::GetDHashPort ()
{
  NS_LOG_FUNCTION_NOARGS();
  return m_dHashPort;
}

Ptr<ChordIdentifier>
ChordNode::GetChordIdentifier (void)
{
  NS_LOG_FUNCTION_NOARGS();
  return m_identifier;
}

std::string
ChordNode::GetName ()
{
  return m_name;
}

Time
ChordNode::GetTimestamp (void)
{
  NS_LOG_FUNCTION_NOARGS();
  return m_timestamp;
}

//Serialization
uint32_t
ChordNode::GetSerializedSize ()
{
  uint32_t size;
  size = m_identifier->GetSerializedSize() + IPV4_ADDRESS_SIZE + sizeof(uint16_t) + sizeof (uint16_t) + sizeof (uint16_t);
  return size;
}

void
ChordNode::Serialize (Buffer::Iterator &start)
{
  m_identifier->Serialize (start);
  start.WriteHtonU32 (m_address.Get());
  start.WriteHtonU16 (m_port);
  start.WriteHtonU16 (m_applicationPort);
  start.WriteHtonU16 (m_dHashPort);
}

void
ChordNode::Deserialize (Buffer::Iterator &start)
{
  m_identifier = Create<ChordIdentifier> ();
  m_identifier->Deserialize (start);
  m_address = Ipv4Address (start.ReadNtohU32());
  m_port = start.ReadNtohU16 ();
  m_applicationPort = start.ReadNtohU16 ();
  m_dHashPort = start.ReadNtohU16 ();
}

void
ChordNode::Print (std::ostream &os)
{
  if (m_name != "")
    os << "Name: " << m_name << "\n";
  os << "Identifier: " << m_identifier << "\n";
  os << "IpAddress: " << m_address << "\n";
  os << "Port: " << m_port << "\n";
  os << "ApplicationPort: " << m_applicationPort << "\n";
  os << "DHashPort: " << m_dHashPort << "\n";
}

} //namespace ns3
