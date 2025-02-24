# ANGLE's Code Review Process

This page describes the review process for ANGLE reviewers and committers. For
instructions on submitting your change list for review, please see
[ContributingCode](ContributingCode.md).

## Reviewing Changes

The author of a CL may designate reviewers. Please feel free to weigh in on
changes even if you are not a designated reviewer!

1.  To review a change, you can either navigate directly to the URL for the CL,
    or, if you are one of the designated reviewers, the change will appear in
    your dashboard at https://chromium-review.googlesource.com/
2.  Review the change listed by looking over the diffs listed in the most recent
    patch set.
    *   You may view the diffs either side-to-side, or in unified diff format.
    *   You can comment on a specific line of code by double-clicking that line,
        or on the file as a whole by clicking the "Add file comment" icon, which
        appears above the diff, in the line number column.
    *   Note that, for CLs submitted as fixes to standing bugs, style issues
        that pre-exist the CL are not required to be addressed in the CL. As a
        reviewer, you can request a follow-up CL to address the style issue if
        you desire. This exception doesn't apply for CLs which implement new
        functionality, perform refactoring, or introduce style issues
        themselves.
3.  Once your review is complete, click the "Review" button
    *   If you are satisfied with the change list as it is, give a positive
        review (Code-Review +1 or +2).
    *   If you think the change list is a good idea, but needs changes, leave
        comments and a neutral review. (Code-Review 0)
    *   If you think the change list should be abandoned, give a negative
        review. (Code-Review -1 or -2)
    *   A +2 code review is required before landing. Only ANGLE committers may
        provide a +2 code review.
    *   ANGLE has a 2-reviewer policy for CLs. This means all changes should get
        a positive review from more than one person before they are accepted.
        This is most usually handled by reserving the +2 review for the second
        reviewer to clear the CL.
    *   If you made comments on the files, the draft comments will appear below
        the cover message. These comments are not published until you click on
        the "Publish Comments" button.
4.  Verification and landing:
    *   If the CL author is not an ANGLE committer, the CL should be verified
        and landed by a committer. Once verified, the "+1 Verified" status may
        be added, and the CL may be landed with the "Publish and Submit" button.
        There should be no need to rebase via the "Rebase Change" button prior
        to landing.
    *   If the CL author is an ANGLE committer, they should verify and land the
        CL themselves.
    *   Please note: Verification and commit-queue workflow may be subject to
        change in the near future.
5.  Cherry-picking to other branches
    *   If the change is needed on other branches, you may be able to land it
        using the "Cherry Pick To" button on the CL page.
    *   If this cherry pick fails, you will need to rebase the patch yourself
        and submit a new change for review on the branch.
