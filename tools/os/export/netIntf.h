/*
 * netIntf.h
 *
 *  Created on: Mar 5, 2017
 *      Author: erik
 */

#ifndef OS_EXPORT_NETINTF_H_
#define OS_EXPORT_NETINTF_H_

#include  <common/k_types.h>

typedef struct OsSocket * OsSocket_t;

typedef struct ipv4Address_
{
  uint32_t ipAddress;
  uint16_t port;
} OSIPv4Address_t;

typedef enum
{
  SOCKETTYPE_DGRAM = 10,
  SOCKETTYPE_STREAM = 20
} SocketType_e;

typedef struct _net
{
  OsSocket_t (* createSocket)(SocketType_e type);
  K_Status_e (* bind)(OsSocket_t s, const OSIPv4Address_t * const address);
  K_Status_e (* deleteSocket)(OsSocket_t client);
  K_Status_e (* setNonBlocking)(OsSocket_t client, K_Boolean_e onoff);
  K_Status_e (* udpSend)(OsSocket_t client, const OSIPv4Address_t * const peer, const uint8_t * const buffer, size_t size);
  const int32_t (* getFD)(const OsSocket_t client);
} INetwork_t;

INetwork_t * getNetIntf(void);

#endif /* OS_EXPORT_NETINTF_H_ */
