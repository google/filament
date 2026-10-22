# Updating the benchmark library

This directory contains a script to update the version of the benchmark library used by Filament.

## Prerequisites

- `bash`: The script is a bash script.
- `curl`: Used to download the benchmark archive.
- `unzip`: Used to decompress the benchmark archive.
- `git`: To stage the updated files.

## Usage

To update the benchmark library, run the following command from this directory:

```bash
./update_benchmark.sh <version_tag>
```

Where `<version_tag>` is the version of benchmark you want to download (e.g., `1.9.4`).

Alternatively, you can specify a commit SHA instead of a version tag:

```bash
./update_benchmark.sh --sha <commit_sha>
```

The script will:

1. Download the specified version of benchmark from GitHub.
2. Unzip the archive.
3. Replace the existing `third_party/benchmark` directory with the new version (excluding the `tnt` directory).
4. Clean up the downloaded archive and temporary files.
5. Stage the changes in git.

After the script finishes, please review the changes and commit them.
