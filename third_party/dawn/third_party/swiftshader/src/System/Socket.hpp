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

#ifndef sw_Socket_hpp
#define sw_Socket_hpp

#if defined(_WIN32)
#	include <winsock2.h>
#else
#	include <sys/socket.h>
typedef int SOCKET;
#endif

namespace sw {

class Socket
{
public:
	Socket(SOCKET socket);
	Socket(const char *address, const char *port);
	~Socket();

	void listen(int backlog = 1);
	bool select(int us);
	Socket *accept();

	int receive(char *buffer, int length);
	void send(const char *buffer, int length);

	static void startup();
	static void cleanup();

private:
	SOCKET socket;
};

}  // namespace sw

#endif  // sw_Socket_hpp
