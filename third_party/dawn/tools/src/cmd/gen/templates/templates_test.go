package templates

import (
	"context"
	"flag"
	"math/rand"
	"path/filepath"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/cmd/gen/common"
	"dawn.googlesource.com/dawn/tools/src/container"
	"dawn.googlesource.com/dawn/tools/src/fileutils"
	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/sem"
	"github.com/stretchr/testify/require"
)

func setupRunFileTest(t *testing.T) (oswrapper.FSTestOSWrapper, *common.Config, string) {
	t.Helper()

	// Identify the "real" DawnRoot so the mock FS can be populated such that
	// fileutils.DawnRoot(mockFS) returns it.
	realOS := oswrapper.GetRealOSWrapper()
	realDawnRoot := fileutils.DawnRoot(realOS)
	require.NotEmpty(t, realDawnRoot, "Could not determine real DawnRoot")

	osw := oswrapper.CreateFSTestOSWrapper()
	cfg := &common.Config{
		OsWrapper: osw,
	}

	// Populate mock FS with DEPS at realDawnRoot so DawnRoot(osw) works.
	err := osw.MkdirAll(realDawnRoot, 0755)
	require.NoError(t, err, "Failed to create root in mock FS")
	err = osw.WriteFile(filepath.Join(realDawnRoot, "DEPS"), []byte(""), 0644)
	require.NoError(t, err, "Failed to write DEPS")

	// Create the directory structure corresponding to ThisDir() to ensure path lookup works.
	thisDir := fileutils.ThisDir()
	err = osw.MkdirAll(thisDir, 0755)
	require.NoError(t, err, "Failed to create ThisDir in mock FS")

	return osw, cfg, realDawnRoot
}

func createTemplateFile(t *testing.T, osw oswrapper.FilesystemWriter, path, content string) {
	t.Helper()
	err := osw.MkdirAll(filepath.Dir(path), 0755)
	require.NoError(t, err, "Failed to create directory for template")
	err = osw.WriteFile(path, []byte(content), 0644)
	require.NoError(t, err, "Failed to write template file")
}

func TestCmd_Run_FileDiscovery(t *testing.T) {
	osw, cfg, realDawnRoot := setupRunFileTest(t)
	ctx := context.Background()

	// Create the template files
	// Note: The glob pattern in templates.go is "src/tint/**.tmpl" and "test/tint/**.tmpl"
	tmplPath := filepath.Join(realDawnRoot, "src", "tint", "test.tmpl")
	createTemplateFile(t, osw, tmplPath, `Test Template 1`)
	tmplPath2 := filepath.Join(realDawnRoot, "test", "tint", "subdir", "test2.tmpl")
	createTemplateFile(t, osw, tmplPath2, `Test Template 2`)

	c := &Cmd{}
	err := c.Run(ctx, cfg)
	require.NoError(t, err, "Run failed")

	// Verify output 1
	outPath := filepath.Join(realDawnRoot, "src", "tint", "test")
	content, err := osw.ReadFile(outPath)
	require.NoError(t, err, "Output file 1 not found")
	require.Contains(t, string(content), "Test Template 1", "Output content 1 mismatch")

	// Verify output 2
	outPath2 := filepath.Join(realDawnRoot, "test", "tint", "subdir", "test2")
	content2, err := osw.ReadFile(outPath2)
	require.NoError(t, err, "Output file 2 not found")
	require.Contains(t, string(content2), "Test Template 2", "Output content 2 mismatch")
}

func TestCmd_Run_ExplicitFiles(t *testing.T) {
	osw, cfg, realDawnRoot := setupRunFileTest(t)
	ctx := context.Background()

	tmplPath := filepath.Join(realDawnRoot, "src", "tint", "explicit.tmpl")
	createTemplateFile(t, osw, tmplPath, `Explicit Template`)

	// Inject args since the existing FlagSet's args cannot be easily modified.
	// NOTE: This means that this test is incompatible with t.Parallel() since it
	// is modifying global state.
	args := []string{tmplPath}
	origCommandLine := flag.CommandLine
	defer func() { flag.CommandLine = origCommandLine }()

	flag.CommandLine = flag.NewFlagSet("test", flag.ContinueOnError)
	err := flag.CommandLine.Parse(args)
	require.NoError(t, err, "Failed to parse mock flags")

	c := &Cmd{}
	err = c.Run(ctx, cfg)
	require.NoError(t, err, "Run failed")

	// Verify output
	outPath := filepath.Join(realDawnRoot, "src", "tint", "explicit")
	content, err := osw.ReadFile(outPath)
	require.NoError(t, err, "Output file not found")

	// Check content
	require.Contains(t, string(content), "Explicit Template", "Output content mismatch")
}

