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

#ifndef CHORD_VNODE_H
#define CHORD_VNODE_H

#include <vector>
#include <map>
#include <string.h>
#include "ns3/ptr.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/chord-identifier.h"
#include "ns3/chord-node.h"
#include "ns3/chord-message.h"
#include "ns3/chord-transaction.h"
#include "ns3/chord-node-table.h"

namespace ns3 {

/**  
 *  \ingroup chordipv4
 *  \class ChordVNode
 *  \brief VirtualNode for running Chord protocol. 
 *  It maintains list of succesors, predecessors, states.
 */

class ChordVNode : public ChordNode
{
  public:
    /**
     *  \brief Constructor
     *  \param node ChordNode
     *  \param maxSuccessorListSize
     *  \param maxPredecessorListSize
     */
    ChordVNode (Ptr<ChordNode> node, uint8_t maxSuccessorListSize, uint8_t maxPredecessorListSize);
    virtual ~ChordVNode();
    virtual void DoDispose ();

    //Retrieval.
    /**
     *  \returns List of Successor ChordNode (s)
     */
    std::vector<Ptr<ChordNode> >&  GetSuccessorList(void);
    /**
     *  \returns List of Predecessor ChordNode(s)
     */
    std::vector<Ptr<ChordNode> >&  GetPredecessorList(void);
    /**
     *  \returns Predecessor ChordNode
     */
    Ptr<ChordNode> GetPredecessor(void);
    /**
     *  \returns Successor ChordNode
     */
    Ptr<ChordNode> GetSuccessor(void);
    /**
     *  \returns Name of Virtual Node
     */
    std::string GetVNodeName (void);
    /**
     *  \returns Finger Table(ChordNodeTable)
     */
    ChordNodeTable& GetFingerTable();
    /**
     *  \returns List of Finger identifiers
     */
    std::vector<Ptr<ChordIdentifier> >& GetFingerIdentifierList ();

    //Storage
    /**
     *  \brief Sets new successor 
     *  \param successor ChordNode
     */
    void SetSuccessor (Ptr<ChordNode> successor);
    /**
     *  \brief Sets new predecessor 
     *  \param predecessor ChordNode
     */
    void SetPredecessor (Ptr<ChordNode> predecessor);
    /**
     *  \brief Replaces currently stored successor list
     *  \param successorList 
     */
    void SynchSuccessorList (std::vector<Ptr<ChordNode> > &successorList);
    /**
     *  \brief Replaces currently stored predecessor list
     *  \param predecessorList
     */
    void SynchPredecessorList (std::vector<Ptr<ChordNode> > &predecessorList);

    //Request packing methods for this VNode
    /**
     *  \brief Packs Join Request
     *  \param chordMessage ChordMessage
     */
    void PackJoinReq (ChordMessage &chordMessage);
    /**
     *  \brief Packs Leave Request
     *  \param chordMessage ChordMessage
     */
    void PackLeaveReq (ChordMessage &chordMessage);
    /**
     *  \brief Packs Lookup Request
     *  \param requestedIdentifier ChordIdentifier
     *  \param chordMessage ChordMessage
     */
    void PackLookupReq (Ptr<ChordIdentifier> requestedIdentifier, ChordMessage &chordMessage);
    /**
     *  \brief Packs Stabilize Request
     *  \param chordMessage ChordMessage
     */
    void PackStabilizeReq(ChordMessage &chordMessage);
    /**
     *  \brief Packs Heartbeat Request
     *  \param chordMessage ChordMessage
     */
    void PackHeartbeatReq(ChordMessage &chordMessage);
    /**
     *  \brief Packs Trace Ring request
     *  \param requestorIdentifier ChordIdentifier
     *  \param chordMessage ChordMessage
     */
    void PackTraceRing(Ptr<ChordNode> requestorIdentifier, ChordMessage &chordMessage);
    /**
     *  \brief Packs Finger Request
     *  \param requestedIdentifier ChordIdentifier
     *  \param chordMessage ChordMessage
     */
    void PackFingerReq (Ptr<ChordIdentifier> requestedIdentifier, ChordMessage &chordMessage);

    //Response packing methods for this VNode

