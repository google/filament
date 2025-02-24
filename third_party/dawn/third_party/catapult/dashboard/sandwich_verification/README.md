# Sandwich Verification

This directory hosts the source code for the Sandwich Verification module.

Before running any of the commands below, ensure you're in the
sandwich_verification folder. From the catapult root, you can run

```
cd dashboard/sandwich_verification
```

# How To Deploy To Prod
To deploy local code to production, call:

```
gcloud builds submit --region=us-central1 --config cloudbuild.yaml --project=chromeperf .
```

This will execute cloudbuild.yaml and will deploy the Cloud Workflow and all
Cloud functions in parallel. The deployed resources will have the "-prod"
suffix. (e.g. start-pinpoint-job-prod).

# How to Roll Production Back to a Previous Version

You will have to `git checkout` to the previous commit you want to roll back
to, and run the above deployment command from that commit.

# Test Changes on Staging Version

## Deploying to Staging

To test local changes on your own version, you can deploy them to staging with:

```
make deploy_staging
```

This will deploy the workflows and functions with the provided suffix version
(e.g. sandwich-verification-workflow-eduardoyap, start-pinpoint-job-eduardoyap
etc). You should be able to see your deployed functions in the [Cloud Functions
dashboard](https://pantheon.corp.google.com/functions/list?referrer=search&project=chromeperf)
and [Cloud Workflow dashboard](https://pantheon.corp.google.com/workflows?referrer=search&project=chromeperf).

## Running Staging Cloud Workflow

From the Cloud Workflow dashboard, you can click on your deployed workflow
version and then on the Execute button. You can provide test data for the
Workflow to execute. For example:

```
{
    "anomaly": {
        "benchmark": "speedometer2",
        "bot_name": "mac-m1_mini_2020-perf",
        "measurement": "AngularJS-TodoMVC",
        "end_git_hash": "777f2001441e9d82bad279fa84a3cb0d21eb2a9c",
        "start_git_hash": "777f2001441e9d82bad279fa84a3cb0d21eb2a9c",
        "story": "Speedometer2",
        "target": "performance_test_suite"
    }
}
```

You can skip waiting for a Pinpoint job to complete by providing your own job_id:

```
{
    "anomaly": {
        ...
    },
    "job_id": "172c55b2660000"
}
```

## Running Staging Cloud Functions

To test individual functions, you can click on the target function from the
[Cloud Functions dashboard](https://pantheon.corp.google.com/functions/list?referrer=search&project=chromeperf) and then click on the TESTING tab. You can provide
test data and the Test Command window will show you a command you can run from
your workstation. Example:

```
curl -m 70 -X POST https://poll-pinpoint-job-eduardoyap-kkdem5ntpa-uw.a.run.app \
-H "Authorization: bearer $(gcloud auth print-identity-token)" \
-H "Content-Type: application/json" \
-d '{
  "job_id": "15d9633dfa0000"
}'
```

# How To Test Locally

Prerequisites for this method:

- Install `functions-framework`: https://cloud.google.com/functions/docs/running/function-frameworks
  - This method is not intended to play nicely with vpython or venv or other python dependency resolution hacks.  It just uses pip. So if you
  can't use pip for whatever reason then this README cannot help you.
  - you may need to install `pip` first (e.g. on a cloudtop where it may not be there already)
  - you may need to run the `pip install` command with `--break-system-packages` to get around an error message
  - e.g. `pip install -r requirements.txt --break-system-packages` from this directory
- `curl`: should be installed already on most workstation environments
- `gcloud`: should also be installed before proceeding
- run `gcloud auth application-default login` (once should be enough, unless you change your auth login settings again for some reason),
  - allows the `functions-framework` dev server to add
  auth credentials to cabe.skia.org grpc requests
 - you'll get some cryptic errors about http 303 responses if you don't do this

In one terminal window, run the following:
```
functions-framework --target GetCabeAnalysis --debug
```
This should start up a local emulation of the cloud functions environment. It
should also log some diagnostic/debug info including the http port that it's
listening on. We'll assume that port is `8080` here.

In a second terminal window, run this command (the `-d` json payload is just
some example data; edit as necessary for your use case):
```
> curl localhost:4242 -X POST  -H "Content-Type: application/json"  -d '{"job_id":"1644c2ca160000", "anomaly":{"benchmark":"rendering.desktop", "measurement":"Compositing.Display.DrawToSwapUs"}}'
```

and it should print something like this to stdout:

```
{
  "statistic": {
    "control_median": 234.5450000010431,
    "lower": -0.5737619917789094,
    "p_value": 0.16015625,
    "treatment_median": 240.78499999977646,
    "upper": 4.42586858995615
  }
}
```

This should produce some output in both terminal windows, as well as generate
some server-side activity visible in the GCP console page for cabe.skia.org. The
request logs for the `cabeserver` container should also contain some evidence of it
handling the request from your local `functions-framework` devserver.
