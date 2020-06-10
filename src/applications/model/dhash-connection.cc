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

#include "dhash-connection.h"
#include "ns3/log.h"

namespace ns3 {
NS_LOG_COMPONENT_DEFINE ("DHashConnection");

DHashConnection::DHashConnection (Ipv4Address ipAddress, uint16_t port, Ptr<Socket> socket)
{
  m_ipAddress = ipAddress;
  m_port = port;
  m_socket = socket;
  m_txState = TX_IDLE;
  m_rxState = RX_IDLE;
  m_totalTxBytes = 0;
  m_currentTxBytes = 0;
  m_lastActivityTime = Simulator::Now();
}

DHashConnection::DHashConnection ()
{ 
  m_ipAddress = 0;
  m_port = 0;
  m_socket = 0;
  m_txState = TX_IDLE;
  m_rxState = RX_IDLE;
  m_totalTxBytes = 0;
  m_currentTxBytes = 0;
  m_lastActivityTime = Simulator::Now();
}
DHashConnection::DHashConnection (const DHashConnection &connection)
{
  Ptr<DHashConnection> dHashConnection = const_cast<DHashConnection *>(&connection);
  m_ipAddress = dHashConnection->GetIpAddress();
  m_port = dHashConnection->GetPort();
  m_socket = dHashConnection->GetSocket();
  m_txState = TX_IDLE;
  m_rxState = RX_IDLE;
  m_totalTxBytes = 0;
  m_currentTxBytes = 0;
  m_lastActivityTime = Simulator::Now();
}


DHashConnection::~DHashConnection ()
{
  if (m_socket != 0)
  {
    m_socket->Close();
    m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket> > ());
    m_socket = 0;
  }
  m_ipAddress = 0;
  m_port = 0;
  m_txState = TX_IDLE;
  m_rxState = RX_IDLE;
  m_totalTxBytes = 0;
  m_currentTxBytes = 0;
  m_txPacketList.clear();
}

void
DHashConnection::DoDispose ()
{
  if (m_socket != 0)
  {
    m_socket->Close();
    m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket> > ());
    m_socket = 0;
  }
  m_ipAddress = 0;
  m_port = 0;
  m_txState = TX_IDLE;
  m_rxState = RX_IDLE;
  m_totalTxBytes = 0;
  m_currentTxBytes = 0;
  m_txPacketList.clear();
}

Ipv4Address
DHashConnection::GetIpAddress()
{
  return m_ipAddress;
}

uint16_t
DHashConnection::GetPort()
{
  return m_port;
}

Ptr<Socket>
DHashConnection::GetSocket()
{
  return m_socket;
}

void 
DHashConnection::SendTCPData (Ptr<Packet> packet)
{
  //Add packet to pending tx list
  m_txPacketList.push_back (packet);
  //Set state to transmitting
  if (m_txState == TX_IDLE)
  {
    //Start transmitting
    m_txState = TRANSMITTING;
    m_socket -> SetSendCallback (MakeCallback(&DHashConnection::WriteTCPBuffer, this));
    //Initiate transmission
    WriteTCPBuffer (m_socket, m_socket->GetTxAvailable ());
  }
  return;
}

