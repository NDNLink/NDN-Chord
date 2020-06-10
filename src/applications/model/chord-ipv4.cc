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

#include "stdint.h"
#include "stdlib.h"
#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/address-utils.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/callback.h"
#include "ns3/random-variable-stream.h"
#include "ns3/object-factory.h"
#include "chord-ipv4.h"
#include "ns3/double.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ChordIpv4Application");
NS_OBJECT_ENSURE_REGISTERED (ChordIpv4);

TypeId
ChordIpv4::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ChordIpv4")
    .SetParent<Application> ()
    .AddConstructor<ChordIpv4> ()
    .AddAttribute ("BootStrapIp",
                   "IP address of a \"well-known\" chord node (mandatory)",
                   Ipv4AddressValue (),
                   MakeIpv4AddressAccessor (&ChordIpv4::m_bootStrapIp),
                   MakeIpv4AddressChecker ())
    .AddAttribute ("BootStrapPort",
                   "Chord protocol port of bootStrapNode (mandatory)",
                   UintegerValue (0),
                   MakeUintegerAccessor (&ChordIpv4::m_bootStrapPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("LocalIpAddress",
                   "Local IP address (mandatory)",
                   Ipv4AddressValue (),
                   MakeIpv4AddressAccessor (&ChordIpv4::m_localIpAddress),
                   MakeIpv4AddressChecker ())
    .AddAttribute ("ListeningPort",
                   "Chord Protocol port (mandatory)",
                   UintegerValue (0),
                   MakeUintegerAccessor (&ChordIpv4::m_listeningPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("ApplicationPort",
                   "Port to be sent in response to Lookup requests (mandatory)",
                   UintegerValue (0),
                   MakeUintegerAccessor (&ChordIpv4::m_applicationPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("DHashEnable",
                   "DHash layer enable flag",
                   BooleanValue (false),
                   MakeBooleanAccessor (&ChordIpv4::m_dHashEnable),
                   MakeBooleanChecker ())
    .AddAttribute ("DHashPort",
                   "Listening Port to be used with DHash layer (mandatory)",
                   UintegerValue (0),
                   MakeUintegerAccessor (&ChordIpv4::m_dHashPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("MaxVNodeSuccessorListSize",
                   "Max Size of successor list to maintain in a VNode",
                   UintegerValue (DEFAULT_MAX_VNODE_SUCCESSOR_LIST_SIZE),
                   MakeUintegerAccessor (&ChordIpv4::m_maxVNodeSuccessorListSize),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("MaxVNodePredecessorListSize",
                   "Max Size of predessor list to maintain in a VNode",
                   UintegerValue (DEFAULT_MAX_VNODE_PREDECESSOR_LIST_SIZE),
                   MakeUintegerAccessor (&ChordIpv4::m_maxVNodePredecessorListSize),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("StabilizeInterval",
                   "Stabilize Interval in milli seconds",
                   TimeValue (MilliSeconds (DEFAULT_STABILIZE_INTERVAL)),
                   MakeTimeAccessor (&ChordIpv4::m_stabilizeInterval),
                   MakeTimeChecker ())
    .AddAttribute ("HeartbeatInterval",
                   "Heartbeat Interval in milli seconds",
                   TimeValue (MilliSeconds (DEFAULT_HEARTBEAT_INTERVAL)),
                   MakeTimeAccessor (&ChordIpv4::m_heartbeatInterval),
                   MakeTimeChecker ())
    .AddAttribute ("MaxMissedKeepAlives",
                   "Number of missed Heartbeats and Stabilize requests before declaring node dead",
                   UintegerValue (DEFAULT_MAX_MISSED_KEEP_ALIVES),
                   MakeUintegerAccessor (&ChordIpv4::m_maxMissedKeepAlives),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("MaxRequestRetries",
                   "Number of request retries before giving up",
                   UintegerValue (DEFAULT_MAX_REQUEST_RETRIES),
                   MakeUintegerAccessor (&ChordIpv4::m_maxRequestRetries),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("RequestTimeout",
                   "Timeout value for request retransmission in milli seconds",
                   TimeValue (MilliSeconds (DEFAULT_REQUEST_TIMEOUT)),
                   MakeTimeAccessor (&ChordIpv4::m_requestTimeout),
                   MakeTimeChecker ())
    .AddAttribute ("DHashInactivityTimeout",
                   "Timeout value for closing inactive TCP connection in milli seconds",
                   TimeValue (MilliSeconds (DEFAULT_CONNECTION_INACTIVITY_TIMEOUT)),
                   MakeTimeAccessor (&ChordIpv4::m_dHashInactivityTimeout),
                   MakeTimeChecker ())
    .AddAttribute ("DHashAuditObjectsTimeout",
                   "Timeout value for auditing stored DHash Objects in milli seconds",
                   TimeValue (MilliSeconds (DEFAULT_AUDIT_OBJECTS_TIMEOUT)),
                   MakeTimeAccessor (&ChordIpv4::m_dHashAuditObjectsTimeout),
                   MakeTimeChecker ())
    .AddAttribute ("FixFingerInterval",
                   "Fix Finger Interval in milli seconds",
                   TimeValue (MilliSeconds (DEFAULT_FIX_FINGER_INTERVAL)),
                   MakeTimeAccessor (&ChordIpv4::m_fixFingerInterval),
                   MakeTimeChecker ())

     ;
  return tid;
}

//Constructor for ChordIpv4 application
ChordIpv4::ChordIpv4 ()
  :m_stabilizeTimer (Timer::CANCEL_ON_DESTROY),
  m_heartbeatTimer (Timer::CANCEL_ON_DESTROY),
  m_fixFingerTimer (Timer::CANCEL_ON_DESTROY)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_socket = 0;
  isBootStrapNode = false;
  //Timer configuration
}

ChordIpv4::~ChordIpv4 ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_socket = 0;
}

void
ChordIpv4::DoDispose (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  StopApplication();
  Application::DoDispose ();
}

void
ChordIpv4::StartApplication (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_INFO("***ChordIpv4 starting on Node: " << GetNode()->GetId() 
              <<"\n***Parameters: "
              <<"\n***bootStrapIp: " << m_bootStrapIp
              <<"\n***bootStrapPort: " << m_bootStrapPort
              <<"\n***listeningPort: " << m_listeningPort
              <<"\n***localIp: " << m_localIpAddress
              );
  if (m_bootStrapIp == m_localIpAddress  && m_bootStrapPort == m_listeningPort)
  {
    isBootStrapNode = true; 
  }

  if (m_socket == 0)
  {
    TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
    m_socket = Socket::CreateSocket (GetNode(), tid);
    InetSocketAddress local = InetSocketAddress (m_localIpAddress, m_listeningPort);
    m_socket->Bind (local);
    m_socket->SetRecvCallback(MakeCallback(&ChordIpv4::ProcessUdpPacket, this));
  }
  else
  {
    m_socket->SetRecvCallback(MakeCallback(&ChordIpv4::ProcessUdpPacket, this));
  }

  //Start DHash layer
  if (m_dHashEnable == true)
  {
    //Create layer
    ObjectFactory factory;
    factory.SetTypeId (DHashIpv4::GetTypeId());
    factory.Set ("LocalIpAddress", Ipv4AddressValue(m_localIpAddress));
    factory.Set ("ListeningPort", UintegerValue(m_dHashPort));
    factory.Set ("ConnectionInactivityTimeout", TimeValue(m_dHashInactivityTimeout));
    factory.Set ("AuditObjectsTimeout", TimeValue(m_dHashAuditObjectsTimeout));
    m_dHashIpv4 = factory.Create<DHashIpv4> ();
    m_dHashIpv4->SetInsertSuccessCallback (MakeCallback(&ChordIpv4::NotifyInsertSuccess, this));
    m_dHashIpv4->SetRetrieveSuccessCallback (MakeCallback(&ChordIpv4::NotifyRetrieveSuccess, this));
    m_dHashIpv4->SetInsertFailureCallback (MakeCallback(&ChordIpv4::NotifyInsertFailure, this));
    m_dHashIpv4->SetRetrieveFailureCallback (MakeCallback(&ChordIpv4::NotifyRetrieveFailure, this));
    //Start layer
    m_dHashIpv4->Start (this);
  }

  //Configure timer
  m_stabilizeTimer.SetFunction(&ChordIpv4::DoPeriodicStabilize, this);
  m_heartbeatTimer.SetFunction(&ChordIpv4::DoPeriodicHeartbeat, this);
  m_fixFingerTimer.SetFunction(&ChordIpv4::DoPeriodicFixFinger, this);

  //Start timers
  m_stabilizeTimer.Schedule(m_stabilizeInterval);
  m_heartbeatTimer.Schedule(m_heartbeatInterval);
  m_fixFingerTimer.Schedule(m_fixFingerInterval);
}

void
ChordIpv4::StopApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();
  if (m_socket != 0)
  {
    //m_socket->Close ();
    m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket> > ());
    //m_socket = 0;
  }
  m_dHashIpv4 -> DoDispose();
  //Cancel Timers
  m_stabilizeTimer.Cancel();
  m_heartbeatTimer.Cancel();
  m_fixFingerTimer.Cancel();
  //Delete vNodes
  m_vNodeMap.Clear();
}


void
ChordIpv4::SetJoinSuccessCallback (Callback<void, std::string, uint8_t*, uint8_t> joinSuccessFn)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_joinSuccessFn = joinSuccessFn;
}

void
ChordIpv4::SetLookupSuccessCallback (Callback<void, uint8_t*, uint8_t, Ipv4Address, uint16_t> lookupSuccessFn)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_lookupSuccessFn = lookupSuccessFn;
}

void
ChordIpv4::SetLookupFailureCallback (Callback<void, uint8_t*, uint8_t> lookupFailureFn)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_lookupFailureFn = lookupFailureFn;
}

void
ChordIpv4::SetDHashLookupSuccessCallback (Callback<void, uint8_t*, uint8_t, Ipv4Address, uint16_t> dHashLookupSuccessFn)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_dHashLookupSuccessFn = dHashLookupSuccessFn;
}

void
ChordIpv4::SetDHashLookupFailureCallback (Callback<void, uint8_t*, uint8_t> dHashLookupFailureFn)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_dHashLookupFailureFn = dHashLookupFailureFn;
}

void
ChordIpv4::SetTraceRingCallback (Callback<void, std::string, uint8_t*, uint8_t> traceRingFn)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_traceRingFn = traceRingFn;
}

void
ChordIpv4::SetVNodeFailureCallback (Callback<void, std::string, uint8_t*, uint8_t> vNodeFailureFn)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_vNodeFailureFn = vNodeFailureFn;
}

