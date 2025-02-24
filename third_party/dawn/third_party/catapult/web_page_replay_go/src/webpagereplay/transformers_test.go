// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package webpagereplay

import (
	"bytes"
	"compress/gzip"
	"fmt"
	"io"
	"io/ioutil"
	"net/http"
	"strconv"
	"testing"
	"time"

	"github.com/kylelemons/godebug/pretty"
)

func TestReplaceTimeStamp(t *testing.T) {
	time_stamp_ms :=
		time.Date(2017, time.June, 1, 23, 0, 0, 0, time.UTC).Unix() * 1000
	replacements := map[string]string{
		"{{WPR_TIME_SEED_TIMESTAMP}}": strconv.FormatInt(time_stamp_ms, 10)}
	script := []byte("var time_seed = {{WPR_TIME_SEED_TIMESTAMP}};")
	transformer := NewScriptInjector(script, replacements)
	req := http.Request{}
	responseHeader := http.Header{
		"Content-Type": []string{"text/html"}}
	resp := http.Response{
		StatusCode: 200,
		Header:     responseHeader,
		Body:       ioutil.NopCloser(bytes.NewReader([]byte("<html></html>")))}
	transformer.Transform(&req, &resp)
	body, err := ioutil.ReadAll(resp.Body)
	resp.Body.Close()
	if err != nil {
		t.Fatal(err)
	}
	expectedContent := []byte(
		fmt.Sprintf("<html><script>var time_seed = %d;</script></html>",
			time_stamp_ms))
	if !bytes.Equal(expectedContent, body) {
		t.Fatal(
			fmt.Errorf("expected : %s \n actual: %s \n", expectedContent, body))
	}
}

// Regression test for https://github.com/catapult-project/catapult/issues/3726
func TestInjectScript(t *testing.T) {
	script := []byte("var foo = 1;")
	transformer := NewScriptInjector(script, nil)
	req := http.Request{}
	responseHeader := http.Header{
		"Content-Type": []string{"text/html"}}
	resp := http.Response{
		StatusCode: 200,
		Header:     responseHeader,
		Body: ioutil.NopCloser(bytes.NewReader([]byte("<html><head><script>" +
			"document.write('<head></head>');</script></head></html>")))}
	transformer.Transform(&req, &resp)
	body, err := ioutil.ReadAll(resp.Body)
	resp.Body.Close()
	if err != nil {
		t.Fatal(err)
	}
	expectedContent := []byte(fmt.Sprintf("<html><head><script>var foo = " +
		"1;</script><script>document.write('<head></head>');</script>" +
		"</head></html>"))
	if !bytes.Equal(expectedContent, body) {
		t.Fatal(
			fmt.Errorf("expected : %s \n actual: %s \n", expectedContent, body))
	}
}

func TestNoTagFound(t *testing.T) {
	script := []byte("var foo = 1;")
	transformer := NewScriptInjector(script, nil)
	req := http.Request{}
	responseHeader := http.Header{
		"Content-Type": []string{"text/html"}}
	resp := http.Response{
		StatusCode: 200,
		Header:     responseHeader,
		Body: ioutil.NopCloser(bytes.NewReader(
			[]byte("no tag random content")))}
	resp.Request = &req
	transformer.Transform(&req, &resp)
	body, err := ioutil.ReadAll(resp.Body)
	resp.Body.Close()
	if err != nil {
		t.Fatal(err)
	}
	expectedContent := []byte(fmt.Sprintf("no tag random content"))
	if !bytes.Equal(expectedContent, body) {
		t.Fatal(
			fmt.Errorf("expected : %s \n actual: %s \n", expectedContent, body))
	}
}

