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

#ifndef DHASH_MESSAGE_H
#define DHASH_MESSAGE_H

#include <vector>
#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/assert.h"
#include "chord-identifier.h"
#include "chord-node.h"
#include "chord-message.h"
#include "dhash-object.h"

namespace ns3 {

/**
 *  \ingroup chordipv4
 *  \class DHashHeader
 *  \brief A small class to pack/unpack length header
 */
class DHashHeader : public Header
{
  public:
    DHashHeader ();
    virtual ~DHashHeader ();

    void SetLength (uint32_t length)
    {
      m_length = length;
    }
    uint32_t GetLength (void) const
    {
      return m_length;
    }

  private:
    uint32_t m_length;

  public:
    static TypeId GetTypeId (void);
    TypeId GetInstanceTypeId (void) const;
    void Print (std::ostream &os) const;
    uint32_t GetSerializedSize (void) const;
    /**
     *  \brief Packs length of DHashMessage
     *  \verbatim
        Packed Structure:

        DHashHeader:
        0 1 2 3 4 5 6 7 8 
        +-+-+-+-+-+-+-+-+
        |               |
        |               |
        |    length     |
        |               |
        +-+-+-+-+-+-+-+-+
        \endverbatim
     *
     */
    void Serialize (Buffer::Iterator start) const;
    uint32_t Deserialize (Buffer::Iterator start);
};    
  

/**
 * \ingroup chordipv4
 * \class DHashMessage 
 * \brief Class to pack/unpack DHash Protocol Messages.
 */
class DHashMessage : public Header
{
  public:
    enum MessageType {
      STORE_REQ = 1,
      STORE_RSP = 2,
      RETRIEVE_REQ = 3,
      RETRIEVE_RSP = 4,    
    };

    enum Status {
      OBJECT_FOUND = 1,
      OBJECT_NOT_FOUND = 2,
      NOT_OWNER = 3,
      STORE_SUCCESS = 4,
      STORE_FAILURE = 5,
    };


    DHashMessage ();
    virtual ~DHashMessage ();
    /**
     *  \brief Sets message type
     *  \param messageType message type
     */

    void SetMessageType (MessageType messageType)
    {
      m_messageType = messageType;
    }
    /**
     *  \returns message type
     */
    MessageType GetMessageType () const
    {
      return m_messageType;
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
     *  \returns transaction Id
     */

    uint32_t GetTransactionId () const
    {
      return m_transactionId;
    }

  private:
    /**
     *  \cond
     */
    MessageType m_messageType;
    uint32_t m_transactionId;
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
     *  \brief Packs DHashMessage
     *  \param start Buffer::Iterator
     *
     *  \verbatim
        Packed Structure:

        DHashMessage:
        0 1 2 3 4 5 6 7 8 
        +-+-+-+-+-+-+-+-+
        |  messageType  |
        +-+-+-+-+-+-+-+-+
        |               |
        |               |
        | transactionId |
        |               |
        +-+-+-+-+-+-+-+-+
        |    Payload    |
        +-+-+-+-+-+-+-+-+
        
        STORE_REQ Payload:
        0 1 2 3 4 5 6 7 8 
        +-+-+-+-+-+-+-+-+
        |               |
        :  dHashObject  :
        |               |
        +-+-+-+-+-+-+-+-+

        STORE_RSP Payload:
        0 1 2 3 4 5 6 7 8 
        +-+-+-+-+-+-+-+-+
        |   statusTag   |
        +-+-+-+-+-+-+-+-+
        |               |
        :   object-     :
        |  Identifier   |
        +-+-+-+-+-+-+-+-+

        RETRIEVE_REQ Payload:
        0 1 2 3 4 5 6 7 8 
        +-+-+-+-+-+-+-+-+
        |               |
        :   object-     :
        |  Identifier   |
        +-+-+-+-+-+-+-+-+

        RETRIEVE_RSP Payload:
        0 1 2 3 4 5 6 7 8 
        +-+-+-+-+-+-+-+-+
        |   statusTag   |
        +-+-+-+-+-+-+-+-+
        |               |
        :  dHashobject  :
        |               |
        +-+-+-+-+-+-+-+-+

        \endverbatim

     */
    void Serialize (Buffer::Iterator start) const;
    /**
     *  \brief Unpacks DHashMessage
     *  \param start Buffer::Iterator 
     */
    uint32_t Deserialize (Buffer::Iterator start);


    struct StoreReq
    {
      Ptr<DHashObject> dHashObject;
      void Print (std::ostream &os) const; 
      uint32_t GetSerializedSize (void) const;
      void Serialize (Buffer::Iterator &start) const;
      uint32_t Deserialize (Buffer::Iterator &start);
    };

   struct StoreRsp
    {
      uint8_t  statusTag;
      Ptr<ChordIdentifier> objectIdentifier;
      void Print (std::ostream &os) const; 
      uint32_t GetSerializedSize (void) const;
      void Serialize (Buffer::Iterator &start) const;
      uint32_t Deserialize (Buffer::Iterator &start);
    };

    struct RetrieveReq
    {
      Ptr<ChordIdentifier> objectIdentifier;
      void Print (std::ostream &os) const; 
      uint32_t GetSerializedSize (void) const;
      void Serialize (Buffer::Iterator &start) const;
      uint32_t Deserialize (Buffer::Iterator &start);
    };

   struct RetrieveRsp
    {
      uint8_t  statusTag;
      Ptr<DHashObject> dHashObject;
      void Print (std::ostream &os) const; 
      uint32_t GetSerializedSize (void) const;
      void Serialize (Buffer::Iterator &start) const;
      uint32_t Deserialize (Buffer::Iterator &start);
    };

   
  private:
    struct
    {
      StoreReq storeReq;
      StoreRsp storeRsp;
      RetrieveReq retrieveReq;
      RetrieveRsp retrieveRsp;
    } m_message;

  public:
    /**
    *  \returns StoreReq structure
    */    
    StoreReq& GetStoreReq ()
    {
      if (m_messageType == 0)
      {
        m_messageType = STORE_REQ;
      }
      else
      {
        NS_ASSERT (m_messageType == STORE_REQ);
      }
      return m_message.storeReq;
    }

    /**
     *  \returns StoreRsp structure
     */    

    StoreRsp& GetStoreRsp ()
    {
      if (m_messageType == 0)
      {
        m_messageType = STORE_RSP;
      }
      else
      {
        NS_ASSERT (m_messageType == STORE_RSP);
      }
      return m_message.storeRsp;
    }  

    /**
     *  \returns RetrieveReq structure
     */    
    RetrieveReq& GetRetrieveReq ()
    {
      if (m_messageType == 0)
      {
        m_messageType = RETRIEVE_REQ;
      }
      else
      {
        NS_ASSERT (m_messageType == RETRIEVE_REQ);
      }
      return m_message.retrieveReq;
    }    
    /**
     *  \returns RetrieveRsp structure
     */    

    RetrieveRsp& GetRetrieveRsp ()
    {
      if (m_messageType == 0)
      {
        m_messageType = RETRIEVE_RSP;
      }
      else
      {
        NS_ASSERT (m_messageType == RETRIEVE_RSP);
      }
      return m_message.retrieveRsp;
    }    

 
}; //class ChordMessage

static inline std::ostream& operator<< (std::ostream& os, const DHashMessage & message)
{
  message.Print (os);
  return os;
}


} //namespace ns3

#endif // DHASH_MESSAGE_H
