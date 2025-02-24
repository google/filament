// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "socket.h"

#include "rwmutex.h"

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#if defined(_WIN32)
#include <atomic>
namespace {
std::atomic<int> wsaInitCount = {0};
}  // anonymous namespace
#else
#include <fcntl.h>
#include <unistd.h>
namespace {
using SOCKET = int;
}  // anonymous namespace
#endif

namespace {
constexpr SOCKET InvalidSocket = static_cast<SOCKET>(-1);
void init() {
#if defined(_WIN32)
  if (wsaInitCount++ == 0) {
    WSADATA winsockData;
    (void)WSAStartup(MAKEWORD(2, 2), &winsockData);
  }
#endif
}

void term() {
#if defined(_WIN32)
  if (--wsaInitCount == 0) {
    WSACleanup();
  }
#endif
}

bool setBlocking(SOCKET s, bool blocking) {
#if defined(_WIN32)
  u_long mode = blocking ? 0 : 1;
  return ioctlsocket(s, FIONBIO, &mode) == NO_ERROR;
#else
  auto arg = fcntl(s, F_GETFL, nullptr);
  if (arg < 0) {
    return false;
  }
  arg = blocking ? (arg & ~O_NONBLOCK) : (arg | O_NONBLOCK);
  return fcntl(s, F_SETFL, arg) >= 0;
#endif
}

bool errored(SOCKET s) {
  if (s == InvalidSocket) {
    return true;
  }
  char error = 0;
  socklen_t len = sizeof(error);
  getsockopt(s, SOL_SOCKET, SO_ERROR, &error, &len);
  return error != 0;
}

}  // anonymous namespace

class dap::Socket::Shared : public dap::ReaderWriter {
 public:
  static std::shared_ptr<Shared> create(const char* address, const char* port) {
    init();

    addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    addrinfo* info = nullptr;
    getaddrinfo(address, port, &hints, &info);

    if (info) {
      auto socket =
          ::socket(info->ai_family, info->ai_socktype, info->ai_protocol);
      auto out = std::make_shared<Shared>(info, socket);
      out->setOptions();
      return out;
    }

    freeaddrinfo(info);
    term();
    return nullptr;
  }

  Shared(SOCKET socket) : info(nullptr), s(socket) {}
  Shared(addrinfo* info, SOCKET socket) : info(info), s(socket) {}

  ~Shared() {
    freeaddrinfo(info);
    close();
    term();
  }

  template <typename FUNCTION>
  void lock(FUNCTION&& f) {
    RLock l(mutex);
    f(s, info);
  }

  void setOptions() {
    RLock l(mutex);
    if (s == InvalidSocket) {
      return;
    }

    int enable = 1;

#if !defined(_WIN32)
    // Prevent sockets lingering after process termination, causing
    // reconnection issues on the same port.
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(enable));

    struct {
      int l_onoff;  /* linger active */
      int l_linger; /* how many seconds to linger for */
    } linger = {false, 0};
    setsockopt(s, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger));
#endif  // !defined(_WIN32)

    // Enable TCP_NODELAY.
    // DAP usually consists of small packet requests, with small packet
    // responses. When there are many frequent, blocking requests made,
    // Nagle's algorithm can dramatically limit the request->response rates.
    setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (char*)&enable, sizeof(enable));
  }

  // dap::ReaderWriter compliance
  bool isOpen() {
    {
      RLock l(mutex);
      if ((s != InvalidSocket) && !errored(s)) {
        return true;
      }
    }
    WLock lock(mutex);
    s = InvalidSocket;
    return false;
  }

  void close() {
    {
      RLock l(mutex);
      if (s != InvalidSocket) {
#if defined(_WIN32)
        closesocket(s);
#else
        ::shutdown(s, SHUT_RDWR);
#endif
      }
    }

    WLock l(mutex);
    if (s != InvalidSocket) {
#if !defined(_WIN32)
      ::close(s);
#endif
      s = InvalidSocket;
    }
  }

  size_t read(void* buffer, size_t bytes) {
    RLock lock(mutex);
    if (s == InvalidSocket) {
      return 0;
    }
    auto len =
        recv(s, reinterpret_cast<char*>(buffer), static_cast<int>(bytes), 0);
    return (len < 0) ? 0 : len;
  }

  bool write(const void* buffer, size_t bytes) {
    RLock lock(mutex);
    if (s == InvalidSocket) {
      return false;
    }
    if (bytes == 0) {
      return true;
    }
    return ::send(s, reinterpret_cast<const char*>(buffer),
                  static_cast<int>(bytes), 0) > 0;
  }

 private:
  addrinfo* const info;
  SOCKET s = InvalidSocket;
  RWMutex mutex;
};

namespace dap {

Socket::Socket(const char* address, const char* port)
    : shared(Shared::create(address, port)) {
  if (shared) {
    shared->lock([&](SOCKET socket, const addrinfo* info) {
      if (bind(socket, info->ai_addr, (int)info->ai_addrlen) != 0) {
        shared.reset();
        return;
      }

      if (listen(socket, 0) != 0) {
        shared.reset();
        return;
      }
    });
  }
}

std::shared_ptr<ReaderWriter> Socket::accept() const {
  std::shared_ptr<Shared> out;
  if (shared) {
    shared->lock([&](SOCKET socket, const addrinfo*) {
      if (socket != InvalidSocket) {
        init();
        out = std::make_shared<Shared>(::accept(socket, 0, 0));
        out->setOptions();
      }
    });
  }
  return out;
}

bool Socket::isOpen() const {
  if (shared) {
    return shared->isOpen();
  }
  return false;
}

void Socket::close() const {
  if (shared) {
    shared->close();
  }
}

std::shared_ptr<ReaderWriter> Socket::connect(const char* address,
                                              const char* port,
                                              uint32_t timeoutMillis) {
  auto shared = Shared::create(address, port);
  if (!shared) {
    return nullptr;
  }

  std::shared_ptr<ReaderWriter> out;
  shared->lock([&](SOCKET socket, const addrinfo* info) {
    if (socket == InvalidSocket) {
      return;
    }

    if (timeoutMillis == 0) {
      if (::connect(socket, info->ai_addr, (int)info->ai_addrlen) == 0) {
        out = shared;
      }
      return;
    }

    if (!setBlocking(socket, false)) {
      return;
    }

    auto res = ::connect(socket, info->ai_addr, (int)info->ai_addrlen);
    if (res == 0) {
      if (setBlocking(socket, true)) {
        out = shared;
      }
    } else {
      const auto microseconds = timeoutMillis * 1000;

      fd_set fdset;
      FD_ZERO(&fdset);
      FD_SET(socket, &fdset);

      timeval tv;
      tv.tv_sec = microseconds / 1000000;
      tv.tv_usec = microseconds - (tv.tv_sec * 1000000);
      res = select(static_cast<int>(socket + 1), nullptr, &fdset, nullptr, &tv);
      if (res > 0 && !errored(socket) && setBlocking(socket, true)) {
        out = shared;
      }
    }
  });

  if (!out) {
    return nullptr;
  }

  return out->isOpen() ? out : nullptr;
}

}  // namespace dap
