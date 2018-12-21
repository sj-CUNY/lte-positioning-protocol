/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
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

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/address-utils.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/tcp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"

#include "lpp-server.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LppServerApplication");

NS_OBJECT_ENSURE_REGISTERED (LppServer);

TypeId
LppServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LppServer")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<LppServer> ()
    .AddAttribute ("ServerAddress", "Address of the server.",
                   AddressValue (),
                   MakeAddressAccessor (&LppServer::m_local),
                   MakeAddressChecker ())
    .AddAttribute ("Port", "Port on which we listen for incoming packets.",
                   UintegerValue (9),
                   MakeUintegerAccessor (&LppServer::m_port),
                   MakeUintegerChecker<uint16_t> ())
    .AddTraceSource ("Rx", "A packet has been received",
                     MakeTraceSourceAccessor (&LppServer::m_rxTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("RxWithAddresses", "A packet has been received",
                     MakeTraceSourceAccessor (&LppServer::m_rxTraceWithAddresses),
                     "ns3::Packet::TwoAddressTracedCallback")
  ;
  return tid;
}

LppServer::LppServer ()
{
  NS_LOG_FUNCTION (this);
  m_size = 0;
  m_dataSize = 0;
}

LppServer::~LppServer()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
  m_socket6 = 0;
}

void
LppServer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
LppServer::SetFill(std::string fill)
{
   NS_LOG_FUNCTION (this << fill);

  uint32_t dataSize = fill.size () + 1;

  if (dataSize != m_dataSize)
    {
      delete [] m_data;
      m_data = new uint8_t [dataSize];
      m_dataSize = dataSize;
    }

  memcpy (m_data, fill.c_str (), dataSize);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;

}


void 
LppServer::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  if (m_socket == 0)
    {
      TypeId tid = TcpSocketFactory::GetTypeId();
      m_socket = Socket::CreateSocket (GetNode (), tid);
   NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "Lpp server started with socket " << m_socket << " Ipaddress " << m_local << ":" << m_port);
     const Ipv4Address ipv4 = Ipv4Address::ConvertFrom (m_local);
      InetSocketAddress local = InetSocketAddress (ipv4, m_port);
      if (m_socket->Bind (local) == -1)
        {
          NS_FATAL_ERROR ("Failed to bind socket");
        }
   }

  if (m_socket6 == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");
      m_socket6 = Socket::CreateSocket (GetNode (), tid);
      Inet6SocketAddress local6 = Inet6SocketAddress (Ipv6Address::GetAny (), m_port);
      if (m_socket6->Bind (local6) == -1)
        {
          NS_FATAL_ERROR ("Failed to bind socket");
        }
   }

          int ret = m_socket->Listen ();
          NS_LOG_DEBUG (this << " Listen () return value= " << ret
                             << " GetErrNo= " << m_socket->GetErrno ()
                             << ".");

          NS_UNUSED (ret);
      m_socket->SetAcceptCallback (MakeCallback (&LppServer::ConnectionRequestCallback,
                                                        this),
                                          MakeCallback (&LppServer::NewConnectionCreatedCallback,
                                                        this));
      m_socket->SetCloseCallbacks (MakeCallback (&LppServer::NormalCloseCallback,
                                                        this),
                                          MakeCallback (&LppServer::ErrorCloseCallback,
                                                        this));
      m_socket->SetSendCallback (MakeCallback (&LppServer::SendCallback,
                                                      this));
 

  m_socket->SetRecvCallback (MakeCallback (&LppServer::HandleRead, this));
  m_socket6->SetRecvCallback (MakeCallback (&LppServer::HandleRead, this));
}

void 
LppServer::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0) 
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
  if (m_socket6 != 0) 
    {
      m_socket6->Close ();
      m_socket6->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
}

bool
LppServer::ConnectionRequestCallback (Ptr<Socket> socket,
                                               const Address &address)
{
  NS_LOG_FUNCTION (this << socket << address);
  return true; // Unconditionally accept the connection request.
}

void
LppServer::NewConnectionCreatedCallback (Ptr<Socket> socket,
                                                  const Address &address)
{
  NS_LOG_FUNCTION (this << socket << address);
  m_peer = address;
  socket->SetCloseCallbacks (MakeCallback (&LppServer::NormalCloseCallback,
                                           this),
                             MakeCallback (&LppServer::ErrorCloseCallback,
                                           this));
  socket->SetRecvCallback (MakeCallback (&LppServer::HandleRead,
                                         this));
  socket->SetSendCallback (MakeCallback (&LppServer::SendCallback,
                                         this));


  /*
   * A typical connection is established after receiving an empty (i.e., no
   * data) TCP packet with ACK flag. The actual data will follow in a separate
   * packet after that and will be received by ReceivedDataCallback().
   *
   * However, that empty ACK packet might get lost. In this case, we may
   * receive the first data packet right here already, because it also counts
   * as a new connection. The statement below attempts to fetch the data from
   * that packet, if any.
   */
 // LppHeader header;
//  header.SendLppPacket(socket, LppHeader::packet_type::REQUEST, LppHeader::packet_subtype::CAPABILITIES, address);
  HandleRead (socket);
}


