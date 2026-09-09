// Copyright 2023 The Dawn & Tint Authors.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice, this
//     list of conditions and the following disclaimer.
//
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//  3. Neither the name of the copyright holder nor the names of its
//     contributors may be used to endorse or promote products derived from
//     this software without specific prior written permission.
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

// Package build implements a extensible build file list and module dependency
// generator for Tint.
// See: docs/tint/gen.md
package build

import (
	"bytes"
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"log"
	"os/exec"
	"path"
	"path/filepath"
	"regexp"
	"runtime"
	"sort"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/cmd/gen/common"
	"dawn.googlesource.com/dawn/tools/src/cnf"
	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/glob"
	"dawn.googlesource.com/dawn/tools/src/match"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"dawn.googlesource.com/dawn/tools/src/template"
	"dawn.googlesource.com/dawn/tools/src/transform"
	"github.com/mzohreva/gographviz/graphviz"
	"github.com/tidwall/jsonc"
)

const srcTint = "src/tint"

func init() {
	common.Register(&Cmd{})
}

type Cmd struct {
	flags struct {
		dot bool
	}
}

func (Cmd) Name() string {
	return "build"
}

func (Cmd) Desc() string {
	return `build generates BUILD.* files in each of Tint's source directories`
}

func (c *Cmd) RegisterFlags(ctx context.Context, cfg *common.Config) ([]string, error) {
	flag.BoolVar(&c.flags.dot, "dot", false, "emit GraphViz DOT files for each target kind")
	return nil, nil
}

// TODO(crbug.com/344014313): Add unittests once fileutils is converted to
// support dependency injection.
func (c Cmd) Run(ctx context.Context, cfg *common.Config) error {
	p := NewProject(CanonicalizePath(path.Join(fileutils.DawnRoot(cfg.OsWrapper), srcTint)), cfg)

	for _, stage := range []struct {
		desc string
		fn   func(p *Project, fsReaderWriter oswrapper.FilesystemReaderWriter) error
	}{
		{"loading 'externals.json'", loadExternals},
		{"populating source files", populateSourceFiles},
		{"scanning source files", scanSourceFiles},
		{"loading directory configs", applyDirectoryConfigs},
		{"applying implicit target conditions", applyImplicitTargetConditions},
		{"building dependencies", buildDependencies},
		{"checking for cycles", checkForCycles},
		{"emitting build files", emitBuildFiles},
	} {
		if cfg.Flags.Verbose {
			log.Printf("%v...\n", stage.desc)
		}
		if err := stage.fn(p, cfg.OsWrapper); err != nil {
			return err
		}
	}

	if c.flags.dot {
		for _, kind := range AllTargetKinds {
			if err := emitDotFile(p, kind, cfg.OsWrapper); err != nil {
				return err
			}
		}
	}

	if cfg.Flags.Verbose {
		log.Println("done")
	}
	return nil
}

// loadExternals loads the 'externals.json' file in this directory.
func loadExternals(p *Project, fsReaderWriter oswrapper.FilesystemReaderWriter) error {
	content, err := fsReaderWriter.ReadFile(p.externalsJsonPath)
	if err != nil {
		return err
	}

	externals := container.NewMap[string, struct {
		IncludePatterns []string
		Condition       string
	}]()
	if err := json.Unmarshal(jsonc.ToJSON(content), &externals); err != nil {
		return fmt.Errorf("failed to parse '%v': %w", p.externalsJsonPath, err)
	}

	for _, name := range externals.Keys() {
		external := externals[name]

		includePatternMatch := func(s string) bool { return false }
		if len(external.IncludePatterns) > 0 {
			matchers := []match.Test{}
			for _, pattern := range external.IncludePatterns {
				matcher, err := match.New(pattern)
				if err != nil {
					return fmt.Errorf("%v: matcher error: %w", p.externalsJsonPath, err)
				}
				matchers = append(matchers, matcher)
			}
			includePatternMatch = func(s string) bool {
				for _, matcher := range matchers {
					if matcher(s) {
						return true
					}
				}
				return false
			}
		}

		cond, err := cnf.Parse(external.Condition)
		if err != nil {
			return fmt.Errorf("%v: could not parse condition: %w",
				p.externalsJsonPath, err)
		}

		name := ExternalDependencyName(name)
		p.externals.Add(name, ExternalDependency{
			Name:                name,
			Condition:           cond,
			includePatternMatch: includePatternMatch,
		})
	}

	return nil
}

