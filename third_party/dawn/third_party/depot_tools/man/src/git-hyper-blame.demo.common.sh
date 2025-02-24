#!/usr/bin/env bash
. demo_repo.sh

# Construct a plausible file history.
set_user "lorem"
V1="Lorem ipsum dolor sit amet, consectetur*
adipiscing elit, sed do eiusmod tempor
incididunt ut labore et dolore magna*
aliqua. Ut enim ad minim veniam, quis
nostrud exercitation ullamco laboris**
nisi ut aliquip ex ea commodo consequat.*"
add "ipsum.txt" "$V1"
c "Added Lorem Ipsum"
tick 95408

set_user "ipsum"
V2="Lorem ipsum dolor sit amet, consectetur*
adipiscing elit, sed do eiusmod tempor
incididunt ut labore et dolore magna
aliqua. Ut enim ad minim veniam, quis
nostrud exercitation ullamco laboris**
nisi ut aliquip ex ea commodo consequat.*"
add "ipsum.txt" "$V2"
c "Change 1"
tick 4493194

set_user "dolor"
V3="Lorem ipsum dolor sit amet, consectetur*
adipiscing elit, sed do eiusmod tempor
incididunt ut labore et dolore magna
aliqua. Ut enim ad minim veniam, quis
nostrud exercitation ullamco laboris*
nisi ut aliquip ex ea commodo consequat."
add "ipsum.txt" "$V3"
c "Change 2"
tick 2817200

set_user "auto-uppercaser"
V4="LOREM IPSUM DOLOR SIT AMET, CONSECTETUR*
ADIPISCING ELIT, SED DO EIUSMOD TEMPOR
INCIDIDUNT UT LABORE ET DOLORE MAGNA
ALIQUA. UT ENIM AD MINIM VENIAM, QUIS
NOSTRUD EXERCITATION ULLAMCO LABORIS*
NISI UT ALIQUIP EX EA COMMODO CONSEQUAT."
add "ipsum.txt" "$V4"
c "Automatic upper-casing of all text."
tick 3273029

set_user "lorem"
V4="LOREM IPSUM DOLOR SIT AMET, CONSECTETUR
ADIPISCING ELIT, SED DO EIUSMOD TEMPOR
INCIDIDUNT UT LABORE ET DOLORE MAGNA
ALIQUA. UT ENIM AD MINIM VENIAM, QUIS
NOSTRUD EXERCITATION ULLAMCO LABORIS
NISI UT ALIQUIP EX EA COMMODO CONSEQUAT."
add "ipsum.txt" "$V4"
c "Change 3."
