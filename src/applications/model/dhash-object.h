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

#ifndef DHASH_OBJECT_H
#define DHASH_OBJECT_H

#include <stdint.h>
#include <ostream>
#include "ns3/ptr.h"
#include "ns3/object.h"
#include "ns3/buffer.h"
#include "chord-identifier.h"

namespace ns3 {
/** 
 *  \ingroup chordipv4
 *  \class DHashObject
 */
class DHashObject : public Object
{

  public:
  /**
   *  \brief Constructor
   *  \param key Pointer to key array
   *  \param sizeOfKey Number of bytes of key (max 255)
   *  \param object Pointer to object byte array
   *  \param sizeOfObject Number of bytes in object array (max 2^32 - 1)
   */
  DHashObject(uint8_t *key,uint8_t sizeOfKey ,uint8_t *object,uint32_t sizeOfObject);
  /**
   *  \brief Constructor
   *  \param identifier ChordIdentifier of DHashObject
   *  \param object Pointer to object byte array
   *  \param sizeOfObject Number of bytes in object array (max 2^32 - 1)
   */
  DHashObject(Ptr<ChordIdentifier> identifier,uint8_t *object,uint32_t sizeOfObject);
  DHashObject();
  
  virtual ~DHashObject ();

  virtual void DoDispose (void);
  //Serialization
    /**
     *  \brief Packs DHashObject
     *  \param start Buffer::Iterator
     *
     *  \verbatim
        Packed Structure:
     
        0 1 2 3 4 5 6 7 8 
        +-+-+-+-+-+-+-+-+
        |               |
        :    object     :
        |   Identifier  |
        +-+-+-+-+-+-+-+-+
        |               |
        |               |
        |  sizeOfObject |
        |               |
        +-+-+-+-+-+-+-+-+
        |               |
        :  object array :
        |               |
        +-+-+-+-+-+-+-+-+
        \endverbatim    
    */
  void Serialize (Buffer::Iterator &start);
  /**
   *  \brief Unpacks DHashObject
   *  \param start Buffer::Iterator 
   */
  uint32_t Deserialize (Buffer::Iterator &start);
  /**
   *  \returns Size of packed structure
   */
  uint32_t GetSerializedSize ();
  void Print (std::ostream &os); 
  
  //Retrieval
  /**
   *  \returns Ptr to ChordIdentifier of DHashObject
   */
  Ptr<ChordIdentifier> GetObjectIdentifier(void);
  /**
   *  \returns Pointer to object array
   */
  uint8_t* GetObject(void);
  /**
   *  \returns Number of bytes in object array
   */
  uint32_t GetSizeOfObject(void);
    
  private:
  /**
   *  \cond
   */
  Ptr<ChordIdentifier> m_objectIdentifier;
  uint8_t  *m_object;
  uint32_t m_sizeOfObject;
  /**
   *  \endcond
   */

}; //class ChordIdentifier

std::ostream& operator<< (std::ostream& os, Ptr<DHashObject> const &DHashObject);


} //namespace ns3

#endif //DHASH_OBJECT_H
