package common

import (
	"bytes"
	"context"
	"fmt"
	"os/exec"
	"strings"
)

// GenTestList queries the CTS for the newline delimited list of test names
func GenTestList(ctx context.Context, ctsDir, node string, testQuery string) (string, error) {
	// Run 'src/common/runtime/cmdline.ts' to obtain the full test list
	cmd := exec.CommandContext(ctx, node,
		"-e", "require('./src/common/tools/setup-ts-in-node.js');require('./src/common/runtime/cmdline.ts');",
		"--", // Start of arguments
		// src/common/runtime/helper/sys.ts expects 'node file.js <args>'
		// and slices away the first two arguments. When running with '-e', args
		// start at 1, so just inject a placeholder argument.
		"placeholder-arg",
		"--list",
		testQuery,
	)
	cmd.Dir = ctsDir

	stderr := bytes.Buffer{}
	cmd.Stderr = &stderr

	out, err := cmd.Output()
	if err != nil {
		return "", fmt.Errorf("failed to generate test list: %w\n%v", err, stderr.String())
	}

	tests := []string{}
	for _, test := range strings.Split(string(out), "\n") {
		if test != "" {
			tests = append(tests, test)
		}
	}

	return strings.Join(tests, "\n"), nil
}
