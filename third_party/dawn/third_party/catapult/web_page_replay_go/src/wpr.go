// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Program wpr records and replays web traffic.
package main

import (
	"crypto/tls"
	"errors"
	"fmt"
	"log"
	"net"
	"net/http"
	"os"
	"os/signal"
	"path/filepath"
	"strconv"
	"strings"
	"time"

	"github.com/catapult-project/catapult/web_page_replay_go/src/webpagereplay"
	"github.com/urfave/cli/v2"
	"golang.org/x/net/http2"
)

const longUsage = `
   %s [installroot|removeroot] [options]
   %s [record|replay] [options] archive_file

   Before: Install a test root CA.
     $ GOPATH=$PWD go run src/wpr.go installroot

   To record web pages:
     1. Start this program in record mode.
        $ GOPATH=$PWD go run src/wpr.go record archive.json
     2. Load the web pages you want to record in a web browser. It is important to
        clear browser caches before this so that all subresources are requested
        from the network.
     3. Kill the process to stop recording.

   To replay web pages:
     1. Start this program in replay mode with a previously recorded archive.
        $ GOPATH=$PWD go run src/wpr.go replay archive.json
     2. Load recorded pages in a web browser. A 404 will be served for any pages or
        resources not in the recorded archive.

   After: Remove the test root CA.
     $ GOPATH=$PWD go run src/wpr.go removeroot`

type CertConfig struct {
	// Flags common to all commands.
	certFile, keyFile, certType string
}

type CommonConfig struct {
	// Info about this command.
	cmd cli.Command

	// Flags common to RecordCommand and ReplayCommand.
	host                                     string
	httpPort, httpsPort, httpSecureProxyPort int
	certConfig                               CertConfig
	injectScripts                            string

	// Computed state.
	root_certs   []tls.Certificate
	transformers []webpagereplay.ResponseTransformer
}

type RecordCommand struct {
	common CommonConfig
	cmd    cli.Command
}

type ReplayCommand struct {
	common CommonConfig
	cmd    cli.Command

	// Custom flags for replay.
	rulesFile                            string
	serveResponseInChronologicalSequence bool
	quietMode                            bool
	disableFuzzyURLMatching              bool
}

type RootCACommand struct {
	certConfig CertConfig
	installer  webpagereplay.Installer
	cmd        cli.Command
}

func (certCfg *CertConfig) Flags() []cli.Flag {
	return []cli.Flag{
		&cli.StringFlag{
			Name:        "https_cert_file",
			Value:       "",
			Usage:       "File containing 1 or more comma separated PEM-encoded X509 certificates to use with SSL.",
			Destination: &certCfg.certFile,
		},
		&cli.StringFlag{
			Name:        "https_key_file",
			Value:       "",
			Usage:       "File containing 1 or more comma separated PEM-encoded private keys to use with SSL.",
			Destination: &certCfg.keyFile,
		},
	}
}

func (common *CommonConfig) Flags() []cli.Flag {
	return append(common.certConfig.Flags(),
		&cli.StringFlag{
			Name:        "host",
			Value:       "localhost",
			Usage:       "IP address to bind all servers to. Defaults to localhost if not specified.",
			Destination: &common.host,
		},
		&cli.IntFlag{
			Name:        "http_port",
			Value:       -1,
			Usage:       "Port number to listen on for HTTP requests, 0 to use any port, or -1 to disable.",
			Destination: &common.httpPort,
		},
		&cli.IntFlag{
			Name:        "https_port",
			Value:       -1,
			Usage:       "Port number to listen on for HTTPS requests, 0 to use any port, or -1 to disable.",
			Destination: &common.httpsPort,
		},
		&cli.IntFlag{
			Name:        "https_to_http_port",
			Value:       -1,
			Usage:       "Port number to listen on for HTTP proxy requests over an HTTPS connection, 0 to use any port, or -1 to disable.",
			Destination: &common.httpSecureProxyPort,
		},
		&cli.StringFlag{
			Name:  "inject_scripts",
			Value: "deterministic.js",
			Usage: "A comma separated list of JavaScript sources to inject in all pages. " +
				"By default a script is injected that eliminates sources of entropy " +
				"such as Date() and Math.random() deterministic. " +
				"CAUTION: Without deterministic.js, many pages will not replay.",
			Destination: &common.injectScripts,
		},
	)
}

