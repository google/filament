# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import contextlib
import re

from recipe_engine import recipe_api

class GSUtilApi(recipe_api.RecipeApi):

  def __init__(self, env_properties, *args, **kwargs):
    super(GSUtilApi, self).__init__(*args, **kwargs)
    self._boto_config_path = env_properties.BOTO_CONFIG
    self._boto_path = env_properties.BOTO_PATH

  @property
  def gsutil_py_path(self):
    return self.repo_resource('gsutil.py')

  def __call__(self,
               cmd,
               name=None,
               use_retry_wrapper=True,
               version=None,
               parallel_upload=False,
               multithreaded=False,
               infra_step=True,
               dry_run=False,
               **kwargs):
    """A step to run arbitrary gsutil commands.

    On LUCI this should automatically use the ambient task account credentials.
    On Buildbot, this assumes that gsutil authentication environment variables
    (AWS_CREDENTIAL_FILE and BOTO_CONFIG) are already set, though if you want to
    set them to something else you can always do so using the env={} kwarg.

    Note also that gsutil does its own wildcard processing, so wildcards are
    valid in file-like portions of the cmd. See 'gsutil help wildcards'.

    Args:
      * cmd (List[str|Path]) - Arguments to pass to gsutil. Include gsutil-level
          options first (see 'gsutil help options').
      * name (str) - Name of the step to use. Defaults to the first non-flag
          token in the cmd.
      * dry_run (bool): If True, don't actually run the step; just log what
          the step would have been.
    """
    if name:
      full_name = 'gsutil ' + name
    else:
      full_name = 'gsutil'  # our fall-through name
      # Find first cmd token not starting with '-'
      for itm in cmd:
        token = str(itm)   # it could be a Path
        if not token.startswith('-'):
          full_name = 'gsutil ' + token
          break

    gsutil_path = self.gsutil_py_path
    cmd_prefix = []

    if use_retry_wrapper:
      # We pass the real gsutil_path to the wrapper so it doesn't have to do
      # brittle path logic.
      cmd_prefix = ['--', gsutil_path]
      gsutil_path = self.resource('gsutil_smart_retry.py')

    if version:
      cmd_prefix.extend(['--force-version', version])

    if parallel_upload:
      cmd_prefix.extend([
          '-o',
          'GSUtil:parallel_composite_upload_threshold=50M'
      ])

    if multithreaded:
      cmd_prefix.extend(['-m'])

    if use_retry_wrapper:
      # The -- argument for the wrapped gsutil.py is escaped as ---- as python
      # 2.7.3 removes all occurrences of --, not only the first. It is unescaped
      # in gsutil_wrapper.py and then passed as -- to gsutil.py.
      # Note, that 2.7.6 doesn't have this problem, but it doesn't hurt.
      cmd_prefix.append('----')
    else:
      cmd_prefix.append('--')

    exec_cmd = ['python3', '-u', gsutil_path] + cmd_prefix + cmd
    if dry_run:
      return self.m.step.empty(full_name,
                               step_text='Pretending to run gsutil command',
                               log_text=' '.join((str(i) for i in exec_cmd)),
                               log_name='command')
    return self.m.step(full_name, exec_cmd, infra_step=infra_step, **kwargs)

  def upload(self, source, bucket, dest, args=None, link_name='gsutil.upload',
             metadata=None, unauthenticated_url=False, **kwargs):
    args = [] if args is None else args[:]
    # Note that metadata arguments have to be passed before the command cp.
    metadata_args = self._generate_metadata_args(metadata)
    full_dest = 'gs://%s/%s' % (bucket, dest)
    cmd = metadata_args + ['cp'] + args + [source, full_dest]
    name = kwargs.pop('name', 'upload')

    result = self(cmd, name, **kwargs)

    if link_name:
      is_dir = '-r' in args or '--recursive' in args
      result.presentation.links[link_name] = self._http_url(
          bucket, dest, is_directory=is_dir, is_anonymous=unauthenticated_url)
    return result

  def download(self, bucket, source, dest, args=None, **kwargs):
    args = [] if args is None else args[:]
    full_source = 'gs://%s/%s' % (bucket, source)
    cmd = ['cp'] + args + [full_source, dest]
    name = kwargs.pop('name', 'download')
    return self(cmd, name, **kwargs)

  def download_url(self, url, dest, args=None, **kwargs):
    args = args or []
    url = self._normalize_url(url)
    cmd = ['cp'] + args + [url, dest]
    name = kwargs.pop('name', 'download_url')
    return self(cmd, name, **kwargs)

  def cat(self, url, args=None, **kwargs):
    args = args or []
    url = self._normalize_url(url)
    cmd = ['cat'] + args + [url]
    name = kwargs.pop('name', 'cat')
    return self(cmd, name, **kwargs)

  def stat(self, url, args=None, **kwargs):
    args = args or []
    url = self._normalize_url(url)
    cmd = ['stat'] + args + [url]
    name = kwargs.pop('name', 'stat')
    return self(cmd, name, **kwargs)

  def copy(self, source_bucket, source, dest_bucket, dest, args=None,
           link_name='gsutil.copy', metadata=None, unauthenticated_url=False,
           **kwargs):
    args = args or []
    args += self._generate_metadata_args(metadata)
    full_source = 'gs://%s/%s' % (source_bucket, source)
    full_dest = 'gs://%s/%s' % (dest_bucket, dest)
    cmd = ['cp'] + args + [full_source, full_dest]
    name = kwargs.pop('name', 'copy')

    result = self(cmd, name, **kwargs)

    if link_name:
      is_dir = '-r' in args or '--recursive' in args
      result.presentation.links[link_name] = self._http_url(
          dest_bucket, dest, is_directory=is_dir,
          is_anonymous=unauthenticated_url)
    return result

  def list(self, url, args=None, **kwargs):
    args = args or []
    url = self._normalize_url(url)
    cmd = ['ls'] + args + [url]
    name = kwargs.pop('name', 'list')
    return self(cmd, name, **kwargs)

  def signurl(self, private_key_file, bucket, dest, args=None, **kwargs):
    args = args or []
    full_source = 'gs://%s/%s' % (bucket, dest)
    cmd = ['signurl'] + args + [private_key_file, full_source]
    name = kwargs.pop('name', 'signurl')
    return self(cmd, name, **kwargs)

  def remove_url(self, url, args=None, **kwargs):
    args = args or []
    url = self._normalize_url(url)
    cmd = ['rm'] + args + [url]
    name = kwargs.pop('name', 'remove')
    return self(cmd, name, **kwargs)

  @contextlib.contextmanager
  def configure_gsutil(self, **kwargs):
    """Temporarily configures the behavior of gsutil.

    For the duration of its context, this method will temporarily append a
    custom Boto file to the BOTO_PATH env var without overwriting bbagent's
    BOTO_CONFIG. See https://cloud.google.com/storage/docs/boto-gsutil for
    possible configurations.

    Args:
      kwargs: Every keyword arg is treated as config line in the temp Boto file.
    """
    if self.m.platform.is_mac:
      # Due to https://bugs.python.org/issue33725, using gsutil to download
      # sufficiently large files on MacOS has been seen to hang indefinitely,
      # and disabling multi-processing avoids that hang.
      kwargs.setdefault('parallel_process_count', '1')
    if not kwargs:
      yield
      return

    # If neither BOTO_CONFIG nor BOTO_PATH are set, gsutil looks at default
    # locations (/etc/boto.cfg and ~/.boto). So give up in that case just to
    # avoid the hassle of incorporating all the defaults. ~All LUCI builds
    # should at least be setting BOTO_CONFIG.
    if not self._boto_config_path and not self._boto_path:
      yield
      return
    custom_boto_path = self.m.path.mkstemp(prefix='custom_boto_')
    contents = [
        '# Generated by $depot_tools.recipe_modules.gsutil',
        # https://cloud.google.com/storage/docs/boto-gsutil seems to indicate
        # that the section headers are important. So certain config lines may
        # not work unless they show up under the appropriate header.
        '[GSUtil]',
    ]
    for k, v in kwargs.items():
      contents.append('%s = %s' % (k, str(v)))
    self.m.file.write_text(
        'write temp Boto file', custom_boto_path, '\n'.join(contents))
    # BOTO_CONFIG can only point to one file; BOTO_PATH can point to multiple,
    # each joined by ':'. If BOTO_CONFIG is set, BOTO_PATH is ignored.
    if self._boto_config_path:
      custom_boto_path = (
          self._boto_config_path + ':' + self.m.path.abspath(custom_boto_path))
    elif self._boto_path:
      custom_boto_path = (
          self._boto_path + ':' + self.m.path.abspath(custom_boto_path))
    with self.m.context(
        env={'BOTO_PATH': custom_boto_path, 'BOTO_CONFIG': None}):
      yield

  def _generate_metadata_args(self, metadata):
    result = []
    if metadata:
      for k, v in sorted(metadata.items(), key=lambda k: k[0]):
        field = self._get_metadata_field(k)
        param = (field) if v is None else ('%s:%s' % (field, v))
        result += ['-h', param]
    return result

  def _normalize_url(self, url):
    gs_prefix = 'gs://'
    # Defines the regex that matches a normalized URL.
    for prefix in (
        gs_prefix,
        'https://storage.cloud.google.com/',
        'https://storage.googleapis.com/',
        ):
      if url.startswith(prefix):
        return gs_prefix + url[len(prefix):]
    raise AssertionError("%s cannot be normalized" % url)

  @classmethod
  def _http_url(cls, bucket, dest, is_directory=False, is_anonymous=False):
    if is_directory:
      # Use GCP console.
      url_template = 'https://console.cloud.google.com/storage/browser/%s/%s'
    elif is_anonymous:
      # Use unauthenticated object viewer.
      url_template = 'https://storage.googleapis.com/%s/%s'
    else:
      # Use authenticated object viewer.
      url_template = 'https://storage.cloud.google.com/%s/%s'
    return url_template % (bucket, dest)

  @staticmethod
  def _get_metadata_field(name, provider_prefix=None):
    """Returns: (str) the metadata field to use with Google Storage

    The Google Storage specification for metadata can be found at:
    https://developers.google.com/storage/docs/gsutil/addlhelp/WorkingWithObjectMetadata
    """
    # Already contains custom provider prefix
    if name.lower().startswith('x-'):
      return name

    # See if it's innately supported by Google Storage
    if name in (
        'Cache-Control',
        'Content-Disposition',
        'Content-Encoding',
        'Content-Language',
        'Content-MD5',
        'Content-Type',
    ):
      return name

    # Add provider prefix
    if not provider_prefix:
      provider_prefix = 'x-goog-meta'
    return '%s-%s' % (provider_prefix, name)
