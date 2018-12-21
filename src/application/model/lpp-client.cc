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
#include <ns3/double.h>
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "lpp-client.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LppClientApplication");

NS_OBJECT_ENSURE_REGISTERED (LppClient);


TypeId
LppClient::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LppClient")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<LppClient> ()
    .AddAttribute ("MaxPackets", 
                   "The maximum number of packets the application will send",
                   UintegerValue (100),
                   MakeUintegerAccessor (&LppClient::m_count),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Interval", 
                   "The time to wait between packets",
                   TimeValue (Seconds (1.0)),
                   MakeTimeAccessor (&LppClient::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("RemoteAddress", 
                   "The destination Address of the outbound packets",
                   AddressValue (),
                   MakeAddressAccessor (&LppClient::m_peerAddress),
                   MakeAddressChecker ())
    .AddAttribute ("RemotePort", 
                   "The destination port of the outbound packets",
                   UintegerValue (30),
                   MakeUintegerAccessor (&LppClient::m_peerPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("PacketSize", "Size of echo data in outbound packets",
                   UintegerValue (100),
                   MakeUintegerAccessor (&LppClient::SetDataSize,
                                         &LppClient::GetDataSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&LppClient::m_txTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("Rx", "A packet has been received",
                     MakeTraceSourceAccessor (&LppClient::m_rxTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("TxWithAddresses", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&LppClient::m_txTraceWithAddresses),
                     "ns3::Packet::TwoAddressTracedCallback")
    .AddTraceSource ("RxWithAddresses", "A packet has been received",
                     MakeTraceSourceAccessor (&LppClient::m_rxTraceWithAddresses),
                     "ns3::Packet::TwoAddressTracedCallback")
  ;
  return tid;
}

LppClient::LppClient ()
{
  NS_LOG_FUNCTION (this);
  m_state = STOPPED;
  m_sent = 0;
  m_socket = 0;
  m_sendEvent = EventId ();
  m_data = 0;
  m_dataSize = 0;
}

LppClient::~LppClient()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;

  delete [] m_data;
  m_data = 0;
  m_dataSize = 0;
}

void 
LppClient::SetRemote (Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = ip;
  m_peerPort = port;
}

void 
LppClient::SetRemote (Address addr)
{
  NS_LOG_FUNCTION (this << addr);
  m_peerAddress = addr;
}

void
LppClient::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}
void
LppClient::restart()
{
  TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");
  LppHeader header;
  //Address from = InetSocketAddress (Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort);
 // m_sendEvent = Simulator::Schedule (Seconds(0.1), &LppHeader::SendLppPacket, this);
//  header.SendLppPacket(m_socket, LppHeader::packet_type::REQUEST, LppHeader::packet_subtype::CERTIFICATE, from, startTime);

  m_state = CONNECTING;
  m_socket = Socket::CreateSocket (GetNode (), tid);
  SetCallbacks();
  m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
}
void 
LppClient::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      SetCallbacks();    
      if (Ipv4Address::IsMatchingType(m_peerAddress) == true)
        {
          if (m_socket->Bind () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_state = CONNECTING;
          m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
  NS_LOG_FUNCTION ("Connecting to " <<  m_peerAddress << ":" << m_peerPort  );
        }
      else if (Ipv4Address::IsMatchingType(m_peerAddress) == true)
        {
          if (m_socket->Bind6 () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_state = CONNECTING;
          m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
        }
      else if (InetSocketAddress::IsMatchingType (m_peerAddress) == true)
        {
          if (m_socket->Bind () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_state = CONNECTING;
          m_socket->Connect (m_peerAddress);
        }
      else if (InetSocketAddress::IsMatchingType (m_peerAddress) == true)
        {
          if (m_socket->Bind6 () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_state = CONNECTING;
          m_socket->Connect (m_peerAddress);
        }
      else
        {
          NS_ASSERT_MSG (false, "Incompatible address type: " << m_peerAddress);
        }

    }

       //ScheduleTransmit (Seconds (0.5));
}
void
LppClient::SetCallbacks()
{

      m_socket->SetConnectCallback (MakeCallback (&LppClient::ConnectionSucceededCallback,
                                                  this),
                                    MakeCallback (&LppClient::ConnectionFailedCallback,
                                                  this));
      m_socket->SetCloseCallbacks (MakeCallback (&LppClient::NormalCloseCallback,
                                                 this),
                                   MakeCallback (&LppClient::ErrorCloseCallback,
                                                 this));

       m_socket->SetRecvCallback (MakeCallback (&LppClient::HandleRead, this));
       m_socket->SetAttribute ("MaxSegLifetime", DoubleValue (0.02)); // 20 ms.
     
}
void 
LppClient::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0) 
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
      m_socket = 0;
    }

  Simulator::Cancel (m_sendEvent);
}

void 
LppClient::SetDataSize (uint32_t dataSize)
{
  NS_LOG_FUNCTION (this << dataSize);

  //
  // If the client is setting the echo packet data size this way, we infer
  // that she doesn't care about the contents of the packet at all, so 
  // neither will we.
  //
  delete [] m_data;
  m_data = 0;
  m_dataSize = 0;
  m_size = dataSize;
}

uint32_t 
LppClient::GetDataSize (void) const
{
  NS_LOG_FUNCTION (this);
  return m_size;
}

void 
LppClient::SetFill (std::string fill)
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
LppClient::SetFill (uint8_t fill, uint32_t dataSize)
{
  NS_LOG_FUNCTION (this << fill << dataSize);
  if (dataSize != m_dataSize)
    {
      delete [] m_data;
      m_data = new uint8_t [dataSize];
      m_dataSize = dataSize;
    }

  memset (m_data, fill, dataSize);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;
}

void 
LppClient::SetFill (uint8_t *fill, uint32_t fillSize, uint32_t dataSize)
{
  NS_LOG_FUNCTION (this << fill << fillSize << dataSize);
  if (dataSize != m_dataSize)
    {
      delete [] m_data;
      m_data = new uint8_t [dataSize];
      m_dataSize = dataSize;
    }

  if (fillSize >= dataSize)
    {
      memcpy (m_data, fill, dataSize);
      m_size = dataSize;
      return;
    }

  //
  // Do all but the final fill.
  //
  uint32_t filled = 0;
  while (filled + fillSize < dataSize)
    {
      memcpy (&m_data[filled], fill, fillSize);
      filled += fillSize;
    }

  //
  // Last fill may be partial
  //
  memcpy (&m_data[filled], fill, dataSize - filled);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;
}

void 
LppClient::ScheduleTransmit (Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent bytes to " <<
                   Ipv4Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort);
 
  m_sendEvent = Simulator::Schedule (dt, &LppClient::Send, this);
}

void 
LppClient::Send (void)
{
  NS_LOG_FUNCTION (this);


  Ptr<Packet> p;
  if (m_dataSize)
    {
      //
      // If m_dataSize is non-zero, we have a data buffer of the same size that we
      // are expected to copy and send.  This state of affairs is created if one of
      // the Fill functions is called.  In this case, m_size must have been set
      // to agree with m_dataSize
      //
      NS_ASSERT_MSG (m_dataSize == m_size, "LppClient::Send(): m_size and m_dataSize inconsistent");
      NS_ASSERT_MSG (m_data, "LppClient::Send(): m_dataSize but no m_data");
      p = Create<Packet> (m_data, m_dataSize);
    }
  else
    {
      //
      // If m_dataSize is zero, the client has indicated that it doesn't care
      // about the data itself either by specifying the data size by setting
      // the corresponding attribute or by not calling a SetFill function.  In
      // this case, we don't worry about it either.  But we do allow m_size
      // to have a value different from the (zero) m_dataSize.
      //
      p = Create<Packet> (m_size);
    }
  Address localAddress;
  m_socket->GetSockName (localAddress);
  // call to the trace sinks before the packet is actually sent,
  // so that tags added to the packet can be sent as well
  m_txTrace (p);
  if (Ipv4Address::IsMatchingType (m_peerAddress))
    {
      m_txTraceWithAddresses (p, localAddress, InetSocketAddress (Ipv4Address::ConvertFrom (m_peerAddress), m_peerPort));
    }
  else if (Ipv4Address::IsMatchingType (m_peerAddress))
    {
      m_txTraceWithAddresses (p, localAddress, InetSocketAddress (Ipv4Address::ConvertFrom (m_peerAddress), m_peerPort));
    }
  m_socket->Send (p);
  ++m_sent;

  if (Ipv4Address::IsMatchingType (m_peerAddress))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " <<
                   Ipv4Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort);
    }
  else if (Ipv4Address::IsMatchingType (m_peerAddress))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " <<
                   Ipv4Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort);
    }
  else if (InetSocketAddress::IsMatchingType (m_peerAddress))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " <<
                   InetSocketAddress::ConvertFrom (m_peerAddress).GetIpv4 () << " port " << InetSocketAddress::ConvertFrom (m_peerAddress).GetPort ());
    }
  else if (InetSocketAddress::IsMatchingType (m_peerAddress))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " <<
                   InetSocketAddress::ConvertFrom (m_peerAddress).GetIpv4 () << " port " << InetSocketAddress::ConvertFrom (m_peerAddress).GetPort ());
    }

  if (m_sent < m_count) 
    {
      ScheduleTransmit (m_interval);
    }
}
void
LppClient::ConnectionSucceededCallback (Ptr<Socket> socket)
{
    NS_LOG_FUNCTION( this << socket);
    NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client to server connection succeeded "); 
    socket->SetRecvCallback (MakeCallback (&LppClient::HandleRead,
                                             this));

    LppHeader header;
    Address from = InetSocketAddress (Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort);
    header.SendLppPacket(m_socket, LppHeader::packet_type::REQUEST, LppHeader::packet_subtype::CERTIFICATE, from );
    startTime = Simulator::Now();
    m_rqCertificate(GetNode());

 //   ScheduleTransmit (Seconds (0.5));
   
}
void
LppClient::ConnectionFailedCallback (Ptr<Socket> socket)
{
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client to server connection failed " << 
                   InetSocketAddress::ConvertFrom (m_peerAddress).GetIpv4 () << " port " << InetSocketAddress::ConvertFrom (m_peerAddress).GetPort ());

       Simulator::Schedule (
          		m_interval, &LppClient::restart, this);
}

