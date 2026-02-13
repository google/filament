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

package chrome

import (
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"log"
	"net"
	"net/http"
	"path/filepath"
	"strings"
	"sync"
	"time"

	"dawn.googlesource.com/dawn/tools/src/cmd/run-cts/common"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/chromedp/cdproto/runtime"
	"github.com/chromedp/chromedp"
	"golang.org/x/net/websocket"
)

type flags struct {
	common.Flags
	chrome string
}

func init() {
	common.Register(&cmd{})
}

type cmd struct {
	flags flags
	state common.State
	query string
}

func (cmd) Name() string {
	return "chrome"
}

func (cmd) Desc() string {
	return "runs the CTS with chrome"
}

func (c *cmd) RegisterFlags(ctx context.Context, cfg common.Config) ([]string, error) {
	c.flags.Register(cfg.OsWrapper)
	flag.StringVar(&c.flags.chrome, "chrome", "", "path to the chrome executable")
	return []string{"[query]"}, nil
}

// TODO(crbug.com/416755658): Add unittest coverage once there is a way to fake
// the Chrome instance.
func (c *cmd) Run(ctx context.Context, cfg common.Config) error {
	state, err := c.flags.Process(cfg.OsWrapper)
	if err != nil {
		return err
	}
	c.state = *state

	// The test query is the optional unnamed argument
	c.query = "webgpu:*"
	switch len(flag.Args()) {
	case 0:
	case 1:
		c.query = flag.Args()[0]
	default:
		return fmt.Errorf("only a single query can be provided")
	}

	if err := c.state.CTS.Node.BuildIfRequired(c.flags.Verbose, cfg.OsWrapper); err != nil {
		return err
	}
	if err := c.state.CTS.Standalone.BuildIfRequired(c.flags.Verbose, cfg.OsWrapper); err != nil {
		return err
	}

	// Find all the test cases that match r.query.
	testCases, err := c.state.CTS.QueryTestCases(c.flags.Verbose, c.query)
	if err != nil {
		return fmt.Errorf("failed to gather tests: %w", err)
	}
	fmt.Printf("Testing %d test cases...\n", len(testCases))

	resultStream := make(chan common.Result, 256)
	go func() {
		c.runTestCases(ctx, testCases, resultStream, cfg.OsWrapper)
		close(resultStream)
	}()

	results, err := common.StreamResults(ctx,
		c.flags.Colors,
		c.state,
		c.flags.Verbose,
		/* coverage */ nil,
		len(testCases),
		resultStream,
		cfg.OsWrapper)
	if err != nil {
		return err
	}

	if err := c.state.Close(results); err != nil {
		return err
	}

	return nil
}

// TODO(crbug.com/416755658): Add unittest coverage once there is a way to fake
// the Chrome instance.
// runTestCases spawns c.flags.NumRunners Chrome instances to run all the test
// cases in testCases. The results of the tests are streamed to results.
// Blocks until all the tests have been run.
func (c *cmd) runTestCases(ctx context.Context, testCases []common.TestCase, results chan<- common.Result, fsReader oswrapper.FilesystemReader) {
	// Create a chan of test indices.
	// This will be read by the test runner goroutines.
	testCaseIndices := make(chan int, 256)
	go func() {
		for i := range testCases {
			testCaseIndices <- i
		}
		close(testCaseIndices)
	}()

	// Spin up the test runner goroutines
	wg := &sync.WaitGroup{}
	for i := 0; i < c.flags.NumRunners; i++ {
		id := i
		wg.Add(1)
		go func() {
			defer wg.Done()
			if err := c.runChromeInstance(ctx, id, testCases, testCaseIndices, results, fsReader); err != nil {
				results <- common.Result{
					Status: common.Fail,
					Error:  fmt.Errorf("Test server error: %w", err),
				}
			}
		}()
	}

	wg.Wait()
}

