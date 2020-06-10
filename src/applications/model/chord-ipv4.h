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

#ifndef CHORD_IPV4_H
#define CHORD_IPV4_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"
#include "ns3/timer.h"
#include "chord-identifier.h"
#include "chord-node.h"
#include "chord-vnode.h"
#include "chord-message.h"
#include "chord-node-table.h"
#include "dhash-ipv4.h"

/* Static defines */
// m = 160 bits i.e. 20 bytes
#define DEFAULT_CHORD_IDENTIFIER_BYTES 20
//Stabilize interval
#define DEFAULT_STABILIZE_INTERVAL 500
//Heartbeat interval
#define DEFAULT_HEARTBEAT_INTERVAL 500
//Fix Finger interval
#define DEFAULT_FIX_FINGER_INTERVAL 10000
// Max missed keep alives (Stabilize and Heartbeat)
#define DEFAULT_MAX_MISSED_KEEP_ALIVES 4
//Request timeout
#define DEFAULT_REQUEST_TIMEOUT 1000
//Max request retries
#define DEFAULT_MAX_REQUEST_RETRIES 3
//Max Successor List Size
#define DEFAULT_MAX_VNODE_SUCCESSOR_LIST_SIZE 8
//Max Predecessor List Size
#define DEFAULT_MAX_VNODE_PREDECESSOR_LIST_SIZE 8


namespace ns3 {

class Socket;
class Packet;

/**  
 *  \ingroup applications
 *  \defgroup chordipv4 ChordIpv4
 */

/**
 *  \ingroup chordipv4
 *  \brief Implementation of Chord/DHash DHT (http://pdos.csail.mit.edu/chord/)
 *
 *  Provides API for Configuring and Operating Chord/DHash DHT
 *
 */

class ChordIpv4 : public Application
{
  public:
    static TypeId GetTypeId (void);
  
    ChordIpv4 ();

    virtual ~ChordIpv4 ();



    /* Application interface (ChordIpv4 Service User) */
    
    /**
     *  \brief Create and Insert VirtualNode(ChordVNode) in Chord overlay network
     *  \param vNodeName Name of VirtualNode(ChordVNode). ChordIpv4 layer makes upcalls based on both VirtualNode(ChordVNode) name as well as key
     * \param key Pointer to key array (VirtualNode(ChordVNode) identifier)
     * \param keyBytes Number of bytes in key (max 255)
     *
     * On invocation, Join request is sent to Boot Strap node (default). If VirtualNode(ChordVNode)(s) already exist on local physical node, then Join Request is sent via node closest (in terms of identifier distance) to the VirtualNode(ChordVNode) identifier. 
     * A Chord VirtualNode(ChordVNode) (which is already part of the network), on receiving Join Request responds if the identifier of requesting node lies under its purview <lies in range (n.predecessor, n]> . If the identifier of requestor is out of purview then the request is forwarded to nearest remote node.
     *
     * On reception of Join Response, ChordIpv4 makes upcall to application if callback is function is set via SetJoinSuccessCallback. Please Note that this notification upcall is made even before initial stabilization (actual insertion into network) procedure. This upcall merely states that lookup involved with Join procedure was a success and possible successor was resolved.
     *
     * This request is retransmitted later in case response is not received on time (configurable). ChordIpv4 gives up retransmission after configurable number of retries.
     *
     * On failure to resolve successor (e.g. boot strap node is down) after a few retries, ChordIpv4 makes notification upcall to function registered via SetInsertFailureCallback by user. On Join failure, VirtualNode(ChordVNode) object is deleted from ChordIpv4 layer.
     */

    void InsertVNode (std::string vNodeName, uint8_t * key, uint8_t keyBytes);
    /**
     *  \brief Lookup owner node of an identifier in Chord Network
     *  \param key Pointer to key array (identifier)
     *  \param lookupKeyBytes Number of bytes in key (max 255)
     *
     *  On invocation, ChordIpv4 layer sends Lookup Request via VirtualNode(ChordVNode) closest to requested identifer (if it is not owner). 
     *  A VirtualNode(ChordVNode) which is owner of reqested identifier responds with its IP address and Application Port. On reception of Response, a notification upcall is made to the function registered via SetLookupSuccessCallback. 
     * This request is retransmitted later in case response is not received on time (configurable). ChordIpv4 gives up retransmission after configurable number of retries.
     * On failure to resolve identifier, ChordIpv4 makes a notification upcall to function registered via SetLookupFailureCallback by user.
     */