func TestInjectScriptToGzipResponse(t *testing.T) {
	script := []byte("var foo = 1;")
	transformer := NewScriptInjector(script, nil)
	req := http.Request{}
	responseHeader := http.Header{
		"Content-Type":     []string{"text/html"},
		"Content-Encoding": []string{"gzip"}}
	var gzippedBody bytes.Buffer
	gz := gzip.NewWriter(&gzippedBody)
	if _, err := gz.Write([]byte("<html></html>")); err != nil {
		t.Fatal(err)
	}
	if err := gz.Close(); err != nil {
		t.Fatal(err)
	}
	resp := http.Response{
		StatusCode: 200,
		Header:     responseHeader,
		Body:       ioutil.NopCloser(bytes.NewReader(gzippedBody.Bytes()))}
	transformer.Transform(&req, &resp)
	var reader io.ReadCloser
	var err error
	if reader, err = gzip.NewReader(resp.Body); err != nil {
		t.Fatal(err)
	}
	var body []byte
	if body, err = ioutil.ReadAll(reader); err != nil {
		t.Fatal(err)
	}
	reader.Close()
	expectedContent := []byte("<html><script>var foo = 1;</script></html>")
	if !bytes.Equal(expectedContent, body) {
		t.Fatal(
			fmt.Errorf("expected : %s \n actual: %s \n", expectedContent, body))
	}
}

func TestInjectScriptToResponse(t *testing.T) {
	tests := []struct {
		desc string
		input string
		want string
	}{
		{
			desc:	"With CSP Nonce script-src",
			input: "script-src 'strict-dynamic' 'nonce-2726c7f26c'",
			want:	"<html><head><script nonce=\"2726c7f26c\">var foo = 1;</script>" +
							"<script>document.write('<head></head>');</script></head></html>",
		},
		{
			desc:	"With CSP Nonce default-src",
			input: "default-src 'strict-dynamic' 'nonce-2726c7f26c'",
			want:	"<html><head><script nonce=\"2726c7f26c\">var foo = 1;</script>" +
							"<script>document.write('<head></head>');</script></head></html>",
		},
		{
			desc:	"With CSP Nonce and both Default and Script",
			input: "default-src 'self' https://foo.com;script-src 'strict-dynamic' 'nonce-2726cf26c'",
			want:	"<html><head><script nonce=\"2726cf26c\">var foo = 1;</script>" +
							"<script>document.write('<head></head>');</script></head></html>",
		},
		{
			desc:	"With CSP Nonce and both Default and Script override",
			input: "default-src 'self' 'nonce-99999cf26c';script-src 'strict-dynamic' 'nonce-2726cf26c'",
			want:	"<html><head><script nonce=\"2726cf26c\">var foo = 1;</script>" +
							"<script>document.write('<head></head>');</script></head></html>",
		},
	}

	for _, tc := range tests {
		script := []byte("var foo = 1;")
		transformer := NewScriptInjector(script, nil)
		req := http.Request{}
		responseHeader := http.Header{
			"Content-Type": []string{"text/html"},
			"Content-Security-Policy": []string{
				tc.input}}
		resp := http.Response{
			StatusCode: 200,
			Header:		 responseHeader,
			Body: ioutil.NopCloser(bytes.NewReader([]byte("<html><head><script>" +
				"document.write('<head></head>');</script></head></html>")))}
		transformer.Transform(&req, &resp)
		body, err := ioutil.ReadAll(resp.Body)
		resp.Body.Close()
		if err != nil {
			t.Fatal(err)
		}
		if diff := pretty.Compare(tc.want, string(body)); diff != "" {
			t.Errorf("TestInjectScript scenario `%s`\nreturned diff (-want +got):\n%s",
				tc.desc, diff)
		}
	}
}

func TestInjectScriptToResponseWithCspHash(t *testing.T) {
	script := []byte("var foo = 1;")
	transformer := NewScriptInjector(script, nil)
	req := http.Request{}
	responseHeader := http.Header{
		"Content-Type": []string{"text/html"},
		"Content-Security-Policy": []string{
			"script-src 'strict-dynamic' " +
			"'sha256-pwltXkdHyMvChFSLNauyy5WItOFOm+iDDsgqRTr8peI='"}}
	resp := http.Response{
		StatusCode: 200,
		Header:     responseHeader,
		Body: ioutil.NopCloser(bytes.NewReader([]byte("<html><head><script>" +
			"document.write('<head></head>');</script></head></html>")))}
	transformer.Transform(&req, &resp)
	assertEquals(t,
		resp.Header.Get("Content-Security-Policy"),
		"script-src 'strict-dynamic' " +
			"'sha256-HbDPY0FOc-FyUADaVWybbiLpgaRgtVUzWzQFo0YhKWc=' " +
			"'sha256-pwltXkdHyMvChFSLNauyy5WItOFOm+iDDsgqRTr8peI=' ")
}

