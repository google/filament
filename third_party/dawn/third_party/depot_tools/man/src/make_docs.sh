#!/usr/bin/env bash

set -e

shopt -s nullglob

# disable metrics report
export DEPOT_TOOLS_METRICS=0

cd $(dirname "$0")

# Script which takes all the asciidoc git-*.txt files in this directory, renders
# them to html + manpage format using git 1.9's doc toolchain, then puts
# them in depot_tools to be committed.

ensure_in_path() {
  local CMD=$1
  local PTH=$(which "$CMD")
  if [[ ! $PTH ]]
  then
    echo Must have "$CMD" on your PATH!
    exit 1
  else
    echo Using \'$PTH\' for ${CMD}.
  fi
}

ensure_in_path xmlto
ensure_in_path git

DFLT_CATALOG_PATH="/usr/local/etc/xml/catalog"
if [[ ! $XML_CATALOG_FILES && -f "$DFLT_CATALOG_PATH" ]]
then
  # Default if you install doctools with homebrew on mac
  export XML_CATALOG_FILES="$DFLT_CATALOG_PATH"
  echo Using \'$DFLT_CATALOG_PATH\' for \$XML_CATALOG_FILES.
fi

# We pull asciidoc to get the right version
BRANCH=10.2.0
ASCIIDOC_HASH=545b79b8d7dae70d12bf0657359bdd36de0c5c26
if [[ ! -d asciidoc || $(cd asciidoc && git rev-parse HEAD) != $ASCIIDOC_HASH ]]
then
  echo Cloning asciidoc
  rm -rf asciidoc
  git clone --branch $BRANCH https://github.com/asciidoc-py/asciidoc-py asciidoc
  (cd asciidoc/asciidoc && ln -s asciidoc.py asciidoc && chmod ug+x asciidoc.py)
fi
echo Asciidoc up to date at $ASCIIDOC_HASH \($BRANCH\)

export PATH=`pwd`/asciidoc/asciidoc:$PATH

# We pull ansi2hash to convert demo script output
BRANCH=1.8.0
ANSI2HTML_HASH=545b79b8d7dae70d12bf0657359bdd36de0c5c26
if [[ ! -d ansi2html || $(git -C ansi2html rev-parse HEAD) != $ANSI2HTML_HASH ]]
then
  echo Cloning ansi2html
  rm -rf ansi2html
  git clone  --single-branch --branch $BRANCH --depth 1 \
    https://github.com/pycontribs/ansi2html.git 2> /dev/null
fi

echo ansi2html up to date at $ANSI2HTML_HASH \($BRANCH\)

# We pull git to get its documentation toolchain
BRANCH=v2.35.1
GITHASH=4c53a8c20f8984adb226293a3ffd7b88c3f4ac1a
if [[ ! -d git || $(git -C git rev-parse HEAD) != $GITHASH ]]
then
  echo Cloning git
  rm -rf git
  git clone --single-branch --branch $BRANCH --depth 1 \
    https://kernel.googlesource.com/pub/scm/git/git.git  2> /dev/null
ed git/Documentation/Makefile <<EOF
H
142
s/Git Manual/Chromium depot_tools Manual
s/Git/depot_tools
wq
EOF

  # fix Makefile to include non-_-prefixed files as MAN7 entries
  {
    shopt -s extglob
    echo H
    echo 35
    for x in "$(echo !\(git-*|_*\).txt)"
    do
      echo i
      echo MAN7_TXT += $x
      echo .
    done
    echo wq
  } | ed git/Documentation/Makefile

  # fix build-docdep.perl to ignore attributes on include::[] macros
  ed git/Documentation/build-docdep.perl <<EOF
H
12
c
	    s/\[[^]]*\]//;
.
wq
EOF

  # Add additional CSS override file
  ed git/Documentation/Makefile <<EOF
H
/ASCIIDOC_EXTRA
a
 -a stylesheet=$(pwd)/git/Documentation/asciidoc-override.css
