package main

import (
	"flag"
	"fmt"
	"os"

	command "github.com/mikefarah/yq/v4/cmd"
)

var (
	yamlPath string
)

func main() {
	flag.StringVar(&yamlPath, "yaml", "", "path of the yaml spec")
	flag.Parse()

	arrays := []string{"constants", "typedefs", "enums", "bitflags", "structs", "callbacks", "functions", "objects"}
	for _, array := range arrays {
		SortArrayByFieldInPlace(array, "name")
	}
}

func SortArrayByFieldInPlace(array string, field string) {
	args := []string{"eval", "-i", fmt.Sprintf(".%s |= sort_by(.%s | downcase)", array, field), yamlPath}

	cmd := command.New()
	cmd.SetArgs(args)
	if err := cmd.Execute(); err != nil {
		os.Exit(1)
	}
}
