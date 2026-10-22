---
layout: spec-page
title: "README: Draco Spec Authoring Information"
---

<ol class="breadcrumb">
  <li class=""><a href="..">Home</a></li>
  <li class=""><a href=".">Draft Specification</a></li>
  <li class="">README</li>
</ol>


![Draco logo graphic](../artwork/draco3d-horiz-320x79.png)


# Authoring Info, Draco 3D Bitstream Specification
{:.no_toc}

This document, once published, will define the Draco 3D Geometry Compression
bitstream format and decoding process.

**Contents**

* TOC
{:toc}

The document is built from plain text section and subsection [Markdown] files
(more specifically, [kramdown] files) using the [Jekyll] static site building
tool. GitHub supports Jekyll natively.

The `docs/` directory of this project is meant to contain only documentation
and web content. The commands below should be issued from `docs/`. We do not
want to pollute the code tree with Ruby and Jekyll config information and
content.

The `docs/spec/` directory contains the files needed to work on this
specification document.


## Building Locally

Contributors will want to preview their edits locally before submitting pull
requests. Doing so requires a sane Ruby and rubygems environment. We use [rbenv]
and [bundler] to "groom" the project environment and avoid conflicts.

_**Important:** All commands are to be run as an ordinary, unprivileged user._


### Ruby and rbenv

This project currently depends on Ruby v2.3.0. Because your distro may lack this
version -- or installing it may conflict with your system's installed version --
first **[install rbenv]**, then install Ruby v2.3.0 within it (again, in
userland).

~~~~~
# list all available versions:
$ rbenv install -l
2.2.6
2.3.0-dev
2.3.0-preview1
2.3.0-preview2
2.3.0

# install a Ruby version:
$ rbenv install 2.3.0
~~~~~


### Bundler

Gem dependencies are managed by [bundler].

~~~~~
$ gem install bundler

# Filesystem location where gems are installed
$ gem env home
# => ~/.rbenv/versions/<ruby-version>/lib/ruby/gems/...
~~~~~


## Fork and Clone the Repo

First, use the **Fork** button on the repo's [homepage] to fork a copy to your
GitHub account.

Second, clone your fork locally:

~~~~~
git clone git@github.com:<username>/draco.git
cd draco
~~~~~

_**Note** that we **strongly** recommend [using SSH] with GitHub, not HTTPS._

Third, add a Git remote `upstream` that points to google/draco:

~~~~~
git remote add upstream git@github.com:google/draco.git
~~~~~

Your local repo with then have two remotes, `upstream` pointing at the
authoritative GitHub repo and `origin` pointing at your GitHub fork.

~~~~~
$ git remote
origin
upstream

$ git remote show origin
* remote origin
  Fetch URL: git@github.com:<username>/draco.git
  Push  URL: git@github.com:<username>/draco.git
  HEAD branch: master
  Remote branch:
    master tracked
  Local branch configured for 'git pull':
    master merges with remote master
  Local ref configured for 'git push':
    master pushes to master (up to date)

$ git remote show upstream
* remote upstream
  Fetch URL: git@github.com:google/draco.git
  Push  URL: git@github.com:google/draco.git
  HEAD branch: master
  Remote branch:
    master tracked
  Local ref configured for 'git push':
    master pushes to master (up to date)
~~~~~

[**See this page**][1] for a longer discussion of managing remotes and general
GitHub workflow.

**Important: The following commands should be issued from the `docs/`
directory.**


### Set Local Ruby Version (rbenv)

_**In the `docs/` directory**_ of your local clone, do:

~~~~~
rbenv local 2.3.0
~~~~~

Regardless of any other Rubies installed on your system, the project environment
will now use v2.3.0 and gems appropriate for it.


### Install Gem Dependencies with Bundler

In the `docs/` directory of your local clone, run

~~~~~
bundle install
~~~~~

Bundler will set dependencies and install needed gems as listed in
`Gemfile.lock`.

_**Note** that you may need Ruby development headers installed on your system
for some gems to compile successfully._


### Build and Preview Locally with Jekyll

~~~~~
bundle exec jekyll serve
~~~~~

This will build the documentation tree and launch a local webserver at
`http://127.0.0.1:4000/docs/` (by default). Jekyll will also watch the
the filesystem for changes and rebuild the document as needed.


## **Markdown & Formatting Conventions**

The spec document is composed mostly of syntax tables, styled with CSS. Mark
them up as follows:

  * Use the [fenced code block][fenced] kramdown syntax: A line beginning with
    three or more tildes (`~`) starts the code block, another such line ends it.

  * Use kramdown's [inline attribute syntax][inline] to apply the CSS class
   `draco-syntax` to your code block by placing `{:.draco-syntax }` on the line
   immediately after the code-block closing delimiter.

  * Some syntax elements as annotated with their type and size in a right-hand
    column. In your text editor, position these annotations at column **86**.

**Example:**

<pre><code>~~~~~
DecodeHeader() {
  draco_string                                                                       UI8[5]
  major_version                                                                      UI8
  minor_version                                                                      UI8
  encoder_type                                                                       UI8
  encoder_method                                                                     UI8
  flags
}
~~~~~
{:.draco-syntax}</code></pre>

... **will render as:**


~~~~~
DecodeHeader() {
  draco_string                                                                       UI8[5]
  major_version                                                                      UI8
  minor_version                                                                      UI8
  encoder_type                                                                       UI8
  encoder_method                                                                     UI8
  flags
}
~~~~~
{:.draco-syntax}


## General GitHub Workflow

Always do your work in a local branch.

~~~~~
git co -b my-branch-name
## work ##
git add <filenames or -A for all>
git ci -m "Reasonably clear commit message"
~~~~~

Push your branch to `origin` (your GitHub fork):

~~~~~
git push origin my-branch-name
~~~~~

Next, visit the `upstream` [homepage]. If you are logged-in, GitHub will be
aware of your recently pushed branch, and offer an in-page widget for submitting
a pull request for the project maintainers to consider.

Once your pull request is merged into upstream's master branch, you may
synchronize your clone (and remote `origin`) as follows:

~~~~~
git co master
git fetch upstream
git merge upstream/master
git push origin
~~~~~

Your old working branch is no longer needed, so do some housekeeping:

~~~~~
git br -d my-branch-name
~~~~~


[Markdown]: https://daringfireball.net/projects/markdown/
[kramdown]: https://kramdown.gettalong.org/
[Jekyll]: https://jekyllrb.com/
[rbenv]: https://github.com/rbenv/rbenv
[bundler]: http://bundler.io/
[install rbenv]: https://github.com/rbenv/rbenv#installation
[homepage]: https://github.com/google/draco
[using SSH]: https://help.github.com/articles/connecting-to-github-with-ssh/
[1]: https://2buntu.com/articles/1459/keeping-your-forked-repo-synced-with-the-upstream-source/
[fenced]: https://kramdown.gettalong.org/syntax.html#fenced-code-blocks
[inline]: https://kramdown.gettalong.org/syntax.html#block-ials