func TestTransformCsp(t *testing.T) {
	tests := []struct {
		desc string
		input string
		inputSha string
		want string
	}{
		{
			desc:  "Just Script",
			input: "script-src 'self' https://foo.com;",
			want:  "script-src 'self' https://foo.com 'unsafe-inline';",
		},
		{
			desc:  "Just Default",
			input: "default-src 'self' https://foo.com;",
			want:  "default-src 'self' https://foo.com 'unsafe-inline';",
		},
		{
			desc:  "Both Script and Default Src",
			input: "default-src 'self' https://foo.com ; script-src 'self' 'nonce-2726c7f26c'",
			want:  "default-src 'self' https://foo.com ; script-src 'self' 'nonce-2726c7f26c'",
		},
		{
			desc:  "Both Script and Default Src No Nonce",
			input: "default-src 'self' https://foo.com ; script-src 'self'",
			want:  "default-src 'self' https://foo.com ; script-src 'self' 'unsafe-inline'",
		},
		{
			desc:  "Sha repeats",
			input: "script-src 'self' blob: https://foo.com 'sha256-XXX' 'sha384-XXX' https://foo2.com 'sha512-XXX', 'sha256-XX';",
			inputSha: "NEW",
			want: "script-src 'self' blob: https://foo.com 'sha256-NEW' 'sha256-XXX' 'sha384-XXX' https://foo2.com 'sha256-NEW' 'sha512-XXX', 'sha256-XX' ;",
		},
	}

	for _, tc := range tests {
		responseHeader := http.Header{"Content-Security-Policy": { tc.input } }
		transformCSPHeader(responseHeader, tc.inputSha)
		got := responseHeader.Get("Content-Security-Policy")
		if diff := pretty.Compare(tc.want, got); diff != "" {
			t.Errorf("TransformCsp scenario `%s`\n[input(%s)]\n returned diff (-want +got):\n%s",
				tc.desc, tc.input, diff)
		}
	}
}

func TestTransformMultipleCspEntries(t *testing.T) {
	tests := []struct {
		desc string
		input []string
		inputSha string
		want []string
	}{
		{
			desc:  "CSP single entry",
			input: []string {"script-src 'self' blob: https://foo.com 'sha256-XX1';"},
			inputSha: "NEW",
			want: []string{"script-src 'self' blob: https://foo.com 'sha256-NEW' 'sha256-XX1' ;"},
		},
		{
			desc:  "CSP first entry relevant",
			input: []string {"script-src 'self' blob: https://foo.com 'sha256-XX1';", "some other data"},
			inputSha: "NEW",
			want: []string { "script-src 'self' blob: https://foo.com 'sha256-NEW' 'sha256-XX1' ;", "some other data"},
		},
		{
			desc:  "Sha second entry relevant",
			input: []string { "some other data", "script-src 'self' blob: https://foo.com 'sha256-XX1';"},
			inputSha: "NEW",
			want: []string { "some other data", "script-src 'self' blob: https://foo.com 'sha256-NEW' 'sha256-XX1' ;"},
		},
		{
			desc:  "no CSP entry",
			input: []string {},
			inputSha: "NEW",
			want: []string {},
		},
	}

	for _, tc := range tests {
		responseHeader := http.Header{"Content-Security-Policy": tc.input }
		transformCSPHeader(responseHeader, tc.inputSha)
		got := responseHeader.Values("Content-Security-Policy")
		if diff := pretty.Compare(tc.want, got); diff != "" {
			t.Errorf("TransformCsp scenario `%s`\n[input(%s)]\n returned diff (-want +got):\n%s",
				tc.desc, tc.input, diff)
		}
	}
}

func assertEquals(t *testing.T, actual, expected string) {
	if expected != actual {
		t.Errorf("Expected \"%s\" but was \"%s\"", expected, actual)
	}
}
