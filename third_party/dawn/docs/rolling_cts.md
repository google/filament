# Rolling CTS

Notes and instructions for rolling CTS manually.

* [Roll history](https://dawn-review.googlesource.com/q/file:third_party/webgpu-cts+-%22%5BDO+NOT+SUBMIT%5D%22)

## Setup
There is a one-time setup needed to authenticate `gcloud` before the cts
roller can be used:

* `sudo apt install -y google-cloud-cli`
* `gcloud auth login`
* `gcloud auth application-default login`

### Verification
You can verify that you're authenticated by executing:

* `gcloud auth list`

which should list your authorizations


## Rolling CTS
The CTS roll can take a long time (sometimes >15 hrs) so it's
recommended to run it in `tmux`, `screen` or some other tool to allow
you to leave the session running in the background.

* `./tools/run cts roll`
