// Copyright 2022 The Dawn & Tint Authors
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

// Package subcmd provides a multi-command interface for command line tools.
package subcmd

import (
	"context"
	"errors"
	"flag"
	"fmt"
	"net/http"
	"net/http/pprof"
	"os"
	"path/filepath"
	"strings"
	"text/tabwriter"
)

// ErrInvalidCLA is the error returned when an invalid command line argument was
// provided, and the usage was already printed.
var ErrInvalidCLA = errors.New("invalid command line args")

// InvalidCLA shows the flag usage, and returns ErrInvalidCLA
func InvalidCLA() error {
	flag.Usage()
	return ErrInvalidCLA
}

// Command is the interface for a command
// Data is a generic data type passed down to the sub-command when run.
type Command[Data any] interface {
	// Name returns the name of the command.
	Name() string
	// Desc returns a description of the command.
	Desc() string
	// RegisterFlags registers all the command-specific flags
	// Returns a list of mandatory arguments that must immediately follow the
	// command name
	RegisterFlags(context.Context, Data) ([]string, error)
	// Run invokes the command
	Run(context.Context, Data) error
}

// DefaultCommand is the interface for a command that should be used as the
// default if no command is specified on the command line. Only one command can
// be the default command.
type DefaultCommand interface {
	IsDefaultCommand()
}

// Run handles the parses the command line arguments, possibly invoking one of
// the provided commands.
// If the command line arguments are invalid, then an error message is printed
// and Run returns ErrInvalidCLA.
func Run[Data any](ctx context.Context, data Data, cmds ...Command[Data]) error {
	_, exe := filepath.Split(os.Args[0])

	flag.Usage = func() {
		out := flag.CommandLine.Output()
		tw := tabwriter.NewWriter(out, 0, 1, 0, ' ', 0)
		fmt.Fprintln(tw, exe, "[command]")
		fmt.Fprintln(tw)
		fmt.Fprintln(tw, "Commands:")
		for _, cmd := range cmds {
			name := cmd.Name()
			_, isDefault := cmd.(DefaultCommand)
			if isDefault {
				name += " (default)"
			}
			fmt.Fprintln(tw, "  ", name, "\t-", strings.Split(cmd.Desc(), "\n")[0])
		}
		fmt.Fprintln(tw)
		fmt.Fprintln(tw, "Common flags:")
		tw.Flush()
		flag.PrintDefaults()
	}

	profile := false
	flag.BoolVar(&profile, "profile", false, "enable a webserver at localhost:8080/profile that exposes a CPU profiler")
	mux := http.NewServeMux()
	mux.HandleFunc("/profile", pprof.Profile)

	// Look for a default command
	var defaultCmd Command[Data]
	for _, cmd := range cmds {
		_, isDefault := cmd.(DefaultCommand)
		if isDefault {
			if defaultCmd != nil {
				panic(fmt.Sprintf("both commands %v and %v implement DefaultCommand", defaultCmd.Name(), cmd.Name()))
			}
			defaultCmd = cmd
		}
	}

	var cmdName string
	if len(os.Args) > 1 {
		cmdName = os.Args[1]
	}

	args := os.Args[1:] // Skip executable name

	// Find the requested command
	cmd := findCmd(cmdName, cmds)
	if cmd != nil {
		args = args[1:] // Skip command name
	}

	help := strings.TrimLeft(cmdName, "-") == "help"
	if help {
		if len(args) > 1 { // help cmd ...
			cmdName = args[1]
			cmd = findCmd(cmdName, cmds)
		}
		if cmd == nil {
			flag.Usage()
			return nil
		}
	}

	if cmd == nil { // Command not found
		if defaultCmd == nil {
			return InvalidCLA() // No default
		}
		cmd = defaultCmd // Use default command
	}

	out := flag.CommandLine.Output()
	mandatory, err := cmd.RegisterFlags(ctx, data)
	if err != nil {
		return err
	}
	flag.Usage = func() {
		flagsAndArgs := append([]string{"<flags>"}, mandatory...)
		fmt.Fprintln(out, exe, cmd.Name(), strings.Join(flagsAndArgs, " "))
		fmt.Fprintln(out)
		fmt.Fprintln(out, cmd.Desc())
		fmt.Fprintln(out)
		fmt.Fprintln(out, "flags:")
		flag.PrintDefaults()
	}
	if help {
		flag.Usage()
		return nil
	}
	if err := flag.CommandLine.Parse(args); err != nil {
		return err
	}
	if nonFlagArgs := flag.Args(); len(nonFlagArgs) < len(mandatory) {
		fmt.Fprintln(out, "missing argument", mandatory[len(nonFlagArgs)])
		fmt.Fprintln(out)
		return InvalidCLA()
	}
	if profile {
		fmt.Println("download profile at: localhost:8080/profile")
		fmt.Println("then run: 'go tool pprof <file>'")
		go http.ListenAndServe(":8080", mux)
	}

	return cmd.Run(ctx, data)
}

func findCmd[Data any](name string, cmds []Command[Data]) Command[Data] {
	for _, c := range cmds {
		if c.Name() == name {
			return c
		}
	}
	return nil
}