// Globs all the source files, and creates populates the Project with Directory, Target and File.
// File name patterns are used to bin each file into a target for the directory.
func populateSourceFiles(p *Project, fsReaderWriter oswrapper.FilesystemReaderWriter) error {
	paths, err := glob.Scan(p.Root, glob.MustParseConfig(`{
		"paths": [
			{
				"include": [
					"*/**.cc",
					"*/**.h",
					"*/**.inl",
					"*/**.mm",
					"*/**.proto",
					"*/**.tmpl"
				]
			},
			{
				"exclude": [
					"fuzzers/**"
				]
			}]
	}`), fsReaderWriter)
	if err != nil {
		return err
	}

	for _, filepath := range paths {
		filepath = CanonicalizePath(filepath)
		dir, name := path.Split(filepath)

		// Strip the .tmpl suffix if present for target kind determination.
		name = strings.TrimSuffix(name, ".tmpl")

		if kind := targetKindFromFilename(name); kind != targetInvalid {
			directory := p.AddDirectory(dir)
			target := p.AddTarget(directory, kind)

			if strings.HasSuffix(filepath, ".tmpl") {
				target.GeneratedSourcePaths.Add(path.Join(dir, name))
				target.AddTemplateFile(p.AddFile(filepath))
				p.AllTemplatePaths.Add(filepath)
			} else {
				target.AddSourceFile(p.AddFile(filepath))
			}

			if kind == targetProto {
				noExt, _ := fileutils.SplitExt(filepath)
				target.AddGeneratedProtobufSource(p.AddGeneratedProtobufSource(noExt + ".pb.h"))
				target.AddGeneratedProtobufSource(p.AddGeneratedProtobufSource(noExt + ".pb.cc"))
			}
		}
	}

	return nil
}

// TODO(crbug.com/344014313): Split this into multiple functions and add
// unittests.
// scanSourceFiles scans all the source files for:
// * #includes to build a dependencies between targets
// * 'GEN_BUILD:' directives
func scanSourceFiles(p *Project, fsReaderWriter oswrapper.FilesystemReaderWriter) error {
	// ParsedFile describes all the includes and conditions found in a source file
	type ParsedFile struct {
		removeFromProject bool
		conditions        []string
		includes          []Include
	}

	// parseFile parses the source file at 'path' represented by 'file'
	// As this is run concurrently, it must not modify any shared state (including file)
	parseFile := func(path string, file *File) (string, *ParsedFile, error) {
		if file.IsGeneratedProtobufSource {
			return "", nil, nil
		}

		body, err := fsReaderWriter.ReadFile(file.AbsPath())
		if err != nil {
			return path, nil, err
		}

		conditions := []Condition{}

		out := &ParsedFile{}
		for i, line := range strings.Split(string(body), "\n") {
			wrapErr := func(err error) error {
				return fmt.Errorf("%v:%v %w", file.Path(), i+1, err)
			}
			if match := reIgnoreFile.FindStringSubmatch(line); len(match) > 0 {
				out.removeFromProject = true
				continue
			}
			if match := reIf.FindStringSubmatch(line); len(match) > 0 {
				condition, err := cnf.Parse(strings.ToLower(match[1]))
				if err != nil {
					condition = Condition{{cnf.Unary{Var: "FAILED_TO_PARSE_CONDITION"}}}
				}
				if len(conditions) > 0 {
					condition = cnf.And(condition, conditions[len(conditions)-1])
				}
				conditions = append(conditions, condition)
			}
			if match := reIfdef.FindStringSubmatch(line); len(match) > 0 {
				conditions = append(conditions, Condition{})
			}
			if match := reIfndef.FindStringSubmatch(line); len(match) > 0 {
				conditions = append(conditions, Condition{})
			}
			if match := reElse.FindStringSubmatch(line); len(match) > 0 {
				if len(conditions) == 0 {
					return path, nil, wrapErr(fmt.Errorf("#else without #if"))
				}
				conditions[len(conditions)-1] = cnf.Not(conditions[len(conditions)-1])
			}
			if match := reElif.FindStringSubmatch(line); len(match) > 0 {
				condition, err := cnf.Parse(strings.ToLower(match[1]))
				if err != nil {
					condition = Condition{{cnf.Unary{Var: "FAILED_TO_PARSE_CONDITION"}}}
				}
				if len(conditions) == 0 {
					return path, nil, wrapErr(fmt.Errorf("#elif without #if"))
				}
				conditions[len(conditions)-1] = cnf.And(cnf.Not(conditions[len(conditions)-1]), condition)
			}
			if match := reEndif.FindStringSubmatch(line); len(match) > 0 {
				if len(conditions) == 0 {
					return path, nil, wrapErr(fmt.Errorf("#endif without #if"))
				}
				conditions = conditions[:len(conditions)-1]
			}
			if match := reCondition.FindStringSubmatch(line); len(match) > 0 {
				out.conditions = append(out.conditions, match[1])
			}
			if !reIgnoreInclude.MatchString(line) {
				for _, re := range []*regexp.Regexp{reInclude, reHashImport, reAtImport} {
					if match := re.FindStringSubmatch(line); len(match) > 0 {
						include := Include{
							Path: match[1],
							Line: i + 1,
						}
						if len(conditions) > 0 {
							include.Condition = conditions[len(conditions)-1]
						}
						out.includes = append(out.includes, include)
					}
				}
			}
		}
		return path, out, nil
	}

	// Create a new map by calling parseFile() on each entry of p.Files
	// This is performed over multiple concurrent goroutines.
	parsedFiles, err := transform.GoMap(p.Files, parseFile)
	if err != nil {
		return err
	}

	// For each file, of each target, of each directory...
	for _, dir := range p.Directories {
		for _, target := range dir.Targets() {
			for _, file := range append(target.SourceFiles(), target.TemplateFiles()...) {
				// Retrieve the parsed file information
				parsed := parsedFiles[file.Path()]

				if parsed.removeFromProject {
					file.RemoveFromProject()
					continue
				}

				// Apply any conditions
				for _, condition := range parsed.conditions {
					cond, err := cnf.Parse(condition)
					if err != nil {
						return fmt.Errorf("%v: could not parse condition: %w", file, err)
					}
					if file.Condition != nil {
						cond = cnf.Optimize(cnf.And(file.Condition, cond))
					}
					file.Condition = cond
				}

				file.Includes = append(file.Includes, parsed.includes...)
			}
		}
	}
	return nil
}

