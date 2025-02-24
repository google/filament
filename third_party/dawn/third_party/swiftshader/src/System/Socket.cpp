// Copyright 2016 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "Socket.hpp"

#if defined(_WIN32)
#	include <ws2tcpip.h>
#else
#	include <unistd.h>
#	include <netdb.h>
#	include <netinet/in.h>
#	include <sys/select.h>
#endif

namespace sw {

Socket::Socket(SOCKET socket)
    : socket(socket)
{
}

Socket::Socket(const char *address, const char *port)
{
#if defined(_WIN32)
	socket = INVALID_SOCKET;
#else
	socket = -1;
#endif

	addrinfo hints = {};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	addrinfo *info = 0;
	getaddrinfo(address, port, &hints, &info);

	if(info)
	{
		socket = ::socket(info->ai_family, info->ai_socktype, info->ai_protocol);
		bind(socket, info->ai_addr, (int)info->ai_addrlen);
	}
}

Socket::~Socket()
{
#if defined(_WIN32)
	closesocket(socket);
#else
	close(socket);
#endif
}

void Socket::listen(int backlog)
{
	::listen(socket, backlog);
}

bool Socket::select(int us)
{
	fd_set sockets;
	FD_ZERO(&sockets);
	FD_SET(socket, &sockets);

	timeval timeout = { us / 1000000, us % 1000000 };

	return ::select(FD_SETSIZE, &sockets, 0, 0, &timeout) >= 1;
}

Socket *Socket::accept()
{
	return new Socket(::accept(socket, 0, 0));
}

int Socket::receive(char *buffer, int length)
{
	return recv(socket, buffer, length, 0);
}

void Socket::send(const char *buffer, int length)
{
	::send(socket, buffer, length, 0);
}

void Socket::startup()
{
#if defined(_WIN32)
	WSADATA winsockData;
	WSAStartup(MAKEWORD(2, 2), &winsockData);
#endif
}

void Socket::cleanup()
{
#if defined(_WIN32)
	WSACleanup();
#endif
}

}  // namespace sw
