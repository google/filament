# AI Tool Use Policy

This policy aims to be compatible with the [LLVM AI Tool Use
Policy](https://llvm.org/docs/AIToolPolicy.html) so that people contributing to
both projects have a similar policy to work with.

Contributors to DirectXShaderCompiler can use whatever tools they would like to
craft their contributions, but there must be a **human in the loop. Contributors
must read and review all LLM-generated code or text before they ask other
project members to review it.** The contributor is always the author and is
fully accountable for their contributions. Contributors should be sufficiently
confident that the contribution is high enough quality that asking for a review
is a good use of scarce maintainer time, and they should be **able to answer
questions about their work during review.**

We expect that new contributors will be less confident in their contributions,
and our guidance to them is to **start with small contributions** that they can
fully understand to build confidence. We aspire to be a welcoming community that
helps new contributors grow their expertise, but learning involves taking small
steps, getting feedback, and iterating. Passing maintainer feedback to an LLM
doesn't help anyone grow and does not sustain our community.

Contributors are expected to **be transparent and label contributions that
contain substantial amounts of tool-generated content.** Our policy on labelling
is intended to facilitate reviews, and not track which parts of the project are
generated. Contributors should note tool usage in their pull request
description, commit message, or wherever authorship is normally indicated for
the work. For instance, use a commit message trailer like Assisted-by: Copilot.
This transparency helps the community develop best practices and understand the
role of these new tools.

## Copilot Code Reviews

Copilot code reviews are allowed.  It's TBD whether we will enable these by
default for all PRs - but feel free to request a review from Copilot.

## Cloud Agents

The cloud-based version of GitHub Copilot is a great way to have multiple agents
work simultaneously and autonomously on issues.  However, we require that these
run in a fork of the repo rather than in the main repo itself.  Then, once the
change has been crafted such that it is ready for others to review, a PR can be
opened to merge it into upstream. Rationale:

* Everyone should be working in a fork rather than creating branches in the main
  repo, agents are no different. 
* We shouldn't be spamming people watching the main repo with the agent's work.
* As you're responsible for the work Copilot is doing, the PR into upstream
  should come from you, not from Copilot. 

Note that cloud agents are only able to build and test on Linux, and this
affects the sort of work an agent can do.

## Local Agents

CLI, or editor-hosted agents, run on your own machine, so there is less concern
about the activity of these agents impacting others.
