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

package common

import (
	"context"
	"fmt"
	"log"
	"sort"
	"time"

	"dawn.googlesource.com/dawn/tools/src/buildbucket"
	"dawn.googlesource.com/dawn/tools/src/gerrit"
)

// BuildsByName is a map of builder name to build result
type BuildsByName map[string]buildbucket.Build

func (b BuildsByName) ids() []buildbucket.BuildID {
	ids := make([]buildbucket.BuildID, 0, len(b))
	for _, build := range b {
		ids = append(ids, build.ID)
	}
	return ids
}

// GetBuilds returns the builds, as declared in the config file, for the given
// patchset
func GetBuilds(
	ctx context.Context,
	cfg Config,
	ps gerrit.Patchset,
	bb *buildbucket.Buildbucket) (BuildsByName, error) {

	builds := BuildsByName{}

	err := bb.SearchBuilds(ctx, ps, func(build buildbucket.Build) error {
		for name, builder := range cfg.Builders {
			if build.Builder == builder {
				builds[name] = build
				break
			}
		}
		return nil
	})
	if err != nil {
		return nil, err
	}

	return builds, err
}

// WaitForBuildsToComplete waits until all the provided builds have finished.
func WaitForBuildsToComplete(
	ctx context.Context,
	cfg Config,
	ps gerrit.Patchset,
	bb *buildbucket.Buildbucket,
	builds BuildsByName) error {

	buildsStillRunning := func() []string {
		out := []string{}
		for name, build := range builds {
			if build.Status.Running() {
				out = append(out, name)
			}
		}
		sort.Strings(out)
		return out
	}

	for {
		// Refresh build status
		for name, build := range builds {
			build, err := bb.QueryBuild(ctx, build.ID)
			if err != nil {
				return fmt.Errorf("failed to query build for '%v': %w", name, err)
			}
			builds[name] = build
		}
		running := buildsStillRunning()
		if len(running) == 0 {
			break
		}
		log.Println("waiting for builds to complete: ", running)
		time.Sleep(time.Minute * 2)
	}

	for name, build := range builds {
		if build.Status == buildbucket.StatusInfraFailure ||
			build.Status == buildbucket.StatusCanceled {
			return fmt.Errorf("%v builder failed with %v", name, build.Status)
		}
	}

	return nil
}

// GetOrStartBuildsAndWait starts the builds as declared in the config file,
// for the given patchset, if they haven't already been started or if retest is
// true. GetOrStartBuildsAndWait then waits for the builds to complete, and then
// returns the results.
func GetOrStartBuildsAndWait(
	ctx context.Context,
	cfg Config,
	ps gerrit.Patchset,
	bb *buildbucket.Buildbucket,
	parentSwarmingRunId string,
	retest bool) (BuildsByName, error) {

	// Collect a list of one build per builder, prioritizing ones that completed
	// (Success/Failure) over ones that didn't (Started/InfraFailure/etc.)
	builds := BuildsByName{}

	if !retest {
		// Find any existing builds for the patchset
		err := bb.SearchBuilds(ctx, ps, func(build buildbucket.Build) error {
			// Find the config for that build
			for name, builder := range cfg.Builders {
				if build.Builder != builder {
					continue
				}

				// Prioritize based on whether the build should have results.
				switch build.Status {
				case buildbucket.StatusSuccess, buildbucket.StatusFailure:
					builds[name] = build
				default:
					if _, alreadyFound := builds[name]; !alreadyFound {
						builds[name] = build
					}
				}
				break
			}
			return nil
		})
		if err != nil {
			return nil, err
		}
	}

	// Returns true if the build should be re-kicked
	shouldKick := func(build buildbucket.Build) bool {
		switch build.Status {
		case buildbucket.StatusUnknown,
			buildbucket.StatusInfraFailure,
			buildbucket.StatusCanceled:
			return true
		}
		return false
	}

	// Kick any missing builds
	for name, builder := range cfg.Builders {
		if build, found := builds[name]; !found || shouldKick(build) {
			build, err := bb.StartBuild(ctx, ps, builder, parentSwarmingRunId, retest)
			if err != nil {
				return nil, err
			}
			log.Printf("started build: %+v\nLogs at: https://ci.chromium.org/ui/b/%v", build, build.ID)
			builds[name] = build
		}
	}

	if err := WaitForBuildsToComplete(ctx, cfg, ps, bb, builds); err != nil {
		return nil, err
	}

	return builds, nil
}
