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


#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"
#include "ns3/timer.h"
#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "chord-identifier.h"
#include "dhash-ipv4.h"
#include "dhash-message.h"
#include "chord-ipv4.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DHashIpv4");
NS_OBJECT_ENSURE_REGISTERED (DHashIpv4);


    
TypeId
DHashIpv4::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DHashIpv4")
  .SetParent<Object> ()
  .AddConstructor<DHashIpv4>()
  .AddAttribute ("LocalIpAddress",
                 "Local IP address",
                 Ipv4AddressValue (),
                 MakeIpv4AddressAccessor (&DHashIpv4::m_localIpAddress),
                 MakeIpv4AddressChecker ())
  .AddAttribute ("ListeningPort",
                 "DHash port",
                 UintegerValue (0),
                 MakeUintegerAccessor (&DHashIpv4::m_dHashPort),
                 MakeUintegerChecker<uint16_t> ())
  .AddAttribute ("ConnectionInactivityTimeout",
                 "Timeout value for tearing down inactive connections in milli seconds",
                 TimeValue (MilliSeconds (DEFAULT_CONNECTION_INACTIVITY_TIMEOUT)),
                 MakeTimeAccessor (&DHashIpv4::m_inactivityTimeout),
                 MakeTimeChecker ())
  .AddAttribute ("AuditObjectsTimeout",
                 "Timeout value for auditing objects in milli seconds",
                 TimeValue (MilliSeconds (DEFAULT_AUDIT_OBJECTS_TIMEOUT)),
                 MakeTimeAccessor (&DHashIpv4::m_auditObjectsTimeout),
                 MakeTimeChecker ())
  ;
  return tid;
}

void
DHashIpv4::Start(Ptr<ChordIpv4> chordIpv4)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_transactionId = 0;
  m_chordApplication = chordIpv4;
  if (m_socket == 0)
  {
    TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");
    m_socket = Socket::CreateSocket (chordIpv4->GetNode(), tid);
    InetSocketAddress local = InetSocketAddress (m_localIpAddress, m_dHashPort);
    m_socket->Bind (local);
    m_socket->SetAcceptCallback (
              MakeCallback(&DHashIpv4::HandleConnectionRequest, this), 
              MakeCallback(&DHashIpv4::HandleAccept, this));
  }
  else
  {
    m_socket->SetAcceptCallback (
              MakeCallback(&DHashIpv4::HandleConnectionRequest, this), 
              MakeCallback(&DHashIpv4::HandleAccept, this));
  }
  m_socket->Listen();

  //Configure Callbacks with Chord layer
  m_chordApplication->SetDHashLookupSuccessCallback (MakeCallback(&DHashIpv4::HandleLookupSuccess, this));
  m_chordApplication->SetDHashLookupFailureCallback (MakeCallback(&DHashIpv4::HandleLookupFailure, this));
  m_chordApplication->SetDHashVNodeKeyOwnershipCallback (MakeCallback(&DHashIpv4::HandleOwnershipTrigger, this));

  m_auditConnectionsTimer.SetFunction(&DHashIpv4::DoPeriodicAuditConnections, this);
  m_auditObjectsTimer.SetFunction(&DHashIpv4::DoPeriodicAuditObjects, this);
  //Start timers
  m_auditConnectionsTimer.Schedule (m_inactivityTimeout);
  m_auditObjectsTimer.Schedule (m_auditObjectsTimeout);
}

DHashIpv4::DHashIpv4 ()
  :m_auditConnectionsTimer(Timer::CANCEL_ON_DESTROY),
   m_auditObjectsTimer(Timer::CANCEL_ON_DESTROY)
{
}

void
DHashIpv4::DoDispose ()
{
  if (m_socket != 0)
  {
    m_socket->SetAcceptCallback (
              MakeNullCallback<bool, Ptr<Socket>, const Address & > (), 
              MakeNullCallback<void, Ptr<Socket>, const Address &> ());
  }
  //Cancel timers
  m_auditConnectionsTimer.Cancel();
  m_auditObjectsTimer.Cancel();
}

DHashIpv4::~DHashIpv4 ()
{
}

