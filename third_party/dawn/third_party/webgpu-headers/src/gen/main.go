package main

import (
	"encoding/json"
	"flag"
	"os"
	"path/filepath"
	"slices"
	"strings"

	_ "embed"

	"github.com/goccy/go-yaml"
)

//go:embed cheader.tmpl
var tmpl string

var (
	schemaPath     string
	yamlPaths      StringListFlag
	outJsonPaths   StringListFlag
	outHeaderPaths StringListFlag
	extPrefix      bool
)

func main() {
	flag.StringVar(&schemaPath, "schema", "", "path of the json schema")
	flag.Var(&yamlPaths, "yaml", "path of the yaml spec")
	flag.Var(&outJsonPaths, "out-json", "output path of the json version of the yaml")
	flag.Var(&outHeaderPaths, "out-header", "output path of the header")
	flag.BoolVar(&extPrefix, "ext-prefix", true, "append prefix to extension identifiers")
	flag.Parse()
	if schemaPath == "" || len(yamlPaths) == 0 || len(outHeaderPaths) != len(yamlPaths) || len(outJsonPaths) != len(yamlPaths) {
		flag.Usage()
		os.Exit(1)
	}

	// Order matters for validation steps, so enforce it.
	if len(yamlPaths) > 1 && filepath.Base(yamlPaths[0]) != "webgpu.yml" {
		panic(`"webgpu.yml" must be the first sequence in the order`)
	}

	// Validate the yaml files (jsonschema, duplications)
	if err := ValidateYamls(schemaPath, yamlPaths); err != nil {
		panic(err)
	}

	// Generate the header files
	for i, yamlPath := range yamlPaths {
		outHeaderPath := outHeaderPaths[i]
		outHeaderFileName := filepath.Base(outHeaderPath)
		outHeaderFileNameSplit := strings.Split(outHeaderFileName, ".")
		if len(outHeaderFileNameSplit) != 2 {
			panic("got invalid out-header file name: " + outHeaderFileName)
		}

		outJsonPath := outJsonPaths[i]

		src, err := os.ReadFile(yamlPath)
		if err != nil {
			panic(err)
		}

		ConvertToJSON(src, outJsonPath)

		dst, err := os.Create(outHeaderPath)
		if err != nil {
			panic(err)
		}

		var yml Yml
		if err := yaml.Unmarshal(src, &yml); err != nil {
			panic(err)
		}

		SortAndTransform(&yml)

		g := &Generator{
			Yml:          &yml,
			HeaderName:   outHeaderFileNameSplit[0],
			UseExtPrefix: extPrefix,
		}
		if err := g.Gen(dst); err != nil {
			panic(err)
		}
	}
}

func UpperConcatCase(s string) string {
	return strings.ToUpper(PascalCase(s))
}

func SortAndTransform(yml *Yml) {
	// Sort structs
	SortStructs(yml.Structs)

	// Sort constants
	slices.SortStableFunc(yml.Constants, func(a, b Constant) int {
		return strings.Compare(UpperConcatCase(a.Name), UpperConcatCase(b.Name))
	})

	// Sort enums
	slices.SortStableFunc(yml.Enums, func(a, b Enum) int {
		// We want to generate extended enum declarations before the normal ones.
		if a.Extended && !b.Extended {
			return -1
		} else if !a.Extended && b.Extended {
			return 1
		}
		return strings.Compare(UpperConcatCase(a.Name), UpperConcatCase(b.Name))
	})

	// Sort bitflags
	slices.SortStableFunc(yml.Bitflags, func(a, b Bitflag) int {
		// We want to generate extended bitflag declarations before the normal ones.
		if a.Extended && !b.Extended {
			return -1
		} else if !a.Extended && b.Extended {
			return 1
		}
		return strings.Compare(UpperConcatCase(a.Name), UpperConcatCase(b.Name))
	})

	// Add free_member function for relevant structs
	for _, s := range yml.Structs {
		if s.FreeMembers {
			yml.Objects = append(yml.Objects, Object{
				Base:     Base{Name: s.Name, Namespace: s.Namespace},
				IsStruct: true,
				Methods: []Function{{
					Base: Base{
						Name:      "free_members",
						Namespace: s.Namespace,
						Doc:       "Frees members which were allocated by the API."},
				}},
			})
		}
	}

	// Sort objects
	slices.SortStableFunc(yml.Objects, func(a, b Object) int {
		return strings.Compare(UpperConcatCase(a.Name), UpperConcatCase(b.Name))
	})

	// Sort methods
	for _, obj := range yml.Objects {
		slices.SortStableFunc(obj.Methods, func(a, b Function) int {
			return strings.Compare(UpperConcatCase(a.Name), UpperConcatCase(b.Name))
		})
	}

	// Add add_ref and release methods for objects
	for i, o := range yml.Objects {
		if !o.Extended && !o.IsStruct {
			yml.Objects[i].Methods = append(yml.Objects[i].Methods,
				Function{
					Base: Base{
						Name:      "add_ref",
						Namespace: o.Namespace,
						Doc:       "TODO",
					},
				},
				Function{
					Base: Base{
						Name:      "release",
						Namespace: o.Namespace,
						Doc:       "TODO",
					},
				})
		}
	}
}

func ConvertToJSON(ymlString []byte, outJsonPath string) {
	var body interface{}
	if err := yaml.Unmarshal(ymlString, &body); err != nil {
		panic(err)
	}

	switch b := body.(type) {
	case map[string]interface{}:
		// Insert an extra copy of the copyright that will get sorted at the
		// top, and auto-gen warning.
		b["__copyright"] = b["copyright"]
		b["_comment"] = "AUTO-GENERATED FILE! Edit webgpu.yml instead."
	default:
		panic("unexpected")
	}

	body = convertToJSONHelper(body)

	if outJson, err := json.MarshalIndent(body, "", "  "); err != nil {
		panic(err)
	} else {
		outJson = append(outJson, '\n')
		if err := os.WriteFile(outJsonPath, outJson, 0644); err != nil {
			panic(err)
		}
	}
}

// From https://stackoverflow.com/a/40737676
// Note the original key order is not preserved as Go's map isn't ordered.
// This is fine because JSON keys are unordered per spec.
func convertToJSONHelper(i interface{}) interface{} {
	switch x := i.(type) {
	case map[interface{}]interface{}:
		m2 := map[string]interface{}{}
		for k, v := range x {
			m2[k.(string)] = convertToJSONHelper(v)
		}
		return m2
	case []interface{}:
		for i, v := range x {
			x[i] = convertToJSONHelper(v)
		}
	}
	return i
}
