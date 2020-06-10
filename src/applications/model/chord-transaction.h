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

#ifndef CHORD_TRANSACTION_H
#define CHORD_TRANSACTION_H

#include <vector>
#include "ns3/object.h"
#include "ns3/nstime.h"
#include "ns3/timer.h"
#include "chord-message.h"


namespace ns3 {

/**
 *  \ingroup chordipv4
 *  \class ChordTransaction
 *  \brief Class to store and operate on ChordTransaction
 */

class ChordTransaction : public Object
{
  public:

    enum Originator {
      APPLICATION = 1,
      DHASH = 2,
    };

    /**
     *  \brief Constructor
     *  \param transactionId
     *  \param chordMessage ChordMessage
     *  \param requestTimeout
     *  \param maxRequestRetries
     */
    ChordTransaction (uint32_t transactionId, ChordMessage chordMessage, Time requestTimeout, uint8_t maxRequestRetries);
    virtual ~ChordTransaction ();
    virtual void DoDispose ();


    //Storage
    /**
     *  \brief Set current retries of sending request ChordMessage
     *  \param retries
     */
    void SetRetries (uint8_t retries);
    /**
     *  \brief Set max retries of sending request ChordMessage
     *  \param retries
     */
    void SetMaxRetries (uint8_t maxRetries);
    /**
     *  \brief Set eventId of retransmission event
     *  \param eventid
     */
    void SetRequestTimeoutEventId (EventId eventId);
    /**
     *  \brief Set Requested Identifier
     *  \param requestedIdentifier ChordIdentifier
     */
    void SetRequestedIdentifier (Ptr<ChordIdentifier> requestedIdentifier);

    //Retrieval
    /**
     *  \returns Request timeout
     */
    Time GetRequestTimeout ();
    /**
     *  \returns Transaction Id
     */
    uint32_t GetTransactionId ();
    /**
     *  \returns Current Retries
     */
    uint8_t GetRetries ();
    /**
     *  \returns Max Retries
     */
    uint8_t GetMaxRetries ();
    /**
     *  \returns ChordMessage
     */
    ChordMessage GetChordMessage ();
    /**
     *  \returns Timeout eventId
     */
    EventId GetRequestTimeoutEventId ();
    /**
     *  \brief Sets request originator
     *  \param originator
     */
    void SetOriginator (ChordTransaction::Originator originator);
    /**
     *  \returns ChordTransaction::Originator
     */
    ChordTransaction::Originator GetOriginator ();
    /**
     *  \return Ptr to requested ChordIdentifier
     */
    Ptr<ChordIdentifier> GetRequestedIdentifier ();

  private:
    /**
     *  \cond
     */ 
    Ptr<ChordIdentifier> m_requestedIdentifier;
    Time  m_requestTimeout;
    EventId m_requestTimeoutEventId;
    uint32_t m_transactionId;
    uint8_t m_retries;
    uint8_t m_maxRetries;
    //RequestMessage for retransmissions
    ChordMessage m_chordMessage;
    //Originator of this transaction
    ChordTransaction::Originator m_originator;
    /**
     *  \endcond
     */
};    




} //namespace ns3



#endif //CHORD_TRANSACTION_H
