# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
import os
import logging

from google.appengine.api import app_identity
from google.appengine.ext import ndb


def _is_devserver():
  server_software = os.environ.get('SERVER_SOFTWARE', '')
  return server_software and server_software.startswith('Development')

_DEFAULT_CATAPULT_PATH = '/catapult'
_DEFAULT_TARGET = 'prod'
if _is_devserver():
  _DEFAULT_TARGET = 'test'

_CONFIG_KEY_NAME = 'pi_cloud_mapper_config_%s' % _DEFAULT_TARGET

_DEFAULT_CONTROL_BUCKET_PATH = 'gs://%s/%s' % (
    app_identity.get_default_gcs_bucket_name(), _DEFAULT_TARGET)

_DEFAULT_SOURCE_DISK_IMAGE = ('https://www.googleapis.com/compute/v1/projects/'
  'debian-cloud/global/images/debian-8-jessie-v20151104')

_GCE_DEFAULT_ZONE = 'us-central1-f'
_GCE_DEFAULT_MACHINE_TYPE = 'n1-standard-1'


class CloudConfig(ndb.Model):
  control_bucket_path = ndb.StringProperty(default=_DEFAULT_CONTROL_BUCKET_PATH)
  setup_scheme = 'http' if _is_devserver() else 'https'
  default_corpus = ndb.StringProperty(
      default='%s://%s' % (
          setup_scheme, app_identity.get_default_version_hostname()))
  urlfetch_service_id = ndb.StringProperty(default='')
  gce_project_name = ndb.StringProperty(
      default=app_identity.get_application_id())
  gce_source_disk_image = ndb.StringProperty(default=_DEFAULT_SOURCE_DISK_IMAGE)
  gce_zone = ndb.StringProperty(default=_GCE_DEFAULT_ZONE)
  gce_machine_type = ndb.StringProperty(default=_GCE_DEFAULT_MACHINE_TYPE)
  trace_upload_bucket = ndb.StringProperty(
      default='%s/traces' % app_identity.get_default_gcs_bucket_name())
  catapult_path = ndb.StringProperty(default=_DEFAULT_CATAPULT_PATH)


def Get():
  config = CloudConfig.get_by_id(_CONFIG_KEY_NAME)
  if not config:
    logging.warning('CloudConfig found, creating a default one.')
    config = CloudConfig(id=_CONFIG_KEY_NAME)

    if 'GCS_BUCKET_NAME' in os.environ:
      config.trace_upload_bucket = os.environ['GCS_BUCKET_NAME']

    config.put()

  return config
