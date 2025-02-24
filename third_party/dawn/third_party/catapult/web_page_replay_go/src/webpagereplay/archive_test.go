// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package webpagereplay

import (
	"net/http"
	"net/http/httptest"
	"net/url"
	"testing"
)

func createArchivedRequest(t *testing.T, ustr string, header http.Header) *ArchivedRequest {
	req := httptest.NewRequest("GET", ustr, nil)
	req.Header = header
	resp := http.Response{StatusCode: 200}
	archivedRequest, err := serializeRequest(req, &resp)
	if err != nil {
		t.Fatalf("failed serialize request %s: %v", ustr, err)
		return nil
	}
	return archivedRequest
}

func validateTrim(t *testing.T, f func(req *http.Request, resp *http.Response) (bool, error), a Archive, expected int) {
	b, err := a.Trim(f)
	if err != nil {
		t.Fatalf("Trim returned an error: %v", err)
	}
	actual := len(b.Requests)
	if actual != expected {
		t.Fatalf("Expected %d request in archive b, found %d.", expected, actual)
	}
}

func TestFindRequestFuzzyMatching(t *testing.T) {
	a := newArchive()
	const u = "https://example.com/a/b/c/+/query?usegapi=1&foo=bar&c=d"
	const host = "example.com"
	req := createArchivedRequest(t, u, nil)
	a.Requests[host] = make(map[string][]*ArchivedRequest)
	a.Requests[host][u] = []*ArchivedRequest{req}
	prepareArchiveForReplay(&a)

	const newUrl = "https://example.com/a/b/c/+/query?usegapi=1&foo=yay&c=d&a=y"
	newReq := httptest.NewRequest("GET", newUrl, nil)

	_, foundResp, err := a.FindRequest(newReq)
	if err != nil {
		t.Fatalf("failed to find %s: %v", newUrl, err)
	}
	if got, want := foundResp.StatusCode, 200; got != want {
		t.Fatalf("status codes do not match. Got: %d. Want: %d", got, want)
	}
}

// Regression test for updating bestRatio when matching query params.
func TestFindClosest(t *testing.T) {
	a := newArchive()
	const host = "example.com"
	a.Requests[host] = make(map[string][]*ArchivedRequest)
	// Store three requests. u1 and u2 match equally well. u1 is chosen because
	// u1<u2.
	const u1 = "https://example.com/index.html?a=f&c=e"
	a.Requests[host][u1] = []*ArchivedRequest{createArchivedRequest(t, u1, nil)}

	const u2 = "https://example.com/index.html?a=g&c=e"
	a.Requests[host][u2] = []*ArchivedRequest{createArchivedRequest(t, u2, nil)}

	const u3 = "https://example.com/index.html?a=b&c=d"
	a.Requests[host][u3] = []*ArchivedRequest{createArchivedRequest(t, u3, nil)}

	prepareArchiveForReplay(&a)

	const newUrl = "https://example.com/index.html?c=e"
	newReq := httptest.NewRequest("GET", newUrl, nil)

	// Check that u1 is returned. FindRequest was previously non-deterministic,
	// due to random map iteration, so run the test several times.
	for i := 0; i < 10; i++ {
		foundReq, foundResp, err := a.FindRequest(newReq)
		if err != nil {
			t.Fatalf("failed to find %s: %v", newUrl, err)
		}
		if got, want := foundResp.StatusCode, 200; got != want {
			t.Fatalf("status codes do not match. Got: %d. Want: %d", got, want)
		}

		query := foundReq.URL.Query()
		if query.Get("a") != "f" || query.Get("c") != "e" {
			t.Fatalf("wrong request is matched\nexpected: %s\nactual: %s", u1, foundReq.URL)
		}
	}
}

