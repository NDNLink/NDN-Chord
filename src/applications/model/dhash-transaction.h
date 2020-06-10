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

#ifndef DHASH_TRANSACTION_H
#define DHASH_TRANSACTION_H

#include "ns3/object.h"
#include "ns3/timer.h"
#include "dhash-message.h"
#include "dhash-connection.h"

namespace ns3 {

/**
 *  \ingroup chordipv4
 *  \class DHashTransaction
 *  \brief Class to store and operate on DHash Transaction
 */

class DHashTransaction : public Object
{

  public:

    enum Originator {
      APPLICATION = 1,
      DHASH = 2,
    };

    /**
     *  \brief Constructor
     *  \param transactionId
     *  \param Ptr to ChordIdentifier of DHashObject
     *  \param dHashMessage DHashMessage
     */
    DHashTransaction (uint32_t transactionId, Ptr<ChordIdentifier> objectIdentifier,DHashMessage dHashMessage);
    virtual ~DHashTransaction ();
    virtual void DoDispose ();
    //Storage
    //Retrieval
    /**
     *  \returns transactionId
     */
    uint32_t GetTransactionId ();
    /**
     *  \returns DHashMessage
     */
    DHashMessage GetDHashMessage ();
    /**
     *  \returns Ptr to ChordIdentifier of DHashObject
     */
    Ptr<ChordIdentifier> GetObjectIdentifier ();
    /**
     *  \brief Set flag to mark transaction as active
     */
    void SetActiveFlag (bool activeFlag);
    /**
     *  \brief Set Connection on which transaction is running
     */
    void SetDHashConnection (Ptr<DHashConnection> dHashConnection);
    /**
     *  \brief Set originator of transaction
     */
    void SetOriginator (DHashTransaction::Originator originator);
    /**
     *  \returns DHashTransaction::Originator
     */
    DHashTransaction::Originator GetOriginator ();
    /**
     *  \returns Active flag
     */
    bool GetActiveFlag ();
    /**
     *  \returns Ptr to DHashConnection
     */
    Ptr<DHashConnection> GetDHashConnection ();
  private:
    /**
     *  \cond
     */
    uint32_t m_transactionId;
    bool m_activeFlag;
    DHashMessage m_dHashMessage;
    Ptr<ChordIdentifier> m_objectIdentifier;
    Ptr<DHashConnection> m_dHashConnection;
    DHashTransaction::Originator m_originator;
    /**
     *  \endcond
     */
};

} //namespace ns3

#endif
