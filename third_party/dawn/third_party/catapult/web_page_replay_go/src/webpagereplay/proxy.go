// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package webpagereplay

import (
	"bytes"
	"crypto/tls"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"strconv"
	"strings"
	"time"
)

const errStatus = http.StatusInternalServerError

func makeLogger(req *http.Request, quietMode bool) func(msg string, args ...interface{}) {
	if quietMode {
		return func(string, ...interface{}) {}
	}
	prefix := fmt.Sprintf("ServeHTTP(%s): ", req.URL)
	return func(msg string, args ...interface{}) {
		log.Print(prefix + fmt.Sprintf(msg, args...))
	}
}

// fixupRequestURL adds a scheme and host to req.URL.
// Adding the scheme is necessary since RoundTrip doesn't like an empty scheme.
// Adding the host is optional, but makes req.URL print more nicely.
func fixupRequestURL(req *http.Request, scheme string) {
	req.URL.Scheme = scheme
	if req.URL.Host == "" {
		req.URL.Host = req.Host
	}
}

// updateDate is the basic function for date adjustment.
func updateDate(h http.Header, name string, now, oldNow time.Time) {
	val := h.Get(name)
	if val == "" {
		return
	}
	oldTime, err := http.ParseTime(val)
	if err != nil {
		return
	}
	newTime := now.Add(oldTime.Sub(oldNow))
	h.Set(name, newTime.UTC().Format(http.TimeFormat))
}

// updateDates updates "Date" header as current time and adjusts "Last-Modified"/"Expires" against it.
func updateDates(h http.Header, now time.Time) {
	oldNow, err := http.ParseTime(h.Get("Date"))
	h.Set("Date", now.UTC().Format(http.TimeFormat))
	if err != nil {
		return
	}
	updateDate(h, "Last-Modified", now, oldNow)
	updateDate(h, "Expires", now, oldNow)
}

// NewReplayingProxy constructs an HTTP proxy that replays responses from an archive.
// The proxy is listening for requests on a port that uses the given scheme (e.g., http, https).
func NewReplayingProxy(a *Archive, scheme string, transformers []ResponseTransformer, quietMode bool) http.Handler {
	return &replayingProxy{a, scheme, transformers, quietMode}
}

type replayingProxy struct {
	a            *Archive
	scheme       string
	transformers []ResponseTransformer
	quietMode    bool
}

func (proxy *replayingProxy) ServeHTTP(w http.ResponseWriter, req *http.Request) {
	if req.URL.Path == "/web-page-replay-generate-200" {
		w.WriteHeader(200)
		return
	}
	if req.URL.Path == "/web-page-replay-command-exit" {
		log.Printf("Shutting down. Received /web-page-replay-command-exit")
		os.Exit(0)
		return
	}
	if req.URL.Path == "/web-page-replay-reset-replay-chronology" {
		log.Printf("Received /web-page-replay-reset-replay-chronology")
		log.Printf("Reset replay order to start.")
		proxy.a.StartNewReplaySession()
		return
	}
	fixupRequestURL(req, proxy.scheme)
	logf := makeLogger(req, proxy.quietMode)

	// Lookup the response in the archive.
	_, storedResp, err := proxy.a.FindRequest(req)
	if err != nil {
		logf("couldn't find matching request: %v", err)
		w.WriteHeader(http.StatusNotFound)
		return
	}
	defer storedResp.Body.Close()

	// Check if the stored Content-Encoding matches an encoding allowed by the client.
	// If not, transform the response body to match the client's Accept-Encoding.
	clientAE := strings.ToLower(req.Header.Get("Accept-Encoding"))
	originCE := strings.ToLower(storedResp.Header.Get("Content-Encoding"))
	if !strings.Contains(clientAE, originCE) {
		logf("translating Content-Encoding [%s] -> [%s]", originCE, clientAE)
		body, err := ioutil.ReadAll(storedResp.Body)
		if err != nil {
			logf("error reading response body from archive: %v", err)
			w.WriteHeader(http.StatusNotFound)
			return
		}
		body, err = decompressBody(originCE, body)
		if err != nil {
			logf("error decompressing response body: %v", err)
			w.WriteHeader(http.StatusNotFound)
			return
		}
		body, ce, err := CompressBody(clientAE, body)
		if err != nil {
			logf("error recompressing response body: %v", err)
			w.WriteHeader(http.StatusNotFound)
			return
		}
		storedResp.Header.Set("Content-Encoding", ce)
		storedResp.Body = ioutil.NopCloser(bytes.NewReader(body))
		// ContentLength has changed, so update the outgoing headers accordingly.
		if storedResp.ContentLength >= 0 {
			storedResp.ContentLength = int64(len(body))
			storedResp.Header.Set("Content-Length", strconv.Itoa(len(body)))
		}
	}

	// Update dates in response header.
	updateDates(storedResp.Header, time.Now())

	// Transform.
	for _, t := range proxy.transformers {
		t.Transform(req, storedResp)
	}

	// Forward the response.
	logf("serving %v response", storedResp.StatusCode)
	for k, v := range storedResp.Header {
		w.Header()[k] = append([]string{}, v...)
	}
	w.WriteHeader(storedResp.StatusCode)
	if _, err := io.Copy(w, storedResp.Body); err != nil {
		logf("warning: client response truncated: %v", err)
	}
}

