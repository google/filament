# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from recipe_engine.config import config_item_context, ConfigGroup, BadConf
from recipe_engine.config import ConfigList, Dict, Single, Static, Set, List

from . import api as gclient_api


def BaseConfig(USE_MIRROR=True,
               CACHE_DIR=None,
               deps_file='.DEPS.git',
               **_kwargs):
  cache_dir = str(CACHE_DIR) if CACHE_DIR else None
  return ConfigGroup(
      solutions=ConfigList(lambda: ConfigGroup(
          name=Single(str),
          url=Single((str, type(None)), empty_val=''),
          deps_file=Single(
              str, empty_val=deps_file, required=False, hidden=False),
          managed=Single(bool, empty_val=True, required=False, hidden=False),
          custom_deps=Dict(value_type=(str, type(None))),
          custom_vars=Dict(value_type=(str, bool)),
          safesync_url=Single(str, required=False),
          revision=Single(
              (str, gclient_api.RevisionResolver), required=False, hidden=True),
      )),
      deps_os=Dict(value_type=str),
      hooks=List(str),
      target_os=Set(str),
      target_os_only=Single(bool, empty_val=False, required=False),
      target_cpu=Set(str),
      target_cpu_only=Single(bool, empty_val=False, required=False),
      cache_dir=Static(cache_dir, hidden=False),

      # Maps 'solution' -> build_property
      # TODO(machenbach): Deprecate this in favor of the one below.
      # http://crbug.com/713356
      got_revision_mapping=Dict(hidden=True),

      # Maps build_property -> 'solution'
      got_revision_reverse_mapping=Dict(hidden=True),

      # Addition revisions we want to pass in.  For now there's a duplication
      # of code here of setting custom vars AND passing in --revision. We hope
      # to remove custom vars later.
      revisions=Dict(value_type=(str, gclient_api.RevisionResolver),
                     hidden=True),

      # TODO(iannucci): HACK! The use of None here to indicate that we apply
      #   this to the solution.revision field is really terrible. I mostly blame
      #   gclient.
      # Maps 'parent_build_property' -> 'custom_var_name'
      # Maps 'parent_build_property' -> None
      # If value is None, the property value will be applied to
      # solutions[0].revision. Otherwise, it will be applied to
      # solutions[0].custom_vars['custom_var_name']
      parent_got_revision_mapping=Dict(hidden=True),
      delete_unversioned_trees=Single(bool, empty_val=True, required=False),

      # Maps canonical repo URL to (local_path, revision).
      #  - canonical gitiles repo URL is "https://<host>/<project>"
      #    where project does not have "/a/" prefix or ".git" suffix.
      #  - solution/path is then used to apply patches as patch root in
      #    bot_update.
      #  - if revision is given, it's passed verbatim to bot_update for
      #    corresponding dependency. Otherwise (i.e. None), the patch will be
      #    applied on top of version pinned in DEPS.
      # This is essentially a allowlist of which repos inside a solution
      # can be patched automatically by bot_update based on
      # api.buildbucket.build.input.gerrit_changes[0].project
      # For example, if bare chromium solution has this entry in repo_path_map
      #     'https://chromium.googlesource.com/angle/angle': (
      #       'src/third_party/angle', 'HEAD')
      # then a patch to Angle project can be applied to a chromium src's
      # checkout after first updating Angle's repo to its main's HEAD.
      repo_path_map=Dict(value_type=tuple, hidden=True),

      # Check out refs/branch-heads.
      # TODO (machenbach): Only implemented for bot_update atm.
      with_branch_heads=Single(bool,
                               empty_val=False,
                               required=False,
                               hidden=True),

      # Check out refs/tags.
      with_tags=Single(bool, empty_val=False, required=False, hidden=True),
      USE_MIRROR=Static(bool(USE_MIRROR)),
  )

config_ctx = config_item_context(BaseConfig)

def ChromiumGitURL(_c, *pieces):
  return '/'.join(('https://chromium.googlesource.com',) + pieces)

# TODO(phajdan.jr): Move to proper repo and add coverage.
def ChromeInternalGitURL(_c, *pieces):  # pragma: no cover
  return '/'.join(('https://chrome-internal.googlesource.com',) + pieces)

@config_ctx()
def android(c):
  c.target_os.add('android')