// TODO(crbug.com/344014313): Figure out a good way to unittest this since it
// is fairly complicated and appears to depend on populateSourceFiles() having
// succeeded already.
// applyDirectoryConfigs loads a 'BUILD.cfg' file in each source directory (if found), and
// applies the config to the Directory and/or Targets.
func applyDirectoryConfigs(p *Project, fsReaderWriter oswrapper.FilesystemReaderWriter) error {
	// For each directory in the project...
	for _, dir := range p.Directories.Values() {
		// Inherit condition from parent first.
		// By the time we visit a child, its parent has already been fully resolved and updated.
		if dir.Parent != nil && dir.Parent.Condition != nil {
			dir.Condition = dir.Parent.Condition
		}

		// Look for directory level configurations.
		path := path.Join(dir.AbsPath(), "BUILD.cfg")
		content, err := fsReaderWriter.ReadFile(path)
		var cfg DirectoryConfig
		if err == nil {
			if err := json.Unmarshal(jsonc.ToJSON(content), &cfg); err != nil {
				return fmt.Errorf("error while parsing '%v': %w", path, err)
			}

			// Apply any directory-level condition
			if cfg.Condition != "" {
				cond, err := cnf.Parse(cfg.Condition)
				if err != nil {
					return fmt.Errorf("%v: could not parse condition: %w", path, err)
				}
				if dir.Condition == nil {
					dir.Condition = cond
				} else {
					dir.Condition = cnf.Optimize(cnf.And(dir.Condition, cond))
				}
			}
		}

		// Apply the combined directory-level condition to all targets in this directory.
		if dir.Condition != nil {
			for _, target := range dir.Targets() {
				target.Condition = dir.Condition
			}
		}

		// For each target config...
		for _, tc := range []struct {
			cfg  *TargetConfig
			kind TargetKind
		}{
			{cfg.Lib, targetLib},
			{cfg.Proto, targetProto},
			{cfg.Test, targetTest},
			{cfg.TestCmd, targetTestCmd},
			{cfg.Bench, targetBench},
			{cfg.BenchCmd, targetBenchCmd},
			{cfg.Fuzz, targetFuzz},
			{cfg.FuzzCmd, targetFuzzCmd},
			{cfg.Cmd, targetCmd},
		} {
			if tc.cfg == nil {
				continue
			}
			target := p.Target(dir, tc.kind)
			if target == nil {
				return fmt.Errorf("%v: no files for target %v", path, tc.kind)
			}

			// Apply any custom output name
			target.OutputName = tc.cfg.OutputName

			// Assert that "test" GN target types have an output name set, as it will
			// be used for the target name. Having different target and output names
			// for tests can cause issues when trying to run tests on Swarming.
			// This should in theory apply to targetBenchCmd as well, but there are
			// currently no plans to run it on Swarming and there is a Chrome
			// dependency on the current target name that will need to be resolved
			// first.
			if tc.kind == targetTestCmd {
				if len(target.OutputName) == 0 {
					return fmt.Errorf(
						"Target of kind %v with cfg %v in dir %v does not contain OutputName",
						tc.kind, tc.cfg, dir.Path)
				}
			}

			if tc.cfg.Condition != "" {
				condition, err := cnf.Parse(tc.cfg.Condition)
				if err != nil {
					return fmt.Errorf("%v: %v", path, err)
				}
				if target.Condition == nil {
					target.Condition = condition
				} else {
					target.Condition = cnf.Optimize(cnf.And(target.Condition, condition))
				}
			}

			// Add any additional internal dependencies
			for _, depPattern := range tc.cfg.AdditionalDependencies.Internal {
				match, err := match.New(depPattern)
				if err != nil {
					return fmt.Errorf("%v: invalid pattern for '%v'.AdditionalDependencies.Internal.'%v': %w", path, tc.kind, depPattern, err)
				}
				additionalDeps := []*Target{}
				for _, target := range p.Targets.Keys() {
					if match(string(target)) {
						additionalDeps = append(additionalDeps, p.Targets[target])
					}
				}
				if len(additionalDeps) == 0 {
					return fmt.Errorf("%v: '%v'.AdditionalDependencies.Internal.'%v' did not match any targets", path, tc.kind, depPattern)
				}
				for _, dep := range additionalDeps {
					if dep != target {
						target.Dependencies.AddInternal(dep)
					}
				}
			}
			// Add any additional internal dependencies
			for _, name := range tc.cfg.AdditionalDependencies.External {
				dep, ok := p.externals[name]
				if !ok {
					return fmt.Errorf("%v: external dependency '%v'.AdditionalDependencies.External.'%v' not declared in '%v'",
						path, tc.kind, name, p.externalsJsonPath)
				}
				target.Dependencies.AddExternal(dep)
			}
		}
	}

	return nil
}

