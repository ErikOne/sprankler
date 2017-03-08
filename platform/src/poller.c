/*
 * poller.c
 *
 *  Created on: Mar 4, 2017
 *      Author: erik
 */

/*
 * poller.c
 *
 *  Created on: May 13, 2014
 *      Author: erik
 *
 *  The poller is a very stripped down version of a reactor. As the poler only gets called at system start up and
 *  the registration of the GPIO files are done at that time we cab skip some of the housekeeping code.
 *
 *  As this is very platform specific code I though it was reasonable to use some OS specific functons
 */

#include <unistd.h>
#include <poll.h>
#include <fcntl.h>

#include <common/k_types.h>

#include <os/memIntf.h>
#include <os/threadIntf.h>
#include <os/utilsIntf.h>
#include <os/netIntf.h>
#include <logging/logging.h>

#include "platform.h"

#define WAKEUP_POSITION     0
#define MAX_POLL_ENTRIES    5
#define POLLER_WAKEUP_PORT  ((uint16_t)60000U)

typedef void (* POLLHANDLER)(int32_t fd, uint8_t * data, uint32_t nbrOfBytes);

struct _PlatformPollerData
{
  int32_t fd;
  POLLHANDLER handler;
};

static struct PollerData
{
  OsThread_t pollerThread;
  struct _PlatformPollerData pollEntries[MAX_POLL_ENTRIES];
  /* Set of active file descriptors, the array may not contain gaps, the number of file descriptors is
   * calculated based on the first 0 file descriptor.
   */
  struct pollfd descriptors[MAX_POLL_ENTRIES];
  OsMutex_t entryLock;
  volatile K_Boolean_e active;
  volatile K_Boolean_e polling;
  OsSocket_t wakeupSocket;
  OSIPv4Address_t wakeupAddress;
} localPollerData;

static K_Status_e localCreateAndRegisterWakeupSocket(struct PollerData * poller);

static void * localPollerTask(void * data);
static void localWakeup(struct PollerData * poller);

K_Status_e platform_initSysPoller(void)
{
  K_Status_e rc = K_Status_General_Error;
  const IMem_t * const mem = getMemIntf();
  const IThread_t * const ti = getThreadIntf();

  mem->memset(&localPollerData, 0, sizeof(localPollerData));

  if (localCreateAndRegisterWakeupSocket(&localPollerData) == K_Status_OK)
  {
    if ((localPollerData.entryLock = ti->mutexCreate()) != NULL)
    {
      localPollerData.active = K_True;
      if ((localPollerData.pollerThread = ti->threadCreate(localPollerTask, &localPollerData)) != NULL)
      {
        rc = K_Status_OK;
      }
      else
      {
        FATAL("Could not create poller Thread\n");
      }
    }
    else
    {
      FATAL("Could not create system poller mutex\n");
    }
  }
  return rc;
}

K_Status_e platform_addPollHandler(int32_t fd, POLLHANDLER handler)
{
  K_Status_e rc = K_Status_General_Error;
  const IThread_t * const ti = getThreadIntf();
  struct PollerData * poller = &localPollerData;

  if (ti->mutexLock(poller->entryLock) == K_Status_OK)
  {
    localWakeup(poller);
    while (poller->polling == K_True)
    {
      ti->threadYield();
    }

    uint32_t i;
    for (i = 0; (i < MAX_POLL_ENTRIES); i++)
    {
      if (poller->descriptors[i].fd == 0)
      {
        poller->descriptors[i].fd = fd;

        poller->descriptors[i].events = POLLPRI | POLLERR;

        poller->pollEntries[i].fd = fd;
        poller->pollEntries[i].handler = handler;

        INFO("Added entry %d to system poller at pos %u\n", fd, i);
        rc = K_Status_OK;
        break;
      }
    }
    ti->mutexUnlock(poller->entryLock);
  }

  return rc;
}

K_Status_e platform_removePollHandler(int32_t fd)
{
  K_Status_e rc = K_Status_General_Error;
  const IThread_t * const ti = getThreadIntf();
  const IMem_t * const mem = getMemIntf();
  struct PollerData * poller = &localPollerData;

  if (ti->mutexLock(poller->entryLock) == K_Status_OK)
  {
    localWakeup(poller);
    while (poller->polling == K_True)
    {
      ti->threadYield();
    }

    uint32_t i, j;
    for (i = 0; (i < MAX_POLL_ENTRIES); i++)
    {
      if (poller->descriptors[i].fd == fd)
      {
        for (j = i; j < MAX_POLL_ENTRIES - 1; j++)
        {
          poller->descriptors[j].fd = poller->descriptors[j + 1].fd;
          poller->descriptors[j].events = poller->descriptors[j + 1].events;

          poller->pollEntries[j].fd = poller->pollEntries[j + 1].fd;
          poller->pollEntries[j].handler = poller->pollEntries[j + 1].handler;
        }

        mem->memset(&poller->descriptors[j], 0, sizeof(struct pollfd));
        mem->memset(&poller->pollEntries[j], 0, sizeof(struct _PlatformPollerData));

        INFO("Removed entry %d to system poller at pos %u\n", fd, i);
        rc = K_Status_OK;
        break;
      }
    }

    ti->mutexUnlock(poller->entryLock);
  }

  return rc;
}

