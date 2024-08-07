from argparse import ArgumentParser
from pyubx2 import UBXReader, UBX_PROTOCOL, ubxhelpers
from matplotlib import pyplot as plt
import numpy as np

NS_IN_WEEK = 7 * 24 * 60 * 60 * 1000 * 1000 * 1000
NS_IN_MS = 1000 * 1000



def itow2ns(week_nr, tow_ms, remainder_ns):
    return week_nr * NS_IN_WEEK + tow_ms * NS_IN_MS + remainder_ns

def parse_file(fname):
    with open(fname, "rb") as file:
        parser = UBXReader(file, protfilter=UBX_PROTOCOL, parsebitfield=True)
        return list([parsed for raw, parsed in parser])
    
def get_timestamps(msgs):
    times = []
    for msg in msgs:
        if msg.identity == "TIM-TM2":
            if not msg.newRisingEdge:
                continue
            time_ns = itow2ns(msg.wnR, msg.towMsR, msg.towSubMsR)
            times.append(time_ns)
    return times
