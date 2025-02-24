# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import os

from telemetry.core import exceptions
from telemetry.core import util
from telemetry.internal.browser import user_agent


def GetFromBrowserOptions(browser_options):
  """Get a list of startup args from the given browser_options."""
  assert not '--no-proxy-server' in browser_options.extra_browser_args, (
      '--no-proxy-server flag is disallowed as Chrome needs to be route to '
      'ts_proxy_server')

  # Override privacy sandbox dialog feature to hide it (crbug.com/330241089).
  browser_options.AppendExtraBrowserArgs(
      ['--enable-features=PrivacySandboxSettings4'])
  browser_options.ConsolidateValuesForArg('--enable-features')

  # Sort to ensure determinism.
  args = list(sorted(browser_options.extra_browser_args))
  if browser_options.environment:
    args = browser_options.environment.AdjustStartupFlags(args)

  args.append('--enable-net-benchmarking')
  args.append('--metrics-recording-only')
  args.append('--no-default-browser-check')
  args.append('--no-first-run')
  args.append('--ignore-background-tasks')

  # Turn on GPU benchmarking extension for all runs. The only side effect of
  # the extension being on is that render stats are tracked. This is believed
  # to be effectively free. And, by doing so here, it avoids us having to
  # programmatically inspect a pageset's actions in order to determine if it
  # might eventually scroll.
  args.append('--enable-gpu-benchmarking')

  # Suppress all permission prompts by atomatically denying them.
  args.append('--deny-permission-prompts')

  # Override the need for a user gesture in order to play media.
  args.append('--autoplay-policy=no-user-gesture-required')

  if browser_options.disable_background_networking:
    args.append('--disable-background-networking')

  args.extend(user_agent.GetChromeUserAgentArgumentFromType(
      browser_options.browser_user_agent_type))

  if browser_options.disable_component_extensions_with_background_pages:
    args.append('--disable-component-extensions-with-background-pages')

  # Disables the start page, as well as other external apps that can
  # steal focus or make measurements inconsistent.
  if browser_options.disable_default_apps:
    args.append('--disable-default-apps')

  # Disable the search geolocation disclosure infobar, as it is only shown a
  # small number of times to users and should not be part of perf comparisons.
  args.append('--disable-search-geolocation-disclosure')

  # Telemetry controls startup tracing via DevTools.
  args.append('--trace-startup-owner=devtools')

  if (browser_options.logging_verbosity ==
      browser_options.NON_VERBOSE_LOGGING):
    args.extend(['--enable-logging', '--v=0'])
  elif (browser_options.logging_verbosity ==
        browser_options.VERBOSE_LOGGING):
    args.extend(['--enable-logging', '--v=1'])
  elif (browser_options.logging_verbosity ==
        browser_options.SUPER_VERBOSE_LOGGING):
    args.extend(['--enable-logging', '--v=2'])

  extensions = [e.local_path for e in browser_options.extensions_to_load]
  if extensions:
    args.append('--load-extension=%s' % ','.join(extensions))

  return args


def GetReplayArgs(network_backend, supports_spki_list=True):
  args = []
  if not network_backend.is_open:
    return args

  # Send all browser traffic (including requests to 127.0.0.1 and localhost) to
  # ts_proxy_server.
  # The proxy should NOT be set to "localhost", otherwise Chrome will first
  # attempt to use the IPv6 version (::1) before falling back to IPv4. This
  # causes issues if the IPv4 port we got randomly assigned on the device is
  # also being used in IPv6 by some other process. See
  # https://crbug.com/1005971 for more information.
  proxy_port = network_backend.forwarder.remote_port
  args.append('--proxy-server=socks://127.0.0.1:%s' % proxy_port)
  args.append('--proxy-bypass-list=<-loopback>')

  if not network_backend.use_live_traffic:
    if supports_spki_list:
      # Ignore certificate errors for certs that are signed with Wpr's root.
      # For more details on this flag, see crbug.com/753948.
      wpr_public_hash_file = os.path.join(
          util.GetCatapultDir(), 'web_page_replay_go', 'wpr_public_hash.txt')
      if not os.path.exists(wpr_public_hash_file):
        raise exceptions.PathMissingError(
            'Unable to find %s' % wpr_public_hash_file)
      with open(wpr_public_hash_file) as f:
        wpr_public_hash = f.readline().strip()
      args.append('--ignore-certificate-errors-spki-list=' + wpr_public_hash)
    else:
      # If --ignore-certificate-errors-spki-list is not supported ignore all
      # certificate errors.
      args.append('--ignore-certificate-errors')

  return args