@config_ctx()
def nacl(c):
  s = c.solutions.add()
  s.name = 'native_client'
  s.url = ChromiumGitURL(c, 'native_client', 'src', 'native_client.git')
  m = c.got_revision_mapping
  m['native_client'] = 'got_revision'

@config_ctx()
def webports(c):
  s = c.solutions.add()
  s.name = 'src'
  s.url = ChromiumGitURL(c, 'webports.git')
  m = c.got_revision_mapping
  m['src'] = 'got_revision'

@config_ctx()
def emscripten_releases(c):
  s = c.solutions.add()
  s.name = 'emscripten-releases'
  s.url = ChromiumGitURL(c, 'emscripten-releases.git')
  m = c.got_revision_mapping
  m['emscripten-releases'] = 'got_revision'

@config_ctx()
def gyp(c):
  s = c.solutions.add()
  s.name = 'gyp'
  s.url = ChromiumGitURL(c, 'external', 'gyp.git')
  m = c.got_revision_mapping
  m['gyp'] = 'got_revision'

@config_ctx()
def build(c):
  s = c.solutions.add()
  s.name = 'build'
  s.url = ChromiumGitURL(c, 'chromium', 'tools', 'build.git')
  m = c.got_revision_mapping
  m['build'] = 'got_revision'

@config_ctx()
def depot_tools(c):  # pragma: no cover
  s = c.solutions.add()
  s.name = 'depot_tools'
  s.url = ChromiumGitURL(c, 'chromium', 'tools', 'depot_tools.git')
  m = c.got_revision_mapping
  m['depot_tools'] = 'got_revision'

@config_ctx()
def skia(c):  # pragma: no cover
  s = c.solutions.add()
  s.name = 'skia'
  s.url = 'https://skia.googlesource.com/skia.git'
  m = c.got_revision_mapping
  m['skia'] = 'got_revision'

@config_ctx()
def skia_buildbot(c):  # pragma: no cover
  s = c.solutions.add()
  s.name = 'skia_buildbot'
  s.url = 'https://skia.googlesource.com/buildbot.git'
  m = c.got_revision_mapping
  m['skia_buildbot'] = 'got_revision'

@config_ctx()
def chrome_golo(c):  # pragma: no cover
  s = c.solutions.add()
  s.name = 'chrome_golo'
  s.url = 'https://chrome-internal.googlesource.com/chrome-golo/chrome-golo.git'
  c.got_revision_mapping['chrome_golo'] = 'got_revision'

@config_ctx()
def infra_puppet(c):  # pragma: no cover
  s = c.solutions.add()
  s.name = 'infra_puppet'
  s.url = 'https://chrome-internal.googlesource.com/infra/puppet.git'
  c.got_revision_mapping['infra_puppet'] = 'got_revision'

@config_ctx()
def build_internal(c):
  s = c.solutions.add()
  s.name = 'build_internal'
  s.url = 'https://chrome-internal.googlesource.com/chrome/tools/build.git'
  c.got_revision_mapping['build_internal'] = 'got_revision'
  # We do not use 'includes' here, because we want build_internal to be the
  # first solution in the list as run_presubmit computes upstream revision
  # from the first solution.
  build(c)
  c.got_revision_mapping['build'] = 'got_build_revision'

@config_ctx()
def pdfium(c):
  soln = c.solutions.add()
  soln.name = 'pdfium'
  soln.url = 'https://pdfium.googlesource.com/pdfium.git'
  m = c.got_revision_mapping
  m['pdfium'] = 'got_revision'

@config_ctx()
def crashpad(c):
  soln = c.solutions.add()
  soln.name = 'crashpad'
  soln.url = 'https://chromium.googlesource.com/crashpad/crashpad.git'

@config_ctx()
def boringssl(c):
  soln = c.solutions.add()
  soln.name = 'boringssl'
  soln.url = 'https://boringssl.googlesource.com/boringssl.git'
  soln.deps_file = 'util/bot/DEPS'

@config_ctx()
def dart(c):
  soln = c.solutions.add()
  soln.name = 'sdk'
  soln.url = ('https://dart.googlesource.com/sdk.git')
  soln.deps_file = 'DEPS'
  soln.managed = False

@config_ctx()
def expect_tests(c):
  soln = c.solutions.add()
  soln.name = 'expect_tests'
  soln.url = 'https://chromium.googlesource.com/infra/testing/expect_tests.git'
  c.got_revision_mapping['expect_tests'] = 'got_revision'


