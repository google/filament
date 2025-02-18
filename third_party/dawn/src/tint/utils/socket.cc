// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/utils/socket.h"

#include "src/tint/utils/macros/compiler.h"
#include "src/tint/utils/rwmutex.h"

#if TINT_BUILD_IS_WIN
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <cstdio>
#endif

#if TINT_BUILD_IS_WIN
#include <atomic>
namespace {
std::atomic<int> wsa_init_count = {0};
}  // anonymous namespace
#else
#include <fcntl.h>
namespace {
using SOCKET = int;
}  // anonymous namespace
#endif

TINT_BEGIN_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);

namespace tint::socket {
namespace {
constexpr SOCKET InvalidSocket = static_cast<SOCKET>(-1);
void Init() {
#if TINT_BUILD_IS_WIN
    if (wsa_init_count++ == 0) {
        WSADATA winsock_data;
        (void)WSAStartup(MAKEWORD(2, 2), &winsock_data);
    }
#endif
}

void Term() {
#if TINT_BUILD_IS_WIN
    if (--wsa_init_count == 0) {
        WSACleanup();
    }
#endif
}

bool SetBlocking(SOCKET s, bool blocking) {
#if TINT_BUILD_IS_WIN
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

bool Errored(SOCKET s) {
    if (s == InvalidSocket) {
        return true;
    }
    char error = 0;
    socklen_t len = sizeof(error);
    getsockopt(s, SOL_SOCKET, SO_ERROR, &error, &len);
    return error != 0;
}

class Impl : public Socket {
  public:
    static std::shared_ptr<Impl> create(const char* address, const char* port) {
        Init();

        addrinfo hints = {};
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;

        addrinfo* info = nullptr;
        auto err = getaddrinfo(address, port, &hints, &info);
#if !TINT_BUILD_IS_WIN
        if (err) {
            printf("getaddrinfo(%s, %s) error: %s\n", address, port, gai_strerror(err));
        }
#else
        (void)err;
#endif

        if (info) {
            auto socket = ::socket(info->ai_family, info->ai_socktype, info->ai_protocol);
            auto out = std::make_shared<Impl>(info, socket);
            out->SetOptions();
            return out;
        }

        Term();
        return nullptr;
    }

    explicit Impl(SOCKET socket) : info(nullptr), s(socket) {}

    Impl(addrinfo* i, SOCKET socket) : info(i), s(socket) {}

    ~Impl() override {
        if (info) {
            freeaddrinfo(info);
        }
        Close();
        Term();
    }

    template <typename FUNCTION>
    void Lock(FUNCTION&& f) {
        RLock l(mutex);
        f(s, info);
    }

    void SetOptions() {
        RLock l(mutex);
        if (s == InvalidSocket) {
            return;
        }

        int enable = 1;

#if !TINT_BUILD_IS_WIN
        // Prevent sockets lingering after process termination, causing
        // reconnection issues on the same port.
        setsockopt(s, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&enable), sizeof(enable));

        struct {
            int l_onoff;  /* linger active */
            int l_linger; /* how many seconds to linger for */
        } linger = {false, 0};
        setsockopt(s, SOL_SOCKET, SO_LINGER, reinterpret_cast<char*>(&linger), sizeof(linger));
#endif  // !TINT_BUILD_IS_WIN

        // Enable TCP_NODELAY.
        setsockopt(s, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&enable), sizeof(enable));
    }

    bool IsOpen() override {
        {
            RLock l(mutex);
            if ((s != InvalidSocket) && !Errored(s)) {
                return true;
            }
        }
        WLock lock(mutex);
        s = InvalidSocket;
        return false;
    }

    void Close() override {
        {
            RLock l(mutex);
            if (s != InvalidSocket) {
#if TINT_BUILD_IS_WIN
                closesocket(s);
#else
                ::shutdown(s, SHUT_RDWR);
#endif
            }
        }

        WLock l(mutex);
        if (s != InvalidSocket) {
#if !TINT_BUILD_IS_WIN
            ::close(s);
#endif
            s = InvalidSocket;
        }
    }

    size_t Read(void* buffer, size_t bytes) override {
        {
            RLock lock(mutex);
            if (s == InvalidSocket) {
                return 0;
            }
            auto len = recv(s, reinterpret_cast<char*>(buffer), bytes, 0);
            if (len > 0) {
                return static_cast<size_t>(len);
            }
        }
        // Socket closed or errored
        WLock lock(mutex);
        s = InvalidSocket;
        return 0;
    }

    bool Write(const void* buffer, size_t bytes) override {
        RLock lock(mutex);
        if (s == InvalidSocket) {
            return false;
        }
        if (bytes == 0) {
            return true;
        }
        return ::send(s, reinterpret_cast<const char*>(buffer), bytes, 0) > 0;
    }

    std::shared_ptr<Socket> Accept() override {
        std::shared_ptr<Impl> out;
        Lock([&](SOCKET socket, const addrinfo*) {
            if (socket != InvalidSocket) {
                Init();
                if (auto sock = ::accept(socket, nullptr, nullptr); s >= 0) {
                    out = std::make_shared<Impl>(sock);
                    out->SetOptions();
                }
            }
        });
        return out;
    }

  private:
    addrinfo* const info;
    SOCKET s = InvalidSocket;
    RWMutex mutex;
};

}  // anonymous namespace

Socket::~Socket() = default;

std::shared_ptr<Socket> Socket::Listen(const char* address, const char* port) {
    auto impl = Impl::create(address, port);
    if (!impl) {
        return nullptr;
    }
    impl->Lock([&](SOCKET socket, const addrinfo* info) {
        if (bind(socket, info->ai_addr, info->ai_addrlen) != 0) {
            impl.reset();
            return;
        }

        if (listen(socket, 0) != 0) {
            impl.reset();
            return;
        }
    });
    return impl;
}

std::shared_ptr<Socket> Socket::Connect(const char* address,
                                        const char* port,
                                        uint32_t timeout_ms) {
    auto impl = Impl::create(address, port);
    if (!impl) {
        return nullptr;
    }

    std::shared_ptr<Socket> out;
    impl->Lock([&](SOCKET socket, const addrinfo* info) {
        if (socket == InvalidSocket) {
            return;
        }

        if (timeout_ms == 0) {
            if (::connect(socket, info->ai_addr, info->ai_addrlen) == 0) {
                out = impl;
            }
            return;
        }

        if (!SetBlocking(socket, false)) {
            return;
        }

        auto res = ::connect(socket, info->ai_addr, info->ai_addrlen);
        if (res == 0) {
            if (SetBlocking(socket, true)) {
                out = impl;
            }
        } else {
            const auto timeout_us = timeout_ms * 1000;

            // Note: These macros introduce identifiers that start with `__`.
            TINT_BEGIN_DISABLE_WARNING(RESERVED_IDENTIFIER);
            fd_set fdset;
            FD_ZERO(&fdset);
            FD_SET(socket, &fdset);
            TINT_END_DISABLE_WARNING(RESERVED_IDENTIFIER);

            timeval tv;
            tv.tv_sec = timeout_us / 1000000;
            using USEC = decltype(tv.tv_usec);
            tv.tv_usec = static_cast<USEC>(timeout_us) - (static_cast<USEC>(tv.tv_sec) * 1000000);
            res = select(static_cast<int>(socket + 1), nullptr, &fdset, nullptr, &tv);
            if (res > 0 && !Errored(socket) && SetBlocking(socket, true)) {
                out = impl;
            }
        }
    });

    if (!out) {
        return nullptr;
    }

    return out->IsOpen() ? out : nullptr;
}

}  // namespace tint::socket

TINT_END_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);