// applyImplicitTargetConditions applies implicit conditions to targets based on their kind
// (combining them with any existing conditions).
func applyImplicitTargetConditions(p *Project, _ oswrapper.FilesystemReaderWriter) error {
	fuzzCond, err := cnf.Parse("tint_build_fuzzers")
	if err != nil {
		return err
	}
	for _, target := range p.Targets.Values() {
		if target.Kind.IsFuzz() || target.Kind.IsFuzzCmd() {
			if target.Condition == nil {
				target.Condition = fuzzCond
			} else {
				target.Condition = cnf.Optimize(cnf.And(target.Condition, fuzzCond))
			}
		}
	}
	return nil
}

// checkInclude checks that the include statement is valid
// file is the file that contains the include
// include is the include statement
// includeCondition holds the required conditions for the include
func checkInclude(file *File, include Include, includeCondition Condition) error {
	noneIfEmpty := func(cond Condition) string {
		if len(cond) == 0 {
			return "<none>"
		}
		return cond.String()
	}
	sourceConditions := cnf.And(cnf.And(include.Condition, file.Condition), file.Target.Condition)
	targetConditions := includeCondition
	if missing := targetConditions.AssumeTrue(sourceConditions); len(missing) > 0 {
		return fmt.Errorf(`%v:%v #include "%v" requires guard: #if %v

%v build conditions: %v
%v build conditions: %v`,
			file.Path(), include.Line, include.Path, strings.ToUpper(missing.String()),
			file.Path(), noneIfEmpty(sourceConditions),
			include.Path, targetConditions,
		)
	}
	return nil
}

// TODO(crbug.com/344014313): Add unittests for this.
// buildDependencies walks all the #includes in all files, building the dependency information for
// all targets and files in the project. Errors if any cyclic includes are found.
func buildDependencies(p *Project, fsReaderWriter oswrapper.FilesystemReaderWriter) error {
	type state int
	const (
		unvisited state = iota
		visiting
		checked
	)

	cache := container.NewMap[string, state]()

	type FileInclude struct {
		file string
		inc  Include
	}

	var walk func(file *File, route []FileInclude) error
	walk = func(file *File, route []FileInclude) error {
		// Adds the dependency to the file and target's list of internal dependencies
		addInternalDependency := func(dep *Target) {
			file.TransitiveDependencies.AddInternal(dep)
			if file.Target != dep {
				file.Target.Dependencies.AddInternal(dep)
			}
		}
		// Adds the dependency to the file and target's list of external dependencies
		addExternalDependency := func(dep ExternalDependency) {
			file.TransitiveDependencies.AddExternal(dep)
			file.Target.Dependencies.AddExternal(dep)
		}

		filePath := file.Path()
		switch cache[filePath] {
		case unvisited:
			cache[filePath] = visiting

			for _, include := range file.Includes {
				if strings.HasPrefix(include.Path, srcTint) {
					// #include "src/tint/..."
					path := include.Path[len(srcTint)+1:] // Strip 'src/tint/'

					includeFile := p.File(path)
					if includeFile == nil {
						includeFile = p.File(path + ".tmpl")
						if includeFile == nil {
							return fmt.Errorf(`%v:%v includes non-existent file '%v'`, file.Path(), include.Line, path)
						}
					}

					if !isValidDependency(file.Target.Kind, includeFile.Target.Kind) {
						return fmt.Errorf(`%v:%v %v target must not include %v target`,
							file.Path(), include.Line, file.Target.Kind, includeFile.Target.Kind)
					}

					addInternalDependency(includeFile.Target)

					// Gather the dependencies for the included file
					if err := walk(includeFile, append(route, FileInclude{file: file.Path(), inc: include})); err != nil {
						return err
					}

					for _, dependency := range includeFile.TransitiveDependencies.Internal() {
						addInternalDependency(dependency)
					}
					for _, dependency := range includeFile.TransitiveDependencies.External() {
						addExternalDependency(dependency)
					}

					includeCondition := cnf.And(includeFile.Condition, includeFile.Target.Condition)
					if err := checkInclude(file, include, includeCondition); err != nil {
						return err
					}
				} else {
					// Check for external includes
					for _, external := range p.externals.Values() {
						if external.includePatternMatch(include.Path) {
							addExternalDependency(external)

							if err := checkInclude(file, include, external.Condition); err != nil {
								return err
							}
						}
					}
				}

			}

			cache[filePath] = checked

		case visiting:
			err := strings.Builder{}
			fmt.Fprintln(&err, "cyclic include found:")
			for _, include := range route {
				fmt.Fprintf(&err, "  %v:%v includes '%v'\n", include.file, include.inc.Line, include.inc.Path)
			}
			return fmt.Errorf("%s", err.String())
		}
		return nil
	}

	for _, file := range p.Files.Values() {
		if err := walk(file, []FileInclude{}); err != nil {
			return err
		}
	}
	return nil

}