void
LppClient::NormalCloseCallback (Ptr<Socket> socket)
{
 NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client to server connection  closed normally " << 
                   InetSocketAddress::ConvertFrom (m_peerAddress).GetIpv4 () << " port " << InetSocketAddress::ConvertFrom (m_peerAddress).GetPort ());

  m_socket->SetCloseCallbacks (MakeNullCallback<void, Ptr<Socket> > (),
                               MakeNullCallback<void, Ptr<Socket> > ());




}
void
LppClient::ErrorCloseCallback (Ptr<Socket> socket)
{ NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client to server connection closed with error" << 
                   InetSocketAddress::ConvertFrom (m_peerAddress).GetIpv4 () << " port " << InetSocketAddress::ConvertFrom (m_peerAddress).GetPort ());
        Simulator::Schedule (
          		m_interval, &LppClient::restart, this);

}
void
LppClient::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  Address localAddress;
  while ((packet = socket->RecvFrom (from)))
    {
      if (InetSocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client received " << packet->GetSize () << " bytes from " <<
                       InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                       InetSocketAddress::ConvertFrom (from).GetPort ());
        }
      else if (InetSocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client received " << packet->GetSize () << " bytes from " <<
                       InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                       InetSocketAddress::ConvertFrom (from).GetPort ());
        }
      socket->GetSockName (localAddress);
      m_rxTrace (packet);
      m_rxTraceWithAddresses (packet, from, localAddress);
      HandlePacket(packet, socket, from);
    }
}