void
ChordIpv4::SetVNodeKeyOwnershipCallback (Callback<void, std::string, uint8_t*, uint8_t, uint8_t*, uint8_t, uint8_t*, uint8_t, Ipv4Address, uint16_t> vNodeKeyOwnershipFn)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_vNodeKeyOwnershipFn = vNodeKeyOwnershipFn;
}

void 
ChordIpv4::SetInsertSuccessCallback (Callback <void, uint8_t*, uint8_t, uint8_t*, uint32_t> insertSuccessFn)
{
  m_insertSuccessFn = insertSuccessFn;
} 

void 
ChordIpv4::SetRetrieveSuccessCallback (Callback <void, uint8_t*, uint8_t, uint8_t*, uint32_t> retrieveSuccessFn)
{
  m_retrieveSuccessFn = retrieveSuccessFn;
} 
   
void 
ChordIpv4::SetInsertFailureCallback (Callback <void, uint8_t*, uint8_t, uint8_t*, uint32_t> insertFailureFn)
{
  m_insertFailureFn = insertFailureFn;
} 

void 
ChordIpv4::SetRetrieveFailureCallback (Callback <void, uint8_t*, uint8_t> retrieveFailureFn)
{
  m_retrieveFailureFn = retrieveFailureFn;
}

void 
ChordIpv4:: SetDHashVNodeKeyOwnershipCallback (Callback <void, uint8_t*, uint8_t, uint8_t*, uint8_t, uint8_t*, uint8_t, Ipv4Address, uint16_t> dHashVNodeKeyOwnershipFn)
{
  m_dHashVNodeKeyOwnershipFn = dHashVNodeKeyOwnershipFn;
}

void
ChordIpv4::NotifyJoinSuccess (std::string vNodeName, Ptr<ChordIdentifier> chordIdentifier)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (!m_joinSuccessFn.IsNull ())
  {
    m_joinSuccessFn (vNodeName ,chordIdentifier->GetKey (), chordIdentifier->GetNumBytes());
  }
}

void
ChordIpv4::NotifyLookupSuccess (Ptr<ChordIdentifier> lookupIdentifier, Ptr<ChordNode> resolvedNode, ChordTransaction::Originator originator)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (!m_lookupSuccessFn.IsNull() && originator == ChordTransaction::APPLICATION)
  {
    m_lookupSuccessFn (lookupIdentifier->GetKey(), lookupIdentifier->GetNumBytes(), resolvedNode->GetIpAddress(), resolvedNode->GetApplicationPort());
  }
  else if (originator == ChordTransaction::DHASH)
  {
    NotifyDHashLookupSuccess (lookupIdentifier, resolvedNode->GetIpAddress(), resolvedNode->GetDHashPort());
  }
}

void
ChordIpv4::NotifyLookupFailure (Ptr<ChordIdentifier> chordIdentifier, ChordTransaction::Originator originator)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (!m_lookupFailureFn.IsNull() && originator == ChordTransaction::APPLICATION)
  {
    m_lookupFailureFn (chordIdentifier->GetKey (), chordIdentifier->GetNumBytes ());
  }
  else if (originator == ChordTransaction::DHASH)
  {
    NotifyDHashLookupFailure (chordIdentifier);
  }
}
void
ChordIpv4::NotifyDHashLookupSuccess (Ptr<ChordIdentifier> lookupIdentifier, Ipv4Address ipAddress, uint16_t port)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (!m_dHashLookupSuccessFn.IsNull())
  {
    m_dHashLookupSuccessFn (lookupIdentifier->GetKey(), lookupIdentifier->GetNumBytes(), ipAddress, port);
  }
}

void
ChordIpv4::NotifyDHashLookupFailure (Ptr<ChordIdentifier> chordIdentifier)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (!m_dHashLookupFailureFn.IsNull())
  {
    m_dHashLookupFailureFn (chordIdentifier->GetKey (), chordIdentifier->GetNumBytes ());
  }
}

void
ChordIpv4::NotifyVNodeKeyOwnership (std::string vNodeName, Ptr<ChordIdentifier> chordIdentifier, Ptr<ChordNode> predecessorNode, Ptr<ChordIdentifier> oldPredecessorIdentifier)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (!m_vNodeKeyOwnershipFn.IsNull ())
  {
    m_vNodeKeyOwnershipFn (vNodeName, chordIdentifier->GetKey (), chordIdentifier->GetNumBytes(), predecessorNode->GetChordIdentifier()->GetKey (), predecessorNode->GetChordIdentifier()->GetNumBytes (), oldPredecessorIdentifier->GetKey(), oldPredecessorIdentifier->GetNumBytes(), predecessorNode->GetIpAddress(), predecessorNode->GetApplicationPort());
  }
  if (m_dHashEnable)
  {
    m_dHashVNodeKeyOwnershipFn (chordIdentifier->GetKey(), chordIdentifier->GetNumBytes(),predecessorNode->GetChordIdentifier()->GetKey (), predecessorNode->GetChordIdentifier()->GetNumBytes (), oldPredecessorIdentifier->GetKey(), oldPredecessorIdentifier->GetNumBytes(), predecessorNode->GetIpAddress(), predecessorNode->GetDHashPort());
  }
}

void
ChordIpv4::NotifyTraceRing (std::string vNodeName, Ptr<ChordIdentifier> chordIdentifier)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (!m_traceRingFn.IsNull ())
  {
    m_traceRingFn (vNodeName ,chordIdentifier->GetKey (), chordIdentifier->GetNumBytes());
  }
}

