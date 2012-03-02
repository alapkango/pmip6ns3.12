/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * UDP6 Implementation
 *
 * Copyright (c) 2010 KUT, ETRI
 * (Korea Univerity of Technology and Education)
 * (Electronics and Telecommunications Research Institute)
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
 * Author: Hyon-Young Choi <commani@gmail.com>
 */
 
#ifndef UDP6_SOCKET_FACTORY_IMPL_H
#define UDP6_SOCKET_FACTORY_IMPL_H

#include "ns3/udp6-socket-factory.h"
#include "ns3/ptr.h"

namespace ns3 {

class Udp6L4Protocol;

/**
 * \ingroup internet
 * \defgroup udp6 Udp
 */

/**
 * \ingroup udp6
 * \brief Object to create UDPv6 socket instances 
 * \internal
 *
 * This class implements the API for creating UDPv6 sockets.
 * It is a socket factory (deriving from class SocketFactory).
 */
class Udp6SocketFactoryImpl : public Udp6SocketFactory
{
public:
  Udp6SocketFactoryImpl ();
  virtual ~Udp6SocketFactoryImpl ();

  void SetUdp (Ptr<Udp6L4Protocol> udp);

  /**
   * \brief Implements a method to create a Udp-based socket and return
   * a base class smart pointer to the socket.
   * \internal
   *
   * \return smart pointer to Socket
   */
  virtual Ptr<Socket> CreateSocket (void);

protected:
  virtual void DoDispose (void);
private:
  Ptr<Udp6L4Protocol> m_udp;
};

} // namespace ns3

#endif /* UDP6_SOCKET_FACTORY_IMPL_H */