    void LookupKey (uint8_t * lookupKey, uint8_t lookupKeyBytes);
    /**
    *  \brief Check whether the any VirtualNode(ChordVNode) running on local physical node owns particular identifier.
    *  \param key Pointer to key array (identifier)
    *  \param lookupKeyBytes Number of bytes in key (max 255)
    *
    *  \returns true if local ChordIpv4 is owner of identifier, false if local ChordIpv4 is not owner.
    *
    *  On invocation, ChordIpv4 layer checks and returns ownership status of given identifier.
    */

    bool CheckOwnership (uint8_t * lookupKey, uint8_t lookupKeyBytes);
    /**
     *  \brief Remove VirtualNode(ChordVNode) from Chord Network
     *  \param vNodeName Name of VirtualNode(ChordVNode)
     *
     *  On invocation, ChordIpv4 layer removes given VirtualNode(ChordVNode) from the Chord Network. Leave Requests are sent to successor and predecessor of the given VirtualNode(ChordVNode), notifying them of removal. 
     *  On receiving a Leave Request, successor node sends back Leave Response. When Leave Response is received back, ChordIpv4 layer triggers DHash (DHashIpv4) layer to transfer stored DHashObjects belonging to given VirtualNode(ChordVNode) to its successor. 
     *
     */    
    void RemoveVNode (std::string vNodeName);
    /**
     *  \brief Registers Callback function for Join Success Notifications.
     *  \param joinSuccessFn Callback function
     */
    void SetJoinSuccessCallback (Callback <void, std::string , uint8_t*, uint8_t> joinSuccessFn);
    /**
     *  \brief Registers Callback function for Lookup Success Notifications.
     *  \param lookupSuccessFn This Callback is passed vNodeName, key and numBytes of key as parameters.
     */
    void SetLookupSuccessCallback (Callback <void, uint8_t*, uint8_t, Ipv4Address, uint16_t> lookupSuccessFn);
    /**
     *  \brief Registers Callback function for Lookup Failure Notifications.
     *  \param lookupFailureFn This Callback is passed lookup key, numBytes of lookup key, resolved IP and resolved port as parameters.
     */
    void SetLookupFailureCallback (Callback <void, uint8_t*, uint8_t> lookupFailureFn);
    /**
     *  \brief Registers Callback function for VirtualNode(ChordVNode) key space ownership change notifications.
     *  \param vNodeKeyOwnershipFn This Callback is passed vNodeName, vNode key, numBytes in vNode key, predecessor key, numBytes in predecessor key, oldPredecessor key, numBytes in oldPredecessor key, IP address of predecessor and application port of predecessor.
     *
     *  This upcall is made when predecessor of a VirtualNode(ChordVNode) changes (key space partitions).
     *  A similar upcall is made to DHash (DHashIpv4) layer, prompting it to shift stored objects to new owner.
     */
    void SetVNodeKeyOwnershipCallback (Callback <void, std::string, uint8_t*, uint8_t, uint8_t*, uint8_t, uint8_t*, uint8_t, Ipv4Address, uint16_t> vNodeKeyOwnershipFn);
    /**
     *  \brief Registers Callback function for Trace Ring packet notification (Diagnostics packet)
     *  \param traceRingFn This Callback is passed vNodeName, vNode key and numBytes in vNode key as parameters
     */
    void SetTraceRingCallback (Callback <void, std::string, uint8_t*, uint8_t> traceRingCallback);
    /**
     *  \brief Registers Callback function for VirtualNode(ChordVNode) failure notification
     *  \param vNodeFailureCallback This Callback is passed vNodeName, vNode key and numBytes in vNode key as parameters
     *  This upcall is made when all successors of a VirtualNode(ChordVNode) fail (stabilization failure) and it cannot stay in Chord Network anymore.
     *  Please note that a bootstrap node (a VirtualNode(ChordVNode) which bootstrapped itself and created a new chord network) will never make this upcall or leave Chord Network.
     */
    void SetVNodeFailureCallback (Callback <void, std::string, uint8_t*, uint8_t> vNodeFailureCallbackFn);
    //DHash (DHashIpv4) Callbacks
    /**
     *  \brief Registers Callback function for DHashObject Insert Success notification 
     *  \param insertSuccessFn This Callback is passed object key, numBytes in object key, object array and numBytes in object array as parameters.
     *
     *  This upcall is made when DHash (DHashIpv4) layer has succesfully stored the requested object in the Chord Network
     */
    void SetInsertSuccessCallback (Callback <void, uint8_t*, uint8_t, uint8_t*, uint32_t> insertSuccessFn);
    /**
     *  \brief Registers Callback function for DHashObject Retrieve Success notifications.
     *  \param retrieveSuccessFn This Callback is passed object key, numBytes in object key, object array and numBytes in object array as parameters.
     *
     *  This upcall is made when DHash (DHashIpv4) layer has successfully retrieved object represented by requested key (identifier).
     */
    void SetRetrieveSuccessCallback (Callback <void, uint8_t*, uint8_t, uint8_t*, uint32_t> retrieveSuccessFn);
    /**
     *  \brief Registers Callback function for DHashObject Insert Failure notifications.
     *  \param insertFailureFn This Callback is passed object key, numBytes in object key, object array and numBytes in object array as parameters.
     *
     *  This upcall is made when DHash (DHashIpv4) layer fails to store the requested object.
     */
    void SetInsertFailureCallback (Callback <void, uint8_t*, uint8_t, uint8_t*, uint32_t> insertFailureFn); 
    /**
     *  \brief Registers Callback function for DHashObject Retrieve Failure notifications.
     *  \param retrieveFailureFn This Callback is passed object key and numBytes in object key as parameters.
     *
     *  This upcall is made when DHash (DHashIpv4) layer fails to retrieve the object represented by requested key (identifier).
     */
    void SetRetrieveFailureCallback (Callback <void, uint8_t*, uint8_t> retrieveFailureFn);

