/******************************************************************************
WHISPER - An advanced database system
Copyright (C) 2008  Iulian Popa

Address: Str Olimp nr. 6
         Pantelimon Ilfov,
         Romania
Phone:   +40721939650
e-mail:  popaiulian@gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#define WIN32_LEAN_AND_MEAN

#include <assert.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

// Need to link with Ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

#include "whisper.h"

bool_t
whs_init ()
{
  static bool_t _inited = FALSE;

  WORD wVersionRequested;
  WSADATA wsaData;

  assert (_inited == FALSE);
  if (_inited)
    return FALSE;

  wVersionRequested = MAKEWORD(2, 2);

  if (WSAStartup(wVersionRequested, &wsaData) != 0)
    return FALSE;

  if ((LOBYTE (wsaData.wVersion) != 2) || (HIBYTE (wsaData.wVersion) != 2))
    {
        WSACleanup();
        return FALSE;
    }

  _inited = TRUE;
  return TRUE;
}

uint32_t
whs_create_client (const char* const        pServer,
                  const char* const        pPort,
                  WH_SOCKET* const           pOutSocket)
{
  struct addrinfo  hints    = {0, };
  struct addrinfo* pResults = NULL;
  struct addrinfo* pIt      = NULL;
  uint32_t         status   = ~0;
  int            sd       = -1;
  const int      on       = 1;


  assert (pServer != NULL);
  assert (pPort != NULL);

  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags    = AI_ADDRCONFIG;

  status = getaddrinfo (pServer, pPort, &hints, &pResults);
  if (status != 0)
    return status;


  for (pIt = pResults; pIt != NULL; pIt = pResults->ai_next)
    {
      sd = socket (pIt->ai_family, pIt->ai_socktype, pIt->ai_protocol);
      if (sd == INVALID_SOCKET)
        continue ;

      if (setsockopt (sd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof on) != 0)
        {
          status = WSAGetLastError ();
          closesocket (sd);

          return status;
        }
      else if (connect (sd, pIt->ai_addr, pIt->ai_addrlen) == 0)
          break;
      else
        {
          status = WSAGetLastError ();
          closesocket (sd);

          return status;
        }
    }

  status = WSAGetLastError ();
  freeaddrinfo (pResults);

  if (pIt != NULL)
    {
      /* We have a connected socket */
      *pOutSocket = sd;
      return WOP_OK;
    }

  return status;
}

uint32_t
whs_create_server (const char* const       pLocalAdress,
                  const char* const       pPort,
                  const uint_t              listenBackLog,
                  WH_SOCKET* const          pOutSocket)
{
  struct addrinfo  hints    = {0, };
  struct addrinfo* pResults = NULL;
  struct addrinfo* pIt      = NULL;
  uint32_t         status   = ~0;
  int            sd       = -1;
  const int      on       = 1;

  assert (pPort != NULL);

  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags    = AI_ADDRCONFIG | AI_NUMERICHOST | AI_PASSIVE;

  status = getaddrinfo (pLocalAdress, pPort, &hints, &pResults);
  if (status != 0)
    return status;

  for (pIt = pResults; pIt != NULL; pIt = pResults->ai_next)
    {
      sd = socket (pIt->ai_family, pIt->ai_socktype, pIt->ai_protocol);
      if (sd == INVALID_SOCKET)
        continue ;

      if ((setsockopt (sd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof on) != 0)
          || (setsockopt (sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on) != 0))
        {
          status = WSAGetLastError ();
          closesocket (sd);

          return status;
        }
      else if (bind (sd, pIt->ai_addr, pIt->ai_addrlen) == 0)
        break;
      else
        {
          status = WSAGetLastError ();
          closesocket (sd);

          return status;
        }
    }

  status = WSAGetLastError ();
  freeaddrinfo (pResults);

  if (pIt != NULL)
    {
      /* We have a valid socket */
      if (listen (sd, listenBackLog) != 0)
        {
          status = WSAGetLastError ();
          closesocket (sd);

          return status;
        }

      *pOutSocket = sd;
      return WOP_OK;
    }

  return status;
}

uint32_t
whs_accept (const WH_SOCKET      sd,
                  WH_SOCKET* const     pConnectSocket)
{
  const WH_SOCKET csd = accept (sd, NULL, NULL);
  if (csd == INVALID_SOCKET)
    return WSAGetLastError ();

  *pConnectSocket = csd;
  return WOP_OK;
}

uint32_t
whs_write (const WH_SOCKET      sd,
                 const uint8_t*       pBuffer,
                 const uint_t         count)
{
  uint_t   wrote = 0;

  assert (count > 0);

  while (wrote < count)
    {
      const int chunk = send (sd, pBuffer + wrote, count - wrote, 0);
      if (chunk < 0)
        {
          const uint32_t status = WSAGetLastError ();
          if (status != WSATRY_AGAIN)
            return status;
        }
      else
        {
          assert (chunk > 0);
          wrote += chunk;
        }
    }

  assert (wrote == count);

  return WOP_OK;
}

uint32_t
whs_read (const WH_SOCKET           sd,
                uint8_t*                  pOutBuffer,
                uint_t* const             pIOCount)
{
  if (*pIOCount == 0)
    return WSAEINVAL;

  while (TRUE)
    {
      const int chunk = recv (sd, pOutBuffer, *pIOCount, 0);
      if (chunk < 0)
        {
          const uint32_t status = WSAGetLastError ();
          if (status != WSATRY_AGAIN)
            return status;
        }
      else
        {
          assert ((uint_t)chunk <= *pIOCount);

          *pIOCount = chunk;
          break;
        }
    }

  return WOP_OK;
}

void
whs_close (const WH_SOCKET sd)
{
  shutdown (sd, SD_BOTH);
  closesocket (sd);
}

void
whs_clean ()
{
  WSACleanup();
}
