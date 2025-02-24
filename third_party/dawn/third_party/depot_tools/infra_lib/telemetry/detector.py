# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Defines the ResourceDetector to capture resource properties."""

import logging
import os
from pathlib import Path
import platform
import sys
from typing import Optional, Sequence

from opentelemetry.sdk import resources

CPU_ARCHITECTURE = "cpu.architecture"
CPU_NAME = "cpu.name"
CPU_COUNT = "cpu.count"
HOST_TYPE = "host.type"
MEMORY_SWAP_TOTAL = "memory.swap.total"
MEMORY_TOTAL = "memory.total"
PROCESS_CWD = "process.cwd"
PROCESS_RUNTIME_API_VERSION = "process.runtime.apiversion"
PROCESS_ENV = "process.env"
OS_NAME = "os.name"
DMI_PATH = Path("/sys/class/dmi/id/product_name")
GCE_DMI = "Google Compute Engine"
PROC_MEMINFO_PATH = Path("/proc/meminfo")


class ProcessDetector(resources.ResourceDetector):
    """ResourceDetector to capture information about the process."""

    def __init__(self, allowed_env: Optional[Sequence[str]] = None) -> None:
        super().__init__()
        self._allowed_env = allowed_env or ["USE"]

    def detect(self) -> resources.Resource:
        env = os.environ
        resource = {
            PROCESS_CWD: os.getcwd(),
            PROCESS_RUNTIME_API_VERSION: sys.api_version,
            resources.PROCESS_PID: os.getpid(),
            resources.PROCESS_OWNER: os.geteuid(),
            resources.PROCESS_EXECUTABLE_NAME: Path(sys.executable).name,
            resources.PROCESS_EXECUTABLE_PATH: sys.executable,
            resources.PROCESS_COMMAND: sys.argv[0],
            resources.PROCESS_COMMAND_ARGS: sys.argv[1:],
        }
        resource.update({
            f"{PROCESS_ENV}.{k}": env[k]
            for k in self._allowed_env if k in env
        })

        return resources.Resource(resource)


class SystemDetector(resources.ResourceDetector):
    """ResourceDetector to capture information about system."""

    def detect(self) -> resources.Resource:
        host_type = "UNKNOWN"

        if DMI_PATH.exists():
            host_type = DMI_PATH.read_text(encoding="utf-8")

        mem_info = MemoryInfo()

        resource = {
            CPU_ARCHITECTURE: platform.machine(),
            CPU_COUNT: os.cpu_count(),
            CPU_NAME: platform.processor(),
            HOST_TYPE: host_type.strip(),
            MEMORY_SWAP_TOTAL: mem_info.total_swap_memory,
            MEMORY_TOTAL: mem_info.total_physical_ram,
            OS_NAME: os.name,
            resources.OS_TYPE: platform.system(),
            resources.OS_DESCRIPTION: platform.platform(),
        }

        return resources.Resource(resource)


class MemoryInfo:
    """Read machine memory info from /proc/meminfo."""

    # Prefixes for the /proc/meminfo file that we care about.
    MEMINFO_VIRTUAL_MEMORY_TOTAL = "VmallocTotal"
    MEMINFO_PHYSICAL_RAM_TOTAL = "MemTotal"
    MEMINFO_SWAP_MEMORY_TOTAL = "SwapTotal"

    def __init__(self) -> None:
        self._total_physical_ram = 0
        self._total_virtual_memory = 0
        self._total_swap_memory = 0
        try:
            contents = PROC_MEMINFO_PATH.read_text(encoding="utf-8")
        except OSError as e:
            logging.warning("Encountered an issue reading /proc/meminfo: %s", e)
            return

        for line in contents.splitlines():
            if line.startswith(self.MEMINFO_SWAP_MEMORY_TOTAL):
                self._total_swap_memory = self._get_mem_value(line)
            elif line.startswith(self.MEMINFO_VIRTUAL_MEMORY_TOTAL):
                self._total_virtual_memory = self._get_mem_value(line)
            elif line.startswith(self.MEMINFO_PHYSICAL_RAM_TOTAL):
                self._total_physical_ram = self._get_mem_value(line)

    @property
    def total_physical_ram(self) -> int:
        return self._total_physical_ram

    @property
    def total_virtual_memory(self) -> int:
        return self._total_virtual_memory

    @property
    def total_swap_memory(self) -> int:
        return self._total_swap_memory

    def _get_mem_value(self, line: str) -> int:
        """Reads an individual line from /proc/meminfo and returns the size.

        This function also converts the read value from kibibytes to bytes
        when the read value has a unit provided for memory size.

        The specification information for /proc files, including meminfo, can
        be found at
        https://www.kernel.org/doc/Documentation/filesystems/proc.txt.

        Args:
            line: The text line read from /proc/meminfo.

        Returns:
            The integer value after conversion.
        """
        components = line.split()
        if len(components) == 1:
            logging.warning(
                "Unexpected /proc/meminfo entry with no label:number value was "
                "provided. Value read: '%s'",
                line,
            )
            return 0
        size = int(components[1])
        if len(components) == 2:
            return size
        # The RHEL and kernel.org specs for /proc/meminfo doesn't give any
        # indication that a memory unit besides kB (kibibytes) is expected,
        # except in the cases of page counts, where no unit is provided.
        if components[2] != "kB":
            logging.warning(
                "Unit for memory consumption in /proc/meminfo does "
                "not conform to expectations. Please review the "
                "read value: %s",
                line,
            )
        return size * 1024
