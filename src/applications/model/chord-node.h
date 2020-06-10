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

#ifndef CHORD_NODE_H
#define CHORD_NODE_H

#include "ns3/object.h"
#include "ns3/nstime.h"
#include "ns3/buffer.h"
#include "ns3/ipv4-address.h"
#include "chord-identifier.h"

namespace ns3 {
#define IPV4_ADDRESS_SIZE 4

/**
 *  \ingroup chordipv4
 *  \class ChordNode
 *  \brief Class to store chord node information
 */

class ChordNode : public Object
{
  public:
    /**
     *  \brief Constructor
     *  \param identifier Ptr to ChordIdentifier of chord node
     *  \param name Name of chord node
     *  \param address Ipv4Address of chord node
     *  \param port Chord Protocol listening port (UDP)
     *  \param applicationPort Port advertised by application using Chord/DHash
     *  \param dhashPort DHash Protocol listening port (TCP)
     */
    ChordNode (Ptr<ChordIdentifier> identifier, Ipv4Address address, uint16_t port, uint16_t applicationPort, uint16_t dHashPort);
    /**
     *  \brief Constructor
     *  \param identifier Ptr to ChordIdentifier of chord node
     *  \param address Ipv4Address of chord node
     *  \param port Chord Protocol listening port (UDP)
     *  \param applicationPort Port advertised by application using Chord/DHash
     *  \param dhashPort DHash Protocol listening port (TCP)
     */
    ChordNode (Ptr<ChordIdentifier> identifier, std::string name, Ipv4Address address, uint16_t port, uint16_t applicationPort, uint16_t dHashPort);
    ChordNode ();
    /**
     *  \brief Copy constructor
     *  \param chordNode Ptr of ChordNode to copy from
     */
    ChordNode (const Ptr<ChordNode> chordNode);

    virtual ~ChordNode ();
    virtual void DoDispose (void);

    //storage
    /**
     *  \brief Sets timestamp of last update from remote ChordNode
     *  \param timestamp timestamp
     */
    void SetTimestamp (Time timestamp);
    /**
     *  \brief Sets the routable flag.
     *  \param routable routable flag
     *  This node will not be selected for routing if this flag is false
     */
    void SetRoutable (bool routable);

    //retrieval
    /** 
     *  \returns Ipv4Address of ChordNode
     */
    Ipv4Address GetIpAddress ();
    /**
     *  \returns port Chord Protocol listening port
     */
    uint16_t GetPort ();
    /**
     *  \returns applicationPort Port advertised by application running on top of Chord/DHash
     */
    uint16_t GetApplicationPort ();
    /**
     *  \returns dhashPort DHash Protocol listening port
     */
    uint16_t GetDHashPort ();
    /**
     *  \returns Ptr to ChordIdentifier of ChordNode
     */
    Ptr<ChordIdentifier> GetChordIdentifier();
    /**
     *  \returns Timestamp
     */
    Time GetTimestamp ();
    /**
     *  \returns Name of ChordNode
     */
    std::string GetName ();
    /**
     *  \returns Routable flag
     */
    bool GetRoutable ();
    //Serialization
    /**
     *  \brief Packs ChordNode
     *  \param start Buffer::Iterator
     *
     *  \verbatim
        Packed Structure:
        0 1 2 3 4 5 6 7 8 
        +-+-+-+-+-+-+-+-+
        |               |
        |               |
        |  Ipv4Address  |
        |               |
        +-+-+-+-+-+-+-+-+
        |               |
        |   Chord Port  |
        +-+-+-+-+-+-+-+-+
        |  Application  |
        |     Port      |
        +-+-+-+-+-+-+-+-+
        |               |
        |   DHash Port  |
        +-+-+-+-+-+-+-+-+
        \endverbatim
     */
    void Serialize (Buffer::Iterator &start);
    /**
     *  \brief Unpacks ChordNode
     *  \param start Buffer::Iterator
     */
    void Deserialize (Buffer::Iterator &start);
    /**
     *  \returns Size of packed ChordNode
     */
    uint32_t GetSerializedSize ();
    /**
     *  \brief Prints ChordNode information
     *  \param os Output stream
     */
    void Print (std::ostream &os);

  private:
    /**
     *  \cond
     */
    Ptr<ChordIdentifier> m_identifier;
    std::string m_name;
    Ipv4Address m_address;
    uint16_t m_port;
    uint16_t m_applicationPort;
    uint16_t m_dHashPort;
    Time m_timestamp;
    bool m_routable;
    /**
     *  \endcond
     */

}; //class ChordNode    

} //namespace ns3

#endif //CHORD_NODE_H
