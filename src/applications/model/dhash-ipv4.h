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

#ifndef DHASH_IPV4_H
#define DHASH_IPV4_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"
#include "ns3/timer.h"
#include "ns3/simulator.h"
#include "ns3/socket.h"
#include "ns3/inet-socket-address.h"
#include "ns3/callback.h"
#include "chord-identifier.h"
#include "dhash-message.h"
#include "dhash-object.h"
#include "dhash-connection.h"
#include "dhash-transaction.h"
#include <map>
#include <vector>

/* Static defines */
#define DEFAULT_CONNECTION_INACTIVITY_TIMEOUT 10000
#define DEFAULT_AUDIT_OBJECTS_TIMEOUT 600000

namespace ns3 {

class Socket;
class Packet;
class ChordIpv4;

/**
 *  \ingroup chordipv4
 *  \class DHashIpv4
 *  \brief DHash over Chord  
 */
class DHashIpv4 : public Object 
{

  public:
    DHashIpv4 ();
    virtual ~DHashIpv4 ();
    void DoDispose (void);
    /**
     *  \brief Start DHash layer
     */
    void Start (Ptr<ChordIpv4> chordIpv4);
    static TypeId GetTypeId (void);


    //Insert value under the key.
    //Application interface (DHASH Service User)
    /**
     *  \brief Insert DHash object into Chord Network
     *  \param key Pointer to key array (identifier)
     *  \param sizeOfKey Number of bytes in key array (max 255)
     *  \param object Pointer to object byte array
     *  \param sizeOfObject Number of bytes of object (max 2^32 - 1)
     *
     *  See ChordIpv4::Insert
     */
    void Insert(uint8_t *key,uint8_t sizeOfKey ,uint8_t *object,uint32_t sizeOfObject);    
     /**
     *  \brief Retrieves object from Chord/DHash (DHashIpv4) network represented by given key (identifier)
     *  \param key Pointer to key array (identifier)
     *  \param sizeOfKey Number of bytes in key (max 255)
     *
     *  See ChordIpv4::Retrieve
     */

    void Retrieve (uint8_t* key, uint8_t sizeOfKey);
    /**
     *  \brief See ChordIpv4::SetInsertSuccessCallback
     */
    void SetInsertSuccessCallback (Callback <void, uint8_t*, uint8_t, uint8_t*, uint32_t>);
    /**
     *  \brief See ChordIpv4::SetRetrieveSuccessCallback
     */
    void SetRetrieveSuccessCallback (Callback <void, uint8_t*, uint8_t, uint8_t*, uint32_t>);
    /**
     *  \brief See ChordIpv4::SetInsertFailureCallback
     */
    void SetInsertFailureCallback (Callback <void, uint8_t*, uint8_t, uint8_t*, uint32_t>); 
    /**
     *  \brief See ChordIpv4::SetRetrieveFailureCallback
     */
    void SetRetrieveFailureCallback (Callback <void, uint8_t*, uint8_t>);

    //Diagnostics interface
    /**
     *  \brief See ChordIpv4::DumpDHashInfo
     */
    void DumpDHashInfo (std::ostream &os);
    /**
     *  \cond
     */
    //Message processing methods
    void ProcessDHashMessage (Ptr<Packet> packet, Ptr<DHashConnection> dHashConnection);
 

    //TCP callbacks
    bool HandleConnectionRequest (Ptr<Socket> socket, const Address& address);
    void HandleAccept (Ptr<Socket> socket, const Address& address);
    void HandleOwnershipTrigger (uint8_t* vNodeKey, uint8_t vNodeBytes,uint8_t* predKey, uint8_t predBytes, uint8_t* oldPredKey, uint8_t oldPredBytes, Ipv4Address predIp, uint16_t predPort);
    void HandleClose (Ptr<Socket> socket);
  
    //Periodic processes
    void DoPeriodicAuditConnections ();
    void DoPeriodicAuditObjects ();

    
  private:
    typedef std::map<ChordIdentifier, Ptr<DHashObject> > DHashObjectMap;
    DHashObjectMap m_dHashObjectTable;
    typedef std::map<Ptr<Socket>, Ptr<DHashConnection> > DHashConnectionMap;
    DHashConnectionMap m_dHashConnectionTable;
    typedef std::map<uint32_t, Ptr<DHashTransaction> > DHashTransactionMap;
    DHashTransactionMap m_dHashTransactionTable;

