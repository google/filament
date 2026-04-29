// Copyright 2026 The Dawn & Tint Authors
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

package main

import (
	"path/filepath"
	"strings"
	"testing"
	"text/template"

	"dawn.googlesource.com/dawn/tools/src/oswrapper"
	"github.com/ben-clayton/webidlparser/ast"
	"github.com/ben-clayton/webidlparser/parser"
	"github.com/stretchr/testify/require"
)

func TestRun_MissingArgs(t *testing.T) {
	tests := []struct {
		name string
		args []string
	}{
		{
			name: "No args",
			args: []string{},
		},
		{
			name: "Missing template",
			args: []string{"--output=out.go", "input.idl"},
		},
		{
			name: "Missing input file",
			args: []string{"--template=tmpl.go", "--output=out.go"},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			wrapper := oswrapper.CreateFSTestOSWrapper()
			err := run(tt.args, wrapper)
			require.ErrorContains(t, err, "idlgen is a tool used to generate code")
		})
	}
}

func TestRun_OutputToStdout(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	require.NoError(t, wrapper.WriteFile("template.tmpl", []byte("{{.}}"), 0644))
	require.NoError(t, wrapper.WriteFile("test.idl", []byte("interface Test {}; interface GPUDevice {};"), 0644))

	args := []string{"--template=template.tmpl", "test.idl"}
	err := run(args, wrapper)
	require.NoError(t, err)
}

func TestRun_OutputToFile(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	require.NoError(t, wrapper.WriteFile("template.tmpl", []byte("{{range .Declarations}}{{.Name}} {{end}}"), 0644))
	require.NoError(t, wrapper.WriteFile("test.idl", []byte("interface Test {}; interface GPUDevice {};"), 0644))

	outputPath := "subdir/out.txt"
	args := []string{"--template=template.tmpl", "--output=" + outputPath, "test.idl"}
	err := run(args, wrapper)
	require.NoError(t, err)

	// Verify file exists and has content
	content, err := wrapper.ReadFile(outputPath)
	require.NoError(t, err)
	require.Contains(t, string(content), "Test GPUDevice")
}

func TestRun_OutputDirectoryCreationFailure(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	require.NoError(t, wrapper.WriteFile("template.tmpl", []byte("{{.}}"), 0644))
	require.NoError(t, wrapper.WriteFile("test.idl", []byte("interface Test {};"), 0644))

	// Create a file named "subdir" to block directory creation
	require.NoError(t, wrapper.WriteFile("subdir", []byte("i am a file"), 0644))

	outputPath := "subdir/out.txt"
	args := []string{"--template=template.tmpl", "--output=" + outputPath, "test.idl"}
	err := run(args, wrapper)
	require.Error(t, err)
	require.ErrorContains(t, err, "failed to create output directory")
}

func TestRun_OutputFileCreationFailure(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	require.NoError(t, wrapper.WriteFile("template.tmpl", []byte("{{.}}"), 0644))
	require.NoError(t, wrapper.WriteFile("test.idl", []byte("interface Test {};"), 0644))

	outputPath := "out.txt"
	// Create a directory at the output path to cause Create to fail
	require.NoError(t, wrapper.Mkdir(outputPath, 0755))

	args := []string{"--template=template.tmpl", "--output=" + outputPath, "test.idl"}
	err := run(args, wrapper)
	require.Error(t, err)
	require.ErrorContains(t, err, "failed to open output file")
}

func TestRun_TemplateReadFailure(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	// Do not create the template file

	require.NoError(t, wrapper.WriteFile("test.idl", []byte("interface Test {};"), 0644))

	args := []string{"--template=template.tmpl", "test.idl"}
	err := run(args, wrapper)
	require.ErrorContains(t, err, "failed to open template file")
}

func TestRun_IDLReadFailure(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	require.NoError(t, wrapper.WriteFile("template.tmpl", []byte("{{.}}"), 0644))

	// Provide a non-existent IDL file
	args := []string{"--template=template.tmpl", "missing.idl"}
	err := run(args, wrapper)
	require.ErrorContains(t, err, "failed to open file 'missing.idl'")
}

func TestRun_IDLParseFailure(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	require.NoError(t, wrapper.WriteFile("template.tmpl", []byte("{{.}}"), 0644))
	// Provide invalid IDL content
	require.NoError(t, wrapper.WriteFile("invalid.idl", []byte("this is definitely not valid idl"), 0644))

	args := []string{"--template=template.tmpl", "invalid.idl"}
	err := run(args, wrapper)
	require.ErrorContains(t, err, "errors found while parsing invalid.idl")
}

