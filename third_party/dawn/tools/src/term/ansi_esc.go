// Copyright 2023 The Dawn & Tint Authors
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

package term

import (
	"io"
	"os"

	"github.com/mattn/go-colorable"
	"github.com/mattn/go-isatty"
)

// ANSI escape sequences
const (
	Escape       = "\u001B["
	PositionLeft = Escape + "0G"
	Reset        = Escape + "0m"

	Bold = Escape + "1m"

	Red     = Escape + "31m"
	Green   = Escape + "32m"
	Yellow  = Escape + "33m"
	Blue    = Escape + "34m"
	Magenta = Escape + "35m"
	Cyan    = Escape + "36m"
	White   = Escape + "37m"
)

// CanUseAnsiEscapeSequences looks at the process's environment to determine
// whether its sensible to emit ansi-escape sequences.
func CanUseAnsiEscapeSequences() bool {
	if os.Getenv("TERM") != "dumb" &&
		((isatty.IsTerminal(os.Stdout.Fd()) && isatty.IsTerminal(os.Stderr.Fd())) ||
			(isatty.IsCygwinTerminal(os.Stdout.Fd()) && isatty.IsCygwinTerminal(os.Stderr.Fd()))) {
		if _, disable := os.LookupEnv("NO_COLOR"); !disable {
			return true
		}
	}
	return false
}

// NewAnsiWriter returns an io.WriteCloser that writes to os.Stdout and can
// consume ANSI escape sequences.
// The returned writer will ensure that concurrent writes are printed whole
// (no interleaving between Write()s).
// If enable is true, then ANSI escape sequences will be emitted
// If enable is false, then ANSI escape sequences will be stripped before
// writing to stdout.
// The returned io.WriteCloser must be closed to flush any pending data
func NewAnsiWriter(enable bool) io.WriteCloser {
	// Create a thread-safe, color supporting stdout wrapper.
	if enable {
		return newMuxWriter(colorable.NewColorableStdout())
	} else {
		return newMuxWriter(colorable.NewNonColorable(os.Stdout))
	}
}

type muxWriter struct {
	data chan []byte
	err  chan error
}

// newMuxWriter returns a thread-safe io.WriteCloser, that writes to w
func newMuxWriter(w io.Writer) *muxWriter {
	m := muxWriter{
		data: make(chan []byte, 256),
		err:  make(chan error, 1),
	}
	go func() {
		defer close(m.err)
		for data := range m.data {
			_, err := w.Write(data)
			if err != nil {
				m.err <- err
				return
			}
		}
		m.err <- nil
	}()
	return &m
}

func (w *muxWriter) Write(data []byte) (n int, err error) {
	w.data <- append([]byte{}, data...)
	return len(data), nil
}

func (w *muxWriter) Close() error {
	close(w.data)
	return <-w.err
}
