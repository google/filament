# Filament Release Guide

This guide makes use of some "environment variables":
- $RELEASE = the new version of Filament we are releasing today. (e.g., 1.9.3)
- $NEXT_RELEASE = the version we plan to release next week (e.g., 1.9.4)

Before starting, ensure that each of these branches is up-to-date with origin:
- release
- rc/$RELEASE
- main

## 0. Check versions.

Make sure the rc/$RELEASE branch has the correct Filament version. It should have the version
corresponding to its name, $RELEASE.

Make sure `MATERIAL_VERSION` has been bumped to a new version if this is a MAJOR or MINOR release
(first two version numbers).

## 1. Bump Filament versions on main to $RELEASE.

Checkout main and run the following command to bump Filament's version to $RELEASE:

```
build/common/bump-version.sh $RELEASE
```

Commit changes to main with the title:

```
Release Filament $RELEASE
```

Do not push to origin yet.

## 2. Update RELEASE_NOTES.md on main.

Create a new header in RELEASE_NOTES.md for $NEXT_RELEASE. Copy the release notes in
NEW_RELEASE_NOTES.md to RELEASE_NOTES.md under the new header. Clear NEW_RELEASE_NOTES.md.

Amend these changes to the "Release Filament $RELEASE" commit.

```
git add -u
git commit --amend --no-edit
```

## 3. Run release script.

```
build/common/release.sh rc/$RELEASE rc/$NEXT_RELEASE
```

This script will merge rc/$RELEASE into release, delete the rc branch, and create a new rc
branch called rc/$NEXT_RELEASE. Verify that everything looks okay locally.

## 4. Push the release branch.

```
git push origin release
```

## 5. Create the GitHub release.

Use the GitHub UI to create a GitHub release corresponding to $RELEASE version.
Make sure the target is set to the release branch.

## 6. Delete the old rc branch (optional).

This step is optional. The old rc branch may be left alive for a few weeks for posterity.

```
git push origin --delete rc/$RELEASE
```

## 7. Bump the version on the new rc branch to $NEXT_RELEASE.

```
git checkout rc/$NEXT_RELEASE
build/common/bump-version.sh $NEXT_RELEASE
```

Commit the changes to rc/$NEXT_RELEASE with the title:

```
Bump version to $NEXT_RELEASE
```

## 8. Push main.

```
git push origin main
```

## 9. Push the new rc branch.

```
git push origin -u rc/$NEXT_RELEASE
```

## 10. Rebuild the GitHub release (if failed).

Sometimes the GitHub release job will fail. In this case, you can manually re-run the release job.

### Remove any assets uploaded to the release (if needed).

For example, if rebuilding the Mac release, ensure that the `filament-<version>-mac.tgz` artifact
is removed from the release assets.

### Update the release branch (if needed).

If you need to add one or more new commits to the release, perform the following:

First, push the new commit(s) to the `release` branch.

Then, with the release branch checked out with the new commit(s), run

```
git tag -f -a <release tagname>
git push origin -f <release tagname>
```

This will update and force push the tag.

### Re-run the GitHub release workflow

Navigate to [Filament's release
workflow](https://github.com/google/filament/actions/workflows/release.yml). Hit the _Run workflow_
dropdown. Modify _Platform to build_ and _Release tag to build_, then hit _Run workflow_. This will
initiate a new release run.

## 11. Kick off the npm and CocoaPods release jobs

Navigate to [Filament's npm deploy
workflow](https://github.com/google/filament/actions/workflows/npm-deploy.yml).
Hit the _Run workflow_ dropdown. Modify _Release tag to deploy_ to the tag corresponding to this
release (for example, v1.42.2).

Navigate to [Filament's CocoaPods deploy
workflow](https://github.com/google/filament/actions/workflows/cocopods-deploy.yml).
Hit the _Run workflow_ dropdown. Modify _Release tag to deploy_ to the tag corresponding to this
release (for example, v1.42.2).