// checkForCycles ensures that the graph of target dependencies are acyclic (a DAG)
func checkForCycles(p *Project, fsReaderWriter oswrapper.FilesystemReaderWriter) error {
	type state int
	const (
		unvisited state = iota
		visiting
		checked
	)

	cache := container.NewMap[TargetName, state]()

	var walk func(t *Target, path []TargetName) error
	walk = func(t *Target, path []TargetName) error {
		switch cache[t.Name] {
		case unvisited:
			cache[t.Name] = visiting
			for _, dep := range t.Dependencies.Internal() {
				if err := walk(dep, append(path, dep.Name)); err != nil {
					return err
				}
			}
			cache[t.Name] = checked
		case visiting:
			err := strings.Builder{}
			fmt.Fprintln(&err, "cyclic target dependency found:")
			for _, t := range path {
				fmt.Fprintln(&err, "  ", string(t))
			}
			fmt.Fprintln(&err, "  ", string(t.Name))
			return fmt.Errorf("%s", err.String())
		}
		return nil
	}

	for _, target := range p.Targets.Values() {
		if err := walk(target, []TargetName{target.Name}); err != nil {
			return err
		}
	}
	return nil
}

// platformOSMap maps Tint's build-flag variables to standard Bazel platform constraints.
// This makes it easier for clients to build w/o needing to set too many flags.
var platformOSMap = map[string]string{
	"tint_build_is_win":   "@platforms//os:windows",
	"tint_build_is_linux": "@platforms//os:linux",
	"tint_build_is_mac":   "@platforms//os:macos",
}

// externalDependencyBazelTargets maps external dependencies to their corresponding Bazel target
// labels. Supported dependencies map to non-empty strings, while unsupported dependencies map
// to empty strings.
var externalDependencyBazelTargets = map[string]string{
	"abseil":                         `"@abseil_cpp//absl/strings",`,
	"dxc-include":                    `"//third_party/directx-shader-compiler:dxc_headers",`,
	"gmock":                          `"@gtest",`,
	"google-benchmark":               `"@benchmark",`,
	"gtest":                          `"@gtest",`,
	"spirv-headers":                  `"@spirv_headers//:spirv_cpp11_headers", "@spirv_headers//:spirv_c_headers",`,
	"spirv-opt-internal":             `"@spirv_tools//:spirv_tools_opt",`,
	"spirv-tools":                    `"@spirv_tools",`,
	"src_utils_crash_handler":        `"//src/utils:crash_handler",`,
	"src_utils":                      `"//src/utils",`,
	"src_utils_chromium_test_compat": `"//src/utils/chromium_test_compat",`,
	// Explicitly mark unsupported dependencies as empty so they can be identified as unsupported
	// and we can omit a block if it only contains unsupported deps.
	"dl":                    "",
	"dxcompiler-for-fuzzer": "",
	"glslang-res-limits":    "",
	"glslang":               "",
	"mesa":                  "",
	"metal":                 "",
	"thread":                "",
	"winsock":               "",
}

// HasCppSrcs returns true if the conditional has any supported C++ files.
func HasCppSrcs(cond *TargetConditional) bool {
	for _, file := range cond.SourceFiles {
		// For now, it's easiest to just reject lists that contain only .mm files.
		if !strings.HasSuffix(file.Name, ".mm") {
			return true
		}
	}
	return false
}