static void * localPollerTask(void * data)
{
  const IThread_t * const ti = getThreadIntf();

  struct PollerData * poller = (struct PollerData *)data;
  while (poller->active == K_True)
  {
    poller->polling = K_False;
    uint32_t nbr;

    if (ti->mutexLock(poller->entryLock) == K_Status_OK)
    {
      for (nbr = 0; (nbr < MAX_POLL_ENTRIES) && (poller->descriptors[nbr].fd != 0); nbr++) ;

      poller->polling = K_True;
      ti->mutexUnlock(poller->entryLock);
    }

    int32_t timeout = -1;
    int32_t result = poll(poller->descriptors, nbr, timeout);
    uint32_t i;

    if (result > 0)
    {
      /* According to sysfs_poll.c when a file is changed POLLPRI and POLLERR is returned, so check for this */
      uint8_t buffer[1024];
      int32_t nbrOfBytes;

      for (i = 0; i < nbr; i++)
      {
        if (poller->descriptors[i].revents != 0)
        {
          if ((poller->descriptors[i].revents & ~(poller->descriptors[i].events)) == 0)
          {
            lseek(poller->descriptors[i].fd, 0, SEEK_SET);
            nbrOfBytes = read(poller->descriptors[i].fd, buffer, sizeof(buffer) - 1);
            if (nbrOfBytes > 0)
            {
              buffer[nbrOfBytes] = '\0';
              if (poller->pollEntries[i].handler != NULL)
              {
                poller->pollEntries[i].handler(poller->pollEntries[i].fd, buffer, (uint32_t) nbrOfBytes);
              }
            }
          }
          else if ((i == WAKEUP_POSITION) && (poller->descriptors[WAKEUP_POSITION].revents & POLLIN) != 0)
          {
            nbrOfBytes = read(poller->descriptors[i].fd, buffer, sizeof buffer);
          }

          else
          {
            ERROR("Unhandled pollevent %u for fd %d\n", poller->descriptors[i].revents, poller->descriptors[i].fd);
          }
        }
      }
    }
    else
    {
      ERROR("Error in polling\n");
    }
  } /*while (1) */

  return NULL;
}

static K_Status_e localCreateAndRegisterWakeupSocket(struct PollerData * poller)
{
  K_Status_e rc = K_Status_General_Error;
  const INetwork_t * const net = getNetIntf();
  uint16_t i;
  if (poller != NULL)
  {
    poller->wakeupSocket = net->createSocket(SOCKETTYPE_DGRAM);
    if (poller->wakeupSocket != NULL)
    {
      poller->wakeupAddress.ipAddress = (127 << 24) | 1;

      if (net->setNonBlocking(poller->wakeupSocket, K_True) == K_Status_OK)
      {
        for (i = 0; i < 0xFFFF; i++)
        {
          poller->wakeupAddress.port = POLLER_WAKEUP_PORT + i;
          if (net->bind(poller->wakeupSocket, &poller->wakeupAddress) == K_Status_OK)
          {
            /* The wake up socket will always be at position 0 */
            poller->descriptors[WAKEUP_POSITION].fd = net->getFD(poller->wakeupSocket);
            poller->descriptors[WAKEUP_POSITION].events = POLLIN | POLLPRI;

            poller->pollEntries[WAKEUP_POSITION].fd = net->getFD(poller->wakeupSocket);
            poller->pollEntries[WAKEUP_POSITION].handler = NULL;

            INFO("System poller is bound at port %u\n", poller->wakeupAddress.port);
            rc = K_Status_OK;
            break;
          }
        }
      }
    }
  }
  return rc;
}

static void localWakeup(struct PollerData * poller)
{
  if (poller != NULL)
  {
    const INetwork_t * const net = getNetIntf();
    uint8_t buffer = 1;
    if (net->udpSend(poller->wakeupSocket, &poller->wakeupAddress, &buffer, sizeof buffer) != K_Status_OK)
    {
      ERROR("Failed to wake up system poller\n");
    }
  }
}

