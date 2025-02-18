// Copyright 2023 The Dawn & Tint Authors.
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

package build

import (
	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/transform"
)

// Dependencies describes the dependencies of a target
type Dependencies struct {
	// The project
	project *Project
	// Target names of all dependencies of this target
	internal container.Set[TargetName]
	// All external dependencies used by this target
	external container.Set[ExternalDependencyName]
}

// NewDependencies returns a new Dependencies
func NewDependencies(p *Project) *Dependencies {
	return &Dependencies{
		project:  p,
		internal: container.NewSet[TargetName](),
		external: container.NewSet[ExternalDependencyName](),
	}
}

// AddInternal adds dep to the list of internal dependencies
func (d *Dependencies) AddInternal(dep *Target) {
	d.internal.Add(dep.Name)
}

// AddExternal adds dep to the list of external dependencies
func (d *Dependencies) AddExternal(dep ExternalDependency) {
	d.external.Add(dep.Name)
}

// Internal returns the sorted list of dependencies of this target
func (d *Dependencies) Internal() []*Target {
	out := make([]*Target, len(d.internal))
	for i, name := range d.internal.List() {
		out[i] = d.project.Targets[name]
	}
	return out
}

// UnconditionalInternal returns the sorted list of dependencies that have no build condition.
func (d *Dependencies) UnconditionalInternal() []*Target {
	return transform.Filter(d.Internal(), func(d *Target) bool { return d.Condition == nil })
}

// External returns the sorted list of external dependencies.
func (d *Dependencies) External() []ExternalDependency {
	out := make([]ExternalDependency, 0, len(d.external))
	for _, name := range d.external.List() {
		out = append(out, d.project.externals[name])
	}
	return out
}

// ConditionalExternalDependencies returns the sorted list of external dependencies that have a
// build condition.
func (d *Dependencies) ConditionalExternal() []ExternalDependency {
	return transform.Filter(d.External(), func(e ExternalDependency) bool { return e.Condition != nil })
}

// ConditionalExternalDependencies returns the sorted list of external dependencies that have no
// build condition.
func (d *Dependencies) UnconditionalExternal() []ExternalDependency {
	return transform.Filter(d.External(), func(e ExternalDependency) bool { return e.Condition == nil })
}

// ContainsExternal returns true if the external dependencies contains name
func (d *Dependencies) ContainsExternal(name ExternalDependencyName) bool {
	return d.external.Contains(name)
}
