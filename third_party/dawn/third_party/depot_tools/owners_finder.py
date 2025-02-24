# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Interactive tool for finding reviewers/owners for a change."""

import os
import copy

import gclient_utils


def first(iterable):
    for element in iterable:
        return element


class OwnersFinder(object):
    COLOR_LINK = '\033[4m'
    COLOR_BOLD = '\033[1;32m'
    COLOR_GREY = '\033[0;37m'
    COLOR_RESET = '\033[0m'

    indentation = 0

    def __init__(self,
                 files,
                 author,
                 reviewers,
                 owners_client,
                 email_postfix='@chromium.org',
                 disable_color=False,
                 ignore_author=False):
        self.email_postfix = email_postfix

        if os.name == 'nt' or disable_color:
            self.COLOR_LINK = ''
            self.COLOR_BOLD = ''
            self.COLOR_GREY = ''
            self.COLOR_RESET = ''

        self.author = author

        filtered_files = files

        reviewers = list(reviewers)
        if author and not ignore_author:
            reviewers.append(author)

        # Eliminate files that existing reviewers can review.
        self.owners_client = owners_client
        approval_status = self.owners_client.GetFilesApprovalStatus(
            filtered_files, reviewers, [])
        filtered_files = [
            f for f in filtered_files
            if approval_status[f] != self.owners_client.APPROVED
        ]

        # If some files are eliminated.
        if len(filtered_files) != len(files):
            files = filtered_files

        self.files_to_owners = self.owners_client.BatchListOwners(files)

        self.owners_to_files = {}
        self._map_owners_to_files()

        self.original_files_to_owners = copy.deepcopy(self.files_to_owners)

        # This is the queue that will be shown in the interactive questions.
        # It is initially sorted by the score in descending order. In the
        # interactive questions a user can choose to "defer" its decision, then
        # the owner will be put to the end of the queue and shown later.
        self.owners_queue = []

        self.unreviewed_files = set()
        self.reviewed_by = {}
        self.selected_owners = set()
        self.deselected_owners = set()
        self.reset()

    def run(self):
        self.reset()
        while self.owners_queue and self.unreviewed_files:
            owner = self.owners_queue[0]

            if (owner in self.selected_owners) or (owner
                                                   in self.deselected_owners):
                continue

            if not any((file_name in self.unreviewed_files)
                       for file_name in self.owners_to_files[owner]):
                self.deselect_owner(owner)
                continue

            self.print_info(owner)

            while True:
                inp = self.input_command(owner)
                if inp in ('y', 'yes'):
                    self.select_owner(owner)
                    break

                if inp in ('n', 'no'):
                    self.deselect_owner(owner)
                    break

                if inp in ('', 'd', 'defer'):
                    self.owners_queue.append(self.owners_queue.pop(0))
                    break

                if inp in ('f', 'files'):
                    self.list_files()
                    break

                if inp in ('o', 'owners'):
                    self.list_owners(self.owners_queue)
                    break

                if inp in ('p', 'pick'):
                    self.pick_owner(gclient_utils.AskForData('Pick an owner: '))
                    break

                if inp.startswith('p ') or inp.startswith('pick '):
                    self.pick_owner(inp.split(' ', 2)[1].strip())
                    break

                if inp in ('r', 'restart'):
                    self.reset()
                    break

                if inp in ('q', 'quit'):
                    # Exit with error
                    return 1

        self.print_result()
        return 0

    def _map_owners_to_files(self):
        for file_name in self.files_to_owners:
            for owner in self.files_to_owners[file_name]:
                self.owners_to_files.setdefault(owner, set())
                self.owners_to_files[owner].add(file_name)

    def reset(self):
        self.files_to_owners = copy.deepcopy(self.original_files_to_owners)
        self.unreviewed_files = set(self.files_to_owners.keys())
        self.reviewed_by = {}
        self.selected_owners = set()
        self.deselected_owners = set()

        # Randomize owners' names so that if many reviewers have identical
        # scores they will be randomly ordered to avoid bias.
        owners = list(
            self.owners_client.ScoreOwners(self.files_to_owners.keys()))
        if self.author and self.author in owners:
            owners.remove(self.author)
        self.owners_queue = owners
        self.find_mandatory_owners()

    def select_owner(self, owner, findMandatoryOwners=True):
        if owner in self.selected_owners or owner in self.deselected_owners\
            or not (owner in self.owners_queue):
            return
        self.writeln('Selected: ' + owner)
        self.owners_queue.remove(owner)
        self.selected_owners.add(owner)
        for file_name in filter(
                lambda file_name: file_name in self.unreviewed_files,
                self.owners_to_files[owner]):
            self.unreviewed_files.remove(file_name)
            self.reviewed_by[file_name] = owner
        if findMandatoryOwners:
            self.find_mandatory_owners()

    def deselect_owner(self, owner, findMandatoryOwners=True):
        if owner in self.selected_owners or owner in self.deselected_owners\
            or not (owner in self.owners_queue):
            return
        self.writeln('Deselected: ' + owner)
        self.owners_queue.remove(owner)
        self.deselected_owners.add(owner)
        for file_name in self.owners_to_files[owner] & self.unreviewed_files:
            self.files_to_owners[file_name].remove(owner)
        if findMandatoryOwners:
            self.find_mandatory_owners()

    def find_mandatory_owners(self):
        continues = True
        for owner in self.owners_queue:
            if owner in self.selected_owners:
                continue
            if owner in self.deselected_owners:
                continue
            if len(self.owners_to_files[owner] & self.unreviewed_files) == 0:
                self.deselect_owner(owner, False)

        while continues:
            continues = False
            for file_name in filter(
                    lambda file_name: len(self.files_to_owners[file_name]) == 1,
                    self.unreviewed_files):
                owner = first(self.files_to_owners[file_name])
                self.select_owner(owner, False)
                continues = True
                break

    def print_file_info(self, file_name, except_owner=''):
        if file_name not in self.unreviewed_files:
            self.writeln(
                self.greyed(file_name + ' (by ' +
                            self.bold_name(self.reviewed_by[file_name]) + ')'))
        else:
            if len(self.files_to_owners[file_name]) <= 3:
                other_owners = []
                for ow in self.files_to_owners[file_name]:
                    if ow != except_owner:
                        other_owners.append(self.bold_name(ow))
                self.writeln(file_name + ' [' + (', '.join(other_owners)) + ']')
            else:
                self.writeln(
                    file_name + ' [' +
                    self.bold(str(len(self.files_to_owners[file_name]))) + ']')

    def print_file_info_detailed(self, file_name):
        self.writeln(file_name)
        self.indent()
        for ow in sorted(self.files_to_owners[file_name]):
            if ow in self.deselected_owners:
                self.writeln(self.bold_name(self.greyed(ow)))
            elif ow in self.selected_owners:
                self.writeln(self.bold_name(self.greyed(ow)))
            else:
                self.writeln(self.bold_name(ow))
        self.unindent()

    def print_owned_files_for(self, owner):
        # Print owned files
        self.writeln(self.bold_name(owner))
        self.writeln(
            self.bold_name(owner) + ' owns ' +
            str(len(self.owners_to_files[owner])) + ' file(s):')
        self.indent()
        for file_name in sorted(self.owners_to_files[owner]):
            self.print_file_info(file_name, owner)
        self.unindent()
        self.writeln()

    def list_owners(self, owners_queue):
        if (len(self.owners_to_files) - len(self.deselected_owners) -
                len(self.selected_owners)) > 3:
            for ow in owners_queue:
                if (ow not in self.deselected_owners
                        and ow not in self.selected_owners):
                    self.writeln(self.bold_name(ow))
        else:
            for ow in owners_queue:
                if (ow not in self.deselected_owners
                        and ow not in self.selected_owners):
                    self.writeln()
                    self.print_owned_files_for(ow)

    def list_files(self):
        self.indent()
        if len(self.unreviewed_files) > 5:
            for file_name in sorted(self.unreviewed_files):
                self.print_file_info(file_name)
        else:
            for file_name in self.unreviewed_files:
                self.print_file_info_detailed(file_name)
        self.unindent()

    def pick_owner(self, ow):
        # Allowing to omit domain suffixes
        if ow not in self.owners_to_files:
            if ow + self.email_postfix in self.owners_to_files:
                ow += self.email_postfix

        if ow not in self.owners_to_files:
            self.writeln(
                'You cannot pick ' + self.bold_name(ow) + ' manually. ' +
                'It\'s an invalid name or not related to the change list.')
            return False

        if ow in self.selected_owners:
            self.writeln('You cannot pick ' + self.bold_name(ow) +
                         ' manually. ' + 'It\'s already selected.')
            return False

        if ow in self.deselected_owners:
            self.writeln('You cannot pick ' + self.bold_name(ow) +
                         ' manually.' + 'It\'s already unselected.')
            return False

        self.select_owner(ow)
        return True

    def print_result(self):
        # Print results
        self.writeln()
        self.writeln()
        if len(self.selected_owners) == 0:
            self.writeln('This change list already has owner-reviewers for all '
                         'files.')
            self.writeln('Use --ignore-current if you want to ignore them.')
        else:
            self.writeln('** You selected these owners **')
            self.writeln()
            for owner in self.selected_owners:
                self.writeln(self.bold_name(owner) + ':')
                self.indent()
                for file_name in sorted(self.owners_to_files[owner]):
                    self.writeln(file_name)
                self.unindent()

    def bold(self, text):
        return self.COLOR_BOLD + text + self.COLOR_RESET

    def bold_name(self, name):
        return (self.COLOR_BOLD + name.replace(self.email_postfix, '') +
                self.COLOR_RESET)

    def greyed(self, text):
        return self.COLOR_GREY + text + self.COLOR_RESET

    def indent(self):
        self.indentation += 1

    def unindent(self):
        self.indentation -= 1

    def print_indent(self):
        return '  ' * self.indentation

    def writeln(self, text=''):
        print(self.print_indent() + text)

    def hr(self):
        self.writeln('=====================')

    def print_info(self, owner):
        self.hr()
        self.writeln(
            self.bold(str(len(self.unreviewed_files))) + ' file(s) left.')
        self.print_owned_files_for(owner)

    def input_command(self, owner):
        self.writeln('Add ' + self.bold_name(owner) + ' as your reviewer? ')
        return gclient_utils.AskForData(
            '[yes/no/Defer/pick/files/owners/quit/restart]: ').lower()