    Ptr<ChordIpv4> m_chordApplication;
    Ipv4Address m_localIpAddress;
    uint16_t m_dHashPort;
    Ptr<Socket> m_socket;

    Time m_inactivityTimeout;
    Timer m_auditConnectionsTimer;
    Time m_auditObjectsTimeout;
    Timer m_auditObjectsTimer;

    uint32_t m_transactionId;
    //Callbacks
    Callback<void, uint8_t*, uint8_t, uint8_t*, uint32_t> m_insertSuccessFn;
    Callback<void, uint8_t*, uint8_t, uint8_t*, uint32_t> m_retrieveSuccessFn;
    Callback<void, uint8_t*, uint8_t, uint8_t*, uint32_t> m_insertFailureFn;
    Callback<void, uint8_t*, uint8_t> m_retrieveFailureFn;



    void SendDHashRequest (Ipv4Address ipAddress, uint16_t port, Ptr<DHashTransaction> dHashTransaction);

    //Connection Layer
    Ptr<DHashConnection> AddConnection (Ptr<Socket> socket, Ipv4Address ipAddress, uint16_t port);
    bool FindConnection (Ptr<Socket> m_socket, Ptr<DHashConnection> &dHashConnection);
    void RemoveConnection (Ptr<Socket> socket);
    bool FindConnection (Ipv4Address ipAddress, uint16_t port, Ptr<DHashConnection>& dHashConnection);


    //Object repository
    void AddObject (Ptr<DHashObject> object);
    bool FindObject (Ptr<ChordIdentifier> objectIdentifier, Ptr<DHashObject>& dHashObject);
    void RemoveObject (Ptr<ChordIdentifier> objectIdentifier);
    void TransferObject (Ptr<DHashObject> dHashObject, DHashTransaction::Originator originator, Ipv4Address ipAddress, uint16_t port);

    //Transaction Layer
    void AddTransaction (Ptr<DHashTransaction> dHashTransaction);
    bool FindTransaction (uint32_t transactionId, Ptr<DHashTransaction>& dHashTransaction);
    void RemoveTransaction (uint32_t transactionId);
    void RemoveActiveTransactions (Ptr<Socket> socket);

    //Notifications
    void NotifyInsertSuccess (Ptr<DHashObject> object);
    void NotifyRetrieveSuccess (Ptr<DHashObject> object);
    void NotifyFailure (Ptr<DHashTransaction> dHashTransaction);
    void NotifyInsertFailure (Ptr<DHashObject> object);
    void NotifyRetrieveFailure (Ptr<ChordIdentifier> objectIdentifier);


    //Packing methods
    void PackStoreReq (Ptr<DHashObject> dHashObject, DHashMessage& dHashMessage);
    void PackStoreRsp (uint32_t transactionId, uint8_t statusTag, Ptr<ChordIdentifier> objectIdentifier, DHashMessage& respMessage);
    void PackRetrieveReq (Ptr<ChordIdentifier> objectIdentifier, DHashMessage& dHashMessage);
    void PackRetrieveRsp (uint32_t transactionId, uint8_t statusTag, Ptr<DHashObject> dHashObject, DHashMessage& respMessage);


    //Processing methods
    void ProcessStoreReq (DHashMessage dHashMessage, Ptr<DHashConnection> dHashConnection);
    void ProcessStoreRsp (DHashMessage dHashMessage, Ptr<DHashConnection> dHashConnection);
    void ProcessRetrieveReq (DHashMessage dHashMessage, Ptr<DHashConnection> dHashConnection);
    void ProcessRetrieveRsp(DHashMessage dHashMessage, Ptr<DHashConnection> dHashConnection);


    //Lookup handle
    void HandleLookupFailure (uint8_t* lookupKey, uint8_t lookupKeyBytes);
    void HandleLookupSuccess (uint8_t* lookupKey, uint8_t lookupKeyBytes, Ipv4Address ipAddress, uint16_t port);
   

    uint32_t GetNextTransactionId ();
    /**
     *  \endcond
     */

};

} //namespace ns3

#endif //CHORD_IPV4_H
