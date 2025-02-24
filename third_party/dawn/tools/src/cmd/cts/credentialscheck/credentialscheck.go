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

package credentialscheck

import (
	"context"
	"flag"
	"fmt"

	commonAuth "dawn.googlesource.com/dawn/tools/src/auth"
	"dawn.googlesource.com/dawn/tools/src/cmd/cts/common"
	"dawn.googlesource.com/dawn/tools/src/gerrit"
	"dawn.googlesource.com/dawn/tools/src/gitiles"
	"go.chromium.org/luci/auth/client/authcli"
	"google.golang.org/api/sheets/v4"
)

func init() {
	common.Register(&cmd{})
}

type cmd struct {
	flags struct {
		auth authcli.Flags
	}
}

func (cmd) Name() string {
	return "credentials-check"
}

func (cmd) Desc() string {
	return "checks if all remote calls that require credentials work"
}

func (c *cmd) RegisterFlags(ctx context.Context, cfg common.Config) ([]string, error) {
	c.flags.auth.Register(
		flag.CommandLine, commonAuth.DefaultAuthOptions(cfg.OsWrapper, sheets.SpreadsheetsScope))
	return nil, nil
}

func (c *cmd) Run(ctx context.Context, cfg common.Config) error {
	auth, err := c.flags.auth.Options()
	if err != nil {
		return fmt.Errorf("failed to obtain authentication options: %w", err)
	}

	gerritConfig, err := gerrit.New(ctx, auth, cfg.Gerrit.Host)
	if err != nil {
		return err
	}

	gitilesConfig, err := gitiles.New(ctx, cfg.Git.Dawn.Host, cfg.Git.Dawn.Project)
	if err != nil {
		return err
	}

	inputs := common.CredCheckInputs{
		GerritConfig:  gerritConfig,
		GitilesConfig: gitilesConfig,
		Querier:       cfg.Querier,
	}

	err = common.CheckAllRequiredCredentials(ctx, inputs)
	if err != nil {
		return err
	}

	return nil
}
