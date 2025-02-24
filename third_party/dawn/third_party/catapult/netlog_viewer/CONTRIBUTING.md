# Contributing to Net Log Viewer

[TOC]


## Check-out the code

```
git clone https://chromium.googlesource.com/catapult
cd catapult/netlog_viewer/netlog_viewer
```


## Running locally for development

```
cd catapult/netlog_viewer
./bin/serve_static
```

The Net Log viewer has no server-side dependencies, so it can be loaded through
`index.html` using any static HTTP server (provided `components/` is
also mapped). See the `serve_static` script above for an example.

This is convenient for development using an edit/reload cycle, however this is
not quite what is deployed to https://netlog-viewer.appspot.com/.


## Running tests

```
cd catapult
./bin/run_dev_server --no-install-hooks --port 8111
```

Now navigate to
[http://localhost:8111/netlog_viewer/tests.html](http://localhost:8111/netlog_viewer/tests.html)
to run the tests and see their results.

Note that running the tests in headless mode does not currently work (i.e.
`netlog_viewer/bin/run_dev_server_tests`).


## Building a version for deployment

```
cd catapult/netlog_viewer
./netlog_viewer_build/build_for_appengine.py
```

This command will package all the HTML/JavaScript/CSS into a single
`vulcanized.html` file under `catapult/netlog_viewer/appengine/static/` (the
build outputs will show up as an untracked files by `git`, and should not be
committed).

The bundled app can be served by any static server. To test it using Flask,
serve it with:

```
cd catapult/netlog_viewer/appengine
pip3 install -r requirements.txt
python3 main.py
```

You may want to use [venv](https://docs.python.org/3/library/venv.html) to
install Flask in a virtual environment.

## Deploying to netlog-viewer.appspot.com

Only certain project OWNERS can publish the checked in code to
[https://netlog-viewer.appspot.com/](https://netlog-viewer.appspot.com/).

For those members, here are the [internal instructions](https://goto.google.com/deploy-cr-netlog-viewer).


## Reporting bugs

File a bug using [this chromium bug
template](https://bugs.chromium.org/p/chromium/issues/entry?components=Internals%3ENetwork%3ELogging)
which will add the component `Internals>Network>Logging`. Please also prefix
the title with `[NetLogViewer]`.


## Contributing changes

Changes should be proposed using a Gerrit code review, with the reviewer set to
one of the [NetLog OWNERS](OWNERS). For instructions on how to use the code
review system, see [catapult/CONTRIBUTING.md](../CONTRIBUTING.md).


## Known issues

This viewer code was extracted from Chromium and has [not yet been
modernized](https://bugs.chromium.org/p/chromium/issues/detail?id=1026294).
