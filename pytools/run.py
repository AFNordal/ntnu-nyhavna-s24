import asyncio
from pathlib import Path
from wsl_link import build_and_flash, connect_over_ws
from websockets.legacy.client import WebSocketClientProtocol


async def reader(sock: WebSocketClientProtocol):
    async for msg in sock:
        print(msg.decode(errors="replace"))


async def writer(sock: WebSocketClientProtocol):
    while True:
        data = await asyncio.get_running_loop().run_in_executor(None, input)
        await sock.send((data + "\r\n").encode())


async def main():
    build_dir = Path(__file__).parents[1] / "tracking-box" / "sd-test" / "build"
    build_dir.mkdir(exist_ok=True)
    print(build_dir)
    project_dir = Path(__file__).parents[1] / "tracking-box" / "sd-test"
    await build_and_flash(build_dir, project_dir, build_type="Release")

    await connect_over_ws(reader, writer)


if __name__ == "__main__":
    asyncio.run(main())
