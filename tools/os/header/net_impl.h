/*
 * net_impl.h
 *
 *  Created on: Mar 5, 2017
 *      Author: erik
 */

#ifndef OS_HEADER_NET_IMPL_H_
#define OS_HEADER_NET_IMPL_H_

#include <os/netIntf.h>

struct OsSocket
{
  SocketType_e type;
  int32_t fd;
  uint32_t localAddress;
  uint16_t localPort;
  uint32_t remoteAddress;
  uint16_t remotePort;
};

OsSocket_t osnet_createSocket(SocketType_e type);
K_Status_e osnet_bind(OsSocket_t s, const OSIPv4Address_t * const address);
K_Status_e osnet_deleteSocket(OsSocket_t client);
K_Status_e osnet_setNonBlocking(OsSocket_t client, K_Boolean_e onoff);
K_Status_e osnet_udpSend(OsSocket_t client, const OSIPv4Address_t * const peer, const uint8_t * const buffer, size_t size);
const int32_t osnet_getFD(const OsSocket_t client);

#endif /* OS_HEADER_NET_IMPL_H_ */
