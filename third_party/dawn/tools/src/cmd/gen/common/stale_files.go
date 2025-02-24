package common

import (
	"fmt"
	"path/filepath"
	"strings"

	"dawn.googlesource.com/dawn/tools/src/fileutils"
)

// StaleFiles is a list of generated file paths that need updating
type StaleFiles []string

func (l StaleFiles) Error() string {
	projectRoot := fileutils.DawnRoot()
	msg := &strings.Builder{}
	fmt.Fprintln(msg, len(l), "files need regenerating:")
	for _, path := range l {
		if rel, err := filepath.Rel(projectRoot, path); err == nil {
			fmt.Fprintln(msg, " •", rel)
		} else {
			fmt.Fprintln(msg, " •", path)
		}
	}
	fmt.Fprintln(msg, "Regenerate these files with: ./tools/run gen")
	return msg.String()
}
