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
	"fmt"
	"path"
	"path/filepath"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/cmd/gen/common"
	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
)

// Project holds information about all the directories, targets and source files
// that makes up a project.
type Project struct {
	// The command line config
	cfg *common.Config
	// The absolute path to the root of the project
	Root string
	// A map of project-relative path to File.
	Files container.Map[string, *File]
	// A map of project-relative path to Directory.
	Directories container.Map[string, *Directory]
	// A map of target name to target.
	Targets container.Map[TargetName, *Target]
	// A list of external project dependencies used by the project
	externals container.Map[ExternalDependencyName, ExternalDependency]
	// Path to the 'externals.json' file
	externalsJsonPath string
}

// NewProject returns a newly initialized Project
func NewProject(root string, cfg *common.Config) *Project {
	return &Project{
		cfg:               cfg,
		Root:              root,
		Files:             container.NewMap[string, *File](),
		Directories:       container.NewMap[string, *Directory](),
		Targets:           container.NewMap[TargetName, *Target](),
		externals:         container.NewMap[ExternalDependencyName, ExternalDependency](),
		externalsJsonPath: filepath.Join(fileutils.DawnRoot(cfg.OsWrapper), "src", "tint", "externals.json"),
	}
}

// AddFile gets or creates a File with the given project-relative path
func (p *Project) AddFile(filepath string) *File {
	file := p.Files.GetOrCreate(filepath, func() *File {
		dir, name := path.Split(filepath)
		return &File{
			Directory:              p.Directory(dir),
			Name:                   name,
			TransitiveDependencies: NewDependencies(p),
		}
	})
	if file.IsGenerated {
		panic("AddFile() called with path that already exists for generated file")
	}
	return file
}

// AddGeneratedFile gets or creates a generated File with the given project-relative path
func (p *Project) AddGeneratedFile(filepath string) *File {
	file := p.Files.GetOrCreate(filepath, func() *File {
		dir, name := path.Split(filepath)
		return &File{
			IsGenerated:            true,
			Directory:              p.Directory(dir),
			Name:                   name,
			TransitiveDependencies: NewDependencies(p),
		}
	})
	if !file.IsGenerated {
		panic("AddGeneratedFile() called with path that already exists for non-generated file")
	}
	return file
}

// File returns the File with the given project-relative path
func (p *Project) File(file string) *File {
	return p.Files[file]
}

// AddTarget gets or creates a Target with the given Directory and TargetKind
func (p *Project) AddTarget(dir *Directory, kind TargetKind) *Target {
	name := p.TargetName(dir, kind)
	return p.Targets.GetOrCreate(name, func() *Target {
		t := &Target{
			Name:             name,
			Directory:        dir,
			Kind:             kind,
			SourceFileSet:    container.NewSet[string](),
			GeneratedFileSet: container.NewSet[string](),
			Dependencies:     NewDependencies(p),
		}
		dir.TargetNames.Add(name)
		p.Targets.Add(name, t)
		return t
	})
}

// Target returns the Target with the given Directory and TargetKind
func (p *Project) Target(dir *Directory, kind TargetKind) *Target {
	return p.Targets[p.TargetName(dir, kind)]
}

// TargetName returns the TargetName of a target in dir with the given kind
func (p *Project) TargetName(dir *Directory, kind TargetKind) TargetName {
	name := TargetName(dir.Path)
	if kind != targetLib {
		name += TargetName(fmt.Sprintf(":%v", kind))
	}
	return name
}

// AddDirectory gets or creates a Directory with the given project-relative path
func (p *Project) AddDirectory(path string) *Directory {
	path = CanonicalizePath(path)
	return p.Directories.GetOrCreate(path, func() *Directory {
		split := strings.Split(path, "/")
		d := &Directory{
			Project:           p,
			Name:              split[len(split)-1],
			Path:              path,
			TargetNames:       container.NewSet[TargetName](),
			SubdirectoryNames: container.NewSet[string](),
		}
		p.Directories[path] = d

		if path != "" {
			d.Parent = p.AddDirectory(strings.Join(split[:len(split)-1], "/"))
			d.Parent.SubdirectoryNames.Add(d.Name)
		}
		return d
	})
}

// Directory returns the Directory with the given project-relative path
func (p *Project) Directory(path string) *Directory {
	return p.Directories[CanonicalizePath(path)]
}

// CanonicalizePath canonicalizes the given path by changing path delimiters to
// '/' and removing any trailing slash
func CanonicalizePath(path string) string {
	return strings.TrimSuffix(filepath.ToSlash(path), "/")
}