// Regression test for https://github.com/catapult-project/catapult/issues/3727
func TestMatchHeaders(t *testing.T) {
	a := newArchive()
	const u = "https://example.com/mail/"
	const host = "example.com"
	headers := http.Header{}
	headers.Set("Accept", "text/html")
	headers.Set("Accept-Language", "en-Us,en;q=0.8")
	headers.Set("Accept-Encoding", "gzip, deflate, br")
	headers.Set("Cookie", "FOO=FOO")
	{
		req := createArchivedRequest(t, u, headers)
		a.Requests[host] = make(map[string][]*ArchivedRequest)
		a.Requests[host][u] = []*ArchivedRequest{req}
	}
	{
		headers.Set("Cookie", "FOO=BAR;SSID=XXhdfdf;LOGIN=HELLO")
		req := createArchivedRequest(t, u, headers)
		a.Requests[host][u] = append(a.Requests[host][u], req)
	}
	prepareArchiveForReplay(&a)

	newReq := httptest.NewRequest("GET", u, nil)
	newReq.Header = headers

	foundReq, _, err := a.FindRequest(newReq)
	if err != nil {
		t.Fatalf("failed to find %s: %v", u, err)
	}
	if got, want := foundReq.Header.Get("Cookie"), headers.Get("Cookie"); got != want {
		t.Fatalf("expected %s , actual %s\n", want, got)
	}
}

// When no header matches, the archived request with the same url should still be returned.
func TestNoHeadersMatch(t *testing.T) {
	a := newArchive()
	const u = "https://example.com/mail/"
	const host = "example.com"
	headers := http.Header{}
	headers.Set("Accept-Encoding", "gzip, deflate, br")
	req := createArchivedRequest(t, u, headers)
	a.Requests[host] = make(map[string][]*ArchivedRequest)
	a.Requests[host][u] = []*ArchivedRequest{req}
	prepareArchiveForReplay(&a)

	newReq := httptest.NewRequest("GET", u, nil)
	newReq.Header = headers
	newReq.Header.Set("Accept-Encoding", "gzip, deflate")

	foundReq, _, err := a.FindRequest(newReq)
	if err != nil {
		t.Fatalf("failed to find %s: %v", u, err)
	}
	if got, want := foundReq.Header.Get("Accept-Encoding"), "gzip, deflate, br"; got != want {
		t.Fatalf("expected %s , actual %s\n", want, got)
	}
}

// Test tie-breaking when an archive contains multiple responses for the same request.
func TestTieBreak(t *testing.T) {
	a := newArchive()
	const u = "https://example.com/mail/"
	const host = "example.com"
	const header1 = "1"
	const header2 = "2"
	{
		headers := http.Header{}
		headers.Set("matched", header1)
		req := createArchivedRequest(t, u, headers)
		a.Requests[host] = make(map[string][]*ArchivedRequest)
		a.Requests[host][u] = []*ArchivedRequest{req}
	}
	{
		headers := http.Header{}
		headers.Set("matched", header2)
		req := createArchivedRequest(t, u, headers)
		a.Requests[host][u] = append(a.Requests[host][u], req)
	}
	prepareArchiveForReplay(&a)

	newReq := httptest.NewRequest("GET", u, nil)
	newReq.Header = http.Header{}

	{
		foundReq, _, err := a.FindRequest(newReq)
		if err != nil {
			t.Fatalf("failed to find %s: %v", u, err)
		}
		if got, want := foundReq.Header.Get("matched"), header1; got != want {
			t.Fatalf("expected %s , actual %s\n", want, got)
		}
	}

	{
		foundReq, _, err := a.FindRequest(newReq)
		if err != nil {
			t.Fatalf("failed to find %s: %v", u, err)
		}
		if got, want := foundReq.Header.Get("matched"), header1; got != want {
			t.Fatalf("expected %s , actual %s\n", want, got)
		}
	}
}

