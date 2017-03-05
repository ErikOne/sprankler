/*
 * net.c
 *
 *  Created on: Mar 5, 2017
 *      Author: erik
 */

#include "net_impl.h"

#include <os/memIntf.h>
#include <logging/logging.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

OsSocket_t osnet_createSocket(SocketType_e type)
{
  OsSocket_t s = NULL;

  int32_t native_type = -1;
  switch (type)
  {
    case SOCKETTYPE_DGRAM:
      native_type = SOCK_DGRAM;
      break;
    case SOCKETTYPE_STREAM:
      native_type = SOCK_STREAM;
      break;
  }

  if (native_type != -1)
  {
    int32_t fd = socket(AF_INET, native_type, 0);

    if (fd > 0)
    {
      int32_t on = 1;
      if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == 0)
      {
        if (setsockopt(fd, SOL_SOCKET, SO_TIMESTAMPNS, &on, sizeof(on)) == 0)
        {
          const IMem_t * const mem = getMemIntf();
          OsSocket_t newSocket = (OsSocket_t) mem->malloc(sizeof(struct OsSocket));
          if (newSocket != NULL)
          {
            mem->memset(newSocket, 0, sizeof(struct OsSocket));
            newSocket->fd = fd;
            newSocket->type = type;
            s = newSocket;
          }
        }
      }

      if (s == NULL)
      {
        close(fd);
      }
    }
  }

  return s;
}

K_Status_e osnet_deleteSocket(OsSocket_t client)
{
  K_Status_e rc = K_Status_General_Error;

  if (client != NULL)
  {
    const IMem_t * const mem = getMemIntf();
    close(client->fd);
    mem->free(client);

    rc = K_Status_OK;
  }

  return rc;
}

K_Status_e osnet_bind(OsSocket_t s, const OSIPv4Address_t * const address)
{
  K_Status_e rc = K_Status_General_Error;

  if (s != NULL)
  {
    const IMem_t * const mem = getMemIntf();
    struct sockaddr_in localSock;

    mem->memset(&localSock, 0, sizeof(localSock));

    localSock.sin_family = AF_INET;
    localSock.sin_port = htons(address->port);
    localSock.sin_addr.s_addr = htonl(address->ipAddress);

    if (bind(s->fd, (struct sockaddr *) &localSock, sizeof(localSock)) == 0)
    {
      s->localAddress = address->ipAddress;
      s->localAddress = address->port;
      rc = K_Status_OK;
    }
    else
    {
      ERROR("Could not bind socket to interface %x, port %u\n", address->ipAddress, address->port);
    }
  }

  return rc;
}

K_Status_e osnet_setNonBlocking(OsSocket_t client, K_Boolean_e onoff)
{
  K_Status_e rc = K_Status_General_Error;

  if (client != NULL)
  {
    int32_t flags = fcntl(client->fd, F_GETFL, 0);
    if (flags > -1)
    {
      if (onoff == K_True)
      {
        flags |= O_NONBLOCK;
      }
      else
      {
        flags &= ~O_NONBLOCK;
      }

      if (fcntl(client->fd, F_SETFL, flags) == 0)
      {
        TRACE("Successfully switched %s non-blocking mode\n", (onoff == K_True)?"on":"off");
        rc = K_Status_OK;
      }
    }

  }
  return rc;

}

K_Status_e osnet_udpSend(OsSocket_t client, const OSIPv4Address_t * const peer, const uint8_t * const buffer, size_t size)
{
  K_Status_e rc = K_Status_General_Error;

  if (client != NULL)
  {
    int32_t bytesSend = -1;
    if (peer != NULL)
    {
      const IMem_t * const mem = getMemIntf();
      struct sockaddr_in remote;

      mem->memset(&remote, 0, sizeof(remote));

      remote.sin_family = AF_INET;
      remote.sin_port = htons(peer->port);
      remote.sin_addr.s_addr = htonl(peer->ipAddress);

      bytesSend = sendto(client->fd, buffer, size, 0, (struct sockaddr *) &remote, sizeof(remote));
    }
    else
    {
      bytesSend = send(client->fd, buffer, size, 0);
    }

    if (bytesSend > -1)
    {
      if ((size_t) bytesSend == size)
      {
        rc = K_Status_OK;
      }
      else
      {
        ERROR("Could not send UDP packet to %08x:%u\n", peer->ipAddress, peer->port);
      }
    }
  }

  return rc;

}

const int32_t osnet_getFD(const OsSocket_t client)
{
  int32_t fd = -1;

  if (client != NULL)
  {
    fd = client->fd;
  }

  return fd;

}

