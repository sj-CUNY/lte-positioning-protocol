/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "lpp-helper.h"
#include "ns3/lpp-server.h"
#include "ns3/lpp-client.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

namespace ns3 {

LppServerHelper::LppServerHelper (const Address& address)
{
  m_factory.SetTypeId (LppServer::GetTypeId ());
  SetAttribute ("ServerAddress", AddressValue (address));
}

void 
LppServerHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
LppServerHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
LppServerHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
LppServerHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
LppServerHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<LppServer> ();
  node->AddApplication (app);

  return app;
}

void
LppServerHelper::SetFill (Ptr<Application> app, std::string fill)
{
  app->GetObject<LppServer>()->SetFill (fill);
}
LppClientHelper::LppClientHelper (Address address, uint16_t port)
{
  m_factory.SetTypeId (LppClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  SetAttribute ("RemotePort", UintegerValue (port));
}

LppClientHelper::LppClientHelper (Address address)
{
  m_factory.SetTypeId (LppClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
}
void 
LppClientHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

void
LppClientHelper::restart(Ptr<Application> app)
{
  app->GetObject<LppClient>()->restart();
}

void
LppClientHelper::CertificateReqTrace(Ptr<Application> app , Callback<void, Ptr<Node> > cback)
{
  app->GetObject<LppClient>()->SetHandleConnect(cback);
}
void
LppClientHelper::CertificateRxTrace(Ptr<Application> app , Callback<void, Ptr<Packet>,uint32_t, Ptr<Node> > cback)
{
  app->GetObject<LppClient>()->SetHandleCertificate(cback);
}
void
LppClientHelper::SetFill (Ptr<Application> app, std::string fill)
{
  app->GetObject<LppClient>()->SetFill (fill);
}

void
LppClientHelper::SetFill (Ptr<Application> app, uint8_t fill, uint32_t dataLength)
{
  app->GetObject<LppClient>()->SetFill (fill, dataLength);
}

void
LppClientHelper::SetFill (Ptr<Application> app, uint8_t *fill, uint32_t fillLength, uint32_t dataLength)
{
  app->GetObject<LppClient>()->SetFill (fill, fillLength, dataLength);
}

ApplicationContainer
LppClientHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
LppClientHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
LppClientHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
LppClientHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<LppClient> ();
  node->AddApplication (app);

  return app;
}

} // namespace ns3
