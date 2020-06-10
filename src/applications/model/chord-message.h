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

#ifndef CHORD_MESSAGE_H
#define CHORD_MESSAGE_H

#include <vector>
#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/assert.h"
#include "chord-identifier.h"
#include "chord-node.h"

namespace ns3 {

 /**  
 * \ingroup chordipv4
 * \class ChordMessage
 * \brief Chord Protocol Messages
 */
class ChordMessage : public Header
{
  public:
    enum MessageType {
      JOIN_REQ = 1,
      JOIN_RSP = 2,
      STABILIZE_REQ = 3,
      STABILIZE_RSP = 4,
      FINGER_REQ = 5,
      FINGER_RSP = 6,
      HEARTBEAT_REQ = 7,
      HEARTBEAT_RSP = 8,
      LOOKUP_REQ = 9,
      LOOKUP_RSP = 10,
      LEAVE_REQ = 11,
      LEAVE_RSP = 12,
      TRACE_RING = 20,
    };

    ChordMessage ();
    virtual ~ChordMessage ();

    /**
     *  \brief Sets message type
     *  \param messageType message type
     */
    void SetMessageType (MessageType messageType)
    {
      m_messageType = messageType;
    }
    /**
     *  \brief Sets transaction Id
     *  \param transactionId transaction Id of request
     */

    void SetTransactionId (uint32_t transactionId)
    {
      m_transactionId = transactionId;
    }
    /**
     *  \brief Sets TTL
     *  \param ttl time to live before request is dropped
     *  Coming Soon!
     */
    void SetTTL(uint8_t ttl)
    {
      m_ttl = ttl;
    } 
    /**
     *  \returns message type
     */

    MessageType GetMessageType () const
    {
      return m_messageType;
    }

    /**
     *  \returns transaction Id
     */
    uint32_t GetTransactionId (void)
    {
      return m_transactionId;
    }
    
    /**
     *  \brief Sets requestor node in message
     *  \param chordNode Ptr of requesting ChordNode
     */
    void SetRequestorNode (Ptr<ChordNode> chordNode)
    {
      m_chordNode = chordNode;
    }
    /**
     *  \returns Ptr to requestor ChordNode
     */
    Ptr<ChordNode> GetRequestorNode ()
    {
      return m_chordNode;
    }
    /**
     *  \returns ttl of request
     */
    uint8_t GetTTL()
    {
      return m_ttl;
    } 

  private:
    /**
     *  \cond
     */
    MessageType m_messageType;
    uint32_t m_transactionId;
    Ptr<ChordNode> m_chordNode;
    uint8_t m_ttl ;
    /**
     *  \endcond
     */

