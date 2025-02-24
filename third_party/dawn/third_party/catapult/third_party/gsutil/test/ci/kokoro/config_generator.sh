#!/bin/bash

# Copyright 2019 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Create a .boto config file for gsutil to use in Kokoro tests.
# https://cloud.google.com/storage/docs/gsutil/commands/config

GSUTIL_KEY=$1
API=$2
OUTPUT_FILE=$3

# Get values from secrets files.
read -r MTLS_TEST_ACCOUNT_REFRESH_TOKEN < $4
read -r MTLS_TEST_ACCOUNT_CLIENT_ID < $5
read -r MTLS_TEST_ACCOUNT_CLIENT_SECRET < $6

MTLS_TEST_CERT_PATH=$7

if [[ $GSUTIL_KEY ]]; then
  echo "[Credentials]
gs_service_key_file = $GSUTIL_KEY

[GSUtil]
default_project_id = bigstore-gsutil-testing
prefer_api = $API
test_hmac_service_account = sa-hmac@bigstore-gsutil-testing.iam.gserviceaccount.com
test_hmac_list_service_account = sa-hmac-list@bigstore-gsutil-testing.iam.gserviceaccount.com
test_hmac_alt_service_account = sa-hmac2@bigstore-gsutil-testing.iam.gserviceaccount.com
test_impersonate_service_account = bigstore-gsutil-impersonation@bigstore-gsutil-testing.iam.gserviceaccount.com
test_impersonate_failure_account = no-impersonation@bigstore-gsutil-testing.iam.gserviceaccount.com" \
> $OUTPUT_FILE

else
  echo "[Credentials]
gs_oauth2_refresh_token = $MTLS_TEST_ACCOUNT_REFRESH_TOKEN
use_client_certificate = True
cert_provider_command = /bin/cat $MTLS_TEST_CERT_PATH

[GSUtil]
default_project_id = dcatest-281318
prefer_api = $API

[OAuth2]
client_id = $MTLS_TEST_ACCOUNT_CLIENT_ID
client_secret = $MTLS_TEST_ACCOUNT_CLIENT_SECRET" \
> $OUTPUT_FILE

fi