func TestRun_TemplateParseFailure(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()
	// Provide invalid template content (unclosed action)
	require.NoError(t, wrapper.WriteFile("invalid.tmpl", []byte("{{.Name"), 0644))
	require.NoError(t, wrapper.WriteFile("test.idl", []byte("interface Test {}; interface GPUDevice {};"), 0644))

	args := []string{"--template=invalid.tmpl", "test.idl"}
	err := run(args, wrapper)
	require.ErrorContains(t, err, "failed to parse template file")
}

func TestRun_SuccessfulExecution(t *testing.T) {
	wrapper := oswrapper.CreateFSTestOSWrapper()

	// A simple template that iterates over declarations and prints their names
	templateContent := `{{range .Declarations}}{{.Name}};{{end}}`
	require.NoError(t, wrapper.WriteFile("template.tmpl", []byte(templateContent), 0644))

	// A simple IDL file. GPUDevice is required to avoid a panic in the patch function.
	idlContent := `
		interface MyInterface {
			void myMethod();
		};
		interface GPUDevice {};
	`
	require.NoError(t, wrapper.WriteFile("test.idl", []byte(idlContent), 0644))

	outputPath := "output.txt"
	args := []string{"--template=template.tmpl", "--output=" + outputPath, "test.idl"}

	err := run(args, wrapper)
	require.NoError(t, err)

	// Verify the output content
	content, err := wrapper.ReadFile(outputPath)
	require.NoError(t, err)

	outputStr := string(content)
	require.Contains(t, outputStr, "File generated by tools/cmd/idlgen.go")
	require.Contains(t, outputStr, "\n\nMyInterface;GPUDevice;")
}

func TestSimplify(t *testing.T) {
	idl := `
		interface Dependent : Base {
			void method();
		};

		interface Base {};

		partial interface Partial {
			void method2();
		};

		interface Partial {
			void method1();
		};

		interface mixin MyMixin {
			const long mixinConst = 1;
		};

		interface WithMixin {
			void originalMethod();
		};

		WithMixin includes MyMixin;

		enum MyEnum {
			"value1"
		};

		enum MyEnum {
			"value2"
		};

		dictionary MyDict {
			long member1;
		};

		partial dictionary MyDict {
			long member2;
		};

		typedef long MyTypedef;

		dictionary BaseDict {
			MyTypedef val;
		};

		dictionary DerivedDict : BaseDict {
			long otherVal;
		};

		interface InterfaceUsingDict {
			void method(DerivedDict d);
		};
	`

	parsed := parser.Parse(idl)
	require.Empty(t, parsed.Errors)

	simplified, decls := simplify(parsed)

	// Check Partial Interface Merging
	partialDecl := decls["Partial"]
	require.NotNil(t, partialDecl)
	partialIface, ok := partialDecl.(*ast.Interface)
	require.True(t, ok)
	require.Equal(t, 2, len(partialIface.Members))

	memberNames := make(map[string]bool)
	for _, m := range partialIface.Members {
		if member, ok := m.(*ast.Member); ok {
			memberNames[member.Name] = true
		}
	}
	require.True(t, memberNames["method1"])
	require.True(t, memberNames["method2"])

	// Check Mixin Embedding
	withMixinDecl := decls["WithMixin"]
	require.NotNil(t, withMixinDecl)
	withMixinIface, ok := withMixinDecl.(*ast.Interface)
	require.True(t, ok)
	require.Equal(t, 2, len(withMixinIface.Members))

	memberNames = make(map[string]bool)
	for _, m := range withMixinIface.Members {
		if member, ok := m.(*ast.Member); ok {
			memberNames[member.Name] = true
		}
	}
	require.True(t, memberNames["originalMethod"])
	require.True(t, memberNames["mixinConst"])

	// Check Partial Enum Merging
	enumDecl := decls["MyEnum"]
	require.NotNil(t, enumDecl)
	enum, ok := enumDecl.(*ast.Enum)
	require.True(t, ok)
	require.Equal(t, 2, len(enum.Values))

	enumValues := make(map[string]bool)
	for _, v := range enum.Values {
		if val, ok := v.(*ast.BasicLiteral); ok {
			enumValues[val.Value] = true
		}
	}
	require.True(t, enumValues["\"value1\""])
	require.True(t, enumValues["\"value2\""])

	// Check Partial Dictionary Merging
	dictDecl := decls["MyDict"]
	require.NotNil(t, dictDecl)
	dict, ok := dictDecl.(*ast.Dictionary)
	require.True(t, ok)
	require.Equal(t, 2, len(dict.Members))

	dictMembers := make(map[string]bool)
	for _, m := range dict.Members {
		dictMembers[m.Name] = true
	}
	require.True(t, dictMembers["member1"])
	require.True(t, dictMembers["member2"])

	// Check Dependency Ordering
	// Base must precede Dependent
	var names []string
	for _, d := range simplified.Declarations {
		names = append(names, nameOf(d))
	}

	checkOrder := func(first, second string) {
		firstIdx := -1
		secondIdx := -1
		for i, name := range names {
			if name == first {
				firstIdx = i
			}
			if name == second {
				secondIdx = i
			}
		}
		require.NotEqual(t, -1, firstIdx, "%s not found in simplified declarations", first)
		require.NotEqual(t, -1, secondIdx, "%s not found in simplified declarations", second)
		require.True(t, firstIdx < secondIdx, "%s should precede %s, but found indices %d and %d", first, second, firstIdx, secondIdx)
	}

	checkOrder("Base", "Dependent")
	checkOrder("MyTypedef", "BaseDict")
	checkOrder("BaseDict", "DerivedDict")
	checkOrder("DerivedDict", "InterfaceUsingDict")
}

