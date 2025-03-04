// Copyright 2021 The Dawn & Tint Authors
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

package templates

import (
	"context"
	"flag"
	"fmt"
	"math/rand"
	"os"
	"path/filepath"
	"reflect"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/cmd/gen/common"
	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/glob"
	"dawn.googlesource.com/dawn/tools/src/template"
	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/gen"
	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/parser"
	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/resolver"
	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/sem"
)

func init() {
	common.Register(&Cmd{})
}

type Cmd struct {
}

func (Cmd) Name() string {
	return "templates"
}

func (Cmd) Desc() string {
	return `templates generates files from <file>.tmpl files found in the Tint source and test directories`
}

func (c *Cmd) RegisterFlags(ctx context.Context, cfg *common.Config) ([]string, error) {
	return nil, nil
}

func (c Cmd) Run(ctx context.Context, cfg *common.Config) error {
	staleFiles := common.StaleFiles{}
	projectRoot := fileutils.DawnRoot()

	files := flag.Args()
	if len(files) == 0 {
		// Recursively find all the template files in the <dawn>/src/tint and
		// <dawn>/test/tint and directories
		var err error
		files, err = glob.Scan(projectRoot, glob.MustParseConfig(`{
			"paths": [{"include": [
				"src/tint/**.tmpl",
				"test/tint/**.tmpl"
			]}]
		}`))
		if err != nil {
			return err
		}
	} else {
		// Make all template file paths project-relative
		for i, f := range files {
			abs, err := filepath.Abs(f)
			if err != nil {
				return fmt.Errorf("failed to get absolute file path for '%v': %w", f, err)
			}
			if !strings.HasPrefix(abs, projectRoot) {
				return fmt.Errorf("template '%v' is not under project root '%v'", abs, projectRoot)
			}
			rel, err := filepath.Rel(projectRoot, abs)
			if err != nil {
				return fmt.Errorf("failed to get project relative file path for '%v': %w", f, err)
			}
			files[i] = rel
		}
	}

	cache := &genCache{}

	// For each template file...
	for _, relTmplPath := range files { // relative to project root
		if cfg.Flags.Verbose {
			fmt.Println("processing", relTmplPath)
		}
		// Make tmplPath absolute
		tmplPath := filepath.Join(projectRoot, relTmplPath)
		tmplDir := filepath.Dir(tmplPath)

		// Create or update the file at relPath if the file content has changed,
		// preserving the copyright year in the header.
		// relPath is a path relative to the template
		writeFile := func(relPath, body, commentPrefix string) error {
			if strings.TrimSpace(body) == "" {
				// Don't write empty files
				return nil
			}

			outPath := filepath.Join(tmplDir, relPath)

			switch filepath.Ext(relPath) {
			case ".cc", ".h", ".inl":
				var err error
				body, err = common.ClangFormat(body)
				if err != nil {
					return err
				}
			}

			// Load the old file
			existing, err := os.ReadFile(outPath)
			if err != nil {
				existing = nil
			}

			// Write the common file header
			if cfg.Flags.Verbose {
				fmt.Println("  writing", outPath)
			}
			sb := strings.Builder{}
			sb.WriteString(common.Header(string(existing), filepath.ToSlash(relTmplPath), commentPrefix))
			sb.WriteString("\n")
			sb.WriteString(body)
			oldContent, newContent := string(existing), sb.String()

			if oldContent != newContent {
				if cfg.Flags.CheckStale {
					staleFiles = append(staleFiles, outPath)
				} else {
					if err := os.MkdirAll(filepath.Dir(outPath), 0777); err != nil {
						return fmt.Errorf("failed to create directory for '%v': %w", outPath, err)
					}
					if err := os.WriteFile(outPath, []byte(newContent), 0666); err != nil {
						return fmt.Errorf("failed to write file '%v': %w", outPath, err)
					}
				}
			}

			return nil
		}

		// Write the content generated using the template and semantic info
		_, tmplFileName := filepath.Split(tmplPath)
		outPath := strings.TrimSuffix(tmplFileName, ".tmpl")
		if err := generate(tmplPath, outPath, cache, writeFile); err != nil {
			return fmt.Errorf("while processing '%v': %w", tmplPath, err)
		}
	}

	if len(staleFiles) > 0 {
		return staleFiles
	}

	return nil
}

