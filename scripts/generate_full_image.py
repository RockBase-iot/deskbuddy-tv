#!/usr/bin/env python3
"""
Post-build script for PlatformIO.
Merges bootloader, partition table, firmware and optional filesystem into a
single binary that can be written starting at flash address 0x0.

Example output:
    .pio/build/nm-tv-154/full_image.bin

Flash command:
    esptool.py --chip esp32 write_flash 0x0 .pio/build/nm-tv-154/full_image.bin
"""

import csv
import os
import subprocess
from os.path import basename, isfile, join

Import("env")  # noqa: F821


def _find_esptool(env):
    pkg_dir = env.PioPlatform().get_package_dir("tool-esptoolpy")
    candidates = [
        join(pkg_dir, "esptool.py"),
        join(pkg_dir, "esptool", "esptool.py"),
    ]
    for path in candidates:
        if isfile(path):
            return path
    raise FileNotFoundError("esptool.py not found in tool-esptoolpy package")


def _resolve_partition_csv(env):
    """Return absolute path to the active partition CSV."""
    project_dir = env.subst("$PROJECT_DIR")
    csv_name = env.BoardConfig().get("build.partitions", "default.csv")

    # 1. project root (custom partition table)
    project_path = join(project_dir, csv_name)
    if isfile(project_path):
        return project_path

    # 2. framework partition tables directory
    framework_dir = env.PioPlatform().get_package_dir("framework-arduinoespressif32")
    framework_path = join(framework_dir, "tools", "partitions", csv_name)
    if isfile(framework_path):
        return framework_path

    raise FileNotFoundError(f"Partition table not found: {csv_name}")


def _parse_partition_table(csv_path):
    """Parse ESP32 partition table CSV into {name: {type, subtype, offset}}."""
    partitions = {}
    with open(csv_path, "r", encoding="utf-8") as f:
        reader = csv.reader(f, skipinitialspace=True)
        for row in reader:
            if not row or row[0].startswith("#"):
                continue
            name, ptype, subtype, offset = row[0], row[1], row[2], row[3]
            partitions[name] = {
                "type": ptype,
                "subtype": subtype,
                "offset": int(offset, 0),
            }
    return partitions


def _find_app_offset(partitions):
    """Use the first app partition (factory or ota_0) as firmware offset."""
    for name, info in partitions.items():
        if info["type"] == "app":
            return info["offset"]
    raise ValueError("No app partition found in partition table")


def _find_fs_offset(partitions):
    """Return offset of the filesystem partition, if present."""
    for name, info in partitions.items():
        if info["type"] == "data" and info["subtype"] in ("spiffs", "littlefs"):
            return info["offset"]
    return None


def generate_full_image(source, target, env):
    build_dir = env.subst("$BUILD_DIR")
    bootloader = join(build_dir, "bootloader.bin")
    partitions = join(build_dir, "partitions.bin")
    firmware = join(build_dir, "firmware.bin")

    required = [bootloader, partitions, firmware]
    missing = [f for f in required if not isfile(f)]
    if missing:
        print(f"[full_image] Skip: missing {', '.join(basename(f) for f in missing)}")
        return

    try:
        csv_path = _resolve_partition_csv(env)
        partitions_info = _parse_partition_table(csv_path)
        app_offset = _find_app_offset(partitions_info)
        fs_offset = _find_fs_offset(partitions_info)
    except Exception as exc:
        print(f"[full_image] Skip: failed to parse partition table: {exc}")
        return

    output = join(build_dir, "full_image.bin")
    if isfile(output):
        try:
            os.remove(output)
        except OSError as exc:
            print(f"[full_image] Warning: could not remove old {output}: {exc}")

    # Filesystem image may be littlefs.bin or spiffs.bin depending on board_build.filesystem.
    fs_image = None
    if fs_offset is not None:
        for candidate in ("littlefs.bin", "spiffs.bin"):
            path = join(build_dir, candidate)
            if isfile(path):
                fs_image = path
                break

    esptool = _find_esptool(env)
    python = env.subst("$PYTHONEXE")

    cmd = [
        python,
        esptool,
        "--chip",
        "esp32",
        "merge_bin",
        "--output",
        output,
        "--fill-flash-size",
        "4MB",
        "0x1000",
        bootloader,
        "0x8000",
        partitions,
        hex(app_offset),
        firmware,
    ]
    if fs_image:
        cmd.extend([hex(fs_offset), fs_image])

    print(f"[full_image] Generating {output}")
    print(f"[full_image] app offset={hex(app_offset)}, fs offset={hex(fs_offset) if fs_offset else 'none'}")

    try:
        subprocess.check_call(cmd)
        print(f"[full_image] OK -> {output}")
    except subprocess.CalledProcessError as exc:
        print(f"[full_image] Failed: {exc}")


# Run after the main firmware target finishes.
env.AddPostAction("buildprog", generate_full_image)
