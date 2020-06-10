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

#ifndef CHORD_NODE_TABLE_H
#define CHORD_NODE_TABLE_H

#include "chord-identifier.h"
#include "chord-node.h"
#include "ns3/object.h"
#include <map>

namespace ns3 {

typedef std::map<ChordIdentifier, Ptr<ChordNode> > ChordNodeMap;
typedef std::map<std::string, Ptr<ChordNode> > ChordNodeNameMap;

/**
 *  \ingroup chordipv4
 *  \class ChordNodeTable
 *  \brief Class to store and operate on ChordNode map
 */

class ChordNodeTable : public Object
{
  public:
    /**
     *  \brief Constructor
     */
    ChordNodeTable ();
    virtual ~ChordNodeTable ();
    virtual void DoDispose ();
    /**
     *  \brief Updates ChordNode in map
     *  \param Ptr to ChordNode
     *
     *  Updates timestamp of already existing ChordNode with same identifier or adds new ChordNode to map
     */
    void UpdateNode (Ptr<ChordNode> &ChordNode);
    /**
     *  \brief Finds ChordNode based on ChordIdentifier
     *  \param chordIdentifier ChordIdentifier of ChordNode to find
     *  \param chordNode Ptr to ChordNode (return result)
     *  \returns true if ChordNode was found in map, otherwise false
     */
    bool FindNode (Ptr<ChordIdentifier> &chordIdentifier, Ptr<ChordNode> &ChordNode);
    /**
     *  \brief Finds ChordNode based on ChordIdentifier
     *  \param name Name of ChordNode to find
     *  \param chordNode Ptr to ChordNode (return result)
     *  \returns true if ChordNode was found in map, otherwise false
     */
    bool FindNode (std::string &name, Ptr<ChordNode> &chordNode);
    /**
     *  \brief Removes ChordNode
     *  \param chordIdentifier Ptr to ChordIdentifier
     */
    void RemoveNode (Ptr<ChordIdentifier> &chordIdentifier);
    /**
     *  \brief Removes ChordNode
     *  \param name Name of ChordNode
     */
    void RemoveNode (std::string &name);
    /**
     *  \brief Finds nearest ChordNode to the given identifier, taken on a circular space
     *  \param targetIdentifier Ptr to target ChordIdentifier
     *  \param chordNode Ptr to ChordNode (return result)
     *  \returns true on success, otherwise false (if no ChordNode in map is routable)
     */
    bool FindNearestNode (Ptr<ChordIdentifier> &targetIdentifier, Ptr<ChordNode> &chordNode);
    /**
     *  \brief Removes all ChordNode's which have not been updated since auditInterval
     *  \param auditInterval audit interval
     */
    void Audit (Time auditInterval);
    /**
     *  \brief Clears ChordNode map
     */
    void Clear();
    /**
     *  \returns Size of map
     */
    uint32_t GetSize ();
    /**
     *  \returns Map of ChordNode(s)
     */
    ChordNodeMap& GetMap();



  private:
    /**
     *  \cond
     */
    ChordNodeMap m_nodeMap;
    ChordNodeNameMap m_nodeNameMap;
    /**
     *  \endcond
     */
  

}; //class ChordNodeTable  



} //namespace ns3



#endif