func TestCmd_Run_StaleCheck(t *testing.T) {
	osw, cfg, realDawnRoot := setupRunFileTest(t)
	ctx := context.Background()

	tmplPath := filepath.Join(realDawnRoot, "src", "tint", "stale.tmpl")
	createTemplateFile(t, osw, tmplPath, `Stale Template`)

	// Create the output file with different content
	outPath := filepath.Join(realDawnRoot, "src", "tint", "stale")
	err := osw.WriteFile(outPath, []byte("Different Content"), 0644)
	require.NoError(t, err, "Failed to write existing output file")

	cfg.Flags.CheckStale = true
	c := &Cmd{}
	err = c.Run(ctx, cfg)

	require.Error(t, err, "Run should have returned an error for stale files")
	staleFiles, ok := err.(common.StaleFiles)
	require.True(t, ok, "Error should be of type common.StaleFiles")
	require.Len(t, staleFiles, 1, "Should have 1 stale file")
	require.Equal(t, outPath, staleFiles[0], "Stale file path mismatch")
}

func TestCmd_Run_StaleCheck_Clean(t *testing.T) {
	osw, cfg, realDawnRoot := setupRunFileTest(t)
	ctx := context.Background()

	tmplPath := filepath.Join(realDawnRoot, "src", "tint", "clean.tmpl")
	createTemplateFile(t, osw, tmplPath, `Clean Template`)

	// First run: generate the file
	c := &Cmd{}
	err := c.Run(ctx, cfg)
	require.NoError(t, err, "First run failed")

	// Verify the file exists
	outPath := filepath.Join(realDawnRoot, "src", "tint", "clean")
	_, err = osw.ReadFile(outPath)
	require.NoError(t, err, "Output file should exist after first run")

	// Second run: check for staleness
	cfg.Flags.CheckStale = true
	err = c.Run(ctx, cfg)
	require.NoError(t, err, "Second run (stale check) should not return error for clean file")
}

func TestCmd_Run_InvalidTemplateSyntax(t *testing.T) {
	osw, cfg, realDawnRoot := setupRunFileTest(t)
	ctx := context.Background()

	tmplPath := filepath.Join(realDawnRoot, "src", "tint", "invalid.tmpl")
	createTemplateFile(t, osw, tmplPath, `{{ invalid syntax }}`)

	c := &Cmd{}
	err := c.Run(ctx, cfg)
	require.Error(t, err, "Run should fail with invalid template syntax")
	require.ErrorContains(t, err, "function \"invalid\" not defined")
}

func TestCmd_Run_MissingIntrinsicDef(t *testing.T) {
	osw, cfg, realDawnRoot := setupRunFileTest(t)
	ctx := context.Background()

	tmplPath := filepath.Join(realDawnRoot, "src", "tint", "missing_intrinsics.tmpl")
	// The template must try to use the intrinsics to trigger the load.
	createTemplateFile(t, osw, tmplPath, `{{ (LoadIntrinsics "src/tint/missing.def").Sem }}`)

	c := &Cmd{}
	err := c.Run(ctx, cfg)
	require.Error(t, err, "Run should fail with missing intrinsic definition")
	// The error comes from ReadFile failing in intrinsicCache.Sem()
	require.ErrorContains(t, err, "does not exist")
}

func TestCmd_Run_TemplateOutsideProjectRoot(t *testing.T) {
	osw, cfg, realDawnRoot := setupRunFileTest(t)
	ctx := context.Background()

	// Create a file outside the project root
	outsidePath := filepath.Join(filepath.Dir(realDawnRoot), "outside_project.tmpl")
	createTemplateFile(t, osw, outsidePath, `Outside Template`)

	// Inject args since the existing FlagSet's args cannot be easily modified.
	// NOTE: This means that this test is incompatible with t.Parallel() since it
	// is modifying global state.
	args := []string{outsidePath}
	origCommandLine := flag.CommandLine
	defer func() { flag.CommandLine = origCommandLine }()

	flag.CommandLine = flag.NewFlagSet("test", flag.ContinueOnError)
	err := flag.CommandLine.Parse(args)
	require.NoError(t, err, "Failed to parse mock flags")

	c := &Cmd{}
	err = c.Run(ctx, cfg)
	require.Error(t, err, "Run should fail with template outside project root")
	require.ErrorContains(t, err, "is not under project root")
}

