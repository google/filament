# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

PYTHON_VERSION_COMPATIBILITY = 'PY3'

DEPS = [
  'gsutil',
  'recipe_engine/path',
]


def RunSteps(api):
  """Move things around in a loop!"""
  local_file = api.path.tmp_base_dir / 'boom'
  bucket = 'example'
  cloud_file = 'some/random/path/to/boom'

  api.gsutil.upload(local_file, bucket, cloud_file,
      metadata={
        'Test-Field': 'value',
        'Remove-Me': None,
        'x-custom-field': 'custom-value',
        'Cache-Control': 'no-cache',
      },
      unauthenticated_url=True)

  # Upload without retry wrapper.
  api.gsutil.upload(local_file, bucket, cloud_file,
      metadata={
        'Test-Field': 'value',
        'Remove-Me': None,
        'x-custom-field': 'custom-value',
        'Cache-Control': 'no-cache',
      },
      unauthenticated_url=True,
      parallel_upload=True,
      multithreaded=True,
      use_retry_wrapper=False)

  # Upload directory contents.
  api.gsutil.upload(
      api.path.tmp_base_dir,
      bucket,
      'some/random/path',
      args=['-r'],
      name='upload -r')
  api.gsutil.upload(
      api.path.tmp_base_dir,
      bucket,
      'some/other/random/path',
      args=['--recursive'],
      name='upload --recursive')

  api.gsutil(['cp',
                    'gs://%s/some/random/path/**' % bucket,
                    'gs://%s/staging' % bucket])

  api.gsutil(['cp',
                    'gs://%s/some/random/path/**' % bucket,
                    'gs://%s/staging' % bucket], version='3.25')

  api.gsutil.download_url(
      'https://storage.cloud.google.com/' + bucket + '/' + cloud_file,
      local_file,
      name='gsutil download url')

  # Non-normalized URL.
  try:
    api.gsutil.download_url(
        'https://someotherservice.localhost',
        local_file,
        name='gsutil download url')
  except AssertionError:
    pass

  new_cloud_file = 'staging/to/boom'
  new_local_file = api.path.tmp_base_dir / 'erang'
  api.gsutil.download(bucket, new_cloud_file, new_local_file)

  private_key_file = 'path/to/key'
  _signed_url = api.gsutil.signurl(private_key_file, bucket, cloud_file,
                                  name='signed url')
  api.gsutil.remove_url('gs://%s/%s' % (bucket, new_cloud_file))

  api.gsutil.list('gs://%s/foo' % bucket)
  api.gsutil.copy(bucket, cloud_file, bucket, new_cloud_file)

  api.gsutil.cat('gs://%s/foo' % bucket)

  api.gsutil.stat('gs://%s/foo' % bucket)

  # Run in dry-run mode.
  api.gsutil.cat('gs://%s/foo' % bucket,
                 name='read remote file',
                 multithreaded=True,
                 dry_run=True)


def GenTests(api):
  yield api.test('basic')