type intrinsicCache struct {
	path           string
	cachedSem      *sem.Sem            // lazily built by sem()
	cachedTable    *gen.IntrinsicTable // lazily built by intrinsicTable()
	cachedPermuter *gen.Permutator     // lazily built by permute()
}

// Sem lazily parses and resolves the intrinsic.def file, returning the semantic info.
func (i *intrinsicCache) Sem() (*sem.Sem, error) {
	if i.cachedSem == nil {
		// Load the intrinsic definition file
		defPath := filepath.Join(fileutils.DawnRoot(), i.path)

		defSource, err := os.ReadFile(defPath)
		if err != nil {
			return nil, err
		}

		// Parse the definition file to produce an AST
		ast, err := parser.Parse(string(defSource), i.path)
		if err != nil {
			return nil, err
		}

		// Resolve the AST to produce the semantic info
		sem, err := resolver.Resolve(ast)
		if err != nil {
			return nil, err
		}

		i.cachedSem = sem
	}
	return i.cachedSem, nil
}

// Table lazily calls and returns the result of BuildIntrinsicTable(),
// caching the result for repeated calls.
func (i *intrinsicCache) Table() (*gen.IntrinsicTable, error) {
	if i.cachedTable == nil {
		sem, err := i.Sem()
		if err != nil {
			return nil, err
		}
		i.cachedTable, err = gen.BuildIntrinsicTable(sem)
		if err != nil {
			return nil, err
		}
	}
	return i.cachedTable, nil
}

// Permute lazily calls NewPermuter(), caching the result for repeated calls,
// then passes the argument to Permutator.Permute()
func (i *intrinsicCache) Permute(overload *sem.Overload) ([]gen.Permutation, error) {
	if i.cachedPermuter == nil {
		sem, err := i.Sem()
		if err != nil {
			return nil, err
		}
		i.cachedPermuter, err = gen.NewPermutator(sem)
		if err != nil {
			return nil, err
		}
	}
	out, err := i.cachedPermuter.Permute(overload)
	if err != nil {
		return nil, fmt.Errorf("while permuting '%v'\n%w", overload, err)
	}
	return out, nil
}

// Cache for objects that are expensive to build, and can be reused between templates.
type genCache struct {
	intrinsicsCache container.Map[string, *intrinsicCache]
}

func (g *genCache) intrinsics(path string) *intrinsicCache {
	if g.intrinsicsCache == nil {
		g.intrinsicsCache = container.NewMap[string, *intrinsicCache]()
	}
	i := g.intrinsicsCache[path]
	if i == nil {
		i = &intrinsicCache{path: path}
		g.intrinsicsCache[path] = i
	}
	return i
}

type generator struct {
	cache         *genCache
	writeFile     WriteFile
	rnd           *rand.Rand
	commentPrefix string
}

// setCommentPrefix sets the prefix used for comments, as used by the template
func (g *generator) setCommentPrefix(commentPrefix string) string {
	g.commentPrefix = commentPrefix
	return ""
}

// WriteFile is a function that Generate() may call to emit a new file from a
// template.
// relPath is the relative path from the currently executing template.
// content is the file content to write.
// comment is the prefix used for line comments
type WriteFile func(relPath, content, comment string) error