void 
DHashIpv4::SetInsertSuccessCallback (Callback <void, uint8_t*, uint8_t, uint8_t*, uint32_t> insertSuccessFn)
{
  m_insertSuccessFn = insertSuccessFn;
} 

void 
DHashIpv4::SetRetrieveSuccessCallback (Callback <void, uint8_t*, uint8_t, uint8_t*, uint32_t> retrieveSuccessFn)
{
  m_retrieveSuccessFn = retrieveSuccessFn;
} 
   
void 
DHashIpv4::SetInsertFailureCallback (Callback <void, uint8_t*, uint8_t, uint8_t*, uint32_t> insertFailureFn)
{
  m_insertFailureFn = insertFailureFn;
} 

void 
DHashIpv4::SetRetrieveFailureCallback (Callback <void, uint8_t*, uint8_t> retrieveFailureFn)
{
  m_retrieveFailureFn = retrieveFailureFn;
}

void
DHashIpv4::NotifyInsertSuccess (Ptr<DHashObject> object)
{
  m_insertSuccessFn (object->GetObjectIdentifier()->GetKey(), object->GetObjectIdentifier()->GetNumBytes(), object->GetObject(), object->GetSizeOfObject());
}

void
DHashIpv4::NotifyRetrieveSuccess (Ptr<DHashObject> object)
{
  m_retrieveSuccessFn (object->GetObjectIdentifier()->GetKey(), object->GetObjectIdentifier()->GetNumBytes(), object->GetObject(), object->GetSizeOfObject());
}

void
DHashIpv4::NotifyFailure (Ptr<DHashTransaction> dHashTransaction)
{
  if (dHashTransaction->GetDHashMessage().GetMessageType() == DHashMessage::STORE_REQ)
  {
    NotifyInsertFailure (dHashTransaction->GetDHashMessage().GetStoreReq().dHashObject);
  }
  else if (dHashTransaction->GetDHashMessage().GetMessageType() == DHashMessage::RETRIEVE_REQ)
  {
    NotifyRetrieveFailure (dHashTransaction->GetObjectIdentifier());
  }
}

void
DHashIpv4::NotifyInsertFailure (Ptr<DHashObject> object)
{
  m_insertFailureFn (object->GetObjectIdentifier()->GetKey(), object->GetObjectIdentifier()->GetNumBytes(), object->GetObject(), object->GetSizeOfObject());
}

void
DHashIpv4::NotifyRetrieveFailure (Ptr<ChordIdentifier> objectIdentifier)
{
  m_retrieveFailureFn (objectIdentifier->GetKey(), objectIdentifier->GetNumBytes()); 
}

void
DHashIpv4::Insert (uint8_t *key,uint8_t sizeOfKey ,uint8_t *object,uint32_t sizeOfObject)
{
  Ptr<ChordIdentifier> objectIdentifier = Create<ChordIdentifier> (key, sizeOfKey);
  Ptr<DHashObject> dHashObject = Create<DHashObject> (objectIdentifier, object, sizeOfObject);
  //Check local ownership
  if (m_chordApplication->CheckOwnership (key, sizeOfKey) == true)
  {
    //Store locally  
    AddObject (dHashObject);
    NotifyInsertSuccess (dHashObject);  
    return;
  }
  //Transfer Object
  TransferObject (dHashObject, DHashTransaction::APPLICATION, Ipv4Address::GetZero(), 0);
}

void
DHashIpv4::Retrieve (uint8_t* key, uint8_t sizeOfKey)
{
  Ptr<ChordIdentifier> objectIdentifier = Create<ChordIdentifier> (key, sizeOfKey);
  //Check local ownership
  if (m_chordApplication->CheckOwnership (key, sizeOfKey) == true)
  {
    //Search local
    Ptr<DHashObject> dHashObject;
    if (FindObject (objectIdentifier, dHashObject) == true)
    {
      NotifyRetrieveSuccess (dHashObject);
    }
    else
    {
      NotifyRetrieveFailure (objectIdentifier);
    }
    return;
  }
  //Create Message
  DHashMessage dHashMessage = DHashMessage ();
  PackRetrieveReq (objectIdentifier, dHashMessage);
  //Create Transaction
  Ptr<DHashTransaction> dHashTransaction = Create<DHashTransaction> (dHashMessage.GetTransactionId(), objectIdentifier, dHashMessage);
  AddTransaction (dHashTransaction);
  //Lookup identifier
  m_chordApplication->DHashLookupKey (key, sizeOfKey);
}

