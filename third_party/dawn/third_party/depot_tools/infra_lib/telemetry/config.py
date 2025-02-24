# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Provides telemetry configuration utilities."""

import configparser
import datetime
import os
from pathlib import Path
import shutil
import tempfile
import time
from typing import Literal
import uuid

DEFAULT_CONFIG_FILE = Path(Path.home(),
                           '.config/chrome_infra/telemetry/telemetry.cfg')

ROOT_SECTION_KEY = "root"
NOTICE_COUNTDOWN_KEY = "notice_countdown"
ENABLED_KEY = "enabled"
ENABLED_REASON_KEY = "enabled_reason"
TRACE_SECTION_KEY = "trace"
DEFAULT_CONFIG = {
    ROOT_SECTION_KEY: {
        NOTICE_COUNTDOWN_KEY: 10
    },
    TRACE_SECTION_KEY: {},
}
BATCH_PUBLISHING_ENABLED_KEY = "batch_publishing"
# The "telemetry in development" config to allow publishing the telemetry, but
# easily filtering it out later.
KEY_DEV = "development"
# Can be set, but the value is unused, pending approvals.
KEY_USER_UUID = "user_uuid"
KEY_USER_UUID_TIMESTAMP = "user_uuid_generated"


class TraceConfig:
    """Tracing specific config in Telemetry config."""

    def __init__(self, config: configparser.ConfigParser) -> None:
        self._config = config

    def update(self, enabled: bool, reason: Literal["AUTO", "USER"]) -> None:
        """Update the config."""
        self._config[TRACE_SECTION_KEY][ENABLED_KEY] = str(enabled)
        self._config[TRACE_SECTION_KEY][ENABLED_REASON_KEY] = reason
        if enabled:
            self.gen_id()

    def gen_id(self, regen=False) -> None:
        """[Re]generate UUIDs."""
        if regen or self._uuid_stale():
            self._config[TRACE_SECTION_KEY][KEY_USER_UUID] = str(uuid.uuid4())
            self._config[TRACE_SECTION_KEY][KEY_USER_UUID_TIMESTAMP] = str(
                int(time.time()))

    @property
    def batch(self) -> bool:
        """Check if batch uploads are configured."""
        return self._config[TRACE_SECTION_KEY].getboolean(
            BATCH_PUBLISHING_ENABLED_KEY, False)

    @batch.setter
    def batch(self, enabled: bool) -> None:
        """Set or delete the batch flag."""
        self._config[TRACE_SECTION_KEY][BATCH_PUBLISHING_ENABLED_KEY] = str(
            enabled)

    def _uuid_stale(self):
        """Check if the UUID is stale or doesn't exist."""
        if (KEY_USER_UUID not in self._config[TRACE_SECTION_KEY] or
                KEY_USER_UUID_TIMESTAMP not in self._config[TRACE_SECTION_KEY]):
            return True

        # Regen the UUID once per week. Regen every Monday so the work week is
        # captured under a single ID.
        regen_ts = int(self._config[TRACE_SECTION_KEY][KEY_USER_UUID_TIMESTAMP])
        regen_dt = datetime.datetime.fromtimestamp(regen_ts)
        today = datetime.datetime.now().replace(hour=0,
                                                minute=0,
                                                second=0,
                                                microsecond=0)
        monday = today - datetime.timedelta(days=today.weekday())
        return regen_dt < monday

    def has_enabled(self) -> bool:
        """Checks if the enabled property exists in config."""
        return ENABLED_KEY in self._config[TRACE_SECTION_KEY]

    @property
    def enabled(self) -> bool:
        """Value of trace.enabled property in telemetry.cfg."""
        return self._config[TRACE_SECTION_KEY].getboolean(ENABLED_KEY, False)

    @property
    def enabled_reason(self) -> Literal["AUTO", "USER"]:
        """Value of trace.enabled_reason property in telemetry.cfg."""
        return self._config[TRACE_SECTION_KEY].get(ENABLED_REASON_KEY, "AUTO")

    @property
    def dev_flag(self):
        """Check the telemetry development flag."""
        return self._config[TRACE_SECTION_KEY].getboolean(KEY_DEV, False)

    @dev_flag.setter
    def dev_flag(self, enabled: bool) -> None:
        """Set or delete the development flag."""
        if enabled:
            self._config[TRACE_SECTION_KEY][KEY_DEV] = str(enabled)
        elif KEY_DEV in self._config[TRACE_SECTION_KEY]:
            del self._config[TRACE_SECTION_KEY][KEY_DEV]

    def user_uuid(self) -> str:
        """Get the user UUID value."""
        return self._config[TRACE_SECTION_KEY].get(KEY_USER_UUID, "")


class RootConfig:
    """Root configs in Telemetry config."""

    def __init__(self, config) -> None:
        self._config = config

    def update(self, notice_countdown: int) -> None:
        """Update the config."""
        self._config[ROOT_SECTION_KEY][NOTICE_COUNTDOWN_KEY] = str(
            notice_countdown)

    @property
    def notice_countdown(self) -> int:
        """Value for root.notice_countdown property in telemetry.cfg."""

        return self._config[ROOT_SECTION_KEY].getint(NOTICE_COUNTDOWN_KEY, 10)


class Config:
    """Telemetry configuration."""

    def __init__(self, path: os.PathLike) -> None:
        self._path = Path(path)
        self._config = configparser.ConfigParser()

        self._config.read_dict(DEFAULT_CONFIG)
        if not self._path.exists():
            self.flush()
        else:
            with self._path.open("r", encoding="utf-8") as configfile:
                self._config.read_file(configfile)

        self._trace_config = TraceConfig(self._config)
        self._root_config = RootConfig(self._config)

    def flush(self) -> None:
        """Flushes the current config to config file."""

        tmpfile = tempfile.NamedTemporaryFile()
        with open(tmpfile.name, "w", encoding="utf-8") as configfile:
            self._config.write(configfile)

        if not self._path.parent.exists():
            self._path.parent.mkdir(parents=True, exist_ok=True)

        shutil.copy(tmpfile.name, self._path)

    @property
    def root_config(self) -> RootConfig:
        """The root config in telemetry."""
        return self._root_config

    @property
    def trace_config(self) -> TraceConfig:
        """The trace config in telemetry."""
        return self._trace_config