func TestEnumEntryName(t *testing.T) {
	tests := []struct {
		input    string
		expected string
	}{
		{"rgba8unorm", "kRgba8Unorm"},
		{"\"bgra8unorm\"", "kBgra8Unorm"},
		{"depth-stencil", "kDepthStencil"},
		{"triangle-list", "kTriangleList"},
		{"line-strip", "kLineStrip"},
	}

	for _, tt := range tests {
		t.Run(tt.input, func(t *testing.T) {
			result := enumEntryName(tt.input)
			require.Equal(t, result, tt.expected)
		})
	}
}

func TestMemberExtraction(t *testing.T) {
	idl := `
		interface Base {
			const long BaseConst = 1;
			attribute long BaseAttr;
			void baseMethod();
		};

		interface Derived : Base {
			const long DerivedConst = 2;
			attribute long DerivedAttr;
			void derivedMethod();
		};

		namespace MyNamespace {
			const long NamespaceConst = 3;
			attribute long NamespaceAttr;
			void namespaceMethod();
		};
	`

	parsed := parser.Parse(idl)
	require.Empty(t, parsed.Errors)

	_, decls := simplify(parsed)

	g := generator{
		declarations: decls,
	}

	base := decls["Base"]
	derived := decls["Derived"]
	namespace := decls["MyNamespace"]

	t.Run("constantsOf", func(t *testing.T) {
		check := func(obj ast.Decl, expected []string) {
			consts := constantsOf(obj)
			require.Len(t, consts, len(expected))
			for i, name := range expected {
				require.Equal(t, name, consts[i].Name)
			}
		}
		check(base, []string{"BaseConst"})
		check(derived, []string{"DerivedConst"})
		check(namespace, []string{"NamespaceConst"})
	})

	t.Run("flattenedConstantsOf", func(t *testing.T) {
		consts := g.flattenedConstantsOf(derived)
		require.Len(t, consts, 2)
		require.Equal(t, "DerivedConst", consts[0].Name)
		require.Equal(t, "BaseConst", consts[1].Name)
	})

	t.Run("attributesOf", func(t *testing.T) {
		check := func(obj ast.Decl, expected []string) {
			attrs := attributesOf(obj)
			require.Len(t, attrs, len(expected))
			for i, name := range expected {
				require.Equal(t, name, attrs[i].Name)
			}
		}
		check(base, []string{"BaseAttr"})
		check(derived, []string{"DerivedAttr"})
		check(namespace, []string{"NamespaceAttr"})
	})

	t.Run("flattenedAttributesOf", func(t *testing.T) {
		attrs := g.flattenedAttributesOf(derived)
		require.Len(t, attrs, 2)
		require.Equal(t, "DerivedAttr", attrs[0].Name)
		require.Equal(t, "BaseAttr", attrs[1].Name)
	})

	t.Run("methodsOf", func(t *testing.T) {
		check := func(obj ast.Decl, expected []string) {
			methods := methodsOf(obj)
			require.Len(t, methods, len(expected))
			for i, name := range expected {
				require.Equal(t, name, methods[i].Name)
			}
		}
		check(base, []string{"baseMethod"})
		check(derived, []string{"derivedMethod"})
		// methodsOf currently returns nil for namespaces
		check(namespace, []string{})
	})

	t.Run("flattenedMethodsOf", func(t *testing.T) {
		methods := g.flattenedMethodsOf(derived)
		require.Len(t, methods, 2)
		require.Equal(t, "derivedMethod", methods[0].Name)
		require.Equal(t, "baseMethod", methods[1].Name)
	})
}

