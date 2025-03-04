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

// Package gitiles provides helpers for interfacing with gitiles
package gitiles

import (
	"context"
	"fmt"
	"net/http"

	"go.chromium.org/luci/common/api/gitiles"
	gpb "go.chromium.org/luci/common/proto/gitiles"
)

// Gitiles is the client to communicate with Gitiles.
type Gitiles struct {
	client  gpb.GitilesClient
	project string
}

// New creates a client to communicate with Gitiles, for the given host and
// project.
func New(ctx context.Context, host, project string) (*Gitiles, error) {
	client, err := gitiles.NewRESTClient(http.DefaultClient, host, false)
	if err != nil {
		return nil, err
	}
	return &Gitiles{client, project}, nil
}

// Hash returns the git hash of the object with the given 'committish' reference.
func (g *Gitiles) Hash(ctx context.Context, ref string) (string, error) {
	res, err := g.client.Log(ctx, &gpb.LogRequest{
		Project:    g.project,
		Committish: ref,
		PageSize:   1,
	})
	if err != nil {
		return "", err
	}
	log := res.GetLog()
	if len(log) == 0 {
		return "", fmt.Errorf("gitiles returned log was empty")
	}
	return log[0].Id, nil
}

// DownloadFile downloads a single file with the given project-relative path at
// the given reference.
func (g *Gitiles) DownloadFile(ctx context.Context, ref, path string) (string, error) {
	res, err := g.client.DownloadFile(ctx, &gpb.DownloadFileRequest{
		Project:    g.project,
		Committish: ref,
		Path:       path,
	})
	if err != nil {
		return "", err
	}
	return res.GetContents(), nil
}

// ListFiles lists the file paths in a project-relative path at the given reference.
func (g *Gitiles) ListFiles(ctx context.Context, ref, path string) ([]string, error) {
	res, err := g.client.ListFiles(ctx, &gpb.ListFilesRequest{
		Project:    g.project,
		Committish: ref,
		Path:       path,
	})
	if err != nil {
		return []string{}, err
	}
	files := res.GetFiles()
	paths := make([]string, len(files))
	for i, f := range files {
		paths[i] = f.GetPath()
	}
	return paths, nil
}
