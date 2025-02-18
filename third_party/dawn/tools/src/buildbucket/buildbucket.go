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

// Package buildbucket provides helpers for interfacing with build-bucket
package buildbucket

import (
	"context"
	"fmt"
	"log"
	"net/url"

	"dawn.googlesource.com/dawn/tools/src/gerrit"
	"dawn.googlesource.com/dawn/tools/src/utils"
	"go.chromium.org/luci/auth"
	bbpb "go.chromium.org/luci/buildbucket/proto"
	"go.chromium.org/luci/grpc/prpc"
	"go.chromium.org/luci/hardcoded/chromeinfra"
)

// Buildbucket is the client to communicate with Buildbucket.
type Buildbucket struct {
	client bbpb.BuildsClient
}

// Builder describes a buildbucket builder
type Builder struct {
	Project string
	Bucket  string
	Builder string
}

// BuildID is a unique identifier of a build
type BuildID int64

// Build describes a buildbucket build
type Build struct {
	ID      BuildID
	Status  BuildStatus
	Builder Builder
}

// BuildStatus is the status of a build
type BuildStatus string

// Enumerator values for BuildStatus
const (
	// Unspecified state. Meaning depends on the context.
	StatusUnknown BuildStatus = "unknown"
	// Build was scheduled, but did not start or end yet.
	StatusScheduled BuildStatus = "scheduled"
	// Build/step has started.
	StatusStarted BuildStatus = "started"
	// A build/step ended successfully.
	// This is a terminal status. It may not transition to another status.
	StatusSuccess BuildStatus = "success"
	// A build/step ended unsuccessfully due to its Build.Input,
	// e.g. tests failed, and NOT due to a build infrastructure failure.
	// This is a terminal status. It may not transition to another status.
	StatusFailure BuildStatus = "failure"
	// A build/step ended unsuccessfully due to a failure independent of the
	// input, e.g. swarming failed, not enough capacity or the recipe was unable
	// to read the patch from gerrit.
	// start_time is not required for this status.
	// This is a terminal status. It may not transition to another status.
	StatusInfraFailure BuildStatus = "infra-failure"
	// A build was cancelled explicitly, e.g. via an RPC.
	// This is a terminal status. It may not transition to another status.
	StatusCanceled BuildStatus = "canceled"
)

// Running returns true if the build is still running
func (s BuildStatus) Running() bool {
	switch s {
	case StatusScheduled, StatusStarted:
		return true
	default:
		return false
	}
}

// pb returns a protobuf BuilderID constructed from the Builder
func (b Builder) pb() *bbpb.BuilderID {
	return &bbpb.BuilderID{
		Project: b.Project,
		Bucket:  b.Bucket,
		Builder: b.Builder,
	}
}

// toBuilder returns a Builder constructed from the protobuf BuilderID
func toBuilder(b *bbpb.BuilderID) Builder {
	return Builder{
		Project: b.Project,
		Bucket:  b.Bucket,
		Builder: b.Builder,
	}
}

// gerritChange returns the protobuf GerritChange from a gerrit.Patchset
func gerritChange(ps gerrit.Patchset) *bbpb.GerritChange {
	host := ps.Host
	if u, err := url.Parse(ps.Host); err == nil && u.Host != "" {
		host = u.Host // Strip scheme from URL
	}
	return &bbpb.GerritChange{
		Host:     host,
		Project:  ps.Project,
		Change:   int64(ps.Change),
		Patchset: int64(ps.Patchset),
	}
}

// toBuildStatus returns a BuildStatus from a protobuf Status
func toBuildStatus(s bbpb.Status) BuildStatus {
	switch s {
	default:
		return StatusUnknown
	case bbpb.Status_SCHEDULED:
		return StatusScheduled
	case bbpb.Status_STARTED:
		return StatusStarted
	case bbpb.Status_SUCCESS:
		return StatusSuccess
	case bbpb.Status_FAILURE:
		return StatusFailure
	case bbpb.Status_INFRA_FAILURE:
		return StatusInfraFailure
	case bbpb.Status_CANCELED:
		return StatusCanceled
	}
}

// toBuild returns a Build from a protobuf Build
func toBuild(b *bbpb.Build) Build {
	return Build{BuildID(b.Id), toBuildStatus(b.Status), toBuilder(b.Builder)}
}

// New creates a client to communicate with Buildbucket.
func New(ctx context.Context, credentials auth.Options) (*Buildbucket, error) {
	http, err := auth.NewAuthenticator(ctx, auth.InteractiveLogin, credentials).Client()
	if err != nil {
		return nil, err
	}
	client, err := bbpb.NewBuildsClient(
		&prpc.Client{
			C:       http,
			Host:    chromeinfra.BuildbucketHost,
			Options: prpc.DefaultOptions(),
		}), nil
	if err != nil {
		return nil, err
	}

	return &Buildbucket{client}, nil
}

// SearchBuilds queries the list of builds performed for the given gerrit change.
func (r *Buildbucket) SearchBuilds(ctx context.Context, ps gerrit.Patchset, f func(Build) error) error {
	pageToken := ""
	for {
		rsp, err := r.client.SearchBuilds(ctx, &bbpb.SearchBuildsRequest{
			Predicate: &bbpb.BuildPredicate{
				GerritChanges: []*bbpb.GerritChange{gerritChange(ps)},
			},
			PageSize:  1000, // Maximum page size.
			PageToken: pageToken,
		})
		if err != nil {
			return err
		}

		for _, res := range rsp.Builds {
			if err := f(toBuild(res)); err != nil {
				return err
			}
		}

		pageToken = rsp.GetNextPageToken()
		if pageToken == "" {
			// No more test variants with unexpected result.
			break
		}
	}

	return nil
}

// StartBuild starts a build.
func (r *Buildbucket) StartBuild(
	ctx context.Context,
	ps gerrit.Patchset,
	builder Builder,
	parentSwarmingRunID string,
	forceBuild bool) (Build, error) {

	id := ""
	if !forceBuild {
		id = utils.Hash(ps, builder)
	}

	req := &bbpb.ScheduleBuildRequest{
		RequestId:     id,
		Builder:       builder.pb(),
		GerritChanges: []*bbpb.GerritChange{gerritChange(ps)},
	}
	if parentSwarmingRunID != "" {
		req.Swarming = &bbpb.ScheduleBuildRequest_Swarming{
			ParentRunId: parentSwarmingRunID,
		}
	}

	build, err := r.client.ScheduleBuild(ctx, req)
	if err != nil {
		return Build{}, fmt.Errorf("failed to start build for patchset %+v on builder %+v: %w", ps, builder, err)
	}

	if status := toBuildStatus(build.Status); !forceBuild && !status.Running() {
		log.Printf("ScheduleBuild() returned with %v, attempting to force a retry...\n", status)
		return r.StartBuild(ctx, ps, builder, parentSwarmingRunID, true)
	}

	return toBuild(build), nil
}

// QueryBuild queries the status of a build.
func (r *Buildbucket) QueryBuild(ctx context.Context, id BuildID) (Build, error) {
	b, err := r.client.GetBuild(ctx, &bbpb.GetBuildRequest{Id: int64(id)})
	if err != nil {
		return Build{}, fmt.Errorf("failed to query build with id %v: %w", id, err)
	}
	return toBuild(b), nil
}
