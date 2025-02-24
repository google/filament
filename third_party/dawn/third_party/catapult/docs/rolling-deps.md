# Updating Chromium's about:tracing (rolling DEPS)

Chromium's DEPS file needs to be rolled to the catapult revision containing your
change in order for it to appear in Chrome's about:tracing or other
third_party/catapult files. This should happen automatically, but you may need
to do it manually in rare cases. See below for more details.

## Automatic rolls

DEPS should be automatically rolled by the auto-roll bot at
[catapult-roll.skia.org](https://catapult-roll.skia.org/).
[catapult-sheriff@chromium.org](https://groups.google.com/a/chromium.org/forum/#!forum/catapult-sheriff)
will be cc-ed on all reviews, and anyone who wants to join that list can
subscribe. It's also the correct list to report a problem with the autoroll. If
you need to stop the autoroll, either sign into that page with a google.com
account, or contact catapult-sheriff@chromium.org.

## Manual rolls

In rare cases, you may need to make changes to chromium at the same time as you
roll catapult DEPs. In this case you would need to do a manual roll. Here are
instructions for rolling catapult DEPS, your CL would also include any other
changes to chromium needed to complete the roll.

First, commit to catapult. Then check the [mirror](https://chromium.googlesource.com/external/github.com/catapult-project/catapult.git)
to find the git hash of your commit. (Note: it may take a few minutes to be
mirrored).

Then edit Chrome's [src/DEPS](https://code.google.com/p/chromium/codesearch#chromium/src/DEPS)
file. Look for a line like:

```
  'src/third_party/catapult':
    Var('chromium_git') + '/external/github.com/catapult-project/catapult.git' + '@' +
    '2da8924915bd6fb7609c518f5b1f63cb606248eb',
```

Update the number to the git hash you want to roll to, and [contribute a
codereview to chrome](http://www.chromium.org/developers/contributing-code)
for your edit. If you are a Chromium committer, feel free to TBR this.