# TODO(crbug.com/1415507): Delete the old infra configs and rename
# the _superproject configs after migration deadline.
@config_ctx()
def infra_superproject(c):
  soln = c.solutions.add()
  soln.name = '.'
  soln.url = 'https://chromium.googlesource.com/infra/infra_superproject.git'
  c.got_revision_mapping['infra'] = 'got_revision'
  c.got_revision_mapping['.'] = 'got_revision_superproject'
  c.repo_path_map.update({
      'https://chromium.googlesource.com/infra/luci/gae':
      ('infra/go/src/go.chromium.org/gae', 'HEAD'),
      'https://chromium.googlesource.com/infra/luci/luci-py':
      ('infra/luci', 'HEAD'),
      'https://chromium.googlesource.com/infra/luci/luci-go':
      ('infra/go/src/go.chromium.org/luci', 'HEAD'),
      'https://chromium.googlesource.com/infra/luci/recipes-py':
      ('infra/recipes-py', 'HEAD'),
      'https://chromium.googlesource.com/infra/infra': ('infra', None)
  })


@config_ctx()
def infra_internal_superproject(c):
  soln = c.solutions.add()
  soln.name = '.'
  soln.custom_vars = {'checkout_internal': True}
  soln.url = 'https://chromium.googlesource.com/infra/infra_superproject.git'
  c.got_revision_mapping['infra_internal'] = 'got_revision'
  c.got_revision_mapping['.'] = 'got_revision_superproject'
  c.repo_path_map.update({
      'https://chrome-internal.googlesource.com/infra/infra_internal':
      ('infra_internal', None)
  })


@config_ctx()
def infra(c):
  soln = c.solutions.add()
  soln.name = 'infra'
  soln.url = 'https://chromium.googlesource.com/infra/infra.git'
  c.got_revision_mapping['infra'] = 'got_revision'
  c.repo_path_map.update({
      'https://chromium.googlesource.com/infra/luci/gae': (
          'infra/go/src/go.chromium.org/gae', 'HEAD'),
      'https://chromium.googlesource.com/infra/luci/luci-py': (
          'infra/luci', 'HEAD'),
      'https://chromium.googlesource.com/infra/luci/luci-go': (
          'infra/go/src/go.chromium.org/luci', 'HEAD'),
      'https://chromium.googlesource.com/infra/luci/recipes-py': (
          'infra/recipes-py', 'HEAD')
  })

@config_ctx()
def infra_internal(c):  # pragma: no cover
  soln = c.solutions.add()
  soln.name = 'infra_internal'
  soln.url = 'https://chrome-internal.googlesource.com/infra/infra_internal.git'
  c.got_revision_mapping['infra_internal'] = 'got_revision'

@config_ctx(includes=['infra'])
def luci_gae(c):
  # luci/gae is checked out as a part of infra.git solution at HEAD.
  c.revisions['infra'] = 'refs/heads/main'
  # luci/gae is developed together with luci-go, which should be at HEAD.
  c.revisions['infra/go/src/go.chromium.org/luci'] = 'refs/heads/main'
  c.revisions['infra/go/src/go.chromium.org/gae'] = (
      gclient_api.RevisionFallbackChain('refs/heads/main'))
  m = c.got_revision_mapping
  del m['infra']
  m['infra/go/src/go.chromium.org/gae'] = 'got_revision'

@config_ctx(includes=['infra'])
def luci_go(c):
  # luci-go is checked out as a part of infra.git solution at HEAD.
  c.revisions['infra'] = 'refs/heads/main'
  c.revisions['infra/go/src/go.chromium.org/luci'] = (
      gclient_api.RevisionFallbackChain('refs/heads/main'))
  m = c.got_revision_mapping
  del m['infra']
  m['infra/go/src/go.chromium.org/luci'] = 'got_revision'

@config_ctx(includes=['infra'])
def luci_py(c):
  # luci-py is checked out as part of infra just to have appengine
  # pre-installed, as that's what luci-py PRESUBMIT relies on.
  c.revisions['infra'] = 'refs/heads/main'
  c.revisions['infra/luci'] = (
      gclient_api.RevisionFallbackChain('refs/heads/main'))
  m = c.got_revision_mapping
  del m['infra']
  m['infra/luci'] = 'got_revision'

