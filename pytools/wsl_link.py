from pathlib import Path
import asyncio
import shlex
from websockets.legacy.client import Connect
import logging
import sys


async def cmake(build_dir: Path, project_dir: Path, build_type="Release"):
    cmd = f"cmake -DCMAKE_BUILD_TYPE={build_type} {project_dir}"
    proc = await asyncio.create_subprocess_exec(*shlex.split(cmd), cwd=build_dir)
    await proc.wait()
    assert proc.returncode == 0


async def make(build_dir: Path):
    cmd = "make -j8"
    proc = await asyncio.create_subprocess_exec(*shlex.split(cmd), cwd=build_dir)
    await proc.wait()
    assert proc.returncode == 0


async def build_and_flash(build_dir: Path, project_dir: Path, build_type="Release"):
    await cmake(build_dir, project_dir, build_type)
    await make(build_dir)
    assert len(uf2s := list(build_dir.glob("*.uf2"))) == 1
    assert len(binary := uf2s[0].read_bytes()) > 0
    print(f"Sending script of size {len(binary)} bytes")
    url = "ws://host.docker.internal:8765/flash"
    async with Connect(url, ping_timeout=None) as sock:
        await sock.send(binary)
        try:
            return await sock.recv()
        except Exception as e:
            print("Error while flashing", e)
            sys.exit(0)


async def connect_over_ws(reader, writer):
    url = "ws://host.docker.internal:8765/forward"
    async with Connect(url, ping_timeout=None) as sock:
        await asyncio.gather(reader(sock), writer(sock))