func TestPatch(t *testing.T) {
	lostMember := &ast.Member{
		Name:        "lost",
		Type:        &ast.TypeName{Name: "Promise"},
		Annotations: []*ast.Annotation{},
	}
	otherMember := &ast.Member{
		Name:        "other",
		Type:        &ast.TypeName{Name: "void"},
		Annotations: []*ast.Annotation{},
	}

	gpuDevice := &ast.Interface{
		Name:    "GPUDevice",
		Members: []ast.InterfaceMember{lostMember, otherMember},
	}

	decls := declarations{
		"GPUDevice": gpuDevice,
	}

	// Execute patch
	patch(nil, decls)

	// Verify [SameObject] was added to 'lost'
	require.True(t, hasAnnotation(lostMember, "SameObject"))

	// Verify 'other' was not touched
	require.False(t, hasAnnotation(otherMember, "SameObject"))
}

func TestGeneratorEval(t *testing.T) {
	tests := []struct {
		name         string
		templateName string
		args         []interface{}
		expected     string
		expectError  string
		setup        func(*template.Template)
	}{
		{
			name:         "SingleArgument",
			templateName: "Simple",
			args:         []interface{}{"Hello"},
			expected:     "Hello",
		},
		{
			name:         "MapArguments",
			templateName: "WithMap",
			args:         []interface{}{"Key", "foo", "Value", "bar"},
			expected:     "foo=bar",
		},
		{
			name:         "TemplateNotFound",
			templateName: "Missing",
			args:         []interface{}{"arg"},
			expectError:  "template 'Missing' not found",
		},
		{
			name:         "InvalidArgCount",
			templateName: "WithMap",
			args:         []interface{}{"Key", "foo", "Value"},
			expectError:  "Eval expects a single argument or list name-value pairs",
		},
		{
			name:         "InvalidArgType",
			templateName: "WithMap",
			args:         []interface{}{123, "foo", "Value", "bar"},
			expectError:  "Eval argument 0 is not a string",
		},
		{
			name:         "ExecutionError",
			templateName: "Fail",
			args:         []interface{}{map[string]string{}},
			expectError:  "while evaluating 'Fail'",
			setup: func(t *template.Template) {
				t.Option("missingkey=error")
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			tmpl := template.New("root")
			_, err := tmpl.Parse(`
{{define "Simple"}}{{.}}{{end}}
{{define "WithMap"}}{{.Key}}={{.Value}}{{end}}
{{define "Fail"}}{{ .MissingField }}{{end}}
`)
			require.NoError(t, err)

			if tt.setup != nil {
				tt.setup(tmpl)
			}
			g := generator{
				t: tmpl,
			}

			out, err := g.eval(tt.templateName, tt.args...)
			if tt.expectError != "" {
				require.Error(t, err)
				require.ErrorContains(t, err, tt.expectError)
			} else {
				require.NoError(t, err)
				require.Equal(t, tt.expected, out)
			}
		})
	}
}

func TestGeneratorInclude(t *testing.T) {
	tests := []struct {
		name           string
		fileName       string
		fileContent    string
		includePath    string
		expectError    string
		verifyTemplate string // Template to execute to verify the include worked
		verifyOutput   string // Expected output from verifyTemplate
	}{
		{
			name:           "Success",
			fileName:       "included.tmpl",
			fileContent:    `{{define "Included"}}Included Content{{end}}`,
			includePath:    "included.tmpl",
			verifyTemplate: `{{template "Included"}}`,
			verifyOutput:   "Included Content",
		},
		{
			name:        "MissingFile",
			includePath: "missing.tmpl",
			expectError: "open templates/missing.tmpl",
		},
		{
			name:        "ParseError",
			fileName:    "invalid.tmpl",
			fileContent: "{{",
			includePath: "invalid.tmpl",
			expectError: "template: invalid.tmpl:1: unclosed action",
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			wrapper := oswrapper.CreateFSTestOSWrapper()
			workingDir := "templates"
			require.NoError(t, wrapper.Mkdir(workingDir, 0755))

			if tt.fileName != "" {
				err := wrapper.WriteFile(filepath.Join(workingDir, tt.fileName), []byte(tt.fileContent), 0644)
				require.NoError(t, err)
			}

			tmpl := template.New("root")
			g := generator{
				t:          tmpl,
				workingDir: workingDir,
				osWrapper:  wrapper,
				funcs:      map[string]interface{}{},
			}

			out, err := g.include(tt.includePath)
			if tt.expectError != "" {
				require.Error(t, err)
				require.ErrorContains(t, err, tt.expectError)
			} else {
				require.NoError(t, err)
				require.Empty(t, out)

				if tt.verifyTemplate != "" {
					t2, err := tmpl.Parse(tt.verifyTemplate)
					require.NoError(t, err)

					sb := strings.Builder{}
					err = t2.Execute(&sb, nil)
					require.NoError(t, err)
					require.Equal(t, tt.verifyOutput, sb.String())
				}
			}
		})
	}
}