    /**
     *  \brief Packs Join Response
     *  \param requestorNode ChordNode
     *  \param transactionId
     *  \param chordMessage ChordMessage
     */
    void PackJoinRsp (Ptr<ChordNode> requestorNode, uint32_t transactionId, ChordMessage &chordMessage);
    /**
     *  \brief Packs Leave Response
     *  \param requestorNode ChordNode
     *  \param successorNode ChordNode
     *  \param predecessorNode ChordNode
     *  \param chordMessage ChordMessage
     */
    void PackLeaveRsp (Ptr<ChordNode> requestorNode, Ptr<ChordNode> successorNode, Ptr<ChordNode> predecessorNode, ChordMessage &chordMessage);
    /**
     *  \brief Packs Lookup Response
     *  \param requestorNode ChordNode
     *  \param transactionId
     *  \param chordMessage ChordMessage
     */
    void PackLookupRsp (Ptr<ChordNode> requestorNode, uint32_t transactionId, ChordMessage &chordMessage);
    /**
     *  \brief Packs Heartbeat Response
     *  \param requestorNode ChordNode
     *  \param chordMessage ChordMessage
     */
    void PackHeartbeatRsp (Ptr<ChordNode> requestorNode, ChordMessage &chordMessage);
    /**
     *  \brief Packs Stabilize Response
     *  \param requestorNode ChordNode
     *  \param chordMessage ChordMessage
     */
    void PackStabilizeRsp (Ptr<ChordNode> requestorNode, ChordMessage &chordMessage);
    /**
     *  \brief Packs Finger Response
     *  \param requestorNode ChordNode
     *  \param requestedIdentifier ChordIdentifier
     *  \param chordMessage ChordMessage
     */
    void PackFingerRsp (Ptr<ChordNode> requestorNode, Ptr<ChordIdentifier> requestedIdentifier, ChordMessage &chordMessage);

    //Processing
    /**
     *  \brief Shift current successor to next one in successor list
     *  \returns false if successor list is empty, otherwise true 
     */
    bool ShiftSuccessor ();
    /**
     *  \brief Shift current predecessor to next one in predecessor list
     *  \returns false if predecessor list is empty, otherwise true 
     *
     */
    bool ShiftPredecessor ();

    //Transactions
    /**
     *  \brief Add ChordTransaction 
     *  \param transactionId
     *  \param chordTransaction Ptr to ChordTransaction
     */
    void AddTransaction (uint32_t transactionId, Ptr<ChordTransaction> chordTransaction);
    /**
     *  \brief Find Transaction
     *  \param transactionId
     *  \param chordTransaction Ptr to ChordTransaction (return result)
     */
    bool FindTransaction (uint32_t transactionId, Ptr<ChordTransaction> &chordTransaction);
    /**
     *  \brief Remove Transaction
     *  \param transactionIf
     */
    void RemoveTransaction (uint32_t transactionId);
    /**
     *  \brief Removes all active transaction
     */
    void RemoveAllTransactions ();
    /**
     *  \brief Generate new transaction Id
     *  \returns transactionId
     */
    uint32_t GetNextTransactionId ();
    
    //Diagnostics 
    /**
     *  \brief Print Successor List
     *  \param os Output stream
     */
    void PrintSuccessorList (std::ostream &os);
    /**
     *  \brief Print Predecessor List
     *  \param os Output stream
     */
    void PrintPredecessorList (std::ostream &os);
    /**
     *  \brief Print Finger Table
     *  \param os Output stream
     */
    void PrintFingerTable (std::ostream &os);
    /**
     *  \brief Print Finger identifier list
     *  \param os Output stream
     */
    void PrintFingerIdentifierList (std::ostream &os);
  
    //Counters
    struct VNodeStats {
    uint32_t fingersLookedUp;
    };
    /**
     *  \returns ChordVNode::VNodeStats
     */
    VNodeStats& GetStats();

  private:
    /**
     *  \cond
     */
    void PopulateFingerIdentifierList ();
    uint32_t m_transactionId;
    std::vector<Ptr<ChordNode> > m_successorList;
    std::vector<Ptr<ChordNode> > m_predecessorList;
    std::vector<Ptr<ChordIdentifier> > m_fingerIdentifierList;
    ChordNodeTable m_fingerTable;
    uint8_t m_maxSuccessorListSize;
    uint8_t m_maxPredecessorListSize;

    Ptr<ChordNode> m_predecessor;
    Ptr<ChordNode> m_successor;

    typedef std::map<uint32_t, Ptr<ChordTransaction> > ChordTransactionMap;

    ChordTransactionMap m_transactionMap;

    VNodeStats m_stats;

    /**
     *  \endcond
     */ 

}; //class ChordVNode

} //namespace ns3

#endif //CHORD_VNODE_H
