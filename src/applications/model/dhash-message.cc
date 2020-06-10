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

#include "dhash-message.h"
#include "dhash-object.h"
#include "ns3/log.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE("DHashMessage");

NS_OBJECT_ENSURE_REGISTERED (DHashMessage);
NS_OBJECT_ENSURE_REGISTERED (DHashHeader);

DHashHeader::DHashHeader ()
{}

DHashHeader::~DHashHeader ()
{}

TypeId
DHashHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DHashHeader")
    .SetParent<Header> ()
    .AddConstructor<DHashHeader> ()
    ;
    return tid;
}

TypeId
DHashHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
DHashHeader::GetSerializedSize (void) const
{
  uint32_t size = sizeof (uint32_t);
  return size;
}

void
DHashHeader::Print (std::ostream &os) const
{
  os << "Length: " << m_length;
}

void
DHashHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteHtonU32 (m_length);
}

uint32_t
DHashHeader::Deserialize (Buffer::Iterator start)
{
  uint32_t size;
  Buffer::Iterator i = start;
  m_length = i.ReadNtohU32 ();
  size = sizeof (uint32_t);
  return size;
}

DHashMessage::DHashMessage ()
{
}

DHashMessage::~DHashMessage ()
{}

TypeId
DHashMessage::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DHashMessage")
    .SetParent<Header> ()
    .AddConstructor<DHashMessage> ()
    ;
  return tid;
}

TypeId
DHashMessage::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
DHashMessage::GetSerializedSize (void) const
{
  uint32_t size = sizeof (uint8_t) + sizeof (uint32_t);
  switch (m_messageType)
  {
    case STORE_REQ:
      size += m_message.storeReq.GetSerializedSize ();
      break;
    case STORE_RSP:
      size += m_message.storeRsp.GetSerializedSize ();
      break;
    case RETRIEVE_REQ:
      size += m_message.retrieveReq.GetSerializedSize ();
      break;
    case RETRIEVE_RSP:
      size += m_message.retrieveRsp.GetSerializedSize ();
      break;
    default:
      NS_ASSERT (false);
  }
  return size;
}

void
DHashMessage::Print (std::ostream &os) const
{
  os << "\n***DHashMessage Dump***\n";
  os << "Header:: \n";
  os << "MessageType: " << m_messageType<<"\n";
  os << "TransactionId: " << m_transactionId<<"\n";
  os << "Payload:: \n";
  switch (m_messageType)
  {
    case STORE_REQ:
      m_message.storeReq.Print (os);
      break;
    case STORE_RSP:
      m_message.storeRsp.Print (os);
      break;
    case RETRIEVE_REQ:
      m_message.retrieveReq.Print (os);
      break;
    case RETRIEVE_RSP:
      m_message.retrieveRsp.Print (os);
      break;
    default:
      break;
  }
  os << "\n***End Of Message***\n";
}

void
DHashMessage::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU8 (m_messageType);
  i.WriteHtonU32 (m_transactionId);

  switch (m_messageType)
  {
    case STORE_REQ:
      m_message.storeReq.Serialize (i);
      break;
    case STORE_RSP:
      m_message.storeRsp.Serialize (i);
      break;
    case RETRIEVE_REQ:
      m_message.retrieveReq.Serialize (i);
      break;
    case RETRIEVE_RSP:
      m_message.retrieveRsp.Serialize (i);
      break;
    default:
      NS_ASSERT (false);
  }
}

uint32_t
DHashMessage::Deserialize (Buffer::Iterator start)
{
  uint32_t size;
  Buffer::Iterator i = start;
  m_messageType = (MessageType) i.ReadU8 ();
  m_transactionId =  i.ReadNtohU32 ();

  size = sizeof (uint8_t) + sizeof (uint32_t);

  switch (m_messageType)
  {
    case STORE_REQ:
      size += m_message.storeReq.Deserialize (i);
      break;
    case STORE_RSP:
      size += m_message.storeRsp.Deserialize (i);
      break;
    case RETRIEVE_REQ:
      size += m_message.retrieveReq.Deserialize (i);
      break;
    case RETRIEVE_RSP:
      size += m_message.retrieveRsp.Deserialize (i);
      break;
    default:
      NS_ASSERT (false);
  }
  return size;
}

