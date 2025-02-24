// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package webpagereplay

// Converts an old archive format to the new format. This file is
// temporary until crbug.com/730036 is fixed) and is used in
// tools/perf/convert_legacy_wpr_archive.

import (
	"bytes"
	"crypto/tls"
	"crypto/x509"
	"encoding/base64"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net"
	"net/http"
	"net/url"
	"os"
	"strconv"

	"github.com/urfave/cli/v2"
)

type ConvertorConfig struct {
	inputFile, outputFile string
	httpPort, httpsPort   int
	keyFile, certFile     string

	// Computed states
	tlsCert  tls.Certificate
	x509Cert *x509.Certificate
}

func (cfg *ConvertorConfig) Flags() []cli.Flag {
	return []cli.Flag{
		&cli.StringFlag{
			Name:        "https_cert_file",
			Value:       "wpr_cert.pem",
			Usage:       "File containing a PEM-encoded X509 certificate to use with SSL.",
			Destination: &cfg.certFile,
		},
		&cli.StringFlag{
			Name:        "https_key_file",
			Value:       "wpr_key.pem",
			Usage:       "File containing a PEM-encoded private key to use with SSL.",
			Destination: &cfg.keyFile,
		},
		&cli.StringFlag{
			Name:        "input_file",
			Destination: &cfg.inputFile,
		},
		&cli.StringFlag{
			Name:        "output_file",
			Destination: &cfg.outputFile,
		},
		&cli.IntFlag{
			Name:        "https_port",
			Value:       -1,
			Usage:       "Python WPR's https port.",
			Destination: &cfg.httpsPort,
		},
		&cli.IntFlag{
			Name:        "http_port",
			Value:       -1,
			Usage:       "Python WPR's http port.",
			Destination: &cfg.httpPort,
		},
	}
}

func (r *ConvertorConfig) recordServerCert(scheme string, serverName string, archive *WritableArchive) error {
	if scheme != "https" {
		return nil
	}
	derBytes, negotiatedProtocol, err := archive.Archive.FindHostTlsConfig(serverName)
	if err == nil && derBytes != nil {
		return err
	}
	derBytes, negotiatedProtocol, err = MintServerCert(serverName, r.x509Cert, r.tlsCert.PrivateKey)
	if err != nil {
		derBytes, negotiatedProtocol, err = MintDummyCertificate(serverName, r.x509Cert, r.tlsCert.PrivateKey)
		if err != nil {
			return err
		}
	}
	archive.RecordTlsConfig(serverName, derBytes, negotiatedProtocol)
	return nil
}

func (r *ConvertorConfig) Convert(c *cli.Context) {
	if r.httpPort == -1 || r.httpsPort == -1 {
		fmt.Printf("must provide ports of python WPR server")
		os.Exit(0)
	}
	file, err := ioutil.ReadFile(r.inputFile)
	if err != nil {
		panic(err)
	}
	fmt.Printf("Loading cert from %v\n", r.certFile)
	fmt.Printf("Loading key from %v\n", r.keyFile)
	r.tlsCert, err = tls.LoadX509KeyPair(r.certFile, r.keyFile)
	if err != nil {
		panic(fmt.Errorf("error opening cert or key files: %v", err))
	}
	r.x509Cert, err = x509.ParseCertificate(r.tlsCert.Certificate[0])
	if err != nil {
		panic(err)
	}
	transport := http.Transport{
		Dial: func(network, addr string) (net.Conn, error) {
			return net.Dial("tcp", fmt.Sprintf("127.0.0.1:%d", r.httpPort))
		},
		DialTLS: func(network, addr string) (net.Conn, error) {
			return tls.Dial(network,
				fmt.Sprintf("127.0.0.1:%d", r.httpsPort),
				&tls.Config{InsecureSkipVerify: true})
		},
	}
	archive, err := OpenWritableArchive(r.outputFile)
	if err != nil {
		panic(fmt.Errorf("cannot open: %v", err))
	}
	type JsonHeader struct {
		Key, Val string
	}
	type JsonRequest struct {
		Headers []JsonHeader
		Method  string
		Url     string
		Body    string
	}

	var requests []JsonRequest
	err = json.Unmarshal(file, &requests)
	if err != nil {
		panic(err)
	}
	for _, req := range requests {
		url, err := url.Parse(req.Url)
		if err != nil {
			panic(fmt.Errorf("failed: %v", err))
		}
		fmt.Printf("%v\n", url)
		reqHeaders := http.Header{}
		for _, h := range req.Headers {
			reqHeaders.Set(h.Key, h.Val)
			fmt.Printf("%s: %s\n", h.Key, h.Val)
		}
		httpReq := http.Request{Method: req.Method, URL: url}
		fmt.Printf("reqHeader %v\n", reqHeaders)
		var requestBody []byte
		if len(req.Body) == 0 {
			httpReq.ContentLength = 0
			reqHeaders.Set("content-length", "0")
		} else {
			requestBody, err = base64.StdEncoding.DecodeString(req.Body)
			if err != nil {
				panic(err)
			}
			httpReq.ContentLength = int64(len(requestBody))
			reqHeaders.Set("content-length", strconv.Itoa(len(requestBody)))
			httpReq.Body = ioutil.NopCloser(bytes.NewReader(requestBody))
		}
		httpReq.Host = url.Host
		httpReq.Header = reqHeaders
		httpReq.Proto = "HTTP/1.1"
		httpReq.ProtoMajor = 1
		httpReq.ProtoMinor = 1
		var resp *http.Response
		resp, err = transport.RoundTrip(&httpReq)
		if err != nil {
			panic(fmt.Errorf("RoundTrip failed: %v", err))
		}

		responseBody, err := ioutil.ReadAll(resp.Body)
		if err != nil {
			panic(fmt.Errorf("warning: origin response truncated: %v", err))
		}
		resp.Body.Close()
		fmt.Printf("status: %d\n", resp.StatusCode)
		if requestBody != nil {
			httpReq.Body = ioutil.NopCloser(bytes.NewReader(requestBody))
		}
		resp.Body = ioutil.NopCloser(bytes.NewReader(responseBody))
		if err := archive.RecordRequest(&httpReq, resp); err != nil {
			panic(fmt.Sprintf("failed recording request: %v", err))
		}
		if err := r.recordServerCert(url.Scheme, url.Host, archive); err != nil {
			// If cert fails to record, it usually because the host
			// is no longer reachable. Do not error out here.
			fmt.Printf("failed recording cert: %v", err)
		}
	}

	if err := archive.Close(); err != nil {
		fmt.Printf("Error flushing archive: %v", err)
	}
}
