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

#include "chord-vnode.h"
#include "chord-node.h"
#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/uinteger.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ChordVNode");

ChordVNode::ChordVNode (Ptr<ChordNode> node, uint8_t maxSuccessorListSize, uint8_t maxPredecessorListSize) : ChordNode (node)
{
  NS_LOG_FUNCTION_NOARGS();
  m_successor = 0;
  m_predecessor = 0;
  m_maxSuccessorListSize = maxSuccessorListSize;
  m_maxPredecessorListSize = maxPredecessorListSize;
  PopulateFingerIdentifierList ();
}

ChordVNode::~ChordVNode ()
{
  NS_LOG_FUNCTION_NOARGS();
}

void
ChordVNode::DoDispose (void)
{
  m_successorList.clear();
  m_predecessorList.clear();
  RemoveAllTransactions ();
}

std::vector<Ptr<ChordNode> >& 
ChordVNode::GetSuccessorList (void)
{
  NS_LOG_FUNCTION_NOARGS();
  return m_successorList;
}

std::vector<Ptr<ChordNode> >& 
ChordVNode::GetPredecessorList (void)
{
  NS_LOG_FUNCTION_NOARGS();
  return m_predecessorList;
}

std::string
ChordVNode::GetVNodeName ()
{
  NS_LOG_FUNCTION_NOARGS ();
  return GetName();
    
}

Ptr<ChordNode>
ChordVNode::GetPredecessor ()
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_predecessorList.front();
}

Ptr<ChordNode>
ChordVNode::GetSuccessor ()
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_successorList.front();
}

ChordNodeTable&
ChordVNode::GetFingerTable ()
{
  return m_fingerTable;
}

void
ChordVNode::SetSuccessor (Ptr<ChordNode> successor)
{
  if (m_successorList.size() != 0)
  {
    m_successorList.erase (m_successorList.begin());
  }
  //Insert new successor
  m_successorList.insert (m_successorList.begin(), successor);
  m_successor = successor;
  //Set timestamp
  m_successor -> SetTimestamp (Simulator::Now());
}

void
ChordVNode::SetPredecessor (Ptr<ChordNode> predecessor)
{
  if (m_predecessorList.size() != 0)
  {
    m_predecessorList.erase (m_predecessorList.begin());
  }
  //Insert new predecessor
  m_predecessorList.insert (m_predecessorList.begin(), predecessor);
  m_predecessor = predecessor;
  //Set timestamp
  m_predecessor -> SetTimestamp (Simulator::Now());
}

void
ChordVNode::AddTransaction (uint32_t transactionId, Ptr<ChordTransaction> chordTransaction)
{
  ChordTransactionMap::iterator iterator = m_transactionMap.find (transactionId);
  if (iterator == m_transactionMap.end())
  {
    //add it
    m_transactionMap.insert(std::make_pair(transactionId, chordTransaction));
    return;
  }
  NS_ASSERT (iterator != m_transactionMap.end());
  return;
}

bool
ChordVNode::FindTransaction (uint32_t transactionId, Ptr<ChordTransaction> &chordTransaction)
{
  ChordTransactionMap::iterator iterator =  m_transactionMap.find (transactionId);
  if (iterator == m_transactionMap.end ())
  {
    //return failure
    return false; 
  }
  //return success and pointer to transaction
  chordTransaction = (*iterator).second;
  return true;

}

void
ChordVNode::RemoveTransaction (uint32_t transactionId)
{
  ChordTransactionMap::iterator iterator = m_transactionMap.find (transactionId);
  if (iterator == m_transactionMap.end())
  {
    return;
  }
  //remove it
  Ptr<ChordTransaction> chordTransaction = iterator->second;
  chordTransaction -> DoDispose();
  m_transactionMap.erase (iterator);
}

void
ChordVNode::RemoveAllTransactions ()
{
  for (ChordTransactionMap::iterator iterator = m_transactionMap.begin(); iterator != m_transactionMap.end(); iterator++ )
  {
    iterator->second->DoDispose();
    m_transactionMap.erase (iterator);
  }
  m_transactionMap.clear();
}

uint32_t
ChordVNode::GetNextTransactionId ()
{
  return m_transactionId++;
}

std::vector<Ptr<ChordIdentifier> >&
ChordVNode::GetFingerIdentifierList ()
{
  return m_fingerIdentifierList;
}