func TestMapGet(t *testing.T) {
	m := newMap()
	m.Put("key1", "value1")
	m.Put("key2", 123)
	m.Put(456, "value3")

	require.Equal(t, "value1", m.Get("key1"))
	require.Equal(t, 123, m.Get("key2"))
	require.Equal(t, "value3", m.Get(456))
	require.Nil(t, m.Get("missing"))
}

func TestIs(t *testing.T) {
	isInterface := is(ast.Interface{})
	isInterfaceOrNamespace := is(ast.Interface{}, ast.Namespace{})

	iface := &ast.Interface{}
	namespace := &ast.Namespace{}
	dictionary := &ast.Dictionary{}

	require.True(t, isInterface(iface))
	require.True(t, isInterface(*iface))
	require.False(t, isInterface(namespace))
	require.False(t, isInterface(dictionary))

	require.True(t, isInterfaceOrNamespace(iface))
	require.True(t, isInterfaceOrNamespace(namespace))
	require.False(t, isInterfaceOrNamespace(dictionary))
}

func TestIsInitializer(t *testing.T) {
	constructor := &ast.Member{
		Type: &ast.TypeName{Name: "constructor"},
	}
	regularMember := &ast.Member{
		Type: &ast.TypeName{Name: "void"},
	}
	notMember := &ast.Interface{}

	require.True(t, isInitializer(constructor))
	require.False(t, isInitializer(regularMember))
	require.False(t, isInitializer(notMember))
}

func TestIsUndefinedType(t *testing.T) {
	undefined := &ast.TypeName{Name: "undefined"}
	notUndefined := &ast.TypeName{Name: "void"}
	notTypeName := &ast.SequenceType{}

	require.True(t, isUndefinedType(undefined))
	require.False(t, isUndefinedType(notUndefined))
	require.False(t, isUndefinedType(notTypeName))
}

func TestHasAnnotation(t *testing.T) {
	annotated := []*ast.Annotation{
		{Name: "Annotated"},
		{Name: "WithValue"},
	}

	t.Run("Interface", func(t *testing.T) {
		node := &ast.Interface{Annotations: annotated}
		require.True(t, hasAnnotation(node, "Annotated"))
		require.True(t, hasAnnotation(node, "WithValue"))
		require.False(t, hasAnnotation(node, "Missing"))
	})

	t.Run("Member", func(t *testing.T) {
		node := &ast.Member{Annotations: annotated}
		require.True(t, hasAnnotation(node, "Annotated"))
		require.True(t, hasAnnotation(node, "WithValue"))
		require.False(t, hasAnnotation(node, "Missing"))
	})

	t.Run("Namespace", func(t *testing.T) {
		node := &ast.Namespace{Annotations: annotated}
		require.True(t, hasAnnotation(node, "Annotated"))
		require.True(t, hasAnnotation(node, "WithValue"))
		require.False(t, hasAnnotation(node, "Missing"))
	})

	t.Run("Parameter", func(t *testing.T) {
		node := &ast.Parameter{Annotations: annotated}
		require.True(t, hasAnnotation(node, "Annotated"))
		require.True(t, hasAnnotation(node, "WithValue"))
		require.False(t, hasAnnotation(node, "Missing"))
	})

	t.Run("Typedef", func(t *testing.T) {
		// Test Annotations field
		node := &ast.Typedef{Annotations: annotated}
		require.True(t, hasAnnotation(node, "Annotated"))
		require.True(t, hasAnnotation(node, "WithValue"))
		require.False(t, hasAnnotation(node, "Missing"))

		// Test TypeAnnotations field
		nodeTypeAnnote := &ast.Typedef{TypeAnnotations: annotated}
		require.True(t, hasAnnotation(nodeTypeAnnote, "Annotated"))
		require.True(t, hasAnnotation(node, "WithValue"))
		require.False(t, hasAnnotation(nodeTypeAnnote, "Missing"))

		// Test both
		nodeBoth := &ast.Typedef{
			Annotations:     []*ast.Annotation{{Name: "A1"}},
			TypeAnnotations: []*ast.Annotation{{Name: "A2"}},
		}
		require.True(t, hasAnnotation(nodeBoth, "A1"))
		require.True(t, hasAnnotation(nodeBoth, "A2"))
		require.False(t, hasAnnotation(nodeBoth, "Missing"))
	})
}

