# Rendering Difference Test

We created a few scripts to run `gltf_viewer` and produce headless renderings.

This is mainly useful for continuous integration where GPUs are generally not available on cloud
machines. To perform software rasterization, these scripts are centered around [Mesa]'s software
rasterizers, but nothing bars us from using another rasterizer like [SwiftShader]. Additionally,
we should be able to use GPUs where available (though this is more of a future work).

The script `render.py` contains the core logic for taking input parameters (such as the test
description file) and then running gltf_viewer to produce the renderings.

In the `test` directory is a list of test descriptions that are specified in json.  Please see
`sample.json` to parse the structure.

## Running the test locally
 - To run the same presbumit as [`test-renderdiff`][presubmit-renderdiff], you can do
   ```
bash test/renderdiff/test.sh
   ```
 - This script will generate the renderings based on the current state of your repo.
   Additionally, it will also compare the generated images with corresponding images in the
   golden repo.
 - To just render without running the test, you could use the following script
   ```
bash test/renderdiff/generate.sh
   ```

## Update the golden images
The golden images are stored in a github repository: https://github.com/google/filament-assets.
Filament team members should have access to write to the repository. A typical flow for updating
the goldens is to upload your changed images into **branch** of `filament-assets`. This branch is
paired with a PR or commit on the `filament` repo.

As an example, imagine I am working on a PR, and I've uploaded my change, which is in a branch
called `my-pr-branch`, to `filament`. This PR requires updating the golden. We would do it
in the following fashion

### Using a script to update the golden repo

 - Run interactive mode in the `update_golden.py` script.
   ```
python3 test/renderdiff/src/update_golden.py
   ```
 - This will guide you through a series of steps to push the changes to a remote branch
   on `filament-assets`.

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
RDIFF_BBRANCH=my-pr-branch-golden
   ```
### Manually updating the golden repo

Doing the above has multiple effects:
 - The presubmit test [`test-renderdiff`][presubmit-renderdiff] will test against the provided
   branch of the golden repo (i.e. `my-pr-branch-golden`).
 - If the PR is merged, then there is another workflow that will merge `my-pr-branch-golden` to
   the `main` branch of the golden repo.

[Mesa]: https://docs.mesa3d.org
[SwiftShader]: https://github.com/google/swiftshader
[presubmit-renderdiff]: https://github.com/google/filament/blob/e85dfe75c86106a05019e13ccdbef67e030af675/.github/workflows/presubmit.yml#L118
