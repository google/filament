// Copyright 2025 The Dawn & Tint Authors
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

package node

import (
	"bytes"
	"strings"
	"testing"
	"time"

	"github.com/stretchr/testify/require"
)

func TestPortListener_Write(t *testing.T) {
	tests := []struct {
		name          string
		inputs        [][]byte
		expectedPort  int
		expectPort    bool
		expectedError string
		expectedOut   string
	}{
		{
			name: "Port in single write",
			inputs: [][]byte{
				[]byte("Listening on [[12345]]"),
			},
			expectedPort: 12345,
			expectPort:   true,
			expectedOut:  "",
		},
		{
			name: "Port split across writes",
			inputs: [][]byte{
				[]byte("Listening on [[12"),
				[]byte("345]]"),
			},
			expectedPort: 12345,
			expectPort:   true,
			expectedOut:  "",
		},
		{
			name: "Port with trailing data after port found",
			inputs: [][]byte{
				[]byte("Listening on [[12345]] Some other output"),
			},
			expectedPort: 12345,
			expectPort:   true,
			expectedOut:  " Some other output",
		},
		{
			name: "No port in input",
			inputs: [][]byte{
				[]byte("Server starting..."),
				[]byte("Still no port."),
			},
			expectPort:  false,
			expectedOut: "",
		},
		{
			name: "Partial port start, no end",
			inputs: [][]byte{
				[]byte("Listening on [[12345"),
			},
			expectPort:  false,
			expectedOut: "",
		},
		{
			name: "Invalid port number",
			inputs: [][]byte{
				[]byte("Listening on [[abc]]"),
			},
			expectPort:    false,
			expectedError: `strconv.Atoi: parsing "abc": invalid syntax`,
			expectedOut:   "",
		},
		{
			name: "Empty port",
			inputs: [][]byte{
				[]byte("Listening on [[]]"),
			},
			expectPort:    false,
			expectedError: `strconv.Atoi: parsing "": invalid syntax`,
			expectedOut:   "",
		},
		{
			name: "Port at beginning of input",
			inputs: [][]byte{
				[]byte("[[7777]] Server ready"),
			},
			expectedPort: 7777,
			expectPort:   true,
			expectedOut:  " Server ready",
		},
		{
			name: "Writes after port found",
			inputs: [][]byte{
				[]byte("Listening on [[12345]]"),
				[]byte("More data"),
				[]byte(" and even more"),
			},
			expectedPort: 12345,
			expectPort:   true,
			expectedOut:  "More data and even more",
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			var outBuf bytes.Buffer
			pl := newPortListener(&outBuf)

			// Channel to receive the port or signal closure from the goroutine
			portResultChan := make(chan struct {
				port int
				ok   bool
			}, 1) // Buffer 1 to allow goroutine to send and exit without blocking

			// Start a goroutine to read from pl.port.
			// This is necessary because pl.port is unbuffered, and pl.Write will block
			// until the port is read.
			go func() {
				pVal, pOk := <-pl.port
				portResultChan <- struct {
					port int
					ok   bool
				}{pVal, pOk}
			}()

			var writeErr error
			for _, input := range tt.inputs {
				// If pl.Write sends to pl.port, the goroutine above will receive it,
				// unblocking pl.Write.
				_, currentErr := pl.Write(input)
				if currentErr != nil {
					writeErr = currentErr // Capture the first error
					break
				}
			}

			if tt.expectedError != "" {
				require.Error(t, writeErr, "Expected an error from Write")
				require.Contains(t, writeErr.Error(), tt.expectedError, "Error message mismatch")

				// If Write errored (e.g. strconv), pl.port might not be written to or closed by portListener.
				// The reading goroutine would block. We check portResultChan with a timeout.
				select {
				case res := <-portResultChan:
					t.Errorf("Unexpected port result (%+v) when Write error was expected", res)
				case <-time.After(50 * time.Millisecond):
					// Expected: goroutine is likely blocked as port was not sent/channel not closed due to error.
				}
			} else {
				require.NoError(t, writeErr, "Expected no error from Write")
				if tt.expectPort {
					select {
					case res := <-portResultChan:
						require.True(t, res.ok, "Expected to read a port, but channel was closed prematurely by sender")
						require.Equal(t, tt.expectedPort, res.port, "Port number mismatch")
					case <-time.After(100 * time.Millisecond): // Timeout for expected port
						t.Fatal("Timeout waiting for expected port")
					}
				} else { // Not expecting port
					select {
					case res := <-portResultChan:
						if res.ok {
							t.Errorf("Did not expect port, but got %d", res.port)
						} else {
							// This means pl.port was closed by sender. portListener.Write only closes *after* sending a port.
							t.Error("Did not expect port, but port channel was closed by sender (implies port was sent and read)")
						}
					case <-time.After(50 * time.Millisecond):
						// Expected: goroutine is likely blocked on <-pl.port as no port was sent and channel not closed.
					}
				}
			}
			require.Equal(t, tt.expectedOut, strings.TrimRight(outBuf.String(), " \n"))

			if tt.expectPort && tt.expectedError == "" {
				testStr := "subsequent write"
				_, subsequentWriteErr := pl.Write([]byte(testStr))
				require.NoError(t, subsequentWriteErr, "Error during subsequent write")
				require.Contains(t, strings.TrimRight(outBuf.String(), " \n"), testStr)
			}
		})
	}
}
