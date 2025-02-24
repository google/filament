# Automate CLs for Chromeperf deployment

Intended audience: Browser Perf Engprod rotation on-calls.

This tool automates some of the weekly deployment steps for chromeperf appengine services.

`update-traffic.go` checks the list of available GAE service versions to find two specific version IDs: the newest version, and the version that currently has a 1.0 allocation of traffic.  If the two version IDs are not the same, it will attempt to update the appropriate .yaml configuration file in your local checkout such that the latest version ID is the one with a 1.0 share of the service's traffic allocation.

`create-cls.sh` is a script that will create a branch for each service, run `update-traffic.go`, and commit any changes, and upload. The outputs of `update-traffic.go` should include the current and updated hashes as well as a gitile link reflecting the change.

# Usage

Since this hasn't been evaluated very heavily, we don't have a deployed binary package for it yet. The bash script is intended to be a temporary time-saving measure.

## create-cls

To run `create-cls.sh`, you may need to run `chmod 755 create-cls.sh` first. Navigate to `catapult/dashboard/go/src/update-traffic` and run:

```
./create-cls.sh
```

The script output should also include the gitile links for each service which can be copy and pasted into the commit message.

## update-traffic

`update-traffic` can be run using the `go` command like so:

```
go run update-traffic.go [options]
```

The intended use case is for updating the traffic allocation entries in various per-service yaml files.  Typically you will want to make a separate CL for each of these updates, and run `update-traffic` once per CL to generate the changed lines.

If you give it a `-checkout-base` flag, it will edit files in your checkout.  If you do not specify this flag, the command will print a summary of the service/version/traffic allocation updates it would have made.

# Example
Suppose you are doing a weekly deployment, which requires a separate CL for each service traffic allocation update.

```
./create-cls.sh
git checkout default
git cl upload
```

And repeat for the other services. Refer to the Deployment section in go/berf-rotation-playbook for additional info.


## Alternative
You could do the following once for each service (substitute `api`, `default`, `pinpoint`, etc for `perf-issue-service` below). From this directory (which should be something like `catapult/dashboard/go/src/update-traffic` in your checkout):
```
git checkout -b update-perf-issue-service
go run update-traffic.go -checkout-base ../../../.. -service-id perf-issue-service
git commit -am 'update yaml'
git cl upload
```

Then submit each CL for review in gerrit as described by go/berf-rotation-playbook in the Deployment section.