// HasSupportedDeps returns true if the conditional has one or more deps supported by Bazel.
// If this returns false, then the block can be omitted from Bazel files.
func HasSupportedDeps(cond *TargetConditional) bool {
	if len(cond.InternalDependencies) > 0 {
		return true
	}
	for _, dep := range cond.ExternalDependencies {
		if externalDependencyBazelTargets[string(dep.Name)] != "" {
			return true
		}
	}
	return false
}

// ExternalDependencyTarget returns the Bazel target label for the given external dependency.
func ExternalDependencyTarget(dep ExternalDependency) (string, error) {
	target, ok := externalDependencyBazelTargets[string(dep.Name)]
	if !ok {
		return "", fmt.Errorf("unhandled external dependency '%v'", dep.Name)
	}
	return target, nil
}

// ConditionTargetLabel returns a Bazel target label for a build variable and optional negation.
// For platform constraints, it returns the standard @platforms//os target.
// For custom flags, it returns the global //src/tint config settings.
//
// Examples:
//
//	ConditionTargetLabel("tint_build_is_win", false)     => "@platforms//os:windows"
//	ConditionTargetLabel("tint_build_glsl_writer", true) => "//src/tint:tint_build_glsl_writer_false"
func ConditionTargetLabel(variable string, isNegated bool) string {
	if label, ok := platformOSMap[variable]; ok {
		return label
	}
	suffix := "_true"
	if isNegated {
		suffix = "_false"
	}
	return "//src/tint:" + variable + suffix
}

// ShouldSkipOrs returns true if the OR expression consists entirely of negated platform variables.
func ShouldSkipOrs(ors cnf.Ors) bool {
	for _, unary := range ors {
		if !unary.Negate {
			return false
		}
		if _, ok := platformOSMap[unary.Var]; !ok {
			return false
		}
	}
	return true
}

// ShouldSkipAnds returns true if the AND expression consists entirely of negated platform variables.
func ShouldSkipAnds(ands cnf.Ands) bool {
	for _, ors := range ands {
		for _, unary := range ors {
			if !unary.Negate {
				return false
			}
			if _, ok := platformOSMap[unary.Var]; !ok {
				return false
			}
		}
	}
	return true
}

// TODO(crbug.com/344014313): Add unittests once fileutils and template are
// converted to support dependency injection
// emitBuildFiles emits a 'BUILD.*' file in each source directory for each
// 'BUILD.*.tmpl' found in this directory.
func emitBuildFiles(p *Project, fsReaderWriter oswrapper.FilesystemReaderWriter) error {
	// Glob all the template files
	templatePaths, err := glob.Glob(path.Join(fileutils.ThisDir(), "*.tmpl"), fsReaderWriter)
	if err != nil {
		return err
	}
	if len(templatePaths) == 0 {
		return fmt.Errorf("no template files found")
	}

	// Load the templates
	templates := container.NewMap[string, *template.Template]()
	for _, path := range templatePaths {
		tmpl, err := template.FromFile(path, fsReaderWriter)
		if err != nil {
			return err
		}
		templates[path] = tmpl
	}

	// process executes all the templates for the directory dir
	// This is run concurrently, so must not modify shared state
	process := func(dir *Directory) (common.StaleFiles, error) {
		stale := common.StaleFiles{}

		// For each template...
		for _, tmplPath := range templatePaths {
			_, tmplName := filepath.Split(tmplPath)
			outputName := strings.TrimSuffix(tmplName, ".tmpl")
			outputPath := path.Join(dir.AbsPath(), outputName)

			// Attempt to read the existing output file
			existing, err := fsReaderWriter.ReadFile(outputPath)
			if err != nil {
				existing = nil
			}
			// If the file is annotated with a GEN_BUILD:DO_NOT_GENERATE directive, leave it alone
			if reDoNotGenerate.Match(existing) {
				continue
			}
			// Buffer for output
			w := &bytes.Buffer{}

			// Write the header
			relTmplPath, err := filepath.Rel(fileutils.DawnRoot(fsReaderWriter), tmplPath)
			if err != nil {
				return nil, err
			}
			w.WriteString(common.Header(string(existing), CanonicalizePath(relTmplPath), "#"))

			// Write the template output
			err = templates[tmplPath].Run(w, dir, map[string]any{
				"ShouldSkipUnary": func(unary cnf.Unary) bool {
					_, ok := platformOSMap[unary.Var]
					return ok
				},
				"ShouldSkipOrs":  ShouldSkipOrs,
				"ShouldSkipAnds": ShouldSkipAnds,
				"PlatformLabel": func(variable string) string {
					return platformOSMap[variable]
				},
				"HasCppSrcs":               HasCppSrcs,
				"HasSupportedDeps":         HasSupportedDeps,
				"ExternalDependencyTarget": ExternalDependencyTarget,
				"ConditionTargetLabel":     ConditionTargetLabel,
			})
			if err != nil {
				return nil, err
			}

			// Format the output if it's a GN file.
			if path.Ext(outputName) == ".gn" {
				unformatted := w.String()

				platform := ""
				cmd := "gn"
				switch runtime.GOOS {
				case "linux":
					platform = "linux64"
				case "windows":
					platform = "win"
					cmd += ".exe"
				case "darwin":
					platform = "mac"
				default:
					return nil, fmt.Errorf("unknown platform %v\n", runtime.GOOS)
				}

				gn_path := filepath.Join("buildtools", platform, cmd)

				gn := exec.Command(gn_path, "format", "--stdin")
				gn.Stdin = bytes.NewReader([]byte(unformatted))
				w.Reset()
				gn.Stdout = w
				gn.Stderr = w
				if err := gn.Run(); err != nil {
					return nil, fmt.Errorf("%v\ngn format failed: %w\n%v", unformatted, err, w.String())
				}
			}

			if string(existing) != w.String() {
				if !p.cfg.Flags.CheckStale {
					if err := fsReaderWriter.WriteFile(outputPath, w.Bytes(), 0666); err != nil {
						return nil, err
					}
				}

				stale = append(stale, outputPath)
			}

		}

		return stale, nil
	}

	// Concurrently run process() on all the directories.
	staleLists, err := transform.GoSlice(p.Directories.Values(), process)
	if err != nil {
		return err
	}

	if p.cfg.Flags.Verbose || p.cfg.Flags.CheckStale {
		// Collect all stale files into a flat list
		stale := transform.Flatten(staleLists)
		if p.cfg.Flags.CheckStale && len(stale) > 0 {
			return stale
		}
		if p.cfg.Flags.Verbose {
			log.Printf("generated %v files\n", len(stale))
		}
	}

	// Generate files that contain the inputs and/or outputs of the build-time source generation.
	generatedSources := p.AllTemplatePaths.List()
	for i, v := range generatedSources {
		generatedSources[i] = "src/tint/" + strings.TrimSuffix(v, ".tmpl")
	}
	depsForGeneratedSources, err := glob.Scan(p.Root, glob.MustParseConfig(`{
		"paths": [{"include": ["**/*.def", "utils/templates/*.inc"]}]
	}`), fsReaderWriter)
	if err != nil {
		return err
	}
	for i, dep := range depsForGeneratedSources {
		depsForGeneratedSources[i] = CanonicalizePath(dep)
	}
	depsForGeneratedSources = append(depsForGeneratedSources, p.AllTemplatePaths.List()...)

	goSources, err := getGoDependencies(p, fsReaderWriter)
	if err != nil {
		return err
	}

	if err := emitGeneratedSourceListForBazel(p, fsReaderWriter, generatedSources, depsForGeneratedSources); err != nil {
		return err
	}
	if err := emitGeneratedSourceListForGN(p, fsReaderWriter, generatedSources, depsForGeneratedSources, goSources); err != nil {
		return err
	}

	return nil
}

