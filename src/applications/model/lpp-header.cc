/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 INESC Porto
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
 *
 * Author: Gustavo J. A. M. Carneiro  <gjc@inescporto.pt>
 */

#include <cmath>

#include "ns3/assert.h"
#include "ns3/log.h"

#include "lpp-header.h"
#define LPP_HEADER_SIZE 15
#define MAX_DATA 512
#define IPV4_ADDRESS_SIZE 4

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LppHeader");






NS_OBJECT_ENSURE_REGISTERED (LppHeader);

LppHeader::LppHeader ()
{
   m_data = 0;
   m_dataSize = 0;
}

LppHeader::~LppHeader ()
{
  if (m_dataSize != 0)
	delete[] m_data;
  m_dataSize = 0;
}

TypeId
LppHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LppHeader")
    .SetParent<Header> ()
    .SetGroupName ("Lpp")
    .AddConstructor<LppHeader> ()
  ;
  return tid;
}
TypeId
LppHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
LppHeader::GetSerializedSize (void) const
{

  return LPP_HEADER_SIZE;
}

void
LppHeader::Print (std::ostream &os) const
{
  /// \todo
}

void
LppHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteU8 (m_flag);
  i.WriteHtonU16 (m_packetLength);
  i.WriteHtonU16 ((uint16_t)m_packetType);
  i.WriteHtonU16 ((uint16_t)m_packetSubType);
  i.WriteHtonU64 (m_packetTime);
}

uint32_t
LppHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_flag =  i.ReadU8 ();
  m_packetLength  = i.ReadNtohU16 ();
  m_packetType = (packet_type)i.ReadNtohU16 ();
  m_packetSubType =(packet_subtype) i.ReadNtohU16 ();
  m_packetTime =  i.ReadNtohU64 ();
  return GetSerializedSize ();
}

void
LppHeader::SendLppPacket(Ptr<Socket> socket, LppHeader::packet_type type, LppHeader::packet_subtype subtype, Address from,  uint8_t flag)
{
  Ptr<Packet> p;
 if (type == LppHeader::packet_type::PROVIDE && subtype == LppHeader::packet_subtype::ASSISTANCE_DATA)
   {
     if (m_dataSize != 0)
     	delete[] m_data;

     m_dataSize = rand() % MAX_DATA; //!< Size of the sent packet
     m_data = new uint8_t[m_dataSize];
    
    random_data(m_data, m_dataSize);  

     p = Create<Packet> (m_data, m_dataSize);
   }
   else
   {
     p = Create<Packet>();
   }
     LppHeader header;
     header.SetPacketLength(LPP_HEADER_SIZE + m_dataSize); 
     header.SetPacketType(type);
     header.SetPacketSubType(subtype);   
     header.SetFlag(flag);   

     p->AddHeader(header);
    
     socket->SendTo (p, 0, from);
     
}
 
void 
LppHeader::random_data(uint8_t *s, int size)
{
  static const char alphanum[] = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYXabcdefghijklmnopqrstuvwxyz";
  for(int i = 0; i < size; ++i)
  {
	s[i] = alphanum[rand() % (sizeof(alphanum)-1)];
  }
//  s[size] = NULL;

}

}  // namespace olsr, ns3