void
ChordVNode::PopulateFingerIdentifierList ()
{
  for (uint16_t i = 0; i<(GetChordIdentifier()->GetNumBytes()*8); i++)
  {
    //Make copy
    Ptr<ChordIdentifier> fingerIdentifier = Create<ChordIdentifier> (GetChordIdentifier());
    //Add power of two
    fingerIdentifier->AddPowerOfTwo(i);
    m_fingerIdentifierList.push_back(fingerIdentifier);
  }
}

void
ChordVNode::PackJoinReq (ChordMessage &chordMessage)
{
  NS_LOG_FUNCTION_NOARGS ();
  chordMessage.SetMessageType (ChordMessage::JOIN_REQ);
  chordMessage.SetRequestorNode (this);
  chordMessage.SetTransactionId (GetNextTransactionId());
}

void
ChordVNode::PackJoinRsp(Ptr<ChordNode> requestorNode, uint32_t transactionId, ChordMessage &chordMessage)
{
  NS_LOG_FUNCTION_NOARGS ();
  chordMessage.SetMessageType (ChordMessage::JOIN_RSP);
  chordMessage.SetRequestorNode (requestorNode);
  chordMessage.SetTransactionId (transactionId);
  chordMessage.GetJoinRsp().successorNode = this;
}

void
ChordVNode::PackLeaveReq (ChordMessage &chordMessage)
{
  NS_LOG_FUNCTION_NOARGS ();
  chordMessage.SetMessageType (ChordMessage::LEAVE_REQ);
  chordMessage.SetRequestorNode (this);
  chordMessage.GetLeaveReq().successorNode = m_successor;
  chordMessage.GetLeaveReq().predecessorNode = m_predecessor;
}

void
ChordVNode::PackLeaveRsp (Ptr<ChordNode> requestorNode, Ptr<ChordNode> successorNode, Ptr<ChordNode> predecessorNode, ChordMessage &chordMessage)
{
  NS_LOG_FUNCTION_NOARGS ();
  chordMessage.SetMessageType (ChordMessage::LEAVE_RSP);
  chordMessage.SetRequestorNode (requestorNode);
  chordMessage.GetLeaveRsp().successorNode = successorNode;
  chordMessage.GetLeaveRsp().predecessorNode = predecessorNode;
}

void
ChordVNode::PackLookupReq(Ptr<ChordIdentifier> requestedIdentifier, ChordMessage &chordMessage)
{
  NS_LOG_FUNCTION_NOARGS ();
  chordMessage.SetMessageType (ChordMessage::LOOKUP_REQ);
  chordMessage.SetRequestorNode (this);
  chordMessage.GetLookupReq().requestedIdentifier = requestedIdentifier;
  chordMessage.SetTransactionId (GetNextTransactionId());
}

void
ChordVNode::PackLookupRsp(Ptr<ChordNode> requestorNode, uint32_t transactionId, ChordMessage &chordMessage)
{
  NS_LOG_FUNCTION_NOARGS ();
  chordMessage.SetMessageType (ChordMessage::LOOKUP_RSP);
  chordMessage.SetRequestorNode (requestorNode);
  chordMessage.SetTransactionId (transactionId);
  chordMessage.GetLookupRsp().resolvedNode = this;
}

void
ChordVNode::PackStabilizeReq(ChordMessage &chordMessage)
{
  NS_LOG_FUNCTION_NOARGS ();
  chordMessage.SetMessageType (ChordMessage::STABILIZE_REQ);
  chordMessage.SetRequestorNode (this);
  chordMessage.GetStabilizeReq().successorIdentifier = m_successor->GetChordIdentifier();
}

void 
ChordVNode::PackStabilizeRsp(Ptr<ChordNode> requestorNode, ChordMessage &chordMessage)
{
  NS_LOG_FUNCTION_NOARGS ();
  chordMessage.SetMessageType (ChordMessage::STABILIZE_RSP);
  chordMessage.SetRequestorNode (requestorNode);
  chordMessage.GetStabilizeRsp().predecessorNode = m_predecessor;
  //Pack successor list
  chordMessage.GetStabilizeRsp().successorListSize = m_successorList.size();
  for (std::vector<Ptr<ChordNode> >::const_iterator nodeIter = m_successorList.begin(); nodeIter != m_successorList.end(); nodeIter++)
  {
    Ptr<ChordNode> node = *nodeIter;
    chordMessage.GetStabilizeRsp().successorList.push_back (node);
  }
}