void
LppClient::computeLocation()
{

}
void
LppClient::HandlePacket(Ptr<Packet> packet, Ptr<Socket> socket, Address from)
{
    //int i = 0;
    NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "client received packet" << packet->GetSize ());
    LppHeader header;
    int flag;
    packet->PeekHeader(header);
    flag = header.GetFlag();
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
		case LppHeader::packet_subtype::LOCATION_INFO:
                     header.SendLppPacket(socket, LppHeader::packet_type::ABORT, LppHeader::packet_subtype::ERROR, from, flag);
                     break;
                case LppHeader::packet_subtype::ASSISTANCE_DATA:
                     if (flag > 0 )
                     {
                        header.SendLppPacket(socket, LppHeader::packet_type::REQUEST, LppHeader::packet_subtype::ASSISTANCE_DATA, from,  flag);
                        break;
		     }
		     
		     computeLocation(/*Assistant Data */);
                     header.SendLppPacket(socket, LppHeader::packet_type::PROVIDE, LppHeader::packet_subtype::LOCATION_INFO, from,  flag);
		     break;
                case LppHeader::packet_subtype::CERTIFICATE:
		     packet->PeekHeader(header);
		     m_rxCertificate(packet, (Simulator::Now () - startTime).GetMilliSeconds(), GetNode());
	      	     m_socket->Close ();
      	             m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());

	             Simulator::Schedule (
          		m_interval, &LppClient::restart, this);
		     break;
                case LppHeader::packet_subtype::ERROR:
                case LppHeader::packet_subtype::FATAL: 
                     header.SendLppPacket(socket, LppHeader::packet_type::ABORT, LppHeader::packet_subtype::ERROR, from,  flag);
    		      break;
          }
	  break;
	 case LppHeader::packet_type::REQUEST:
           switch(subtype)
           {
		case LppHeader::packet_subtype::CAPABILITIES:
                     header.SendLppPacket(socket, LppHeader::packet_type::PROVIDE, LppHeader::packet_subtype::CAPABILITIES, from, flag);
		     break;
		case LppHeader::packet_subtype::LOCATION_INFO:
                     header.SendLppPacket(socket, LppHeader::packet_type::REQUEST, LppHeader::packet_subtype::ASSISTANCE_DATA, from,  1+rand()%5);
                     break;
                case LppHeader::packet_subtype::ASSISTANCE_DATA:
                case LppHeader::packet_subtype::CERTIFICATE:
                     header.SendLppPacket(socket, LppHeader::packet_type::ABORT, LppHeader::packet_subtype::ERROR, from,  flag);
                     break;
                case LppHeader::ERROR:
                case LppHeader::FATAL: 
                     header.SendLppPacket(socket, LppHeader::packet_type::ABORT, LppHeader::packet_subtype::ERROR, from,  flag);
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

void
LppClient::SetHandleConnect(ConnectCback cback)
{
      m_rqCertificate = cback;
}
void
LppClient::SetHandleCertificate(CertificateCback cback)
{
      m_rxCertificate = cback;
}

} // Namespace ns3
