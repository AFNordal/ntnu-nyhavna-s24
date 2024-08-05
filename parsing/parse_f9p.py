from argparse import ArgumentParser
from pyubx2 import UBXReader, UBX_PROTOCOL, ubxhelpers
import datetime


def get_fname():
    parser = ArgumentParser()
    parser.add_argument("file")
    args = parser.parse_args()
    return args.file


def main():
    fname = get_fname()
    with open(fname, "rb") as file:
        parser = UBXReader(file, protfilter=UBX_PROTOCOL, parsebitfield=True)
        for raw, parsed in parser:
            if parsed.identity == "TIM-TM2":
                if not parsed.newRisingEdge:
                    continue
                assert parsed.newFallingEdge
                # print(parsed.wnR)
                # print(parsed.towMsR)
                # print(parsed.towSubMsR)
                print(ubxhelpers.itow2utc(parsed.towMsR))
                # break
                # print(parsed.config_poll(0, 1, ["count"]))


if __name__ == "__main__":
    main()
