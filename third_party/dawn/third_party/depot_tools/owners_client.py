# Copyright (c) 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import random

import gerrit_util
import git_common


class OwnersClient(object):
    """Interact with OWNERS files in a repository.

    This class allows you to interact with OWNERS files in a repository both the
    Gerrit Code-Owners plugin REST API, and the owners database implemented by
    Depot Tools in owners.py:

        - List all the owners for a group of files.
        - Check if files have been approved.
        - Suggest owners for a group of files.

    All code should use this class to interact with OWNERS files instead of the
    owners database in owners.py
    """
    # '*' means that everyone can approve.
    EVERYONE = '*'

    # Possible status of a file.
    # - INSUFFICIENT_REVIEWERS: The path needs owners approval, but none of its
    #   owners is currently a reviewer of the change.
    # - PENDING: An owner of this path has been added as reviewer, but approval
    #   has not been given yet.
    # - APPROVED: The path has been approved by an owner.
    APPROVED = 'APPROVED'
    PENDING = 'PENDING'
    INSUFFICIENT_REVIEWERS = 'INSUFFICIENT_REVIEWERS'

    def ListOwners(self, path):
        """List all owners for a file.

        The returned list is sorted so that better owners appear first.
        """
        raise Exception('Not implemented')

    def BatchListOwners(self, paths):
        """List all owners for a group of files.

        Returns a dictionary {path: [owners]}.
        """
        if not paths:
            return dict()
        nproc = min(gerrit_util.MAX_CONCURRENT_CONNECTION, len(paths))
        with git_common.ScopedPool(nproc, kind='threads') as pool:
            return dict(
                pool.imap_unordered(lambda p: (p, self.ListOwners(p)), paths))

    def GetFilesApprovalStatus(self, paths, approvers, reviewers):
        """Check the approval status for the given paths.

        Utility method to check for approval status when a change has not yet
        been created, given reviewers and approvers.

        See GetChangeApprovalStatus for description of the returned value.
        """
        approvers = set(approvers)
        if approvers:
            approvers.add(self.EVERYONE)
        reviewers = set(reviewers)
        if reviewers:
            reviewers.add(self.EVERYONE)
        status = {}
        owners_by_path = self.BatchListOwners(paths)
        for path, owners in owners_by_path.items():
            owners = set(owners)
            if owners.intersection(approvers):
                status[path] = self.APPROVED
            elif owners.intersection(reviewers):
                status[path] = self.PENDING
            else:
                status[path] = self.INSUFFICIENT_REVIEWERS
        return status

    def ScoreOwners(self, paths, exclude=None):
        """Get sorted list of owners for the given paths."""
        if not paths:
            return []
        exclude = exclude or []
        owners = []
        queues = self.BatchListOwners(paths).values()
        for i in range(max(len(q) for q in queues)):
            for q in queues:
                if i < len(q) and q[i] not in owners and q[i] not in exclude:
                    owners.append(q[i])
        return owners

    def SuggestOwners(self, paths, exclude=None):
        """Suggest a set of owners for the given paths."""
        exclude = exclude or []

        paths_by_owner = {}
        owners_by_path = self.BatchListOwners(paths)
        for path, owners in owners_by_path.items():
            for owner in owners:
                paths_by_owner.setdefault(owner, set()).add(path)

        selected = []
        missing = set(paths)
        for owner in self.ScoreOwners(paths, exclude=exclude):
            missing_len = len(missing)
            missing.difference_update(paths_by_owner[owner])
            if missing_len > len(missing):
                selected.append(owner)
            if not missing:
                break

        return selected


class GerritClient(OwnersClient):
    """Implement OwnersClient using OWNERS REST API."""
    def __init__(self, host, project, branch):
        super(GerritClient, self).__init__()

        self._host = host
        self._project = project
        self._branch = branch
        self._owners_cache = {}
        self._best_owners_cache = {}

        # Seed used by Gerrit to shuffle code owners that have the same score.
        # Can be used to make the sort order stable across several requests,
        # e.g. to get the same set of random code owners for different file
        # paths that have the same code owners.
        self._seed = random.getrandbits(30)

    def _FetchOwners(self, path, cache, highest_score_only=False):
        # Always use slashes as separators.
        path = path.replace(os.sep, '/')
        if path not in cache:
            # GetOwnersForFile returns a list of account details sorted by order
            # of best reviewer for path. If owners have the same score, the
            # order is random, seeded by `self._seed`.
            data = gerrit_util.GetOwnersForFile(
                self._host,
                self._project,
                self._branch,
                path,
                resolve_all_users=False,
                highest_score_only=highest_score_only,
                seed=self._seed)
            cache[path] = [
                d['account']['email'] for d in data['code_owners']
                if 'account' in d and 'email' in d['account']
            ]
            # If owned_by_all_users is true, add everyone as an owner at the end
            # of the owners list.
            if data.get('owned_by_all_users', False):
                cache[path].append(self.EVERYONE)
        return cache[path]

    def ListOwners(self, path):
        return self._FetchOwners(path, self._owners_cache)

    def ListBestOwners(self, path):
        return self._FetchOwners(path,
                                 self._best_owners_cache,
                                 highest_score_only=True)

    def BatchListBestOwners(self, paths):
        """List only the higest-scoring owners for a group of files.

        Returns a dictionary {path: [owners]}.
        """
        with git_common.ScopedPool(kind='threads') as pool:
            return dict(
                pool.imap_unordered(lambda p: (p, self.ListBestOwners(p)),
                                    paths))


def GetCodeOwnersClient(host, project, branch):
    """Get a new OwnersClient.

    Uses GerritClient and raises an exception if code-owners plugin is not
    available."""
    if gerrit_util.IsCodeOwnersEnabledOnHost(host):
        return GerritClient(host, project, branch)
    raise Exception(
        'code-owners plugin is not enabled. Ask your host admin to enable it '
        'on %s. Read more about code-owners at '
        'https://chromium-review.googlesource.com/'
        'plugins/code-owners/Documentation/index.html.' % host)