// TODO(crbug.com/416755658): Add unittest coverage once there is a way to fake
// the Chrome instance.
// runChromeInstance starts a Chrome instance, takes case indices from
// testCaseIndices, and requests the server run the test with the given index.
// The result of the test run is written to the results chan.
// Once the testCaseIndices chan has been closed and all taken tests have been
// completed, the Chrome instance is shutdown and runChromeInstance returns.
func (c *cmd) runChromeInstance(
	ctx context.Context,
	id int,
	testCases []common.TestCase,
	testCaseIndices <-chan int,
	results chan<- common.Result,
	fsReader oswrapper.FilesystemReader) error {

	port := 8800 + id

	type Request struct {
		Query     string `json:"q"`
		UseWorker bool   `json:"w"`
	}
	type Response struct {
		Type     string
		Message  string
		Log      string `json:"log"`
		Status   string
		Duration float64 `json:"js_duration_ms"`
	}
	requests := make(chan Request, 64)
	responses := make(chan Response, 64)
	errs := make(chan error, 64)

	handler := http.NewServeMux()
	handler.HandleFunc("/test_page.html", serveFile("webgpu-cts/test_page.html", fsReader))
	handler.HandleFunc("/test_runner.js", serveFile("webgpu-cts/test_runner.js", fsReader))
	handler.HandleFunc("/third_party/webgpu-cts/resources/",
		serveDir("/third_party/webgpu-cts/resources/", c.flags.CTS+"/out/resources/", fsReader))
	handler.HandleFunc("/third_party/webgpu-cts/src/",
		serveDir("/third_party/webgpu-cts/src/", c.flags.CTS+"/out/", fsReader))
	handler.HandleFunc("/", websocket.Handler(func(ws *websocket.Conn) {
		go func() {
			d := json.NewDecoder(ws)
			for {
				response := Response{}
				if err := d.Decode(&response); err == nil {
					responses <- response
				} else {
					errs <- err
					return
				}
			}
		}()
		e := json.NewEncoder(ws)
		for request := range requests {
			if err := e.Encode(request); err != nil {
				errs <- err
				return
			}
		}
	}).ServeHTTP)

	server := &http.Server{Addr: fmt.Sprint(":", port), Handler: handler}
	ln, err := net.Listen("tcp", server.Addr)
	if err != nil {
		return err
	}

	go func() {
		if c.flags.Verbose {
			fmt.Fprintf(c.state.Stdout, "[%v] listening on port %v\n", id, port)
		}
		if err := server.Serve(ln); err != nil {
			errs <- err
		}
	}()

	origin := fmt.Sprintf("http://localhost:%v", port)
	execOpts := []chromedp.ExecAllocatorOption{
		chromedp.NoDefaultBrowserCheck,
		chromedp.NoFirstRun,
		chromedp.Flag("unsafely-treat-insecure-origin-as-secure", origin),
		chromedp.Flag("enable-unsafe-webgpu", true),
		chromedp.Flag("enable-features", "Vulkan,UseSkiaRenderer"),
	}

	if c.flags.chrome != "" {
		execOpts = append(execOpts, chromedp.ExecPath(c.flags.chrome))
	}

	allocCtx, cancel := chromedp.NewExecAllocator(context.Background(), execOpts...)
	defer cancel()

	runCtx, cancel := chromedp.NewContext(allocCtx,
		chromedp.WithLogf(log.Printf),
		chromedp.WithErrorf(log.Printf))
	defer cancel()

	if c.flags.Verbose {
		chromedp.ListenTarget(runCtx, func(ev interface{}) {
			switch ev := ev.(type) {
			case *runtime.EventConsoleAPICalled:
				args := make([]string, len(ev.Args))
				for i := range ev.Args {
					args[i] = string(ev.Args[i].Value)
				}
				log.Println(ev.Type, strings.Join(args, " "))
			case *runtime.EventExceptionThrown:
				log.Println(ev.ExceptionDetails.Error())
			}
		})
	}
	if err := chromedp.Run(runCtx,
		chromedp.Navigate(origin+"/test_page.html"),
		chromedp.Evaluate(fmt.Sprintf("window.setupWebsocket(%v);", port),
			nil)); err != nil {
		return err
	}

nextTestCase:
	for idx := range testCaseIndices {
		res := common.Result{Index: idx, TestCase: testCases[idx]}
		if c.flags.Verbose {
			fmt.Println("Starting", res.TestCase)
		}
		requests <- Request{Query: string(res.TestCase)}

		for {
			// Future enhancements: Timeouts, browser restarting.
			select {
			case response := <-responses:
				switch response.Type {
				case "CONNECTION_ACK":
					if c.flags.Verbose {
						fmt.Fprintf(c.state.Stdout, "[%v] connected\n", id)
					}
				case "TEST_STARTED":
				case "TEST_HEARTBEAT":
				case "TEST_STATUS":
					res.Status = common.Status(response.Status)
					res.Duration = time.Duration(response.Duration) * time.Millisecond
				case "TEST_FINISHED":
					results <- res
					continue nextTestCase
				case "TEST_LOG":
					res.Message += response.Log

				default:
					return fmt.Errorf("unknown response type: %v", response.Type)
				}

			case err := <-errs:
				return err
			}
		}
	}

	if err := server.Shutdown(ctx); err != nil {
		return err
	}

	return nil
}

func serveFile(relPath string, fsReader oswrapper.FilesystemReader) func(http.ResponseWriter, *http.Request) {
	dawnRoot := fileutils.DawnRoot(fsReader)
	return func(w http.ResponseWriter, r *http.Request) {
		fullPath := filepath.Join(dawnRoot, relPath)
		if !fileutils.IsFile(fullPath, fsReader) {
			log.Printf("'%v' file does not exist", fullPath)
		}
		http.ServeFile(w, r, filepath.Join(dawnRoot, relPath))
	}
}

func serveDir(remote, local string, fsReader oswrapper.FilesystemReader) func(http.ResponseWriter, *http.Request) {
	return func(w http.ResponseWriter, r *http.Request) {
		fullPath := filepath.Join(local, strings.TrimPrefix(r.URL.Path, remote))
		if !fileutils.IsFile(fullPath, fsReader) {
			log.Printf("'%v' file does not exist", fullPath)
		}
		http.ServeFile(w, r, fullPath)
	}
}