/* Message Payloads */


/* STORE_REQ */
uint32_t
DHashMessage::StoreReq::GetSerializedSize (void) const
{
  uint32_t size;
  size = dHashObject->GetSerializedSize();
  return size; 
}

void
DHashMessage::StoreReq::Print (std::ostream &os) const
{
  os << "StoreReq: \n";
  os << "DHash Object Dump: " << dHashObject;
}

void
DHashMessage::StoreReq::Serialize (Buffer::Iterator &start) const
{
  dHashObject->Serialize(start);
}

uint32_t
DHashMessage::StoreReq::Deserialize (Buffer::Iterator &start)
{
  dHashObject = Create<DHashObject> ();
  dHashObject->Deserialize(start);
  return GetSerializedSize();
}

/* STORE_RSP */
uint32_t
DHashMessage::StoreRsp::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint8_t) + objectIdentifier->GetSerializedSize() ;
  return size; 
}

void
DHashMessage::StoreRsp::Print (std::ostream &os) const
{
  os << "StoreRsp: \n";
  os << "Status: \n" << statusTag;
  os << "Object Identifier: " << objectIdentifier;
}

void
DHashMessage::StoreRsp::Serialize (Buffer::Iterator &start) const
{
  start.WriteU8 (statusTag);
  objectIdentifier -> Serialize(start);
}

uint32_t
DHashMessage::StoreRsp::Deserialize (Buffer::Iterator &start)
{
  statusTag = (Status) start.ReadU8();
  objectIdentifier = Create<ChordIdentifier> ();
  objectIdentifier -> Deserialize(start);
  return GetSerializedSize();
}

/* RETRIEVE_REQ */
uint32_t
DHashMessage::RetrieveReq::GetSerializedSize (void) const
{
  uint32_t size;
  size = objectIdentifier->GetSerializedSize();
  return size; 
}

void
DHashMessage::RetrieveReq::Print (std::ostream &os) const
{
  os << "RetrieveReq: \n";
  os << "Object Identifier: " << objectIdentifier;
}

void
DHashMessage::RetrieveReq::Serialize (Buffer::Iterator &start) const
{
  objectIdentifier->Serialize(start);
}

uint32_t
DHashMessage::RetrieveReq::Deserialize (Buffer::Iterator &start)
{
  objectIdentifier = Create<ChordIdentifier> ();
  objectIdentifier->Deserialize(start);
  return GetSerializedSize();
}

/* STORE_RSP */
uint32_t
DHashMessage::RetrieveRsp::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint8_t);
  if (statusTag == DHashMessage::OBJECT_FOUND)
  {
    size = size + dHashObject->GetSerializedSize();
  }
  return size; 
}

void
DHashMessage::RetrieveRsp::Print (std::ostream &os) const
{
  os << "RetrieveRsp: \n";
  os << "Status: \n" << statusTag;
  os << "Object Dump: " << dHashObject;
}

void
DHashMessage::RetrieveRsp::Serialize (Buffer::Iterator &start) const
{
  start.WriteU8 (statusTag);
  if (statusTag == DHashMessage::OBJECT_FOUND)
  {
    dHashObject->Serialize (start);
  }
}

uint32_t
DHashMessage::RetrieveRsp::Deserialize (Buffer::Iterator &start)
{
  statusTag = (Status) start.ReadU8();
  if (statusTag == DHashMessage::OBJECT_FOUND)
  {
    dHashObject = Create<DHashObject> ();
    dHashObject -> Deserialize(start);
  }
  return GetSerializedSize();
}

} //namespace ns3
