from pathlib import Path
import asyncio
import time
import serial
from websockets.legacy.server import Serve, WebSocketServerProtocol
from wtools import get_url
import serial.tools.list_ports
import win32api


def load_script(script: bytes):
    # path = Path("E:\\").joinpath("flash.uf2")
    drive = Path("D:\\")
    for idx in range(20):

        print(f"Looking for pico dir {idx}")

        if drive.exists():
            info = win32api.GetVolumeInformation(str(drive))
            assert info[0] == "RPI-RP2"
            print("Trying to flash")
            with drive.joinpath("flash.uf2").open("wb") as f:
                f.write(script)
            print("Wrote script")
            return
        try:
            serial.serial_for_url(url=get_url(), baudrate=1200)
        except Exception as e:
            pass
        time.sleep(0.5)

    raise Exception("Could not flash")


async def forward(websocket: WebSocketServerProtocol):
    loop = asyncio.get_running_loop()
    for _ in range(20):
        try:
            ser = serial.serial_for_url(url=get_url(), baudrate=9000, timeout=0.1)
            break
        except Exception as e:
            time.sleep(0.2)
    else:
        raise Exception("Connection error")

    async def reader():
        while websocket.open:
            if data := await loop.run_in_executor(None, ser.read, 4096):
                await websocket.send(data)

    async def writer():
        while websocket.open:
            if data := await websocket.recv():
                await loop.run_in_executor(None, ser.write, data)

    # async def watchdog():
    #     while websocket.open:
    #         await asyncio.sleep(0.5)
    #         ser.write(b"\xff\x01\xff\xff")

    await asyncio.gather(reader(), writer())
    ser.close()


async def handle(websocket: WebSocketServerProtocol):

    if websocket.path == "/flash":
        data = await websocket.recv()
        load_script(data)
        await websocket.send(b"OK")

    elif websocket.path == "/forward":
        while True:
            await forward(websocket)

    else:
        raise Exception("Unknown path")


async def main():

    async with Serve(handle, "localhost", 8765, ping_timeout=None):
        await asyncio.Future()


if __name__ == "__main__":
    asyncio.run(main())