void
DHashIpv4::SendDHashRequest (Ipv4Address ipAddress, uint16_t port, Ptr<DHashTransaction> dHashTransaction)
{
  Ptr<Packet> packet = Create<Packet> ();
  packet->AddHeader (dHashTransaction->GetDHashMessage());
  if (packet->GetSize())
  {
    //Set activity flag
    dHashTransaction->SetActiveFlag (true);
    //Check for existing connections
    Ptr<DHashConnection> connection;
    if (FindConnection (ipAddress, port, connection) != true)
    {
      //Open new connection
      TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");
      Ptr<Socket> socket = Socket::CreateSocket(m_chordApplication->GetNode(), tid);
      connection = AddConnection (socket, ipAddress, port);
      socket->Bind ();
      socket->Connect (InetSocketAddress (ipAddress, port));
    }
    dHashTransaction->SetDHashConnection (connection);
    connection->SendTCPData(packet);
    return;
  }
}

void
DHashIpv4::AddTransaction (Ptr<DHashTransaction> dHashTransaction)
{
  //Add transaction to transactionID-TransactionMap
  m_dHashTransactionTable.insert (std::make_pair(dHashTransaction->GetDHashMessage().GetTransactionId(), dHashTransaction));
}

bool
DHashIpv4::FindTransaction (uint32_t transactionId, Ptr<DHashTransaction>& dHashTransaction)
{
  DHashTransactionMap::iterator iterator = m_dHashTransactionTable.find (transactionId);
  if (iterator == m_dHashTransactionTable.end())
  {
    return false;
  }
  dHashTransaction = (*iterator).second;
  return true;
}

void
DHashIpv4::RemoveTransaction (uint32_t transactionId)
{
  DHashTransactionMap::iterator iterator = m_dHashTransactionTable.find (transactionId);
  if (iterator == m_dHashTransactionTable.end())
  {
    return;
  }
  m_dHashTransactionTable.erase (iterator);
  return;
}

void
DHashIpv4::RemoveActiveTransactions (Ptr<Socket> socket)
{
  NS_LOG_INFO ("Connection lost, clearing transactions");
  for (DHashTransactionMap::iterator iterator = m_dHashTransactionTable.begin(); iterator != m_dHashTransactionTable.end(); )
  {
    Ptr<DHashTransaction> dHashTransaction = (*iterator).second;
    if (dHashTransaction->GetActiveFlag() && dHashTransaction->GetDHashConnection()->GetSocket() == socket)
    {
      //Report failure and remove
      NotifyFailure (dHashTransaction);
      m_dHashTransactionTable.erase (iterator++);
    }
    else
      ++iterator;
  }
}

bool
DHashIpv4::HandleConnectionRequest (Ptr<Socket> socket, const Address& address)
{
  //Accept all connections
  return true;
}

void 
DHashIpv4::HandleAccept (Ptr<Socket> socket, const Address& address)
{
  InetSocketAddress from = InetSocketAddress::ConvertFrom (address);
  AddConnection (socket, from.GetIpv4(), from.GetPort());
}

void
DHashIpv4::HandleClose (Ptr<Socket> socket)
{
  //Remove all active transactions running on this socket
  RemoveActiveTransactions (socket);
  //Remove from connection list
  RemoveConnection (socket);
}


void
DHashIpv4::HandleLookupSuccess (uint8_t* lookupKey, uint8_t lookupKeyBytes, Ipv4Address ipAddress, uint16_t port)
{
  NS_LOG_INFO ("*******LOOKUP SUCCESS");
  Ptr<ChordIdentifier> objectIdentifier = Create<ChordIdentifier> (lookupKey, lookupKeyBytes);
  //For all matching transactions, transmit requests
  for (DHashTransactionMap::iterator iterator = m_dHashTransactionTable.begin(); iterator != m_dHashTransactionTable.end(); iterator++)
  {
    Ptr<DHashTransaction> dHashTransaction = (*iterator).second;
    if (objectIdentifier->IsEqual(dHashTransaction->GetObjectIdentifier()))
    {
      //Only transmit for new transactions
      if (!dHashTransaction->GetActiveFlag())
      {
        SendDHashRequest (ipAddress, port, dHashTransaction);
      }

    }
  }
}