@config_ctx(includes=['infra'])
def recipes_py(c):
  c.revisions['infra'] = 'refs/heads/main'
  c.revisions['infra/recipes-py'] = (
      gclient_api.RevisionFallbackChain('refs/heads/main'))
  m = c.got_revision_mapping
  del m['infra']
  m['infra/recipes-py'] = 'got_revision'

@config_ctx()
def recipes_py_bare(c):
  soln = c.solutions.add()
  soln.name = 'recipes-py'
  soln.url = 'https://chromium.googlesource.com/infra/luci/recipes-py'
  c.got_revision_mapping['recipes-py'] = 'got_revision'

@config_ctx()
def catapult(c):
  soln = c.solutions.add()
  soln.name = 'catapult'
  soln.url = 'https://chromium.googlesource.com/catapult'
  c.got_revision_mapping['catapult'] = 'got_revision'

@config_ctx(includes=['infra_internal'])
def infradata_master_manager(c):
  soln = c.solutions.add()
  soln.name = 'infra-data-master-manager'
  soln.url = (
      'https://chrome-internal.googlesource.com/infradata/master-manager.git')
  del c.got_revision_mapping['infra_internal']
  c.got_revision_mapping['infra-data-master-manager'] = 'got_revision'


@config_ctx()
def infradata_cloud_run(c):
  soln = c.solutions.add()
  soln.name = 'infra-data-cloud-run'
  soln.url = 'https://chrome-internal.googlesource.com/infradata/cloud-run.git'
  c.got_revision_mapping['infra-data-cloud-run'] = 'got_revision'


@config_ctx()
def infradata_config(c):
  soln = c.solutions.add()
  soln.name = 'infra-data-config'
  soln.url = 'https://chrome-internal.googlesource.com/infradata/config.git'
  c.got_revision_mapping['infra-data-config'] = 'got_revision'

@config_ctx()
def infradata_rbe(c):
  soln = c.solutions.add()
  soln.name = 'infradata-rbe'
  soln.url = 'https://chrome-internal.googlesource.com/infradata/rbe.git'
  c.got_revision_mapping['infradata-rbe'] = 'got_revision'

@config_ctx()
def with_branch_heads(c):
  c.with_branch_heads = True

@config_ctx()
def with_tags(c):
  c.with_tags = True

@config_ctx()
def custom_tabs_client(c):
  soln = c.solutions.add()
  soln.name = 'custom_tabs_client'
  # TODO(pasko): test custom-tabs-client within a full chromium checkout.
  soln.url = 'https://chromium.googlesource.com/custom-tabs-client'
  c.got_revision_mapping['custom_tabs_client'] = 'got_revision'

@config_ctx()
def gerrit_test_cq_normal(c):
  soln = c.solutions.add()
  soln.name = 'gerrit-test-cq-normal'
  soln.url = 'https://chromium.googlesource.com/playground/gerrit-cq/normal.git'

@config_ctx()
def dawn(c):
  soln = c.solutions.add()
  soln.name = 'dawn'
  soln.url = 'https://dawn.googlesource.com/dawn.git'
  c.got_revision_mapping['dawn'] = 'got_revision'

@config_ctx()
def celab(c):
  soln = c.solutions.add()
  # soln.name must match the repo name for `dep` to work properly
  soln.name = 'cel'
  soln.url = 'https://chromium.googlesource.com/enterprise/cel.git'
  c.got_revision_mapping['cel'] = 'got_revision'

@config_ctx()
def openscreen(c):
  s = c.solutions.add()
  s.name = 'openscreen'
  s.url = 'https://chromium.googlesource.com/openscreen'
  c.got_revision_mapping['openscreen'] = 'got_revision'

@config_ctx()
def devtools(c):
  s = c.solutions.add()
  s.name = 'devtools'
  s.url = 'https://chromium.googlesource.com/devtools/devtools-frontend.git'
  c.got_revision_mapping['devtools'] = 'got_revision'
  c.repo_path_map.update({
      'https://chromium.googlesource.com/devtools/devtools-frontend': (
          'devtools/devtools-frontend', 'HEAD'),
  })

@config_ctx()
def tint(c):
  soln = c.solutions.add()
  soln.name = 'tint'
  soln.url = 'https://dawn.googlesource.com/tint.git'
  c.got_revision_mapping['tint'] = 'got_revision'

@config_ctx()
def gerrit(c):
  s = c.solutions.add()
  s.name = 'gerrit'
  s.url = 'https://gerrit.googlesource.com/gerrit.git'

