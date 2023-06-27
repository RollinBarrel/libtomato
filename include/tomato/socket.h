#pragma once

#ifdef _WIN32

#pragma comment(lib, "Ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1

#include <winsock2.h>
#include <windows.h>
#define SOCKET_IS_INVALID(s) s == INVALID_SOCKET
#define SOCKERROR WSAGetLastError()
#define SOCKET_WOULDBLOCK WSAEWOULDBLOCK
#define SOCKET_INPROGRESS WSAEINPROGRESS
#define SOCKET_ALREADY WSAEALREADY
#define SOCKET_CONNREFUSED WSAECONNREFUSED
#define SOCKET_NOSIGNAL 0

#else

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#define SOCKET_IS_INVALID(s) s == -1
#define SOCKERROR errno
#define SOCKET int
#define SOCKET_WOULDBLOCK EWOULDBLOCK
#define SOCKET_INPROGRESS EINPROGRESS
#define SOCKET_ALREADY EALREADY
#define SOCKET_CONNREFUSED ECONNREFUSED
#define SOCKET_NOSIGNAL MSG_NOSIGNAL

#endif