void
DHashIpv4::HandleLookupFailure (uint8_t* lookupKey, uint8_t lookupKeyBytes)
{
  Ptr<ChordIdentifier> objectIdentifier = Create<ChordIdentifier> (lookupKey, lookupKeyBytes);
  //For all matching transactions, report failure
  for (DHashTransactionMap::iterator iterator = m_dHashTransactionTable.begin(); iterator != m_dHashTransactionTable.end(); )
  {
    Ptr<DHashTransaction> dHashTransaction = (*iterator).second;
    if (objectIdentifier->IsEqual(dHashTransaction->GetObjectIdentifier()))
    {
      //Report Failure
      if (dHashTransaction->GetDHashMessage().GetMessageType() == DHashMessage::STORE_REQ)
      {
        NotifyInsertFailure (dHashTransaction->GetDHashMessage().GetStoreReq().dHashObject);
      }
      else if (dHashTransaction->GetDHashMessage().GetMessageType() == DHashMessage::RETRIEVE_REQ)
      {
        NotifyRetrieveFailure (objectIdentifier);
      }
      //Erase transaction
      m_dHashTransactionTable.erase (iterator++);
    }
    else
      ++iterator;
  }
}

void
DHashIpv4::HandleOwnershipTrigger (uint8_t* vNodeKey, uint8_t vNodeBytes, uint8_t* predKey, uint8_t predBytes, uint8_t* oldPredKey, uint8_t oldPredBytes, Ipv4Address predIp, uint16_t predPort)
{
  Ptr<ChordIdentifier> predIdentifier = Create<ChordIdentifier> (predKey, predBytes);
  Ptr<ChordIdentifier> oldPredIdentifier = Create<ChordIdentifier> (oldPredKey, oldPredBytes);
  Ptr<ChordIdentifier> vNodeIdentifier = Create<ChordIdentifier> (vNodeKey, vNodeBytes);
  
  //No need to transfer anything if predecessor crashed or left us
  if (oldPredIdentifier->IsInBetween(predIdentifier, vNodeIdentifier))
  {
    return;
  }
  for (DHashObjectMap::iterator iterator = m_dHashObjectTable.begin(); iterator != m_dHashObjectTable.end(); iterator++)
  {
    Ptr<DHashObject> dHashObject = (*iterator).second;
    Ptr<ChordIdentifier> objectIdentifier = dHashObject->GetObjectIdentifier();
    if (objectIdentifier->IsInBetween(oldPredIdentifier, predIdentifier))
    {
      //Transfer object
      TransferObject (dHashObject, DHashTransaction::DHASH ,predIp, predPort);
    }
  }
}

void
DHashIpv4::ProcessDHashMessage (Ptr<Packet> packet, Ptr<DHashConnection> dHashConnection)
{
  DHashMessage dHashMessage = DHashMessage ();
  packet->RemoveHeader (dHashMessage);
  NS_LOG_INFO (dHashMessage);
  switch (dHashMessage.GetMessageType ())
  {
    case DHashMessage::STORE_REQ:
      ProcessStoreReq (dHashMessage, dHashConnection);
      break;
    case DHashMessage::STORE_RSP:
      ProcessStoreRsp (dHashMessage, dHashConnection);
      break;
    case DHashMessage::RETRIEVE_REQ:
      ProcessRetrieveReq (dHashMessage, dHashConnection);
      break;
    case DHashMessage::RETRIEVE_RSP:
      ProcessRetrieveRsp (dHashMessage, dHashConnection);
      break;
    default:
      break;
    
  }

}