// spyFS is a wrapper around FilesystemReader that records calls to ReadFile.
type spyFS struct {
	oswrapper.FilesystemReader
	readFileCounts map[string]int
	readFileErr    error
}

func newSpyFS(base oswrapper.FilesystemReader) *spyFS {
	return &spyFS{
		FilesystemReader: base,
		readFileCounts:   make(map[string]int),
	}
}

func (s *spyFS) ReadFile(name string) ([]byte, error) {
	s.readFileCounts[name]++
	if s.readFileErr != nil {
		return nil, s.readFileErr
	}
	return s.FilesystemReader.ReadFile(name)
}

func TestIntrinsicCache_Sem_Caching(t *testing.T) {
	osw, _, realDawnRoot := setupRunFileTest(t)
	spy := newSpyFS(osw)

	defPath := filepath.Join(realDawnRoot, "src/tint/intrinsics.def")
	defContent := `type T`
	createTemplateFile(t, osw, defPath, defContent)

	cache := &intrinsicCache{
		path:     "src/tint/intrinsics.def",
		fsReader: spy,
	}

	// First call should parse and cache
	sem1, err := cache.Sem()
	require.NoError(t, err)
	require.NotNil(t, sem1)
	require.Equal(t, 1, spy.readFileCounts[defPath])

	// Second call should return cached value without reading file
	sem2, err := cache.Sem()
	require.NoError(t, err)
	require.Equal(t, sem1, sem2)
	require.Equal(t, 1, spy.readFileCounts[defPath])
}

func TestIntrinsicCache_Sem_ReadFileError(t *testing.T) {
	osw, _, realDawnRoot := setupRunFileTest(t)
	spy := newSpyFS(osw)

	defPath := filepath.Join(realDawnRoot, "src/tint/intrinsics.def")
	// The file doesn't exist in osw, but also inject an error in spy
	// to ensure testing of the ReadFile error propagation specifically.
	expectedErr := filepath.ErrBadPattern // Just a distinctive error
	spy.readFileErr = expectedErr

	cache := &intrinsicCache{
		path:     "src/tint/intrinsics.def",
		fsReader: spy,
	}

	sem, err := cache.Sem()
	require.Error(t, err)
	require.Equal(t, expectedErr, err)
	require.Nil(t, sem)
	require.Equal(t, 1, spy.readFileCounts[defPath])
}

func TestIntrinsicCache_Sem_ParseError(t *testing.T) {
	osw, _, realDawnRoot := setupRunFileTest(t)

	defPath := filepath.Join(realDawnRoot, "src/tint/intrinsics.def")
	createTemplateFile(t, osw, defPath, `£`)

	cache := &intrinsicCache{
		path:     "src/tint/intrinsics.def",
		fsReader: osw,
	}

	sem, err := cache.Sem()
	require.Error(t, err)
	require.Nil(t, sem)
	require.ErrorContains(t, err, "src/tint/intrinsics.def:1:1")
}

func TestIntrinsicCache_Table_Caching(t *testing.T) {
	osw, _, realDawnRoot := setupRunFileTest(t)
	spy := newSpyFS(osw)

	defPath := filepath.Join(realDawnRoot, "src/tint/intrinsics.def")
	defContent := `type T`
	createTemplateFile(t, osw, defPath, defContent)

	cache := &intrinsicCache{
		path:     "src/tint/intrinsics.def",
		fsReader: spy,
	}

	// First call
	table1, err := cache.Table()
	require.NoError(t, err)
	require.NotNil(t, table1)
	require.NotNil(t, cache.cachedTable)
	require.Equal(t, 1, spy.readFileCounts[defPath])

	// Second call
	table2, err := cache.Table()
	require.NoError(t, err)
	require.Equal(t, table1, table2)
	require.Equal(t, 1, spy.readFileCounts[defPath])
}