func TestHasConstructor(t *testing.T) {
	constructorMember := &ast.Member{
		Type: &ast.TypeName{Name: "constructor"},
	}
	regularMember := &ast.Member{
		Type: &ast.TypeName{Name: "void"},
	}

	withConstructor := &ast.Interface{
		Members: []ast.InterfaceMember{regularMember, constructorMember},
	}
	withoutConstructor := &ast.Interface{
		Members: []ast.InterfaceMember{regularMember},
	}

	require.True(t, hasConstructor(withConstructor))
	require.False(t, hasConstructor(withoutConstructor))
}

func TestPromiseDetection_HappyPath(t *testing.T) {
	tests := []struct {
		method    string
		isPromise bool
	}{
		{"asyncMethod", true},
		{"syncMethod", false},
		{"asyncValue", true},
		{"syncValue", false},
	}

	idl := `
		interface Test {
			Promise<void> asyncMethod();
			void syncMethod();
			Promise<long> asyncValue();
			long syncValue();
		};
	`
	parsed := parser.Parse(idl)
	require.Empty(t, parsed.Errors)
	_, decls := simplify(parsed)
	testIface := decls["Test"].(*ast.Interface)

	methods := methodsOf(testIface)
	methodMap := make(map[string]*Method)
	for _, m := range methods {
		methodMap[m.Name] = m
	}

	for _, tt := range tests {
		t.Run(tt.method, func(t *testing.T) {
			method := methodMap[tt.method]
			require.NotNil(t, method)
			retType := method.Overloads[0].Type
			require.Equal(t, tt.isPromise, isPromiseType(retType), "isPromiseType failed for %s", tt.method)
			require.Equal(t, tt.isPromise, returnsPromise(method), "returnsPromise failed for %s", tt.method)
		})
	}
}

func TestPromiseDetection_InconsistentOverloads(t *testing.T) {
	// Manually construct a method with inconsistent overloads
	method := &Method{
		Name: "mixedMethod",
		Overloads: []*ast.Member{
			{Type: &ast.ParametrizedType{Name: "Promise", Elems: []ast.Type{&ast.TypeName{Name: "void"}}}},
			{Type: &ast.TypeName{Name: "void"}},
		},
	}
	require.Panics(t, func() {
		returnsPromise(method)
	})
}

func TestSetlikeOf(t *testing.T) {
	setlikePattern := &ast.Pattern{Type: ast.Setlike}
	otherPattern := &ast.Pattern{Type: ast.Iterable}

	withSetlike := &ast.Interface{
		Patterns: []*ast.Pattern{otherPattern, setlikePattern},
	}
	withoutSetlike := &ast.Interface{
		Patterns: []*ast.Pattern{otherPattern},
	}
	notInterface := &ast.Dictionary{}

	require.Equal(t, setlikePattern, setlikeOf(withSetlike))
	require.Nil(t, setlikeOf(withoutSetlike))
	require.Nil(t, setlikeOf(notInterface))
}

func TestPascalCase(t *testing.T) {
	tests := []struct {
		input    string
		expected string
	}{
		{"foo_bar", "FooBar"},
		{"gl-Position", "GlPosition"},
		{"texture_2d", "Texture2D"},
		{"simple", "Simple"},
		{"camelCase", "CamelCase"},
		{"_leading_underscore", "LeadingUnderscore"},
		{"trailing_underscore_", "TrailingUnderscore"},
		{"multiple___underscores", "MultipleUnderscores"},
		{"123_numbers", "123Numbers"},
		{"mixed-separators_test", "MixedSeparatorsTest"},
	}

	for _, tt := range tests {
		t.Run(tt.input, func(t *testing.T) {
			result := pascalCase(tt.input)
			require.Equal(t, result, tt.expected)
		})
	}
}