void
DHashIpv4::ProcessStoreReq (DHashMessage dHashMessage, Ptr<DHashConnection> dHashConnection)
{
  Ptr<DHashObject> object = dHashMessage.GetStoreReq().dHashObject;

  AddObject (object);
  //Send positive response back
  Ptr<Packet> packet = Create<Packet> ();
  DHashMessage respMessage = DHashMessage();
  PackStoreRsp (dHashMessage.GetTransactionId(), DHashMessage::STORE_SUCCESS, object->GetObjectIdentifier(), respMessage);
  packet->AddHeader(respMessage);
  dHashConnection -> SendTCPData (packet);
  /*
   *
   *  Cannot check ownership and then store. When a new VNode joins, it might not have set its predecessor. 
   *
   *
  //Check ownership
  if (m_chordApplication->CheckOwnership (object->GetObjectIdentifier()->GetKey(), object->GetObjectIdentifier()->GetNumBytes()) == true)
  {
    AddObject (object);
    //Send positive response back
    Ptr<Packet> packet = Create<Packet> ();
    DHashMessage respMessage = DHashMessage();
    PackStoreRsp (dHashMessage.GetTransactionId(), DHashMessage::STORE_SUCCESS, object->GetObjectIdentifier(), respMessage);
    packet->AddHeader(respMessage);
    dHashConnection -> SendTCPData (packet);
    return;
  }
  else
  {
    Ptr<Packet> packet = Create<Packet> ();
    DHashMessage respMessage = DHashMessage();
    //Send negative response back    
    PackStoreRsp (dHashMessage.GetTransactionId(), DHashMessage::NOT_OWNER, object->GetObjectIdentifier(), respMessage);
    packet->AddHeader(respMessage);
    dHashConnection -> SendTCPData (packet);
    return; 
  }*/
}

void
DHashIpv4::ProcessStoreRsp(DHashMessage dHashMessage, Ptr<DHashConnection> dHashConnection)
{
  Ptr<DHashTransaction> dHashTransaction;
  if (FindTransaction(dHashMessage.GetTransactionId(), dHashTransaction) != true)
  {
    return;
  }
  //Notify user
  if (dHashMessage.GetStoreRsp().statusTag == DHashMessage::STORE_SUCCESS)
  {  
    if (dHashTransaction->GetOriginator() == DHashTransaction::APPLICATION)
    { 
      NotifyInsertSuccess (dHashTransaction->GetDHashMessage().GetStoreReq().dHashObject);
    }
    else if (dHashTransaction->GetOriginator() == DHashTransaction::DHASH)
    {
      //Remove object from local store
      RemoveObject (dHashTransaction->GetObjectIdentifier());
    }
  }   
  else
  {
    if (dHashTransaction->GetOriginator() == DHashTransaction::APPLICATION)
    { 
      NotifyInsertFailure (dHashTransaction->GetDHashMessage().GetStoreReq().dHashObject);
    }
  }
  //Remove transaction
  RemoveTransaction (dHashMessage.GetTransactionId()); 
}

void
DHashIpv4::ProcessRetrieveReq (DHashMessage dHashMessage, Ptr<DHashConnection> dHashConnection)
{
  Ptr<ChordIdentifier> objectIdentifier = dHashMessage.GetRetrieveReq().objectIdentifier;
  Ptr<DHashObject> dHashObject;
  if (FindObject(objectIdentifier, dHashObject) == true)
  {
    //Send positive response back
    Ptr<Packet> packet = Create<Packet> ();
    DHashMessage respMessage = DHashMessage();
    PackRetrieveRsp (dHashMessage.GetTransactionId(), DHashMessage::OBJECT_FOUND, dHashObject, respMessage);
    packet->AddHeader(respMessage);
    dHashConnection -> SendTCPData (packet);
    return;
  }
  else
  {
    Ptr<Packet> packet = Create<Packet> ();
    DHashMessage respMessage = DHashMessage();
    //Send negative response back    
    PackRetrieveRsp (dHashMessage.GetTransactionId(), DHashMessage::OBJECT_NOT_FOUND, NULL, respMessage);
    packet->AddHeader(respMessage);
    dHashConnection -> SendTCPData (packet);
    return; 
  }
}

