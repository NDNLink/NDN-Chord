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
#ifndef CHORD_IDENTIFIER_H
#define CHORD_IDENTIFIER_H

#include <stdint.h>
#include <ostream>
#include "ns3/ptr.h"
#include "ns3/object.h"
#include "ns3/buffer.h"

namespace ns3 {
/**
 *  \ingroup chordipv4
 *  \class ChordIdentifier
 *  \brief Class to store and operate on keys. 
 *  We assume keys are in little-endian format 
 */
class ChordIdentifier : public Object
{

  public:
  /**
   *  \brief Constructor 
   */
  ChordIdentifier ();
  /**
   *  \brief Constructor to store key
   *  \param Pointer to key array(identifier)
   *  \param numBytes Number of bytes in key
   */
  ChordIdentifier (uint8_t* key, uint8_t numBytes);
  /**
   *  \brief Copy Constructor
   *  \param identifier Ptr of ChordIdentifier to copy from
   */
  ChordIdentifier (Ptr<ChordIdentifier> identifier);
  /**
   *  \brief Copy Constructor
   *  \param identifier ChordIdentifier to copy from
   */
  ChordIdentifier (const ChordIdentifier& identifier);
  virtual ~ChordIdentifier ();
  virtual void DoDispose (void);

  /**
   *  \brief Tests ChordIndentifier for equality 
   *  \param identifier Ptr of ChordIdentifier to compare with
   *  \returns true if identifiers are equal, otherwise returns false
   */
  bool IsEqual (Ptr<ChordIdentifier> identifier); 
  /**
   *  \brief Tests if the ChordIdentifier is less than given ChordIdentifier
   *  \param identifier Ptr of ChordIdentifier to compare with
   *  \returns true if identifier is less than given value, otherwise returns false
   */
  bool IsLess (Ptr<ChordIdentifier> identifier);
  /**
   *  \brief Tests if the ChordIdentifier is greater than given ChordIdentifier
   *  \param identifier Ptr of ChordIdentifier to compare with
   *  \returns true if identifier is greater than given value, otherwise returns false
   */
  bool IsGreater (Ptr<ChordIdentifier> identifier);
  /**
   *  \brief Tests if the ChordIdentifier lies in between interval (identifierLow,identifierHigh] taken on a circular space.
   *  \param identifierLow Ptr of low ChordIdentifier 
   *  \param identifierHigh Ptr of high ChordIdentifier
   *  \returns true if identifier lies in given range, otherwise returns false 
   */
  bool IsInBetween (Ptr<ChordIdentifier> identifierLow, Ptr<ChordIdentifier> identifierHigh);
  /**
   *  \brief Adds power of two to identifier (identifier + 2^(powerOfTwo))
   *  \param powerOfTwo Power of two to add
   */
  void AddPowerOfTwo (uint16_t powerOfTwo);
  /**
   *  \returns Pointer to stored key array
   */
  uint8_t* GetKey (void);  
  /**
   *  \returns number of bytes in key array
   */
  uint8_t GetNumBytes (void);
  //Assignment

  /**
   *  \brief Stores key (identifier)
   *  \param key Pointer to key array (identifier)
   *  \param numBytes Number of bytes in key array
   */
  void SetKey (uint8_t* key, uint8_t numBytes);
  //Serialization
  /**
   *  \brief Serializes ChordIdentifier
   *  
   *  Packed Structure:
   *  \verbatim
      0 1 2 3 4 5 6 7 8 
      +-+-+-+-+-+-+-+-+
      |    numBytes   |
      +-+-+-+-+-+-+-+-+
      |               |
      :   key Array   :
      |               |
      +-+-+-+-+-+-+-+-+
      \endverbatim
   */
  void Serialize (Buffer::Iterator &start);
  /**
   *  \brief Deserializes packed ChordIdentifier
   *  \param start Buffer::Iterator of packed structure
   */
  uint32_t Deserialize (Buffer::Iterator &start);
  /**
   *  \returns Size of packed structure
   */
  uint32_t GetSerializedSize ();
  /**
   *  \brief Prints ChordIdentifier
   *  \param os Output Stream
   */
  void Print (std::ostream &os); 

  private:
  /**
   *  \cond
   */
  void CopyIdentifier(Ptr<ChordIdentifier> identifier);

  uint8_t *m_key;
  uint8_t m_numBytes;
  /**
   *  \endcond
   */
  //Operators
  friend bool operator < (const ChordIdentifier &identifierL, const ChordIdentifier &identifierR);
  friend bool operator == (const ChordIdentifier &identifierL, const ChordIdentifier identifierR);

}; //class ChordIdentifier

std::ostream& operator<< (std::ostream& os, Ptr<ChordIdentifier> const &identifier);
bool operator < (const ChordIdentifier &identifierL, const ChordIdentifier &identifierR);
bool operator == (const ChordIdentifier &identifierL, const ChordIdentifier identifierR);


} //namespace ns3

#endif //CHORD_IDENTIFIER_H