func (certCfg *CertConfig) CheckArgs(c *cli.Context) error {
	if certCfg.certFile == "" && certCfg.keyFile == "" {
		certCfg.certFile = "wpr_cert.pem,ecdsa_cert.pem"
		certCfg.keyFile = "wpr_key.pem,ecdsa_key.pem"
	}
	return nil
}

func (common *CommonConfig) CheckArgs(c *cli.Context) error {
	if c.Args().Len() > 1 {
		return errors.New("too many args")
	}
	if c.Args().Len() != 1 {
		return errors.New("must specify archive_file")
	}
	if common.httpPort == -1 && common.httpsPort == -1 && common.httpSecureProxyPort == -1 {
		return errors.New("must specify at least one port flag")
	}

	err := common.certConfig.CheckArgs(c)
	if err != nil {
		return err
	}

	// Load certFiles.
	certFiles := strings.Split(common.certConfig.certFile, ",")
	keyFiles := strings.Split(common.certConfig.keyFile, ",")
	if len(certFiles) != len(keyFiles) {
		return fmt.Errorf("list of cert files given should match list of key files")
	}
	for i := 0; i < len(certFiles); i++ {
		log.Printf("Loading cert from %v\n", certFiles[i])
		log.Printf("Loading key from %v\n", keyFiles[i])
		root_cert, err := tls.LoadX509KeyPair(certFiles[i], keyFiles[i])
		if err != nil {
			return fmt.Errorf("error opening cert or key files: %v", err)
		}
		common.root_certs = append(common.root_certs, root_cert)
	}
	return nil
}

func (common *CommonConfig) ProcessInjectedScripts(timeSeedMs int64) error {
	if common.injectScripts != "" {
		for _, scriptFile := range strings.Split(common.injectScripts, ",") {
			log.Printf("Loading script from %v\n", scriptFile)
			// Replace {{WPR_TIME_SEED_TIMESTAMP}} with the time seed.
			replacements := map[string]string{"{{WPR_TIME_SEED_TIMESTAMP}}": strconv.FormatInt(timeSeedMs, 10)}
			si, err := webpagereplay.NewScriptInjectorFromFile(scriptFile, replacements)
			if err != nil {
				return fmt.Errorf("error opening script %s: %v", scriptFile, err)
			}
			common.transformers = append(common.transformers, si)
		}
	}

	return nil
}

func (r *RecordCommand) Flags() []cli.Flag {
	return r.common.Flags()
}

func (r *ReplayCommand) Flags() []cli.Flag {
	return append(r.common.Flags(),
		&cli.StringFlag{
			Name:        "rules_file",
			Value:       "",
			Usage:       "File containing rules to apply to responses during replay",
			Destination: &r.rulesFile,
		},
		&cli.BoolFlag{
			Name: "serve_response_in_chronological_sequence",
			Usage: "When an incoming request matches multiple recorded " +
				"responses, serve response in chronological sequence. " +
				"I.e. wpr responds to the first request with the first " +
				"recorded response, and the second request with the " +
				"second recorded response.",
			Destination: &r.serveResponseInChronologicalSequence,
		},
		&cli.BoolFlag{
			Name:        "disable_fuzzy_url_matching",
			Usage:       "When doing playback, require URLs to match exactly.",
			Destination: &r.disableFuzzyURLMatching,
		},
		&cli.BoolFlag{
			Name: "quiet_mode",
			Usage: "quiets the logging output by not logging the " +
				"ServeHTTP url call and responses",
			Destination: &r.quietMode,
		})
}

func (r *RootCACommand) Flags() []cli.Flag {
	return append(r.certConfig.Flags(),
		&cli.StringFlag{
			Name:        "android_device_id",
			Value:       "",
			Usage:       "Device id of an android device. Only relevant for Android",
			Destination: &r.installer.AndroidDeviceId,
		},
		&cli.StringFlag{
			Name:        "adb_binary_path",
			Value:       "adb",
			Usage:       "Path to adb binary. Only relevant for Android",
			Destination: &r.installer.AdbBinaryPath,
		},
		// Most desktop machines Google engineers use come with certutil installed.
		// In the chromium lab, desktop bots do not have certutil. Instead, desktop bots
		// deploy certutil binaries to <chromium src>/third_party/nss/certutil.
		// To accommodate chromium bots, the following flag accepts a custom path to
		// certutil. Otherwise WPR assumes that certutil resides in the PATH.
		&cli.StringFlag{
			Name:        "certutil_path",
			Value:       "certutil",
			Usage:       "Path to Network Security Services (NSS)'s certutil tool.",
			Destination: &r.installer.CertUtilBinaryPath,
		})
}

