package main

import (
	"errors"
	"fmt"
	"os"
	"slices"

	"github.com/goccy/go-yaml"
	"github.com/santhosh-tekuri/jsonschema/v5"
	_ "github.com/santhosh-tekuri/jsonschema/v5/httploader"
)

func ValidateYamls(schemaPath string, yamlPaths []string) error {
	// Validation through json schema
	for _, yamlPath := range yamlPaths {
		yamlFile, err := os.ReadFile(yamlPath)
		if err != nil {
			return fmt.Errorf("ValidateYaml: %w", err)
		}
		var yml map[string]any
		if err := yaml.Unmarshal(yamlFile, &yml); err != nil {
			return fmt.Errorf("ValidateYaml: %w", err)
		}

		schema := jsonschema.MustCompile(schemaPath)
		if err := schema.Validate(yml); err != nil {
			return fmt.Errorf("ValidateYaml: %w", err)
		}
	}

	// Validation of possible duplication of entries across multiple yaml files
	if err := mergeAndValidateDuplicates(yamlPaths); err != nil {
		panic(err)
	}

	// TODO: add dependency check validations
	return nil
}

func mergeAndValidateDuplicates(yamlPaths []string) (errs error) {
	constants := make(map[string]Constant)
	enums := make(map[string]Enum)
	bitflags := make(map[string]Bitflag)
	structs := make(map[string]Struct)
	callbacks := make(map[string]Callback)
	functions := make(map[string]Function)
	objects := make(map[string]Object)

	for _, yamlPath := range yamlPaths {
		src, err := os.ReadFile(yamlPath)
		if err != nil {
			panic(err)
		}

		var data Yml
		if err := yaml.Unmarshal(src, &data); err != nil {
			panic(err)
		}

		for _, c := range data.Constants {
			if _, ok := constants[c.Name]; ok {
				errs = errors.Join(errs, fmt.Errorf("merge: constants.%s in %s was already found previously while parsing, duplicates are not allowed", c.Name, yamlPath))
			}
			constants[c.Name] = c
		}
		for _, e := range data.Enums {
			if prevEnum, ok := enums[e.Name]; ok {
				if !e.Extended {
					errs = errors.Join(errs, fmt.Errorf("merge: enums.%s in %s is being extended but isn't marked as one", e.Name, yamlPath))
				}
				for _, entry := range e.Entries {
					if entry != nil {
						if slices.ContainsFunc(prevEnum.Entries, func(e *EnumEntry) bool { return e != nil && e.Name == entry.Name }) {
							errs = errors.Join(errs, fmt.Errorf("merge: enums.%s.%s in %s was already found previously while parsing, duplicates are not allowed", e.Name, entry.Name, yamlPath))
						}
						prevEnum.Entries = append(prevEnum.Entries, entry)
					}
				}
				enums[e.Name] = prevEnum
			} else {
				enums[e.Name] = e
			}
		}
		for _, bf := range data.Bitflags {
			if prevBf, ok := bitflags[bf.Name]; ok {
				if !bf.Extended {
					errs = errors.Join(errs, fmt.Errorf("merge: bitflags.%s in %s is being extended but isn't marked as one", bf.Name, yamlPath))
				}
				for _, entry := range bf.Entries {
					if slices.ContainsFunc(prevBf.Entries, func(e BitflagEntry) bool { return e.Name == entry.Name }) {
						errs = errors.Join(errs, fmt.Errorf("merge: bitflags.%s.%s in %s was already found previously while parsing, duplicates are not allowed", bf.Name, entry.Name, yamlPath))
					}
					if entry.Value == "" && len(entry.ValueCombination) == 0 {
						errs = errors.Join(errs, fmt.Errorf("merge: bitflags.%s.%s in %s was extended but doesn't have a value or value_combination, extended bitflag entries must have an explicit value", bf.Name, entry.Name, yamlPath))
					}
					prevBf.Entries = append(prevBf.Entries, entry)
				}
				bitflags[bf.Name] = prevBf
			} else {
				bitflags[bf.Name] = bf
			}
		}
		for _, c := range data.Callbacks {
			if _, ok := callbacks[c.Name]; ok {
				errs = errors.Join(errs, fmt.Errorf("merge: callbacks.%s in %s was already found previously while parsing, duplicates are not allowed", c.Name, yamlPath))
			}
			callbacks[c.Name] = c
		}
		for _, s := range data.Structs {
			if _, ok := structs[s.Name]; ok {
				errs = errors.Join(errs, fmt.Errorf("merge: structs.%s in %s was already found previously while parsing, duplicates are not allowed", s.Name, yamlPath))
			}
			structs[s.Name] = s
		}
		for _, f := range data.Functions {
			if _, ok := functions[f.Name]; ok {
				errs = errors.Join(errs, fmt.Errorf("merge: functions.%s in %s was already found previously while parsing, duplicates are not allowed", f.Name, yamlPath))
			}
			functions[f.Name] = f
		}
		for _, o := range data.Objects {
			if prevObj, ok := objects[o.Name]; ok {
				if !o.Extended {
					errs = errors.Join(errs, fmt.Errorf("merge: objects.%s in %s is being extended but isn't marked as one", o.Name, yamlPath))
				}
				for _, method := range o.Methods {
					if slices.ContainsFunc(prevObj.Methods, func(f Function) bool { return f.Name == method.Name }) {
						errs = errors.Join(errs, fmt.Errorf("merge: objects.%s.%s in %s was already found previously while parsing, duplicates are not allowed", o.Name, method.Name, yamlPath))
					}
					prevObj.Methods = append(prevObj.Methods, method)
				}
				objects[o.Name] = prevObj
			} else {
				objects[o.Name] = o
			}
		}

		for _, s := range structs {
			if s.Type == "extension" {
				for _, extend := range s.Extends {
					if _, exists := structs[extend]; !exists {
						errs = errors.Join(errs, fmt.Errorf("merge: struct.%s extends struct.%s that's not found (listed extends should not have the 'struct.' prefix)", s.Name, extend))
					}
				}
			}
		}
	}
	return
}