  public:
    static TypeId GetTypeId (void);
    TypeId GetInstanceTypeId (void) const;
    /**
     *  \brief Prints ChordMessage
     *  \param os Output Stream
     */
    void Print (std::ostream &os) const;
    /**
     *  \returns Size in bytes of packed ChordMessage
     */
    uint32_t GetSerializedSize (void) const;
    /**
     *  \brief Packs ChordMessage
     *  \param start Buffer::Iterator
     *
     *  \verbatim
        Packed Structure:
     
        ChordMessage Header:
     
        0 1 2 3 4 5 6 7 8 
        +-+-+-+-+-+-+-+-+
        |  messageType  |
        +-+-+-+-+-+-+-+-+
        |     ttl       |
        +-+-+-+-+-+-+-+-+
        |               |
        |               |
        | transactionId |
        |               |
        +-+-+-+-+-+-+-+-+
        |               |
        : requestorNode :
        |               |
        +-+-+-+-+-+-+-+-+
        |               | 
        :   Payload     :
        |               |
        +-+-+-+-+-+-+-+-+
     
        JOIN_REQ Payload:
        None
      
        JOIN_RSP Payload:
        0 1 2 3 4 5 6 7 8 
        +-+-+-+-+-+-+-+-+
        |               |
        : successorNode :
        |               |
        +-+-+-+-+-+-+-+-+
      
        LEAVE_REQ Payload:
        0 1 2 3 4 5 6 7 8 
        +-+-+-+-+-+-+-+-+
        |               |
        : successorNode :
        |               |
        +-+-+-+-+-+-+-+-+
        |               |
        :predecessorNode:
        |               |
        +-+-+-+-+-+-+-+-+
     
        LEAVE_RSP Payload:
        0 1 2 3 4 5 6 7 8 
        +-+-+-+-+-+-+-+-+
        |               |
        : successorNode :
        |               |
        +-+-+-+-+-+-+-+-+
        |               |
        :predecessorNode:
        |               |
        +-+-+-+-+-+-+-+-+
      
        STABILIZE_REQ Payload:
        0 1 2 3 4 5 6 7 8 
        +-+-+-+-+-+-+-+-+
        |               |
        : successor-    :
        | Identifier    |
        +-+-+-+-+-+-+-+-+
      
        STABILIZE_RSP Payload:
        0 1 2 3 4 5 6 7 8 
        +-+-+-+-+-+-+-+-+
        |               |
        :predecessorNode:
        |               |
        +-+-+-+-+-+-+-+-+
        |               |
        : successorNode :
        |     List      |
        +-+-+-+-+-+-+-+-+
       
        FINGER_REQ Payload:
        0 1 2 3 4 5 6 7 8 
        +-+-+-+-+-+-+-+-+
        |               |
        : requested-    :
        | Identifier    |
        +-+-+-+-+-+-+-+-+
      
        FINGER_RSP Payload:
        0 1 2 3 4 5 6 7 8 
        +-+-+-+-+-+-+-+-+
        |               |
        : requested-    :
        | Identifier    |
        +-+-+-+-+-+-+-+-+
        |               |
        :  fingerNode   :
        |               |
        +-+-+-+-+-+-+-+-+
      
        HEARTBEAT_REQ Payload:
        0 1 2 3 4 5 6 7 8 
        +-+-+-+-+-+-+-+-+
        |               |
        : predecessor-  :
        | Identifier    |
        +-+-+-+-+-+-+-+-+
      
        HEARTBEAT_RSP Payload:
        0 1 2 3 4 5 6 7 8 
        +-+-+-+-+-+-+-+-+
        |               |
        : successorNode :
        |               |
        +-+-+-+-+-+-+-+-+
        |               |
        :predecessorNode:
        |     List      |
        +-+-+-+-+-+-+-+-+
     
        LOOKUP_REQ Payload:
        0 1 2 3 4 5 6 7 8 
        +-+-+-+-+-+-+-+-+
        |               |
        : requestor-    :
        | Identifier    |
        +-+-+-+-+-+-+-+-+
     
        LOOKUP_RSP Payload:
        0 1 2 3 4 5 6 7 8 
        +-+-+-+-+-+-+-+-+
        |               |
        : resolvedNode  :
        |               |
        +-+-+-+-+-+-+-+-+
     
        TRACE_RING Payload:
        0 1 2 3 4 5 6 7 8 
        +-+-+-+-+-+-+-+-+
        |               |
        : successor-    :
        | Identifier    |
        +-+-+-+-+-+-+-+-+
      
        \endverbatim
     */
    void Serialize (Buffer::Iterator start) const;
    /**
     *  \brief Unpacks ChordMessage
     *  \param start Buffer::Iterator 
     */
    uint32_t Deserialize (Buffer::Iterator start);

   struct JoinReq
    {
      void Print (std::ostream &os) const; 
      uint32_t GetSerializedSize (void) const;
      void Serialize (Buffer::Iterator &start) const;
      uint32_t Deserialize (Buffer::Iterator &start);
    };

    struct JoinRsp
    {
      Ptr<ChordNode> successorNode;
      void Print (std::ostream &os) const; 
      uint32_t GetSerializedSize (void) const;
      void Serialize (Buffer::Iterator &start) const;
      uint32_t Deserialize (Buffer::Iterator &start);
    };

    struct StabilizeReq
    {
      Ptr<ChordIdentifier> successorIdentifier;
      void Print (std::ostream &os) const; 
      uint32_t GetSerializedSize (void) const;
      void Serialize (Buffer::Iterator &start) const;
      uint32_t Deserialize (Buffer::Iterator &start);
    };

    struct StabilizeRsp
    {
      Ptr<ChordNode> predecessorNode;
      uint8_t successorListSize;
      std::vector<Ptr<ChordNode> > successorList;
      void Print (std::ostream &os) const; 
      uint32_t GetSerializedSize (void) const;
      void Serialize (Buffer::Iterator &start) const;
      uint32_t Deserialize (Buffer::Iterator &start);
    };