func getListener(host string, port int) (net.Listener, error) {
	addr, err := net.ResolveTCPAddr("tcp", fmt.Sprintf("%v:%d", host, port))
	if err != nil {
		return nil, err
	}
	return net.ListenTCP("tcp", addr)
}

// Copied from https://golang.org/src/net/http/server.go.
// This is to make dead TCP connections to eventually go away.
type tcpKeepAliveListener struct {
	*net.TCPListener
}

func (ln tcpKeepAliveListener) Accept() (c net.Conn, err error) {
	tc, err := ln.AcceptTCP()
	if err != nil {
		return
	}
	tc.SetKeepAlive(true)
	tc.SetKeepAlivePeriod(3 * time.Minute)
	return tc, nil
}

func startServers(tlsconfig *tls.Config, httpHandler, httpsHandler http.Handler, common *CommonConfig) {
	type Server struct {
		Scheme string
		Host   string
		Port   int
		*http.Server
	}

	servers := []*Server{}

	if common.httpPort > -1 {
		servers = append(servers, &Server{
			Scheme: "http",
			Host:   common.host,
			Port:   common.httpPort,
			Server: &http.Server{
				Addr:    fmt.Sprintf("%v:%v", common.host, common.httpPort),
				Handler: httpHandler,
			},
		})
	}
	if common.httpsPort > -1 {
		servers = append(servers, &Server{
			Scheme: "https",
			Host:   common.host,
			Port:   common.httpsPort,
			Server: &http.Server{
				Addr:      fmt.Sprintf("%v:%v", common.host, common.httpsPort),
				Handler:   httpsHandler,
				TLSConfig: tlsconfig,
			},
		})
	}
	if common.httpSecureProxyPort > -1 {
		servers = append(servers, &Server{
			Scheme: "https",
			Host:   common.host,
			Port:   common.httpSecureProxyPort,
			Server: &http.Server{
				Addr:      fmt.Sprintf("%v:%v", common.host, common.httpSecureProxyPort),
				Handler:   httpHandler, // this server proxies HTTP requests over an HTTPS connection
				TLSConfig: nil,         // use the default since this is as a proxy, not a MITM server
			},
		})
	}

	for _, s := range servers {
		s := s
		go func() {
			var ln net.Listener
			var err error
			switch s.Scheme {
			case "http":
				ln, err = getListener(s.Host, s.Port)
				if err != nil {
					break
				}
				logServeStarted(s.Scheme, ln)
				err = s.Serve(tcpKeepAliveListener{ln.(*net.TCPListener)})
			case "https":
				ln, err = getListener(s.Host, s.Port)
				if err != nil {
					break
				}
				logServeStarted(s.Scheme, ln)
				http2.ConfigureServer(s.Server, &http2.Server{})
				tlsListener := tls.NewListener(tcpKeepAliveListener{ln.(*net.TCPListener)}, s.TLSConfig)
				err = s.Serve(tlsListener)
			default:
				panic(fmt.Sprintf("unknown s.Scheme: %s", s.Scheme))
			}
			if err != nil {
				log.Printf("Failed to start server on %s://%s: %v", s.Scheme, s.Addr, err)
			}
		}()
	}

	log.Printf("Use Ctrl-C to exit.")
	select {}
}

func logServeStarted(scheme string, ln net.Listener) {
	log.Printf("Starting server on %s://%s", scheme, ln.Addr().String())
}

func (r *RecordCommand) Run(c *cli.Context) error {
	archiveFileName := c.Args().First()
	archive, err := webpagereplay.OpenWritableArchive(archiveFileName)
	if err != nil {
		cli.ShowSubcommandHelp(c)
		os.Exit(1)
	}
	defer archive.Close()
	log.Printf("Opened archive %s", archiveFileName)

	// Install a SIGINT handler to close the archive before shutting down.
	go func() {
		sigchan := make(chan os.Signal, 1)
		signal.Notify(sigchan, os.Interrupt)
		<-sigchan
		log.Printf("Shutting down")
		log.Printf("Writing archive file to %s", archiveFileName)
		if err := archive.Close(); err != nil {
			log.Printf("Error flushing archive: %v", err)
		}
		os.Exit(0)
	}()

	timeSeedMs := time.Now().Unix() * 1000
	if err := r.common.ProcessInjectedScripts(timeSeedMs); err != nil {
		log.Printf("Error processing injected scripts: %v", err)
		os.Exit(1)
	}
	archive.DeterministicTimeSeedMs = timeSeedMs

	httpHandler := webpagereplay.NewRecordingProxy(archive, "http", r.common.transformers)
	httpsHandler := webpagereplay.NewRecordingProxy(archive, "https", r.common.transformers)
	tlsconfig, err := webpagereplay.RecordTLSConfig(r.common.root_certs, archive)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error creating TLSConfig: %v", err)
		os.Exit(1)
	}
	startServers(tlsconfig, httpHandler, httpsHandler, &r.common)
	return nil
}

