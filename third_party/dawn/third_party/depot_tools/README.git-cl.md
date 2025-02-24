# git-cl

The git-cl README describes the git-cl command set. This document describes how
code review and git work together in general, intended for people familiar with
git but unfamiliar with the code review process supported by Rietveld and
Gerrit.


## Basic interaction with git

The fundamental problem you encounter when you try to mix git and code review is
that with git it's nice to commit code locally, while during a code review
you're often requested to change something about your code. There are a few
different ways you can handle this workflow with git:

1. Rewriting a single commit. Say the origin commit is O, and you commit your
   initial work in a commit A, making your history like O--A. After review
   comments, you `git commit --amend`, effectively erasing A and making a new
   commit A', so history is now O--A'. (Equivalently, you can use
   `git reset --soft` or `git rebase -i`.)
2. Writing follow-up commits. Initial work is again in A, and after review
   comments, you write a new commit B so your history looks like O--A--B. When
   you upload the revised patch, you upload the diff of O..B, not A..B; you
   always upload the full diff of what you're proposing to change.

The Rietveld patch uploader just takes arguments to `git diff`, so either of the
above workflows work fine.  If all you want to do is upload a patch, you can use
the upload.py provided by Rietveld with arguments like this:

    upload.py --server server.com <args to "git diff">

The first time you upload, it creates a new issue; for follow-ups on the same
issue, you need to provide the issue number:

    upload.py --server server.com --issue 1234 <args to "git diff">


## git-cl to the rescue

git-cl simplifies the above in the following ways:

1. `git cl config` puts a persistent --server setting in your .git/config.
2. The first time you upload an issue, the issue number is associated with the
   current *branch*.  If you upload again, it will upload on the same issue.
   (Note that this association is tied to a branch, not a commit, which means
   you need a separate branch per review.)
3. If your branch is _tracking_ (in the `git checkout --track` sense) another
   one (like origin/main), calls to `git cl upload` will diff against that
   branch by default.  (You can still pass arguments to `git diff` on the
   command line, if necessary.)

In the common case, this means that calling simply `git cl upload` will always
upload the correct diff to the correct place.


## Patch series

The above is all you need to know for working on a single patch.

Things get much more complicated when you have a series of commits that you want
to get reviewed. Say your history looks like O--A--B--C. If you want to upload
that as a single review, everything works just as above.

But what if you upload each of A, B, and C as separate reviews? What if you
then need to change A?

1. One option is rewriting history: write a new commit A', then use
   `git rebase -i` to insert that diff in as O--A--A'--B--C as well as squash
   it. This is sometimes not possible if B and C have touched some lines
   affected by A'.
2. Another option, and the one espoused by software like topgit, is for you to
   have separate branches for A, B, and C, and after writing A' you merge it
   into each of those branches. (topgit automates this merging process.)  This
   is also what is recommended by git-cl, which likes having different branch
   identifiers to hang the issue number off of.  Your history ends up looking
   like:

       O---A---B---C
            \   \   \
             A'--B'--C'

   Which is ugly, but it accurately tracks the real history of your work, can be
   thrown away at the end by committing A+A' as a single `squash` commit.

In practice, this comes up pretty rarely. Suggestions for better workflows are
welcome.

## Bash auto completion

1. Ensure that your base git commands are autocompleted
[doc](https://git-scm.com/book/en/v1/Git-Basics-Tips-and-Tricks).
2. Add this to your .bashrc:

        # The next line enables bash completion for git cl.
        if [ -f "$HOME/bin/depot_tools/git_cl_completion.sh" ]; then
          . "$HOME/bin/depot_tools/git_cl_completion.sh"
        fi

3. Profit.