void
DHashConnection::WriteTCPBuffer (Ptr<Socket> socket, uint32_t txSpace)
{
  m_lastActivityTime = Simulator::Now();
  if (m_totalTxBytes == 0)
  {
    //Start new packet Tx
    m_currentTxBytes = 0; 
    if (m_txPacketList.size() == 0)
    {
      m_txState = TX_IDLE;
      return;
    }
    m_currentTxPacket = *(m_txPacketList.begin());
    m_totalTxBytes = m_currentTxPacket->GetSize();
    DHashHeader dHashHeader;
    dHashHeader.SetLength (m_totalTxBytes);
    m_currentTxPacket->AddHeader(dHashHeader);
    m_totalTxBytes = m_currentTxPacket->GetSize();
  }

  //Transmit m_currentTxPacket
  uint32_t availTxBytes = socket->GetTxAvailable ();
  // 2 Things: Either full packet fits or we have to fragment
  if ((m_totalTxBytes-m_currentTxBytes) <= availTxBytes)
  {
    //Send entire packet at once and remove current packet from queue
    socket->Send (m_currentTxPacket, 0);
    m_txPacketList.erase (m_txPacketList.begin());
    m_totalTxBytes = 0;
    m_currentTxBytes = 0;    
    return;
  }
  else
  {
    //Fragment and send packet
    socket->Send (m_currentTxPacket->CreateFragment(m_currentTxBytes, availTxBytes));
    m_currentTxBytes = m_currentTxBytes + availTxBytes;
  }
}

void
DHashConnection::ReadTCPBuffer (Ptr<Socket> socket)
{
  m_lastActivityTime = Simulator::Now();
  uint32_t availRxBytes = socket->GetRxAvailable();
  Ptr<Packet> packet = socket->Recv(availRxBytes, 0);

  while (availRxBytes > 0)
  {
    Ptr<Packet> messagePacket = AssembleMessage(packet, availRxBytes);
    if (messagePacket != NULL)
    {
      m_recvFn (messagePacket, this);
    }
  }

}

Ptr<Packet>
DHashConnection::AssembleMessage (Ptr<Packet>& packet, uint32_t &availRxBytes)
{
  if (m_rxState == RX_IDLE)
  { 
    //Receive new packet
    DHashHeader dHashHeader = DHashHeader ();
    packet->RemoveHeader(dHashHeader);
    availRxBytes = availRxBytes - dHashHeader.GetSerializedSize();
    m_totalRxBytes = dHashHeader.GetLength();
    m_currentRxBytes = 0;
    m_currentRxPacket = Create<Packet> ();
    m_rxState = RECEIVING;
  }
  if ((m_totalRxBytes-m_currentRxBytes) <=  packet->GetSize())
  {  
    //Deliver message
    m_currentRxPacket->AddAtEnd(packet->CreateFragment(0, m_totalRxBytes-m_currentRxBytes));
    //remove bytes
    packet->RemoveAtStart (m_totalRxBytes-m_currentRxBytes);
    availRxBytes = availRxBytes - (m_totalRxBytes-m_currentRxBytes);
    m_rxState = RX_IDLE;
    return m_currentRxPacket; 
  }
  else
  {
    //concat received packet
    m_currentRxPacket->AddAtEnd (packet);
    availRxBytes = 0;
    return NULL;
  }
}

void
DHashConnection::SetRecvCallback (Callback<void, Ptr<Packet>, Ptr<DHashConnection> > recvFn)
{
  m_recvFn = recvFn;
}

Time
DHashConnection::GetLastActivityTime ()
{
  return m_lastActivityTime;
}


//Needed for std::map
bool
operator < (const DHashConnection &connectionL, const DHashConnection &connectionR)
{
  Ptr<DHashConnection> connL = const_cast<DHashConnection *>(&connectionL);
  Ptr<DHashConnection> connR = const_cast<DHashConnection *>(&connectionR);
  if (connL->GetIpAddress() < connL->GetIpAddress())
    return true;
  else if (connL->GetIpAddress() == connL->GetIpAddress())
  {
    if (connL->GetPort() < connR->GetPort())
      return true;
    else
      return false;
  }
  else
    return false;
} 

bool
operator == (const DHashConnection &connectionL, const DHashConnection &connectionR)
{
  Ptr<DHashConnection> connL = const_cast<DHashConnection *>(&connectionL);
  Ptr<DHashConnection> connR = const_cast<DHashConnection *>(&connectionR);
  if ((connL->GetIpAddress() == connL->GetIpAddress()) && (connL->GetPort() == connR->GetPort()))
    return true;
  else
    return false;
}

}
