# Rendering Difference Test

This tool (`/test/renderdiff`) is a collection of scripts to run visual regression tests on both `gltf_viewer` models and standalone Filament sample applications, producing headless renderings for automated diff comparison against golden references.

This is mainly useful for continuous integration where GPUs are generally not available on cloud
machines. To perform software rasterization, these scripts are centered around [Mesa]'s
software rasterizers, but nothing bars us from using another rasterizer like [SwiftShader].
Additionally, we should be able to use GPUs where available (though this is more of a future
work).

The script `render.py` contains the core logic for taking input parameters (such as the test
description file) and executing the corresponding tests.

In the `test` directory is a list of test descriptions that are specified in JSON. Please see
[`test/renderdiff/FORMAT.md`](FORMAT.md) and [`test/renderdiff/tests/sample.json`](tests/sample.json)
to glean the structure, and [`arch.md`](../../arch.md) for the end-to-end framework architecture.
These tests support both `gltf_test` (glTF models) and `sample_test`
(standalone sample binaries), declaring which **renderers** they run on using the `platform-backend`
specification format (e.g., `"desktop-opengl"`, `"desktop-vulkan"`).

## Setting up python

The `renderdiff` project uses `python` extensively. To install the dependencies for producing
renderings, do the following step
- Set up a virtual environment (from the root directory)
  ```
  python3 -m venv venv
  . ./venv/bin/activate
  ```
- Install the rendering dependencies
  ```
  pip install -r test/renderdiff/src/rendering_requirements.txt
  ```
- Install the viewer depdencies
  ```
  pip install -r test/renderdiff/src/viewer_requirements.txt
  ```
- For the commands in the following section, do not exit the virtual environment. Once you've
  completed all your work, you can exit with
  ```
  deactivate
  ```

## Running the test locally

- To run the same presbumit as [`test-renderdiff`](presubmit-renderdiff), you can do
  ```
  bash test/renderdiff/local_test.sh
  ```
- This script will generate the renderings based on the current state of your repo.
  Additionally, it will also compare the generated images with corresponding images in the
  golden repo.
- To just render without running the test, you could use the following script
  ```
  bash test/renderdiff/generate.sh
  ```

> [!IMPORTANT]
> **The goldens are rendered on Linux/aarch64.** A golden's path records the platform and the
> backend (`desktop-opengl` and so on) but not the host, and different hosts produce different
> pixels for reasons unrelated to any change: a different compiler, a different Mesa build, a
> different libm. On any other host, including macOS, `local_test.sh` prints a warning and its
> comparison results are advisory. Rendering locally is still the fastest way to see what a change
> does; to judge it against the goldens, read the presubmit result.

## Command-line Options

- `--test=<path>`: Path to the test suite configuration JSON file (defaults to
  `test/renderdiff/tests/presubmit.json`). For example, `--test=test/renderdiff/tests/sample.json`.
- `--test_filter=<filter>`: Run a subset of tests using fnmatch wildcards (`*`). It filters against
  the pattern `{test.name}.{platform}-{backend}.{target}`.
- `--no_rebuild`: Skip rebuilding the executables (`gltf_viewer` and `filament-samples`).
- `--build-only`: Build the executables and stop, without rendering. Used by the CI job that warms
  the compiler cache.
- `--num_threads=<number>`: Set the number of threads for rendering. If not set, the system's default is used.

For example, to run all `MSAA` tests on Vulkan without rebuilding and using 8 threads:

```
bash test/renderdiff/local_test.sh --test_filter='MSAA.*vulkan*.*' --no_rebuild --num_threads=8
```

## Update the golden images

The golden images are stored in a github repository:
https://github.com/google/filament-assets. Filament team members should have access to write
to the repository. A typical flow for updating the goldens is to upload your changed images
into **branch** of `filament-assets`. This branch is paired with a PR or commit on the
`filament` repo.

Because the goldens have to come from the same host presubmit renders on, the images themselves
have to be produced by CI rather than on your machine. The routes below are ordered by how much you
have to do by hand; the last two are only correct if you are running them on Linux/aarch64.

### Automated update via commit message

If you are confident in your changes and want CI to handle the update for you, add the following
line to the commit message of your working branch on `filament`:

```
RDIFF_ACCEPT_NEW_GOLDENS
```

This has the following effects:
- The presubmit test `test-renderdiff` will be bypassed (it will not perform rendering or
  comparison).
- When the PR is merged, the postsubmit CI will automatically:
    1. Build Filament and generate the new images, on the same runner presubmit compares on.
    2. Upload them to a temporary branch in `filament-assets`.
    3. Merge that branch into `main`.