void
ChordVNode::PackHeartbeatReq(ChordMessage &chordMessage)
{
  NS_LOG_FUNCTION_NOARGS ();
  chordMessage.SetMessageType (ChordMessage::HEARTBEAT_REQ);
  chordMessage.SetRequestorNode (this);
  chordMessage.GetHeartbeatReq().predecessorIdentifier = m_predecessor->GetChordIdentifier();
}

void
ChordVNode::PackFingerReq (Ptr<ChordIdentifier> requestedIdentifier, ChordMessage &chordMessage)
{
  NS_LOG_FUNCTION_NOARGS ();
  chordMessage.SetMessageType (ChordMessage::FINGER_REQ);
  chordMessage.SetRequestorNode (this);
  chordMessage.GetFingerReq().requestedIdentifier = requestedIdentifier;
}

void
ChordVNode::PackFingerRsp (Ptr<ChordNode> requestorNode, Ptr<ChordIdentifier> requestedIdentifier, ChordMessage &chordMessage)
{
  NS_LOG_FUNCTION_NOARGS ();
  chordMessage.SetMessageType (ChordMessage::FINGER_RSP);
  chordMessage.SetRequestorNode (requestorNode);
  chordMessage.GetFingerRsp().requestedIdentifier = requestedIdentifier;
  chordMessage.GetFingerRsp().fingerNode = this;
}

void 
ChordVNode::PackHeartbeatRsp(Ptr<ChordNode> requestorNode, ChordMessage &chordMessage)
{
  NS_LOG_FUNCTION_NOARGS ();
  chordMessage.SetMessageType (ChordMessage::HEARTBEAT_RSP);
  chordMessage.SetRequestorNode (requestorNode);
  chordMessage.GetHeartbeatRsp().successorNode = m_successor;
  //Pack predecessor list
  chordMessage.GetHeartbeatRsp().predecessorListSize = m_predecessorList.size();
  //Pack subsequent list
  for (std::vector<Ptr<ChordNode> >::const_iterator nodeIter = m_predecessorList.begin(); nodeIter != m_predecessorList.end(); nodeIter++)
  {
    Ptr<ChordNode> node = *nodeIter;
    chordMessage.GetHeartbeatRsp().predecessorList.push_back (node);
  }
}

void
ChordVNode::PackTraceRing(Ptr<ChordNode> requestorNode, ChordMessage &chordMessage)
{
  NS_LOG_FUNCTION_NOARGS ();
  chordMessage.SetMessageType (ChordMessage::TRACE_RING);
  chordMessage.SetRequestorNode (requestorNode);
  chordMessage.GetTraceRing().successorIdentifier = m_successor->GetChordIdentifier();
}

bool
ChordVNode::ShiftSuccessor ()
{
  if (m_successorList.size() > 1)
  {
    //Remove current successor
    m_successorList.erase (m_successorList.begin());
    //Set new successor
    SetSuccessor(m_successorList.front());
    return true;
  }
  else
    return false;
}

bool
ChordVNode::ShiftPredecessor ()
{
  if (m_predecessorList.size() > 1)
  {
    //Remove current predecessor
    m_predecessorList.erase (m_predecessorList.begin());
    //Set new predecessor
    SetPredecessor(m_predecessorList.front());
    return true;
  }
  else
    return false;
}

void
ChordVNode::SynchSuccessorList (std::vector<Ptr<ChordNode> > &successorList)
{
  if (m_successorList.size() > 1)
  {
    if (successorList.size() > 1)
    {
      //clear list
      m_successorList.erase(m_successorList.begin()+1, m_successorList.end());
    }
    else
    {
      if (!successorList.front()-> GetChordIdentifier()->IsEqual(GetChordIdentifier()))
      {
        //Put this element in 2nd position and return
        m_successorList.erase(m_successorList.begin()+1);
        m_successorList.insert(m_successorList.begin()+1, successorList.front());
        //set timestamp
        //successorList.front()->SetTimestamp(Simulator::Now());
      }
      return;
    }
  }
 
  uint8_t count = 0;
  for (std::vector<Ptr<ChordNode> >::iterator nodeIter = successorList.begin(); nodeIter != successorList.end(); nodeIter++)
  {
    Ptr<ChordNode> node = *nodeIter;
    //Check wrap around
    if (node -> GetChordIdentifier()->IsEqual(GetChordIdentifier()))
    {
      //Wrap around has occurred
      break;
    }
    m_successorList.push_back (node);
    //Set Timestamp
    //node->SetTimestamp(Simulator::Now());
    count++;
    if (count > m_maxSuccessorListSize-1)
    {
      break;
    }
  }
}

