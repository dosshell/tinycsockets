#include "implementation_detector.h"
#ifdef USE_POSIX_IMPL

#include "tinycsocket.h"

#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <stdlib.h>
#include <netdb.h>

#include <errno.h>
#include <string.h>

#include <stdbool.h>

static const int NO_SOCKET = -1;
static const int FAILURE = -1;

// Internal structures
typedef struct TinyCSocketCtxInternal
{
  int socket;
} TinyCSocketCtxInternal;

int tinycsocket_init()
{
  // Not needed for posix
  return TINYCSOCKET_SUCCESS;
}

int tinycsocket_free()
{
  // Not needed for posix
  return TINYCSOCKET_SUCCESS;
}

int tinycsocket_create_socket(TinyCSocketCtx** outSocketCtx)
{
  if (outSocketCtx == NULL || *outSocketCtx != NULL)
    return TINYCSOCKET_ERROR_INVALID_ARGUMENT;

  // Allocate socket data
  TinyCSocketCtxInternal** ppInternalCtx = (TinyCSocketCtxInternal**)outSocketCtx;
  *ppInternalCtx = (TinyCSocketCtxInternal*)malloc(sizeof(TinyCSocketCtxInternal));
  if (*ppInternalCtx == NULL)
  {
    return TINYCSOCKET_ERROR_MEMORY;
  }
  TinyCSocketCtxInternal* pInternalCtx = (*ppInternalCtx);
  
  pInternalCtx->socket = NO_SOCKET;

  return TINYCSOCKET_SUCCESS;
}

int tinycsocket_destroy_socket(TinyCSocketCtx** inoutSocketCtx)
{
  if (inoutSocketCtx == NULL)
  return TINYCSOCKET_SUCCESS;

  TinyCSocketCtxInternal* pInternalCtx = (TinyCSocketCtxInternal*)(*inoutSocketCtx);
  close(pInternalCtx->socket);

  free(*inoutSocketCtx);
  *inoutSocketCtx = NULL;

  return TINYCSOCKET_SUCCESS;
}

int tinycsocket_connect(TinyCSocketCtx* inoutSocketCtx, const char* address, const char* port)
{
  TinyCSocketCtxInternal* pInternalCtx = inoutSocketCtx;
  if (pInternalCtx == NULL || pInternalCtx->socket != NO_SOCKET)
  {
    return TINYCSOCKET_ERROR_INVALID_ARGUMENT;
  }
  u_short portNumber = atoi(port);
  if (portNumber <= 0)
  {
    return TINYCSOCKET_ERROR_INVALID_ARGUMENT;
  }
  
  struct addrinfo *serverinfo = NULL, *ptr = NULL, hints;
  
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  if (getaddrinfo(address, port, &hints, &serverinfo) != 0)
  {
    return TINYCSOCKET_ERROR_ADDRESS_LOOKUP_FAILED;
  }

  // Try to connect
  bool didConnect = false;
  for (ptr = serverinfo; ptr != NULL; ptr = ptr->ai_next)
  {
    pInternalCtx->socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if (pInternalCtx->socket == NO_SOCKET)
    {
      continue;
    }

    if (connect(pInternalCtx->socket, ptr->ai_addr, ptr->ai_addrlen) == FAILURE)
    {
      close(pInternalCtx->socket);
      continue;
    }

    didConnect = true;
  }

  if (!didConnect)
  {
    return TINYCSOCKET_ERROR_CONNECTION_REFUSED;
  }
  return TINYCSOCKET_SUCCESS;
}

int tinycsocket_send_data(TinyCSocketCtx* inSocketCtx, const void* data, const size_t bytes)
{
  TinyCSocketCtxInternal* pInternalCtx = inSocketCtx;
  if (pInternalCtx->socket == NO_SOCKET)
  {
    return TINYCSOCKET_ERROR_INVALID_ARGUMENT;
  }

  if (send(pInternalCtx->socket, data, bytes, 0) == FAILURE)
  {
    int sendError = errno;
    switch (sendError)
    {
      case ENOTCONN:
        return TINYCSOCKET_ERROR_NOT_CONNECTED;
      default:
        return TINYCSOCKET_ERROR_UNKNOWN;
    }
  }
  return TINYCSOCKET_SUCCESS;
}

int tinycsocket_recieve_data(TinyCSocketCtx* inSocketCtx,
                             const void* buffer,
                             const size_t bufferByteSize,
                             int* outBytesRecieved)
{
  if (inSocketCtx == NULL || buffer == NULL || bufferByteSize == NULL)
  {
    return TINYCSOCKET_ERROR_INVALID_ARGUMENT;
  }

  TinyCSocketCtxInternal* pInternalCtx = inSocketCtx;
  int recvResult = recv(pInternalCtx->socket, buffer, bufferByteSize, 0);
  if (recvResult == -1)
  {
    return TINYCSOCKET_ERROR_UNKNOWN;
  }
  if (outBytesRecieved != NULL)
  {
    *outBytesRecieved = recvResult;
  }
  return TINYCSOCKET_SUCCESS;
}

int tinycsocket_bind(TinyCSocketCtx* inSocketCtx, const char* address, const char* port)
{
  TinyCSocketCtxInternal* pInternalCtx = inSocketCtx;

  if (pInternalCtx == NULL || pInternalCtx->socket != NO_SOCKET)
  {
    return TINYCSOCKET_ERROR_INVALID_ARGUMENT;
  }

  struct addrinfo* result = NULL;
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE;

  int addressInfo = getaddrinfo(NULL, port, &hints, &result);
  if (addressInfo != 0)
  {
    return TINYCSOCKET_ERROR_KERNEL;
  }

  bool didConnect = false;
  pInternalCtx->socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (pInternalCtx->socket == NO_SOCKET)
  {
    return TINYCSOCKET_ERROR_KERNEL;
  }

  if (bind(pInternalCtx->socket, result->ai_addr, (int)result->ai_addrlen) != FAILURE)
  {
    didConnect = true;
  }

  freeaddrinfo(result);

  if (!didConnect)
  {
    return TINYCSOCKET_ERROR_UNKNOWN;
  }

  return TINYCSOCKET_SUCCESS;
}

int tinycsocket_listen(TinyCSocketCtx* inoutSocketCtx)
{
  TinyCSocketCtxInternal* pInternalCtx = inoutSocketCtx;

  if (pInternalCtx == NULL || pInternalCtx->socket == NO_SOCKET)
  {
    return TINYCSOCKET_ERROR_INVALID_ARGUMENT;
  }

  if (listen(pInternalCtx->socket, SOMAXCONN) == FAILURE)
  {
    return TINYCSOCKET_ERROR_UNKNOWN;
  }

  return TINYCSOCKET_SUCCESS;
}

int tinycsocket_accept(TinyCSocketCtx* inListenSocketCtx, TinyCSocketCtx* inoutBindSocketCtx)
{
  TinyCSocketCtxInternal* pInternalCtx = inListenSocketCtx;
  TinyCSocketCtxInternal* pInternalBindCtx = inoutBindSocketCtx;

  pInternalBindCtx->socket = accept(pInternalCtx->socket, NULL, NULL);

  return TINYCSOCKET_SUCCESS;
}

#endif