// Test tie breaking in chronological order when an archive contains multiple responses for
// the same request.
func TestTieBreakChronologicalOrder(t *testing.T) {
	a := newArchive()
	const u = "https://example.com/mail/"
	const host = "example.com"
	const header1 = "1"
	const header2 = "2"
	{
		headers := http.Header{}
		headers.Set("matched", header1)
		req := createArchivedRequest(t, u, headers)
		a.Requests[host] = make(map[string][]*ArchivedRequest)
		a.Requests[host][u] = []*ArchivedRequest{req}
	}
	{
		headers := http.Header{}
		headers.Set("matched", header2)
		req := createArchivedRequest(t, u, headers)
		a.Requests[host][u] = append(a.Requests[host][u], req)
	}
	a.ServeResponseInChronologicalSequence = true
	prepareArchiveForReplay(&a)

	newReq := httptest.NewRequest("GET", u, nil)
	newReq.Header = http.Header{}

	{
		foundReq, _, err := a.FindRequest(newReq)
		if err != nil {
			t.Fatalf("failed to find %s: %v", u, err)
		}
		if got, want := foundReq.Header.Get("matched"), header1; got != want {
			t.Fatalf("expected %s , actual %s\n", want, got)
		}
	}

	{
		foundReq, _, err := a.FindRequest(newReq)
		if err != nil {
			t.Fatalf("failed to find %s: %v", u, err)
		}
		if got, want := foundReq.Header.Get("matched"), header2; got != want {
			t.Fatalf("expected %s , actual %s\n", want, got)
		}
	}

	// Test starting a new replay session to reset the serving sequence.
	{
		a.StartNewReplaySession()
		foundReq, _, err := a.FindRequest(newReq)
		if err != nil {
			t.Fatalf("failed to find %s: %v", u, err)
		}
		if got, want := foundReq.Header.Get("matched"), header1; got != want {
			t.Fatalf("expected %s , actual %s\n", want, got)
		}
	}
}

func TestMerge(t *testing.T) {
	a := newArchive()
	b := newArchive()
	const host = "example.com"
	a.Requests[host] = make(map[string][]*ArchivedRequest)
	b.Requests[host] = make(map[string][]*ArchivedRequest)
	// Create three requests with very closely matching params; only the first one
	// is different.
	const u1 = "https://example.com/index.html?a=AB&b=1&c=2"
	a.Requests[host][u1] = []*ArchivedRequest{createArchivedRequest(t, u1, nil)}
	b.Requests[host][u1] = []*ArchivedRequest{createArchivedRequest(t, u1, nil)}

	const u2 = "https://example.com/index.html?a=A&b=1&c=2"
	a.Requests[host][u2] = []*ArchivedRequest{createArchivedRequest(t, u2, nil)}

	const u3 = "https://example.com/index.html?a=B&b=1&c=2"
	b.Requests[host][u3] = []*ArchivedRequest{createArchivedRequest(t, u3, nil)}

	// Merging an archive with itself should yield the same results.
	if len(a.Requests[host]) != 2 {
		t.Fatalf("Expected 2 requests in archive a")
	}
	_ = a.Merge(&a)
	if len(a.Requests[host]) != 2 {
		t.Fatalf("Expected 2 requests in archive a")
	}

	if len(b.Requests[host]) != 2 {
		t.Fatalf("Expected 2 requests in archive b")
	}
	_ = b.Merge(&b)
	if len(b.Requests[host]) != 2 {
		t.Fatalf("Expected 2 requests in archive b")
	}

	// Merge b into a.
	_ = a.Merge(&b)
	if size := len(a.Requests[host]); size != 3 {
		t.Fatalf("Expected 3 requests in archive a but got %d", size)
	}
	if len(b.Requests[host]) != 2 {
		t.Fatalf("Expected 2 requests in archive b")
	}
}