void
ChordIpv4::NotifyVNodeFailure (std::string vNodeName, Ptr<ChordIdentifier> chordIdentifier)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (!m_vNodeFailureFn.IsNull())
  {
    m_vNodeFailureFn (vNodeName, chordIdentifier->GetKey (), chordIdentifier->GetNumBytes());
  }
}

void
ChordIpv4::NotifyInsertSuccess (uint8_t* key, uint8_t keyBytes, uint8_t* object, uint32_t objectBytes)
{
  m_insertSuccessFn (key, keyBytes, object, objectBytes);
}

void
ChordIpv4::NotifyRetrieveSuccess (uint8_t* key, uint8_t keyBytes, uint8_t* object, uint32_t objectBytes)
{
  m_retrieveSuccessFn (key, keyBytes, object, objectBytes);
}


void
ChordIpv4::NotifyInsertFailure (uint8_t* key, uint8_t keyBytes, uint8_t* object, uint32_t objectBytes)
{
  m_insertFailureFn (key, keyBytes, object, objectBytes);
}

void
ChordIpv4::NotifyRetrieveFailure (uint8_t* key, uint8_t keyBytes)
{
  m_retrieveFailureFn (key, keyBytes); 
}

void
ChordIpv4::Insert (uint8_t *key, uint8_t sizeOfKey ,uint8_t *object,uint32_t sizeOfObject)
{
  if (m_dHashEnable)
  {
    m_dHashIpv4->Insert(key, sizeOfKey, object, sizeOfObject);
  }
}

void
ChordIpv4::Retrieve (uint8_t *key, uint8_t sizeOfKey)
{
  if (m_dHashEnable)
  {
    m_dHashIpv4->Retrieve(key, sizeOfKey);
  }
}

void
ChordIpv4::InsertVNode (std::string vNodeName, uint8_t* key, uint8_t keyBytes)
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_INFO ("Creating Chord Virtual Node at NS3 physical node: " << GetNode ()->GetId());
  Ptr<ChordIdentifier> chordIdentifier = Create<ChordIdentifier> (key, keyBytes);
  //Create VNode object
  Ptr<ChordNode> node = Create<ChordNode> (chordIdentifier, vNodeName, m_localIpAddress, m_listeningPort, m_applicationPort, m_dHashPort);
  Ptr<ChordVNode> vNode = Create<ChordVNode> (node, m_maxVNodeSuccessorListSize, m_maxVNodePredecessorListSize);
  //Own up entire key-space
  vNode-> SetSuccessor (Create<ChordNode> (vNode));
  vNode-> SetPredecessor (Create<ChordNode> (vNode));
  //Set routable = false
  vNode->SetRoutable(false);

  /* bootStrapIp is same as local Ip and no v-nodes exist. In that case we need to create a new chord */
  if (isBootStrapNode && m_vNodeMap.GetSize() == 0)
  {
    //Create a new chord network
    //Insert VNode into list
    Ptr<ChordNode> chordNode = DynamicCast<ChordNode>(vNode);
    m_vNodeMap.UpdateNode (chordNode);
    DoFixFinger (vNode);
    NotifyJoinSuccess(vNode->GetVNodeName(), vNode->GetChordIdentifier());
    return;
  }

  //Insert VNode into list
  Ptr<ChordNode> chordNode = DynamicCast<ChordNode>(vNode); 
  m_vNodeMap.UpdateNode (chordNode);
  //Send this request to bootstrap IP
  Ptr<Packet> packet = Create<Packet> ();
  ChordMessage chordMessage = ChordMessage ();
  vNode->PackJoinReq (chordMessage);
  //Add transaction
  Ptr<ChordTransaction> chordTransaction = Create<ChordTransaction> (chordMessage.GetTransactionId(), chordMessage, m_requestTimeout, m_maxRequestRetries);
  //Add to vNode
  vNode -> AddTransaction (chordMessage.GetTransactionId(), chordTransaction);

  //Start transaction timer
  EventId requestTimeoutId = Simulator::Schedule (chordTransaction->GetRequestTimeout(), &ChordIpv4::HandleRequestTimeout, this, vNode, chordMessage.GetTransactionId());
  chordTransaction -> SetRequestTimeoutEventId (requestTimeoutId);
  packet->AddHeader (chordMessage);
  if (packet->GetSize())
  {
    NS_LOG_INFO ("Sending JoinReq\n" << chordMessage);
    if (m_vNodeMap.GetSize() > 1)
    {
      if (RoutePacket (vNode->GetChordIdentifier(), packet) == true)
      {
        return;
      }
    }
    //Default:: Send to bootstrap node
    SendPacket (packet, m_bootStrapIp, m_bootStrapPort);
  }
}


void
ChordIpv4::RemoveVNode (std::string vNodeName)
{
  NS_LOG_INFO ("Removing Chord Virtual Node at NS3 physical node: " << GetNode ()->GetId());
  NS_LOG_FUNCTION_NOARGS ();
  //Iterate through the  m_vNodeMap and find the specified Vnode which has to leave.
  //Iterate VNode list and check if v node exists
  Ptr<ChordNode> chordNode; 
  if (m_vNodeMap.FindNode(vNodeName, chordNode) != true)
  {
    return;
  }
  Ptr<ChordVNode> virtualNode = DynamicCast<ChordVNode>(chordNode);

  //Send this request to bootstrap IP
  Ptr<Packet> packet = Create<Packet> ();
  ChordMessage chordMessage = ChordMessage ();
  virtualNode ->PackLeaveReq (chordMessage);
 
  packet->AddHeader (chordMessage);
  if (packet->GetSize())
  {
    NS_LOG_INFO ("Sending LeaveReq\n" << chordMessage);
    SendPacket (packet,virtualNode-> GetSuccessor() -> GetIpAddress() , virtualNode -> GetSuccessor() -> GetPort());
    SendPacket (packet,virtualNode-> GetPredecessor() -> GetIpAddress() , virtualNode -> GetPredecessor() -> GetPort());
  }

  //delete VNode from m_vNodeMap  
  DeleteVNode(vNodeName);
  return;
}

void
ChordIpv4::LookupKey (uint8_t* lookupKey, uint8_t lookupKeyBytes)
{
  NS_LOG_FUNCTION_NOARGS ();
  Ptr<ChordIdentifier> requestedIdentifier = Create<ChordIdentifier> (lookupKey, lookupKeyBytes);
  DoLookup (requestedIdentifier, ChordTransaction::APPLICATION);
}
void
ChordIpv4::DHashLookupKey (uint8_t* lookupKey, uint8_t lookupKeyBytes)
{
  NS_LOG_FUNCTION_NOARGS ();
  Ptr<ChordIdentifier> requestedIdentifier = Create<ChordIdentifier> (lookupKey, lookupKeyBytes);
  DoLookup (requestedIdentifier, ChordTransaction::DHASH);
}

void
ChordIpv4::DoLookup (Ptr<ChordIdentifier> requestedIdentifier, ChordTransaction::Originator originator)
{
  NS_LOG_FUNCTION_NOARGS ();
  //Find local
  Ptr<ChordVNode> virtualNode;
  bool ret = LookupLocal (requestedIdentifier, virtualNode);
  if (ret == true)
  {
    //We are owner, report success
    NotifyLookupSuccess(requestedIdentifier, virtualNode, originator);
    return;
  } 
  //Initiate lookup request
  
  if (FindNearestVNode (requestedIdentifier, virtualNode) == true)
  {
    Ptr<Packet> packet = Create<Packet> ();
    ChordMessage chordMessage = ChordMessage ();
    virtualNode->PackLookupReq (requestedIdentifier, chordMessage);
    //Add transaction
    Ptr<ChordTransaction> chordTransaction = Create<ChordTransaction> (chordMessage.GetTransactionId(), chordMessage, m_requestTimeout, m_maxRequestRetries);
    chordTransaction->SetOriginator(originator);
    chordTransaction->SetRequestedIdentifier (requestedIdentifier);
    //Add to vNode
    virtualNode -> AddTransaction (chordMessage.GetTransactionId(), chordTransaction);

    //Start transaction timer
    EventId requestTimeoutId = Simulator::Schedule (chordTransaction->GetRequestTimeout(), &ChordIpv4::HandleRequestTimeout, this, virtualNode, chordMessage.GetTransactionId());
    chordTransaction -> SetRequestTimeoutEventId (requestTimeoutId);
    packet->AddHeader (chordMessage);
    if (packet->GetSize())
    {
      RouteViaFinger (requestedIdentifier, virtualNode, packet); 
      //SendPacket (packet, virtualNode->GetSuccessor()->GetIpAddress(), virtualNode->GetSuccessor()->GetPort());
    }
  }  
  else
  {
    NotifyLookupFailure (requestedIdentifier, originator);
    return;
  }
}


