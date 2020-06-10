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

#include "chord-identifier.h"
#include "ns3/abort.h"
#include "ns3/log.h"
#include <stdlib.h>

NS_LOG_COMPONENT_DEFINE ("ChordIdentifier");

namespace ns3 {

ChordIdentifier::ChordIdentifier ()
{
  m_key = 0;
  m_numBytes = 0;
}

ChordIdentifier::ChordIdentifier (uint8_t* key, uint8_t numBytes)
{
  NS_LOG_FUNCTION_NOARGS();
  //Allocate memory to store key
  m_key = (uint8_t *) malloc(numBytes * sizeof (uint8_t));
  NS_ABORT_MSG_IF (m_key == 0,"ChordIdentifier::ChordIdentifier() malloc failed");
  //Copy entire key to this area
  for (int i=0; i<numBytes; i++)
  {
    m_key[i] = key[i];
  }
  //Save numBytes
  m_numBytes = numBytes;
}

ChordIdentifier::ChordIdentifier(Ptr<ChordIdentifier> identifier)
{
  NS_LOG_FUNCTION_NOARGS();
  CopyIdentifier(identifier);
}


ChordIdentifier::ChordIdentifier(const ChordIdentifier& identifier)
{
  NS_LOG_FUNCTION_NOARGS();
  Ptr<ChordIdentifier> chordIdentifier = const_cast<ChordIdentifier *>(&identifier);
  CopyIdentifier (chordIdentifier);
}

ChordIdentifier::~ChordIdentifier ()
{
  NS_LOG_FUNCTION_NOARGS();
  //Free memory
  if (m_key != 0)
  {
    free (m_key);
    m_numBytes= 0;
    m_key = 0;
  }
}

void
ChordIdentifier::DoDispose (void)
{
  NS_LOG_FUNCTION_NOARGS();
  //Free memory
  if (m_key != 0)
  {
    free (m_key);
    m_numBytes= 0;
    m_key = 0;
  }
}

void
ChordIdentifier::CopyIdentifier(Ptr<ChordIdentifier> identifier)
{
  NS_LOG_FUNCTION_NOARGS();
  uint8_t *key = identifier->GetKey();
  m_numBytes = identifier->GetNumBytes();

  //Allocate memory to store key
  m_key = (uint8_t *) malloc(m_numBytes * sizeof (uint8_t));
  NS_ABORT_MSG_IF (m_key == 0,"ChordIdentifier::ChordIdentifier() malloc failed");
  //Copy entire key to this area
  for (int i=0; i<m_numBytes; i++)
  {
    m_key[i] = key[i];
  }
}

bool
ChordIdentifier::IsEqual (Ptr<ChordIdentifier> identifier)
{
  NS_LOG_FUNCTION_NOARGS();
  //Retrieve key
  uint8_t* key= identifier->GetKey();
  //Check for contents
  for (int i= 0; i<m_numBytes; i++)
  {
    if (key[i] != m_key[i])
      return false;
  }
  //Keys are equal, return true
  return true;
}

bool
ChordIdentifier::IsLess (Ptr<ChordIdentifier> identifier)
{
  NS_LOG_FUNCTION_NOARGS();
  //Retrieve key
  uint8_t* key= identifier->GetKey();
  //Compare two keys byte by byte
  for (int i= m_numBytes-1; i>=0; i--)
  {
    if (m_key[i] < key[i])
      return true;
    else if (m_key[i] > key[i])
      return false;
  }
  //Keys are equal, return false
  return false;
}

bool
ChordIdentifier::IsGreater (Ptr<ChordIdentifier> identifier)
{
  NS_LOG_FUNCTION_NOARGS();
  //Retrieve key
  uint8_t* key= identifier->GetKey();
  //Compare two keys byte by byte
  for (int i= m_numBytes-1; i>=0; i--)
  {
    if (m_key[i] > key[i])
      return true;
    else if (m_key[i] < key[i])
      return false;
  }
  //Keys are equal, return false
  return false;
}

bool
ChordIdentifier::IsInBetween (Ptr<ChordIdentifier> identifierLow, Ptr<ChordIdentifier> identifierHigh)
{
  NS_LOG_FUNCTION_NOARGS();
  //Check for existence in between range (keyLow,keyHigh], taken on circular identifier space, in clockwise direction

  //Check for wrap-around
  if (identifierHigh->IsGreater(identifierLow))
  {
    //No wrap-around
    if (!IsGreater(identifierLow))
      return false;

    if (IsLess(identifierHigh) || IsEqual(identifierHigh))
      return true;
    else 
      return false;
  }
  else if (identifierHigh->IsLess(identifierLow))
  {
    //Wrap-around is there
    //Either key lies on left of 0 or on right. So check for both scenarios
    if (IsGreater(identifierHigh) || IsEqual(identifierHigh))
    {
      //Key lies on left of 0?
      if (IsGreater(identifierLow))
        return true;
      else 
        return false;
    }
    //Check if key lies on right of 0 and in between
    if (IsLess(identifierHigh) || IsEqual(identifierHigh))
    {
      //Key lies on right of 0?
      if (IsLess(identifierLow))
        return true;
      else 
        return false;
    }
  }
  else if (identifierHigh->IsEqual(identifierLow))
  {
    //Everything is in between! (except keyHigh)
    if (!IsEqual(identifierHigh))
      return true;
    return false;
  }
  NS_LOG_ERROR ("Something is seriously is wrong..");
  //Something is seriously wrong, cannot be here
  return false;
}

void
ChordIdentifier::AddPowerOfTwo (uint16_t powerOfTwo)
{
  uint8_t powZero = 0x01;
  //Find byte position
  uint8_t position = powerOfTwo / 8;
  NS_ASSERT (position < m_numBytes);
  uint8_t shift = powerOfTwo % 8;
  //Add power
  uint8_t prevVal = m_key[position];
  m_key[position] = m_key[position] + (powZero << shift);
  //if carry is there
  while ((m_key[position] < prevVal) && (position <= (m_numBytes-1)))
  {
    position++;
    prevVal = m_key[position]; 
    m_key[position] = m_key[position] + 0x01;
  }
}

uint8_t* 
ChordIdentifier::GetKey (void)
{
  NS_LOG_FUNCTION_NOARGS();
  return m_key;
}

uint8_t
ChordIdentifier::GetNumBytes()
{
  NS_LOG_FUNCTION_NOARGS();
  return m_numBytes;
}

void 
ChordIdentifier::SetKey(uint8_t* key, uint8_t numBytes)
{
  NS_LOG_FUNCTION_NOARGS();

  if (m_key != 0)
  {
    //Free previously stored key
    free (m_key);
    m_key = 0;
  }
  //Allocate memory to store key
  m_key = (uint8_t *) malloc(numBytes * sizeof (uint8_t));
  NS_ABORT_MSG_IF (m_key == 0,"ChordIdentifier::ChordIdentifier() malloc failed");
  //Copy entire key to this area
  for (int i=0; i<numBytes; i++)
  {
    m_key[i] = key[i];
  }
  //Save numBytes
  m_numBytes = numBytes;
}

void
ChordIdentifier::Serialize (Buffer::Iterator &start)
{
  NS_LOG_FUNCTION_NOARGS();
  start.WriteU8 (m_numBytes);
  uint8_t j;
  for (j=0 ; j<m_numBytes; j++)
  {
    start.WriteU8 (m_key[j]);
  }
}

uint32_t 
ChordIdentifier::Deserialize (Buffer::Iterator &start)
{
  NS_LOG_FUNCTION_NOARGS();
  uint8_t j;

  m_numBytes = start.ReadU8 ();
  //Allocate memory
  if (m_key == 0)
  {
    m_key = (uint8_t *) malloc(m_numBytes * sizeof (uint8_t));
    NS_ABORT_MSG_IF (m_key == 0,"ChordIdentifier::ChordIdentifier() malloc failed");
  }
  else
  {
    //return 0;
  }
  /* Retrieve key from buffer */
  for (j=0; j<m_numBytes;j++)
  {
    m_key[j] = start.ReadU8 ();
  }  

  return GetSerializedSize ();
}

uint32_t
ChordIdentifier::GetSerializedSize ()
{
  uint32_t size;
  size = GetNumBytes() + sizeof (uint8_t);
  return size;

}

void 
ChordIdentifier::Print (std::ostream &os) 
{
  os << "Bytes: " << (uint16_t) m_numBytes << "\n";
  os << "Key: \n";
  os << "[ ";
  for (uint8_t j=0;j<m_numBytes;j++)
  {
    os << std::hex << "0x" <<(uint16_t) m_key[j] << " ";
  }
  os << std::dec << "]\n";
}

std::ostream& operator<< (std::ostream& os, Ptr<ChordIdentifier> const &identifier)
{
  identifier->Print(os);
  return os;
}

bool operator== (ChordIdentifier &chordIdentifierL, ChordIdentifier &chordIdentifierR)
{
  Ptr<ChordIdentifier> identifierL = const_cast<ChordIdentifier *> (&chordIdentifierL);
  Ptr<ChordIdentifier> identifierR = const_cast<ChordIdentifier *> (&chordIdentifierR);
  if (identifierL->IsEqual(identifierR))
    return true;
  else
    return false;
}
bool operator < (const ChordIdentifier &chordIdentifierL, const ChordIdentifier &chordIdentifierR)
{
  Ptr<ChordIdentifier> identifierL = const_cast<ChordIdentifier *> (&chordIdentifierL);
  Ptr<ChordIdentifier> identifierR = const_cast<ChordIdentifier *> (&chordIdentifierR);
  if (identifierL->IsLess(identifierR))
    return true;
  else
    return false;
}

} //namespace ns3
