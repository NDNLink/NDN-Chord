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

#include "dhash-transaction.h"

namespace ns3 {

DHashTransaction::DHashTransaction (uint32_t transactionId, Ptr<ChordIdentifier> objectIdentifier,DHashMessage dHashMessage)
{
  m_transactionId = transactionId;
  m_dHashMessage = dHashMessage;
  m_objectIdentifier = objectIdentifier;
  m_activeFlag = false;
}

DHashTransaction::~DHashTransaction ()
{
  //Use DoDispose()
}

void
DHashTransaction::DoDispose()
{
}

void
DHashTransaction::SetActiveFlag (bool activeFlag)
{
  m_activeFlag = activeFlag;
}

uint32_t
DHashTransaction::GetTransactionId ()
{
  return m_transactionId;
}

DHashMessage
DHashTransaction::GetDHashMessage ()
{
  return m_dHashMessage;
}

Ptr<ChordIdentifier>
DHashTransaction::GetObjectIdentifier ()
{
  return m_objectIdentifier;
}

bool
DHashTransaction::GetActiveFlag()
{
  return m_activeFlag;
}

Ptr<DHashConnection> 
DHashTransaction::GetDHashConnection ()
{
  return m_dHashConnection;
}

DHashTransaction::Originator
DHashTransaction::GetOriginator ()
{
  return m_originator;
}

void
DHashTransaction::SetOriginator (DHashTransaction::Originator originator)
{
  m_originator = originator;
}
void
DHashTransaction::SetDHashConnection (Ptr<DHashConnection> dHashConnection)
{
  m_dHashConnection = dHashConnection;
}

}