void
ChordIpv4::ProcessUdpPacket (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION_NOARGS ();
  Ptr<Packet> packet;
  Address from;
  while (packet = socket->RecvFrom(from))
  {
    if (InetSocketAddress::IsMatchingType (from))
    {
      InetSocketAddress address = InetSocketAddress::ConvertFrom (from);
      NS_LOG_INFO ("ChordIpv4: Received " << packet->GetSize() << " bytes packet from " <<  address.GetIpv4());
      Ipv4Address fromIpAddress = address.GetIpv4();

      ChordMessage chordMessage = ChordMessage ();
      //Retrieve and Deserialize chord message
      packet->RemoveHeader(chordMessage);
      NS_LOG_INFO ("ChordMessage: " << chordMessage);
      switch (chordMessage.GetMessageType ())
      {
       case ChordMessage::JOIN_REQ:
         ProcessJoinReq (chordMessage);
         break;
       case ChordMessage::JOIN_RSP:
         ProcessJoinRsp (chordMessage);
         break;
       case ChordMessage::LEAVE_REQ:
         ProcessLeaveReq (chordMessage);
         break;
       case ChordMessage::LEAVE_RSP:
         ProcessLeaveRsp (chordMessage);
         break;
       case ChordMessage::LOOKUP_REQ:
         ProcessLookupReq (chordMessage);
         break;
       case ChordMessage::LOOKUP_RSP:
         ProcessLookupRsp (chordMessage);
         break;
       case ChordMessage::STABILIZE_REQ:
         ProcessStabilizeReq (chordMessage);
         break;
       case ChordMessage::STABILIZE_RSP:
         ProcessStabilizeRsp (chordMessage);
         break;
       case ChordMessage::HEARTBEAT_REQ:
         ProcessHeartbeatReq (chordMessage);
         break;
       case ChordMessage::HEARTBEAT_RSP:
         ProcessHeartbeatRsp (chordMessage);
         break;
       case ChordMessage::FINGER_REQ:
         ProcessFingerReq (chordMessage);
         break;
       case ChordMessage::FINGER_RSP:
         ProcessFingerRsp (chordMessage);
         break;
       case ChordMessage::TRACE_RING:
         ProcessTraceRing (chordMessage);
         break;
       default:
         break;

      }
    }
  }
}

void
ChordIpv4::ProcessJoinReq (ChordMessage chordMessage)
{
  NS_LOG_FUNCTION_NOARGS ();
  Ptr<ChordNode> requestorNode = chordMessage.GetRequestorNode();
  uint32_t transactionId = chordMessage.GetTransactionId ();
  if (m_vNodeMap.GetSize() == 0)
  {
    //No vNode exists as yet, drop this request.
    return;
  }
  Ptr<Packet> packet = Create<Packet> ();
  //Check if we can be this node's successor
  Ptr<ChordVNode> virtualNode;
  bool ret = LookupLocal (requestorNode->GetChordIdentifier(), virtualNode);
  if (ret == true)
  {
    ChordMessage chordMessageRsp = ChordMessage ();
    virtualNode->PackJoinRsp (requestorNode, transactionId, chordMessageRsp);
    packet-> AddHeader (chordMessageRsp);
    //Send packet
    if (packet->GetSize())
    {
      NS_LOG_INFO("Sending JoinRsp: "<<chordMessageRsp);
      SendPacket(packet, requestorNode->GetIpAddress(), requestorNode->GetPort());
    }
    return;
  }
  //Could not resolve join request, forward to nearest successor
  packet->AddHeader(chordMessage);
  if (packet->GetSize())
  {
    RoutePacket (requestorNode->GetChordIdentifier(), packet);
  }
}

void
ChordIpv4::ProcessJoinRsp (ChordMessage chordMessage)
{
  NS_LOG_FUNCTION_NOARGS ();
  //Extract info from packet
  Ptr<ChordNode> requestorNode = chordMessage.GetRequestorNode();
  Ptr<ChordNode> successorNode = chordMessage.GetJoinRsp().successorNode;
  //Find virtual node which sent this message
  Ptr<ChordVNode> virtualNode;
  bool ret = FindVNode(requestorNode->GetChordIdentifier(), virtualNode);
  if (ret == true)
  {  
    //Find Transaction
    Ptr<ChordTransaction> chordTransaction;
    if (virtualNode->FindTransaction(chordMessage.GetTransactionId(), chordTransaction) == false)
    {
      //No transaction exists, return from here
      return;
    }
    //VNode found, set its successor and stabilize
    virtualNode->SetSuccessor(Create<ChordNode> (successorNode));
    //Make this node routable
    virtualNode->SetRoutable (true);
    //cancel transaction
    virtualNode->RemoveTransaction (chordTransaction->GetTransactionId());
    DoStabilize(virtualNode);
    DoFixFinger (virtualNode);
    //notify application about join success
    NotifyJoinSuccess(virtualNode->GetVNodeName(), requestorNode->GetChordIdentifier());
  }
}

void
ChordIpv4::ProcessLookupReq (ChordMessage chordMessage)
{
  NS_LOG_FUNCTION_NOARGS ();
  Ptr<ChordNode> requestorNode = chordMessage.GetRequestorNode();
  Ptr<ChordIdentifier> requestedIdentifier = chordMessage.GetLookupReq().requestedIdentifier;
  uint32_t transactionId = chordMessage.GetTransactionId ();
  if (m_vNodeMap.GetSize() == 0)
  {
    //No vNode exists as yet, drop this request.
    return;
  }
  Ptr<Packet> packet = Create<Packet> ();
  //Check if we are owner of requestedIdentifier
  Ptr<ChordVNode> virtualNode;
  bool ret = LookupLocal (requestedIdentifier, virtualNode);
  if (ret == true)
  {
    ChordMessage chordMessageRsp = ChordMessage ();
    virtualNode->PackLookupRsp (requestorNode,  transactionId, chordMessageRsp);
    packet-> AddHeader (chordMessageRsp);
    //Send packet
    if (packet->GetSize())
    {
      NS_LOG_INFO("Sending LookupRsp: "<<chordMessageRsp);
      SendPacket(packet, requestorNode->GetIpAddress(), requestorNode->GetPort());
    }
    return;
  }
  //Could not resolve lookup request, forward to nearest successor
  packet->AddHeader(chordMessage);
  RoutePacket (requestedIdentifier, packet);
}

void
ChordIpv4::ProcessLeaveReq(ChordMessage chordMessage)
{
  NS_LOG_FUNCTION_NOARGS ();

  Ptr<ChordNode> requestorNode = chordMessage.GetRequestorNode();
  //Read payload and get vnode identifier
  Ptr<ChordNode> successorNode = chordMessage.GetLeaveReq().successorNode;
  Ptr<ChordNode> predecessorNode = chordMessage.GetLeaveReq().predecessorNode;

  //Find VNode
  Ptr<ChordVNode> virtualNode;

  //Are we successor node?
  bool ret;
  ret = FindVNode(successorNode->GetChordIdentifier(), virtualNode);
  if (ret == true && virtualNode->GetPredecessor()->GetChordIdentifier()->IsEqual(requestorNode->GetChordIdentifier()))
  {
    //Reset own predecessor
    Ptr<ChordNode> oldPredecessorNode = virtualNode->GetPredecessor();
    virtualNode->SetPredecessor(Create<ChordNode> (predecessorNode));
    NotifyVNodeKeyOwnership (virtualNode->GetVNodeName(), virtualNode->GetChordIdentifier(), virtualNode->GetPredecessor(), oldPredecessorNode->GetChordIdentifier());
    //Send Leave Rsp (only required in case of successor)
    ChordMessage respMessage = ChordMessage ();
    virtualNode->PackLeaveRsp (requestorNode, successorNode, predecessorNode, chordMessage);
    Ptr<Packet> packet = Create<Packet> ();
    packet->AddHeader (chordMessage);
    SendPacket (packet, requestorNode->GetIpAddress(), requestorNode->GetPort());
    NS_LOG_INFO("Predecessor changed for VNode");
  }

  //Are we predecessor node?
  ret = FindVNode(predecessorNode->GetChordIdentifier(), virtualNode);
  if (ret == true && virtualNode->GetSuccessor()->GetChordIdentifier()->IsEqual(requestorNode->GetChordIdentifier()))
  {
    //Reset own successor
    virtualNode->SetSuccessor(Create<ChordNode> (successorNode));
    DoFixFinger (virtualNode);
    virtualNode->SetRoutable(true);
    NS_LOG_INFO("Successor changed for VNode");
  }
}