func TestAdd(t *testing.T) {
	// generate a test server so we can capture and inspect the request
	testServer := httptest.NewServer(http.HandlerFunc(func(res http.ResponseWriter, req *http.Request) {
		res.Write([]byte("body"))
	}))
	defer func() { testServer.Close() }()

	a := newArchive()
	if len(a.Requests) != 0 {
		t.Fatalf("Expected empty archive")
	}

	requestURL, _ := url.Parse(testServer.URL)
	requestURLString := testServer.URL
	if err := a.Add("GET", requestURLString, AddModeAppend); err != nil {
		t.Fatalf("Error: %v", err)
	}

	requestMap := a.Requests[requestURL.Host]
	if len(requestMap) != 1 {
		t.Fatalf("Expected 1 requests in archive a")
	}
	if len(requestMap[requestURLString]) != 1 {
		t.Fatalf("Expected 1 requests in archive a")
	}

	requestURLString2 := requestURLString + "?q=something"
	if err := a.Add("GET", requestURLString2, AddModeAppend); err != nil {
		t.Fatalf("Error: %v", err)
	}
	if len(requestMap[requestURLString]) != 1 {
		t.Fatalf("Expected 1 requests in archive a")
	}
	if len(requestMap[requestURLString2]) != 1 {
		t.Fatalf("Expected 1 requests in archive a")
	}

	// Append another request with the URL
	if err := a.Add("GET", requestURLString2, AddModeAppend); err != nil {
		t.Fatalf("Error: %v", err)
	}
	if len(requestMap[requestURLString]) != 1 {
		t.Fatalf("Expected 1 requests in archive a")
	}
	if len(requestMap[requestURLString2]) != 2 {
		t.Fatalf("Expected 1 requests in archive a")
	}

	// Add but ignore existing request.
	requestURLString3 := requestURLString + "/request3/?q=something_else"
	if err := a.Add("GET", requestURLString2, AddModeSkipExisting); err != nil {
		t.Fatalf("Error: %v", err)
	}
	if len(requestMap[requestURLString]) != 1 {
		t.Fatalf("Expected 1 requests in archive a")
	}
	if len(requestMap[requestURLString2]) != 2 {
		t.Fatalf("Expected 1 requests in archive a")
	}
	if len(requestMap[requestURLString3]) != 0 {
		t.Fatalf("Expected 1 requests in archive a")
	}
	// Adding new requests works as usual.
	if err := a.Add("GET", requestURLString3, AddModeSkipExisting); err != nil {
		t.Fatalf("Error: %v", err)
	}
	if len(requestMap[requestURLString]) != 1 {
		t.Fatalf("Expected 1 requests in archive a")
	}
	if len(requestMap[requestURLString2]) != 2 {
		t.Fatalf("Expected 1 requests in archive a")
	}
	if len(requestMap[requestURLString3]) != 1 {
		t.Fatalf("Expected 1 requests in archive a")
	}

	// Add but overwrite existing ones
	requestURLString4 := requestURLString + "/request4/"
	if err := a.Add("GET", requestURLString2, AddModeOverwriteExisting); err != nil {
		t.Fatalf("Error: %v", err)
	}
	if len(requestMap[requestURLString]) != 1 {
		t.Fatalf("Expected 1 requests in archive a")
	}
	if len(requestMap[requestURLString2]) != 1 {
		t.Fatalf("Expected 1 requests in archive a")
	}
	if len(requestMap[requestURLString3]) != 1 {
		t.Fatalf("Expected 1 requests in archive a")
	}
	if len(requestMap[requestURLString4]) != 0 {
		t.Fatalf("Expected 1 requests in archive a")
	}
	// Adding new requests works as usual.
	if err := a.Add("GET", requestURLString4, AddModeOverwriteExisting); err != nil {
		t.Fatalf("Error: %v", err)
	}
	if len(requestMap[requestURLString]) != 1 {
		t.Fatalf("Expected 1 requests in archive a")
	}
	if len(requestMap[requestURLString2]) != 1 {
		t.Fatalf("Expected 1 requests in archive a")
	}
	if len(requestMap[requestURLString3]) != 1 {
		t.Fatalf("Expected 1 requests in archive a")
	}
	if len(requestMap[requestURLString4]) != 1 {
		t.Fatalf("Expected 1 requests in archive a")
	}

}

func TestTrim(t *testing.T) {
	a := newArchive()

	const host1 = "example.com"
	const host2 = "example.gov"
	const host3 = "example.org"
	const u1 = "https://example.com/index.html?a=A"
	const u2 = "https://example.gov/index.html?a=A"
	a.Requests[host1] = make(map[string][]*ArchivedRequest)
	a.Requests[host1][u1] = []*ArchivedRequest{createArchivedRequest(t, u1, nil)}
	a.Requests[host2] = make(map[string][]*ArchivedRequest)
	a.Requests[host2][u2] = []*ArchivedRequest{createArchivedRequest(t, u2, nil)}

	validateTrim(t, func(req *http.Request, resp *http.Response) (bool, error) { return true, nil }, a, 0)

	validateTrim(t, func(req *http.Request, resp *http.Response) (bool, error) { return false, nil }, a, 2)

	validateTrim(t, func(req *http.Request, resp *http.Response) (bool, error) { return req.Host == host1, nil }, a, 1)

	validateTrim(t, func(req *http.Request, resp *http.Response) (bool, error) { return req.Host == host2, nil }, a, 1)

	validateTrim(t, func(req *http.Request, resp *http.Response) (bool, error) { return req.Host == host3, nil }, a, 2)
}