void
DHashIpv4::ProcessRetrieveRsp(DHashMessage dHashMessage, Ptr<DHashConnection> dHashConnection)
{
  Ptr<DHashTransaction> dHashTransaction;
  if (FindTransaction(dHashMessage.GetTransactionId(), dHashTransaction) != true)
  {
    return;
  }
  //Notify user
  if (dHashMessage.GetRetrieveRsp().statusTag == DHashMessage::OBJECT_FOUND)
  {
    NotifyRetrieveSuccess (dHashMessage.GetRetrieveRsp().dHashObject);
  }   
  else
  {
    NotifyRetrieveFailure (dHashTransaction->GetDHashMessage().GetRetrieveReq().objectIdentifier);
  }
  //Remove transaction
  RemoveTransaction (dHashMessage.GetTransactionId()); 
}

void
DHashIpv4::DoPeriodicAuditConnections ()
{
  //Remove inactive connections
  for (DHashConnectionMap::iterator iterator = m_dHashConnectionTable.begin(); iterator != m_dHashConnectionTable.end(); )
  {
    Ptr<DHashConnection> dHashConnection = (*iterator).second;
    if ((dHashConnection->GetLastActivityTime().GetMilliSeconds() + m_inactivityTimeout.GetMilliSeconds()) < Simulator::Now().GetMilliSeconds())
    {
      //Remove all active transactions running on this socket
      RemoveActiveTransactions (dHashConnection->GetSocket());
      //Remove from table
      m_dHashConnectionTable.erase (iterator++);
    }
    else
      ++iterator;
  }
  //Restart timer
  m_auditConnectionsTimer.Schedule (m_inactivityTimeout);
}

void
DHashIpv4::DoPeriodicAuditObjects ()
{
  //Transfer objects which don't belong here
  for (DHashObjectMap::iterator iterator = m_dHashObjectTable.begin(); iterator != m_dHashObjectTable.end(); iterator++)
  {
    Ptr<DHashObject> dHashObject = (*iterator).second;
    if (m_chordApplication->CheckOwnership (dHashObject->GetObjectIdentifier()->GetKey(), dHashObject->GetObjectIdentifier()->GetNumBytes()) != true)
    {
      TransferObject (dHashObject, DHashTransaction::DHASH, Ipv4Address::GetZero(), 0);
    }
  }
  //Restart audit timer
  m_auditObjectsTimer.Schedule(m_auditObjectsTimeout);
}


void
DHashIpv4::PackStoreReq (Ptr<DHashObject> dHashObject, DHashMessage& dHashMessage)
{
  dHashMessage.SetMessageType (DHashMessage::STORE_REQ);
  dHashMessage.SetTransactionId (GetNextTransactionId());
  dHashMessage.GetStoreReq().dHashObject = dHashObject;
}


void
DHashIpv4::PackStoreRsp (uint32_t transactionId, uint8_t statusTag, Ptr<ChordIdentifier> objectIdentifier,DHashMessage& respMessage)
{
  respMessage.SetMessageType (DHashMessage::STORE_RSP);
  respMessage.SetTransactionId (transactionId);
  respMessage.GetStoreRsp().objectIdentifier = objectIdentifier;
  respMessage.GetStoreRsp().statusTag = statusTag;
}

void
DHashIpv4::PackRetrieveReq (Ptr<ChordIdentifier> objectIdentifier, DHashMessage& dHashMessage)
{
  dHashMessage.SetMessageType (DHashMessage::RETRIEVE_REQ);
  dHashMessage.SetTransactionId (GetNextTransactionId());
  dHashMessage.GetRetrieveReq().objectIdentifier = objectIdentifier;
}

void
DHashIpv4::PackRetrieveRsp (uint32_t transactionId, uint8_t statusTag, Ptr<DHashObject> dHashObject, DHashMessage& respMessage)
{
  respMessage.SetMessageType (DHashMessage::RETRIEVE_RSP);
  respMessage.SetTransactionId (transactionId);
  respMessage.GetRetrieveRsp().statusTag = statusTag;
  respMessage.GetRetrieveRsp().dHashObject = dHashObject;
}

void
DHashIpv4::AddObject (Ptr<DHashObject> object)
{
  ChordIdentifier chordIdentifier = *(PeekPointer(object->GetObjectIdentifier()));
  m_dHashObjectTable.insert (std::make_pair(chordIdentifier, object));
}



void 
DHashIpv4::RemoveObject (Ptr<ChordIdentifier> objectIdentifier)
{
   m_dHashObjectTable.erase (objectIdentifier);
}