// NewRecordingProxy constructs an HTTP proxy that records responses into an archive.
// The proxy is listening for requests on a port that uses the given scheme (e.g., http, https).
func NewRecordingProxy(a *WritableArchive, scheme string, transformers []ResponseTransformer) http.Handler {
	http.DefaultTransport.(*http.Transport).TLSClientConfig = &tls.Config{InsecureSkipVerify: true}
	return &recordingProxy{http.DefaultTransport.(*http.Transport), a, scheme, transformers}
}

type recordingProxy struct {
	tr           *http.Transport
	a            *WritableArchive
	scheme       string
	transformers []ResponseTransformer
}

func (proxy *recordingProxy) ServeHTTP(w http.ResponseWriter, req *http.Request) {
	if req.URL.Path == "/web-page-replay-generate-200" {
		w.WriteHeader(200)
		return
	}
	if req.URL.Path == "/web-page-replay-command-exit" {
		log.Printf("Shutting down. Received /web-page-replay-command-exit")
		if err := proxy.a.Close(); err != nil {
			log.Printf("Error flushing archive: %v", err)
		}
		os.Exit(0)
		return
	}
	fixupRequestURL(req, proxy.scheme)
	logf := makeLogger(req, false)
	// https://github.com/golang/go/issues/16036. Server requests always
	// have non-nil body even for GET and HEAD. This prevents http.Transport
	// from retrying requests on dead reused conns. Catapult Issue 3706.
	if req.ContentLength == 0 {
		req.Body = nil
	}

	// TODO(catapult:3742): Implement Brotli support. Remove br advertisement for now.
	ce := req.Header.Get("Accept-Encoding")
	req.Header.Set("Accept-Encoding", strings.TrimSuffix(ce, ", br"))

	// Read the entire request body (for POST) before forwarding to the server
	// so we can save the entire request in the archive.
	var requestBody []byte
	if req.Body != nil {
		var err error
		requestBody, err = ioutil.ReadAll(req.Body)
		if err != nil {
			logf("read request body failed: %v", err)
			w.WriteHeader(errStatus)
			return
		}
		req.Body = ioutil.NopCloser(bytes.NewReader(requestBody))
	}

	// Make the external request.
	// If RoundTrip fails, convert the response to a 500.
	resp, err := proxy.tr.RoundTrip(req)
	if err != nil {
		logf("RoundTrip failed: %v", err)
		resp = &http.Response{
			Status:     http.StatusText(errStatus),
			StatusCode: errStatus,
			Proto:      req.Proto,
			ProtoMajor: req.ProtoMajor,
			ProtoMinor: req.ProtoMinor,
			Body:       ioutil.NopCloser(bytes.NewReader(nil)),
		}
	}

	// Copy the entire response body.
	responseBody, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		logf("warning: origin response truncated: %v", err)
	}
	resp.Body.Close()

	// Restore req body (which was consumed by RoundTrip) and record original response without transformation.
	resp.Body = ioutil.NopCloser(bytes.NewReader(responseBody))
	if req.Body != nil {
		req.Body = ioutil.NopCloser(bytes.NewReader(requestBody))
	}
	if err := proxy.a.RecordRequest(req, resp); err != nil {
		logf("failed recording request: %v", err)
	}

	// Restore req and response body which are consumed by RecordRequest.
	if req.Body != nil {
		req.Body = ioutil.NopCloser(bytes.NewReader(requestBody))
	}
	resp.Body = ioutil.NopCloser(bytes.NewReader(responseBody))

	// Transform.
	for _, t := range proxy.transformers {
		t.Transform(req, resp)
	}

	responseBodyAfterTransform, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		logf("warning: transformed response truncated: %v", err)
	}

	// Forward the response.
	logf("serving %d, %d bytes", resp.StatusCode, len(responseBodyAfterTransform))
	for k, v := range resp.Header {
		w.Header()[k] = append([]string{}, v...)
	}
	w.WriteHeader(resp.StatusCode)
	if n, err := io.Copy(w, bytes.NewReader(responseBodyAfterTransform)); err != nil {
		logf("warning: client response truncated (%d/%d bytes): %v", n, len(responseBodyAfterTransform), err)
	}
}