func (r *ReplayCommand) Run(c *cli.Context) error {
	archiveFileName := c.Args().First()
	log.Printf("Loading archive file from %s\n", archiveFileName)
	archive, err := webpagereplay.OpenArchive(archiveFileName)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error opening archive file: %v", err)
		os.Exit(1)
	}
	log.Printf("Opened archive %s", archiveFileName)

	archive.ServeResponseInChronologicalSequence = r.serveResponseInChronologicalSequence
	archive.DisableFuzzyURLMatching = r.disableFuzzyURLMatching
	if archive.DisableFuzzyURLMatching {
		log.Printf("Disabling fuzzy URL matching.")
	}

	timeSeedMs := archive.DeterministicTimeSeedMs
	if timeSeedMs == 0 {
		// The time seed hasn't been set in the archive. Time seeds used to not be
		// stored in the archive, so this is expected to happen when loading old
		// archives. Just revert to the previous behavior: use the current time as
		// the seed.
		timeSeedMs = time.Now().Unix() * 1000
	}
	if err := r.common.ProcessInjectedScripts(timeSeedMs); err != nil {
		log.Printf("Error processing injected scripts: %v", err)
		os.Exit(1)
	}

	if r.rulesFile != "" {
		t, err := webpagereplay.NewRuleBasedTransformer(r.rulesFile)
		if err != nil {
			fmt.Fprintf(os.Stderr, "Error opening rules file %s: %v\n", r.rulesFile, err)
			os.Exit(1)
		}
		r.common.transformers = append(r.common.transformers, t)
		log.Printf("Loaded replay rules from %s", r.rulesFile)
	}

	httpHandler := webpagereplay.NewReplayingProxy(archive, "http", r.common.transformers, r.quietMode)
	httpsHandler := webpagereplay.NewReplayingProxy(archive, "https", r.common.transformers, r.quietMode)
	tlsconfig, err := webpagereplay.ReplayTLSConfig(r.common.root_certs, archive)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error creating TLSConfig: %v", err)
		os.Exit(1)
	}
	startServers(tlsconfig, httpHandler, httpsHandler, &r.common)
	return nil
}

func (r *RootCACommand) Install(c *cli.Context) error {
	if err := r.installer.InstallRoot(
		r.certConfig.certFile, r.certConfig.keyFile); err != nil {
		fmt.Fprintf(os.Stderr, "Install root failed: %v", err)
		os.Exit(1)
	}
	return nil
}

func (r *RootCACommand) Remove(c *cli.Context) error {
	r.installer.RemoveRoot()
	return nil
}

func main() {
	progName := filepath.Base(os.Args[0])

	var record RecordCommand
	var replay ReplayCommand
	var installroot RootCACommand
	var removeroot RootCACommand

	record.cmd = cli.Command{
		Name:   "record",
		Usage:  "Record web pages to an archive",
		Flags:  record.Flags(),
		Before: record.common.CheckArgs,
		Action: record.Run,
	}

	replay.cmd = cli.Command{
		Name:   "replay",
		Usage:  "Replay a previously-recorded web page archive",
		Flags:  replay.Flags(),
		Before: replay.common.CheckArgs,
		Action: replay.Run,
	}

	installroot.cmd = cli.Command{
		Name:   "installroot",
		Usage:  "Install a test root CA",
		Flags:  installroot.Flags(),
		Before: installroot.certConfig.CheckArgs,
		Action: installroot.Install,
	}

	removeroot.cmd = cli.Command{
		Name:   "removeroot",
		Usage:  "Remove a test root CA",
		Flags:  removeroot.Flags(),
		Before: removeroot.certConfig.CheckArgs,
		Action: removeroot.Remove,
	}

	app := cli.NewApp()
	app.Commands = []*cli.Command{&record.cmd, &replay.cmd, &installroot.cmd, &removeroot.cmd}
	app.Usage = "Web Page Replay"
	app.UsageText = fmt.Sprintf(longUsage, progName, progName)
	app.HideVersion = true
	app.Version = ""
	app.Writer = os.Stderr
	app.RunAndExitOnError()
}