    //DHash (DHashIpv4) application interface
    /**
     *  \cond
     */
    void DHashLookupKey (uint8_t * lookupKey, uint8_t lookupKeyBytes);
    void SetDHashLookupSuccessCallback (Callback <void, uint8_t*, uint8_t, Ipv4Address, uint16_t>);
    void SetDHashLookupFailureCallback (Callback <void, uint8_t*, uint8_t>);
    void SetDHashVNodeKeyOwnershipCallback (Callback <void, uint8_t*, uint8_t, uint8_t*, uint8_t, uint8_t*, uint8_t, Ipv4Address, uint16_t>);

    /**
     *  \endcond
     */
    //DHash (DHashIpv4) User interface
    /**
     *  \brief Inserts an object into the Chord/DHash (DHashIpv4) network.
     *  \param key Pointer to key array (identifier)
     *  \param sizeOfKey Number of bytes in key (max 255)
     *  \param object Pointer to object byte array
     *  \param sizeOfObject Number of bytes of object (max 2^32 - 1)
     *
     *  On invocation, DHash (DHashIpv4) layer triggers lookup request in Chord Network to locate owner of given object identifier. 
     *  On successful lookup, DHash (DHashIpv4) layer establishes TCP connection with owner node and transfers the given object. Application is then notified of success.
     *  Application is also notified if storage fails (due to failed lookup etc.)
     *  
     *  DHash (DHashIpv4) layer also runs periodic auditing of objects after configurable interval. During audit, the layer checks ownership of each stored object and transfers any misplaced objects.
     *
     *  For transfer of objects, TCP connection is reused if it already exists with remote node. TCP connection(s) are torn down after configurable inactivity interval. 
     *
     */
    void Insert (uint8_t *key, uint8_t sizeOfKey ,uint8_t *object,uint32_t sizeOfObject);
    /**
     *  \brief Retrieves object from Chord/DHash (DHashIpv4) network represented by given key (identifier)
     *  \param key Pointer to key array (identifier)
     *  \param sizeOfKey Number of bytes in key (max 255)
     *
     *  On invocation, DHash (DHashIpv4) layer triggers lookup request in Chord Network to locate owner of a given object identifier.
     *  On successful lookup, DHash (DHashIpv4) layer establishes TCP connection with owner and requests transfer of object. On successful retrieval, application is notified of success and given the object pointer.
     *  On failure to retrieve object (due to failed lookup, non-existent object etc.), application is notified of failure.
     *
     *  TCP connections are bounded by inactivity timer and failure is reported to application if object transfer stalls.
     */
    void Retrieve (uint8_t* key, uint8_t sizeOfKey);