void
ChordIpv4::ProcessLeaveRsp (ChordMessage chordMessage)
{
  NS_LOG_FUNCTION_NOARGS ();
  Ptr<ChordNode> requestorNode = chordMessage.GetRequestorNode();
  Ptr<ChordNode> successorNode = chordMessage.GetLeaveRsp().successorNode;
  Ptr<ChordNode> predecessorNode = chordMessage.GetLeaveRsp().predecessorNode;
  
  Ptr<ChordIdentifier> requestorIdentifier = requestorNode->GetChordIdentifier();
  Ptr<ChordIdentifier> successorIdentifier = successorNode->GetChordIdentifier();
  Ptr<ChordIdentifier> predecessorIdentifier = predecessorNode->GetChordIdentifier();
  
  if (!m_dHashVNodeKeyOwnershipFn.IsNull() && m_dHashEnable)
  {
    //Send trigger to dHash
    m_dHashVNodeKeyOwnershipFn (successorIdentifier->GetKey(), successorIdentifier->GetNumBytes(), requestorIdentifier->GetKey(), requestorIdentifier->GetNumBytes(), predecessorIdentifier->GetKey(), predecessorIdentifier->GetNumBytes(), successorNode->GetIpAddress(), successorNode->GetDHashPort());
  }
}

void
ChordIpv4::ProcessLookupRsp (ChordMessage chordMessage)
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_INFO ("Received Lookup Response");
  //Extract info from packet
  Ptr<ChordNode> requestorNode = chordMessage.GetRequestorNode();
  Ptr<ChordNode> resolvedNode = chordMessage.GetLookupRsp().resolvedNode;
  //Find virtual node which sent this message
  Ptr<ChordVNode> virtualNode;
  bool ret = FindVNode(requestorNode->GetChordIdentifier(), virtualNode);
  if (ret == true)
  {  
    //Find Transaction
    Ptr<ChordTransaction> chordTransaction;
    if (virtualNode->FindTransaction(chordMessage.GetTransactionId(), chordTransaction) == false)
    {
      //No transaction exists, return from here
      return;
    }
    Ptr<ChordIdentifier> requestedIdentifier = chordTransaction->GetRequestedIdentifier ();
    ChordTransaction::Originator originator = chordTransaction->GetOriginator();
    //cancel transaction
    virtualNode->RemoveTransaction (chordTransaction->GetTransactionId());
    //notify application about lookup success
    NotifyLookupSuccess(requestedIdentifier, resolvedNode, originator);
  }
}


void
ChordIpv4::ProcessStabilizeReq(ChordMessage chordMessage)
{
  Ptr<ChordNode> requestorNode = chordMessage.GetRequestorNode();

  //Read payload and get vnode identifier
  Ptr<ChordIdentifier> vNodeIdentifier = chordMessage.GetStabilizeReq().successorIdentifier;

  //Find VNode
  Ptr<ChordVNode> virtualNode;
  bool ret = FindVNode(vNodeIdentifier, virtualNode);
  if (ret == false)
  {
    //VNode does not exist here, drop packet
    return;
  }

  //Stabilize
  
  if (requestorNode->GetChordIdentifier()->IsInBetween(virtualNode->GetPredecessor()->GetChordIdentifier(), virtualNode->GetChordIdentifier()))
  {
    //Reset own predecessor
    Ptr<ChordNode> predecessorNode = Create<ChordNode> (requestorNode);
    Ptr<ChordNode> oldPredecessorNode = virtualNode->GetPredecessor();
    virtualNode->SetPredecessor(predecessorNode);
    //Check if requestor can be our successor as well (bootstrap case)
    if (virtualNode->GetSuccessor()->GetChordIdentifier()->IsEqual(virtualNode->GetChordIdentifier()))
    {
      //Reset Successor as well
      Ptr<ChordNode> successorNode = Create<ChordNode> (requestorNode);
      virtualNode->SetSuccessor(successorNode);
      virtualNode->SetRoutable(true);
      //Stabilize
      DoStabilize(virtualNode);
      DoFixFinger (virtualNode);
    }
    NotifyVNodeKeyOwnership (virtualNode->GetVNodeName(), virtualNode->GetChordIdentifier(), virtualNode->GetPredecessor(), oldPredecessorNode->GetChordIdentifier());
    NS_LOG_INFO("Predecessor changed for VNode");
  }
  Ptr<Packet> packet = Create<Packet> ();
  //Send Response
  ChordMessage chordMessageRsp = ChordMessage ();
  virtualNode->PackStabilizeRsp(requestorNode, chordMessageRsp);
  packet-> AddHeader (chordMessageRsp);
  if (packet->GetSize())
  {
    NS_LOG_INFO ("Sending StabilizeRsp: "<<chordMessageRsp);
    SendPacket(packet, requestorNode->GetIpAddress(), requestorNode->GetPort());
  }
  return;
}

void
ChordIpv4::ProcessStabilizeRsp(ChordMessage chordMessage)
{
  //Extract info
  Ptr<ChordNode> requestorNode = chordMessage.GetRequestorNode();
  //Read payload
  Ptr<ChordNode> predecessorNode = chordMessage.GetStabilizeRsp().predecessorNode;

  //Find VNode
  Ptr<ChordVNode> virtualNode;
  bool ret = FindVNode(requestorNode->GetChordIdentifier(), virtualNode);
  if (ret == false)
  {
    //VNode does not exist here, drop packet
    return;
  }

  //Reset Successor if needed
  if (!virtualNode->GetChordIdentifier()->IsEqual(predecessorNode->GetChordIdentifier()))
  {
    //We need to reset successor and restabilize new successor
    Ptr<ChordNode> successorNode = Create<ChordNode> (predecessorNode);
    virtualNode->SetSuccessor(successorNode);
    virtualNode->SetRoutable(true);
    NS_LOG_INFO("Successor changed for VNode");
    //Trigger stabilization
    DoStabilize(virtualNode);
    DoFixFinger (virtualNode);
    return;
  }
  //Reset timestamp
  virtualNode->GetSuccessor()->SetTimestamp(Simulator::Now());
  //Synch successor list
  virtualNode->SynchSuccessorList (chordMessage.GetStabilizeRsp().successorList);
}

void
ChordIpv4::ProcessHeartbeatReq(ChordMessage chordMessage)
{
  Ptr<ChordNode> requestorNode = chordMessage.GetRequestorNode();

  //Read payload and get vnode identifier
  Ptr<ChordIdentifier> vNodeIdentifier = chordMessage.GetHeartbeatReq().predecessorIdentifier;

  //Find VNode
  Ptr<ChordVNode> virtualNode;
  bool ret = FindVNode(vNodeIdentifier, virtualNode);
  if (ret == false)
  {
    //VNode does not exist here, drop packet
    return;
  }

  //Reply to heartbeat
  Ptr<Packet> packet = Create<Packet> ();
  ChordMessage chordMessageRsp = ChordMessage ();
  virtualNode->PackHeartbeatRsp(requestorNode, chordMessageRsp);
  packet-> AddHeader (chordMessageRsp);
  if (packet->GetSize())
  {
    NS_LOG_INFO ("Sending StabilizeRsp: "<<chordMessageRsp);
    SendPacket(packet, requestorNode->GetIpAddress(), requestorNode->GetPort());
  }
  return;
}

