# Becoming a committer

Committers are people that are considered to have write access to the Dawn repository.
All CLs submitted to the Dawn repository need to be reviewed by one or (for non-trivial ones) two committers.
In addition to this requirement, all CLs must also be reviewed or authored by an OWNER of the changed files.

This privilege is granted with some expectation of responsibility: committers are people who care about the Dawn project and want to help it meet its goals.
A committer is not just someone who can make changes, but someone who has demonstrated their ability to collaborate with the team, get the most knowledgeable people to review code, contribute high-quality code, and follow through to fix issues (in code or tests).

A committer is a contributor to the Dawn project’s success and a citizen helping the projects succeed.

## How to become a committer?

_Note to Googlers: the process is a bit different for WebGPU team members, reach out to an @google.com top-level OWNER directly._

In a nutshell, contribute 20 non-trivial patches and get at least three different people to review them (you'll need three people to support you).
Then ask someone to nominate you.
You're demonstrating your:

*   commitment to the project (20 good patches requires a lot of your valuable time),
*   ability to collaborate with the team,
*   understanding of how the team works (policies, processes for testing and code review, etc),
*   understanding of the projects' code base and coding style, and
*   ability to write good code (last but certainly not least)

A current committer nominates you by sending email to one of the top-level OWNERs containing:

*   your first and last name
*   your email address in Gerrit
*   an explanation of why you should be a committer,
*   embedded list of links to revisions (about top 10) containing your patches

Top-level OWNERs will deliberate and come to a consensus, targeting to reach it in 5 working days.
Once you get approval, you are granted additional CR+2 review permissions.
In some cases the process could drag out slightly.
Ping OWNERs and keep writing patches!
Even in the rare cases where a nomination fails, the objection is usually something easy to address like “more patches” or “not enough people are familiar with this person’s work.”

## Maintaining committer status

You don't really need to do much to maintain committer status: just keep being awesome and helping the Dawn project!

In the unhappy event that a committer continues to disregard good citizenship (or actively disrupts the project), we may need to revoke that person's status.
The process is the same as for nominating a new committer: someone suggests the revocation with a good reason and OWNERs deliberate.
We hope that's simple enough, and that we never have to test it in practice.

In addition, as a security measure, if you are inactive on Gerrit (no upload, no comment and no review) for more than a year, we may revoke your committer privileges.
An email notification is sent about 7 days prior to the removal.
This is not meant as a punishment, so if you wish to resume contributing after that, contact top level OWNERs to ask that it be restored, and we will normally do so.

(This document was inspired by [https://v8.dev/docs/become-committer](https://v8.dev/docs/become-committer) itself inspired by its Chromium equivalent)