// emitDotFile writes a GraphViz DOT file visualizing the target dependency graph
func emitDotFile(p *Project, kind TargetKind, fsWriter oswrapper.FilesystemWriter) error {
	g := graphviz.Graph{}
	nodes := container.NewMap[TargetName, int]()
	targets := []*Target{}
	for _, target := range p.Targets.Values() {
		if target.Kind == kind {
			targets = append(targets, target)
		}
	}
	for _, target := range targets {
		nodes.Add(target.Name, g.AddNode(string(target.Name)))
	}
	for _, target := range targets {
		for _, dep := range target.Dependencies.Internal() {
			g.AddEdge(nodes[target.Name], nodes[dep.Name], "")
		}
	}

	g.MakeDirected()

	g.DefaultNodeAttribute(graphviz.Shape, graphviz.ShapeBox)
	g.DefaultNodeAttribute(graphviz.FontName, "Courier")
	g.DefaultNodeAttribute(graphviz.FontSize, "14")
	g.DefaultNodeAttribute(graphviz.Style, graphviz.StyleFilled+","+graphviz.StyleRounded)
	g.DefaultNodeAttribute(graphviz.FillColor, "yellow")

	g.DefaultEdgeAttribute(graphviz.FontName, "Courier")
	g.DefaultEdgeAttribute(graphviz.FontSize, "12")

	file, err := fsWriter.Create(path.Join(p.Root, fmt.Sprintf("%v.dot", kind)))
	if err != nil {
		return err
	}
	defer file.Close()

	g.GenerateDOT(file)
	return nil
}

