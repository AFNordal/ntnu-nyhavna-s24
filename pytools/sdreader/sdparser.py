import pathlib
import re

data_dir = pathlib.Path(__file__).parent / 'data'
latest_trace = sorted(data_dir.glob('transfer_*'))[-1]
serials = sorted((latest_trace /'serialLog').glob('*'))

data = bytearray()
for serial in serials:
    data.extend(serial.read_bytes())
print(len(data))

sep = re.compile(b"\xfe([\x00-\xfd])(.*?)(?=\xfe[\x00-\xfd])", flags=re.DOTALL)
for m in sep.finditer(data):
    print(m)
    # id_ = int.from_bytes(m[1], 'little')
    # if id_ == 10:
    #     print(m[2])