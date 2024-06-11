import asyncio
import aioconsole
import serial_asyncio
from wtools import get_url


async def main():
    com = get_url()
    print(f"{com=}")
    reader, writer = await serial_asyncio.open_serial_connection(url=com, 
                                                                 baudrate=115200,
                                                                 timeout=0.05)
    async def run_reader():
        while True:
            print((await reader.read(1)).decode(errors='backslashreplace'), end='')
            
    async def run_writer():
        while True:
            if data := await aioconsole.ainput():
                writer.write(data.encode() + b'\n\r')
                await writer.drain()
    await asyncio.gather(run_reader(), run_writer())
        
asyncio.run(main())