func emitGeneratedSourceListForBazel(p *Project, fsReaderWriter oswrapper.FilesystemReaderWriter, generatedSources []string, depsForGeneratedSources []string) error {
	bzlPath := path.Join(p.Root, "generated_sources.bzl")
	sb := &strings.Builder{}
	sb.WriteString(common.Header("", "", "#"))
	sb.WriteString("\n")

	emitList(sb, generatedSources, "tint_generated_sources = [\n", "]\n\n", "    \"gen/", "\",\n")

	// Convert the list of dependencies into Bazel labels.
	var labels []string
	for _, dep := range depsForGeneratedSources {
		dir, base := path.Split(dep)
		dir = strings.TrimSuffix(dir, "/")
		labels = append(labels, fmt.Sprintf("//src/tint/%s:%s", dir, base))
	}
	emitList(sb, labels, "tint_generation_dependencies = [\n", "]\n", "    \"", "\",\n")

	return writeFileIfStale(p, fsReaderWriter, bzlPath, sb.String())
}

func emitGeneratedSourceListForGN(p *Project, fsReaderWriter oswrapper.FilesystemReaderWriter, generatedSources []string, depsForGeneratedSources []string, goSources []string) error {
	gniPath := path.Join(p.Root, "generated_sources.gni")
	sb := &strings.Builder{}
	sb.WriteString(common.Header("", "", "#"))
	sb.WriteString("\n")
	sb.WriteString("import(\"tint.gni\")\n\n")

	deps := append(depsForGeneratedSources, goSources...)
	emitList(sb, generatedSources, "tint_generated_sources = [\n", "]\n\n", "  \"${root_gen_dir}/", "\",\n")
	emitList(sb, deps, "tint_generation_dependencies = [\n", "]\n", "  \"", "\",\n")

	return writeFileIfStale(p, fsReaderWriter, gniPath, sb.String())
}

func writeFileIfStale(p *Project, fsReaderWriter oswrapper.FilesystemReaderWriter, filepath string, content string) error {
	existing, _ := fsReaderWriter.ReadFile(filepath)
	if string(existing) != content {
		if p.cfg.Flags.CheckStale {
			return common.StaleFiles{filepath}
		}
		if err := fsReaderWriter.WriteFile(filepath, []byte(content), 0666); err != nil {
			return err
		}
	}
	return nil
}

func emitList(sb *strings.Builder, items []string, prefix, suffix, itemPrefix, itemSuffix string) {
	sb.WriteString(prefix)
	for _, item := range items {
		sb.WriteString(itemPrefix)
		sb.WriteString(item)
		sb.WriteString(itemSuffix)
	}
	sb.WriteString(suffix)
}

var (
	// Regular expressions used by this file
	reIf            = regexp.MustCompile(`\s*#\s*if\s+(.*)`)
	reIfdef         = regexp.MustCompile(`\s*#\s*ifdef\s+(.*)`)
	reIfndef        = regexp.MustCompile(`\s*#\s*ifndef\s+(.*)`)
	reElse          = regexp.MustCompile(`\s*#\s*else\s+(.*)`)
	reElif          = regexp.MustCompile(`\s*#\s*elif\s+(.*)`)
	reEndif         = regexp.MustCompile(`\s*#\s*endif`)
	reInclude       = regexp.MustCompile(`\s*#\s*include\s*(?:\"|<)([^(\"|>)]+)(?:\"|>)`)
	reHashImport    = regexp.MustCompile(`\s*#\s*import\s*\<([\w\/\.]+)\>`)
	reAtImport      = regexp.MustCompile(`\s*@\s*import\s*(\w+)\s*;`)
	reIgnoreFile    = regexp.MustCompile(`//\s*GEN_BUILD:IGNORE_FILE`)
	reIgnoreInclude = regexp.MustCompile(`//\s*GEN_BUILD:IGNORE_INCLUDE`)
	reCondition     = regexp.MustCompile(`//\s*GEN_BUILD:CONDITION\((.*)\)\s*$`)
	reDoNotGenerate = regexp.MustCompile(`#\s*GEN_BUILD:DO_NOT_GENERATE`)
)

func getGoDependencies(p *Project, fsReaderWriter oswrapper.FilesystemReaderWriter) ([]string, error) {
	dawnRoot := path.Dir(path.Dir(p.Root))
	files, err := glob.Scan(dawnRoot, glob.MustParseConfig(`{
		"paths": [
			{"include": ["tools/src/cmd/gen/**/*.go", "tools/src/tint/intrinsic/**/*.go"]},
			{"exclude": ["tools/src/cmd/gen/build/**", "**/*_test.go"]}
		]
	}`), fsReaderWriter)
	if err != nil {
		return nil, err
	}

	var goFiles []string
	for _, f := range files {
		relToTint, err := filepath.Rel("src/tint", f)
		if err != nil {
			return nil, err
		}
		goFiles = append(goFiles, filepath.ToSlash(relToTint))
	}
	sort.Strings(goFiles)
	return goFiles, nil
}

// HasObjcSrcs returns true if the target contains any Objective-C++ .mm files.
func (t *Target) HasObjcSrcs() bool {
	for name := range t.SourceFileSet {
		if strings.HasSuffix(name, ".mm") {
			return true
		}
	}
	return false
}