    //Diagnostics Interface
    /**
     *  \brief Dumps VirtualNode(ChordVNode) information
     *  \param vNodeName VirtualNode(ChordVNode) name
     *  \param os Output stream
     */
    void DumpVNodeInfo (std::string vNodeName, std::ostream &os);
    /**
     *  \brief Fires Trace Ring packet
     *  \param vNodeName VirtualNode(ChordVNode) name
     *
     *  On invocation, a VirtualNode(ChordVNode) sends Trace Ring packet towards its successor. On reception of such a packet, application on that physical node is notified of reception and the packet is forwarded further. The originator of this packet removes it from the network once it has travelled around the ring. This packet can be used to trace the ring structure in simulator.
     */
    void FireTraceRing (std::string vNodeName);
    /**
     *  \brief Manually fix fingers
     *  \param vNodeName VirtualNode(ChordVNode) name
     */
    void FixFingers (std::string vNodeName);
    //DHash (DHashIpv4) Diagnostics 
    /**
     *  \brief Dumps DHash (DHashIpv4) layer information
     *  \param os Output Stream
     *
     *  Dumps information regarding stored DHashObject (s), active transactions and number of active TCP connections
     */
    void DumpDHashInfo (std::ostream &os);

  protected:
    virtual void DoDispose (void);
  private:
    virtual void StartApplication (void);
    virtual void StopApplication (void);

    /**
     *  \cond
     */
    bool m_dHashEnable;

    Ptr<Socket> m_socket;
    Ipv4Address m_bootStrapIp;
    uint16_t m_bootStrapPort;
    Ipv4Address m_localIpAddress;
    uint16_t m_listeningPort;
    uint16_t m_applicationPort;
    uint16_t m_dHashPort;
    Ptr<DHashIpv4> m_dHashIpv4;

    uint8_t m_maxVNodeSuccessorListSize;
    uint8_t m_maxVNodePredecessorListSize;

    bool isBootStrapNode;

    ChordNodeTable m_vNodeMap;

    //Timers
    Timer m_stabilizeTimer;
    Time m_stabilizeInterval;
    Timer m_heartbeatTimer;
    Time m_heartbeatInterval;
    Timer m_fixFingerTimer;
    Time m_fixFingerInterval;
  

    Time m_requestTimeout;
    Time m_dHashAuditObjectsTimeout;
    Time m_dHashInactivityTimeout;
    uint8_t m_maxMissedKeepAlives;

    uint8_t m_maxRequestRetries;

    void StabilizeTimerExpire();
    void HeartBeatTimerExpire();

    //Periodic task methods
    void DoPeriodicStabilize();
    void DoPeriodicHeartbeat();
    void DoPeriodicFixFinger();

    //Callbacks
    Callback<void, std::string, uint8_t*, uint8_t> m_joinSuccessFn;
    Callback<void, uint8_t*, uint8_t, Ipv4Address, uint16_t> m_lookupSuccessFn;
    Callback<void, uint8_t*, uint8_t> m_lookupFailureFn;
    Callback<void, std::string, uint8_t*, uint8_t, uint8_t*, uint8_t, uint8_t*, uint8_t, Ipv4Address, uint16_t> m_vNodeKeyOwnershipFn;
    Callback<void, std::string, uint8_t*, uint8_t> m_traceRingFn;
    Callback<void, std::string, uint8_t*, uint8_t> m_vNodeFailureFn;
    //DHash (DHashIpv4) callbacks
    Callback<void, uint8_t*, uint8_t, Ipv4Address, uint16_t> m_dHashLookupSuccessFn;
    Callback<void, uint8_t*, uint8_t> m_dHashLookupFailureFn;

    //dHash-user Interface callbacks
    Callback<void, uint8_t*, uint8_t, uint8_t*, uint32_t> m_insertSuccessFn;
    Callback<void, uint8_t*, uint8_t, uint8_t*, uint32_t> m_retrieveSuccessFn;
    Callback<void, uint8_t*, uint8_t, uint8_t*, uint32_t> m_insertFailureFn;
    Callback<void, uint8_t*, uint8_t> m_retrieveFailureFn;
    Callback <void, uint8_t*, uint8_t, uint8_t*, uint8_t, uint8_t*, uint8_t, Ipv4Address, uint16_t> m_dHashVNodeKeyOwnershipFn;

