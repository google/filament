// Copyright 2022 The Dawn & Tint Authors
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

package cov

import (
	"bytes"
	"context"
	"fmt"
	"io"
	"net/http"
	"os"
	"path/filepath"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/fileutils"
)

// StartServer starts a localhost http server to display the coverage data.
// Calls started() when the server is started, and then blocks until the context is cancelled.
func StartServer(ctx context.Context, port int, covData []byte, started func() error) error {
	ctx, stop := context.WithCancel(ctx)

	url := fmt.Sprintf("http://localhost:%v/index.html", port)
	handler := http.NewServeMux()
	handler.HandleFunc("/index.html", func(w http.ResponseWriter, r *http.Request) {
		f, err := os.Open(filepath.Join(fileutils.DawnRoot(), "tools/src/cov/view-coverage.html"))
		if err != nil {
			fmt.Fprint(w, "file not found")
			w.WriteHeader(http.StatusNotFound)
			return
		}
		defer f.Close()
		io.Copy(w, f)
	})
	handler.HandleFunc("/coverage.dat", func(w http.ResponseWriter, r *http.Request) {
		io.Copy(w, bytes.NewReader(covData))
	})
	handler.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		rel := r.URL.Path
		if r.URL.Path == "" {
			http.Redirect(w, r, url, http.StatusSeeOther)
			return
		}
		if strings.Contains(rel, "..") {
			w.WriteHeader(http.StatusBadRequest)
			fmt.Fprint(w, "file path must not contain '..'")
			return
		}
		f, err := os.Open(filepath.Join(fileutils.DawnRoot(), r.URL.Path))
		if err != nil {
			w.WriteHeader(http.StatusNotFound)
			fmt.Fprintf(w, "file '%v' not found", r.URL.Path)
			return
		}
		defer f.Close()
		io.Copy(w, f)
	})
	handler.HandleFunc("/viewer.closed", func(w http.ResponseWriter, r *http.Request) {
		stop()
	})

	server := &http.Server{Addr: fmt.Sprint(":", port), Handler: handler}
	go server.ListenAndServe()

	if err := started(); err != nil {
		return err
	}

	<-ctx.Done()
	err := server.Shutdown(ctx)
	switch err {
	case nil, context.Canceled:
		return nil
	default:
		return err
	}
}
