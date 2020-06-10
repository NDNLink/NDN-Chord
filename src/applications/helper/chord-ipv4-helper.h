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

#ifndef CHORD_IPV4_HELPER_H
#define CHORD_IPV4_HELPER_H

//include(s)
#include <stdint.h>
#include "ns3/application-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"

namespace ns3 {

/* Helper class to install Chord protocol on a ns-3 node */

class ChordIpv4Helper
{
  public:

    /* Constructor(s) */

    /**
     * \brief Creates ChordIpv4 layer without DHash Layer
     * \param bootStrapIp Ipv4 Address of a known node in chord network
     * \param bootStrapPort Port of known boot strap node
     * \param localIpAddress Local Ipv4 to use with chord protocol. This allows multi-homing functionality
     * \param listeningPort Port used by chord protocol running on UDP
     * \param applicationPort Port used by application running on top of Chord. This port will be resolved during lookup.
     */
    ChordIpv4Helper(Ipv4Address bootStrapIp, uint16_t bootStrapPort, Ipv4Address localIpAddress,uint16_t listeningPort, uint16_t applicationPort);

    /**
     * \brief Creates ChordIpv4 layer with DHash Layer
     * \param bootStrapIp Ipv4 Address of a known node in chord network
     * \param bootStrapPort Port of known boot strap node
     * \param localIpAddress Local Ipv4 to use with chord protocol. This allows multi-homing functionality
     * \param listeningPort Port used by chord protocol running on UDP
     * \param applicationPort Port used by application running on top of Chord. This port will be resolved during lookup.
     * \param dhashPort Port used by DHash Layer to listen for incoming TCP requests
     */

    ChordIpv4Helper(Ipv4Address bootStrapIp, uint16_t bootStrapPort, Ipv4Address localIpAddress,uint16_t listeningPort, uint16_t applicationPort, uint16_t dHashPort);

    /**
     * Record an attribute to be set in each Application after it is is created.
     *
     * \param name the name of the attribute to set
     * \param value the value of the attribute to set
     */

    void SetAttribute(std::string name, const AttributeValue &value);

    /**
     * Create a ChordIpv4 Application on the specified Node.
     *
     * \param node The node on which to create the Application.  The node is
     *             specified by a Ptr<Node>.
     *
     * \returns An ApplicationContainer holding the Application created,
     */
    ApplicationContainer Install (Ptr<Node> node) const;

    /**
     * Create a ChordIpv4 Application on specified node
     *
     * \param nodeName The node on which to create the application.  The node
     *                 is specified by a node name previously registered with
     *                 the Object Name Service.
     *
     * \returns An ApplicationContainer holding the Application created.
     */
    ApplicationContainer Install (std::string nodeName) const;

    /**
     * \param c The nodes on which to create the Applications.  The nodes
     *          are specified by a NodeContainer.
     *
     * Create one ChordIpv4 Application on each of the Nodes in the
     * NodeContainer.
     *
     * \returns The applications created, one Application per Node in the 
     *          NodeContainer.
     */
    ApplicationContainer Install (NodeContainer c) const;

  private:
/**
 *  \internal
 */    
    Ptr<Application> InstallPriv (Ptr<Node> node) const;
    ObjectFactory m_factory;



}; //class ChordIpv4Helper

} //namespace ns3

#endif /* CHORD_HELPER_H */

