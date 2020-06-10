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

#include "stdlib.h"
#include "chord-node-table.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ChordNodeTable");

ChordNodeTable::ChordNodeTable ()
{
}

ChordNodeTable::~ChordNodeTable()
{
}

void
ChordNodeTable::DoDispose()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_nodeMap.clear();
  m_nodeNameMap.clear();
}

void
ChordNodeTable::UpdateNode(Ptr<ChordNode> &chordNode)
{
  NS_LOG_FUNCTION_NOARGS ();
  ChordIdentifier chordIdentifier = *(PeekPointer(chordNode->GetChordIdentifier()));
  ChordNodeMap::iterator iterator = m_nodeMap.find (chordIdentifier);
  if (iterator == m_nodeMap.end())
  {
    //add it
    m_nodeMap.insert(std::make_pair(chordIdentifier, chordNode));
    //Timestamp
    chordNode->SetTimestamp(Simulator::Now());
  }
  else
  {
    //Update Time stamp
    iterator->second->SetTimestamp(Simulator::Now());  
  }
  
  //Name map
  if (chordNode->GetName() == "")
    return;

  ChordNodeNameMap::iterator iter = m_nodeNameMap.find (chordNode->GetName());
  if (iter == m_nodeNameMap.end())
  {
    //add it
    m_nodeNameMap.insert(std::make_pair(chordNode->GetName(), chordNode));
    //Timestamp
    chordNode->SetTimestamp(Simulator::Now());
  }
  else
  {
    //Update Time stamp
    iter->second->SetTimestamp(Simulator::Now());  
  }

  return;
}

bool
ChordNodeTable::FindNode (Ptr<ChordIdentifier> &chordId, Ptr<ChordNode> &chordNode)
{
  NS_LOG_FUNCTION_NOARGS ();
  ChordIdentifier chordIdentifier = *(PeekPointer(chordId));
  ChordNodeMap::iterator iterator = m_nodeMap.find (chordIdentifier);
  if (iterator == m_nodeMap.end())
  {
    return false;
  }
  chordNode = (*iterator).second;
  return true;
}

bool
ChordNodeTable::FindNode (std::string &name, Ptr<ChordNode> &chordNode)
{
  NS_LOG_FUNCTION_NOARGS ();
  ChordNodeNameMap::iterator iterator = m_nodeNameMap.find (name);
  if (iterator == m_nodeNameMap.end())
  {
    return false;
  }
  chordNode = (*iterator).second;
  return true;
}

void
ChordNodeTable::RemoveNode (Ptr<ChordIdentifier> &chordId)
{
  NS_LOG_FUNCTION_NOARGS ();
  ChordIdentifier chordIdentifier = *(PeekPointer(chordId));
  ChordNodeMap::iterator iterator = m_nodeMap.find (chordIdentifier);
  if (iterator == m_nodeMap.end())
  {
    return;
  }

  //remove from name map
  if (iterator->second->GetName() != "")
  {
    ChordNodeNameMap::iterator iter = m_nodeNameMap.find (iterator->second->GetName());
    if (iter != m_nodeNameMap.end())
    {
      m_nodeNameMap.erase (iter);
    }
  }

  m_nodeMap.erase (iterator);
}

void
ChordNodeTable::RemoveNode (std::string &name)
{
  NS_LOG_FUNCTION_NOARGS ();
  ChordNodeNameMap::iterator iterator = m_nodeNameMap.find (name);
  if (iterator == m_nodeNameMap.end())
  { 
    return;
  }

  //remove from node table
  ChordNodeMap::iterator iter = m_nodeMap.find (iterator->second->GetChordIdentifier());
  if (iter != m_nodeMap.end())
  {
    m_nodeMap.erase (iter);
  }

  m_nodeNameMap.erase (iterator);
}

/*  Logic: We need to find node whose key is nearest to the requested key. Our aim is to minimize lookup hops.
 *
 *  Step 1: Iterate for all nodes and maximize identifier for nodes which satisfy: node lies inBetween (0,key] <closestNodeOnRight>
 *  Step 2: If none found in step 1, send to node with highest key number <closestNodeOnLeft>
 */

bool
ChordNodeTable::FindNearestNode (Ptr<ChordIdentifier> &targetIdentifier, Ptr<ChordNode> &chordNode)
{
  NS_LOG_FUNCTION_NOARGS ();
  Ptr<ChordNode> closestNodeOnRight = 0;
  Ptr<ChordNode> closestNodeOnLeft = 0;
  Ptr<ChordNode> nearestNode = 0;
  Ptr<ChordIdentifier> zeroIdentifier = Create<ChordIdentifier> ();
  uint8_t* key;
  key = (uint8_t *) malloc(sizeof(uint8_t) * targetIdentifier->GetNumBytes());
  memset (key, 0 , targetIdentifier->GetNumBytes());
  zeroIdentifier->SetKey (key, targetIdentifier->GetNumBytes());
  free (key);

  //Step: 1
  for(ChordNodeMap::iterator nodeIter = m_nodeMap.begin(); nodeIter != m_nodeMap.end(); nodeIter++)
  {
    Ptr<ChordNode> node = (*nodeIter).second;
    if (node->GetRoutable() == false)
      continue;
    if (node->GetChordIdentifier()->IsInBetween(zeroIdentifier, targetIdentifier))
    {
      if (closestNodeOnRight == 0)
      {
        //Initialize
        closestNodeOnRight = node;
      }
      else if (node->GetChordIdentifier()->IsGreater(closestNodeOnRight->GetChordIdentifier()))
      {
        //Maximize
        closestNodeOnRight = node;
      }
    }
    //Store greatest key node as well
    if (closestNodeOnLeft == 0)
    {
      //Initialize
      closestNodeOnLeft = node;
    }
    else if (node->GetChordIdentifier()->IsGreater(closestNodeOnLeft->GetChordIdentifier()))
    {
      //Maximize
      closestNodeOnLeft = node;
    }
  }
  //Step 2:
  if (closestNodeOnRight == 0)
  {
    if (closestNodeOnLeft != 0)
    {
      nearestNode = closestNodeOnLeft;
    }
    else
    {
      return false;
    }
  }
  else if (closestNodeOnRight != 0)
  {
    nearestNode = closestNodeOnRight;
  }
  chordNode = nearestNode;
  return true;
}

uint32_t
ChordNodeTable::GetSize ()
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_nodeMap.size();
}

ChordNodeMap& 
ChordNodeTable::GetMap()
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_nodeMap;
}

void
ChordNodeTable::Clear()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_nodeMap.clear();
  m_nodeNameMap.clear();
}

void
ChordNodeTable::Audit (Time auditInterval)
{
  for (ChordNodeMap::iterator nodeIter = m_nodeMap.begin(); nodeIter != m_nodeMap.end(); )
  {
    Ptr<ChordNode> node = (*nodeIter).second;
    if (node->GetTimestamp().GetMilliSeconds() + auditInterval.GetMilliSeconds() < Simulator::Now().GetMilliSeconds())
    {
      //Remove stale entry
      m_nodeMap.erase (nodeIter++);
    }
    else
      ++nodeIter;
  }
}

} //namespace ns3