bool
DHashIpv4::FindObject (Ptr<ChordIdentifier> objectIdentifier, Ptr<DHashObject>& dHashObject)
{
  ChordIdentifier chordIdentifier = *(PeekPointer(objectIdentifier));
  DHashObjectMap::iterator iterator = m_dHashObjectTable.find (chordIdentifier);
  if (iterator == m_dHashObjectTable.end())
  {
    return false;
  } 
  dHashObject = (*iterator).second;
  return true;
}


void
DHashIpv4::TransferObject (Ptr<DHashObject> dHashObject, DHashTransaction::Originator originator, Ipv4Address ipAddress, uint16_t port)
{
  //Create Message
  DHashMessage dHashMessage = DHashMessage ();
  PackStoreReq (dHashObject, dHashMessage);
  //Create transaction
  Ptr<DHashTransaction> dHashTransaction = Create<DHashTransaction> (dHashMessage.GetTransactionId(), dHashObject->GetObjectIdentifier(), dHashMessage);
  dHashTransaction->SetOriginator(originator);
  AddTransaction (dHashTransaction);
  if (ipAddress == Ipv4Address::GetZero())
  {
    //Lookup indentifier
    m_chordApplication->DHashLookupKey (dHashObject->GetObjectIdentifier()->GetKey(), dHashObject->GetObjectIdentifier()->GetNumBytes());
    return;
  }
  //Send DHash request to specified IP
  SendDHashRequest (ipAddress, port, dHashTransaction);
}

Ptr<DHashConnection>
DHashIpv4::AddConnection (Ptr<Socket> socket, Ipv4Address ipAddress, uint16_t port)
{
  //Create new connection control block
  Ptr<DHashConnection> dHashConnection = Create<DHashConnection> (ipAddress, port, socket);
  socket->SetRecvCallback (MakeCallback(&DHashConnection::ReadTCPBuffer, dHashConnection));
  dHashConnection->SetRecvCallback(MakeCallback(&DHashIpv4::ProcessDHashMessage, this));

  socket->SetCloseCallbacks (MakeCallback(&DHashIpv4::HandleClose, this),
                             MakeCallback(&DHashIpv4::HandleClose, this));
  //Add new connection to map
  m_dHashConnectionTable.insert (std::make_pair(socket, dHashConnection));
  return dHashConnection;
}

bool
DHashIpv4::FindConnection (Ptr<Socket> socket, Ptr<DHashConnection>& dHashConnection)
{
  DHashConnectionMap::iterator iterator = m_dHashConnectionTable.find (socket);
  if (iterator == m_dHashConnectionTable.end())
  {
    return false;
  }
  dHashConnection = (*iterator).second;
  return true;
}

bool
DHashIpv4::FindConnection (Ipv4Address ipAddress, uint16_t port, Ptr<DHashConnection>& dHashConnection)
{
  for (DHashConnectionMap::iterator iterator = m_dHashConnectionTable.begin(); iterator != m_dHashConnectionTable.end(); iterator++)
  {
    Ptr<DHashConnection> connection = (*iterator).second;
    if (connection->GetIpAddress() == ipAddress && connection->GetPort() == port)
    {
      //Connection found
      dHashConnection = connection;
      return true;
    }
  }
  return false;
}

void
DHashIpv4::RemoveConnection (Ptr<Socket> socket)
{
  DHashConnectionMap::iterator iterator = m_dHashConnectionTable.find (socket);
  if (iterator == m_dHashConnectionTable.end())
  {
    return;
  }

  m_dHashConnectionTable.erase (iterator);
  return;
}

uint32_t
DHashIpv4::GetNextTransactionId ()
{
  return m_transactionId++;
}

//Diagnostics
void
DHashIpv4::DumpDHashInfo (std::ostream &os)
{
  //Dump stats
  os << "**** Info for DHash Layer ****\n";
  os << "Active TCP Connections: " << m_dHashConnectionTable.size() << "\n";
  os << "Stored DHash Objects: " << m_dHashObjectTable.size() << "\n";
  os << "Pending Transactions: " << m_dHashTransactionTable.size() << "\n";
}

} //namespace ns3