@config_ctx(includes=['gerrit'])
def gerrit_plugins_binary_size(c):
  s = c.solutions.add()
  s.name = 'gerrit_plugins_binary_size'
  s.url = ChromiumGitURL(c, 'infra', 'gerrit-plugins', 'binary-size.git')
  c.got_revision_mapping['gerrit_plugins_binary_size'] = 'got_revision'

@config_ctx(includes=['gerrit'])
def gerrit_plugins_buildbucket(c):
  s = c.solutions.add()
  s.name = 'gerrit_plugins_buildbucket'
  s.url = ChromiumGitURL(c, 'infra', 'gerrit-plugins', 'buildbucket.git')
  c.got_revision_mapping['gerrit_plugins_buildbucket'] = 'got_revision'

@config_ctx(includes=['gerrit'])
def gerrit_plugins_chromium_behavior(c):
  s = c.solutions.add()
  s.name = 'gerrit_plugins_chromium_behavior'
  s.url = ChromiumGitURL(c, 'infra', 'gerrit-plugins', 'chromium-behavior.git')
  c.got_revision_mapping['gerrit_plugins_chromium_behavior'] = 'got_revision'

@config_ctx(includes=['gerrit'])
def gerrit_plugins_chromium_binary_size(c):
  s = c.solutions.add()
  s.name = 'gerrit_plugins_chromium_binary_size'
  s.url = ChromiumGitURL(c, 'infra', 'gerrit-plugins',
                         'chromium-binary-size.git')
  c.got_revision_mapping['gerrit_plugins_chromium_binary_size'] = 'got_revision'

@config_ctx(includes=['gerrit'])
def gerrit_plugins_chumpdetector(c):
  s = c.solutions.add()
  s.name = 'gerrit_plugins_chumpdetector'
  s.url = ChromiumGitURL(c, 'infra', 'gerrit-plugins', 'chumpdetector.git')
  c.got_revision_mapping['gerrit_plugins_chumpdetector'] = 'got_revision'

@config_ctx(includes=['gerrit'])
def gerrit_plugins_code_coverage(c):
  s = c.solutions.add()
  s.name = 'gerrit_plugins_code_coverage'
  s.url = ChromiumGitURL(c, 'infra', 'gerrit-plugins', 'code-coverage.git')
  c.got_revision_mapping['gerrit_plugins_code_coverage'] = 'got_revision'

@config_ctx(includes=['gerrit'])
def gerrit_plugins_git_numberer(c):
  s = c.solutions.add()
  s.name = 'gerrit_plugins_git_numberer'
  s.url = ChromiumGitURL(c, 'infra', 'gerrit-plugins', 'git-numberer.git')
  c.got_revision_mapping['gerrit_plugins_git_numberer'] = 'got_revision'

@config_ctx(includes=['gerrit'])
def gerrit_plugins_landingwidget(c):
  s = c.solutions.add()
  s.name = 'gerrit_plugins_landingwidget'
  s.url = ChromiumGitURL(c, 'infra', 'gerrit-plugins', 'landingwidget.git')
  c.got_revision_mapping['gerrit_plugins_landingwidget'] = 'got_revision'

@config_ctx(includes=['gerrit'])
def gerrit_plugins_tricium(c):
  s = c.solutions.add()
  s.name = 'gerrit_plugins_tricium'
  s.url = ChromiumGitURL(c, 'infra', 'gerrit-plugins', 'tricium.git')
  c.got_revision_mapping['gerrit_plugins_tricium'] = 'got_revision'

@config_ctx()
def crossbench(c):
  soln = c.solutions.add()
  soln.name = 'crossbench'
  soln.url = 'https://chromium.googlesource.com/crossbench'
  c.got_revision_mapping['crossbench'] = 'got_revision'


@config_ctx()
def website(c):  # pragma: no cover
  s = c.solutions.add()
  s.name = 'website'
  s.url = ChromiumGitURL(c, 'website.git')
  m = c.got_revision_mapping
  m['src'] = 'got_revision'


@config_ctx()
def ytdevinfra(c):
  soln = c.solutions.add()
  soln.name = 'ytdevinfra'
  soln.url = 'https://lbshell-internal.googlesource.com/cobalt_src.git'
  c.got_revision_mapping['ytdevinfra'] = 'got_revision'
