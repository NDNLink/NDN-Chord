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
#include "dhash-object.h"

NS_LOG_COMPONENT_DEFINE ("DHashObject");

namespace ns3 {

DHashObject:: DHashObject(uint8_t *key,uint8_t sizeOfKey ,uint8_t *object,uint32_t sizeOfObject)
{
  NS_LOG_FUNCTION_NOARGS();

  //Save identifier
  m_objectIdentifier = Create<ChordIdentifier> (key, sizeOfKey);

  //Allocate memory to store the object
  m_object = (uint8_t *) malloc(sizeOfObject * sizeof (uint8_t));
  NS_ABORT_MSG_IF (m_object == 0,"DHashObject:: DHashObject() malloc failed");

  //Copy entire object to this area
  for (uint32_t i=0; i<sizeOfObject; i++)
  {
    m_object[i] = object[i];
  }

  //Save numBytes
  m_sizeOfObject = sizeOfObject;
}

DHashObject::DHashObject(Ptr<ChordIdentifier> identifier,uint8_t *object,uint32_t sizeOfObject)
{  
  NS_LOG_FUNCTION_NOARGS();

  m_objectIdentifier = identifier;

  //Allocate memory to store the object
  m_object = (uint8_t *) malloc(sizeOfObject * sizeof (uint8_t));
  NS_ABORT_MSG_IF (m_object == 0,"DHashObject:: DHashObject() malloc failed");

  //Copy entire object to this area
  for (uint32_t i=0; i<sizeOfObject; i++)
  {
    m_object[i] = object[i];
  }

  //Save numBytes
  m_sizeOfObject = sizeOfObject;

}


DHashObject::DHashObject ()
{
  m_object = 0;
  m_sizeOfObject = 0;
}

DHashObject::~DHashObject ()
{
  NS_LOG_FUNCTION_NOARGS();
}

void
DHashObject::DoDispose (void)
{
  NS_LOG_FUNCTION_NOARGS();
  //Free memory
  if (m_object != 0)
  {
    free (m_object);
    m_object = 0;
    m_sizeOfObject = 0; 
  }

}

uint8_t*
DHashObject::GetObject ()
{
  return m_object;
}

Ptr<ChordIdentifier>
DHashObject::GetObjectIdentifier()
{
  return m_objectIdentifier;
}

uint32_t
DHashObject::GetSizeOfObject()
{
  NS_LOG_FUNCTION_NOARGS();
  return  m_sizeOfObject;
}

void
DHashObject::Serialize (Buffer::Iterator &start)
{
  NS_LOG_FUNCTION_NOARGS();

  //Serialize the chord identifier
  m_objectIdentifier->Serialize(start);

  //Serialize the object
  start.WriteHtonU32 (m_sizeOfObject);

  for (uint32_t j=0 ; j<m_sizeOfObject; j++)
  {
    start.WriteU8 ( m_object[j]);
  }

}

uint32_t 
DHashObject::Deserialize (Buffer::Iterator &start)
{
  NS_LOG_FUNCTION_NOARGS();
  uint32_t j;


  m_objectIdentifier = Create<ChordIdentifier> ();
  m_objectIdentifier->Deserialize(start);

  m_sizeOfObject = start.ReadNtohU32();

  //Allocate memory for the object
  if (m_object == 0)
  {
    m_object = (uint8_t *) malloc(m_sizeOfObject * sizeof (uint8_t));
    NS_ABORT_MSG_IF ( m_object == 0,"  DHashObject::Deserialize malloc failed");
  }
  /* Retrieve object from buffer */
  for (j=0; j< m_sizeOfObject;j++)
  {
    m_object[j] = start.ReadU8 ();
  }  

  return GetSerializedSize ();
}

uint32_t
DHashObject::GetSerializedSize ()
{
  uint32_t size;
  size =  m_objectIdentifier->GetSerializedSize() + sizeof(uint32_t) + m_sizeOfObject;
  return size;
}

void 
DHashObject::Print (std::ostream &os) 
{
  m_objectIdentifier->Print (os);
  os << "Bytes: " << (uint16_t)  m_sizeOfObject << "\n";
  os << "Object: \n";
  os << "[ ";
  for (uint32_t j=0;j< m_sizeOfObject;j++)
  {
    os << std::hex << "0x" <<(uint32_t)  m_object[j] << " ";
  }
  os << std::dec << "]\n";
}

std::ostream& operator<< (std::ostream& os, Ptr<DHashObject> const &DHashObject)
{
  DHashObject->Print(os);
  return os;
}

} //namespace ns3

