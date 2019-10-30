#!/bin/bash

set -e

function print_help {
    local SELF_NAME=`basename $0`
    echo "$SELF_NAME. Upload artifacts to Filament's Google Cloud Storage."
    echo ""
    echo "Usage:"
    echo "    $SELF_NAME <destination> <artifact path>..."
    echo ""
    echo "Notes:"
    echo "    <destination> is a Google Cloud bucket folder URI (e.g., gs://filament-build/foobar/)"
    echo "    <artifact>    is the path to an artifact to upload to the GCS bucket"
    echo ""
    echo "    The GCLOUD_KEY_BASE64 environment variable must be set to a base64 encoded GCS service"
    echo "account key with write permissions to Filament's bucket."
}

if [[ $# -lt 2 ]]; then
    print_help
    exit 1
fi

DESTINATION="$1"
shift
ARTIFACTS="$@"

echo "Uploading artifacts ${ARTIFACTS} to ${DESTINATION}."

# Install the gcloud sdk.
WORKFLOW_OS=`echo \`uname\` | sed "s/Darwin/mac/" | tr [:upper:] [:lower:]`
source `dirname $0`/../$WORKFLOW_OS/install_gcloud_sdk.sh

gcloud --version
gsutil --version

# Authenticate.
echo $GCLOUD_KEY_BASE64 | base64 --decode > "$HOME/gcloud-key.json"
gcloud auth activate-service-account --key-file=$HOME/gcloud-key.json

gsutil cp "${ARTIFACTS}" "${DESTINATION}"
