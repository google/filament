# Copyright (C) 2019 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from github import Github
import os, sys

def print_usage():
    print('Upload assets to a Filament GitHub release.')
    print()
    print('Usage:')
    print('  upload-assets.py <tag> <asset>...')
    print()
    print('Notes:')
    print('  The GitHub release must already be created prior to running this script. This is typically done')
    print('  through the GitHub web UI.')
    print()
    print('  <tag> is the Git tag for the desired release to attach assets to, for example, "v1.4.2".')
    print()
    print('  <asset> is a path to the asset file to upload. The file name will be used as the name of the')
    print('  asset.')
    print()
    print('  The GITHUB_API_KEY environment variable must be set to a valid authentication token for a')
    print('  collaborator account of the Filament repository.')

# The first argument is the path to this script.
if len(sys.argv) < 3:
    print_usage()
    sys.exit(1)

tag = sys.argv[1]
assets = sys.argv[2:]

authentication_token = os.environ.get('GITHUB_API_KEY')
if not authentication_token:
    sys.stderr.write('Error: the GITHUB_API_KEY is not set.\n')
    sys.exit(1)

g = Github(authentication_token)

FILAMENT_REPO = "google/filament"
filament = g.get_repo(FILAMENT_REPO)

def find_release_from_tag(repo, tag):
    """ Find a release in the repo for the given Git tag string. """
    releases = repo.get_releases()
    for r in releases:
        if r.tag_name == tag:
            return r
    return None

release = find_release_from_tag(filament, tag)
if not release:
    sys.stderr.write(f"Error: Could not find release with tag '{tag}'.\n")
    sys.exit(1)

print(f"Found release with tag '{tag}'.")

for asset_path in assets:
    sys.stdout.write(f'Uploding asset: {asset_path}... ')
    asset_name = os.path.basename(asset_path)
    asset = release.upload_asset(asset_path, name=asset_name)
    if asset:
        sys.stdout.write('Success!\n')