The cost of this route is that nothing verifies the new goldens before they land: presubmit does
not compare, so the first evidence that they are right is the next PR passing.

### Using the 'Renderdiff Goldens' workflow

To see the new goldens applied in presubmit before the change lands, generate them first:

- Run the **Renderdiff Goldens** workflow from the Actions tab, selecting your `filament` branch
  and giving it a branch name for the golden repo (for example `my-pr-branch-golden`). The workflow
  renders on the presubmit runner and pushes the images to that branch, overwriting it if it
  already exists.
- Add the following line to the commit message of your working branch on `filament`:
  ```
  RDIFF_BRANCH=my-pr-branch-golden
  ```

Doing the above has multiple effects:
- The presubmit test [`test-renderdiff`][presubmit-renderdiff] will test against the provided
  branch of the golden repo (i.e. `my-pr-branch-golden`), so you can confirm the change is green
  before it merges.
- If the PR is merged, then there is another workflow that will merge `my-pr-branch-golden`
  to the `main` branch of the golden repo.

### From the images a presubmit run produced

Every `test-renderdiff` run attaches its renders to the run as the `presubmit-renderdiff-result`
artifact, including runs whose comparison failed, which is exactly when the images are interesting.
This is the route to use when the **Renderdiff Goldens** workflow is not available, for instance
when the workflow file itself is only present on your branch.

- Download and unpack `presubmit-renderdiff-result` from the run.
- Build `diffimg` locally, which the script uses to work out which images actually changed:
  ```
  ./build.sh debug diffimg
  ```
- Push the images to a branch of the golden repo. This needs write access to `filament-assets`,
  over SSH by default:
  ```
  python3 test/renderdiff/src/update_golden.py \
      --branch=my-pr-branch-golden \
      --source=<unpacked artifact>/renders \
      --commit-msg="New goldens for my change" \
      --push-to-remote
  ```
- Add `RDIFF_BRANCH=my-pr-branch-golden` to the commit message, as above.

### Manually updating the golden repo

- Check out the golden repo
  ```
  git clone git@github.com:google/filament-assets.git
  ```
- Create a branch on the golden repo
  ```
  cd filament-assets
  git switch -c my-pr-branch-golden
  ```
- Copy the new images to their appropriate place in `filament-assets`
- Push the `filament-assets` working branch to remote
  ```
  git push origin my-pr-branch-golden
  ```
- In the commit message of your working branch on `filament`, add the following line
  ```
  RDIFF_BRANCH=my-pr-branch-golden
  ```

### Using a script to update the golden repo

For the interactive equivalent of the above, which walks you through generating, reviewing and
pushing the changes:

- Make sure you've completed the steps in 'Setting up python'
- Run interactive mode in the `update_golden.py` script.
  ```
  python3 test/renderdiff/src/update_golden.py
  ```
- Note the host caveat above: images generated this way are only the right goldens if you are on
  Linux/aarch64.

## Viewing test results

We provide a viewer for looking at the result of a test run. The viewer is a webapp that can
be used by pointing your browser to a localhost port. If you input the viewer with a PR or a
directory, it will parse the test result and show the results and the rendered and/or golden
images.

![Viewer](docs/images/renderdiff_example.png)

To run the viewer of a test output directory that has been generated locally, you would run
the following

```
python3 test/renderdiff/src/viewer.py --diff=[test output]
```

where `[test output]` is a directory containing the `compare_results.json` of the test run.
For example, it could be `out/renderdiff/diffs/presubmit` for the standard path to the
`presubmit` test output.

To see the results of a Pull Request initiated test run, you would do the following

```
python3 test/renderdiff/src/viewer.py --pr_number=[PR #] --github_token=[github token]
```

where `[PR #]` is the numeric ID of your pull request, and the `[github token]` is an acess
token that you (as a github user) needs to generate ([reference][github_token_ref]).

To see the results of a specific run, you would do the following

```
python3 test/renderdiff/src/viewer.py --run_number=[RUN #] --github_token=[github token]
```

where `[RUN #]` is the numeric ID of the run. You can find the run number in the URL of the
GitHub Actions page. For example, in the URL
`https://github.com/google/filament/actions/runs/18023632663/job/51286323708?pr=9264`,
the run number is `18023632663`.

[github_token_ref]: https://docs.github.com/en/authentication/keeping-your-account-and-data-secure/managing-your-personal-access-tokens
[Mesa]: https://docs.mesa3d.org
[SwiftShader]: https://github.com/google/swiftshader
[presubmit-renderdiff]: https://github.com/google/filament/blob/e85dfe75c86106a05019e13ccdbef67e030af675/.github/workflows/presubmit.yml#L118
