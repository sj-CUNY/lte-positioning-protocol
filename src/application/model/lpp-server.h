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

#ifndef LPP_SERVER_H
#define LPP_SERVER_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/traced-callback.h"
#include "ns3/lpp-header.h"
#include <ns3/tcp-socket.h>
#include <ns3/tcp-socket-factory.h>

namespace ns3 {

class Socket;
class Packet;

/**
 * \ingroup applications 
 * \defgroup udpecho UdpEcho
 */

/**
 * \ingroup udpecho
 * \brief A Udp Echo server
 *
 * Every packet received is sent back.
 */
class LppServer : public Application 
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  LppServer ();
  virtual ~LppServer ();
  void SetFill (std::string fill);

protected:
  virtual void DoDispose (void);

private:
  const uint8_t ASSISTANCE = 1;
  const uint8_t DONE = 0;
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  /**
   * \brief Handle a packet reception.
   *
   * This function is called by lower layers.
   *
   * \param socket the socket the packet was received to.
   */

  void HandlePacket(Ptr<Socket> socket, Ptr<Packet> packet, Address from);
  void HandleRead (Ptr<Socket> socket);
  bool ConnectionRequestCallback (Ptr<Socket>   socket,
                                  const Address &address);
   void NewConnectionCreatedCallback (Ptr<Socket>    socket,
                                     const Address  &address);
 
  void ErrorCloseCallback (Ptr<Socket> socket);
  void NormalCloseCallback (Ptr<Socket> socket);

  void SendCallback (Ptr<Socket> socket, uint32_t availableBufferSize);
  uint16_t m_port; //!< Port on which we listen for incoming packets.
  Address m_peer;
  uint8_t *m_data; //!< packet payload data
  uint32_t m_size; //!< Size of the sent packet
  uint32_t m_dataSize; //!< Size of the sent packet
  Ptr<Socket> m_socket; //!< IPv4 Socket
  Ptr<Socket> m_socket6; //!< IPv6 Socket
  Address m_local; //!< local  address
  uint16_t m_more;
  uint8_t m_state;
  /// Callbacks for tracing the packet Rx events
  TracedCallback<Ptr<const Packet> > m_rxTrace;

  /// Callbacks for tracing the packet Rx events, includes source and destination addresses
  TracedCallback<Ptr<const Packet>, const Address &, const Address &> m_rxTraceWithAddresses;
};

} // namespace ns3

#endif /* LPP_SERVER_H */