// generate executes the template tmpl, calling writeFile with the output.
// See https://golang.org/pkg/text/template/ for documentation on the template
// syntax.
func generate(tmplPath, outPath string, cache *genCache, writeFile WriteFile) error {
	g := generator{
		cache:         cache,
		writeFile:     writeFile,
		rnd:           rand.New(rand.NewSource(4561123)),
		commentPrefix: "//",
	}

	funcs := map[string]any{
		"SetCommentPrefix":                    g.setCommentPrefix,
		"SplitDisplayName":                    gen.SplitDisplayName,
		"Scramble":                            g.scramble,
		"IsEnumEntry":                         is(sem.EnumEntry{}),
		"IsEnumMatcher":                       is(sem.EnumMatcher{}),
		"IsFQN":                               is(sem.FullyQualifiedName{}),
		"IsInt":                               is(1),
		"IsTemplateEnumParam":                 is(sem.TemplateEnumParam{}),
		"IsTemplateNumberParam":               is(sem.TemplateNumberParam{}),
		"IsTemplateTypeParam":                 is(sem.TemplateTypeParam{}),
		"IsType":                              is(sem.Type{}),
		"ElementType":                         gen.ElementType,
		"DeepestElementType":                  gen.DeepestElementType,
		"IsAbstract":                          gen.IsAbstract,
		"IsDeclarable":                        gen.IsDeclarable,
		"IsHostShareable":                     gen.IsHostShareable,
		"OverloadUsesType":                    gen.OverloadUsesType,
		"OverloadUsesReadWriteStorageTexture": gen.OverloadUsesReadWriteStorageTexture,
		"OverloadNeedsDesktopGLSL":            gen.OverloadNeedsDesktopGLSL,
		"IsFirstIn":                           isFirstIn,
		"IsLastIn":                            isLastIn,
		"LoadIntrinsics":                      func(path string) *intrinsicCache { return g.cache.intrinsics(path) },
		"WriteFile": func(relPath, content string) (string, error) {
			return "", g.writeFile(relPath, content, g.commentPrefix)
		},
	}
	t, err := template.FromFile(tmplPath)
	if err != nil {
		return err
	}
	w := &strings.Builder{}
	if err := t.Run(w, nil, funcs); err != nil {
		return err
	}
	return writeFile(outPath, w.String(), g.commentPrefix)
}

// scramble randomly modifies the input string so that it is no longer equal to
// any of the strings in 'avoid'.
func (g *generator) scramble(str string, avoid container.Set[string]) (string, error) {
	bytes := []byte(str)
	passes := g.rnd.Intn(5) + 1

	const chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz"

	char := func() byte { return chars[g.rnd.Intn(len(chars))] }
	replace := func(at int) { bytes[at] = char() }
	delete := func(at int) { bytes = append(bytes[:at], bytes[at+1:]...) }
	insert := func(at int) { bytes = append(append(bytes[:at], char()), bytes[at:]...) }

	for i := 0; i < passes || avoid.Contains(string(bytes)); i++ {
		if len(bytes) > 0 {
			at := g.rnd.Intn(len(bytes))
			switch g.rnd.Intn(3) {
			case 0:
				replace(at)
			case 1:
				delete(at)
			case 2:
				insert(at)
			}
		} else {
			insert(0)
		}
	}
	return string(bytes), nil
}

// is returns a function that returns true if the value passed to the function
// matches the type of 'ty'.
func is(ty any) func(any) bool {
	rty := reflect.TypeOf(ty)
	return func(v any) bool {
		ty := reflect.TypeOf(v)
		return ty == rty || ty == reflect.PtrTo(rty)
	}
}

// isFirstIn returns true if v is the first element of the given slice.
func isFirstIn(v, slice any) bool {
	s := reflect.ValueOf(slice)
	count := s.Len()
	if count == 0 {
		return false
	}
	return s.Index(0).Interface() == v
}

// isFirstIn returns true if v is the last element of the given slice.
func isLastIn(v, slice any) bool {
	s := reflect.ValueOf(slice)
	count := s.Len()
	if count == 0 {
		return false
	}
	return s.Index(count-1).Interface() == v
}