    struct FingerReq
    {
      Ptr<ChordIdentifier> requestedIdentifier;
      void Print (std::ostream &os) const; 
      uint32_t GetSerializedSize (void) const;
      void Serialize (Buffer::Iterator &start) const;
      uint32_t Deserialize (Buffer::Iterator &start);
    };

    struct FingerRsp
    {
      Ptr<ChordIdentifier> requestedIdentifier;
      Ptr<ChordNode> fingerNode;
      void Print (std::ostream &os) const; 
      uint32_t GetSerializedSize (void) const;
      void Serialize (Buffer::Iterator &start) const;
      uint32_t Deserialize (Buffer::Iterator &start);
    };

    struct HeartbeatReq
    {
      Ptr<ChordIdentifier> predecessorIdentifier;
      void Print (std::ostream &os) const; 
      uint32_t GetSerializedSize (void) const;
      void Serialize (Buffer::Iterator &start) const;
      uint32_t Deserialize (Buffer::Iterator &start);
    };

    struct HeartbeatRsp
    {
      Ptr<ChordNode> successorNode;
      uint8_t predecessorListSize;
      std::vector<Ptr<ChordNode> > predecessorList;
      void Print (std::ostream &os) const; 
      uint32_t GetSerializedSize (void) const;
      void Serialize (Buffer::Iterator &start) const;
      uint32_t Deserialize (Buffer::Iterator &start);
    };

    struct LookupReq
    {
      Ptr<ChordIdentifier> requestedIdentifier;
      void Print (std::ostream &os) const; 
      uint32_t GetSerializedSize (void) const;
      void Serialize (Buffer::Iterator &start) const;
      uint32_t Deserialize (Buffer::Iterator &start);
    };

    struct LookupRsp
    {
      Ptr<ChordNode> resolvedNode;
      void Print (std::ostream &os) const; 
      uint32_t GetSerializedSize (void) const;
      void Serialize (Buffer::Iterator &start) const;
      uint32_t Deserialize (Buffer::Iterator &start);
    };
 
    struct LeaveReq
    {
      Ptr<ChordNode> successorNode;
      Ptr<ChordNode> predecessorNode;
      void Print (std::ostream &os) const; 
      uint32_t GetSerializedSize (void) const;
      void Serialize (Buffer::Iterator &start) const;
      uint32_t Deserialize (Buffer::Iterator &start);
    };

    struct LeaveRsp
    {
      Ptr<ChordNode> successorNode;
      Ptr<ChordNode> predecessorNode;
      void Print (std::ostream &os) const;
      uint32_t GetSerializedSize (void) const;
      void Serialize (Buffer::Iterator &start) const;
      uint32_t Deserialize (Buffer::Iterator &start);
    };
   
    struct TraceRing
    {
      Ptr<ChordIdentifier> successorIdentifier;
      void Print (std::ostream &os) const; 
      uint32_t GetSerializedSize (void) const;
      void Serialize (Buffer::Iterator &start) const;
      uint32_t Deserialize (Buffer::Iterator &start);
    };

  private:
    /**
     *  \cond
     */
    struct
    {
      JoinReq joinReq;
      JoinRsp joinRsp;
      LeaveReq leaveReq;
      LeaveRsp leaveRsp;
      StabilizeReq stabilizeReq;
      StabilizeRsp stabilizeRsp;
      FingerReq fingerReq;
      FingerRsp fingerRsp;
      HeartbeatReq heartbeatReq;
      HeartbeatRsp heartbeatRsp;
      LookupReq lookupReq;
      LookupRsp lookupRsp;
      TraceRing traceRing;
    } m_message;
    /**
     *  \endcond
     */
  public:

    /**
     *  \returns JoinReq structure
     */    
    JoinReq& GetJoinReq ()
    {
      if (m_messageType == 0)
      {
        m_messageType = JOIN_REQ;
      }
      else
      {
        NS_ASSERT (m_messageType == JOIN_REQ);
      }
      return m_message.joinReq;
    }    

