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

import "path"

// Include describes a single #include in a file
type Include struct {
	Path      string
	Line      int
	Condition Condition
}

// File holds information about a source file
type File struct {
	// The file is generated from a target
	IsGenerated bool
	// The directory that holds this source file
	Directory *Directory
	// The target that this source file belongs to
	Target *Target
	// The name of the file
	Name string
	// An optional condition used to build this source file
	Condition Condition
	// All the #include made by this file
	Includes []Include
	// All the transitive dependencies of this file
	TransitiveDependencies *Dependencies
}

// Path returns the project-relative path of the file
func (f *File) Path() string {
	return path.Join(f.Directory.Path, f.Name)
}

// AbsPath returns the absolute path of the file
func (f *File) AbsPath() string {
	return path.Join(f.Directory.AbsPath(), f.Name)
}

// RemoveFromProject removes the File from the project
func (f *File) RemoveFromProject() {
	path := f.Path()
	f.Target.SourceFileSet.Remove(path)
	f.Directory.Project.Files.Remove(path)
}