void
ChordIpv4::ProcessHeartbeatRsp(ChordMessage chordMessage)
{
  //Extract info
  Ptr<ChordNode> requestorNode = chordMessage.GetRequestorNode();
  //Read payload -- Not needed for Heartbeats
  /*
  */
  //Find VNode
  Ptr<ChordVNode> virtualNode;
  bool ret = FindVNode(requestorNode->GetChordIdentifier(), virtualNode);
  if (ret == false)
  {
    //VNode does not exist here, drop packet
    return;
  }
  //Reset timestamp
  virtualNode->GetPredecessor()->SetTimestamp(Simulator::Now());
  //Synch predecessor list
  virtualNode->SynchPredecessorList (chordMessage.GetHeartbeatRsp().predecessorList);
}

void
ChordIpv4::ProcessFingerReq (ChordMessage chordMessage)
{
  Ptr<ChordNode> requestorNode = chordMessage.GetRequestorNode();
  Ptr<ChordIdentifier> requestedIdentifier = chordMessage.GetFingerReq().requestedIdentifier;
  if (m_vNodeMap.GetSize() == 0)
  {
    //No vNode exists as yet, drop this request.
    return;
  }
  Ptr<Packet> packet = Create<Packet> ();
  //Check if we are owner of requestedIdentifier
  Ptr<ChordVNode> virtualNode;
  bool ret = LookupLocal (requestedIdentifier, virtualNode);
  if (ret == true)
  {
    ChordMessage chordMessageRsp = ChordMessage ();
    virtualNode->PackFingerRsp (requestorNode, requestedIdentifier,chordMessageRsp);
    packet-> AddHeader (chordMessageRsp);
    //Send packet
    if (packet->GetSize())
    {
      NS_LOG_INFO("Sending FingerRsp: "<<chordMessageRsp);
      SendPacket(packet, requestorNode->GetIpAddress(), requestorNode->GetPort());
    }
    return;
  }
  //Could not resolve finger request, forward to successor
  packet->AddHeader(chordMessage);
  Ptr<ChordVNode> vNode;
  if (FindNearestVNode (requestedIdentifier, vNode) == true)
  {
    SendPacket (packet, vNode->GetSuccessor()->GetIpAddress(), vNode->GetSuccessor()->GetPort());
  }
  else
  {
    SendViaAnyVNode (packet);
  }
}

void
ChordIpv4::ProcessFingerRsp (ChordMessage chordMessage)
{
  //Extract info from packet
  Ptr<ChordNode> requestorNode = chordMessage.GetRequestorNode();
  Ptr<ChordIdentifier> requestedIdentifier = chordMessage.GetFingerRsp().requestedIdentifier;
  Ptr<ChordNode> fingerNode = chordMessage.GetFingerRsp().fingerNode;
  //Find virtual node which sent this message
  Ptr<ChordVNode> virtualNode;
  bool ret = FindVNode(requestorNode->GetChordIdentifier(), virtualNode);
  if (ret == true)
  { 
    //Save finger lookup in table
    Ptr<ChordNode> finger = Create<ChordNode> (fingerNode);
    virtualNode->GetFingerTable().UpdateNode(fingerNode); 
  }
}

void
ChordIpv4::HandleRequestTimeout (Ptr<ChordVNode> vNode, uint32_t transactionId)
{
  NS_LOG_FUNCTION_NOARGS ();
  //Find transaction
  Ptr<ChordTransaction> chordTransaction;
  if (vNode -> FindTransaction (transactionId, chordTransaction) == false)
  {
    //Transaction does not exist
    return;
  }
  //Retransmit and reschedule if needed
  if (chordTransaction->GetRetries() > chordTransaction->GetMaxRetries())
  {
    //Report failure
    if (chordTransaction->GetChordMessage().GetMessageType() == ChordMessage::JOIN_REQ)
    {
      NS_LOG_ERROR ("Join request failed!");
      NotifyVNodeFailure (vNode->GetVNodeName(), vNode->GetChordIdentifier());
      //Delete vNode
      DeleteVNode(vNode->GetChordIdentifier());
    }
    else if (chordTransaction->GetChordMessage().GetMessageType() == ChordMessage::LOOKUP_REQ)
    {
      NS_LOG_ERROR ("Lookup Request failed!");
      NotifyLookupFailure (chordTransaction->GetChordMessage().GetLookupReq().requestedIdentifier, chordTransaction->GetOriginator());
      //cancel transaction
      vNode->RemoveTransaction (chordTransaction->GetTransactionId());
    }
    return;
  }
  else
  {
    //Retransmit
    uint8_t retries = chordTransaction->GetRetries();
    chordTransaction->SetRetries (retries+1);
    Ptr<Packet> packet = Create<Packet> ();
    packet->AddHeader (chordTransaction->GetChordMessage());
    if (packet->GetSize())
    {
      NS_LOG_INFO ("Retransmission Req\n" << chordTransaction->GetChordMessage());
      SendPacket (packet, m_bootStrapIp, m_bootStrapPort);
    }
    //Reschedule
    //Start transaction timer
    EventId requestTimeoutId = Simulator::Schedule (chordTransaction->GetRequestTimeout(), &ChordIpv4::HandleRequestTimeout, this, vNode, transactionId);
    chordTransaction -> SetRequestTimeoutEventId (requestTimeoutId);
  }
}





bool
ChordIpv4::FindVNode (Ptr<ChordIdentifier> chordIdentifier, Ptr<ChordVNode>& virtualNode)
{
  NS_LOG_FUNCTION_NOARGS ();
  //Iterate VNode list and check if v node exists
  Ptr<ChordNode> chordNode; 
  if (m_vNodeMap.FindNode(chordIdentifier, chordNode) != true)
  {
    return false;
  }
  virtualNode = DynamicCast<ChordVNode>(chordNode);
  return true;
}

void
ChordIpv4::DeleteVNode (Ptr<ChordIdentifier> chordIdentifier)
{
  NS_LOG_FUNCTION_NOARGS ();

  m_vNodeMap.RemoveNode(chordIdentifier);
}

void
ChordIpv4::DeleteVNode (std::string vNodeName)
{
  m_vNodeMap.RemoveNode(vNodeName);
}


bool
ChordIpv4::LookupLocal (Ptr<ChordIdentifier> chordIdentifier, Ptr<ChordVNode>& virtualNode)
{
  NS_LOG_FUNCTION_NOARGS ();
  //Iterate VNode list and check if we are owner
  for(ChordNodeMap::iterator vNodeIter = m_vNodeMap.GetMap().begin(); vNodeIter != m_vNodeMap.GetMap().end(); vNodeIter++)
  {
    Ptr<ChordVNode> vNode = DynamicCast<ChordVNode>((*vNodeIter).second);
    if (vNode->GetPredecessor() == 0)
      continue;
    if (chordIdentifier->IsInBetween(vNode->GetPredecessor()->GetChordIdentifier (), vNode->GetChordIdentifier () ))
    {
      //Do not accept ownership if we have set ourselves as predecessor, but accept in bootstrap case. This means our predecessor recently died and we are waiting for someone to send us stabilize. 
      if (vNode->GetPredecessor()->GetChordIdentifier()->IsEqual(vNode->GetChordIdentifier()) && !vNode->GetPredecessor()->GetChordIdentifier()->IsEqual(vNode->GetSuccessor()->GetChordIdentifier()))
      {
        return false;
      }

      //We are the owner. Set virtualNode pointer and return success
      virtualNode = DynamicCast<ChordVNode>((*vNodeIter).second);
      return true;
    }
  }
  NS_LOG_INFO ("NOT OWNER, localIpAddress: "<<m_localIpAddress<<" listeningPort"<<m_listeningPort);
  //We are not owner, return failure
  return false;
}

