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

#include "chord-transaction.h"
#include "ns3/log.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ChordTransaction");

ChordTransaction::ChordTransaction (uint32_t transactionId, ChordMessage chordMessage, Time requestTimeout, uint8_t maxRequestRetries)
{
  NS_LOG_FUNCTION_NOARGS();
  m_transactionId = transactionId;
  m_chordMessage = chordMessage;
  m_requestTimeout = requestTimeout;
  m_maxRetries = maxRequestRetries;
  m_requestTimeoutEventId = EventId ();
}

ChordTransaction::~ChordTransaction ()
{
  //Use DoDispose()
}

void
ChordTransaction::DoDispose ()
{
  NS_LOG_FUNCTION_NOARGS();
  //cancel timer if running
  Simulator::Cancel(m_requestTimeoutEventId);
}

void
ChordTransaction::SetRetries (uint8_t retries)
{
  NS_LOG_FUNCTION_NOARGS();
  m_retries = retries;
}

void
ChordTransaction::SetMaxRetries (uint8_t maxRetries)
{
  NS_LOG_FUNCTION_NOARGS();
  m_maxRetries = maxRetries;
}

void
ChordTransaction::SetRequestTimeoutEventId (EventId eventId)
{
  NS_LOG_FUNCTION_NOARGS();
  m_requestTimeoutEventId = eventId;
}

uint32_t
ChordTransaction::GetTransactionId ()
{
  NS_LOG_FUNCTION_NOARGS();
  return m_transactionId;
}

uint8_t
ChordTransaction::GetRetries ()
{
  NS_LOG_FUNCTION_NOARGS();
  return m_retries;
}

uint8_t
ChordTransaction::GetMaxRetries ()
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_maxRetries;
}

Time
ChordTransaction::GetRequestTimeout ()
{
  NS_LOG_FUNCTION_NOARGS();
  return m_requestTimeout;

}

ChordMessage
ChordTransaction::GetChordMessage ()
{
  NS_LOG_FUNCTION_NOARGS();
  return m_chordMessage;
}

EventId
ChordTransaction::GetRequestTimeoutEventId ()
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_requestTimeoutEventId;
}

void 
ChordTransaction::SetOriginator (ChordTransaction::Originator originator)
{
  m_originator = originator;
}

ChordTransaction::Originator
ChordTransaction::GetOriginator ()
{
  return m_originator;
}

Ptr<ChordIdentifier>
ChordTransaction::GetRequestedIdentifier ()
{
  return m_requestedIdentifier;
}

void
ChordTransaction::SetRequestedIdentifier (Ptr<ChordIdentifier> requestedIdentifier)
{
  m_requestedIdentifier = requestedIdentifier;
}

} //namespace ns3
