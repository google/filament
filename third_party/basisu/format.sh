#!/bin/bash

# Loops through all text files tracked by Git.
git grep -zIl '' |
while IFS= read -rd '' f; do
    # Exclude some types of files.
    if [[ $f == *"proj" ]]; then
        continue
    elif [[ $f == *"filters" ]]; then
        continue
    elif [[ $f == *"sln" ]]; then
        continue
    elif [[ $f == *"json" ]]; then
        continue
    elif [[ $f == *"min.js" ]]; then
        continue
    elif [[ $f == *"coder.js" ]]; then
        continue
    fi
    # Ensures that files are UTF-8 formatted.
    recode UTF-8 $f 2> /dev/null
    # Ensures that files have LF line endings.
    dos2unix $f 2> /dev/null
    # Ensures that files do not contain a BOM.
    sed -i '1s/^\xEF\xBB\xBF//' "$f"
    # Ensures that files end with newline characters.
    tail -c1 < "$f" | read -r _ || echo >> "$f";
done

git diff > patch.patch
FILESIZE=$(stat -c%s patch.patch)
MAXSIZE=5

# If no patch has been generated all is OK, clean up, and exit.
if (( FILESIZE < MAXSIZE )); then
    printf "Files in this commit comply with the formatting rules.\n"
    rm -f patch.patch
    exit 0
fi

# A patch has been created, notify the user, clean up, and exit.
printf "\n*** The following differences were found between the code "
printf "and the formatting rules:\n\n"
cat patch.patch
printf "\n*** Aborting, please fix your commit(s) with 'git commit --amend' or 'git rebase -i <hash>'\n"
rm -f patch.patch
exit 1