.
-1
j
/XMLTO_EXTRA
a
 --skip-validation
.
-1
j
/^\$(MAN_HTML):
a
 asciidoc-override.css
.
-1
j
wq
EOF

  cat >> git/Documentation/asciidoc.conf <<EOF

[macros]
(?su)[\\\\]?(?P<name>demo):(?P<target>\S*?)\[\]=

[demo-inlinemacro]
{sys3:cd $(pwd); ./{docname}.demo.{target}.sh | python3 filter_demo_output.py {backend} }
EOF

fi
echo Git up to date at $GITHASH \($BRANCH\)

if [[ ! -d demo_repo ]]
then
  ./prep_demo_repo.sh
fi

# build directory files for 'essential' and 'helper' sections of the depot_tools
# manpage.
for category in helper essential
do
  {
    PRINTED_BANNER=0
    for x in *.${category}.txt
    do
      # If we actually have tools in the category, print the banner first.
      if [[ $PRINTED_BANNER != 1 ]]
      then
        PRINTED_BANNER=1
        # ex.
        # CATEGORY TOOLS
        # --------------
        BANNER=$(echo $category tools | awk '{print toupper($0)}')
        echo $BANNER
        for i in $(seq 1 ${#BANNER})
        do
          echo -n -
        done
        echo
        cat _${category}_prefix.txt 2> /dev/null || true
        echo
      fi

      # ex.
      # linkgit:git-tool[1]::
      # \tinclude::_git-tool_desc.category.txt[]
      PLAIN_PATH=${x%%_desc.*.txt}
      PLAIN_PATH=${PLAIN_PATH:1}
      echo "linkgit:$PLAIN_PATH[1]::"
      echo -e "include::${x}[]"
      echo
    done
  } > __${category}.txt
done

JOBS=1
HTML_TARGETS=()
MAN1_TARGETS=()
MAN7_TARGETS=()
for x in *.txt *.css
do
  TO="git/Documentation/$x"
  if [[ ! -f "$TO" ]] || ! cmp --silent "$x" "$TO"
  then
    echo \'$x\' differs
    cp $x "$TO"
    # Exclude files beginning with _ from the target list. This is useful to
    # have includable snippet files.
    if [[ ${x:0:1} != _ && ${x:(-4)} == .txt ]]
    then
      HTML_TARGETS+=("${x%%.txt}.html")
      if [[ ! "$NOMAN" ]]
      then
        if [[ ${x:0:3} == git ]]
        then
          MAN1_TARGETS+=("${x%%.txt}.1")
        else
          MAN7_TARGETS+=("${x%%.txt}.7")
        fi
      fi
      JOBS=$[$JOBS + 2]
    fi
  fi
done

if [[ ${#HTML_TARGETS} == 0 && ${#MAN1_TARGETS} == 0 && ${#MAN7_TARGETS} == 0 ]]
then
  echo No targets to build
  exit
fi

VER="v$(git rev-parse --short HEAD)"
if [[ ! -f git/version ]] || ! cmp --silent git/version <(echo "$VER")
then
  echo Version changed, cleaning.
  echo "$VER" > git/version
  (cd git/Documentation && make clean)
fi

# This export is so that asciidoc sys snippets which invoke git run relative to
# depot_tools instead of the git clone.
(
  export GIT_DIR="$(git rev-parse --git-dir)" &&
  cd git/Documentation &&
  GIT_EDITOR=true make "${MAN1_TARGETS[@]}" "${MAN7_TARGETS[@]}" "${HTML_TARGETS[@]}"
)

for x in "${HTML_TARGETS[@]}"
do
  echo Copying ../html/$x
  tr -d '\015' <"git/Documentation/$x"  >"../html/$x"
done

for x in "${MAN1_TARGETS[@]}"
do
  echo Copying ../man1/$x
  cp "git/Documentation/$x" ../man1
done

for x in "${MAN7_TARGETS[@]}"
do
  echo Copying ../man7/$x
  cp "git/Documentation/$x" ../man7
done