void
ChordVNode::SynchPredecessorList (std::vector<Ptr<ChordNode> > &predecessorList)
{
  if (m_predecessorList.size() > 1)
  {
    if (predecessorList.size() > 1)
    {
      //clear list
      m_predecessorList.erase(m_predecessorList.begin()+1, m_predecessorList.end());
    }
    else
    {
      if (!predecessorList.front()-> GetChordIdentifier()->IsEqual(GetChordIdentifier()))
      {
        //Put this element in 2nd position and return
        m_predecessorList.erase(m_predecessorList.begin()+1);
        m_predecessorList.insert(m_predecessorList.begin()+1, predecessorList.front());
        //Set Timestamp
        //m_predecessorList.front()->SetTimestamp(Simulator::Now());
      }
      return;
    }
  }
 
  uint8_t count = 0;
  for (std::vector<Ptr<ChordNode> >::iterator nodeIter = predecessorList.begin(); nodeIter != predecessorList.end(); nodeIter++)
  {
    Ptr<ChordNode> node = *nodeIter;
    //Check wrap around
    if (node -> GetChordIdentifier()->IsEqual(GetChordIdentifier()))
    {
      //Wrap around has occurred
      break;
    }
    m_predecessorList.push_back (node);
    //Set timestamp
    //node->SetTimestamp(Simulator::Now());
    count++;
    if (count > m_maxPredecessorListSize-1)
    {
      break;
    }
  }
}

ChordVNode::VNodeStats&
ChordVNode::GetStats ()
{
  return m_stats;
}

void
ChordVNode::PrintSuccessorList (std::ostream &os)
{
  os << "SuccessorList Size: " << m_successorList.size() << " Dumping SuccessorList:: \n";
  for (std::vector<Ptr<ChordNode> >::iterator nodeIter = m_successorList.begin(); nodeIter != m_successorList.end(); nodeIter++)
  {
    Ptr<ChordNode> node = *nodeIter;
    os << "Successor Ip: " << node->GetIpAddress() << "\n";
    os << "Successor Port: " << node->GetPort() << "\n";
    os << "Successor Identifier: " << node->GetChordIdentifier() << "\n";
    os << "****" << "\n";
  }
}
void
ChordVNode::PrintPredecessorList (std::ostream &os)
{
  os << "PredecessorList Size: " << m_predecessorList.size() << " Dumping PredecessorList:: \n";
  for (std::vector<Ptr<ChordNode> >::iterator nodeIter = m_predecessorList.begin(); nodeIter != m_predecessorList.end(); nodeIter++)
  {
    Ptr<ChordNode> node = *nodeIter;
    os << "Predecessor Ip: " << node->GetIpAddress() << "\n";
    os << "Predecessor Port: " << node->GetPort() << "\n";
    os << "Predecessor Identifier: " << node->GetChordIdentifier() << "\n";
    os << "****" << "\n";
  }
}

void
ChordVNode::PrintFingerTable (std::ostream &os)
{
  os << "FingerTable Size: " << m_fingerTable.GetSize() << " Dumping Finger Table:: \n";
  for (ChordNodeMap::iterator nodeIter = m_fingerTable.GetMap().begin(); nodeIter != m_fingerTable.GetMap().end(); nodeIter++)
  {
    Ptr<ChordNode> node = (*nodeIter).second;
    os << "Finger Ip: " << node->GetIpAddress() << "\n";
    os << "Finger Port: " << node->GetPort() << "\n";
    os << "Finger Identifier: " << node->GetChordIdentifier() << "\n";
    os << "****" << "\n";
  }
}

void
ChordVNode::PrintFingerIdentifierList (std::ostream &os)
{
  os << "Lookup was made for: " << m_fingerIdentifierList.size() << " fingers\n";
  for (std::vector<Ptr<ChordIdentifier> >::iterator fingerIter = m_fingerIdentifierList.begin(); fingerIter != m_fingerIdentifierList.end(); fingerIter++)
  {
    os << (*fingerIter);
  }
}

} //namespace ns3
