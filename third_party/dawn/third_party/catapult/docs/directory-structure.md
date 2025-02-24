# Adding a directory to catapult

## Where should the code live?

Catapult is intended to be a set of performance tools, mostly based on tracing,
for developers of Chromium and other software to analyze that software’s
performance. It has a lot of supporting libraries and tooling to make this
happen. We’d like to create an organizational structure that makes it easy to
find the performance tooling developers want, and hard to accidentally depend
on something internal. Furthermore, we’d like to make it easy for code in
catapult to eventually grow to its own repo, when possible. To that end, we
use these guidelines to decide where code should live.

  * Is it a **performance product**? Meant be used by external developers for
    performance analysis? Some examples include telemetry, perf dashboard. If
    so, it should be at `toplevel`.
  * Is it used only by our buildbot or build process? Put it in
    `catapult_build`.
  * If it's neither of the above, it should go in `common/`
  * If it is aspiring to be its own repo someday, that doesn't affect where it
    goes. You should follow the above rules for directory placement. Third
    party must only be real third party repos to conform to rules of repos
    which include catapult.
  * If something is experimental, then talk with the catapult admins to build
    the best guess of where it should go.

## How should directories be structured?
We have some rules on directory structure to add consistency and avoid
overloaded python imports.

  * Toplevel directories are **not** modules. E.g. if `x` is a toplevel
    directory, `x/__init__.py` **does not** exist. Directories in `common/`
    do not have this restriction.
  * Toplevel directories and directories in `common` should centralize all
    their path computation and sys.path manipulation in their master init file
    ([example](https://github.com/catapult-project/catapult/blob/master/telemetry/telemetry/__init__.py)).
  * Projects using web server should provide a module which defines all the
    search paths to their html & javascript resources in their top directory
    ([example](https://github.com/catapult-project/catapult/blob/master/dashboard/dashboard_project.py)).
  * Build code should be separate from production code. Build scripts for
    projects should be in `x/x_build/`
  * If you have a feature that has an implementation in JS and Py, then it
    should be in the same folder.
  * HTML search paths are arranged such that they have the same _name_ as they
    would in python. E.g. `tracing/tracing/base/math.html` is
    `/tracing/base/math.html` for an HTML import, and
    `tracing/tracing/base/math.py` is `tracing.base.math` in python.
  * Executable files (e.g. files chmodded to +x) must live in `x/bin`.
    `bin/` must not be a module, e.g. contain `__init__.py`, as such a name
    would create namespace conflicts. Executable files should **not** have a
    `.py` extension.
  * We use a single dev server, `$catapult/bin/run_dev_server`; and have
    per-project `bin/run_dev_server_tests` scripts.
  * All python modules should have unique names. `$catpult/catapult_build`
    instead of `$catapult/build`.

## How to add tests
Catapult supports two types of tests:

  * **dev_server** tests allow for UI testing and JavaScript testing. You can
    read more about adding them in the
    [dev_server tests guide](/docs/dev-server-tests.md). If you want to run
    dev_server tests, please create a `bin/run_dev_server_tests` python
    executable like [this](/dashboard/bin/run_dev_server_tests).
  * **python** tests use the [python unit testing framework]. If you want to run
    python tests, please create a `bin/run_py_tests` executable like
    [this](/catapult_build/bin/run_py_tests).

Both types of tests should be added to the configuration in
[build_steps.py](/catapult_build/build_steps.py). Please see the comments in
that file for full documentation on specifying test commands, arguments,
disabled platforms, and required environment variables.

[python unit testing framework]: https://docs.python.org/2/library/unittest.html
