# gclient

gclient is a tool for managing a modular checkout of source code from multiple
source code repositories. It wraps underlying source code management commands
to provide support for distributing tree updates, status commands, and diffs
across multiple checked-out working directories.

The gclient script is controlled by a `.gclient` file at the top of a directory
tree which will contain source code from multiple locations. A `.gclient` file
is a Python script that defines a list of `solutions` with the following format:

    solutions = [
      { "name"        : "src",
        "url"         : "https://chromium.googlesource.com/chromium/src.git",
        "custom_deps" : {
          # To use the trunk of a component instead of what's in DEPS:
          #"component": "https://github.com/luci/luci-go",
          # To exclude a component from your working copy:
          #"data/really_large_component": None,
        }
      },
    ]

A `solution` is a collection of component pieces of software that will be
checked out in a specific directory layout for building together.

Each entry in the `solutions` list is defined by a Python dictionary that
contains the following items:

- `name`: The name of the directory in which the solution will be checked out.
- `url`: The URL from which this solution will be checked out. gclient expects
  that the checked-out solution will contain a file named `DEPS` that in turn
  defines the specific pieces that must be checked out to create the working
  directory layout for building and developing the solution's software.
- `deps_file`: A string containing just the filename (not a path) of the file in
  the solution dir to use as the list of dependencies. This tag is optional, and
  defaults to `DEPS`.
- `custom_deps`: A dictionary containing optional custom overrides for entries
  in the solution's `DEPS` file.  This can be used to have the local working
  directory *not* check out and update specific components, or to sync the local
  working-directory copy of a given component to a different specific revision,
  or a branch, or the head of a tree. It can also be used to append new entries
  that do not exist in the `DEPS` file.

Within each checked-out solution, gclient expects to find a file typically named
`DEPS` (it actually uses the value of the `deps_file` key above) which defines
the different component pieces of software that must be checked out for the
solution. The `DEPS` file is a Python script that defines a dictionary named
`deps`:

    deps = {
      "src/outside": "https://outside-server/one/repo.git@12345677890123456778901234567789012345677890",
      "src/component": "https://dont-use-github.com/its/unreliable.git@0000000000000000000000000000000000000000",
      "src/relative": "/another/repo.git@aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
    }

Each item in the `deps` dictionary consists of a key-value pair. The key is the
directory into which the component will be checked out, relative to the
directory containing the `.gclient` file. The value is the URL from which that
directory will be checked out. If there is no address scheme (that is, no
`http:` prefix), then the value must begin with a slash and is treated
relative to the root of the solution's repository.

The URL typically contains a specific revision or change number (as appropriate
for the underlying SCM system) to `freeze` the external software at a specific,
known state. Alternatively, if there is no revision or change number, the URL
will track the latest changes on the specific trunk or branch.