bool
ChordIpv4::CheckOwnership (uint8_t* lookupKey, uint8_t lookupKeyBytes)
{
  NS_LOG_FUNCTION_NOARGS ();
  Ptr<ChordIdentifier> lookupIdentifier = Create<ChordIdentifier> (lookupKey, lookupKeyBytes);
  Ptr<ChordVNode> chordVNode;
  if (LookupLocal(lookupIdentifier , chordVNode) == true)
  {
    return true;
  }
  return false;
}

void
ChordIpv4::SendPacket (Ptr<Packet> packet, Ipv4Address destinationIp, uint16_t destinationPort)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_socket->SendTo (packet, 0, InetSocketAddress (destinationIp, destinationPort));
}

/*  Logic: We need to send packet via virtual node whose key is nearest to the requested key. Our aim is to minimize lookup hops.
 *
 *  Step 1: Iterate for all virtual nodes and maximize identifier for v-nodes which satisfies: vnode lies inBetween (0,key] <closestVNodeOnRight>
 *  Step 2: If none found in step 1, send to v-node with highest key number <closestVNodeOnLeft>
 */
bool
ChordIpv4::FindNearestVNode (Ptr<ChordIdentifier> targetIdentifier, Ptr<ChordVNode> &virtualNode)
{
  NS_LOG_FUNCTION_NOARGS ();
  Ptr<ChordNode> chordNode; 
  if (m_vNodeMap.FindNearestNode(targetIdentifier, chordNode) != true)
  {
    return false;
  }
  virtualNode = DynamicCast<ChordVNode>(chordNode);
  return true;
}

//TODO: Change logic to check health of vnode before sending
bool 
ChordIpv4::SendViaAnyVNode (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION_NOARGS ();
  //Find first v-node with valid successor
  for(ChordNodeMap::iterator vNodeIter = m_vNodeMap.GetMap().begin(); vNodeIter != m_vNodeMap.GetMap().end(); vNodeIter++)
  {
    Ptr<ChordVNode> vNode = DynamicCast<ChordVNode>((*vNodeIter).second);
    //Choose any node whose successor is not set as self
    if (!vNode->GetSuccessor()->GetChordIdentifier()->IsEqual(vNode->GetChordIdentifier()))
    {
      SendPacket (packet, vNode->GetSuccessor()->GetIpAddress(), vNode->GetSuccessor()->GetPort());
      return true;
    }
  }
  return false;
}

bool
ChordIpv4::RoutePacket (Ptr<ChordIdentifier> targetIdentifier, Ptr<Packet> packet)
{
  if (packet->GetSize())
  {
    Ptr<ChordVNode> vNode;
    //Choose best vNode
    if (FindNearestVNode (targetIdentifier, vNode) == true)
    {
      if (RouteViaFinger (targetIdentifier, vNode, packet) == true)
        return true;
    }
    else
    {
      return SendViaAnyVNode (packet);
    }
  }
  return false;
}

bool
ChordIpv4::RouteViaFinger (Ptr<ChordIdentifier> targetIdentifier, Ptr<ChordVNode> vNode, Ptr<Packet> packet)
{
  if (packet->GetSize())
  {
    Ptr<ChordNode> remoteNode;
    //Choose nearest finger
    if (vNode->GetFingerTable().FindNearestNode(targetIdentifier, remoteNode) == true)
    {
      SendPacket (packet, remoteNode->GetIpAddress(), remoteNode->GetPort());
      return true;
    }
    else
    {
      //Send to successor
      SendPacket (packet, vNode->GetSuccessor()->GetIpAddress(), vNode->GetSuccessor()->GetPort());
      return true;
    }
  }
  return false;
}



void
ChordIpv4::DoPeriodicStabilize()
{
  //Loop for all v-nodes
  for(ChordNodeMap::iterator vNodeIter = m_vNodeMap.GetMap().begin(); vNodeIter != m_vNodeMap.GetMap().end(); vNodeIter++)
  {
    Ptr<ChordVNode> vNode = DynamicCast<ChordVNode>((*vNodeIter).second);
    //Check if successor is alive. Shift successor if necessary. If all else fails, send CHORD_FAILURE to user and remove vNode
    //Compare timestamp and check if current successor has died
    if (vNode->GetSuccessor()->GetTimestamp().GetMilliSeconds() + m_stabilizeInterval.GetMilliSeconds() * m_maxMissedKeepAlives < Simulator::Now().GetMilliSeconds ())
    {
      //Successor has failed
      //Shift vNode successor
      if (vNode->ShiftSuccessor() == false)
      {
        //If this is last node and we are bootstrap node, do not report failure or remove this node. This can be only removed manually.
        if (isBootStrapNode && m_vNodeMap.GetSize() == 1)
        {
          //Reset successor as self
          vNode -> SetSuccessor (Create<ChordNode> (vNode));
          vNode -> SetRoutable (false);

          continue;
        }
 
        //No successor(s) in list, report failure and remove vNode
        NotifyVNodeFailure (vNode->GetVNodeName(), vNode->GetChordIdentifier());
        //Delete vNode
        DeleteVNode(vNode->GetChordIdentifier());
        if (m_vNodeMap.GetSize() == 0)
          break;
        else
          continue;
      }
    }
    //Fire stablize req
    DoStabilize (vNode);
  }
  //RescheduleTimer
  m_stabilizeTimer.Schedule (m_stabilizeInterval);

}

void
ChordIpv4::DoPeriodicHeartbeat()
{
  //Loop for all v-nodes
  for(ChordNodeMap::iterator vNodeIter = m_vNodeMap.GetMap().begin(); vNodeIter != m_vNodeMap.GetMap().end(); vNodeIter++)
  {
    Ptr<ChordVNode> vNode = DynamicCast<ChordVNode>((*vNodeIter).second);

    if (vNode->GetPredecessor()->GetTimestamp().GetMilliSeconds() + m_heartbeatInterval.GetMilliSeconds() * m_maxMissedKeepAlives < Simulator::Now().GetMilliSeconds ())
    {
      Ptr<ChordNode> oldPredecessorNode = vNode->GetPredecessor();
      //Predecessor has failed
      //Shift vNode predecessor
      if (vNode->ShiftPredecessor() == false)
      {
        //Reset predecessor as self node
        vNode -> SetPredecessor(Create<ChordNode> (vNode));
        continue;
      }
      else
      {
        //Predecessor shift success, trigger key space change
        NotifyVNodeKeyOwnership (vNode->GetVNodeName(), vNode->GetChordIdentifier(), vNode->GetPredecessor(), oldPredecessorNode->GetChordIdentifier());
      }
    }
    //Fire stablize req
    DoHeartbeat (vNode);
  }
  //RescheduleTimer
  m_heartbeatTimer.Schedule (m_heartbeatInterval);

}


void
ChordIpv4::DoPeriodicFixFinger ()
{
  for(ChordNodeMap::iterator vNodeIter = m_vNodeMap.GetMap().begin(); vNodeIter != m_vNodeMap.GetMap().end(); vNodeIter++)
  {
    Ptr<ChordVNode> vNode = DynamicCast<ChordVNode>((*vNodeIter).second);
    DoFixFinger (vNode);
  }
  //RescheduleTimer
  //Use random variable and introduce variance of 100ms

  Ptr<NormalRandomVariable> interval = CreateObject<NormalRandomVariable> ();
  interval->SetAttribute ("Mean", DoubleValue(m_fixFingerInterval.GetMilliSeconds()));
  interval->SetAttribute ("Variance", DoubleValue(100.0));
  m_fixFingerTimer.Schedule (MilliSeconds(interval->GetInteger()));
}

