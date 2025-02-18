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

#ifndef SRC_TINT_UTILS_SOCKET_H_
#define SRC_TINT_UTILS_SOCKET_H_

#include <stddef.h>
#include <stdint.h>

#include <memory>

namespace tint::socket {

/// Socket provides an OS abstraction to a TCP socket.
class Socket {
  public:
    /// Connects to the given TCP address and port.
    /// @param address the target socket address
    /// @param port the target socket port
    /// @param timeout_ms the timeout for the connection attempt.
    ///        If timeout_ms is non-zero and no connection was made before timeout_ms milliseconds,
    ///        then nullptr is returned.
    /// @returns the connected Socket, or nullptr on failure
    static std::shared_ptr<Socket> Connect(const char* address,
                                           const char* port,
                                           uint32_t timeout_ms);

    /// Begins listening for connections on the given TCP address and port.
    /// Call Accept() on the returned Socket to block and wait for a connection.
    /// @param address the socket address to listen on. Use "localhost" for connections from only
    ///        this machine, or an empty string to allow connections from any incoming address.
    /// @param port the socket port to listen on
    /// @returns the Socket that listens for connections
    static std::shared_ptr<Socket> Listen(const char* address, const char* port);

    /// Destructor
    virtual ~Socket();

    /// Attempts to read at most `n` bytes into buffer, returning the actual number of bytes read.
    /// read() will block until the socket is closed or at least one byte is read.
    /// @param buffer the output buffer. Must be at least `n` bytes in size.
    /// @param n the maximum number of bytes to read
    /// @return the number of bytes read, or 0 if the socket was closed or errored
    virtual size_t Read(void* buffer, size_t n) = 0;

    /// Writes `n` bytes from buffer into the socket.
    /// @param buffer the source data buffer. Must be at least `n` bytes in size.
    /// @param n the number of bytes to read from `buffer`
    /// @returns true on success, or false if there was an error or the socket was
    /// closed.
    virtual bool Write(const void* buffer, size_t n) = 0;

    /// @returns true if the socket has not been closed.
    virtual bool IsOpen() = 0;

    /// Closes the socket.
    virtual void Close() = 0;

    /// Blocks for a connection to be made to the listening port, or for the Socket to be closed.
    /// @returns a pointer to the next established incoming connection
    virtual std::shared_ptr<Socket> Accept() = 0;
};

}  // namespace tint::socket

#endif  // SRC_TINT_UTILS_SOCKET_H_
