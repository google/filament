package main

import (
	"math"
	"regexp"
	"slices"
	"strings"
)

type Yml struct {
	Copyright  string `yaml:"copyright"`
	Name       string `yaml:"name"`
	Doc        string `yaml:"doc"`
	EnumPrefix uint16 `yaml:"enum_prefix"`

	Constants []Constant `yaml:"constants"`
	Typedefs  []Typedef  `yaml:"typedefs"`
	Enums     []Enum     `yaml:"enums"`
	Bitflags  []Bitflag  `yaml:"bitflags"`
	Structs   []Struct   `yaml:"structs"`
	Callbacks []Callback `yaml:"callbacks"`
	Functions []Function `yaml:"functions"`
	Objects   []Object   `yaml:"objects"`
}

type Base struct {
	Name      string `yaml:"name"`
	Namespace string `yaml:"namespace"`
	Doc       string `yaml:"doc"`
	Extended  bool   `yaml:"extended"`
}

type Constant struct {
	Base  `yaml:",inline"`
	Value string `yaml:"value"`
}

type Typedef struct {
	Base `yaml:",inline"`
	Type string `yaml:"type"`
}

type Enum struct {
	Base    `yaml:",inline"`
	Entries []*EnumEntry `yaml:"entries"`
}
type EnumEntry struct {
	Base  `yaml:",inline"`
	Value *uint16 `yaml:"value"`
}

type Bitflag struct {
	Base    `yaml:",inline"`
	Entries []BitflagEntry `yaml:"entries"`
}
type BitflagEntry struct {
	Base             `yaml:",inline"`
	Value            string   `yaml:"value"`
	ValueCombination []string `yaml:"value_combination"`
}

type PointerType string

const (
	PointerTypeMutable   PointerType = "mutable"
	PointerTypeImmutable PointerType = "immutable"
)

type ParameterType struct {
	Name                string      `yaml:"name"`
	Namespace           string      `yaml:"namespace"`
	Doc                 string      `yaml:"doc"`
	Type                string      `yaml:"type"`
	PassedWithOwnership *bool       `yaml:"passed_with_ownership"`
	Pointer             PointerType `yaml:"pointer"`
	Optional            bool        `yaml:"optional"`
	Default             *string     `yaml:"default"`
}

type Callback struct {
	Base  `yaml:",inline"`
	Style string          `yaml:"style"`
	Args  []ParameterType `yaml:"args"`
}

type Function struct {
	Base     `yaml:",inline"`
	Returns  *ParameterType  `yaml:"returns"`
	Callback *string         `yaml:"callback"`
	Args     []ParameterType `yaml:"args"`
}

type Struct struct {
	Base        `yaml:",inline"`
	Type        string          `yaml:"type"`
	FreeMembers bool            `yaml:"free_members"`
	Members     []ParameterType `yaml:"members"`
	Extends     []string        `yaml:"extends"`
}

type Object struct {
	Base    `yaml:",inline"`
	Methods []Function `yaml:"methods"`

	IsStruct bool `yaml:"-"`
}

var arrayTypeRegexp = regexp.MustCompile(`array<([a-zA-Z0-9._]+)>`)

func SortStructs(structs []Struct) {
	type node struct {
		visited    bool
		depth      int
		Extensions []string
		Struct
	}
	nodeMap := make(map[string]node, len(structs))
	for _, s := range structs {
		node := nodeMap[s.Name]
		node.Struct = s
		nodeMap[s.Name] = node
		if s.Type == "extension" {
			for _, extend := range s.Extends {
				parent := nodeMap[extend]
				parent.Extensions = append(parent.Extensions, s.Name)
				nodeMap[extend] = parent
			}
		}
	}

	slices.SortStableFunc(structs, func(a, b Struct) int {
		return strings.Compare(PascalCase(a.Name), PascalCase(b.Name))
	})

	var computeDepth func(string) int
	computeDepth = func(name string) int {
		node, ok := nodeMap[name]
		if !ok {
			panic("found invalid non-existing type: " + name)
		}

		if node.visited {
			return node.depth
		}

		maxDependentDepth := 0
		for _, member := range node.Members {
			if strings.HasPrefix(member.Type, "struct.") {
				dependentDepth := computeDepth(strings.TrimPrefix(member.Type, "struct."))
				maxDependentDepth = int(math.Max(float64(maxDependentDepth), float64(dependentDepth+1)))
			} else {
				matches := arrayTypeRegexp.FindStringSubmatch(member.Type)
				if len(matches) == 2 {
					typ := matches[1]
					if strings.HasPrefix(typ, "struct.") {
						dependentDepth := computeDepth(strings.TrimPrefix(typ, "struct."))
						maxDependentDepth = int(math.Max(float64(maxDependentDepth), float64(dependentDepth+1)))
					}
				}
			}
		}
		for _, extension := range node.Extensions {
			dependentDepth := computeDepth(extension)
			maxDependentDepth = int(math.Max(float64(maxDependentDepth), float64(dependentDepth+1)))
		}

		node.depth = maxDependentDepth
		node.visited = true
		nodeMap[name] = node
		return maxDependentDepth
	}

	for _, s := range structs {
		computeDepth(s.Name)
	}
	slices.SortStableFunc(structs, func(a, b Struct) int {
		return nodeMap[a.Name].depth - nodeMap[b.Name].depth
	})
}
