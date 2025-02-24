#!/usr/bin/env bash

NO_AUTOPREP=True
. common_demo_functions.sh

rm -rf $REMOTE
mkdir -p $REMOTE
cd $REMOTE

set_user "remote"

silent git init

WS=build/whitespace_file.txt

mkdir build
cat > "$WS" <<EOF
Copyright 2014 The Chromium Authors. All rights reserved.
Use of this useless file is governed by a BSD-style license that can be
found in the LICENSE file.


This file is used for making non-code changes to trigger buildbot cycles. Make
any modification below this line.

=====================================================================

Let's make a story. Add one sentence for every commit:

CHÄPTER 1:
It was a dark and blinky night; the rain fell in torrents -- except at
occasional intervals, when it was checked by a violent gust of wind which
swept up the streets (for it is in London that our scene lies), rattling along
the housetops, and fiercely agitating the scanty flame of the lamps that
struggled against the elements. A hooded figure emerged.

It was a Domo-Banana.

"What took you so long?", inquired his wife.

Silence. Oblivious to his silence, she continued, "Did Mr. Usagi enjoy the
waffles you brought him?" "You know him, he's not one to forego a waffle,
no matter how burnt," he snickered.

The pause was filled with the sound of compile errors.

CHAPTER 2:
The jelly was as dark as night, and just as runny.
The Domo-Kun shuddered, remembering the way Mr. Usagi had speared his waffles
with his fork, watching the runny jelly spread and pool across his plate,
like the blood of a dying fawn. \"It reminds me of that time --\" he started, as
his wife cut in quickly: \"-- please. I can't bear to hear it.\". A flury of
images coming from the past flowed through his mind.
EOF
git add "$WS"

c "Always output seccomp error messages to stderr"
c "ozone: evdev: Filter devices by path"
c "ContentView->ContentViewCore in ContentViewRenderView"
c "linux_aura: Use system configuration for middle clicking the titlebar."
c "[fsp] Add requestUnmount() method together with the request manager."
c "don't use glibc-specific execinfo.h on uclibc builds"
c "Make ReflectorImpl use mailboxes"

git tag stage_1

c "Change the Pica load benchmark to listen for the polymer-ready event"
c "Remove AMD family check for the flapper crypto accelerator."
c "Temporarily CHECK(trial) in ChromeRenderProcessObserver::OnSetFieldTrialGroup."

echo -e '/Banana\ns/Banana/Kun\nwq' | silent ed "$WS"
git add "$WS"
set_user 'local'
c "Fix terrible typo."

set_user 'remote'
c "Revert 255617, due to it not tracking use of the link doctor page properly."

cat >> "$WS" <<EOF

"You recall what happened on Mulholland drive?" The ceiling fan rotated slowly
overhead, barely disturbing the thick cigarette smoke. No doubt was left about
when the fan was last cleaned.

There was an poignant pause.
EOF
git add "$WS"
set_user 'local'
c 'Finish chapter 2'

git tag stage_2

cat >> "$WS" <<EOF

CHAPTER 3:
Hilariousness! This chapter is awesome!
EOF
git add "$WS"
set_user 'remote'
c "Add best chapter2 ever!"

c "Ensure FS is exited for all not-in-same-page navigations."
c "Refactor data interchange format."