    //Upcall (notify) methods
    void NotifyJoinSuccess (std::string vNodeName, Ptr<ChordIdentifier> chordIdentifier);
    void NotifyLookupSuccess (Ptr<ChordIdentifier> lookupIdentifier, Ptr<ChordNode> resolvedNode, ChordTransaction::Originator originator);
    void NotifyLookupFailure (Ptr<ChordIdentifier> chordIdentifier, ChordTransaction::Originator originator);
    void NotifyVNodeKeyOwnership (std::string vNodeName, Ptr<ChordIdentifier> chordIdentifier, Ptr<ChordNode> predecessorNode, Ptr<ChordIdentifier> oldPredecessorIdentifier);
    void NotifyTraceRing (std::string vNodeName, Ptr<ChordIdentifier> chordIdentifier);
    void NotifyVNodeFailure (std::string vNodeName, Ptr<ChordIdentifier> chordIdentifier);
    //DHash (DHashIpv4) Notifications
    void NotifyDHashLookupSuccess (Ptr<ChordIdentifier> lookupIdentifier, Ipv4Address ipAddress, uint16_t port);
    void NotifyDHashLookupFailure (Ptr<ChordIdentifier> chordIdentifier);
    //DHash (DHashIpv4) User Notifications
    void NotifyInsertSuccess (uint8_t* key, uint8_t keyBytes, uint8_t* object, uint32_t objectBytes);
    void NotifyRetrieveSuccess (uint8_t* key, uint8_t keyBytes, uint8_t* object, uint32_t objectBytes);
    void NotifyInsertFailure (uint8_t* key, uint8_t keyBytes, uint8_t* object, uint32_t objectBytes);
    void NotifyRetrieveFailure (uint8_t* key, uint8_t keyBytes);

    //Message processing methods
    void ProcessUdpPacket (Ptr<Socket> socket);
    void ProcessJoinReq (ChordMessage chordMessage);
    void ProcessJoinRsp (ChordMessage chordMessage);
    void ProcessLeaveReq (ChordMessage chordMessage);
    void ProcessLeaveRsp (ChordMessage chordMessage);
    void ProcessLookupReq (ChordMessage chordMessage);
    void ProcessLookupRsp (ChordMessage chordMessage);
    void ProcessStabilizeReq (ChordMessage chordMessage);
    void ProcessStabilizeRsp (ChordMessage chordMessage);
    void ProcessHeartbeatReq (ChordMessage chordMessage);
    void ProcessHeartbeatRsp (ChordMessage chordMessage);
    void ProcessFingerReq (ChordMessage chordMessage);
    void ProcessFingerRsp (ChordMessage chordMessage);
    void ProcessTraceRing (ChordMessage chordMessage);


    void DoLookup (Ptr<ChordIdentifier> requestedIdentifier, ChordTransaction::Originator orginator);
    void DoStabilize (Ptr<ChordVNode> virtualNode);
    void DoHeartbeat (Ptr<ChordVNode> virtualNode);
    void DoFixFinger (Ptr<ChordVNode> virtualNode);

    bool FindVNode (Ptr<ChordIdentifier> chordIdentifier, Ptr<ChordVNode>& virtualNode);
    bool FindVNode (std::string vNodeName, Ptr<ChordVNode>& virtualNode);
    void DeleteVNode (Ptr<ChordIdentifier> chordIdentifier);
    void DeleteVNode (std::string vNodeName);
    bool LookupLocal (Ptr<ChordIdentifier> chordIdentifier, Ptr<ChordVNode>& virtualNode);
    
    //Send/Routing Methods
    void SendPacket (Ptr<Packet> packet, Ipv4Address destinationIp, uint16_t destinationPort);
    bool FindNearestVNode (Ptr<ChordIdentifier> targetIdentifier, Ptr<ChordVNode> &virtualNode);
    bool SendViaAnyVNode (Ptr<Packet> packet);
    bool RoutePacket (Ptr<ChordIdentifier> targetIdentifier, Ptr<Packet> packet);
    bool RouteViaFinger (Ptr<ChordIdentifier> targetIdentifier, Ptr<ChordVNode> vNode, Ptr<Packet> packet);

    //Timeouts
    void HandleRequestTimeout (Ptr<ChordVNode> chordVNode, uint32_t transactionId);

    //TODO: Trace call back(s)
    /**
     *  \endcond
     */

}; //class ChordIpv4

} //namespace ns3

#endif //CHORD_IPV4_H

