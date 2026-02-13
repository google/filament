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

package export

import (
	"context"
	"flag"
	"fmt"
	"log"
	"os"
	"os/exec"
	"path/filepath"

	"dawn.googlesource.com/dawn/tools/src/auth"
	"dawn.googlesource.com/dawn/tools/src/cmd/cts/common"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/git"
	"dawn.googlesource.com/dawn/tools/src/gitiles"
	"go.chromium.org/luci/auth/client/authcli"
	"google.golang.org/api/sheets/v4"
)

func init() {
	common.Register(&cmd{})
}

type cmd struct {
	flags struct {
		auth     authcli.Flags
		results  common.ResultSource
		npmPath  string
		nodePath string
	}
}

func (cmd) Name() string {
	return "export"
}

func (cmd) Desc() string {
	return "exports the latest CTS results to Google sheets"
}

func (c *cmd) RegisterFlags(ctx context.Context, cfg common.Config) ([]string, error) {
	c.flags.auth.Register(
		flag.CommandLine,
		auth.DefaultAuthOptions(cfg.OsWrapper, sheets.SpreadsheetsScope))
	c.flags.results.RegisterFlags(cfg)
	npmPath, _ := exec.LookPath("npm")
	flag.StringVar(&c.flags.npmPath, "npm", npmPath, "path to npm")
	flag.StringVar(&c.flags.nodePath, "node", fileutils.NodePath(cfg.OsWrapper), "path to node")
	return nil, nil
}

// TODO(crbug.com/416731783): Add unittest coverage when there is a way to
// avoid network interactions from gitiles.
func (c *cmd) Run(ctx context.Context, cfg common.Config) error {
	// Validate command line arguments
	auth, err := c.flags.auth.Options()
	if err != nil {
		return fmt.Errorf("failed to obtain authentication options: %w", err)
	}

	// Grab the resultsByExecutionMode
	resultsByExecutionMode, err := c.flags.results.GetResults(ctx, cfg, auth)
	if err != nil {
		return err
	}
	if len(resultsByExecutionMode) == 0 {
		return fmt.Errorf("no results found")
	}

	// Note: GetResults() will update the Patchset to the latest roll, if not explicitly set.
	ps := c.flags.results.Patchset

	// Find the CTS revision
	dawn, err := gitiles.New(ctx, cfg.Git.Dawn.Host, cfg.Git.Dawn.Project)
	if err != nil {
		return fmt.Errorf("failed to open dawn host: %w", err)
	}
	deps, err := dawn.DownloadFile(ctx, ps.RefsChanges(), "DEPS")
	if err != nil {
		return fmt.Errorf("failed to download DEPS from %v: %w", ps.RefsChanges(), err)
	}
	_, ctsHash, err := common.UpdateCTSHashInDeps(deps, "<unused>", "<unused>")
	if err != nil {
		return fmt.Errorf("failed to find CTS hash in deps: %w", err)
	}

	log.Printf("checking out cts @ '%v'...", ctsHash)

	tmpDir, err := os.MkdirTemp("", "dawn-cts-export")
	if err != nil {
		return err
	}
	defer os.RemoveAll(tmpDir)

	ctsDir := filepath.Join(tmpDir, "cts")

	gitExe, err := exec.LookPath("git")
	if err != nil {
		return fmt.Errorf("failed to find git on PATH: %w", err)
	}

	git, err := git.New(gitExe)
	if err != nil {
		return err
	}

	log.Printf("cloning cts to '%v'...", ctsDir)
	repo, err := git.Clone(ctsDir, cfg.Git.CTS.HttpsURL(), nil)
	if err != nil {
		return fmt.Errorf("failed to clone cts: %v", err)
	}

	if _, err := repo.Fetch(ctsHash, nil); err != nil {
		return fmt.Errorf("failed to fetch cts: %v", err)
	}
	if err := repo.Checkout(ctsHash, nil); err != nil {
		return fmt.Errorf("failed to clone cts: %v", err)
	}

	return common.Export(ctx, auth, cfg.Sheets.ID, ctsDir, c.flags.nodePath, c.flags.npmPath, resultsByExecutionMode)
}
