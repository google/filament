# Building

Building the project is not usually needed for local development.
However, for exports to WPT, NodeJS, [or deployment](https://gpuweb.github.io/cts/),
files can be pre-generated.

## Build types

The project can be built several different ways, each with a different output directory:

### 0. on-the-fly builds (no output directory)

Use `npm run start` to launch a server that live-compiles everything as needed.
Use `tools/run_node` and other tools to run under `ts-node` which compiles at runtime.

### 1. `out` directory

**Built with**: `npm run standalone`

**Serve locally with**: `npx grunt serve`

**Used for**: Static deployment of the CTS, primarily for [gpuweb.github.io/cts](https://gpuweb.github.io/cts/).

### 2. `out-wpt` directory

**Built with**: `npm run wpt`

**Used for**: Deploying into [Web Platform Tests](https://web-platform-tests.org/). See [below](#export-to-wpt) for more information.

Contains:

- An adapter for running WebGPU CTS tests under WPT
- A copy of the needed files from `out/`
- A copy of any `.html` test cases from `src/`

### 3. `out-node` directory

**Built with**: `npm run node`

**Used for**: Running NodeJS tools, if you want to specifically avoid the live-compilation overhead of the `tools/` versions, or are running on a deployment which no longer has access to `ts-node` (which is a build-time dependency). For example:

- `node out-node/common/runtime/cmdline.js` ([source](../src/common/runtime/cmdline.ts)) - A command line interface test runner
- `node out-node/common/runtime/server.js` ([source](../src/common/runtime/server.ts)) - An HTTP server for executing CTS tests with a REST interface

## Testing

To build and run all pre-submit checks (including type and lint checks and
unittests), use:

```sh
npm test
```

For checks only:

```sh
npm run check
```


## Export to WPT

Run `npm run wpt`.

Copy (or symlink) the `out-wpt/` directory as the `webgpu/` directory in your
WPT checkout or your browser's "internal" WPT test directory.
