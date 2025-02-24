# Copyright (c) 2013 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""
Utilities for requesting information for a Gerrit server via HTTPS.

https://gerrit-review.googlesource.com/Documentation/rest-api.html
"""

from __future__ import annotations

import base64
import contextlib
import functools
import http.cookiejar
import json
import logging
import os
import random
import re
import shutil
import socket
import subprocess
import sys
import tempfile
import threading
import time
import urllib.parse

from dataclasses import dataclass
from io import StringIO
from multiprocessing.pool import ThreadPool
from typing import Any, Container, Dict, List, Optional
from typing import Tuple, TypedDict, cast

import httplib2
import httplib2.socks

import auth
import gclient_utils
import metrics
import metrics_utils
import newauth
import scm
import subprocess2


# HACK: httplib2 has significant bugs with its proxy support in
# python3. All httplib2 code should be rewritten to just use python
# stdlib which does not have these bugs.
#
# Prior to that, however, we will directly patch the buggy
# implementation of httplib2.socks.socksocket.__rewriteproxy which does
# not properly expect bytes as its argument instead of str.
#
# Note that __rewriteproxy is inherently buggy, as it relies on the
# python stdlib client to send the entire request header in a single
# call to socket.sendall, which is not explicitly guaranteed.
#
# Changes:
#   * all string literals changed to bytes literals.
#   * added more http methods to recognize.
#   * all __symbols changed to _socksocket__symbols (Python __ munging).
#   * Type annotations added to function signature.
def __fixed_rewrite_proxy(self: httplib2.socks.socksocket, header: bytes):
    """ rewrite HTTP request headers to support non-tunneling proxies
    (i.e. those which do not support the CONNECT method).
    This only works for HTTP (not HTTPS) since HTTPS requires tunneling.
    """
    host, endpt = None, None
    hdrs = header.split(b"\r\n")
    for hdr in hdrs:
        if hdr.lower().startswith(b"host:"):
            host = hdr
        elif hdr.lower().split(b" ")[0] in (b"get", b"head", b"post", b"put",
                                            b"patch"):
            endpt = hdr
    if host and endpt:
        hdrs.remove(host)
        hdrs.remove(endpt)
        host = host.split(b" ")[1]
        endpt = endpt.split(b" ")
        if self._socksocket__proxy[4] != None \
           and self._socksocket__proxy[5] != None:
            hdrs.insert(0, self._socksocket__getauthheader())
        hdrs.insert(0, b"Host: %s" % host)
        hdrs.insert(0,
                    b"%s http://%s%s %s" % (endpt[0], host, endpt[1], endpt[2]))
    return b"\r\n".join(hdrs)


httplib2.socks.socksocket._socksocket__rewriteproxy = __fixed_rewrite_proxy

# TODO: Should fix these warnings.
# pylint: disable=line-too-long

LOGGER = logging.getLogger()
# With a starting sleep time of 12.0 seconds, x <= [1.8-2.2]x backoff, and six
# total tries, the sleep time between the first and last tries will be ~6 min
# (excluding time for each try).
TRY_LIMIT = 6
SLEEP_TIME = 12.0
MAX_BACKOFF = 2.2
MIN_BACKOFF = 1.8

# Controls the transport protocol used to communicate with Gerrit.
# This is parameterized primarily to enable GerritTestCase.
GERRIT_PROTOCOL = 'https'

# Controls how many concurrent Gerrit connections there can be.
MAX_CONCURRENT_CONNECTION = 20


def time_sleep(seconds):
    # Use this so that it can be mocked in tests without interfering with python
    # system machinery.
    return time.sleep(seconds)


def time_time():
    # Use this so that it can be mocked in tests without interfering with python
    # system machinery.
    return time.time()


def log_retry_and_sleep(seconds, attempt, try_limit):
    LOGGER.info('Will retry in %d seconds (%d more times)...', seconds,
                try_limit - attempt - 1)
    time_sleep(seconds)
    return seconds * random.uniform(MIN_BACKOFF, MAX_BACKOFF)


class GerritError(Exception):
    """Exception class for errors commuicating with the gerrit-on-borg service."""
    def __init__(self, http_status, message, *args, **kwargs):
        super(GerritError, self).__init__(*args, **kwargs)
        self.http_status = http_status
        self.message = '(%d) %s' % (self.http_status, message)

    def __str__(self):
        return self.message


def _QueryString(params, first_param=None):
    """Encodes query parameters in the key:val[+key:val...] format specified here:

    https://gerrit-review.googlesource.com/Documentation/rest-api-changes.html#list-changes
    """
    q = [urllib.parse.quote(first_param)] if first_param else []
    q.extend(['%s:%s' % (key, val.replace(" ", "+")) for key, val in params])
    return '+'.join(q)


class SSOHelper(object):
    """SSOHelper finds a Google-internal SSO helper."""

    _sso_cmd: Optional[str] = None

    def find_cmd(self) -> str:
        """Returns the cached command-line to invoke git-remote-sso.

        If git-remote-sso is not in $PATH, returns None.
        """
        if self._sso_cmd is not None:
            return self._sso_cmd
        cmd = shutil.which('git-remote-sso')
        if cmd is None:
            cmd = ''
        self._sso_cmd = cmd
        return cmd


# Global instance
ssoHelper = SSOHelper()


@functools.lru_cache(maxsize=None)
def ShouldUseSSO(host: str, email: str) -> bool:
    """Return True if we should use SSO for the given Gerrit host and user."""
    LOGGER.debug("Determining whether we should use SSO...")
    if not newauth.Enabled():
        LOGGER.debug("SSO=False: not opted in")
        return False
    if not host.endswith('.googlesource.com'):
        LOGGER.debug("SSO=False: non-googlesource host %r", host)
        return False
    if newauth.SkipSSO():
        LOGGER.debug("SSO=False: set skip SSO config")
        return False
    if not ssoHelper.find_cmd():
        LOGGER.debug("SSO=False: no SSO command")
        return False
    if not email:
        LOGGER.debug(
            "SSO=True: email is empty or missing (and SSO command available)")
        return True
    if email.endswith('@google.com'):
        LOGGER.debug("SSO=True: email is google.com")
        return True
    if not email.endswith('@chromium.org'):
        LOGGER.debug("SSO=False: not chromium.org")
        return False
    authenticator = SSOAuthenticator()
    try:
        records = GetAccountEmails(host, 'self', authenticator=authenticator)
    except GerritError as e:
        if e.http_status == 400:
            # This is likely because the user doesn't have an account on the Gerrit host.
            LOGGER.debug("SSO=False: get account emails returned 400")
            return False
        raise
    if any(email == r['email'] for r in records):
        LOGGER.debug("SSO=True: email is linked to google.com")
        return True
    LOGGER.debug("SSO=False: unlinked chromium.org")
    return False


class _Authenticator(object):
    """Base authenticator class for authenticator implementations to subclass."""

    # Cached _Authenticator subclass instance, resolved via get().
    _resolved: Optional[_Authenticator] = None
    _resolved_lock = threading.Lock()

    def authenticate(self, conn: HttpConn):
        """Adds authentication information to the HttpConn."""
        raise NotImplementedError()

    def debug_summary_state(self) -> str:
        """If this _Authenticator has any debugging information about its state,
        _WriteGitPushTraces will call this to include in the git push traces.

        Return value is any relevant debugging information with all PII/secrets
        redacted.
        """
        raise NotImplementedError()

    @classmethod
    def is_applicable(cls, *, conn: Optional[HttpConn] = None) -> bool:
        """Must return True if this Authenticator is available in the current
        environment.

        If conn is not None, return True if this Authenticator is
        applicable to the given conn in the current environment.
        """
        raise NotImplementedError()

    def ensure_authenticated(self, gerrit_host: str, git_host: str) -> Tuple[bool, str]:
        """Returns (bypassable, error message).

        If the error message is empty, there is no error to report.
        If bypassable is true, the caller will allow the user to continue past the
        error.
        """
        return (True, '')

    @classmethod
    def get(cls):
        """Returns: (_Authenticator) The identified _Authenticator to use.

        Probes the local system and its environment and identifies the
        _Authenticator instance to use.

        The resolved _Authenticator instance is cached as a class variable.
        """
        with cls._resolved_lock:
            if ret := cls._resolved:
                return ret

            use_new_auth = newauth.Enabled()

            # Allow skipping SSOAuthenticator for local testing purposes.
            skip_sso = newauth.SkipSSO()

            if use_new_auth:
                LOGGER.debug('_Authenticator.get: using new auth stack')
                if LuciContextAuthenticator.is_applicable():
                    LOGGER.debug(
                        '_Authenticator.get: using LUCI context authenticator')
                    ret = LuciContextAuthenticator()
                else:
                    LOGGER.debug(
                        '_Authenticator.get: using chained authenticator')
                    a = [
                        SSOAuthenticator(),
                        # GCE detection can't distinguish cloud workstations.
                        GceAuthenticator(),
                        GitCredsAuthenticator(),
                        NoAuthenticator(),
                    ]
                    if skip_sso:
                        LOGGER.debug(
                            'Authenticator.get: skipping SSOAuthenticator.')
                        a = a[1:]
                    ret = ChainedAuthenticator(a)
                cls._resolved = ret
                return ret
            authenticators = [
                LuciContextAuthenticator,
                GceAuthenticator,
                CookiesAuthenticator,
            ]

            for candidate in authenticators:
                if candidate.is_applicable():
                    LOGGER.debug('_Authenticator.get: Selected %s.',
                                 candidate.__name__)
                    ret = candidate()
                    cls._resolved = ret
                    return ret

            auth_names = ', '.join(a.__name__ for a in authenticators)
            raise ValueError(
                f"Could not find suitable authenticator, tried: [{auth_names}]."
            )


def debug_auth() -> Tuple[str, str]:
    """Returns the name of the chosen auth scheme and any additional debugging
    information for that authentication scheme. """
    authn = _Authenticator.get()
    return authn.__class__.__name__, authn.debug_summary_state()


def ensure_authenticated(gerrit_host: str, git_host: str) -> Tuple[bool, str]:
    """Returns (bypassable, error message).

    If the error message is empty, there is no error to report. If bypassable is
    true, the caller will allow the user to continue past the error.
    """
    return _Authenticator.get().ensure_authenticated(gerrit_host, git_host)


class SSOAuthenticator(_Authenticator):
    """SSOAuthenticator implements a Google-internal authentication scheme.

    TEMPORARY configuration for Googlers (one `url` block for each Gerrit host):

        [url "sso://chromium/"]
          insteadOf = https://chromium.googlesource.com/
          insteadOf = http://chromium.googlesource.com/
        [depot-tools]
          useNewAuthStack = 1
    """

    # This is set to true in tests, allows _parse_config to consume expired
    # cookies.
    _testing_load_expired_cookies = False

    # How long we should wait for the sso helper to write and close stdout.
    # Overridden in tests.
    _timeout_secs = 5

    # The required fields for the sso config, expected from the sso helper's
    # output.
    _required_fields = frozenset(
        ['http.cookiefile', 'http.proxy', 'include.path'])

    @dataclass
    class SSOInfo:
        proxy: httplib2.ProxyInfo
        cookies: http.cookiejar.CookieJar
        headers: Dict[str, str]

    # SSOInfo is a cached blob of information used by the `authenticate` method.
    _sso_info: Optional[SSOInfo] = None
    _sso_info_lock = threading.Lock()

    @classmethod
    def _resolve_sso_cmd(cls) -> Tuple[str, ...]:
        """Returns the cached command-line to invoke git-remote-sso.

        If git-remote-sso is not in $PATH, returns ().
        """
        cmd = ssoHelper.find_cmd()
        if not cmd:
            return ()
        return (
            cmd,
            '-print_config',
            'sso://*.git.corp.google.com',
        )

    @classmethod
    def is_applicable(cls, *, conn: Optional[HttpConn] = None) -> bool:
        if not cls._resolve_sso_cmd():
            return False
        email: str = scm.GIT.GetConfig(os.getcwd(), 'user.email', default='')
        if conn is not None:
            return ShouldUseSSO(conn.host, email)
        return email.endswith('@google.com')

    @classmethod
    def _parse_config(cls, config: Dict[str, str]) -> SSOInfo:
        """Parses the sso info from the given config.

        Note: update cls._required_fields appropriately when making
        changes to this method, to ensure the field values are captured
        from the sso helper.
        """
        fullAuthHeader = cast(
            str,
            scm.GIT.Capture([
                'config',
                '-f',
                config['include.path'],
                'http.extraHeader',
            ]))
        headerKey, headerValue = fullAuthHeader.split(':', 1)
        headers = {headerKey.strip(): headerValue.strip()}

        proxy_host, proxy_port = config['http.proxy'].split(':', 1)

        cfpath = config['http.cookiefile']
        cj = http.cookiejar.MozillaCookieJar(cfpath)
        # NOTE: python3.8 doesn't support httponly cookie lines, so we parse
        # this manually. Once we move to python3.10+, this hack can be removed.
        with open(cfpath) as cf:
            cookiedata = cf.read().replace('#HttpOnly_', '')
        # _really_load is the way that MozillaCookieJar subclasses
        # FileCookieJar. Calling this directly is better than reimplementing the
        # entire _really_load function manually.
        cj._really_load(
            StringIO(cookiedata),
            cfpath,
            ignore_discard=False,
            ignore_expires=cls._testing_load_expired_cookies,
        )

        return cls.SSOInfo(proxy=httplib2.ProxyInfo(
            httplib2.socks.PROXY_TYPE_HTTP_NO_TUNNEL, proxy_host.encode(),
            int(proxy_port)),
                           cookies=cj,
                           headers=headers)

    @classmethod
    def _launch_sso_helper(cls) -> SSOInfo:
        """Launches the git-remote-sso process and extracts the parsed SSOInfo.

        Raises an exception if something goes wrong.
        """
        cmd = cls._resolve_sso_cmd()

        with tempdir() as tdir:
            tf = os.path.join(tdir, 'git-remote-sso.stderr')

            with open(tf, mode='w') as stderr_file:
                # NOTE: The git-remote-sso helper does the following:
                #
                # 1. writes files to disk.
                # 2. writes config to stdout, referencing those files.
                # 3. closes stdout (sending EOF to us, ending the stdout data).
                #    - Note: on Windows, stdout.read() times out anyway, so
                #      instead we process stdout line by line.
                # 4. waits for stdin to close.
                # 5. deletes files on disk (which is why we make sys.stdin a PIPE
                #    instead of closing it outright).
                #
                # NOTE: the http.proxy value in the emitted config points to
                # a socket which is owned by a system service, not `proc` itself.
                with subprocess2.Popen(cmd,
                                       stdout=subprocess2.PIPE,
                                       stderr=stderr_file,
                                       stdin=subprocess2.PIPE,
                                       encoding='utf-8') as proc:
                    stderr_file.close()  # we can close after process starts.
                    timedout = False

                    def _fire_timeout():
                        nonlocal timedout
                        timedout = True
                        proc.kill()

                    timer = threading.Timer(cls._timeout_secs, _fire_timeout)
                    timer.start()
                    try:
                        config = {}
                        for line in proc.stdout:
                            if not line:
                                break
                            field, value = line.strip().split('=', 1)
                            config[field] = value
                            # Stop reading if we have all the required fields.
                            if cls._required_fields.issubset(config.keys()):
                                break
                    finally:
                        timer.cancel()

                    missing_fields = cls._required_fields - config.keys()
                    if timedout:
                        # Within the time limit, at least one required field was
                        # still missing. Killing the process has been triggered,
                        # even if we now have all fields.
                        details = ''
                        if missing_fields:
                            details = ': missing fields [{names}]'.format(
                                names=', '.join(missing_fields))
                        LOGGER.error(
                            'SSOAuthenticator: Timeout: %r: reading config%s.',
                            cmd, details)
                        raise subprocess.TimeoutExpired(
                            cmd=cmd, timeout=cls._timeout_secs)
                    # if the process already ended, then something is wrong.
                    retcode = proc.poll()
                    # if stdout was closed without all required data, we need to
                    # wait for end-of-process here and hope for an error message
                    # - the poll above is racy in this case (we could see stdout
                    # EOF but the process may not have quit yet).
                    if not retcode and missing_fields:
                        retcode = proc.wait(timeout=cls._timeout_secs)
                        # We timed out while doing `wait` - we can't safely open
                        # stderr on windows, so just emit a generic timeout
                        # exception.
                        if retcode is None:
                            LOGGER.error(
                                'SSOAuthenticator: Timeout: %r: waiting error output.',
                                cmd)
                            raise subprocess.TimeoutExpired(
                                cmd=cmd, timeout=cls._timeout_secs)

                    # Finally, if the poll or wait ended up getting the retcode,
                    # it means the process failed, so we can read the stderr
                    # file and reflect it back to the user.
                    if retcode is not None:
                        # process failed - we should be able to read the tempfile.
                        with open(tf, encoding='utf-8') as stderr:
                            sys.exit(
                                f'SSOAuthenticator: exit {retcode}: {stderr.read().strip()}'
                            )

                    return cls._parse_config(config)

    @classmethod
    def _get_sso_info(cls) -> SSOInfo:
        with cls._sso_info_lock:

            info = cls._sso_info
            if not info:
                info = cls._launch_sso_helper()
                # HACK: `git-remote-sso` doesn't always properly warm up the
                # cookies - in this case, run a canned git operation against
                # a public repo to cause `git-remote-sso` to warm the cookies
                # up, and then try pulling config again.
                #
                # BUG: b/342644760
                if not any(c.domain == '.google.com' for c in info.cookies):
                    LOGGER.debug('SSO: Refreshing .google.com cookies.')
                    scm.GIT.Capture(['ls-remote', 'sso://chromium/All-Projects'])
                    info = cls._launch_sso_helper()
                    if not any(c.domain == '.google.com' for c in info.cookies):
                        raise ValueError('Unable to extract .google.com cookie.')
                cls._sso_info = info
            return info

    def authenticate(self, conn: HttpConn):
        sso_info = self._get_sso_info()
        conn.proxy_info = sso_info.proxy
        conn.req_headers.update(sso_info.headers)

        # Now we must rewrite:
        #   https://xxx.googlesource.com ->
        #   http://xxx.git.corp.google.com
        parsed = urllib.parse.urlparse(conn.req_uri)
        parsed = parsed._replace(scheme='http')
        if (hostname :=
                parsed.hostname) and hostname.endswith('.googlesource.com'):
            assert not parsed.port, "SSOAuthenticator: netloc: port not supported"
            assert not parsed.username, "SSOAuthenticator: netloc: username not supported"
            assert not parsed.password, "SSOAuthenticator: netloc: password not supported"

            hostname_parts = hostname.rsplit('.', 2)  # X, googlesource, com
            conn.req_host = hostname_parts[0] + '.git.corp.google.com'
            parsed = parsed._replace(netloc=conn.req_host)
        conn.req_uri = parsed.geturl()

        # Finally, add cookies
        sso_info.cookies.add_cookie_header(conn)
        assert 'Cookie' in conn.req_headers, (
            'sso_info.cookies.add_cookie_header failed to add Cookie.')

    def debug_summary_state(self) -> str:
        return ''


class CookiesAuthenticator(_Authenticator):
    """_Authenticator implementation that uses ".gitcookies" for token.

    Expected case for developer workstations.
    """

    _EMPTY = object()

    def __init__(self):
        # Credentials will be loaded lazily on first use. This ensures
        # _Authenticator get() can always construct an authenticator, even if
        # something is broken. This allows 'creds-check' to proceed to actually
        # checking creds later, rigorously (instead of blowing up with a cryptic
        # error if they are wrong).
        self._gitcookies = self._EMPTY

    @classmethod
    def is_applicable(cls, *, conn: Optional[HttpConn] = None) -> bool:
        # We consider CookiesAuthenticator always applicable for now.
        return True

    @property
    def gitcookies(self):
        if self._gitcookies is self._EMPTY:
            self._gitcookies = self._get_gitcookies()
        return self._gitcookies

    @classmethod
    def get_new_password_url(cls, host):
        assert not host.startswith('http')
        # Assume *.googlesource.com pattern.
        parts = host.split('.')

        # remove -review suffix if present.
        if parts[0].endswith('-review'):
            parts[0] = parts[0][:-len('-review')]

        return 'https://%s/new-password' % ('.'.join(parts))

    @classmethod
    def _get_new_password_message(cls, host):
        if host is None:
            return ('Git host for Gerrit upload is unknown. Check your remote '
                    'and the branch your branch is tracking. This tool assumes '
                    'that you are using a git server at *.googlesource.com.')
        url = cls.get_new_password_url(host)
        return 'You can (re)generate your credentials by visiting %s' % url

    @classmethod
    def get_gitcookies_path(cls) -> str:
        if envVal := os.getenv('GIT_COOKIES_PATH'):
            return envVal

        return os.path.expanduser(
            scm.GIT.GetConfig(os.getcwd(), 'http.cookiefile',
                              os.path.join('~', '.gitcookies')))

    @classmethod
    def _get_gitcookies(cls):
        gitcookies = {}
        path = cls.get_gitcookies_path()
        if not os.path.exists(path):
            return gitcookies

        try:
            f = gclient_utils.FileRead(path, 'rb').splitlines()
        except IOError:
            return gitcookies

        for line in f:
            try:
                fields = line.strip().split('\t')
                if line.strip().startswith('#') or len(fields) != 7:
                    continue
                domain, xpath, key, value = fields[0], fields[2], fields[
                    5], fields[6]
                if xpath == '/' and key == 'o':
                    if value.startswith('git-'):
                        login, secret_token = value.split('=', 1)
                        gitcookies[domain] = (login, secret_token)
                    else:
                        gitcookies[domain] = ('', value)
            except (IndexError, ValueError, TypeError) as exc:
                LOGGER.warning(exc)
        return gitcookies

    def _get_auth_for_host(self, host):
        for domain, creds in self.gitcookies.items():
            if http.cookiejar.domain_match(host, domain):
                return (creds[0], creds[1])
        return None

    def authenticate(self, conn: HttpConn):
        a = self._get_auth_for_host(conn.req_host)
        if a:
            login, cred = a
            if login:
                secret = base64.b64encode(f'{login}:{cred}'.encode('utf-8'))
                conn.req_headers[
                    'Authorization'] = f'Basic {secret.decode("utf-8")}'
            else:
                conn.req_headers['Authorization'] = f'Bearer {cred}'

    def ensure_authenticated(self, gerrit_host: str, git_host: str) -> Tuple[bool, str]:
        """Returns (bypassable, error message).

        If the error message is empty, there is no error to report.
        If bypassable is true, the caller will allow the user to continue past the
        error.
        """
        # Lazy-loader to identify Gerrit and Git hosts.
        gerrit_auth = self._get_auth_for_host(gerrit_host)
        git_auth = self._get_auth_for_host(git_host)
        if gerrit_auth and git_auth:
            if gerrit_auth == git_auth:
                return True, ''
            all_gsrc = self._get_auth_for_host('d0esN0tEx1st.googlesource.com')
            print(
                'WARNING: You have different credentials for Gerrit and git hosts:\n'
                '           %s\n'
                '           %s\n'
                '        Consider running the following command:\n'
                '          git cl creds-check\n'
                '        %s\n'
                '        %s' %
                (git_host, gerrit_host,
                 ('Hint: delete creds for .googlesource.com' if all_gsrc else
                  ''), self._get_new_password_message(git_host)))
            return True, 'If you know what you are doing'

        missing = (([] if gerrit_auth else [gerrit_host]) +
                   ([] if git_auth else [git_host]))
        return False, ('Credentials for the following hosts are required:\n'
                       '  %s\n'
                       'These are read from %s\n'
                       '%s' % ('\n  '.join(missing), self.get_gitcookies_path(),
                               self._get_new_password_message(git_host)))


    # Used to redact the cookies from the gitcookies file.
    GITCOOKIES_REDACT_RE = re.compile(r'1/.*')

    def debug_summary_state(self) -> str:
        gitcookies_path = self.get_gitcookies_path()
        if os.path.isfile(gitcookies_path):
            gitcookies = gclient_utils.FileRead(gitcookies_path)
            return self.GITCOOKIES_REDACT_RE.sub('REDACTED', gitcookies)
        return ''

    def get_auth_email(self, host):
        """Best effort parsing of email to be used for auth for the given host."""
        a = self._get_auth_for_host(host)
        if not a:
            return None
        login = a[0]
        # login typically looks like 'git-xxx.example.com'
        if not login.startswith('git-') or '.' not in login:
            return None
        username, domain = login[len('git-'):].split('.', 1)
        return '%s@%s' % (username, domain)


class GceAuthenticator(_Authenticator):
    """_Authenticator implementation that uses GCE metadata service for token.
    """

    _INFO_URL = 'http://metadata.google.internal'
    _ACQUIRE_URL = ('%s/computeMetadata/v1/instance/'
                    'service-accounts/default/token' % _INFO_URL)
    _ACQUIRE_HEADERS = {"Metadata-Flavor": "Google"}

    _cache_is_gce = None
    _token_cache = None
    _token_expiration = None

    @classmethod
    def is_applicable(cls, *, conn: Optional[HttpConn] = None):
        if os.getenv('SKIP_GCE_AUTH_FOR_GIT'):
            return False
        if cls._cache_is_gce is None:
            cls._cache_is_gce = cls._test_is_gce()
        return cls._cache_is_gce

    @classmethod
    def _test_is_gce(cls):
        # Based on https://cloud.google.com/compute/docs/metadata#runninggce
        resp, _ = cls._get(cls._INFO_URL)
        if resp is None:
            return False
        return resp.get('metadata-flavor') == 'Google'

    @staticmethod
    def _get(url, **kwargs):
        next_delay_sec = 1.0
        for i in range(TRY_LIMIT):
            p = urllib.parse.urlparse(url)
            if p.scheme not in ('http', 'https'):
                raise RuntimeError("Don't know how to work with protocol '%s'" %
                                   p.scheme)
            try:
                resp, contents = httplib2.Http().request(url, 'GET', **kwargs)
            except (socket.error, httplib2.HttpLib2Error,
                    httplib2.socks.ProxyError) as e:
                LOGGER.debug('GET [%s] raised %s', url, e)
                return None, None
            LOGGER.debug('GET [%s] #%d/%d (%d)', url, i + 1, TRY_LIMIT,
                         resp.status)
            if resp.status < 500:
                return (resp, contents)

            # Retry server error status codes.
            LOGGER.warning('Encountered server error')
            if TRY_LIMIT - i > 1:
                next_delay_sec = log_retry_and_sleep(next_delay_sec, i,
                                                     TRY_LIMIT)
        return None, None

    @classmethod
    def _get_token_dict(cls):
        # If cached token is valid for at least 25 seconds, return it.
        if cls._token_cache and time_time() + 25 < cls._token_expiration:
            return cls._token_cache

        resp, contents = cls._get(cls._ACQUIRE_URL,
                                  headers=cls._ACQUIRE_HEADERS)
        if resp is None or resp.status != 200:
            return None
        cls._token_cache = json.loads(contents)
        cls._token_expiration = cls._token_cache['expires_in'] + time_time()
        return cls._token_cache

    def authenticate(self, conn: HttpConn):
        token_dict = self._get_token_dict()
        if not token_dict:
            return
        conn.req_headers[
            'Authorization'] = '%(token_type)s %(access_token)s' % token_dict

    def debug_summary_state(self) -> str:
        # TODO(b/343230702) - report ambient account name.
        return ''


class LuciContextAuthenticator(_Authenticator):
    """_Authenticator implementation that uses LUCI_CONTEXT ambient local auth.
    """
    @staticmethod
    def is_applicable(*, conn: Optional[HttpConn] = None):
        return auth.has_luci_context_local_auth()

    def __init__(self):
        self._authenticator = auth.Authenticator(' '.join(
            [auth.OAUTH_SCOPE_EMAIL, auth.OAUTH_SCOPE_GERRIT]))

    @property
    def luci_auth(self) -> auth.Authenticator:
        return self._authenticator

    def authenticate(self, conn: HttpConn):
        conn.req_headers[
            'Authorization'] = f'Bearer {self._authenticator.get_access_token().token}'

    def debug_summary_state(self) -> str:
        # TODO(b/343230702) - report ambient account name.
        return ''


class GitCredsAuthenticator(_Authenticator):
    """_Authenticator implementation that uses `git-credential-luci` with OAuth.

    This is similar to LuciContextAuthenticator, except that it is for
    local non-google.com developer credentials.
    """

    def __init__(self):
        self._authenticator = auth.GerritAuthenticator()

    @property
    def luci_auth(self) -> auth.Authenticator:
        return self._authenticator

    def authenticate(self, conn: HttpConn):
        conn.req_headers[
            'Authorization'] = f'Bearer {self._authenticator.get_access_token()}'

    def debug_summary_state(self) -> str:
        # TODO(b/343230702) - report ambient account name.
        return ''

    @classmethod
    def gerrit_account_exists(cls, host: str) -> bool:
        """Return True if the Gerrit account exists.

        This checks the user currently logged in with git-credential-luci.
        If the user is not logged in with git-credential-luci, returns False.

        This method caches positive results in the user's Git config.
        """
        cwd = os.getcwd()
        LOGGER.debug("Checking Gerrit account existence for %r", host)
        hosts = scm.GIT.GetConfigList(cwd, 'depot-tools.hosthasaccount')
        if host in hosts:
            # If a user deletes their Gerrit account, then this cache
            # might be stale.  This should be a rare case and a user can
            # just delete this from their Git config.
            LOGGER.debug("Using cached account existence for Gerrit host %r",
                         host)
            return True
        try:
            info = GetAccountDetails(host, authenticator=cls())
        except auth.GitLoginRequiredError:
            LOGGER.debug(
                "Cannot check Gerrit account existence; missing git-credential-luci login"
            )
            return False
        except GerritError as e:
            if e.http_status == 400:
                # This is likely because the user doesn't have an
                # account on the Gerrit host.
                LOGGER.debug(
                    "Gerrit account check returned 400; likely account missing")
                return False
            raise
        if 'email' not in info:
            LOGGER.debug("Gerrit account does not exist on %r", host)
            return False
        LOGGER.debug("Gerrit account exists on %r", host)
        try:
            scm.GIT.SetConfig(cwd,
                              'depot-tools.hostHasAccount',
                              host,
                              append=True)
        except subprocess2.CalledProcessError as e:
            # This may be called outside of a Git repository (e.g., when
            # fetching from scratch), in which case we don't have a Git
            # repository to cache the results of our check, so skip the
            # caching.
            LOGGER.debug(
                "Got error trying to cache 'depot-tools.hostHasAccount': %s", e)
        return True

    def is_applicable(self, *, conn: Optional[HttpConn] = None):
        return self.gerrit_account_exists(conn.host)


class NoAuthenticator(_Authenticator):
    """_Authenticator implementation that does no auth.
    """

    @staticmethod
    def is_applicable(*, conn: Optional[HttpConn] = None):
        return True

    def authenticate(self, conn: HttpConn):
        pass

    def debug_summary_state(self) -> str:
        return ''


class ChainedAuthenticator(_Authenticator):
    """_Authenticator that delegates to others in sequence.

    Authenticators should implement the method `is_applicable_for`.
    """

    def __init__(self, authenticators: List[_Authenticator]):
        self.authenticators = list(authenticators)

    def is_applicable(self, *, conn: Optional[HttpConn] = None) -> bool:
        return bool(any(
            a.is_applicable(conn=conn) for a in self.authenticators))

    def authenticate(self, conn: HttpConn):
        for a in self.authenticators:
            if a.is_applicable(conn=conn):
                a.authenticate(conn)
                break
        else:
            raise ValueError(
                f'{self!r} has no applicable authenticator for {conn!r}')

    def debug_summary_state(self) -> str:
        return ''


class ReqParams(TypedDict):
    uri: str
    method: str
    headers: Dict[str, str]
    body: Optional[str]


class HttpConn(httplib2.Http):
    """HttpConn is an httplib2.Http with additional request-specific fields."""

    def __init__(self, *args, req_host: str, req_uri: str, req_method: str,
                 req_headers: Dict[str, str], req_body: Optional[str],
                 **kwargs) -> None:
        self.req_host = req_host
        self.req_uri = req_uri
        self.req_method = req_method
        self.req_headers = req_headers
        self.req_body = req_body
        super().__init__(*args, **kwargs)

    @property
    def req_params(self) -> ReqParams:
        return {
            'uri': self.req_uri,
            'method': self.req_method,
            'headers': self.req_headers,
            'body': self.req_body,
        }

    # NOTE: We want to use HttpConn with CookieJar.add_cookie_header, so have
    # compatible interface for that here.
    #
    # NOTE: Someone should really normalize this 'HttpConn' and httplib2
    # implementation to just be plain python3 stdlib instead. All of this was
    # written during the bad old days of python2.6/2.7, pre-vpython.
    def has_header(self, header: str) -> bool:
        return header in self.req_headers

    def get_full_url(self) -> str:
        return self.req_uri

    def get_header(self,
                   header: str,
                   default: Optional[str] = None) -> Optional[str]:
        return self.req_headers.get(header, default)

    def add_unredirected_header(self, header: str, value: str):
        # NOTE: httplib2 does not support unredirected headers.
        self.req_headers[header] = value

    @property
    def unverifiable(self) -> bool:
        return False

    @property
    def origin_req_host(self) -> str:
        return self.req_host

    @property
    def type(self) -> str:
        return urllib.parse.urlparse(self.req_uri).scheme

    @property
    def host(self) -> str:
        return self.req_host


def CreateHttpConn(host: str,
                   path: str,
                   reqtype='GET',
                   headers: Optional[Dict[str, str]] = None,
                   body: Optional[Dict] = None,
                   timeout=300,
                   *,
                   authenticator: Optional[_Authenticator] = None) -> HttpConn:
    """Opens an HTTPS connection to a Gerrit service, and sends a request."""
    headers = headers or {}
    bare_host = host.partition(':')[0]

    url = path
    if not url.startswith('/'):
        url = '/' + url
    if not url.startswith('/a/'):
        url = '/a%s' % url

    rendered_body: Optional[str] = None
    if body:
        rendered_body = json.dumps(body, sort_keys=True)
        headers.setdefault('Content-Type', 'application/json')

    uri = urllib.parse.urljoin(f'{GERRIT_PROTOCOL}://{host}', url)
    conn = HttpConn(timeout=timeout,
                    req_host=host,
                    req_uri=uri,
                    req_method=reqtype,
                    req_headers=headers,
                    req_body=rendered_body)

    if authenticator is None:
        authenticator = _Authenticator.get()
    # TODO(crbug.com/1059384): Automatically detect when running on cloudtop.
    if isinstance(authenticator, GceAuthenticator):
        print('If you\'re on a cloudtop instance, export '
              'SKIP_GCE_AUTH_FOR_GIT=1 in your env.')

    authenticator.authenticate(conn)

    if 'Authorization' not in conn.req_headers:
        LOGGER.debug('No authorization found for %s.' % bare_host)

    if LOGGER.isEnabledFor(logging.DEBUG):
        LOGGER.debug('%s %s', conn.req_method, conn.req_uri)
        LOGGER.debug('conn.proxy_info=%r', conn.proxy_info)
        for key, val in conn.req_headers.items():
            if key in ('Authorization', 'Cookie'):
                val = 'HIDDEN'
            LOGGER.debug('%s: %s', key, val)
        if conn.req_body:
            LOGGER.debug(conn.req_body)

    return conn


def ReadHttpResponse(conn: HttpConn,
                     accept_statuses: Container[int] = frozenset([200]),
                     max_tries=TRY_LIMIT):
    """Reads an HTTP response from a connection into a string buffer.

    Args:
        conn: An Http object created by CreateHttpConn above.
        accept_statuses: Treat any of these statuses as success. Default: [200]
            Common additions include 204, 400, and 404.
        max_tries: The maximum number of times the request should be attempted.
    Returns:
        A string buffer containing the connection's reply.
    """
    response = contents = None
    sleep_time = SLEEP_TIME
    for idx in range(max_tries):
        before_response = time.time()
        try:
            response, contents = conn.request(**conn.req_params)
        except socket.timeout:
            if idx < max_tries - 1:
                sleep_time = log_retry_and_sleep(sleep_time, idx, max_tries)
                continue
            raise
        contents = contents.decode('utf-8', 'replace')

        response_time = time.time() - before_response
        metrics.collector.add_repeated(
            'http_requests',
            metrics_utils.extract_http_metrics(conn.req_params['uri'],
                                               conn.req_params['method'],
                                               response.status, response_time))

        # If response.status is an accepted status,
        # or response.status < 500 then the result is final; break retry loop.
        # If the response is 404/409 it might be because of replication lag,
        # so keep trying anyway. If it is 429, it is generally ok to retry after
        # a backoff.
        if (response.status in accept_statuses or response.status < 500
                and response.status not in [404, 409, 429]):
            LOGGER.debug('got response %d for %s %s', response.status,
                         conn.req_params['method'], conn.req_params['uri'])
            # If 404 was in accept_statuses, then it's expected that the file
            # might not exist, so don't return the gitiles error page because
            # that's not the "content" that was actually requested.
            if response.status == 404:
                contents = ''
            break

        # A status >=500 is assumed to be a possible transient error; retry.
        http_version = 'HTTP/%s' % ('1.1' if response.version == 11 else '1.0')
        LOGGER.warning(
            'A transient error occurred while querying %s:\n'
            '%s %s %s\n'
            '%s %d %s\n'
            '%s', conn.req_host, conn.req_params['method'],
            conn.req_params['uri'], http_version, http_version, response.status,
            response.reason, contents)

        if idx < max_tries - 1:
            sleep_time = log_retry_and_sleep(sleep_time, idx, max_tries)
    # end of retries loop

    # Help the type checker a bit here - it can't figure out the `except` logic
    # in the loop above.
    assert response, (
        'Impossible: End of retry loop without response or exception.')

    if response.status in accept_statuses:
        return StringIO(contents)

    if response.status in (302, 401, 403):
        www_authenticate = response.get('www-authenticate')
        if not www_authenticate:
            print('Your Gerrit credentials might be misconfigured.')
        elif not newauth.Enabled():
            auth_match = re.search('realm="([^"]+)"', www_authenticate, re.I)
            host = auth_match.group(1) if auth_match else conn.req_host
            new_password_url = CookiesAuthenticator.get_new_password_url(host)
            print('Authentication failed. Please make sure your .gitcookies '
                  f'file has credentials for {host}.')
            print(f'(Re)generate credentials here: {new_password_url}')
        print('Try:\n  git cl creds-check')

    reason = '%s: %s' % (response.reason, contents)
    raise GerritError(response.status, reason)


def ReadHttpJsonResponse(conn,
                         accept_statuses: Container[int] = frozenset([200]),
                         max_tries=TRY_LIMIT) -> dict:
    """Parses an https response as json."""
    fh = ReadHttpResponse(conn, accept_statuses, max_tries)
    # The first line of the response should always be: )]}'
    s = fh.readline()
    if s and s.rstrip() != ")]}'":
        raise GerritError(200, 'Unexpected json output: %s' % s[:100])
    s = fh.read()
    if not s:
        return {}
    return json.loads(s)


def CallGerritApi(host, path, **kwargs):
    """Helper for calling a Gerrit API that returns a JSON response."""
    conn_kwargs = {}
    conn_kwargs.update(
        (k, kwargs[k]) for k in ['reqtype', 'headers', 'body'] if k in kwargs)
    conn = CreateHttpConn(host, path, **conn_kwargs)
    read_kwargs = {}
    read_kwargs.update(
        (k, kwargs[k]) for k in ['accept_statuses'] if k in kwargs)
    return ReadHttpJsonResponse(conn, **read_kwargs)


def QueryChanges(host,
                 params,
                 first_param=None,
                 limit=None,
                 o_params=None,
                 start=None):
    """
    Queries a gerrit-on-borg server for changes matching query terms.

    Args:
        params: A list of key:value pairs for search parameters, as documented
            here (e.g. ('is', 'owner') for a parameter 'is:owner'):
            https://gerrit-review.googlesource.com/Documentation/user-search.html#search-operators
        first_param: A change identifier
        limit: Maximum number of results to return.
        start: how many changes to skip (starting with the most recent)
        o_params: A list of additional output specifiers, as documented here:
            https://gerrit-review.googlesource.com/Documentation/rest-api-changes.html#list-changes

    Returns:
        A list of json-decoded query results.
    """
    # Note that no attempt is made to escape special characters; YMMV.
    if not params and not first_param:
        raise RuntimeError('QueryChanges requires search parameters')
    path = 'changes/?q=%s' % _QueryString(params, first_param)
    if start:
        path = '%s&start=%s' % (path, start)
    if limit:
        path = '%s&n=%d' % (path, limit)
    if o_params:
        path = '%s&%s' % (path, '&'.join(['o=%s' % p for p in o_params]))
    return ReadHttpJsonResponse(CreateHttpConn(host, path, timeout=30))


def GenerateAllChanges(host,
                       params,
                       first_param=None,
                       limit=500,
                       o_params=None,
                       start=None):
    """Queries a gerrit-on-borg server for all the changes matching the query
    terms.

    WARNING: this is unreliable if a change matching the query is modified while
    this function is being called.

    A single query to gerrit-on-borg is limited on the number of results by the
    limit parameter on the request (see QueryChanges) and the server maximum
    limit.

    Args:
        params, first_param: Refer to QueryChanges().
        limit: Maximum number of requested changes per query.
        o_params: Refer to QueryChanges().
        start: Refer to QueryChanges().

    Returns:
        A generator object to the list of returned changes.
    """
    already_returned = set()

    def at_most_once(cls):
        for cl in cls:
            if cl['_number'] not in already_returned:
                already_returned.add(cl['_number'])
                yield cl

    start = start or 0
    cur_start = start
    more_changes = True

    while more_changes:
        # This will fetch changes[start..start+limit] sorted by most recently
        # updated. Since the rank of any change in this list can be changed any
        # time (say user posting comment), subsequent calls may overalp like
        # this: > initial order ABCDEFGH query[0..3]  => ABC > E gets updated.
        # New order: EABCDFGH query[3..6] => CDF   # C is a dup query[6..9] =>
        # GH    # E is missed.
        page = QueryChanges(host, params, first_param, limit, o_params,
                            cur_start)
        for cl in at_most_once(page):
            yield cl

        more_changes = [cl for cl in page if '_more_changes' in cl]
        if len(more_changes) > 1:
            raise GerritError(
                200,
                'Received %d changes with a _more_changes attribute set but should '
                'receive at most one.' % len(more_changes))
        if more_changes:
            cur_start += len(page)

    # If we paged through, query again the first page which in most
    # circumstances will fetch all changes that were modified while this
    # function was run.
    if start != cur_start:
        page = QueryChanges(host, params, first_param, limit, o_params, start)
        for cl in at_most_once(page):
            yield cl


def MultiQueryChanges(host,
                      params,
                      change_list,
                      limit=None,
                      o_params=None,
                      start=None):
    """Initiate a query composed of multiple sets of query parameters."""
    if not change_list:
        raise RuntimeError(
            "MultiQueryChanges requires a list of change numbers/id's")
    q = [
        'q=%s' % '+OR+'.join([urllib.parse.quote(str(x)) for x in change_list])
    ]
    if params:
        q.append(_QueryString(params))
    if limit:
        q.append('n=%d' % limit)
    if start:
        q.append('S=%s' % start)
    if o_params:
        q.extend(['o=%s' % p for p in o_params])
    path = 'changes/?%s' % '&'.join(q)
    try:
        result = ReadHttpJsonResponse(CreateHttpConn(host, path))
    except GerritError as e:
        msg = '%s:\n%s' % (e.message, path)
        raise GerritError(e.http_status, msg)
    return result


def GetGerritFetchUrl(host):
    """Given a Gerrit host name returns URL of a Gerrit instance to fetch from."""
    return '%s://%s/' % (GERRIT_PROTOCOL, host)


def GetCodeReviewTbrScore(host, project):
    """Given a Gerrit host name and project, return the Code-Review score for TBR.
    """
    conn = CreateHttpConn(host,
                          '/projects/%s' % urllib.parse.quote(project, ''))
    project = ReadHttpJsonResponse(conn)
    if ('labels' not in project or 'Code-Review' not in project['labels']
            or 'values' not in project['labels']['Code-Review']):
        return 1
    return max([int(x) for x in project['labels']['Code-Review']['values']])


def GetChangePageUrl(host, change_number):
    """Given a Gerrit host name and change number, returns change page URL."""
    return '%s://%s/#/c/%d/' % (GERRIT_PROTOCOL, host, change_number)


def GetChangeUrl(host, change):
    """Given a Gerrit host name and change ID, returns a URL for the change."""
    return '%s://%s/a/changes/%s' % (GERRIT_PROTOCOL, host, change)


def GetChange(host, change, accept_statuses: Container[int] = frozenset([200])):
    """Queries a Gerrit server for information about a single change."""
    path = 'changes/%s' % change
    return ReadHttpJsonResponse(CreateHttpConn(host, path),
                                accept_statuses=accept_statuses)


def GetChangeDetail(host, change, o_params=None):
    """Queries a Gerrit server for extended information about a single change."""
    path = 'changes/%s/detail' % change
    if o_params:
        path += '?%s' % '&'.join(['o=%s' % p for p in o_params])
    return ReadHttpJsonResponse(CreateHttpConn(host, path))


def GetChangeCommit(host: str, change: str, revision: str = 'current') -> dict:
    """Query a Gerrit server for a revision associated with a change."""
    path = 'changes/%s/revisions/%s/commit?links' % (change, revision)
    return ReadHttpJsonResponse(CreateHttpConn(host, path))


def GetChangeCurrentRevision(host, change):
    """Get information about the latest revision for a given change."""
    return QueryChanges(host, [], change, o_params=('CURRENT_REVISION', ))


def GetChangeRevisions(host, change):
    """Gets information about all revisions associated with a change."""
    return QueryChanges(host, [], change, o_params=('ALL_REVISIONS', ))


def GetChangeReview(host, change, revision=None):
    """Gets the current review information for a change."""
    if not revision:
        jmsg = GetChangeRevisions(host, change)
        if not jmsg:
            return None

        if len(jmsg) > 1:
            raise GerritError(
                200, 'Multiple changes found for ChangeId %s.' % change)
        revision = jmsg[0]['current_revision']
    path = 'changes/%s/revisions/%s/review'
    return ReadHttpJsonResponse(CreateHttpConn(host, path))


def GetChangeComments(host, change):
    """Get the line- and file-level comments on a change."""
    path = 'changes/%s/comments' % change
    return ReadHttpJsonResponse(CreateHttpConn(host, path))


def GetChangeRobotComments(host, change):
    """Gets the line- and file-level robot comments on a change."""
    path = 'changes/%s/robotcomments' % change
    return ReadHttpJsonResponse(CreateHttpConn(host, path))


def GetRelatedChanges(host, change, revision='current'):
    """Gets the related changes for a given change and revision."""
    path = 'changes/%s/revisions/%s/related' % (change, revision)
    return ReadHttpJsonResponse(CreateHttpConn(host, path))


def AbandonChange(host, change, msg=''):
    """Abandons a Gerrit change."""
    path = 'changes/%s/abandon' % change
    body = {'message': msg} if msg else {}
    conn = CreateHttpConn(host, path, reqtype='POST', body=body)
    return ReadHttpJsonResponse(conn)


def MoveChange(host, change, destination_branch):
    """Move a Gerrit change to different destination branch."""
    path = 'changes/%s/move' % change
    body = {'destination_branch': destination_branch, 'keep_all_votes': True}
    conn = CreateHttpConn(host, path, reqtype='POST', body=body)
    return ReadHttpJsonResponse(conn)


def RestoreChange(host, change, msg=''):
    """Restores a previously abandoned change."""
    path = 'changes/%s/restore' % change
    body = {'message': msg} if msg else {}
    conn = CreateHttpConn(host, path, reqtype='POST', body=body)
    return ReadHttpJsonResponse(conn)


def RebaseChange(host, change, base=None):
    """Rebases a change."""
    path = f'changes/{change}/rebase'
    body = {'base': base} if base else {}
    conn = CreateHttpConn(host, path, reqtype='POST', body=body)
    # If a rebase fails due to a merge conflict, Gerrit returns 409. Retrying
    # more than once probably won't help since the merge conflict will still
    # exist.
    return ReadHttpJsonResponse(conn, max_tries=2)


def SubmitChange(host, change):
    """Submits a Gerrit change via Gerrit."""
    path = 'changes/%s/submit' % change
    conn = CreateHttpConn(host, path, reqtype='POST')
    # If a submit fails due to a merge conflict, Gerrit returns 409. Retrying
    # more than once probably won't help since the merge conflict will still
    # exist.
    return ReadHttpJsonResponse(conn, max_tries=2)


def GetChangesSubmittedTogether(host, change):
    """Get all changes submitted with the given one."""
    path = 'changes/%s/submitted_together?o=NON_VISIBLE_CHANGES' % change
    conn = CreateHttpConn(host, path, reqtype='GET')
    return ReadHttpJsonResponse(conn)


def PublishChangeEdit(host, change, notify=True):
    """Publish a Gerrit change edit."""
    path = 'changes/%s/edit:publish' % change
    body = {'notify': 'ALL' if notify else 'NONE'}
    conn = CreateHttpConn(host, path, reqtype='POST', body=body)
    return ReadHttpJsonResponse(conn, accept_statuses=(204, ))


def ChangeEdit(host, change, path, data):
    """Puts content of a file into a change edit."""
    path = 'changes/%s/edit/%s' % (change, urllib.parse.quote(path, ''))
    body = {
        'binary_content':
        'data:text/plain;base64,%s' %
        base64.b64encode(data.encode('utf-8')).decode('utf-8')
    }
    conn = CreateHttpConn(host, path, reqtype='PUT', body=body)
    return ReadHttpJsonResponse(conn, accept_statuses=(204, 409))


def SetChangeEditMessage(host, change, message):
    """Sets the commit message of a change edit."""
    path = 'changes/%s/edit:message' % change
    body = {'message': message}
    conn = CreateHttpConn(host, path, reqtype='PUT', body=body)
    return ReadHttpJsonResponse(conn, accept_statuses=(204, 409))


def HasPendingChangeEdit(host, change):
    conn = CreateHttpConn(host, 'changes/%s/edit' % change)
    try:
        ReadHttpResponse(conn)
    except GerritError as e:
        # 204 No Content means no pending change.
        if e.http_status == 204:
            return False
        raise
    return True


def DeletePendingChangeEdit(host, change):
    conn = CreateHttpConn(host, 'changes/%s/edit' % change, reqtype='DELETE')
    # On success, Gerrit returns status 204; if the edit was already deleted it
    # returns 404.  Anything else is an error.
    ReadHttpResponse(conn, accept_statuses=[204, 404])


def CherryPick(host, change, destination, revision='current', message=None):
    """Create a cherry-pick commit from the given change, onto the given
    destination.
    """
    path = 'changes/%s/revisions/%s/cherrypick' % (change, revision)
    body = {'destination': destination}
    if message:
        body['message'] = message
    conn = CreateHttpConn(host, path, reqtype='POST', body=body)

    # If a cherry pick fails due to a merge conflict, Gerrit returns 409.
    # Retrying more than once probably won't help since the merge conflict will
    # still exist.
    return ReadHttpJsonResponse(conn, max_tries=2)


def CherryPickCommit(host, project, commit, destination):
    """Cherry-pick a commit from a project to a destination branch."""
    project = urllib.parse.quote(project, '')
    path = f'projects/{project}/commits/{commit}/cherrypick'
    body = {'destination': destination}
    conn = CreateHttpConn(host, path, reqtype='POST', body=body)
    return ReadHttpJsonResponse(conn)


def GetFileContents(host, change, path):
    """Get the contents of a file with the given path in the given revision.

    Returns:
        A bytes object with the file's contents.
    """
    path = 'changes/%s/revisions/current/files/%s/content' % (
        change, urllib.parse.quote(path, ''))
    conn = CreateHttpConn(host, path, reqtype='GET')
    return base64.b64decode(ReadHttpResponse(conn).read())


def SetCommitMessage(host, change, description, notify='ALL'):
    """Updates a commit message."""
    assert notify in ('ALL', 'NONE')
    path = 'changes/%s/message' % change
    body = {'message': description, 'notify': notify}
    conn = CreateHttpConn(host, path, reqtype='PUT', body=body)
    try:
        ReadHttpResponse(conn, accept_statuses=[200, 204])
    except GerritError as e:
        raise GerritError(
            e.http_status,
            'Received unexpected http status while editing message '
            'in change %s' % change)


def GetCommitIncludedIn(host, project, commit):
    """Retrieves the branches and tags for a given commit.

    https://gerrit-review.googlesource.com/Documentation/rest-api-projects.html#get-included-in

    Returns:
        A JSON object with keys of 'branches' and 'tags'.
    """
    path = 'projects/%s/commits/%s/in' % (urllib.parse.quote(project,
                                                             ''), commit)
    conn = CreateHttpConn(host, path, reqtype='GET')
    return ReadHttpJsonResponse(conn, accept_statuses=[200])


def IsCodeOwnersEnabledOnHost(host):
    """Check if the code-owners plugin is enabled for the host."""
    path = 'config/server/capabilities'
    capabilities = ReadHttpJsonResponse(CreateHttpConn(host, path))
    return 'code-owners-checkCodeOwner' in capabilities


def IsCodeOwnersEnabledOnRepo(host, repo):
    """Check if the code-owners plugin is enabled for the repo."""
    repo = PercentEncodeForGitRef(repo)
    path = '/projects/%s/code_owners.project_config' % repo
    config = ReadHttpJsonResponse(CreateHttpConn(host, path))
    return not config['status'].get('disabled', False)


def GetOwnersForFile(host,
                     project,
                     branch,
                     path,
                     limit=100,
                     resolve_all_users=True,
                     highest_score_only=False,
                     seed=None,
                     o_params=('DETAILS', )):
    """Gets information about owners attached to a file."""
    path = 'projects/%s/branches/%s/code_owners/%s' % (urllib.parse.quote(
        project, ''), urllib.parse.quote(branch,
                                         ''), urllib.parse.quote(path, ''))
    q = ['resolve-all-users=%s' % json.dumps(resolve_all_users)]
    if highest_score_only:
        q.append('highest-score-only=%s' % json.dumps(highest_score_only))
    if seed:
        q.append('seed=%d' % seed)
    if limit:
        q.append('n=%d' % limit)
    if o_params:
        q.extend(['o=%s' % p for p in o_params])
    if q:
        path = '%s?%s' % (path, '&'.join(q))
    return ReadHttpJsonResponse(CreateHttpConn(host, path))


def GetReviewers(host, change):
    """Gets information about all reviewers attached to a change."""
    path = 'changes/%s/reviewers' % change
    return ReadHttpJsonResponse(CreateHttpConn(host, path))


def GetReview(host, change, revision):
    """Gets review information about a specific revision of a change."""
    path = 'changes/%s/revisions/%s/review' % (change, revision)
    return ReadHttpJsonResponse(CreateHttpConn(host, path))


def AddReviewers(host,
                 change,
                 reviewers=None,
                 ccs=None,
                 notify=True,
                 accept_statuses: Container[int] = frozenset([200, 400, 422])):
    """Add reviewers to a change."""
    if not reviewers and not ccs:
        return None
    if not change:
        return None
    reviewers = frozenset(reviewers or [])
    ccs = frozenset(ccs or [])
    path = 'changes/%s/revisions/current/review' % change

    body = {
        'drafts': 'KEEP',
        'reviewers': [],
        'notify': 'ALL' if notify else 'NONE',
    }
    for r in sorted(reviewers | ccs):
        state = 'REVIEWER' if r in reviewers else 'CC'
        body['reviewers'].append({
            'reviewer': r,
            'state': state,
            'notify': 'NONE',  # We handled `notify` argument above.
        })

    conn = CreateHttpConn(host, path, reqtype='POST', body=body)
    # Gerrit will return 400 if one or more of the requested reviewers are
    # unprocessable. We read the response object to see which were rejected,
    # warn about them, and retry with the remainder.
    resp = ReadHttpJsonResponse(conn, accept_statuses=accept_statuses)

    errored = set()
    for result in resp.get('reviewers', {}).values():
        r = result.get('input')
        state = 'REVIEWER' if r in reviewers else 'CC'
        if result.get('error'):
            errored.add(r)
            LOGGER.warning('Note: "%s" not added as a %s' % (r, state.lower()))
    if errored:
        # Try again, adding only those that didn't fail, and only accepting 200.
        AddReviewers(host,
                     change,
                     reviewers=(reviewers - errored),
                     ccs=(ccs - errored),
                     notify=notify,
                     accept_statuses=[200])


def SetReview(host,
              change,
              revision='current',
              msg=None,
              labels=None,
              notify=None,
              ready=None,
              automatic_attention_set_update: Optional[bool] = None):
    """Sets labels and/or adds a message to a code review.

    https://gerrit-review.googlesource.com/Documentation/rest-api-changes.html#set-review
    """
    if not msg and not labels:
        return
    path = f'changes/{change}/revisions/{revision}/review'
    body: Dict[str, Any] = {'drafts': 'KEEP'}
    if msg:
        body['message'] = msg
    if labels:
        body['labels'] = labels
    if notify is not None:
        body['notify'] = 'ALL' if notify else 'NONE'
    if ready:
        body['ready'] = True
    if automatic_attention_set_update is not None:
        body[
            'ignore_automatic_attention_set_rules'] = not automatic_attention_set_update
    conn = CreateHttpConn(host, path, reqtype='POST', body=body)
    response = ReadHttpJsonResponse(conn)
    if labels:
        for key, val in labels.items():
            if ('labels' not in response or key not in response['labels']
                    or int(response['labels'][key] != int(val))):
                raise GerritError(
                    200,
                    'Unable to set "%s" label on change %s.' % (key, change))
    return response


def ResetReviewLabels(host,
                      change,
                      label,
                      value='0',
                      message=None,
                      notify=None):
    """Resets the value of a given label for all reviewers on a change."""
    # This is tricky, because we want to work on the "current revision", but
    # there's always the risk that "current revision" will change in between
    # API calls.  So, we check "current revision" at the beginning and end; if
    # it has changed, raise an exception.
    jmsg = GetChangeCurrentRevision(host, change)
    if not jmsg:
        raise GerritError(
            200, 'Could not get review information for change "%s"' % change)
    value = str(value)
    revision = jmsg[0]['current_revision']
    path = 'changes/%s/revisions/%s/review' % (change, revision)
    message = message or ('%s label set to %s programmatically.' %
                          (label, value))
    jmsg = GetReview(host, change, revision)
    if not jmsg:
        raise GerritError(
            200, 'Could not get review information for revision %s '
            'of change %s' % (revision, change))
    for review in jmsg.get('labels', {}).get(label, {}).get('all', []):
        if str(review.get('value', value)) != value:
            body = {
                'drafts': 'KEEP',
                'message': message,
                'labels': {
                    label: value
                },
                'on_behalf_of': review['_account_id'],
            }
            if notify:
                body['notify'] = notify
            conn = CreateHttpConn(host, path, reqtype='POST', body=body)
            response = ReadHttpJsonResponse(conn)
            if str(response['labels'][label]) != value:
                username = review.get('email', jmsg.get('name', ''))
                raise GerritError(
                    200, 'Unable to set %s label for user "%s"'
                    ' on change %s.' % (label, username, change))
    jmsg = GetChangeCurrentRevision(host, change)
    if not jmsg:
        raise GerritError(
            200, 'Could not get review information for change "%s"' % change)

    if jmsg[0]['current_revision'] != revision:
        raise GerritError(
            200, 'While resetting labels on change "%s", '
            'a new patchset was uploaded.' % change)


def CreateChange(host, project, branch='main', subject='', params=()):
    """
    Creates a new change.

    Args:
        params: A list of additional ChangeInput specifiers, as documented here:
            (e.g. ('is_private', 'true') to mark the change private.
            https://gerrit-review.googlesource.com/Documentation/rest-api-changes.html#change-input

    Returns:
        ChangeInfo for the new change.
    """
    path = 'changes/'
    body = {'project': project, 'branch': branch, 'subject': subject}
    body.update(dict(params))
    for key in 'project', 'branch', 'subject':
        if not body[key]:
            raise GerritError(200, '%s is required' % key.title())

    conn = CreateHttpConn(host, path, reqtype='POST', body=body)
    return ReadHttpJsonResponse(conn, accept_statuses=[201])


def CreateGerritBranch(host, project, branch, commit):
    """Creates a new branch from given project and commit

    https://gerrit-review.googlesource.com/Documentation/rest-api-projects.html#create-branch

    Returns:
        A JSON object with 'ref' key.
    """
    path = 'projects/%s/branches/%s' % (project, branch)
    body = {'revision': commit}
    conn = CreateHttpConn(host, path, reqtype='PUT', body=body)
    response = ReadHttpJsonResponse(conn, accept_statuses=[201, 409])
    if response:
        return response
    raise GerritError(200, 'Unable to create gerrit branch')


def CreateGerritTag(host, project, tag, commit):
    """Creates a new tag at the given commit.

    https://gerrit-review.googlesource.com/Documentation/rest-api-projects.html#create-tag

    Returns:
        A JSON object with 'ref' key.
    """
    path = 'projects/%s/tags/%s' % (project, tag)
    body = {'revision': commit}
    conn = CreateHttpConn(host, path, reqtype='PUT', body=body)
    response = ReadHttpJsonResponse(conn, accept_statuses=[201])
    if response:
        return response
    raise GerritError(200, 'Unable to create gerrit tag')


def GetHead(host, project):
    """Retrieves current HEAD of Gerrit project

    https://gerrit-review.googlesource.com/Documentation/rest-api-projects.html#get-head

    Returns:
        A JSON object with 'ref' key.
    """
    path = 'projects/%s/HEAD' % (project)
    conn = CreateHttpConn(host, path, reqtype='GET')
    response = ReadHttpJsonResponse(conn, accept_statuses=[200])
    if response:
        return response
    raise GerritError(200, 'Unable to update gerrit HEAD')


def UpdateHead(host, project, branch):
    """Updates Gerrit HEAD to point to branch

    https://gerrit-review.googlesource.com/Documentation/rest-api-projects.html#set-head

    Returns:
        A JSON object with 'ref' key.
    """
    path = 'projects/%s/HEAD' % (project)
    body = {'ref': branch}
    conn = CreateHttpConn(host, path, reqtype='PUT', body=body)
    response = ReadHttpJsonResponse(conn, accept_statuses=[200])
    if response:
        return response
    raise GerritError(200, 'Unable to update gerrit HEAD')


def GetGerritBranch(host, project, branch):
    """Gets a branch info from given project and branch name.

    See:
    https://gerrit-review.googlesource.com/Documentation/rest-api-projects.html#get-branch

    Returns:
        A JSON object with 'revision' key if the branch exists, otherwise None.
    """
    path = 'projects/%s/branches/%s' % (project, branch)
    conn = CreateHttpConn(host, path, reqtype='GET')
    return ReadHttpJsonResponse(conn, accept_statuses=[200, 404])


def GetProjectHead(host, project):
    conn = CreateHttpConn(host,
                          '/projects/%s/HEAD' % urllib.parse.quote(project, ''))
    return ReadHttpJsonResponse(conn, accept_statuses=[200])


def GetAccountDetails(host,
                      account_id='self',
                      *,
                      authenticator: Optional[_Authenticator] = None):
    """Returns details of the account.

    If account_id is not given, uses magic value 'self' which corresponds to
    whichever account user is authenticating as.

    Documentation:
    https://gerrit-review.googlesource.com/Documentation/rest-api-accounts.html#get-account

    Returns None if account is not found (i.e., Gerrit returned 404).
    """
    conn = CreateHttpConn(host,
                          '/accounts/%s' % account_id,
                          authenticator=authenticator)
    return ReadHttpJsonResponse(conn, accept_statuses=[200, 404])


class EmailRecord(TypedDict):
    email: str
    preferred: bool  # This should be NotRequired[bool] in 3.11+


def GetAccountEmails(host,
                     account_id='self',
                     *,
                     authenticator: Optional[_Authenticator] = None
                     ) -> Optional[List[EmailRecord]]:
    """Returns all emails for this account, and an indication of which of these
    is preferred.

    If account_id is not given, uses magic value 'self' which corresponds to
    whichever account user is authenticating as.

    Requires Modify Account permission to view emails other than 'self'.

    Documentation:
    https://gerrit-review.googlesource.com/Documentation/rest-api-accounts.html#list-account-emails

    Returns None if account is not found (i.e. Gerrit returned 404).
    """
    conn = CreateHttpConn(host,
                          '/accounts/%s/emails' % account_id,
                          authenticator=authenticator)
    resp = ReadHttpJsonResponse(conn, accept_statuses=[200, 404])
    if resp is None:
        return None
    return cast(List[EmailRecord], resp)


def ValidAccounts(host, accounts, max_threads=10):
    """Returns a mapping from valid account to its details.

    Invalid accounts, either not existing or without unique match,
    are not present as returned dictionary keys.
    """
    assert not isinstance(accounts, str), type(accounts)
    accounts = list(set(accounts))
    if not accounts:
        return {}

    def get_one(account):
        try:
            return account, GetAccountDetails(host, account)
        except GerritError:
            return None, None

    valid = {}
    with contextlib.closing(ThreadPool(min(max_threads,
                                           len(accounts)))) as pool:
        for account, details in pool.map(get_one, accounts):
            if account and details:
                valid[account] = details
    return valid


def PercentEncodeForGitRef(original):
    """Applies percent-encoding for strings sent to Gerrit via git ref metadata.

    The encoding used is based on but stricter than URL encoding (Section 2.1 of
    RFC 3986). The only non-escaped characters are alphanumerics, and 'SPACE'
    (U+0020) can be represented as 'LOW LINE' (U+005F) or 'PLUS SIGN' (U+002B).

    For more information, see the Gerrit docs here:

    https://gerrit-review.googlesource.com/Documentation/user-upload.html#message
    """
    safe = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 '
    encoded = ''.join(c if c in safe else '%%%02X' % ord(c) for c in original)

    # Spaces are not allowed in git refs; gerrit will interpret either '_' or
    # '+' (or '%20') as space. Use '_' since that has been supported the
    # longest.
    return encoded.replace(' ', '_')


@contextlib.contextmanager
def tempdir():
    tdir = None
    try:
        tdir = tempfile.mkdtemp(suffix='gerrit_util')
        yield tdir
    finally:
        if tdir:
            gclient_utils.rmtree(tdir)


def ChangeIdentifier(project, change_number):
    """Returns change identifier "project~number" suitable for |change| arg of
    this module API.

    Such format is allows for more efficient Gerrit routing of HTTP requests,
    comparing to specifying just change_number.
    """
    assert int(change_number)
    return '%s~%s' % (urllib.parse.quote(project, ''), change_number)