void
ChordIpv4::DoFixFinger (Ptr<ChordVNode> virtualNode)
{
  //Do not fix fingers for unstable v-nodes
  if (virtualNode->GetSuccessor()->GetChordIdentifier()->IsEqual(virtualNode->GetChordIdentifier()))
  {
    return;
  }
  //Remove stale entries from finger table; Remove even if finger request failed in last try. TODO: Use transactions for finger requests??
  virtualNode->GetFingerTable().Audit (m_fixFingerInterval);

  virtualNode->GetStats().fingersLookedUp = 0;

  for (std::vector<Ptr<ChordIdentifier> >::iterator fingerIter = virtualNode->GetFingerIdentifierList().begin(); fingerIter != virtualNode->GetFingerIdentifierList().end(); fingerIter++)
  {
    Ptr<ChordIdentifier> fingerIdentifier = *fingerIter;
    //Do not lookup local identifiers
    Ptr<ChordVNode> vNode;
    if (LookupLocal (fingerIdentifier, vNode) == true)
    {
      continue;
    }
    //Do not lookup fingers between successor and this node
    if (fingerIdentifier->IsInBetween(virtualNode->GetChordIdentifier(), virtualNode->GetSuccessor()->GetChordIdentifier()))
    {
      //Make routing entry
      Ptr<ChordNode> fingerNode = Create<ChordNode> (fingerIdentifier, virtualNode->GetSuccessor()->GetIpAddress(), virtualNode->GetSuccessor()->GetPort(), virtualNode->GetSuccessor()->GetApplicationPort(), virtualNode->GetSuccessor()->GetDHashPort());
      virtualNode->GetFingerTable().UpdateNode(fingerNode); 
      continue;
    }
    Ptr<Packet> packet = Create<Packet> ();
    ChordMessage chordMessage = ChordMessage ();
    virtualNode->PackFingerReq(fingerIdentifier, chordMessage); 
    packet->AddHeader (chordMessage);
    if (packet->GetSize())
    {
      virtualNode->GetStats().fingersLookedUp++;
      NS_LOG_INFO ("Sending FingerReq: " << chordMessage);
      //Route using successor for finger fixing
      SendPacket (packet, virtualNode->GetSuccessor()->GetIpAddress(), virtualNode->GetSuccessor()->GetPort());
      //RoutePacket (fingerIdentifier, packet);
    }
  }
}

void
ChordIpv4::DoStabilize(Ptr<ChordVNode> virtualNode)
{
  if (virtualNode->GetSuccessor()->GetChordIdentifier()->IsEqual(virtualNode->GetChordIdentifier()))
  {
    //Reset timestamp
    virtualNode->GetSuccessor()->SetTimestamp (Simulator::Now());
    return;
  }
  Ptr<Packet> packet = Create<Packet> ();
  ChordMessage chordMessage = ChordMessage ();
  virtualNode->PackStabilizeReq(chordMessage);
  packet-> AddHeader (chordMessage);
  if (packet->GetSize())
  {
    NS_LOG_INFO ("Sending StabilizeReq: " << chordMessage);
    SendPacket(packet, virtualNode->GetSuccessor()->GetIpAddress(), virtualNode->GetSuccessor()->GetPort());
  }
}

void
ChordIpv4::DoHeartbeat(Ptr<ChordVNode> virtualNode)
{
  if (virtualNode -> GetPredecessor()->GetChordIdentifier()->IsEqual(virtualNode->GetChordIdentifier()))
  {
    //Reset timestamp
    virtualNode->GetPredecessor()->SetTimestamp (Simulator::Now());
    return;
  }
  Ptr<Packet> packet = Create<Packet> ();
  ChordMessage chordMessage = ChordMessage ();
  virtualNode->PackHeartbeatReq(chordMessage);
  packet->AddHeader (chordMessage);
  if (packet->GetSize())
  {
    NS_LOG_INFO ("Sending HeartbeatReq: " << chordMessage);
    SendPacket(packet, virtualNode->GetPredecessor()->GetIpAddress(), virtualNode->GetPredecessor()->GetPort());
  }
  
}


bool
ChordIpv4::FindVNode (std::string vNodeName, Ptr<ChordVNode>& virtualNode)
{
  Ptr<ChordNode> chordNode; 
  if (m_vNodeMap.FindNode(vNodeName, chordNode) != true)
  {
    return false;
  }
  virtualNode = DynamicCast<ChordVNode>(chordNode);
  return true;
}


/******* Diagnostics  *******/
void
ChordIpv4::DumpVNodeInfo(std::string vNodeName, std::ostream &os)
{
  Ptr<ChordVNode> virtualNode;
  bool ret = FindVNode(vNodeName, virtualNode);
  if (ret == true)
  {
    //Dump stats
    os << "**** Info for VNode: " << virtualNode->GetVNodeName() << " ****\n";
    os << "Local IP: " << m_localIpAddress << "\n";
    os << "Local Port: " << m_listeningPort << "\n";
    os << "VNode Identifier: " << virtualNode->GetChordIdentifier() << "\n";
    os << "Successor Ip: " << virtualNode->GetSuccessor()->GetIpAddress() << "\n";
    os << "Successor Port: " << virtualNode->GetSuccessor()->GetPort() << "\n";
    os << "Successor Identifier: " << virtualNode->GetSuccessor()->GetChordIdentifier() << "\n";

    os << "Predecessor Ip: " << virtualNode->GetPredecessor()->GetIpAddress() << "\n";
    os << "Predecessor Port: " << virtualNode->GetPredecessor()->GetPort() << "\n";
    os << "Predecessor Identifier: " << virtualNode->GetPredecessor()->GetChordIdentifier() << "\n";

    virtualNode -> PrintSuccessorList (os);
    virtualNode -> PrintPredecessorList (os);
    virtualNode -> PrintFingerTable (os);
    //virtualNode -> PrintFingerIdentifierList (os);
    os << "Fingers actually looked up: " << virtualNode->GetStats().fingersLookedUp << "\n";
  }
  else
  {
    os << "No Such VNode Exists" << "\n";
  }
}

void
ChordIpv4::DumpDHashInfo (std::ostream &os)
{
  m_dHashIpv4->DumpDHashInfo(os);
}

void
ChordIpv4::FixFingers (std::string vNodeName)
{
  Ptr<ChordVNode> virtualNode;
  bool ret = FindVNode(vNodeName, virtualNode);
  if (ret == true)
  {
    DoFixFinger(virtualNode);
  }
}

void
ChordIpv4::FireTraceRing (std::string vNodeName)
{
  Ptr<ChordVNode> virtualNode;
  bool ret = FindVNode(vNodeName, virtualNode);
  if (ret == true)
  {
    NotifyTraceRing(virtualNode->GetVNodeName(), virtualNode->GetChordIdentifier());
    Ptr<Packet> packet = Create<Packet> ();
    ChordMessage chordMessageFwd = ChordMessage ();
    virtualNode->PackTraceRing(virtualNode, chordMessageFwd);
    packet-> AddHeader (chordMessageFwd);
    if (packet->GetSize())
    {
      NS_LOG_INFO ("Forwarding TraceRing: "<<chordMessageFwd);
      SendPacket(packet, virtualNode->GetSuccessor()->GetIpAddress(), virtualNode->GetSuccessor()->GetPort());
    }

  }
}

void
ChordIpv4::ProcessTraceRing(ChordMessage chordMessage)
{
  //Make Up-call
  NS_LOG_FUNCTION_NOARGS ();
  Ptr<ChordNode> requestorNode = chordMessage.GetRequestorNode();

  //Read payload and get vnode identifier
  Ptr<ChordIdentifier> vNodeIdentifier = chordMessage.GetTraceRing().successorIdentifier;

  //Find VNode
  Ptr<ChordVNode> virtualNode;
  bool ret = FindVNode(vNodeIdentifier, virtualNode);
  if (ret == false)
  {
    //VNode does not exist here, drop packet
    return;
  }
  NotifyTraceRing(virtualNode->GetVNodeName(), virtualNode->GetChordIdentifier());
  //Forward Trace Ring
  //Originator has to remove the packet
  if (requestorNode->GetChordIdentifier()->IsEqual(vNodeIdentifier))
  {
    return;
  }
  Ptr<Packet> packet = Create<Packet> ();
  ChordMessage chordMessageFwd = ChordMessage ();
  virtualNode->PackTraceRing(requestorNode, chordMessageFwd);
  packet-> AddHeader (chordMessageFwd);
  if (packet->GetSize())
  {
    NS_LOG_INFO ("Forwarding TraceRing: "<<chordMessageFwd);
    SendPacket(packet, virtualNode->GetSuccessor()->GetIpAddress(), virtualNode->GetSuccessor()->GetPort());
  }

}



} //namespace ns3
