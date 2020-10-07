# Filament Release Guide

This guide makes use of some "environment variables":
- $RELEASE = the new version of Filament we are releasing today. (e.g., 1.9.3)
- $NEXT_RELEASE = the version we plan to release next week (e.g., 1.9.4)

Before starting, ensure that each of these branches is up-to-date with origin:
- release
- rc/$RELEASE
- main

## 0. Make sure the rc/$RELEASE branch has the correct version.

It should have the version corresponding to its name, $RELEASE.

## 1. Update RELEASE_NOTES.md on the rc branch.

Checkout the rc/$RELEASE branch. In RELEASE_NOTES.md, locate the header corresponding to $RELEASE
and write release notes. To see which commits make up the release, run:

```
build/common/release.sh -c rc/$RELEASE
```

Commit the changes to rc/$RELEASE with the title:

```
Update RELEASE_NOTES for $RELEASE
```

## 2. Bump versions on main to $RELEASE.

Checkout main and run the following command to bump Filament's version to $RELEASE:

```
build/common/bump-version.sh $RELEASE
```

Commit changes to main with the title:

```
Bump version to $RELEASE
```

Do not push to origin yet.

## 3. Cherry-pick RELEASE_NOTES change from rc branch to main.

```
git cherry-pick rc/$RELEASE
```

Update the headers. The "Next release" header becomes a header for $NEXT_RELEASE, and a new "Next
release" header is added.

For example, this:

```
## Next release (main branch)
- foo
- bar

## v1.9.3
- baz
- bat
```

becomes:

```
## Next release (main branch)

## v1.9.4
- foo
- bar

## v1.9.3
- baz
- bat
```

Ammend these changes to the cherry-picked change.

```
git add -u
git commit --amend --no-edit
```

## 4. Run release script.

```
build/common/release.sh rc/$RELEASE rc/$NEXT_RELEASE
```

This script will merge rc/$RELEASE into release, delete the rc branch, and create a new rc
branch called rc/$NEXT_RELEASE. Verify that everything looks okay locally.

## 5. Push the release branch.

```
git push origin release
```

## 6. Create the GitHub release.

Use the GitHub UI to create a GitHub release corresponding to $RELEASE version.
Make sure the target is set to the release branch.

## 7. Delete the old rc branch.

```
git push origin --delete rc/$RELEASE
```

## 8. Bump the version on the new rc branch to $NEXT_RELEASE.

```
git checkout rc/$NEXT_RELEASE
build/common/bump-version.sh $NEXT_RELEASE
```

Commit the changes to rc/$NEXT_RELEASE with the title:

```
Bump version to $NEXT_RELEASE
```

## 9. Push main.

```
git push origin main
```

## 10. Push the new rc branch.

```
git push origin rc/$NEXT_RELEASE
```
