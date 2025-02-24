# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""The tests for resource detector classes."""

import getpass
import logging
import os
from pathlib import Path
import platform
import sys

from . import detector
from opentelemetry.sdk import resources


def mock_exists(path: os.PathLike, val: bool):
    """Mock Path.exists for specified path."""

    exists = Path.exists

    def _mock_exists(*args, **kwargs):
        if args[0] == path:
            return val
        return exists(*args, **kwargs)

    return _mock_exists


def mock_read_text(path: os.PathLike, val: str):
    """Mock Path.read_text for specified path."""

    real_read_text = Path.read_text

    def _mock_read_text(*args, **kwargs):
        if args[0] == path:
            return val
        return real_read_text(*args, **kwargs)

    return _mock_read_text


def test_process_info_capture() -> None:
    """Test that ProcessDetector captures correct process info."""
    env_var = list(os.environ.keys())[0]

    d = detector.ProcessDetector(allowed_env=[env_var])
    attrs = d.detect().attributes

    assert attrs[resources.PROCESS_PID] == os.getpid()
    assert attrs[detector.PROCESS_CWD] == os.getcwd()
    assert attrs[resources.PROCESS_COMMAND] == sys.argv[0]
    assert attrs[resources.PROCESS_COMMAND_ARGS] == tuple(sys.argv[1:])
    assert attrs[resources.PROCESS_EXECUTABLE_NAME] == Path(sys.executable).name
    assert attrs[resources.PROCESS_EXECUTABLE_PATH] == sys.executable
    assert attrs[f"process.env.{env_var}"] == os.environ[env_var]


def test_system_info_captured(monkeypatch) -> None:
    """Test that SystemDetector captures the correct system info."""

    monkeypatch.setattr(getpass, "getuser", lambda: "someuser")
    monkeypatch.setattr(Path, "exists", mock_exists(detector.DMI_PATH, True))
    monkeypatch.setattr(
        Path,
        "read_text",
        mock_read_text(detector.DMI_PATH, detector.GCE_DMI),
    )

    d = detector.SystemDetector()
    attrs = d.detect().attributes

    assert attrs[detector.CPU_COUNT] == os.cpu_count()
    assert attrs[detector.HOST_TYPE] == "Google Compute Engine"
    assert attrs[detector.OS_NAME] == os.name
    assert attrs[resources.OS_TYPE] == platform.system()
    assert attrs[resources.OS_DESCRIPTION] == platform.platform()
    assert attrs[detector.CPU_ARCHITECTURE] == platform.machine()
    assert attrs[detector.CPU_NAME] == platform.processor()


def test_memory_info_class(monkeypatch) -> None:
    proc_meminfo_contents = """
SwapTotal: 15 kB
VmallocTotal: 25 kB
MemTotal: 35 kB
    """
    monkeypatch.setattr(Path, "exists",
                        mock_exists(detector.PROC_MEMINFO_PATH, True))
    monkeypatch.setattr(
        Path,
        "read_text",
        mock_read_text(detector.PROC_MEMINFO_PATH, proc_meminfo_contents),
    )

    m = detector.MemoryInfo()
    assert m.total_swap_memory == 15 * 1024
    assert m.total_physical_ram == 35 * 1024
    assert m.total_virtual_memory == 25 * 1024


def test_memory_info_class_warns_on_unexpected_unit(monkeypatch,
                                                    caplog) -> None:
    proc_meminfo_contents = """
SwapTotal: 15 mB
VmallocTotal: 25 gB
MemTotal: 35 tB
    """
    monkeypatch.setattr(Path, "exists",
                        mock_exists(detector.PROC_MEMINFO_PATH, True))
    monkeypatch.setattr(
        Path,
        "read_text",
        mock_read_text(detector.PROC_MEMINFO_PATH, proc_meminfo_contents),
    )
    caplog.set_level(logging.WARNING)

    m = detector.MemoryInfo()
    assert "Unit for memory consumption in /proc/meminfo" in caplog.text
    # We do not attempt to correct unexpected units
    assert m.total_swap_memory == 15 * 1024
    assert m.total_physical_ram == 35 * 1024
    assert m.total_virtual_memory == 25 * 1024