    /**
     *  \returns JoinRsp structure
     */    
    JoinRsp& GetJoinRsp ()
    {
      if (m_messageType == 0)
      {
        m_messageType = JOIN_RSP;
      }
      else
      {
        NS_ASSERT (m_messageType == JOIN_RSP);
      }
      return m_message.joinRsp;
    }    

    /**
     *  \returns LeaveReq structure
     */    
    LeaveReq& GetLeaveReq ()
    {
      if (m_messageType == 0)
      {
        m_messageType = LEAVE_REQ;
      }
      else
      {
        NS_ASSERT (m_messageType == LEAVE_REQ);
      }
      return m_message.leaveReq;
    }    

    /**
     *  \returns LeaveRsp structure
     */    
    LeaveRsp& GetLeaveRsp ()
    {
      if (m_messageType == 0)
      {
        m_messageType = LEAVE_RSP;
      }
      else
      {
        NS_ASSERT (m_messageType == LEAVE_RSP);
      }
      return m_message.leaveRsp;
    }    
   

    /**
     *  \returns StabilizeReq structure
     */    
    StabilizeReq& GetStabilizeReq ()
    {
      if (m_messageType == 0)
      {
        m_messageType = STABILIZE_REQ;
      }
      else
      {
        NS_ASSERT (m_messageType == STABILIZE_REQ);
      }
      return m_message.stabilizeReq;
    }

    /**
     *  \returns StabilizeRsp structure
     */    
    StabilizeRsp& GetStabilizeRsp ()
    {
      if (m_messageType == 0)
      {
        m_messageType = STABILIZE_RSP;
      }
      else
      {
        NS_ASSERT (m_messageType == STABILIZE_RSP);
      }
      return m_message.stabilizeRsp;
    }

    /**
     *  \returns FingerReq structure
     */    
    FingerReq& GetFingerReq ()
    {
      if (m_messageType == 0)
      {
        m_messageType = FINGER_REQ;
      }
      else
      {
        NS_ASSERT (m_messageType == FINGER_REQ);
      }
      return m_message.fingerReq;
    }    

    /**
     *  \returns FingerRsp structure
     */    
    FingerRsp& GetFingerRsp ()
    {
      if (m_messageType == 0)
      {
        m_messageType = FINGER_RSP;
      }
      else
      {
        NS_ASSERT (m_messageType == FINGER_RSP);
      }
      return m_message.fingerRsp;
    }

    /**
     *  \returns HeartbeatReq structure
     */    
    HeartbeatReq& GetHeartbeatReq ()
    {
      if (m_messageType == 0)
      {
        m_messageType = HEARTBEAT_REQ;
      }
      else
      {
        NS_ASSERT (m_messageType == HEARTBEAT_REQ);
      }
      return m_message.heartbeatReq;
    }

    /**
     *  \returns HeartbeatRsp structure
     */    
    HeartbeatRsp& GetHeartbeatRsp ()
    {
      if (m_messageType == 0)
      {
        m_messageType = HEARTBEAT_RSP;
      }
      else
      {
        NS_ASSERT (m_messageType == HEARTBEAT_RSP);
      }
      return m_message.heartbeatRsp;
    }

    /**
     *  \returns LookupReq structure
     */    
    LookupReq& GetLookupReq ()
    {
      if (m_messageType == 0)
      {
        m_messageType = LOOKUP_REQ;
      }
      else
      {
        NS_ASSERT (m_messageType == LOOKUP_REQ);
      }
      return m_message.lookupReq;
    }

    /**
     *  \returns LookupRsp structure
     */    
    LookupRsp& GetLookupRsp ()
    {
      if (m_messageType == 0)
      {
        m_messageType = LOOKUP_RSP;
      }
      else
      {
        NS_ASSERT (m_messageType == LOOKUP_RSP);
      }
      return m_message.lookupRsp;
    } 

    /**
     *  \returns TraceRing structure
     */    
    TraceRing& GetTraceRing ()
    {
      if (m_messageType == 0)
      {
        m_messageType = TRACE_RING;
      }
      else
      {
        NS_ASSERT (m_messageType == TRACE_RING);
      }
      return m_message.traceRing;
    }    
  
}; //class ChordMessage

static inline std::ostream& operator<< (std::ostream& os, const ChordMessage & message)
{
  message.Print (os);
  return os;
}


} //namespace ns3

#endif //CHORD_MESSAGE_H