void
LppServer::NormalCloseCallback (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  if (socket == m_socket)
    {
   }
}


void
LppServer::ErrorCloseCallback (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  if (socket == m_socket)
    {
    }
}


void
LppServer::SendCallback (Ptr<Socket> socket, uint32_t availableBufferSize)
{
  NS_LOG_FUNCTION (this << socket << availableBufferSize);

} // end of `void SendCallback (Ptr<Socket> socket, uint32_t availableBufferSize)`


void 
LppServer::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  Ptr<Packet> packet;
  Address from;
  Address localAddress;
  while ((packet = socket->RecvFrom (from)))
    {
      socket->GetSockName (localAddress);
      m_rxTrace (packet);
      m_rxTraceWithAddresses (packet, from, localAddress);
      if (InetSocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server received " << packet->GetSize () << " bytes from " <<
                       InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                       InetSocketAddress::ConvertFrom (from).GetPort ());
        }
      else if (Inet6SocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server received " << packet->GetSize () << " bytes from " <<
                       Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
                       Inet6SocketAddress::ConvertFrom (from).GetPort ());
        }
      HandlePacket(socket, packet, from);
      }
}

void 
LppServer::HandlePacket(Ptr<Socket> socket, Ptr<Packet> packet, Address from)
{
    LppHeader header;
    packet->PeekHeader(header);
    uint8_t flag = 0;
   if ( header.GetFlag() != 0)
	flag = header.GetFlag()-1;
   

    LppHeader::packet_type type = header.GetPacketType();
    LppHeader::packet_subtype subtype = header.GetPacketSubType();
    packet->RemoveAllPacketTags ();
    packet->RemoveAllByteTags ();
    switch(type)
    {
	case LppHeader::packet_type::ABORT:
	      m_socket->Close ();
      	      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
	      break;
	case LppHeader::packet_type::PROVIDE:
         switch(subtype)
         {
		case LppHeader::packet_subtype::CAPABILITIES:
                     header.SendLppPacket(socket, LppHeader::packet_type::REQUEST, LppHeader::packet_subtype::LOCATION_INFO, from);
		     break;
		case LppHeader::packet_subtype::LOCATION_INFO:
                     header.SendLppPacket(socket, LppHeader::packet_type::PROVIDE, LppHeader::packet_subtype::CERTIFICATE, from);
                     break;
                case LppHeader::packet_subtype::ASSISTANCE_DATA:
                case LppHeader::packet_subtype::CERTIFICATE:
                case LppHeader::packet_subtype::ERROR:
                case LppHeader::packet_subtype::FATAL: 
		     header.SendLppPacket(socket, LppHeader::packet_type::ABORT, LppHeader::packet_subtype::ERROR, from);  
    		      break;
          }
	  break;
	 case LppHeader::packet_type::REQUEST:
           switch(subtype)
           {
		case LppHeader::packet_subtype::CAPABILITIES:
		case LppHeader::packet_subtype::LOCATION_INFO:
                     header.SendLppPacket(socket, LppHeader::packet_type::ABORT, LppHeader::packet_subtype::ERROR, from);
                     break;
                case LppHeader::packet_subtype::ASSISTANCE_DATA:
                     header.SendLppPacket(socket, LppHeader::packet_type::PROVIDE, LppHeader::packet_subtype::ASSISTANCE_DATA, from, flag);
    		     break;
                case LppHeader::packet_subtype::CERTIFICATE:
                     header.SendLppPacket(socket, LppHeader::packet_type::REQUEST, LppHeader::packet_subtype::CAPABILITIES, from);
                     break;
                case LppHeader::ERROR:
                case LppHeader::FATAL: 
                     header.SendLppPacket(socket, LppHeader::packet_type::ABORT, LppHeader::packet_subtype::ERROR, from);
    		      break;
        

           }

     } 	

      if (InetSocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server sent " << packet->GetSize () << " bytes to " <<
                       InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                       InetSocketAddress::ConvertFrom (from).GetPort ());
        }
      else if (Inet6SocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server sent " << packet->GetSize () << " bytes to " <<
                       Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
                       Inet6SocketAddress::ConvertFrom (from).GetPort ());
        }

}

} // Namespace ns3