func TestIntrinsicCache_Permute_Caching(t *testing.T) {
	osw, _, realDawnRoot := setupRunFileTest(t)
	spy := newSpyFS(osw)

	defPath := filepath.Join(realDawnRoot, "src/tint/intrinsics.def")
	// Define a simple function that can be permuted.
	defContent := `
fn F()
`
	createTemplateFile(t, osw, defPath, defContent)

	cache := &intrinsicCache{
		path:     "src/tint/intrinsics.def",
		fsReader: spy,
	}

	// Load semantics to get an overload to permute.
	s, err := cache.Sem()
	require.NoError(t, err)
	require.NotEmpty(t, s.Builtins)
	require.NotEmpty(t, s.Builtins[0].Overloads)
	require.Equal(t, 1, spy.readFileCounts[defPath])
	overload := s.Builtins[0].Overloads[0]

	// Reset cachedPermuter to ensure that the lazy initialization is tested.
	cache.cachedPermuter = nil

	perms1, err := cache.Permute(overload)
	require.NoError(t, err)
	require.NotNil(t, perms1)
	require.NotNil(t, cache.cachedPermuter)
	// Since cachedSem is already set, no new file read should happen.
	require.Equal(t, 1, spy.readFileCounts[defPath])

	perms2, err := cache.Permute(overload)
	require.NoError(t, err)
	require.Equal(t, perms1, perms2)
	// Still no file read.
	require.Equal(t, 1, spy.readFileCounts[defPath])
}

func TestGenCache_Intrinsics(t *testing.T) {
	osw, _, _ := setupRunFileTest(t)
	cache := &genCache{
		fsReader: osw,
	}

	// First call for path 1
	i1 := cache.intrinsics("path/to/def1")
	require.NotNil(t, i1)
	require.Equal(t, "path/to/def1", i1.path)
	require.Equal(t, osw, i1.fsReader)

	// Second call for path 1 should return same instance
	i2 := cache.intrinsics("path/to/def1")
	require.True(t, i1 == i2, "Should return the same instance for the same path")

	// Call for path 2 should return new instance
	i3 := cache.intrinsics("path/to/def2")
	require.NotNil(t, i3)
	require.Equal(t, "path/to/def2", i3.path)
	require.True(t, i1 != i3, "Should return different instance for different path")
}

func TestGenerator_Scramble(t *testing.T) {
	// Initialize generator with a fixed seed for deterministic behavior.
	g := generator{
		rnd: rand.New(rand.NewSource(123)),
	}

	initial := "InitialString"
	avoid := container.NewSet[string]()

	// Test 1: Verify scramble changes the string.
	scrambled, err := g.scramble(initial, avoid)
	require.NoError(t, err)
	require.NotEqual(t, initial, scrambled)

	// Test 2: Verify scramble avoids strings in |avoid|.
	avoid.Add(scrambled)
	scrambled2, err := g.scramble(initial, avoid)
	require.NoError(t, err)
	require.NotEqual(t, initial, scrambled2)
	require.NotEqual(t, scrambled, scrambled2)
	require.False(t, avoid.Contains(scrambled2))
}

func TestGenerator_SetCommentPrefix(t *testing.T) {
	g := generator{}
	require.Empty(t, g.commentPrefix)

	ret := g.setCommentPrefix("// New Prefix")
	require.Empty(t, ret, "setCommentPrefix should return empty string")
	require.Equal(t, "// New Prefix", g.commentPrefix)
}

func TestHelper_Is(t *testing.T) {
	// Test with sem.EnumEntry
	isEnumEntry := is(sem.EnumEntry{})
	require.True(t, isEnumEntry(sem.EnumEntry{}), "Should be true for sem.EnumEntry")
	require.True(t, isEnumEntry(&sem.EnumEntry{}), "Should be true for *sem.EnumEntry")
	require.False(t, isEnumEntry(sem.EnumMatcher{}), "Should be false for sem.EnumMatcher")

	// Test with basic types
	isInt := is(1)
	require.True(t, isInt(1), "Should be true for int")
	require.True(t, isInt(100), "Should be true for int")
	val := 5
	require.True(t, isInt(&val), "Should be true for *int")
	require.False(t, isInt("string"), "Should be false for string")
}

func TestHelper_IsFirstIn(t *testing.T) {
	slice := []int{1, 2, 3}
	require.True(t, isFirstIn(1, slice), "1 should be first in [1, 2, 3]")
	require.False(t, isFirstIn(2, slice), "2 should not be first in [1, 2, 3]")

	empty := []int{}
	require.False(t, isFirstIn(1, empty), "Should be false for empty slice")
}

func TestHelper_IsLastIn(t *testing.T) {
	slice := []int{1, 2, 3}
	require.True(t, isLastIn(3, slice), "3 should be last in [1, 2, 3]")
	require.False(t, isLastIn(2, slice), "2 should not be last in [1, 2, 3]")

	empty := []int{}
	require.False(t, isLastIn(1, empty), "Should be false for empty slice")
}