def test_memory_info_class_no_units(monkeypatch) -> None:
    proc_meminfo_contents = """
SwapTotal: 15
    """
    monkeypatch.setattr(Path, "exists",
                        mock_exists(detector.PROC_MEMINFO_PATH, True))
    monkeypatch.setattr(
        Path,
        "read_text",
        mock_read_text(detector.PROC_MEMINFO_PATH, proc_meminfo_contents),
    )

    m = detector.MemoryInfo()
    assert m.total_swap_memory == 15


def test_memory_info_class_no_provided_value(monkeypatch, caplog) -> None:
    proc_meminfo_contents = """
SwapTotal:
    """
    monkeypatch.setattr(Path, "exists",
                        mock_exists(detector.PROC_MEMINFO_PATH, True))
    monkeypatch.setattr(
        Path,
        "read_text",
        mock_read_text(detector.PROC_MEMINFO_PATH, proc_meminfo_contents),
    )
    caplog.set_level(logging.WARNING)

    detector.MemoryInfo()
    assert "Unexpected /proc/meminfo entry with no label:number" in caplog.text


def test_system_info_to_capture_memory_resources(monkeypatch) -> None:
    proc_meminfo_contents = """
SwapTotal: 15 kB
VmallocTotal: 25 kB
MemTotal: 35 kB
    """
    monkeypatch.setattr(Path, "exists",
                        mock_exists(detector.PROC_MEMINFO_PATH, True))
    monkeypatch.setattr(
        Path,
        "read_text",
        mock_read_text(detector.PROC_MEMINFO_PATH, proc_meminfo_contents),
    )

    d = detector.SystemDetector()
    attrs = d.detect().attributes

    assert attrs[detector.MEMORY_TOTAL] == 35 * 1024
    assert attrs[detector.MEMORY_SWAP_TOTAL] == 15 * 1024


def test_system_info_to_capture_host_type_bot(monkeypatch) -> None:
    """Test that SystemDetector captures host type as Google Compute Engine."""

    monkeypatch.setattr(Path, "exists", mock_exists(detector.DMI_PATH, True))
    monkeypatch.setattr(
        Path,
        "read_text",
        mock_read_text(detector.DMI_PATH, detector.GCE_DMI),
    )

    d = detector.SystemDetector()
    attrs = d.detect().attributes

    assert attrs[detector.CPU_COUNT] == os.cpu_count()
    assert attrs[detector.HOST_TYPE] == detector.GCE_DMI
    assert attrs[detector.OS_NAME] == os.name
    assert attrs[resources.OS_TYPE] == platform.system()
    assert attrs[resources.OS_DESCRIPTION] == platform.platform()
    assert attrs[detector.CPU_ARCHITECTURE] == platform.machine()
    assert attrs[detector.CPU_NAME] == platform.processor()


def test_system_info_to_capture_host_type_from_dmi(monkeypatch) -> None:
    """Test that SystemDetector captures dmi product name as host type."""

    monkeypatch.setattr(getpass, "getuser", lambda: "someuser")
    monkeypatch.setattr(Path, "exists", mock_exists(detector.DMI_PATH, True))
    monkeypatch.setattr(Path, "read_text",
                        mock_read_text(detector.DMI_PATH, "SomeId"))

    d = detector.SystemDetector()
    attrs = d.detect().attributes

    assert attrs[detector.CPU_COUNT] == os.cpu_count()
    assert attrs[detector.HOST_TYPE] == "SomeId"
    assert attrs[detector.OS_NAME] == os.name
    assert attrs[resources.OS_TYPE] == platform.system()
    assert attrs[resources.OS_DESCRIPTION] == platform.platform()
    assert attrs[detector.CPU_ARCHITECTURE] == platform.machine()
    assert attrs[detector.CPU_NAME] == platform.processor()


def test_system_info_to_capture_host_type_unknown(monkeypatch) -> None:
    """Test that SystemDetector captures host type as UNKNOWN."""

    monkeypatch.setattr(Path, "exists", mock_exists(detector.DMI_PATH, False))

    d = detector.SystemDetector()
    attrs = d.detect().attributes

    assert attrs[detector.CPU_COUNT] == os.cpu_count()
    assert attrs[detector.HOST_TYPE] == "UNKNOWN"
    assert attrs[detector.OS_NAME] == os.name
    assert attrs[resources.OS_TYPE] == platform.system()
    assert attrs[resources.OS_DESCRIPTION] == platform.platform()
    assert attrs[detector.CPU_ARCHITECTURE] == platform.machine()
    assert attrs[detector.CPU_NAME] == platform.processor()
